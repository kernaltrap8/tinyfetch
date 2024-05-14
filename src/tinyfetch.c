// tinyfetch Copyright (C) 2024 kernaltrap8
// This program comes with ABSOLUTELY NO WARRANTY
// This is free software, and you are welcome to redistribute it
// under certain conditions

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
#else
#include <sys/sysctl.h>
#endif
#include "tinyfetch.h"

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
        shell detection
*/

char *get_parent_shell(void) {
  pid_t ppid = getppid(); // get parent proc ID
  char cmdline_path[64];
  snprintf(cmdline_path, sizeof(cmdline_path), CMDLINE_PATH, ppid);

  FILE *cmdline_file = fopen(cmdline_path, "r"); // open /proc/%d/cmdline
  if (cmdline_file == NULL) {
    return NULL; // return NULL if cmdline_file doesnt exist
  }

  char cmdline[256];
  if (fgets(cmdline, sizeof(cmdline), cmdline_file) ==
      NULL) { // if something bad happened, free memory and return nullptr
    fclose(cmdline_file);
    return NULL;
  }

  fclose(cmdline_file); // close the file

  if (cmdline[0] == '-') { // NOTE: this fixes a bug that occurs sometimes
                           // either when using tmux or Konsole.
    memmove(cmdline, cmdline + 1, strlen(cmdline));
  }

  char *newline_pos =
      strchr(cmdline, '\n'); // seek newline, if its there, remove it.
  if (newline_pos != NULL) {
    *newline_pos = '\0';
  }

  if (!strncmp(cmdline, "/bin/", 5)) {
    return strdup(cmdline + 5); // return string with /bin/ removed, if it
                                // exists in the first place.
  }

  return strdup(cmdline); // return the contents of cmdline
}

/*
        get uptime
*/

#ifdef __linux__
long int get_uptime(void) {
  struct sysinfo s_info; // define struct for sysinfo
  int e = sysinfo(&s_info);
  if (e != 0) {
    return -1;
  }

  return s_info.uptime; // return uptime
}

void format_uptime(long int uptime) {
  int hours, minutes, seconds;

  hours = uptime / 3600;
  uptime %= 3600;
  minutes = uptime / 60;
  seconds = uptime % 60;

  printf("%d hours, %d minutes, %d seconds\n", hours, minutes, seconds);
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

int get_cpu_count(void) {
#ifdef __linux__
  return sysconf(_SC_NPROCESSORS_ONLN);
#else
  int cpu_count;
  size_t count_size = sizeof(cpu_count);
  if (sysctlbyname("hw.ncpu", &cpu_count, &count_size, NULL, 0) == -1) {
    perror("sysctlbyname");
    return -1;
  }
  return cpu_count;
#endif
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
  pretext(pretext_OS);
  // tiny.sysname doesnt return what uname -o would, so here we check if it ==
  // Linux, and if it does, print GNU/ before tiny.sysname
  if (!strcmp(tiny.sysname, "Linux")) {
    printf("GNU/");
  }
  fetchinfo(tiny.sysname); // OS name
}

void tinydist(void) {
  pretext(pretext_distro);
  char *distro_name = file_parser_char("/etc/os-release",
                                       "NAME=%s"); // parsing and isolating the
                                                   // PRETTY_NAME and VERSON_ID
  char *distro_ver =
      file_parser_char("/etc/os-release", "VERSION_ID=\"%[^\"]\"%*c");
  if (!strcmp(distro_name, "(null)") && !strcmp(distro_ver, "(null)")) {
    distro_name = "UNIX-Like OS";
    distro_ver = " ";
  }
  tinyinit();
  printf("%s %s %s \n", distro_name, distro_ver, tiny.machine);
  free(distro_name);
  free(distro_ver);
}

void tinykern(void) {
  tinyinit();
  pretext(pretext_kernel);
  fetchinfo(tiny.release); // gets kernel name
}

void tinyshell(void) {
  pretext(pretext_shell);
  char *shell = get_parent_shell();
  if (shell == NULL) {
    free(shell);
    shell = getenv("SHELL");
  }
  printf("%s\n", shell);
  free(shell);
}

#ifdef __linux__ // only include this code if __linux__ is defined
void tinyuptime(void) {
  long int uptime = get_uptime();
  if (uptime == -1) {
    ;
  } else {
    pretext(pretext_uptime);
    format_uptime(uptime);
  }
}
#endif

void tinywm(void) {
  char *wm = getenv("XDG_CURRENT_DESKTOP");
  if (wm != NULL) {
    pretext(pretext_wm);
    printf("%s\n", wm); // wm variable taken from getenv()
  }
}

void tinyram(void) {
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
    pretext(pretext_ram);
    printf("%.2f GiB used / %.2f GiB total (%.2f GiB free)\n", ram_used_gib,
           total_ram_gib, ram_free_gib);
  }
}

