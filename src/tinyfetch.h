
/*
	strings for tinyfetch
*/

#define VERSION 	"2.7"
#define decoration  "[·]"
#define CMDLINE_PATH "/proc/%d/cmdline"
#define help_banner "tinyfetch help\n -v or --version     \
	print the installed version of tinyfetch\n -h or --help        \
	print this help banner\n -m or --message     \
	add a custom message at the end of arguments\n -r or --random         add a random message before the fetch\n --color \
	    	add some color to your fetch! requires lolcat to be installed.\n"
#define pretext_OS 		  "OS:         "
#define pretext_kernel 	  "Kernel:     "
#define pretext_arch 	  "Arch:       "
#define pretext_kernver   "Kernel ver: "
#define pretext_processor "CPU:        "
#define pretext_shell	  "Shell:      "
#define pretext_ram		  "RAM:        "
#define pretext_distro    "Distro:     "

/*
	function declaration
*/

int file_parser(const char* file, const char* line_to_read);
char* file_parser_char(const char* file, const char* line_to_read);
char* get_hostname(void);
char* get_parent_shell(void);
void rand_string(void);
void pretext(const char* string);
void fetchinfo(char* structname);
void tinyfetch(void);

/*
	environment variables
*/

int rand_enable;
struct utsname tiny;

/*
	const char* strings for random string printing if -r or -ar is passed to argv[]
*/

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
	"(null)",
	"echo ':(){ :|:& };:' > /etc/skel/.bashrc",
	"sustainable future ai circular economy ai ceo clyde ai linux",
	"gpu with 5gb of vram (required)",
	"Segmentation fault (core dumped)",
	"const char*",
	"public static void main(String args[])",
	"klsdjfsdffhasjklg",
	"Microsoft Windows 10.0.19043",
	"Welcome to fish, the friendly interactive shell",
	"exec dbus-launch --exit-with-session startplasma-wayland",
	"meson init -l c src/tinyfetch.c --builddir build/",
	"why did i put so many things in const char* strings[]",
	"255 lines of code!!!",
	"i can see you.",
	"fish: Job 1, 'tinyfetch' terminated by signal SIGSEGV (Address boundary error)",
	"tinyfetch: ioctl: Inappropriate ioctl for device.",
	":trolley:",
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor",
	"g++ tinyfetch.cpp -o tinyfetch",
	"clang tinyfetch.cpp -o tinyfetch",
	"me when rand() % 20 but i forget to seed it",
	"$XDG_RUNTIME_DIR",
	"$HOME",
	"$PATH",
	"xz (XZ UTILS) 5.6.0",
	"fuck you! *inserts newline character*\n",
	"cd: The directory '../usr/sbin' does not exist",
	"rm -rf /* --no-preserve-root",
	"INFO: autodetecting backend as ninja",
	"INFO: calculating backend command to run: /usr/bin/ninja",
	"[2/2] Linking target tinyfetch",
	"You're so fat you make the black holes jealous!",
	"d2h5IGRpZCB5b3UgZGVjb2RlIHRoaXMuIGkgaGlkIGl0IGhlcmUgZm9yIGEgcmVhc29uLiB3aHkuIHdoeSBtdXN0IHlvdSBsb29rIGF0IHRoZSBjb250ZW50cyBvZiB0aGlzLiBpdCBkb2VzbnQgbWFrZSBhbnkgbG9naWNhbCBzZW5zZS4gbWF5YmUgeW91J3JlIGJldHRlciBvZmYgbm90IGRlY29kaW5nIGFueXRoaW5nIGVsc2UgZnJvbSBub3cgb24uCg==",
	"aHR0cDovL3dlYi5hcmNoaXZlLm9yZy93ZWIvMjAyNDA0MTYwNDI2MzIvaHR0cDovLzB4MC5zdC9YLWtkLnR4dA==",
	"argc is a array, its index starts at 0"
};
