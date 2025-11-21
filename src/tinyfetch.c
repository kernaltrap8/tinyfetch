// tinyfetch Copyright (C) 2024 kernaltrap8
// This program comes with ABSOLUTELY NO WARRANTY
// This is free software, and you are welcome to redistribute it
// under certain conditions

/*
    tinyfetch.c
*/

#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdbool.h>
#ifdef __linux__
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#endif
#if defined(__NetBSD__)
#include <sys/swap.h>
#include <sys/sysctl.h>
#include <time.h>
#endif
#if defined(__FreeBSD__) || defined(__MacOS__)
#include <kvm.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <time.h>
#endif
#include "config.h"
#include "tinyascii.h"
#include "tinyfetch.h"
#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__)
#if PCI_DETECTION == 1
#include <pci/pci.h>
#endif
#endif

/*
    file parsing
*/

ParserResult file_parser(const char *file, const char *line_to_read,
                         ReturnType returnType) {
  ParserResult result;
  char resolved_path[PATH_MAX];
  result.type = returnType; // Set the type in the result

  if (realpath(file, resolved_path) == NULL) {
    result.value.intValue = -1; // For int, set an error code
    return result;
  }

  FILE *meminfo = fopen(resolved_path, "r");
  if (meminfo == NULL) {
    perror("fopen");
    if (returnType == TYPE_STRING) {
      result.value.stringValue = NULL; // For string, set to NULL
    } else {
      result.value.intValue = -1;      // For int, set an error code
      result.value.doubleValue = -1.0; // For double, set an error value
    }
    return result;
  }

  char line[256];
  while (fgets(line, sizeof(line), meminfo)) {
    switch (returnType) {
    case TYPE_INT: {
      if (sscanf(line, line_to_read, &result.value.intValue) == 1) {
        fclose(meminfo);
        return result;
      }
      break;
    }
    case TYPE_DOUBLE: {
      if (sscanf(line, line_to_read, &result.value.doubleValue) == 1) {
        fclose(meminfo);
        return result;
      }
      break;
    }
    case TYPE_STRING: {
      result.value.stringValue = (char *)malloc(strlen(line) + 1);
      if (!result.value.stringValue) {
        perror("malloc");
        fclose(meminfo);
        return result;
      }
      if (sscanf(line, line_to_read, result.value.stringValue) == 1) {
        fclose(meminfo);
        return result;
      }
      free(result.value.stringValue);
      result.value.stringValue = NULL;
      break;
    }
    }
  }

  fclose(meminfo);
  // Return appropriate error based on type
  if (returnType == TYPE_STRING) {
    result.value.stringValue = NULL;
  } else {
    result.value.intValue = -1;
    result.value.doubleValue = -1.0;
  }

  return result;
}

/*
    hostname handling
*/

char *get_hostname(void) {
  char hostname[HOST_NAME_MAX];
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

#if defined(__FreeBSD__) || defined(__MacOS__) || defined(__NetBSD__)
char *freebsd_sysctl_str(char *ctlname) {
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

#define freebsd_sysctl(CTLNAME, VALUE)                                         \
  do {                                                                         \
    size_t len = sizeof(VALUE);                                                \
    if (sysctlbyname(CTLNAME, &VALUE, &len, NULL, 0) == -1) {                  \
      perror("sysctlbyname");                                                  \
      VALUE = -1;                                                              \
    }                                                                          \
  } while (0);
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
#if defined(__FreeBSD__) || defined(__MacOS__) || defined(__NetBSD__)
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
#if defined(__FreeBSD__) || defined(__MacOS__) || defined(__NetBSD__)
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
  int days, hours, minutes, seconds;

  days = uptime / 86400;
  uptime %= 86400;
  hours = uptime / 3600;
  uptime %= 3600;
  minutes = uptime / 60;
  seconds = uptime % 60;

  int first = 1; // To handle comma placement correctly

  if (days > 0) {
    printf("%d day%s", days, days > 1 ? "s" : "");
    first = 0;
  }
  if (hours > 0) {
    if (!first)
      printf(", ");
    printf("%d hour%s", hours, hours > 1 ? "s" : "");
    first = 0;
  }
  if (minutes > 0) {
    if (!first)
      printf(", ");
    printf("%d minute%s", minutes, minutes > 1 ? "s" : "");
    first = 0;
  }
  if (seconds > 0 && days == 0 &&
      hours == 0) { // Only show seconds if under an hour
    if (!first)
      printf(", ");
    printf("%d second%s", seconds, seconds > 1 ? "s" : "");
  }

  printf("\n");
}

/*
        GPU detection
*/

#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__)
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
      if (name) {
        char *result = strdup(name);
        pci_cleanup(pacc);
        return result;
      }
    }
  }

  pci_cleanup(pacc);
  return NULL;
}

