project('tinyfetch', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

executable('tinyfetch', 'src/tinyfetch.c', install : true, c_args: ['-Os', '-s', '-fomit-frame-pointer', '-fno-unwind-tables', '-fno-asynchronous-unwind-tables', '-g0'])
