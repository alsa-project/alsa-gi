name = 'alsaseq'
namespace = 'ALSASeq'

# Depends on glib-2.0 and gobject-2.0
gobject = dependency('gobject-2.0',
  version: '>=2.34.0'
)

dependencies = [
  gobject,
]

sources = [
  'client.c',
  'port.c',
]

headers = [
  'client.h',
  'port.h',
]

# Generate marshallers for GObject signals.
signal_marshallers = gnome.genmarshal(name + '_sigs_marshal',
  prefix: name + '_sigs_marshal',
  sources: name + '_sigs_marshal.list',
  stdinc: true,
)
sources += signal_marshallers

mapfile = name + '.map'
vflag = '-Wl,--version-script,' + join_paths(meson.current_source_dir(), mapfile)

library = library(name,
  sources: sources + headers,
  version: meson.project_version(),
  soversion: meson.project_version().split('.')[0],
  install: true,
  install_dir: join_paths(get_option('libdir'), meson.project_name()),
  dependencies: dependencies,
  link_args : vflag,
  link_depends : mapfile,
)

install_headers(headers,
  subdir: join_paths(meson.project_name(), name),
)

pkg = import('pkgconfig')
pkg.generate(
  version: meson.project_version(),
  libraries: library,
  requires: [
    'glib-2.0',
    'gobject-2.0',
  ],
  name: name,
  description: '',
  filebase: name,
  subdirs: name,
)

if get_option('gir')
  gnome.generate_gir(library,
    sources: sources + headers,
    nsversion: '0.0',
    namespace: namespace,
    symbol_prefix: name + '_',
    identifier_prefix: namespace,
    export_packages: name,
    dependencies: dependencies,
    includes: [
      'GLib-2.0',
      'GObject-2.0',
    ],
    install: true,
  )
endif
