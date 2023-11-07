project('jdeflate-test', 'c', default_options: ['buildtype=release', 'warning_level=3'])


deps = []
deps += [dependency('', fallback : ['ctoolbox', 'lib'])]
deps += [dependency('', fallback : ['jdeflate', 'lib'])]

cc = meson.get_compiler('c')
if cc.get_id() == 'msvc'
  add_project_arguments('-D__MSVC__', language: 'c')
endif

executable('inflator', sources: ['inflator.c'], dependencies: deps)
executable('deflator', sources: ['deflator.c'], dependencies: deps)
executable('zstrm',    sources: [   'zstrm.c'], dependencies: deps)
