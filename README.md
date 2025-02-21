# tinyfetch
Fetch program written in C.
# Bulding
Tinyfetch uses the meson build system. You must have it installed to build `tinyfetch`.<br>
Run `meson setup build`, then `meson compile -C build`
# Optional dependencies
An optional dependency can be linked into tinyfetch which is used for GPU detection. On platforms without PCIe lanes, the preprocessor macro `PCI_DETECTION` in `tinyfetch.c` can be disabled to exclude this code.<br>
To disable linking in `meson.build`, remove the `-lpci` flag from `link_args`.
# Supported platforms
Linux<br>
FreeBSD
