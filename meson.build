project('barbarafs', 'c', default_options : ['c_std=c11'])

fuse = dependency('fuse')
glib = dependency('glib-2.0')

# Build the etpan dependency manually because they have their own system
etpan_prefix = run_command('libetpan-config', ['--prefix']).stdout().strip()
etpan_cflags = run_command('libetpan-config', ['--cflags']).stdout().strip().split()
etpan_libs = run_command('libetpan-config', ['--libs']).stdout().strip().split()
etpan_inc_dir = join_paths(etpan_prefix, 'include')
etpan_inc = include_directories(etpan_inc_dir)
if etpan_cflags.length() == 0
  etpan = declare_dependency(
    include_directories: etpan_inc,
    link_args: etpan_libs)
else
  etpan = declare_dependency(
    compile_args: etpan_cflags,
    include_directories: etpan_inc,
    link_args: etpan_libs)
endif

bfs_src = ['bfs.c', 'bfs_file.c', 'bfs_imap.c', 'bfs_smtp.c']

executable('barbarafs', 'main.c', bfs_src, dependencies : [fuse, glib, etpan])
