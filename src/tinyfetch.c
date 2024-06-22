// tinyfetch Copyright (C) 2024 kernaltrap8
// This program comes with ABSOLUTELY NO WARRANTY
// This is free software, and you are welcome to redistribute it
// under certain conditions

/*
    tinyfetch.c
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>
#ifdef __linux__
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <sys/sysinfo.h>
#endif
#ifdef __FreeBSD__
#include <fcntl.h>
#include <kvm.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#endif
#include "tinyascii.h"
#include "tinyfetch.h"

#define PCI_DETECTION 1

#if PCI_DETECTION == 1
#include <pci/pci.h>
#endif

/*
    file parsing
*/

int file_parser(const char *file, const char *line_to_read) {
  FILE *meminfo = fopen(file, "r"); // open the file to parse
  if (meminfo == NULL) {
    return -1; // return an error code if the file doesnt exist
  }

  char line[256]; // char buffer
  while (fgets(line, sizeof(line), meminfo)) {
    int ram; // int data type to store the memory info in to print
    if (sscanf(line, line_to_read, &ram) == 1) {
      fclose(meminfo); // close the file
      return ram;      // return the contents
    }
  }

  fclose(meminfo); // close the file
  return -1;       // negative exit code. if we get here, an error occurred.
}

char *file_parser_char(const char *file, const char *line_to_read) {
  FILE *meminfo = fopen(file, "r"); // open the file to parse
  if (meminfo == NULL) {
    return NULL; // return an error code if the file doesnt exist
  }

  char line[256]; // char buffer
  while (fgets(line, sizeof(line), meminfo)) {
    char *parsed_string =
        (char *)malloc(strlen(line) + 1); // allocate memory for the string
    if (!parsed_string) {
      fclose(meminfo);
      return NULL;
    }

    if (sscanf(line, line_to_read, parsed_string) == 1) {
      fclose(meminfo);
      return parsed_string;
    }

    free(parsed_string);
  }

  fclose(meminfo); // close the file
  return NULL;     // null exit code. if we get here, an error occurred.
}
/*
    hostname handling
*/

char *get_hostname(void) {
  char hostname[256];
  if (gethostname(hostname, sizeof(hostname)) ==
      0) { // if the gethostname command works, return the value from it.
           // otherise return a nullptr.
    return strdup(hostname);
  } else {
    return NULL;
  }
}

/*
    FreeBSD sysctl calling
*/

#ifdef __FreeBSD__
char *freebsd_sysctl(char *ctlname) {
  char buf[1024];
  size_t buf_size = sizeof(buf);

  if (sysctlbyname(ctlname, buf, &buf_size, NULL, 0) == -1) {
    perror("sysctlbyname");
    return NULL;
  }

  char *ctlreturn = strdup(buf);
  if (ctlreturn == NULL) {
    perror("strdup");
    return NULL;
  }

  return ctlreturn;
}

int freebsd_sysctl_int(const char *ctlname) {
  int value;
  size_t len = sizeof(value);

  // Convert the name to a sysctl MIB and retrieve the value
  if (sysctlbyname(ctlname, &value, &len, NULL, 0) == -1) {
    perror("sysctlbyname");
    return -1;
  }

  return value;
}

long long longlong_freebsd_sysctl(const char *ctlname) {
  long long value;
  size_t len = sizeof(value);

  if (sysctlbyname(ctlname, &value, &len, NULL, 0) == -1) {
    perror("sysctlbyname");
    return -1;
  }

  return value;
}
#endif

/*
    shell detection
*/

