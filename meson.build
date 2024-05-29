project('tinyfetch', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

link_args = []

uname_output = run_command('uname', check: true).stdout().strip()

if uname_output == 'Linux'
	link_args += ['-Os', '-s', '-fomit-frame-pointer', '-fno-unwind-tables', '-fno-asynchronous-unwind-tables', '-g0']
	message('Link args: -Os, -s, -fomit-frame-pointer, -fno-unwind-tables, -fno-asynchronous-unwind-tables, -g0')
elif uname_output == 'FreeBSD'
	link_args += ['-Os', '-lkvm']
endif

# Define the executable target
executable('tinyfetch', 'src/tinyfetch.c', install : true, c_args: link_args)
