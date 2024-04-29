// tinyfetch Copyright (C) 2024 kernaltrap8
// This program comes with ABSOLUTELY NO WARRANTY
// This is free software, and you are welcome to redistribute it
// under certain conditions

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/utsname.h>
#ifdef __linux__
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#endif
#include "tinyfetch.h"

/*
	file parsing
*/

int file_parser(const char* file, const char* line_to_read) {
	FILE* meminfo = fopen(file, "r"); // open the file to parse
	if (meminfo == NULL) {
		return -1; // return an error code if the file doesnt exist
	}

	char line[256]; // char buffer
	while (fgets(line, sizeof(line), meminfo)) {
		int ram; // int data type to store the memory info in to print
		if (sscanf(line, line_to_read, &ram) == 1) {
			fclose(meminfo); // close the file
			return ram; // return the contents
		}
	}

	fclose(meminfo); // close the file
	return -1; // negative exit code. if we get here, an error occurred.
}

char* file_parser_char(const char* file, const char* line_to_read) {
	FILE* meminfo = fopen(file, "r"); // open the file to parse
	if (meminfo == NULL) {
		return NULL; // return an error code if the file doesnt exist
	}

	char line[256]; // char buffer
	while (fgets(line, sizeof(line), meminfo)) {
		char* parsed_string = (char*)malloc(strlen(line) + 1); // allocate memory for the string
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
	return NULL; // null exit code. if we get here, an error occurred.
}

/*
	hostname handling
*/

char* get_hostname(void) {
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == 0) { // if the gethostname command works, return the value from it. otherise return a nullptr.
		return strdup(hostname);
	} else {
		return NULL;
	}
}

/*
	shell detection
*/

char* get_parent_shell(void) {
	pid_t ppid = getppid(); // get parent proc ID
	char cmdline_path[64];
	snprintf(cmdline_path, sizeof(cmdline_path), CMDLINE_PATH, ppid);

	FILE* cmdline_file = fopen(cmdline_path, "r"); // open /proc/%d/cmdline
	if (cmdline_file == NULL) {
		return NULL; // return NULL if cmdline_file doesnt exist
	}

	char cmdline[256];
	if (fgets(cmdline, sizeof(cmdline), cmdline_file) == NULL) { // if something bad happened, free memory and return nullptr
		fclose(cmdline_file);
		return NULL;
	}

	fclose(cmdline_file); // close the file
	
	if (cmdline[0] == '-') { // NOTE: this fixes a bug that occurs sometimes either when using tmux or Konsole.
		memmove(cmdline, cmdline + 1, strlen(cmdline));
	}
	
	char* newline_pos = strchr(cmdline, '\n'); // seek newline, if its there, remove it.
	if (newline_pos != NULL) {
		*newline_pos = '\0';
	}

	if (!strncmp(cmdline, "/bin/", 5)) {
		return strdup(cmdline + 5); // return string with /bin/ removed, if it exists in the first place.
	}


	return strdup(cmdline); // return the contents of cmdline
}