#ifdef __linux__
char *get_parent_shell(void) {
  pid_t ppid = getppid(); // get parent proc ID
  char cmdline_path[64];
  snprintf(cmdline_path, sizeof(cmdline_path), CMDLINE_PATH, ppid);

  FILE *cmdline_file = fopen(cmdline_path, "r"); // open /proc/%d/cmdline
  if (cmdline_file == NULL) {
    return NULL; // return NULL if cmdline_file doesnt exist
  }

  char cmdline[1024]; // was 256
  if (fgets(cmdline, sizeof(cmdline), cmdline_file) ==
      NULL) { // if something bad happened, free memory and return nullptr
    fclose(cmdline_file);
    return NULL;
  }

  fclose(cmdline_file); // close the file

  cmdline[strcspn(cmdline, "\n")] = '\0';

  if (cmdline[0] == '-') { // NOTE: this fixes a bug that occurs sometimes
                           // either when using tmux or Konsole.
    memmove(cmdline, cmdline + 1, strlen(cmdline));
  }

  char *newline_pos =
      strchr(cmdline, '\n'); // seek newline, if its there, remove it.
  if (newline_pos != NULL) {
    *newline_pos = '\0';
  }

  if (!strncmp(cmdline, "/bin/", 5) ||
      !strncmp(cmdline, "/usr/local/bin", 15)) {
    return strdup(cmdline + ((cmdline[1] == 'u') ? 15 : 5));
  }
  if (!strncmp(cmdline, "/usr/bin/", 9)) {
    return strdup(cmdline + 9);
  }

  return strdup(cmdline); // return the contents of cmdline
}
#endif
#ifdef __FreeBSD__
char *get_parent_shell_noproc(void) {
  char *shell_path = getenv("SHELL");
  if (shell_path == NULL) {
    return NULL; // $SHELL not set
  }

  char *shell_name =
      strrchr(shell_path, '/'); // find the last occurrence of '/'
  if (shell_name == NULL) {
    shell_name =
        shell_path; // if '/' not found, the entire path is the shell name
  } else {
    shell_name++; // move past the '/'
  }

  // remove characters if needed
  char *newline_pos = strchr(shell_name, '\n');
  if (newline_pos != NULL) {
    *newline_pos = '\0';
  }

  return strdup(shell_name);
}
#endif

#ifdef __linux__
long int get_uptime(void) {
  struct sysinfo s_info; // define struct for sysinfo
  int e = sysinfo(&s_info);
  if (e != 0) {
    return -1;
  }

  return s_info.uptime; // return uptime
}
#endif
#ifdef __FreeBSD__
long int get_uptime_freebsd(void) {
  int mib[2];
  size_t len;
  struct timeval boottime;

  mib[0] = CTL_KERN;
  mib[1] = KERN_BOOTTIME;

  len = sizeof(boottime);
  if (sysctl(mib, 2, &boottime, &len, NULL, 0) == -1) {
    perror("sysctl");
    return -1;
  }

  time_t now = time(NULL);
  time_t uptime = now - boottime.tv_sec;

  return uptime;
}
#endif

void format_uptime(long int uptime) {
  int hours, minutes, seconds;

  hours = uptime / 3600;
  uptime %= 3600;
  minutes = uptime / 60;
  seconds = uptime % 60;

  printf("%d hours, %d minutes, %d seconds\n", hours, minutes, seconds);
}

/*
        GPU detection
*/

#if PCI_DETECTION == 1
char *get_gpu_name() {
  struct pci_access *pacc;
  struct pci_dev *dev;
  char namebuf[1024], *name;

  pacc = pci_alloc();
  pci_init(pacc);
  pci_scan_bus(pacc);

  for (dev = pacc->devices; dev; dev = dev->next) {
    pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
    if ((dev->device_class == PCI_CLASS_DISPLAY_VGA) ||
        (dev->device_class == PCI_CLASS_DISPLAY_3D)) {

      name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE,
                             dev->vendor_id, dev->device_id);
      if (!name) {
        return NULL;
      } else {
        pci_cleanup(pacc);
        return strdup(name);
      }
    }
  }
  return 0;
}
#endif

/*
    main printing functions
*/

void pretext(const char *string) {
  printf("%s", string);
  fflush(stdout); // flush stdout, just in case.
}

void fetchinfo(char *structname) { printf("%s\n", structname); }

void tinyinit(void) {
  if (uname(&tiny) == -1) {
    perror("uname");
  }
}

void rand_string(void) {
  if (rand_enable == 1) {
    srand(time(NULL));
    int num_strings = sizeof(strings) / sizeof(strings[0]);
    int n = rand() % num_strings;
    fflush(stdout);
    printf("%s %s\n", decoration, strings[n]);
  }
}

void trim_spaces(char *str) {
  int len = strlen(str);
  while (len > 0 && isspace(str[len - 1])) {
    str[--len] = '\0';
  }
}

void message(char *message) {
  // the syntax used for this function
  // is weird. in the tinyfetch()
  // function, we are passing argv[2]
  // from main into the char *msg, which is then passed here and printed out.
  // but this only executes if custom_message is 1, which gets modified
  // when -m is passed into tinyfetch.
  if (custom_message == 1) {
    fflush(stdout);
    printf("%s %s\n", decoration, message);
  }
}

