tests = [
  ['alsactl-client',    'ALSACtl'],
  ['alsactl-enums',     'ALSACtl'],
  ['alsaseq-client',    'ALSASeq'],
  ['alsaseq-port',      'ALSASeq'],
  ['alsaseq-enums',     'ALSASeq'],
  ['alsatimer-client',  'ALSATimer'],
  ['alsatimer-enums',   'ALSATimer'],
  ['alsarawmidi-enums', 'ALSARawmidi'],
]

objdir = join_paths(meson.build_root(), 'src')

envs = environment()
envs.append('LD_LIBRARY_PATH', objdir, separator : ':')
envs.append('GI_TYPELIB_PATH', objdir, separator : ':')

foreach test : tests
    name = test[0]
    script = test[0]
    namespace = test[1]
    prog = find_program(script)
    test(name, prog,
      env: envs,
    )
endforeach
