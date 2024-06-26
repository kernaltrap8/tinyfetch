project('tinyfetch', 'c',
  version : '4.1a',
  default_options : ['warning_level=3'])

c_args = []
link_args = []
config_h = configuration_data()

uname_output = run_command('uname', check: true).stdout().strip()

if uname_output == 'Linux'
  c_args += ['-Os', '-s', '-fomit-frame-pointer', '-fno-unwind-tables', '-fno-asynchronous-unwind-tables', '-g0']
  pci_dep = dependency('libpci', required: false)

  if pci_dep.found()
    link_args += ['-lpci']
    config_h.set('PCI_DETECTION', 1)
  endif

  configure_file(output: 'config.h', configuration: config_h)
  executable('tinyfetch', 'src/tinyfetch.c', install : true, c_args: c_args, link_args: link_args)
elif uname_output == 'FreeBSD'
  c_args += ['-Os']
  link_args += ['-lkvm']
  executable('tinyfetch', 'src/tinyfetch.c', install : true, c_args: c_args, link_args: link_args)
endif
