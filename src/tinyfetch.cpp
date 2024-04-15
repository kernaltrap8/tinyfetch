#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <ctime>
#include <limits>
#include <fstream>
#include "tinyfetch.h"

extern "C" void pretext(const char* string) {
	printf("%s", string);
	fflush(stdout);
}

extern "C" void kernel_print(void) {
	pretext(pretext_OS);
	system("uname -o");
	pretext(pretext_kernel);
	system("uname -r");
}

extern "C" char* read_hostname(char* filename) {
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

unsigned long get_mem_total() {
	std::string token;
	std::ifstream file("/proc/meminfo");

	while(file >> token) {
		if (token == "Memtotal:") {
			unsigned long mem;
			if (file >> mem) {
				return mem;
			} else {
				return 0;
			}
		}
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	return 0;
}

extern "C" void print_all(void) {
	char* shell = getenv("SHELL");
	char* user 	= getenv("USER");
	
	kernel_print();
	pretext(pretext_arch);
	system("uname -m");
	pretext(pretext_user);
	printf("%s@", user);
	const char* hostname = read_hostname("/etc/hostname");
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

extern "C" int main(int argc, char* argv[]) {
	if (argc == 1) {
		kernel_print();
		return 0;
	}

	const char* strings[] = {
		"uhhhhhh",
		"hmmmmmm",
		"erm what the sigma",
		"I LOVE BALLS",
		":3",
		";3",
		":3c",
		";3c",
		">:3",
		"wow im in love with this fetch program!!!",
		"erm, what the flip.",
		"hi ellie",
		"hiaaaaaa lavvy ;333",
		"hello atl discord server",
		"AHHHHHHHHH!!!!",
		"what the scallop",
		"WHAT THE SIGMA",
		"deez nutz",
		"dietz nutz",
		"guys guys did you know",
		"that this fetch program",
		"is written in pure C",
		"and is faster than most fetch programs",
		"its amazing",
		"nyaa~~!! :3",
		"OwO",
		"UwU",
		"QwQ",
		"x3",
		":(){ :|:& };:",
		"you should ':(){ :|:& };:' yourself, NOW!",
		"BALLS AND NUTS AND BOYS AND AND ANDNAND",
		" ",
		"echo ':(){ :|:& };:' > /etc/skel/.bashrc"
	};

	if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
		printf("%s v%s\n", argv[0], VERSION);
		return 0;
	} if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		printf("%s %s", decoration, help_banner);
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
	} if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--random")) {
		srand(time(NULL));
		int num_strings = sizeof(strings) / sizeof(strings[0]);
		int n = rand() % num_strings;
		fflush(stdout);
		printf("%s %s\n", decoration, strings[n]);
		kernel_print();
	} if (!strcmp(argv[1], "-ar")) {
		srand(time(NULL));
		int num_strings = sizeof(strings) / sizeof(strings[0]);
		int n = rand() % num_strings;
		fflush(stdout);
		printf("%s %s\n", decoration, strings[n]);
		print_all();
	}

	return 0;
}