int get_cpu_count(void) {
#ifdef __linux__
  return sysconf(_SC_NPROCESSORS_ONLN);
#endif
#ifdef __FreeBSD__
  int cpu_count = freebsd_sysctl_int("hw.ncpu");
  return cpu_count;
#endif
}

#ifdef __FreeBSD__
int get_swap_stats(long long *total, long long *used, long long *free) {
  (*total) = -1;
  (*used) = -1;
  (*free) = -1;
  kvm_t *kvmh = NULL;
  long page_s = sysconf(_SC_PAGESIZE);
  kvmh = kvm_open(NULL, "/dev/null", "/dev/null", O_RDONLY, NULL);
  if (!kvmh)
    return -1;
  struct kvm_swap k_swap;
  if (kvm_getswapinfo(kvmh, &k_swap, 1, 0) != -1) {
    (*total) = k_swap.ksw_total * page_s;
    (*used) = k_swap.ksw_used * page_s;
    (*free) = (k_swap.ksw_total - k_swap.ksw_used) * page_s;
  } else
    return -1;
  if (kvm_close(kvmh) == -1)
    return -1;
  return 0;
}
#endif

void tinyascii(void) {
  if (ascii_enable == 1) {
    char *distro_name =
        file_parser_char("/etc/os-release", "PRETTY_NAME=\"%s\"");
    // int distro_name[] = {'k'};
    (distro_name[0] == 'A' || distro_name[0] == 'a')
        ? (tinyascii_p1 = a_p1, tinyascii_p2 = a_p2, tinyascii_p3 = a_p3,
           tinyascii_p4 = a_p4, tinyascii_p5 = a_p5, tinyascii_p6 = a_p6,
           tinyascii_p7 = a_p7, tinyascii_p8 = a_p8, tinyascii_p9 = a_p9)
        : NULL;
    (distro_name[0] == 'B' || distro_name[0] == 'b')
        ? (tinyascii_p1 = b_p1, tinyascii_p2 = b_p2, tinyascii_p3 = b_p3,
           tinyascii_p4 = b_p4, tinyascii_p5 = b_p5, tinyascii_p6 = b_p6,
           tinyascii_p7 = b_p7, tinyascii_p8 = b_p8, tinyascii_p9 = b_p9)
        : NULL;
    (distro_name[0] == 'C' || distro_name[0] == 'c')
        ? (tinyascii_p1 = c_p1, tinyascii_p2 = c_p2, tinyascii_p3 = c_p3,
           tinyascii_p4 = c_p4, tinyascii_p5 = c_p5, tinyascii_p6 = c_p6,
           tinyascii_p7 = c_p7, tinyascii_p8 = c_p8, tinyascii_p9 = c_p9)
        : NULL;
    (distro_name[0] == 'D' || distro_name[0] == 'd')
        ? (tinyascii_p1 = d_p1, tinyascii_p2 = d_p2, tinyascii_p3 = d_p3,
           tinyascii_p4 = d_p4, tinyascii_p5 = d_p5, tinyascii_p6 = d_p6,
           tinyascii_p7 = d_p7, tinyascii_p8 = d_p8, tinyascii_p9 = d_p9)
        : NULL;
    (distro_name[0] == 'E' || distro_name[0] == 'e')
        ? (tinyascii_p1 = e_p1, tinyascii_p2 = e_p2, tinyascii_p3 = e_p3,
           tinyascii_p4 = e_p4, tinyascii_p5 = e_p5, tinyascii_p6 = e_p6,
           tinyascii_p7 = e_p7, tinyascii_p8 = e_p8, tinyascii_p9 = e_p9)
        : NULL;
    (distro_name[0] == 'F' || distro_name[0] == 'f')
        ? (tinyascii_p1 = f_p1, tinyascii_p2 = f_p2, tinyascii_p3 = f_p3,
           tinyascii_p4 = f_p4, tinyascii_p5 = f_p5, tinyascii_p6 = f_p6,
           tinyascii_p7 = f_p7, tinyascii_p8 = f_p8, tinyascii_p9 = f_p9)
        : NULL;
    (distro_name[0] == 'G' || distro_name[0] == 'g')
        ? (tinyascii_p1 = g_p1, tinyascii_p2 = g_p2, tinyascii_p3 = g_p3,
           tinyascii_p4 = g_p4, tinyascii_p5 = g_p5, tinyascii_p6 = g_p6,
           tinyascii_p7 = g_p7, tinyascii_p8 = g_p8, tinyascii_p9 = g_p9)
        : NULL;
    (distro_name[0] == 'H' || distro_name[0] == 'h')
        ? (tinyascii_p1 = h_p1, tinyascii_p2 = h_p2, tinyascii_p3 = h_p3,
           tinyascii_p4 = h_p4, tinyascii_p5 = h_p5, tinyascii_p6 = h_p6,
           tinyascii_p7 = h_p7, tinyascii_p8 = h_p8, tinyascii_p9 = h_p9)
        : NULL;
    (distro_name[0] == 'I' || distro_name[0] == 'i')
        ? (tinyascii_p1 = i_p1, tinyascii_p2 = i_p2, tinyascii_p3 = i_p3,
           tinyascii_p4 = i_p4, tinyascii_p5 = i_p5, tinyascii_p6 = i_p6,
           tinyascii_p7 = i_p7, tinyascii_p8 = i_p8, tinyascii_p9 = i_p9)
        : NULL;
    (distro_name[0] == 'J' || distro_name[0] == 'j')
        ? (tinyascii_p1 = j_p1, tinyascii_p2 = j_p2, tinyascii_p3 = j_p3,
           tinyascii_p4 = j_p4, tinyascii_p5 = j_p5, tinyascii_p6 = j_p6,
           tinyascii_p7 = j_p7, tinyascii_p8 = j_p8, tinyascii_p9 = j_p9)
        : NULL;
    (distro_name[0] == 'K' || distro_name[0] == 'k')
        ? (tinyascii_p1 = k_p1, tinyascii_p2 = k_p2, tinyascii_p3 = k_p3,
           tinyascii_p4 = k_p4, tinyascii_p5 = k_p5, tinyascii_p6 = k_p6,
           tinyascii_p7 = k_p7, tinyascii_p8 = k_p8, tinyascii_p9 = k_p9)
        : NULL;

    free(distro_name);
  }
}