void tinycpu(void) {
  tinyinit();
  pretext(pretext_processor);
  char *cpu = file_parser_char("/proc/cpuinfo", "model name      : %[^\n]");
  char *cpu_fallback = file_parser_char("/proc/cpuinfo", "cpu      : %[^\n]");
  int cpu_count = get_cpu_count();
  if (cpu != NULL) {
    printf("%s (%d)\n", cpu, cpu_count);
    free(cpu);
  } else if (cpu_fallback != NULL) {
    printf("%s (%d)\n", cpu_fallback, cpu_count);
    free(cpu_fallback);
  } else {
    printf("Unknown %s CPU (%d)\n", tiny.machine, cpu_count);
  }
}

void tinyswap(void) {
  int total_swap = file_parser("/proc/meminfo", "SwapTotal: %d kB");
  int swap_free = file_parser("/proc/meminfo", "SwapFree: %d kB");
  if (total_swap != -1 && swap_free != -1) {
    int swap_used = total_swap - swap_free;
    double swap_total_gib = total_swap / (1024.0 * 1024.0);
    double swap_used_gib = swap_used / (1024.0 * 1024.0);
    pretext(pretext_swap);
    printf("%.2f GiB used / %.2f GiB total\n", swap_used_gib, swap_total_gib);
  }
}

void tinykerninfo(void) {
  tinyinit();
  pretext(pretext_kernver); // kernel version
  fetchinfo(tiny.version);
}

void tinyfetch(void) {
  // all pretext functions do the same thing (defined at the top of this file)
  // when a char string as passed to it, it will print it then flush the stdout
  // buffer.
  tinyuser();
  rand_string(); // this function is only ran if rand_enable is 1, which is
                 // enabled in the cmdline args handling.
  tinyos();      // get the OS name
  tinydist();    // get the dist name
  tinykern();    // get kernel name
  tinyshell();   // get shell name
#ifdef __linux
  tinyuptime(); // get uptime if building on Linux
#endif
  tinywm();       // get Window Manager/DE name
  tinycpu();      // get CPU name
  tinyram();      // get ram values
  tinyswap();     // get swap values
  tinykerninfo(); // get kernel info
}
/*
        main function
*/

int main(int argc, char *argv[]) {

  /*
          command line args
  */

  // if no args are passed, run the default printing function

  if (argc == 1) {
    tinyfetch();
    return 0;
  }

  if (argc > 1) {
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
      fflush(
          stdout); // we must flush the buffer before printing to prevent issues
      printf("%s %s\n", decoration, argv[2]);
      tinyfetch();
      return 0;
    } else if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--random") ||
               (!strcmp(argv[1], "-r") && !strcmp(argv[2], "--color"))) {
      rand_enable = 1;
      tinyfetch();
    } else if (!strcmp(argv[1], "--color")) {
      rand_enable = 1;
      if (access("/usr/bin/lolcat", F_OK) == -1) {
        perror("access");
        printf("lolcat is not installed! cannot print using colors.\n");
        return 1;
      } else {
        char buffer[256];
        strncpy(buffer, argv[0], sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        strcat(buffer, " | lolcat");
        printf("%s", buffer);
        (void)system(buffer); // breaks shell detectio
      }
    } else if (!strcmp(argv[1], "-o")) {
      tinyos();
    } else if (!strcmp(argv[1], "-d")) {
      tinydist();
    } else if (!strcmp(argv[1], "-k")) {
      tinykern();
    } else if (!strcmp(argv[1], "-s")) {
      tinyshell();
    } else if (!strcmp(argv[1], "-u")) {
#ifdef __linux__
      tinyuptime();
#endif
#ifndef __linux__
      printf("Incompatible operating system!\n");
#endif
    } else if (!strcmp(argv[1], "-w")) {
      tinywm();
    } else if (!strcmp(argv[1], "--ram")) {
      tinyram();
    } else if (!strcmp(argv[1], "-c")) {
      tinycpu();
    } else if (!strcmp(argv[1], "--swap")) {
      tinyswap();
      int c = get_cpu_count();
      printf("(%d)", c);
    } else if (!strcmp(argv[1], "--kernel-info")) {
      tinykerninfo();
    } else if (!strcmp(argv[1], "--genie")) {
      rand_enable = 1;
      rand_string();
    } else if (!strcmp(argv[1], "--user")) {
      tinyuser();
    } else {
      printf("tinyfetch: Unknown command line argument.\n %s %s", decoration,
             help_banner);
      return 1;
    }
  }

  // exit
  return 0;
}
