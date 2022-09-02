project('deflate', 'c', default_options: ['buildtype=release', 'warning_level=3'])


deps = []
deps += [dependency('', fallback : ['ctoolbox', 'lib'])]
deps += [dependency('', fallback : [ 'deflate', 'lib'])]

executable('inflator', sources: ['inflator.c'], dependencies: deps)
executable('deflator', sources: ['deflator.c'], dependencies: deps)
executable('zstrm',    sources: [   'zstrm.c'], dependencies: deps)