void tinyuser(void) {
  tinyinit();
  char *user = getenv("USER");
  printf("%s@", user); // username@, username is taken from char* user
  if (user != NULL) {
    int total_length = strlen(user) + strlen(tiny.nodename) + 1;
    printf("%s\n", tiny.nodename);
    for (int i = 0; i < total_length; i++) {
      printf("-");
    }
    printf("\n");
  } else {
    perror("strlen");
  }
}

void tinyos(void) {
  tinyinit();
  if (ascii_enable == 1)
    printf("%s", tinyascii_p1);
  pretext(pretext_OS);
  // tiny.sysname doesnt return what uname -o would, so here we check if it ==
  // Linux, and if it does, print GNU/ before tiny.sysname
  if (!strcmp(tiny.sysname, "Linux")) {
    printf("GNU/");
  }
  fetchinfo(tiny.sysname); // OS name
}

void tinydist(void) {
  if (ascii_enable == 1)
    printf("%s", tinyascii_p2);
  pretext(pretext_distro);
  char *distro_name = file_parser_char("/etc/os-release",
                                       "NAME=%s"); // parsing and isolating the
                                                   // PRETTY_NAME and VERSON_ID
  char *distro_ver =
      file_parser_char("/etc/os-release", "VERSION_ID=\"%[^\"]\"%*c");
  if (!strcmp(distro_name, "(null)") || !strcmp(distro_ver, "(null)")) {
    distro_name = "UNIX-Like OS";
    distro_ver = " ";
  }
  if (!strcmp(distro_ver, "(null)")) {
    distro_ver = "";
  }
  tinyinit();
  printf("%s %s %s \n", distro_name, distro_ver, tiny.machine);
  free(distro_name);
  free(distro_ver);
}

void tinykern(void) {
  tinyinit();
  if (ascii_enable == 1)
    printf("%s", tinyascii_p3);
  pretext(pretext_kernel);
  fetchinfo(tiny.release); // gets kernel name
}

void tinyshell(void) {
  if (ascii_enable == 1)
    printf("%s", tinyascii_p4);
  pretext(pretext_shell);
#ifdef __linux__
  char *shell = get_parent_shell();
#endif
#ifdef __FreeBSD__
  char *shell = get_parent_shell_noproc();
#endif
  printf("%s\n", shell);
  free(shell);
}