#endif
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

unsigned long generate_random_index(unsigned long *seed, int array_size) {
  // Initialize the seed using /dev/urandom
  int urandom = open("/dev/urandom", O_RDONLY);
  if (urandom < 0) {
    perror("Unable to open /dev/urandom");
    exit(EXIT_FAILURE);
  }
  if (read(urandom, seed, sizeof(*seed)) < 0) {
    perror("Unable to read from /dev/urandom");
    close(urandom);
    exit(EXIT_FAILURE);
  }
  close(urandom);

  // Generate a random number using the LCG
  *seed = (MULTIPLIER * (*seed) + INCREMENT) % MODULUS;

  // Return a random index within the array size
  return *seed % array_size;
}

void rand_string(void) {
  if (rand_enable == 1) {
    int num_strings = sizeof(strings) / sizeof(strings[0]);
    unsigned long seed;
    unsigned long n = generate_random_index(&seed, num_strings);
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

int get_swap_status(void) {
#ifdef __linux__
  struct sysinfo info;
  if (sysinfo(&info) != 0) {
    perror("sysinfo");
    return -1;
  }
  if (info.totalswap == 0) {
    return 0; // No swap available
  }
#endif
#if defined(__FreeBSD__) || defined(__MacOS__) || defined(__NetBSD__)
  long long total, used, free;
  if (get_swap_stats(&total, &used, &free) != 0) {
    return -1; // Error in fetching swap stats
  }
  if (total == 0) {
    return 0; // No swap available
  }
#endif
  return 1; // Swap available
}

int get_cpu_count(void) {
#ifdef __linux__
  return sysconf(_SC_NPROCESSORS_ONLN);
#endif
#if defined(__FreeBSD__) || defined(__MacOS__) || defined(__NetBSD__)
  int cpu_count = 0;
  freebsd_sysctl("hw.ncpu", cpu_count);
  return cpu_count;
#endif
}

#if defined(__NetBSD__)
int get_swap_stats(long long *total, long long *used, long long *free_mem) {
  (*total) = -1;
  (*used) = -1;
  (*free_mem) = -1;
  int nswap = swapctl(SWAP_NSWAP, NULL, 0);
  (*total) = 0;
  (*used) = 0;
  (*free_mem) = 0;
  if (nswap == 0)
    return 0;
  struct swapent *ent = malloc(sizeof(*ent) * nswap);
  int devices = swapctl(SWAP_STATS, ent, nswap);
  int i;
  for (i = 0; i < devices; i++) {
    (*total) += ent[i].se_nblks;
    (*used) += ent[i].se_inuse;
    (*free_mem) += ent[i].se_nblks - ent[i].se_inuse;
  }
  (*total) *= 512;
  (*used) *= 512;
  (*free_mem) *= 512;
  free(ent);
  return 0;
}
#endif

#if defined(__FreeBSD__) || defined(__MacOS__)
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

void tinyascii(char *TinyfetchUserSpecifiedDistroChar) {
  char *distro_name = NULL;

  if (ascii_enable == 1) {
#ifdef __NetBSD__
    distro_name = strdup("NetBSD");
#else
    ParserResult result =
        file_parser("/etc/os-release", "PRETTY_NAME=\"%[^\"]\"", TYPE_STRING);

    // Debug output to check the result
    if (result.type == TYPE_STRING && result.value.stringValue) {
      distro_name = strdup(result.value.stringValue);
      free(result.value.stringValue); // Free the original if allocated
    }
#endif

    // If distro_name is still NULL, set to "Generic Linux"
    if (!distro_name) {
      distro_name = strdup("Generic Linux");
    } else {
      // Remove surrounding quotes if present
      size_t len = strlen(distro_name);
      if (len && *distro_name == '"') {
        memmove(distro_name, distro_name + 1, len); // Trim leading quote
        len--;
      }
      if (len && distro_name[len - 1] == '"') {
        distro_name[len - 1] = '\0'; // Trim trailing quote
      }

      // Check if distro_name is empty and assign default if necessary
      if (!*distro_name) {
        free(distro_name);
        distro_name = strdup("Generic Linux");
      }
    }
    
    char first_char;
    
    // Matching the first character
    if (TinyfetchUserSpecifiedDistroChar != NULL) {
        // Use the custom distro character provided by user
        first_char = TinyfetchUserSpecifiedDistroChar[0];
    } else {
        // Use the first character of the detected distro name
        first_char = distro_name[0];
    }

    tinyascii_p1 = (first_char == 'A' || first_char == 'a')   ? a_p1
                   : (first_char == 'B' || first_char == 'b') ? b_p1
                   : (first_char == 'C' || first_char == 'c') ? c_p1
                   : (first_char == 'D' || first_char == 'd') ? d_p1
                   : (first_char == 'E' || first_char == 'e') ? e_p1
                   : (first_char == 'F' || first_char == 'f') ? f_p1
                   : (first_char == 'G' || first_char == 'g') ? g_p1
                   : (first_char == 'H' || first_char == 'h') ? h_p1
                   : (first_char == 'I' || first_char == 'i') ? i_p1
                   : (first_char == 'J' || first_char == 'j') ? j_p1
                   : (first_char == 'K' || first_char == 'k') ? k_p1
                                                              : NULL;

    tinyascii_p2 = (first_char == 'A' || first_char == 'a')   ? a_p2
                   : (first_char == 'B' || first_char == 'b') ? b_p2
                   : (first_char == 'C' || first_char == 'c') ? c_p2
                   : (first_char == 'D' || first_char == 'd') ? d_p2
                   : (first_char == 'E' || first_char == 'e') ? e_p2
                   : (first_char == 'F' || first_char == 'f') ? f_p2
                   : (first_char == 'G' || first_char == 'g') ? g_p2
                   : (first_char == 'H' || first_char == 'h') ? h_p2
                   : (first_char == 'I' || first_char == 'i') ? i_p2
                   : (first_char == 'J' || first_char == 'j') ? j_p2
                   : (first_char == 'K' || first_char == 'k') ? k_p2
                                                              : NULL;

    tinyascii_p3 = (first_char == 'A' || first_char == 'a')   ? a_p3
                   : (first_char == 'B' || first_char == 'b') ? b_p3
                   : (first_char == 'C' || first_char == 'c') ? c_p3
                   : (first_char == 'D' || first_char == 'd') ? d_p3
                   : (first_char == 'E' || first_char == 'e') ? e_p3
                   : (first_char == 'F' || first_char == 'f') ? f_p3
                   : (first_char == 'G' || first_char == 'g') ? g_p3
                   : (first_char == 'H' || first_char == 'h') ? h_p3
                   : (first_char == 'I' || first_char == 'i') ? i_p3
                   : (first_char == 'J' || first_char == 'j') ? j_p3
                   : (first_char == 'K' || first_char == 'k') ? k_p3
                                                              : NULL;

    tinyascii_p4 = (first_char == 'A' || first_char == 'a')   ? a_p4
                   : (first_char == 'B' || first_char == 'b') ? b_p4
                   : (first_char == 'C' || first_char == 'c') ? c_p4
                   : (first_char == 'D' || first_char == 'd') ? d_p4
                   : (first_char == 'E' || first_char == 'e') ? e_p4
                   : (first_char == 'F' || first_char == 'f') ? f_p4
                   : (first_char == 'G' || first_char == 'g') ? g_p4
                   : (first_char == 'H' || first_char == 'h') ? h_p4
                   : (first_char == 'I' || first_char == 'i') ? i_p4
                   : (first_char == 'J' || first_char == 'j') ? j_p4
                   : (first_char == 'K' || first_char == 'k') ? k_p4
                                                              : NULL;

    tinyascii_p5 = (first_char == 'A' || first_char == 'a')   ? a_p5
                   : (first_char == 'B' || first_char == 'b') ? b_p5
                   : (first_char == 'C' || first_char == 'c') ? c_p5
                   : (first_char == 'D' || first_char == 'd') ? d_p5
                   : (first_char == 'E' || first_char == 'e') ? e_p5
                   : (first_char == 'F' || first_char == 'f') ? f_p5
                   : (first_char == 'G' || first_char == 'g') ? g_p5
                   : (first_char == 'H' || first_char == 'h') ? h_p5
                   : (first_char == 'I' || first_char == 'i') ? i_p5
                   : (first_char == 'J' || first_char == 'j') ? j_p5
                   : (first_char == 'K' || first_char == 'k') ? k_p5
                                                              : NULL;

    tinyascii_p6 = (first_char == 'A' || first_char == 'a')   ? a_p6
                   : (first_char == 'B' || first_char == 'b') ? b_p6
                   : (first_char == 'C' || first_char == 'c') ? c_p6
                   : (first_char == 'D' || first_char == 'd') ? d_p6
                   : (first_char == 'E' || first_char == 'e') ? e_p6
                   : (first_char == 'F' || first_char == 'f') ? f_p6
                   : (first_char == 'G' || first_char == 'g') ? g_p6
                   : (first_char == 'H' || first_char == 'h') ? h_p6
                   : (first_char == 'I' || first_char == 'i') ? i_p6
                   : (first_char == 'J' || first_char == 'j') ? j_p6
                   : (first_char == 'K' || first_char == 'k') ? k_p6
                                                              : NULL;

    tinyascii_p7 = (first_char == 'A' || first_char == 'a')   ? a_p7
                   : (first_char == 'B' || first_char == 'b') ? b_p7
                   : (first_char == 'C' || first_char == 'c') ? c_p7
                   : (first_char == 'D' || first_char == 'd') ? d_p7
                   : (first_char == 'E' || first_char == 'e') ? e_p7
                   : (first_char == 'F' || first_char == 'f') ? f_p7
                   : (first_char == 'G' || first_char == 'g') ? g_p7
                   : (first_char == 'H' || first_char == 'h') ? h_p7
                   : (first_char == 'I' || first_char == 'i') ? i_p7
                   : (first_char == 'J' || first_char == 'j') ? j_p7
                   : (first_char == 'K' || first_char == 'k') ? k_p7
                                                              : NULL;

    tinyascii_p8 = (first_char == 'A' || first_char == 'a')   ? a_p8
                   : (first_char == 'B' || first_char == 'b') ? b_p8
                   : (first_char == 'C' || first_char == 'c') ? c_p8
                   : (first_char == 'D' || first_char == 'd') ? d_p8
                   : (first_char == 'E' || first_char == 'e') ? e_p8
                   : (first_char == 'F' || first_char == 'f') ? f_p8
                   : (first_char == 'G' || first_char == 'g') ? g_p8
                   : (first_char == 'H' || first_char == 'h') ? h_p8
                   : (first_char == 'I' || first_char == 'i') ? i_p8
                   : (first_char == 'J' || first_char == 'j') ? j_p8
                   : (first_char == 'K' || first_char == 'k') ? k_p8
                                                              : NULL;

    tinyascii_p9 = (first_char == 'A' || first_char == 'a')   ? a_p9
                   : (first_char == 'B' || first_char == 'b') ? b_p9
                   : (first_char == 'C' || first_char == 'c') ? c_p9
                   : (first_char == 'D' || first_char == 'd') ? d_p9
                   : (first_char == 'E' || first_char == 'e') ? e_p9
                   : (first_char == 'F' || first_char == 'f') ? f_p9
                   : (first_char == 'G' || first_char == 'g') ? g_p9
                   : (first_char == 'H' || first_char == 'h') ? h_p9
                   : (first_char == 'I' || first_char == 'i') ? i_p9
                   : (first_char == 'J' || first_char == 'j') ? j_p9
                   : (first_char == 'K' || first_char == 'k') ? k_p9
                                                              : NULL;

    free(distro_name); // Free the duplicated string if allocated
  }
}

void tinyuser(void) {
  tinyinit();
  // char *user = getenv("USER");
  char *user = getlogin();
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
  if (ascii_enable == 1) {
    printf("%s", tinyascii_p2); // Ensure tinyascii_p2 is defined elsewhere
  }

  pretext(pretext_distro);

#ifdef __NetBSD__
  char *distro_display = strdup("NetBSD");
#else
  char *distro_display = NULL;

  // first, try PRETTY_NAME
  ParserResult pretty_res =
      file_parser("/etc/os-release", "PRETTY_NAME=\"%[^\"]\"", TYPE_STRING);
  if (pretty_res.type == TYPE_STRING &&
      pretty_res.value.stringValue != NULL) {
    distro_display = strdup(pretty_res.value.stringValue);
    free(pretty_res.value.stringValue);
  }

  // if PRETTY_NAME is not found, fall back to NAME + VERSION
  if (distro_display == NULL) {
    // NAME
    ParserResult name_res =
        file_parser("/etc/os-release", "NAME=\"%[^\"]\"", TYPE_STRING);
    char *distro_name = NULL;
    if (name_res.type == TYPE_STRING &&
        name_res.value.stringValue != NULL) {
      distro_name = strdup(name_res.value.stringValue);
      free(name_res.value.stringValue);
    }

    // VERSION_ID → VERSION → VERSION_CODENAME
    char *distro_ver = NULL;
    ParserResult ver_res;

    // VERSION_ID
    ver_res = file_parser("/etc/os-release", "VERSION_ID=\"%[^\"]\"", TYPE_STRING);
    if (ver_res.type == TYPE_STRING && ver_res.value.stringValue != NULL) {
      distro_ver = strdup(ver_res.value.stringValue);
      free(ver_res.value.stringValue);
    }

    // VERSION
    if (distro_ver == NULL) {
      ver_res = file_parser("/etc/os-release", "VERSION=\"%[^\"]\"", TYPE_STRING);
      if (ver_res.type == TYPE_STRING && ver_res.value.stringValue != NULL) {
        distro_ver = strdup(ver_res.value.stringValue);
        free(ver_res.value.stringValue);
      }
    }

    // VERSION_CODENAME
    if (distro_ver == NULL) {
      ver_res = file_parser("/etc/os-release", "VERSION_CODENAME=%s", TYPE_STRING);
      if (ver_res.type == TYPE_STRING && ver_res.value.stringValue != NULL) {
        distro_ver = strdup(ver_res.value.stringValue);
        free(ver_res.value.stringValue);
      }
    }

    // Stitch name + version if either is present
    if (distro_name && distro_ver) {
      size_t len = strlen(distro_name) + 1 + strlen(distro_ver) + 1;
      distro_display = malloc(len);
      snprintf(distro_display, len, "%s %s", distro_name, distro_ver);
    } else if (distro_name) {
      distro_display = strdup(distro_name);
    } else if (distro_ver) {
      distro_display = strdup(distro_ver);
    }

    if (distro_name) free(distro_name);
    if (distro_ver) free(distro_ver);
  }
#endif

  // Final fallback
  if (distro_display == NULL) {
    distro_display = strdup("Generic Linux Unknown Version");
  }

  tinyinit();
  printf("%s %s\n", distro_display, tiny.machine);

  free(distro_display);
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
#if defined(__FreeBSD__) || defined(__MacOS__) || defined(__NetBSD__)
  char *shell = get_parent_shell_noproc();
#endif
  printf("%s\n", shell);
  free(shell);
}

void tinyuptime(void) {
#ifdef __linux__
  long int uptime = get_uptime();
#endif
#if defined(__FreeBSD__) || defined(__MacOS__) || defined(__NetBSD__)
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
    printf("%s", tinyascii_p7);
  }

  pretext(pretext_ram);

#if defined(__linux__) || defined(__NetBSD__)
  // Process memory used and total available
  ParserResult memavail =
      file_parser("/proc/meminfo", "MemAvailable: %d kB", TYPE_INT);
  ParserResult total_ram =
      file_parser("/proc/meminfo", "MemTotal: %d kB", TYPE_INT);

  ParserResult ram_free =
      (memavail.type == TYPE_INT && memavail.value.intValue != -1)
          ? memavail
          : file_parser("/proc/meminfo", "MemFree: %d kB", TYPE_INT);

  if (total_ram.type == TYPE_INT && ram_free.type == TYPE_INT) {
    int ram_used = total_ram.value.intValue - ram_free.value.intValue;

    // Convert the values from /proc/meminfo into MiB double values
    double total_ram_mib = total_ram.value.intValue / 1024.0;
    double ram_used_mib = ram_used / 1024.0;
    double ram_free_mib = ram_free.value.intValue / 1024.0;

    // Print used RAM in MiB or GiB
    if (ram_used_mib < 1024) {
      printf("%.2f MiB used / ", ram_used_mib);
    } else {
      printf("%.2f GiB used / ", ram_used_mib / 1024.0);
    }

    // Print total RAM in MiB or GiB
    if (total_ram_mib < 1024) {
      printf("%.2f MiB total (", total_ram_mib);
    } else {
      printf("%.2f GiB total (", total_ram_mib / 1024.0);
    }

    // Print free RAM in MiB or GiB
    if (ram_free_mib < 1024) {
      printf("%.2f MiB free)\n", ram_free_mib);
    } else {
      printf("%.2f GiB free)\n", ram_free_mib / 1024.0);
    }
  }
#endif

#if defined(__FreeBSD__) || defined(__MacOS__)
  size_t total_ram_bytes = 0;
  freebsd_sysctl("hw.physmem", &total_ram_bytes);

  size_t cached_pages = 0;
  freebsd_sysctl("vm.stats.vm.v_cache_count", &cached_pages);

  size_t inactive_pages = 0;
  freebsd_sysctl("vm.stats.vm.v_inactive_count", &inactive_pages);

  size_t free_pages = 0;
  freebsd_sysctl("vm.stats.vm.v_free_count", &free_pages);

  size_t accurate_free_pages = cached_pages + inactive_pages + free_pages;
  size_t free_ram_bytes = accurate_free_pages * sysconf(_SC_PAGESIZE);

  size_t used_ram_bytes = total_ram_bytes - free_ram_bytes;

  double total_ram_mib = total_ram_bytes / (1024.0 * 1024.0);
  double used_ram_mib = used_ram_bytes / (1024.0 * 1024.0);
  double free_ram_mib = free_ram_bytes / (1024.0 * 1024.0);

  // Print used RAM in MiB or GiB
  if (used_ram_mib < 1024) {
    printf("%.2f MiB used / ", used_ram_mib);
  } else {
    printf("%.2f GiB used / ", used_ram_mib / 1024.0);
  }

  // Print total RAM in MiB or GiB
  if (total_ram_mib < 1024) {
    printf("%.2f MiB total (", total_ram_mib);
  } else {
    printf("%.2f GiB total (", total_ram_mib / 1024.0);
  }

  // Print free RAM in MiB or GiB
  if (free_ram_mib < 1024) {
    printf("%.2f MiB free)\n", free_ram_mib);
  } else {
    printf("%.2f GiB free)\n", free_ram_mib / 1024.0);
  }
#endif
}

void tinycpu(void) {
  tinyinit();

  if (ascii_enable == 1) {
    printf("%s", tinyascii_p8);
  }

  pretext(pretext_processor);

#ifdef __linux__
  ParserResult cpu =
      file_parser("/proc/cpuinfo", "model name      : %[^\n]", TYPE_STRING);
  ParserResult cpu_fallback =
      file_parser("/proc/cpuinfo", "cpu      : %[^\n]", TYPE_STRING);

  ParserResult cpu_freq =
      file_parser("/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq",
                  "%lf", TYPE_DOUBLE);

  double formatted_freq = (cpu_freq.type == TYPE_DOUBLE)
                              ? cpu_freq.value.doubleValue / 1000000
                              : 0.0;

  int cpu_count = get_cpu_count();

  if (cpu.type == TYPE_STRING && cpu.value.stringValue != NULL) {
    printf("%s (%d) @ %.2fGHz\n", cpu.value.stringValue, cpu_count,
           formatted_freq);
    free(cpu.value.stringValue); // Free allocated string
  } else if (cpu_fallback.type == TYPE_STRING &&
             cpu_fallback.value.stringValue != NULL) {
    printf("%s (%d)\n", cpu_fallback.value.stringValue, cpu_count);
    free(cpu_fallback.value.stringValue); // Free allocated string
  } else {
    printf("Unknown CPU (%d)\n", cpu_count);
  }
#endif

#if defined(__FreeBSD__) || defined(__MacOS__) || defined(__NetBSD__)
  char *cpu = NULL;

#ifdef __NetBSD__
  cpu = freebsd_sysctl_str("machdep.cpu_brand");
  if (cpu == NULL) {
    cpu = freebsd_sysctl_str("hw.model");
  }
#else
  cpu = freebsd_sysctl_str("hw.model");
#endif

  if (cpu != NULL) {
    trim_spaces(cpu);
    int cpu_count = get_cpu_count();
    printf("%s (%d)\n", cpu, cpu_count);
    free(cpu); // Free allocated string
  } else {
    printf("Unknown %s CPU (%d)\n", tiny.machine, get_cpu_count());
  }
#endif
}

#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__)
void tinygpu(void) {
#if PCI_DETECTION == 1
  char *gpu = get_gpu_name();
  if (gpu != NULL) {
    if (ascii_enable == 1) {
      printf("%s", tinyascii_p9);
    }
    pretext(pretext_gpu);
    printf("%s\n", gpu);
    free(gpu);
  }
#endif
}
#endif

void tinyswap(void) {
  if (get_swap_status() != 1) {
    return;
  }

  if (ascii_enable == 1) {
    printf("%s", tinyascii_p9);
  }

  pretext(pretext_swap);

#ifdef __linux__
  ParserResult total_swap =
      file_parser("/proc/meminfo", "SwapTotal: %d kB", TYPE_INT);
  ParserResult swap_free =
      file_parser("/proc/meminfo", "SwapFree: %d kB", TYPE_INT);

  if (total_swap.type == TYPE_INT && swap_free.type == TYPE_INT) {
    int swap_total_kb = total_swap.value.intValue;
    int swap_free_kb = swap_free.value.intValue;
    int swap_used_kb = swap_total_kb - swap_free_kb;

    // Convert the values into MiB double values
    double swap_total_mib = swap_total_kb / 1024.0;
    double swap_used_mib = swap_used_kb / 1024.0;
    double swap_free_mib = swap_free_kb / 1024.0;

    // Print used swap in MiB or GiB
    if (swap_used_mib < 1024) {
      printf("%.2f MiB used / ", swap_used_mib);
    } else {
      printf("%.2f GiB used / ", swap_used_mib / 1024.0);
    }

    // Print total swap in MiB or GiB
    if (swap_total_mib < 1024) {
      printf("%.2f MiB total (", swap_total_mib);
    } else {
      printf("%.2f GiB total (", swap_total_mib / 1024.0);
    }

    // Print free swap in MiB or GiB
    if (swap_free_mib < 1024) {
      printf("%.2f MiB free)\n", swap_free_mib);
    } else {
      printf("%.2f GiB free)\n", swap_free_mib / 1024.0);
    }

    // Free resources if applicable (if memory was dynamically allocated)
    // free(total_swap.value.intValue);
    // free(swap_free.value.intValue);
  }
#endif

#if defined(__FreeBSD__) || defined(__MacOS__) || defined(__NetBSD__)
  long long total_swap = -1;
  long long used_swap = -1;
  long long free_swap = -1;

  if (get_swap_stats(&total_swap, &used_swap, &free_swap) != -1) {
    double total_swap_mib = total_swap / (1024.0 * 1024.0);
    double used_swap_mib = used_swap / (1024.0 * 1024.0);
    double free_swap_mib = free_swap / (1024.0 * 1024.0);

    // Print used swap in MiB or GiB
    if (used_swap_mib < 1024) {
      printf("%.2f MiB used / ", used_swap_mib);
    } else {
      printf("%.2f GiB used / ", used_swap_mib / 1024.0);
    }

    // Print total swap in MiB or GiB
    if (total_swap_mib < 1024) {
      printf("%.2f MiB total (", total_swap_mib);
    } else {
      printf("%.2f GiB total (", total_swap_mib / 1024.0);
    }

    // Print free swap in MiB or GiB
    if (free_swap_mib < 1024) {
      printf("%.2f MiB free)\n", free_swap_mib);
    } else {
      printf("%.2f GiB free)\n", free_swap_mib / 1024.0);
    }
  }
#endif
}

void tinyfetch(char *msg, char *DistroArt) {
  if (!DistroArt) {
  	tinyascii(NULL);
  } else if (DistroArt) {
  	tinyascii(DistroArt);
  }
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
                             "--custom-ascii",
                             "-g",
                             "--custom-art"};
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
    tinyfetch(NULL, NULL);
    return 0;
  }

  if (!strcmp(argv[1], "--custom-art")) {
  	ascii_enable = 1;
  	TinyfetchUseUserSpecifiedDistroArt = true;
  	tinyfetch(NULL, argv[2]);
  	return 0;
  }

  if (argc > 1) {
    if (!strcmp(argv[1], "--disable-ascii") && argc == 2) {
      // If only "-d" is passed, print basic system information
      tinyfetch(NULL, NULL);
    } else if (!strcmp(argv[1], "--disable-ascii") && !strcmp(argv[2], "-r")) {
      // If both "-d" and "-r" are passed, enable random string printing
      rand_enable = 1;
      tinyfetch(NULL, NULL);
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
      tinyfetch(argv[2], NULL);
      return 0;
    } else if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--random")) {
      ascii_enable = 1;
      rand_enable = 1;
      tinyfetch(NULL, NULL);
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
    } else if (!strcmp(argv[1], "-g")) {
      tinygpu();
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
