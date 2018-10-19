modules = {
  'ctl':      ['ALSACtl',     false,  false],
  'seq':      ['ALSASeq',     true,   false],
  'timer':    ['ALSATimer',   true,   false],
  'card':     ['ALSACard',    false,  false],
}

foreach path, params: modules
  namespace = params[0]
  has_marshaller = params[1]
  has_enumerator = params[2]

  name = 'alsa' + path

  # Fill these lists in each directory.
  sources = []
  headers = []
  subdir(path)

  # Depends on glib-2.0 and gobject-2.0
  gobject = dependency('gobject-2.0',
    version: '>=2.34.0'
  )

  dependencies = [
    gobject,
  ]

  # Generate marshallers for GObject signals.
  if has_marshaller
    signal_marshallers = gnome.genmarshal(name + '_sigs_marshal',
      prefix: name + '_sigs_marshal',
      sources: join_paths(path, name + '_sigs_marshal.list'),
      stdinc: true,
    )
    sources += signal_marshallers
  endif

  # Generate enumerators for GObject fashion.
  if has_enumerator
    enums = gnome.mkenums(name + '_enums',
      sources: join_paths(path, name + '_enum_types.h'),
      identifier_prefix: namespace,
      symbol_prefix: name,
      c_template: join_paths(path, name + '_enums.c.template'),
      h_template: join_paths(path, name + '_enums.h.template'),
      install_header: true,
    )
    headers += join_paths(path, name + '_enum_types.h')
    sources += enums
  endif

  # Constrain public APIs by linker option.
  mapfile = join_paths(path, name + '.map')
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
    include_directories: include_directories(path),
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
endforeach