/*
	get uptime
*/
#ifdef __linux__
long int get_uptime() {
    struct sysinfo s_info;
    int e = sysinfo(&s_info);
    if (e != 0) {
		return -1;
    }
    
    return s_info.uptime;
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

void rand_string(void) {
	if (rand_enable == 1) {
		srand(time(NULL));
		int num_strings = sizeof(strings) / sizeof(strings[0]);
		int n = rand() % num_strings;
		fflush(stdout);
		printf("%s %s\n", decoration, strings[n]);
	} else {}
}

void pretext(const char* string) {
	printf("%s", string);
	fflush(stdout); // flush stdout, just in case.
}

void fetchinfo(char* structname) {
	printf("%s\n", structname);
}

void tinyfetch(void) {
	// all pretext functions do the same thing (defined at the top of this file)
	// when a char string as passed to it, it will print it then flush the stdout buffer.
	if (uname(&tiny) == -1) {
		perror("uname");
	}
		
	char* user 	= getenv("USER");
	char* shell = get_parent_shell();
	char* wm    = getenv("XDG_CURRENT_DESKTOP");
	printf("%s@", user); // username@, username is taken from char* user
	
	int total_length = strlen(user) + strlen(tiny.nodename) + 1; // calculate the chars needed for the decoration under user@host
	// taken from utsname struct
	printf("%s\n", tiny.nodename);
	for (int i = 0; i < total_length; i++) {
		printf("-");
	}
	printf("\n");
	
	rand_string(); // this function is only ran if rand_enable is 1, which is enabled in the cmdline args handling.
	pretext(pretext_OS);
	// tiny.sysname doesnt return what uname -o would, so here we check if it == Linux, and if it does, print GNU/ before tiny.sysname
	if (!strcmp(tiny.sysname, "Linux")) {
		printf("GNU/");
	}
	
	fetchinfo(tiny.sysname);
	pretext(pretext_distro);
	char* distro_name = file_parser_char("/etc/os-release", "PRETTY_NAME=\"%[^\"]\""); // parsing at isolating the PRETTY_NAME and VERSON_ID
	char* distro_ver = file_parser_char("/etc/os-release", "VERSION_ID=\"%[^\"]\"%*c");
	printf("%s %s\n", distro_name, distro_ver);
	free(distro_name);
	free(distro_ver);
	pretext(pretext_kernel);
	fetchinfo(tiny.release); // gets kernel name
	pretext(pretext_arch);
	printf(tiny.machine); // CPU arch
	printf("\n");
	pretext(pretext_shell);

	if (shell == NULL) { // if get_parent_shell() fails, fallback to getenv
		free(shell);
		shell = getenv("SHELL");
	}

	printf("%s\n", shell); // shell var taken from getenv()
	free(shell);
	#ifdef __linux__ // only include this code if __linux__ is defined
	long int uptime = get_uptime();
	if (uptime == -1) {
		;
	} else {
		pretext(pretext_uptime);
		format_uptime(uptime);
	}
	#endif

	if (wm == NULL) {
		;
	} else {
		pretext(pretext_wm);
		printf("%s\n", wm); // wm variable taken from getenv()
	}

	pretext(pretext_processor);
	char* cpu = file_parser_char("/proc/cpuinfo", "model name      : %[^\n]");
	printf("%s\n", cpu);

	// process memory used and total avail.
	int total_ram = file_parser("/proc/meminfo", "MemTotal: %d kB");
	int ram_free = file_parser("/proc/meminfo", "MemFree: %d kB");

	if (total_ram != -1 && ram_free != -1) { // if we got the values correctly, print them
		int ram_used = total_ram - ram_free;
		// convert the values from /proc/meminfo into GiB double values
		double total_ram_gib = total_ram / (1024.0 * 1024.0);
   	 	double ram_used_gib = ram_used / (1024.0 * 1024.0);
    	double ram_free_gib = ram_free / (1024.0 * 1024.0);
		pretext(pretext_ram);
		printf("%.2f GiB / %.2f GiB (%.2f GiB free)\n", ram_used_gib, total_ram_gib, ram_free_gib);
	} else {} // empty else statement, this will make nothing happen and not print ram avail/used.

	pretext(pretext_kernver); // kernel version
	fetchinfo(tiny.version);
}

/*
	main function
*/

int main(int argc, char* argv[]) {	
	
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
			fflush(stdout); // we must flush the buffer before printing to prevent issues
			printf("%s  %s\n", decoration, argv[2]);
			tinyfetch();
			return 0;
		} else if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--random") || (!strcmp(argv[1], "-r") && !strcmp(argv[2], "--color"))) {
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
		} else {
			printf("tinyfetch: Unknown command line argument.\n %s %s", decoration, help_banner);
			return 1;
		}
	}

	// exit
	return 0;
}
