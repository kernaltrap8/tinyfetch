#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define VERSION "0.5"
#define help_banner "%s help\n -v or --version     print the installed version of tinyfetch\n -h or --help        print this help banner\n -a or --all         display all information\n -m or --message     add a custom message at the end of arguments\n"

void kernel_print(void) {
	system("uname -o");
	system("uname -r");
}

void print_all(void) {
	kernel_print();
	system("uname -m");
	system("uname -v");
	system("uname -p");
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
		printf(help_banner, argv[0]);
		return 0;
	} if (!strcmp(argv[1], "-m") || !strcmp(argv[1], "--message")) {
		if (argc < 3) {
			printf("no message provided.\n");
			return 1;
		}
		printf("%s", argv[2]);
		kernel_print();
		return 0;
	} if (!strcmp(argv[1], "-a") || !strcmp(argv[1], "--all")) {
			print_all();
	}

	return 0;
}
