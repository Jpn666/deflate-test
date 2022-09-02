project('deflate', 'c', default_options: ['buildtype=release', 'warning_level=3'])


conf = configuration_data()

# CPU configuration
if target_machine.endian() == 'little'
  conf.set('CTB_CFG_LITTLEENDIAN', true)
else
  if target_machine.endianess() == 'big'
    conf.set('CTB_CFG_BIGENDIAN', true)
  else
    abort()
  endif
endif

target = target_machine.cpu_family()
conf.set('CTB_CFG_ENV64', target.contains('64'))
if target.contains('x86')
  conf.set('CTB_CFG_FASTUNALIGNED', true)
else
  conf.set('CTB_CFG_STRICTALIGNMENT', true)
endif

# System platform
if target_machine.system() == 'windows'
  conf.set('CTB_CFG_PLATFORM_WINDOWS', true)
else
  if target_machine.system() == 'haiku'
    conf.set('CTB_CFG_PLATFORM_BEOS', true)
  else
    conf.set('CTB_CFG_PLATFORM_UNIX', true)
  endif
endif


configure_file(output: 'ctbconfig.h', configuration: conf)


includes = include_directories(['deflate', 'deflate/ctoolbox'])
common = files([
  'deflate/src/inflator.c',
  'deflate/src/deflator.c',
  'deflate/src/zstrm.c',
  'deflate/ctoolbox/crypto/src/crc32.c',
  'deflate/ctoolbox/crypto/src/adler32.c'
])


externalasm = false
if target == 'x86_64'
  if target_machine.system() == 'windows'
    externalasm = true
  else
    if target_machine.system() != 'haiku'
      externalasm = true
    endif
  endif
endif

externalobjs = []
if externalasm
  add_project_arguments(  '-DCRC32_CFG_EXTERNALASM', language: 'c')
  add_project_arguments('-DADLER32_CFG_EXTERNALASM', language: 'c')
  base = 'deflate/ctoolbox/crypto/src/asm'
  if target_machine.system() == 'windows'
    platformpath = 'windows'
  else
    if target_machine.system() != 'haiku'
      platformpath = 'unix'
    endif
  endif
  externalobjs += [base / platformpath /   'crc32x64.o']
  externalobjs += [base / platformpath / 'adler32x64.o']
endif


largs = []
if externalasm
  cc = meson.get_compiler('c')
  if cc.get_linker_id() == 'ld.bfd'
    largs += ['-z', 'noexecstack']
  endif
endif

executable('inflator', sources: common + ['inflator.c'], include_directories: includes, objects: externalobjs, link_args: largs)
executable('deflator', sources: common + ['deflator.c'], include_directories: includes, objects: externalobjs, link_args: largs)
executable('zstrm',    sources: common + [   'zstrm.c'], include_directories: includes, objects: externalobjs, link_args: largs)
