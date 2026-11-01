project(
  'libgxdp',
  'c',
  version: '48.alpha',
  meson_version: '>= 1.1',
  license: 'LGPLv2.1+'
)

gtk_dep = dependency('gtk4', version: '>= 4.22.0')

subdir('src')

if get_option('tests')
  subdir('tests')
endif

summary('Tests', get_option('tests'))
