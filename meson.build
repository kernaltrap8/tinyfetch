project('tinyfetch', 'c',
  version : '4.1a',
  default_options : [
    'warning_level=3',
    'c_std=c99',
    'optimization=s'
  ])

cc = meson.get_compiler('c')
config_h = configuration_data()

# Extra compiler flags for Linux only
if host_machine.system() == 'linux'
  add_project_arguments([
    '-fomit-frame-pointer',
    '-fno-unwind-tables',
    '-fno-asynchronous-unwind-tables',
    '-g0'
  ], language: 'c')
endif

# Dependencies
pci_dep = dependency('libpci', required: false)

if pci_dep.found()
  config_h.set('PCI_DETECTION', 1)
else
  config_h.set('PCI_DETECTION', 0)
endif

# Platform-specific extras
extra_libs = []
extra_includes = []

system = host_machine.system()
if system == 'linux'
  if pci_dep.found()
    extra_libs += [pci_dep]
  endif
elif system == 'freebsd'
  if pci_dep.found()
    extra_libs += [pci_dep]
  endif
  extra_libs += [cc.find_library('kvm')]
  extra_includes += include_directories('/usr/local/include')
elif system == 'netbsd'
  # NetBSD might not have pkg-config support, so fallback to find_library
  pciutils_lib = cc.find_library('pciutils', dirs : ['/usr/pkg/lib'], required : false)
  if pciutils_lib.found()
    extra_libs += [pciutils_lib]
    config_h.set('PCI_DETECTION', 1)
  endif
  extra_includes += include_directories('/usr/pkg/include')
endif

configure_file(output: 'config.h', configuration: config_h)

executable('tinyfetch', 'src/tinyfetch.c',
  install : true,
  include_directories : extra_includes,
  dependencies : [pci_dep],
  link_with : extra_libs
)
