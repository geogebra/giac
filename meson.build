project('giac', 'c', 'cpp',
  version : '1.9.0',
  meson_version : '>= 1.2.0',
  default_options : ['cpp_std=c++11'],
)

# ---------- Options ----------

giac_version = get_option('giac_version')

# ---------- Compilers ----------

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

# ---------- Source files (51 .cc + 1 .c from src/giac/cpp/) ----------

giac_sources = files(
  'src/giac/cpp/alg_ext.cc',
  'src/giac/cpp/cocoa.cc',
  'src/giac/cpp/csturm.cc',
  'src/giac/cpp/derive.cc',
  'src/giac/cpp/desolve.cc',
  'src/giac/cpp/ezgcd.cc',
  'src/giac/cpp/freeglut_stroke_roman.c',
  'src/giac/cpp/gauss.cc',
  'src/giac/cpp/gausspol.cc',
  'src/giac/cpp/gen.cc',
  'src/giac/cpp/global.cc',
  'src/giac/cpp/help.cc',
  'src/giac/cpp/identificateur.cc',
  'src/giac/cpp/ifactor.cc',
  'src/giac/cpp/index.cc',
  'src/giac/cpp/input_lexer.cc',
  'src/giac/cpp/input_parser.cc',
  'src/giac/cpp/intg.cc',
  'src/giac/cpp/intgab.cc',
  'src/giac/cpp/isom.cc',
  'src/giac/cpp/lin.cc',
  'src/giac/cpp/maple.cc',
  'src/giac/cpp/mathml.cc',
  'src/giac/cpp/misc.cc',
  'src/giac/cpp/modfactor.cc',
  'src/giac/cpp/modpoly.cc',
  'src/giac/cpp/moyal.cc',
  'src/giac/cpp/opengl.cc',
  'src/giac/cpp/pari.cc',
  'src/giac/cpp/permu.cc',
  'src/giac/cpp/plot.cc',
  'src/giac/cpp/plot3d.cc',
  'src/giac/cpp/prog.cc',
  'src/giac/cpp/quater.cc',
  'src/giac/cpp/risch.cc',
  'src/giac/cpp/rpn.cc',
  'src/giac/cpp/series.cc',
  'src/giac/cpp/solve.cc',
  'src/giac/cpp/sparse.cc',
  'src/giac/cpp/subst.cc',
  'src/giac/cpp/sym2poly.cc',
  'src/giac/cpp/symbolic.cc',
  'src/giac/cpp/tex.cc',
  'src/giac/cpp/threaded.cc',
  'src/giac/cpp/ti89.cc',
  'src/giac/cpp/tinymt32.cc',
  'src/giac/cpp/TmpFGLM.cpp',
  'src/giac/cpp/TmpLESystemSolver.cpp',
  'src/giac/cpp/unary.cc',
  'src/giac/cpp/usual.cc',
  'src/giac/cpp/vecteur.cc',
)

# ---------- Common preprocessor defines (all platforms) ----------

common_defines = [
  '-DGIAC_GGB',
  '-DIN_GIAC',
  '-DGIAC_GENERIC_CONSTANTS',
  '-DHAVE_NO_HOME_DIRECTORY',
  '-DTIMEOUT',
  '-DHAVE_LIBMPFR',
  '-DVERSION="' + giac_version + '"',
]

# ---------- Platform-conditional defines ----------

platform_defines = []

if host_machine.system() == 'linux'
  platform_defines += [
    '-DHAVE_UNISTD_H',
    '-DHAVE_SYS_TIMES_H',
    '-DHAVE_SYS_TIME_H',
    '-DHAVE_SYSCONF',
    '-DHAVE_LIBPTHREAD',
    '-DSIZEOF_VOID_P=8',
  ]
elif host_machine.system() == 'darwin'
  # Check if this is iOS/Catalyst via subsystem (Meson >= 1.2.0)
  is_ios = false
  if host_machine.subsystem() == 'ios'
    is_ios = true
    platform_defines += [
      '-DAPPLE_SMART',
      '-DNO_GETTEXT',
      '-DNO_SCANDIR',
      '-DOSX_10_9_CXX',
      '-DHAVE_CONFIG_H',
      '-D_IOS_FIX_',
      '-DHAVE_SYS_TIMES_H',
      '-DHAVE_SYS_TIME_H',
    ]
  else
    # macOS desktop
    platform_defines += [
      '-DHAVE_UNISTD_H',
      '-DAPPLE_SMART',
      '-DNO_SCANDIR',
      '-DNO_GETTEXT',
      '-DHAVE_SYS_TIMES_H',
      '-DHAVE_SYS_TIME_H',
    ]
  endif
