// tinyfetch Copyright (C) 2024 kernaltrap8
// This program comes with ABSOLUTELY NO WARRANTY
// This is free software, and you are welcome to redistribute it
// under certain conditions

#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <ctime>
#include <limits>
#include <fstream>
#include <unistd.h>
#include "tinyfetch.hpp"

extern "C" void pretext(const char* string) {
	printf("%s", string);
	fflush(stdout); // flush stdout to fix issues from system()
}
 
/*
	file parsing
*/

extern "C" char* read_hostname(const char* filename) {
	char* buffer = NULL;
	int string_size, read_size;
	FILE* handler = fopen(filename, "r"); // open file passed to this function

	if (handler) { // only run this statement if file exists
		fseek(handler, 0, SEEK_END); // seek EoF
		string_size = ftell(handler); // get string size
		rewind(handler);

		buffer = (char*) malloc(sizeof(char) * (string_size + 1)); // allocate memory for buffer for the exact size of the string we are printing
		read_size = fread(buffer, sizeof(char), string_size, handler); // get size of string to print

		buffer[string_size] = '\0';

		if (string_size != read_size) { // if we didnt get the size correctly, free the memory allocated and set the buffer to NULL
			free(buffer);
			buffer = NULL;
		}

		fclose(handler); // close the buffer
	}

	return buffer; // return contents of the buffer
}

extern "C" int file_parser(const char* file, const char* line_to_read) {
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

extern "C" char* file_parser_char(const char* file, const char* line_to_read) {
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

extern "C" char* get_hostname_bsd() {
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == 0) { // if the gethostname command works, return the value from it. otherise return a nullptr.
		return strdup(hostname);
	} else {
		return nullptr;
	}
}

/*
	shell detection
*/

extern "C" char* get_parent_shell() {
	pid_t ppid = getppid(); // get parent proc ID
	char cmdline_path[64];
	snprintf(cmdline_path, sizeof(cmdline_path), CMDLINE_PATH, ppid);

	FILE* cmdline_file = fopen(cmdline_path, "r"); // open /proc/%d/cmdline
	if (cmdline_file == NULL) {
		return nullptr; // return NULL if cmdline_file doesnt exist
	}

	char cmdline[256];
	if (fgets(cmdline, sizeof(cmdline), cmdline_file) == NULL) { // if something bad happened, free memory and return nullptr
		fclose(cmdline_file);
		return nullptr;
	}

	fclose(cmdline_file); // close the file

	if (cmdline[0] == '-') { // NOTE: this fixes a bug that occurs sometimes either when using tmux or Konsole.
		memmove(cmdline, cmdline + 1, strlen(cmdline));
	}

	char* newline_pos = strchr(cmdline, '\n'); // seek newline, if its there, remove it.
	if (newline_pos != NULL) {
		*newline_pos = '\0';
	}

	return strdup(cmdline); // return the contents of cmdline
}

/*
	main printing functions
*/

extern "C" void rand_string() {
	if (rand_enable == 1) {
		srand(time(NULL));
		int num_strings = sizeof(strings) / sizeof(strings[0]);
		int n = rand() % num_strings;
		fflush(stdout);
		printf("%s %s\n", decoration, strings[n]);
	} else {}
}

extern "C" void tinyfetch(void) {
	// all pretext functions do the same thing (defined at the top of this file)
	// when a char string as passed to it, it will print it then flush the stdout buffer.
	printf("%s@", user); // username@, username is taken from char* user
	char* hostname = read_hostname("/etc/hostname"); // read the hostname file
	int total_length = strlen(user) + strlen(hostname);
	
	if (!hostname) { // if the file doesnt exist, fallback to BSD-style hostname retrieval.
		printf("%s\n", get_hostname_bsd());
		for (int i = 0; i < total_length; i++) {
			printf("-");
		}
		printf("\n");
		free(hostname); // free the memory alloc to the buff
	} else {
		printf("%s", hostname);
		for (int i = 0; i < total_length; i++) {
			printf("-");
		}
		printf("\n");
		free(hostname); // free the memory alloc to the buff
	}

	rand_string(); // this function is only ran if rand_enable is 1, which is enabled in the cmdline args handling.
	pretext(pretext_OS);
	(void)system("uname -o"); // returns OS name
	pretext(pretext_distro);
	char* distro_name = file_parser_char("/etc/os-release", "PRETTY_NAME=\"%[^\"]\""); // parsing at isolating the PRETTY_NAME and VERSON_ID
	char* distro_ver = file_parser_char("/etc/os-release", "VERSION_ID=\"%[^\"]\"%*c");
	printf("%s %s\n", distro_name, distro_ver);
	free(distro_name);
	free(distro_ver);
	pretext(pretext_kernel);
	(void)system("uname -r"); // gets kernel name
	pretext(pretext_shell);

	if (shell == nullptr) { // if get_parent_shell() fails, fallback to getenv
		free(shell);
		shell = getenv("SHELL");
	}

	printf("%s\n", shell); // shell var taken from getenv()
	pretext(pretext_processor);
	(void)system("uname -p"); // gets processor name
	// process memory used and total avail.
	int total_ram = file_parser("/proc/meminfo", "MemTotal: %d kB");
	int ram_free = file_parser("/proc/meminfo", "MemFree: %d kB");

	if (total_ram != -1 && ram_free != -1) { // if we got the values correctly, print them
		pretext(pretext_ram);
		printf("%d KiB / %d KiB\n", ram_free, total_ram);
	} else {} // empty else statement, this will make nothing happen and not print ram avail/used.

	pretext(pretext_arch);
	(void)system("uname -m"); // CPU arch
	pretext(pretext_kernver);
	(void)system("uname -v"); // kernel version
}

/*
	main function
*/

extern "C" int main(int argc, char* argv[]) {	
	
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
		} else if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--random")) {
			// start of random number gen. we have two because if{} statememnts are isolated in C
			rand_enable = 1;
			tinyfetch();
		} else if (!strcmp(argv[1], "--color")) {
			rand_enable = 1;
			(void)system("tinyfetch -r | lolcat"); // breaks shell detection
		} else {
			printf("tinyfetch: Unknown command line argument.\n %s %s", decoration, help_banner);
			return 1;
		}
	}

	// exit
	return 0;
}
