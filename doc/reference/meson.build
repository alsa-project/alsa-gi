configure_file(
  input : 'version.xml.in',
  output : 'version.xml',
  configuration : conf
)

gnome.gtkdoc('alsa-gobject',
  src_dir: join_paths(meson.source_root(), 'src'),
  main_xml: 'alsa-gobject.xml',
  install: true
)
