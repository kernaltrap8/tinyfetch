# tinyfetch
fetch program written in pure C
# bulding
this program uses the meson build system. you must have it installed to build `tinyfetch`.<br>
run `meson setup build`, then `meson compile build`
# optional dependencies
an optional dependency can be linked into tinyfetch which is used for GPU detection. on platforms without PCIe lanes, the preprocessor macro `PCI_DETECTION` in `tinyfetch.c` can be disabled to exclude this code.<br>
to disable linking in `meson.build`, remove the `-lpci` flag from `link_args`.
# supported platforms
Linux - since the start of the project<br>
FreeBSD - since 2024-27-05
