// tinyfetch Copyright (C) 2024 kernaltrap8
// This program comes with ABSOLUTELY NO WARRANTY
// This is free software, and you are welcome to redistribute it
// under certain conditions

/*
    tinyfetch.h
*/

#define VERSION "5.4"
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
    environment variables
*/

int rand_enable;
int custom_message;
struct utsname tiny;

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
