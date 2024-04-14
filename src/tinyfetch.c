#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define VERSION 	"0.8"
#define decoration  "[·]"
#define help_banner "%s help\n -v or --version     \
	print the installed version of tinyfetch\n -h or --help        \
	print this help banner\n -a or --all         \
	display all information\n -m or --message     \
	add a custom message at the end of arguments\n you can also combine args like \"%s -am \'message\' to get the full fetch, plus a custom message.\n"

#define pretext_OS 		  "OS:         "
#define pretext_kernel 	  "Kernel:     "
#define pretext_arch 	  "Arch:       "
#define pretext_kernver   "Kernel ver: "
#define pretext_processor "CPU:        "
#define pretext_shell	  "Shell:      "
#define pretext_user	  "User:       "

void pretext(char* string) {
	printf(string);
	fflush(stdout);
}

void kernel_print(void) {
	pretext(pretext_OS);
	system("uname -o");
	pretext(pretext_kernel);
	system("uname -r");
}

char* read_hostname(char* filename) {
	char* buffer = NULL;
	int string_size, read_size;
	FILE* handler = fopen(filename, "r");

	if (handler) {
		fseek(handler, 0, SEEK_END);
		string_size = ftell(handler);
		rewind(handler);

		buffer = (char*) malloc(sizeof(char) * (string_size + 1));
		read_size = fread(buffer, sizeof(char), string_size, handler);

		buffer[string_size] = '\0';

		if (string_size != read_size) {
			free(buffer);
			buffer = NULL;
		}

		fclose(handler);
	}

	return buffer;
}

void print_all(void) {
	char* shell = getenv("SHELL");
	char* user 	= getenv("USER");
	
	kernel_print();
	pretext(pretext_arch);
	system("uname -m");
	pretext(pretext_user);
	printf("%s@", user);
	char* hostname = read_hostname("/etc/hostname");
	if (hostname) {
		printf("%s", hostname);
		free(hostname);
	}
	pretext(pretext_shell);
	printf("%s\n", shell);
	pretext(pretext_processor);
	system("uname -p");
	pretext(pretext_kernver);
	system("uname -v");
}

int main(int argc, char* argv[]) {
	if (argc == 1) {
		kernel_print();
		return 0;
	}
	
	if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
		printf("%s v%s", argv[0], VERSION);
		return 0;
	} if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		printf(help_banner, argv[0], argv[0]);
		return 0;
	} if (!strcmp(argv[1], "-m") || !strcmp(argv[1], "--message")) {
		if (argc < 3) {
			printf("no message provided.\n");
			return 1;
		}
		fflush(stdout);
		printf("%s  %s\n", decoration, argv[2]);
		kernel_print();
		return 0;
	} if (!strcmp(argv[1], "-am")) {
		if (argc < 3) {
			printf("no message provided.\n");
			return 1;
		}
		fflush(stdout);
		printf("%s %s\n", decoration, argv[2]);
		print_all();
	} if (!strcmp(argv[1], "-a") || !strcmp(argv[1], "--all")) {
			print_all();
	}

	return 0;
}
