project('alsa-gobject', 'c',
  version: '0.0.1',
  license: 'LGPL-2.1+',
  meson_version: '>= 0.32.0',
)

conf = configuration_data()
conf.set('VERSION', meson.project_version())

# For g-i and documentation.
gnome = import('gnome')

subdir('src/ctl')
subdir('src/seq')
subdir('src/timer')
subdir('src/card')
subdir('doc')
#subdir('tests')
