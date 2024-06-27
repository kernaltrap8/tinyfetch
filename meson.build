project('tinyfetch', 'c',
  version : '4.1a',
  default_options : ['warning_level=3'])

c_args = []
link_args = []


uname_output = run_command('uname', check: true).stdout().strip()
if uname_output == 'Linux'
  config_h = configuration_data()
  pci_dep = dependency('libpci', required: false)
  if pci_dep.found()
    link_args += ['-lpci']
    config_h.set('PCI_DETECTION', 1)
  else
   config_h.set('PCI_DETECTION', 0)
  endif
  configure_file(output: 'config.h', configuration: config_h)
  c_args += ['-Os', '-s', '-fomit-frame-pointer', '-fno-unwind-tables', '-fno-asynchronous-unwind-tables', '-g0']
  executable('tinyfetch', 'src/tinyfetch.c', install : true, c_args: c_args, link_args: link_args)
elif uname_output == 'FreeBSD'
  config_h = configuration_data()
  pci_dep = dependency('libpci', required: false)
  if pci_dep.found()
    link_args += ['-L/usr/local/lib, '-lpci']
    config_h.set('PCI_DETECTION', 1)
  else
   config_h.set('PCI_DETECTION', 0)
  endif
  configure_file(output: 'config.h', configuration: config_h)
  c_args += ['-Os']
  link_args += ['-lkvm']
  inc_dirs = include_directories('/usr/local/include')
  executable('tinyfetch', 'src/tinyfetch.c', install : true, c_args: c_args, link_args: link_args, include_directories: inc_dirs)
endif
