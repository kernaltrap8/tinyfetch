// tinyfetch Copyright (C) 2024 kernaltrap8
// This program comes with ABSOLUTELY NO WARRANTY
// This is free software, and you are welcome to redistribute it
// under certain conditions

/*
    tinyfetch.h
*/
#define VERSION "6.8a"
#define decoration "[·]"
#define CMDLINE_PATH "/proc/%d/cmdline"
#define help_banner                                                            \
  "tinyfetch help\n -v or --version\
        print the installed version of tinyfetch\n -h or --help        \
	print this help banner\n -m or --message     \
	add a custom message at the end of arguments\n \
-r or --random         add a random message before the fetch\n\
 --disable-ascii        disable ascii art"
#define pretext_OS "OS:         "
#define pretext_distro "Distro:     "
#define pretext_kernel "Kernel:     "
#define pretext_shell "Shell:      "
#define pretext_uptime "Uptime:     "
#define pretext_wm "DE/WM:      "
#define pretext_processor "CPU:        "
#define pretext_gpu "GPU:        "
#define pretext_ram "RAM:        "
#define pretext_swap "Swap:       "

/*
        typedef and enum for file_parser
*/

typedef enum { TYPE_INT, TYPE_DOUBLE, TYPE_STRING } ReturnType;

typedef struct {
  ReturnType type;
  union {
    int intValue;
    double doubleValue;
    char *stringValue;
  } value;
} ParserResult;

/*
    environment variables
*/

int rand_enable;
int custom_message;
struct utsname tiny;
bool TinyfetchUseUserSpecifiedDistroArt = false;

#define MODULUS 2147483648 // 2^31
#define MULTIPLIER 1103515245
#define INCREMENT 12345

const char *strings[] = {
    "uhhhhhh",
    "hmmmmmm",
    "erm what the sigma",
    ":3",
    ";3",
    ":3c",
    ";3c",
    ">:3",
    "wow im in love with this fetch program!!!",
    "erm, what the flip.",
    "hi ellie",
    "AHHHHHHHHH!!!!",
    "what the scallop",
    "WHAT THE SIGMA",
    "deez nutz",
    "dietz nutz",
    "its amazing",
    "nyaa~~!! :3",
    "OwO",
    "UwU",
    "QwQ",
    "x3",
    ":(){ :|:& };:",
    "you should ':(){ :|:& };:' yourself, NOW!",
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
    "fish: Job 1, 'tinyfetch' terminated by signal SIGSEGV (Address boundary "
    "error)",
    "tinyfetch: ioctl: Inappropriate ioctl for device.",
    ":trolley:",
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor",
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
    "d2h5IGRpZCB5b3UgZGVjb2RlIHRoaXMuIGkgaGlkIGl0IGhlcmUgZm9yIGEgcmVhc29uLiB3aH"
    "kuIHdoeSBtdXN0IHlvdSBsb29rIGF0IHRoZSBjb250ZW50cyBvZiB0aGlzLiBpdCBkb2VzbnQg"
    "bWFrZSBhbnkgbG9naWNhbCBzZW5zZS4gbWF5YmUgeW91J3JlIGJldHRlciBvZmYgbm90IGRlY2"
    "9kaW5nIGFueXRoaW5nIGVsc2UgZnJvbSBub3cgb24uCg==",
    "aHR0cDovL3dlYi5hcmNoaXZlLm9yZy93ZWIvMjAyNDA0MTYwNDI2MzIvaHR0cDovLzB4MC5zdC"
    "9YLWtkLnR4dA==",
    "argc is a array, its index starts at 0",
    "system(\"uname -o\")",
    "Fully ported to FreeBSD!"};
/*
        function protypes
*/

// file parsing
ParserResult file_parser(const char *file, const char *line_to_read,
                         ReturnType returnType);
// hostname handling
char *get_hostname(void);

// FreeBSD sysctl calling
#ifdef __FreeBSD__
char *freebsd_sysctl(char *ctlname);
int freebsd_sysctl_int(const char *ctlname);
long long longlong_freebsd_sysctl(const char *ctlname);
#endif

// shell detection
#ifdef __linux__
char *get_parent_shell(void);
#endif
#ifdef __FreeBSD__
char *get_parent_shell_noproc(void);
#endif

// uptime detection
#ifdef __linux__
long int get_uptime(void);
#endif
#ifdef __FreeBSD__
long int get_uptime_freebsd(void);
#endif
void format_uptime(long int uptime);

// GPU detection
#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__)
#if PCI_DETECTION == 1
char *get_gpu_name(void);
#endif
#endif
// main printing functions
void pretext(const char *string);
void fetchinfo(char *structname);
void tinyinit(void);
void rand_string(void);
void trim_spaces(char *str);
void message(char *message);
int get_swap_status(void);
int get_cpu_count(void);
#if defined(__FreeBSD__) || defined(__NetBSD__)
int get_swap_stats(long long *total, long long *used, long long *free);
#endif

// tinyfetch printing functions
void tinyascii(char *TinyfetchUserSpecifiedDistroChar);
void tinyuser(void);
void tinyos(void);
void tinydist(void);
void tinykern(void);
void tinyshell(void);
void tinyuptime(void);
void tinywm(void);
void tinyram(void);