void tinyuptime(void) {
#ifdef __linux__
  long int uptime = get_uptime();
#endif
#ifdef __FreeBSD__
  long int uptime = get_uptime_freebsd();
#endif
  if (uptime == -1) {
    ;
  } else {
    if (ascii_enable == 1)
      printf("%s", tinyascii_p5);
    pretext(pretext_uptime);
    format_uptime(uptime);
  }
}

void tinywm(void) {
  char *wm = getenv("XDG_CURRENT_DESKTOP");
  if (wm != NULL) {
    if (ascii_enable == 1) {
      printf("%s", tinyascii_p6);
    }
    pretext(pretext_wm);
    printf("%s\n", wm); // wm variable taken from getenv()
  }
}

void tinyram(void) {
  if (ascii_enable == 1) {
    char *wm = getenv("XDG_CURRENT_DESKTOP");
    if (wm == NULL) {
      ;
    }
    printf("%s", tinyascii_p7);
  }
  pretext(pretext_ram);
#ifdef __linux__
  // process memory used and total avail.
  int memavail = file_parser("/proc/meminfo", "MemAvailable: %d kB");
  int total_ram = file_parser("/proc/meminfo", "MemTotal: %d kB");
  int ram_free = (memavail != -1)
                     ? memavail
                     : file_parser("/proc/meminfo", "MemFree: %d kB");

  if (total_ram != -1 && ram_free != -1) {
    int ram_used = total_ram - ram_free;

    // convert the values from /proc/meminfo into GiB double values
    double total_ram_gib = total_ram / (1024.0 * 1024.0);
    double ram_used_gib = ram_used / (1024.0 * 1024.0);
    double ram_free_gib = ram_free / (1024.0 * 1024.0);
    printf("%.2f GiB used / %.2f GiB total (%.2f GiB free)\n", ram_used_gib,
           total_ram_gib, ram_free_gib);
  }
#endif

#ifdef __FreeBSD__
  long long total_ram = longlong_freebsd_sysctl("hw.physmem");
  long free_ram = longlong_freebsd_sysctl("vm.stats.vm.v_free_count") *
                  sysconf(_SC_PAGESIZE);
  long long used_ram = total_ram - free_ram;
  double total_ram_gib = total_ram / (1024.0 * 1024.0 * 1024.0);
  double used_ram_gib = used_ram / (1024.0 * 1024.0 * 1024.0);
  double free_ram_gib = free_ram / (1024.0 * 1024.0 * 1024.0);
  printf("%.2f GiB used / %.2f GiB total (%.2f GiB free)\n", used_ram_gib,
         total_ram_gib, free_ram_gib);
#endif
}

void tinycpu(void) {
  tinyinit();
  if (ascii_enable == 1) {
    printf("%s", tinyascii_p8);
  }
  pretext(pretext_processor);
#ifdef __linux__
  char *cpu = file_parser_char("/proc/cpuinfo", "model name      : %[^\n]");
  char *cpu_fallback = file_parser_char("/proc/cpuinfo", "cpu      : %[^\n]");
  int cpu_count = get_cpu_count();
  if (cpu != NULL) {
    printf("%s (%d)\n", cpu, cpu_count);
    free(cpu);
  } else if (cpu_fallback != NULL) {
    printf("%s (%d)\n", cpu_fallback, cpu_count);
    free(cpu_fallback);
  }
#endif
#ifdef __FreeBSD__
  char *cpu = freebsd_sysctl("hw.model");
  trim_spaces(cpu);
  int cpu_count = get_cpu_count();
  if (cpu != NULL) {
    printf("%s (%d)\n", cpu, cpu_count);
  } else {
    printf("Unknown %s CPU (%d)\n", tiny.machine, cpu_count);
  }
#endif
}

void tinygpu(void) {
#if PCI_DETECTION == 1
  if (get_gpu_name() != NULL) {
    if (ascii_enable == 1) {
      printf("%s", tinyascii_p9);
    }
    pretext(pretext_gpu);
    printf("%s\n", get_gpu_name());
  }
#endif
}