elif host_machine.system() == 'windows'
  platform_defines += [
    '-DGIAC_MPQS',
    '-D__MINGW_H',
    '-DMINGW32',
    '-DHAVE_NO_SYS_TIMES_H',
    '-DHAVE_NO_SYS_RESOURCE_WAIT_H',
    '-DHAVE_NO_PWD_H',
    '-DHAVE_NO_CWD',
    '-DNO_CLOCK',
    '-Dusleep=',
    '-DYY_NO_UNISTD_H',
  ]
elif host_machine.system() == 'android'
  platform_defines += [
    '-DHAVE_UNISTD_H',
    '-DNO_BSD',
    '-DHAVE_CONFIG_H',
  ]
elif host_machine.system() == 'emscripten'
  platform_defines += [
    '-DHAVE_UNISTD_H',
    '-DHAVE_CONFIG_H',
    '-DEMCC2',
    '-Dgammaf=tgammaf',
  ]
endif

add_project_arguments(common_defines + platform_defines, language : ['c', 'cpp'])

# ---------- Common compiler flags ----------

common_cpp_args = []
common_c_args = []

if cpp.get_id() != 'msvc'
  common_cpp_args += ['-fpermissive', '-fexceptions']
  common_c_args += ['-fexceptions']
endif

add_project_arguments(common_cpp_args, language : 'cpp')
add_project_arguments(common_c_args, language : 'c')

# ---------- Include directories ----------

giac_inc = include_directories('src/giac/headers')

jdk_inc_dirs = [include_directories('src/jni/jdkHeaders')]
if host_machine.system() == 'darwin'
  jdk_inc_dirs += include_directories('src/jni/jdkHeaders/darwin')
elif host_machine.system() == 'linux' or host_machine.system() == 'android'
  jdk_inc_dirs += include_directories('src/jni/jdkHeaders/linux')
elif host_machine.system() == 'windows'
  jdk_inc_dirs += include_directories('src/jni/jdkHeaders/win')
endif

# Android architecture-specific headers
android_inc = []
if host_machine.system() == 'android'
  if host_machine.cpu_family() == 'arm'
    android_inc = [include_directories('src/giac/headers/android/arm')]
  elif host_machine.cpu_family() == 'aarch64'
    android_inc = [include_directories('src/giac/headers/android/arm64')]
  elif host_machine.cpu_family() == 'x86'
    android_inc = [include_directories('src/giac/headers/android/x86')]
  elif host_machine.cpu_family() == 'x86_64'
    android_inc = [include_directories('src/giac/headers/android/x86_64')]
  endif
endif

simpleinterface_inc = include_directories('src/simpleInterface/headers')

# ---------- Dependencies: GMP and MPFR ----------
# MPFR must precede GMP in link order (MPFR FAQ).

prebuilt_base = meson.project_source_root() / 'src' / 'jni' / 'prebuilt'

if meson.is_cross_build()
  # Determine prebuilt directory from host_machine
  if host_machine.system() == 'android'
    if host_machine.cpu_family() == 'arm'
      prebuilt_dir = prebuilt_base / 'android' / 'arm-v7'
    elif host_machine.cpu_family() == 'aarch64'
      prebuilt_dir = prebuilt_base / 'android' / 'arm-v8'
    elif host_machine.cpu_family() == 'x86'
      prebuilt_dir = prebuilt_base / 'android' / 'x86'
    elif host_machine.cpu_family() == 'x86_64'
      prebuilt_dir = prebuilt_base / 'android' / 'x86-64'
    endif
  elif host_machine.system() == 'darwin'
    if host_machine.subsystem() == 'ios'
      if host_machine.cpu_family() == 'aarch64'
        prebuilt_dir = prebuilt_base / 'ios' / 'arm64'
      elif host_machine.cpu_family() == 'x86_64'
        prebuilt_dir = prebuilt_base / 'iphonesimulator' / 'x86_64'
      endif
    else
      if host_machine.cpu_family() == 'aarch64'
        prebuilt_dir = prebuilt_base / 'maccatalyst' / 'arm64'
      elif host_machine.cpu_family() == 'x86_64'
        prebuilt_dir = prebuilt_base / 'maccatalyst' / 'x86_64'
      endif
    endif
  elif host_machine.system() == 'linux'
    if host_machine.cpu_family() == 'arm'
      prebuilt_dir = prebuilt_base / 'linux' / 'arm-v7'
    elif host_machine.cpu_family() == 'x86_64'
      prebuilt_dir = prebuilt_base / 'linux' / 'x86-64'
    endif
  elif host_machine.system() == 'windows'
    prebuilt_dir = prebuilt_base / 'windows' / 'clang64'
  elif host_machine.system() == 'emscripten'
    prebuilt_dir = meson.project_source_root() / 'src' / 'giac.js' / 'prebuilt'
  endif

  mpfr_dep = declare_dependency(
    dependencies : cc.find_library('mpfr', dirs : prebuilt_dir, static : true),
  )
  gmp_dep = declare_dependency(
    dependencies : cc.find_library('gmp', dirs : prebuilt_dir, static : true),
  )
