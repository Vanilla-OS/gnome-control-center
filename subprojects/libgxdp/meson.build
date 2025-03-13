project(
  'libgxdp',
  'c',
  version: '48.alpha',
  meson_version: '>= 1.1',
  license: 'LGPLv2.1+'
)

gtk_dep = dependency('gtk4', version: '>= 4.17.1')

subdir('src')

if get_option('tests')
  subdir('tests')
endif