void tinyswap(void) {
  if (ascii_enable == 1) {
    printf("%s", tinyascii_p9);
  }
  pretext(pretext_swap);
#ifdef __linux__
  int total_swap = file_parser("/proc/meminfo", "SwapTotal: %d kB");
  int swap_free = file_parser("/proc/meminfo", "SwapFree: %d kB");
  if (total_swap != -1 && swap_free != -1) {
    int swap_used = total_swap - swap_free;
    double swap_total_gib = total_swap / (1024.0 * 1024.0);
    double swap_used_gib = swap_used / (1024.0 * 1024.0);
    double swap_free_gib = swap_free / (1024.0 * 1024.0);
    printf("%.2f GiB used / %.2f GiB total (%.2f GiB free)\n", swap_used_gib,
           swap_total_gib, swap_free_gib);
  }
#endif
#ifdef __FreeBSD__
  long long total_swap = -1;
  long long used_swap = -1;
  long long free_swap = -1;
  if (get_swap_stats(&total_swap, &used_swap, &free_swap) != -1) {
    double total_swap_gib = total_swap / (1024.0 * 1024.0 * 1024.0);
    double used_swap_gib = used_swap / (1024.0 * 1024.0 * 1024.0);
    double free_swap_gib = free_swap / (1024.0 * 1024.0 * 1024.0);
    printf("%.2f GiB used / %.2f GiB total (%.2f GiB free)\n", used_swap_gib,
           total_swap_gib, free_swap_gib);
  }
#endif
}

void tinyfetch(char *msg) {
  tinyascii();
  tinyuser();
  rand_string();
  message(msg);
  tinyos();
  tinydist();
  tinykern();
  tinyshell();
  tinyuptime();
  tinywm();
  tinycpu();
  tinygpu();
  tinyram();
  tinyswap();
}

int isValidArgument(char *arg) {
  const char *validArgs[] = {"-v",
                             "-h",
                             "-m",
                             "-r",
                             "-o",
                             "-d",
                             "-k",
                             "-s",
                             "-u",
                             "-w",
                             "--ram",
                             "-c",
                             "--swap",
                             "--genie",
                             "--disable-ascii",
                             "--user",
                             "--custom-ascii"};
  size_t numArgs = sizeof(validArgs) / sizeof(validArgs[0]);
  for (size_t i = 0; i < numArgs; ++i) {
    if (strcmp(arg, validArgs[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    ascii_enable = 1;
    tinyfetch(NULL);
    return 0;
  }

  if (argc > 1) {
    if (!strcmp(argv[1], "--disable-ascii") && argc == 2) {
      // If only "-d" is passed, print basic system information
      tinyfetch(NULL);
    } else if (!strcmp(argv[1], "--disable-ascii") && !strcmp(argv[2], "-r")) {
      // If both "-d" and "-r" are passed, enable random string printing
      rand_enable = 1;
      tinyfetch(NULL);
    }
    if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
      printf("%s v%s\n", argv[0], VERSION);
      return 0;
    } else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
      printf("%s %s", decoration, help_banner);
      return 0;
    } else if (!strcmp(argv[1], "-m") || !strcmp(argv[1], "--message")) {
      if (argc < 3) {
        printf("no message provided.\n");
        return 1;
      }
      ascii_enable = 1;
      custom_message = 1;
      tinyfetch(argv[2]);
      return 0;
    } else if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--random")) {
      ascii_enable = 1;
      rand_enable = 1;
      tinyfetch(NULL);
    } else if (!strcmp(argv[1], "-o")) {
      tinyos();
    } else if (!strcmp(argv[1], "-d")) {
      tinydist();
    } else if (!strcmp(argv[1], "-k")) {
      tinykern();
    } else if (!strcmp(argv[1], "-s")) {
      tinyshell();
    } else if (!strcmp(argv[1], "-u")) {
      tinyuptime();
    } else if (!strcmp(argv[1], "-w")) {
      tinywm();
    } else if (!strcmp(argv[1], "--ram")) {
      tinyram();
    } else if (!strcmp(argv[1], "-c")) {
      tinycpu();
    } else if (!strcmp(argv[1], "--swap")) {
      tinyswap();
    } else if (!strcmp(argv[1], "--genie")) {
      rand_enable = 1;
      rand_string();
    } else if (!strcmp(argv[1], "--user")) {
      tinyuser();
    }

    if (argc == 2) {
      if (!isValidArgument(argv[1])) {
        printf("tinyfetch: Unknown command line argument.\n %s %s", decoration,
               help_banner);
        return 1;
      }
    } else if (argc == 3) {
      if (!isValidArgument(argv[2])) {
        printf("tinyfetch: Unknown command line argument.\n %s %s", decoration,
               help_banner);
        return 1;
      }
    }

  } else {
    printf("tinyfetch: Unknown command line argument.\n %s %s", decoration,
           help_banner);
  }
  return 0;
}