else
  # Native desktop build: use system libraries
  gmp_dep = dependency('gmp')
  # MPFR's pkg-config emits a bare -lgmp without GMP's -L path.
  # On systems where GMP and MPFR are in separate directories (e.g. Homebrew),
  # this causes a linker error. We work around this by using MPFR's
  # pkg-config in pure mode (no unmanaged dependencies) and depending on
  # our GMP dependency explicitly.
  _mpfr_pc = dependency('mpfr', method : 'pkg-config')
  gmp_libdir = gmp_dep.get_variable(pkgconfig : 'libdir', default_value : '')
  if gmp_libdir != ''
    mpfr_dep = declare_dependency(
      dependencies : [_mpfr_pc],
      link_args : ['-L' + gmp_libdir],
    )
  else
    mpfr_dep = _mpfr_pc
  endif
endif

# ---------- Target: libgiac (static library, all platforms) ----------

libgiac = static_library('giac',
  giac_sources,
  include_directories : [giac_inc] + android_inc,
  dependencies : [mpfr_dep, gmp_dep],
)

giac_dep = declare_dependency(
  link_with : libgiac,
  include_directories : [giac_inc],
  dependencies : [mpfr_dep, gmp_dep],
)

# ---------- Target: libjavagiac (shared library, conditional on JNI) ----------

jni_dep = dependency('jni', required : get_option('with_jni'))

if jni_dep.found() and host_machine.system() != 'emscripten'
  javagiac_link_args = []
  javagiac_deps = [giac_dep, jni_dep]

  if host_machine.system() == 'darwin' and not is_ios
    javagiac_link_args += [
      '-framework', 'Accelerate',
      '-framework', 'CoreFoundation',
      '-lc++',
      '-lpthread',
    ]
  elif host_machine.system() == 'linux'
    javagiac_link_args += ['-static-libgcc', '-static-libstdc++', '-s']
  elif host_machine.system() == 'windows'
    javagiac_link_args += [
      '-Wl,--add-stdcall-alias',
      '-static-libgcc',
      '-static-libstdc++',
      '-static',
    ]
  elif host_machine.system() == 'android'
    javagiac_link_args += ['-s']
  endif

  javagiac_kwargs = {
    'sources' : files('src/jni/cpp/giac_wrap.cxx'),
    'include_directories' : [giac_inc] + jdk_inc_dirs + android_inc,
    'dependencies' : javagiac_deps,
    'link_args' : javagiac_link_args,
  }

  if host_machine.system() == 'windows'
    javagiac_kwargs += {
      'vs_module_defs' : 'src/jni/giac.def',
    }
  endif

  libjavagiac = shared_library('javagiac', kwargs : javagiac_kwargs)
endif

# ---------- Target: minigiac (executable, native desktop only) ----------

if get_option('with_minigiac') and not meson.is_cross_build()
  minigiac_link_args = []
  if host_machine.system() == 'linux'
    minigiac_link_args += ['-static-libgcc', '-static-libstdc++']
  endif

  minigiac = executable('minigiac',
    'src/minigiac/cpp/minigiac.cc',
    include_directories : [giac_inc],
    dependencies : [giac_dep],
    link_args : minigiac_link_args,
  )
endif

# ---------- Target: libsimpleinterface (iOS/Catalyst only) ----------

if get_option('with_simpleinterface')
  libsimpleinterface = static_library('simpleinterface',
    files(
      'src/simpleInterface/cpp/ContextBridge.cpp',
      'src/simpleInterface/cpp/GenBridge.cpp',
    ),
    include_directories : [giac_inc, simpleinterface_inc],
    dependencies : [giac_dep],
  )
endif

# ---------- Target: WASM (Emscripten only) ----------

if host_machine.system() == 'emscripten'
  wasm_link_args = [
    '-s', 'EXPORTED_FUNCTIONS=["_caseval"]',
    '-s', 'EXPORTED_RUNTIME_METHODS=["cwrap"]',
    '-s', 'TOTAL_MEMORY=67108864',
    '-s', 'WASM=1',
    '-s', 'ALLOW_MEMORY_GROWTH=1',
    '-s', 'NO_EXIT_RUNTIME=1',
    '-s', 'PRECISE_I64_MATH=1',
    '-fwasm-exceptions',
    '--js-library', meson.project_source_root() / 'src' / 'giac.js' / 'js' / 'time.js',
  ]

  giac_wasm = executable('giacggb',
    giac_sources,
    include_directories : [giac_inc],
    dependencies : [mpfr_dep, gmp_dep],
    link_args : wasm_link_args,
  )

  if get_option('with_wasm_embed')
    embed_script = find_program('scripts/embed-wasm.sh')
    custom_target('giac_embedded_js',
      output : 'giac.wasm.js',
      depends : giac_wasm,
      command : [embed_script, giac_wasm.full_path(), '@OUTPUT@'],
      build_by_default : true,
    )
  endif
endif
