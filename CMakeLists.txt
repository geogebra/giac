# This file has been tested on Linux, Mac and Windows.
#
# Note that the source tree comes with prebuilt versions
# of GMP and MPFR for a large variety of platforms
# and they are used by default. This helps in linking
# Giac as much statically as possible and improves
# portability. If you don't want this, change
# ${GMP_STATIC} to gmp and ${MPFR_STATIC} to mpfr.
#
# If you use the default setting (the static option),
# you don't need the prerequisities gmp and mpfr below.
#
# This build machinery does not use config.h. Instead,
# all definitions are given explicitly.
#
# Building on Linux and Mac
# -------------------------
#
# Install cmake, make, gmp and mpfr: On Linux, use the standard
# packaging tool -- on Mac use Homebrew. Then issue
# "mkdir build; cd build; cmake ..; make".
#
# Building on Windows
# -------------------
#
# Use MSYS2 with CLANG64 flavor. Necessary packages are:
#
# * base-devel
# * mingw-w64-clang-x86_64-cc
# * mingw-w64-clang-x86_64-gmp
# * mingw-w64-clang-x86_64-mpfr
# * mingw-w64-clang-x86_64-cmake
# * mingw-w64-clang-x86_64-make
#
# Install them via "pacman -S ...". Then issue
# "mkdir build; cd build; CC=clang CXX=clang++ cmake -G "MinGW Makefiles" ..; mingw32-make".
#
# For a 32-bit build the CLANG32 flavor must be used, and instead of "x86_64" "i686" must be set.

cmake_minimum_required(VERSION 3.10)
project(minigiac)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Detection of gmp/mpfr could be done via find_package, but this
# seems unsupported.

# We link GMP and MPFR statically to the JNI/DLL. This may break MPFR
# in some cases, see https://www.mpfr.org/faq.html#undef_ref1 if
# you encounter strange bugs.

function(find_static_library LIB_NAME OUT VENDOR_PATH)

    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

    find_library(
        FOUND_${LIB_NAME}_STATIC
        ${LIB_NAME}
        NO_DEFAULT_PATH
        PATHS ${VENDOR_PATH}
        )

    if (FOUND_${LIB_NAME}_STATIC)
        get_filename_component(ABS_FILE ${FOUND_${LIB_NAME}_STATIC} ABSOLUTE)
    else()
        message(SEND_ERROR "Unable to find library ${LIB_NAME} in ${VENDOR_PATH}")
    endif()

    set(${OUT} ${ABS_FILE} PARENT_SCOPE)

endfunction()

# Use prebuilt versions of GMP and MPFR to support static linking.

set(PREBUILT_DIR ${CMAKE_SOURCE_DIR}/src/jni/prebuilt)

if(LINUX)
    message(STATUS "Using prebuilt gmp/mpfr (Linux)")
    find_static_library(gmp GMP_STATIC ${PREBUILT_DIR}/linux/x86-64/)
    find_static_library(mpfr MPFR_STATIC ${PREBUILT_DIR}/linux/x86-64/)
endif()

if(APPLE)
    message(STATUS "Using prebuilt gmp/mpfr (Mac)")
    find_static_library(gmp GMP_STATIC ${PREBUILT_DIR}/osx/x86-64/)
    find_static_library(mpfr MPFR_STATIC ${PREBUILT_DIR}/osx/x86-64/)
endif()

if(WIN32)
    if("$ENV{MSYSTEM}" STREQUAL "CLANG64")
        message(STATUS "Using prebuilt gmp/mpfr for clang64")
        find_static_library(gmp GMP_STATIC ${PREBUILT_DIR}/windows/clang64/)
        find_static_library(mpfr MPFR_STATIC ${PREBUILT_DIR}/windows/clang64/)
    endif()
    if("$ENV{MSYSTEM}" STREQUAL "CLANG32")
        message(STATUS "Using prebuilt gmp/mpfr for clang32")
        find_static_library(gmp GMP_STATIC ${PREBUILT_DIR}/windows/clang32/)
        find_static_library(mpfr MPFR_STATIC ${PREBUILT_DIR}/windows/clang32/)
    endif()
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-fpermissive -std=c++0x")

set(GIAC_SOURCES
        src/giac/cpp/alg_ext.cc
        src/giac/cpp/cocoa.cc
        src/giac/cpp/csturm.cc
        src/giac/cpp/derive.cc
        src/giac/cpp/desolve.cc
        src/giac/cpp/ezgcd.cc
        src/giac/cpp/freeglut_stroke_roman.c
        src/giac/cpp/gauss.cc
        src/giac/cpp/gausspol.cc
        src/giac/cpp/gen.cc
        src/giac/cpp/global.cc
        src/giac/cpp/help.cc
        src/giac/cpp/identificateur.cc
        src/giac/cpp/ifactor.cc
        src/giac/cpp/index.cc
        src/giac/cpp/input_lexer.cc
        src/giac/cpp/input_parser.cc
        src/giac/cpp/intg.cc
        src/giac/cpp/intgab.cc
        src/giac/cpp/isom.cc
        src/giac/cpp/lin.cc
        src/giac/cpp/maple.cc
        src/giac/cpp/mathml.cc
        src/giac/cpp/misc.cc
        src/giac/cpp/modfactor.cc
        src/giac/cpp/modpoly.cc
        src/giac/cpp/moyal.cc
        src/giac/cpp/opengl.cc
        src/giac/cpp/pari.cc
        src/giac/cpp/permu.cc
        src/giac/cpp/plot.cc
        src/giac/cpp/plot3d.cc
        src/giac/cpp/prog.cc
        src/giac/cpp/quater.cc
        src/giac/cpp/risch.cc
        src/giac/cpp/rpn.cc
        src/giac/cpp/series.cc
        src/giac/cpp/solve.cc
        src/giac/cpp/sparse.cc
        src/giac/cpp/subst.cc
        src/giac/cpp/sym2poly.cc
        src/giac/cpp/symbolic.cc
        src/giac/cpp/tex.cc
        src/giac/cpp/threaded.cc
        src/giac/cpp/ti89.cc
        src/giac/cpp/tinymt32.cc
        src/giac/cpp/TmpFGLM.cpp
        src/giac/cpp/TmpLESystemSolver.cpp
        src/giac/cpp/unary.cc
        src/giac/cpp/usual.cc
        src/giac/cpp/vecteur.cc
        src/giac/headers/android/gmp.h
        src/giac/headers/android/mpfr.h
        src/giac/headers/alg_ext.h
        src/giac/headers/cocoa.h
        # src/giac/headers/config.h
        src/giac/headers/csturm.h
        src/giac/headers/derive.h
        src/giac/headers/desolve.h
        src/giac/headers/dispatch.h
        src/giac/headers/ezgcd.h
        src/giac/headers/fac_table.h
        src/giac/headers/fib_table.h
        src/giac/headers/first.h
        src/giac/headers/fits_s.h
        src/giac/headers/fits_u.h
        src/giac/headers/fraction.h
        src/giac/headers/gauss.h
        src/giac/headers/gausspol.h
        src/giac/headers/gen.h
        src/giac/headers/gen_inverse.h
        src/giac/headers/giac.h
        src/giac/headers/giacintl.h
        src/giac/headers/giacPCH.h
        src/giac/headers/global.h
        src/giac/headers/gmp-mparam.h
        src/giac/headers/gmp.h
        src/giac/headers/gmp_replacements.h
        src/giac/headers/gmpxx.h
        src/giac/headers/help.h
        src/giac/headers/identificateur.h
        src/giac/headers/ieee_floats.h
        src/giac/headers/ifactor.h
        src/giac/headers/index.h
        src/giac/headers/input_lexer.h
        src/giac/headers/input_parser.h
        src/giac/headers/intg.h
        src/giac/headers/intgab.h
        src/giac/headers/isom.h
        src/giac/headers/lexer.h
        src/giac/headers/lexer_tab_int.h
        src/giac/headers/lin.h
        src/giac/headers/longlong.h
        src/giac/headers/maple.h
        src/giac/headers/mathml.h
        src/giac/headers/misc.h
        src/giac/headers/modfactor.h
        src/giac/headers/modpoly.h
        src/giac/headers/monomial.h
        src/giac/headers/moyal.h
        src/giac/headers/mp_bases.h
        src/giac/headers/mparam.h
        src/giac/headers/mpf2mpfr.h
        src/giac/headers/mpfr-gmp.h
        src/giac/headers/mpfr-impl.h
        src/giac/headers/mpfr-intmax.h
        src/giac/headers/mpfr-longlong.h
        src/giac/headers/mpfr-thread.h
        src/giac/headers/mpfr.h
        src/giac/headers/opengl.h
        src/giac/headers/pari.h
        src/giac/headers/path.h
        src/giac/headers/permu.h
        src/giac/headers/plot.h
        src/giac/headers/plot3d.h
        src/giac/headers/poly.h
        src/giac/headers/prog.h
        src/giac/headers/quater.h
        src/giac/headers/risch.h
        src/giac/headers/rpn.h
        src/giac/headers/series.h
        src/giac/headers/solve.h
        src/giac/headers/sparse.h
        src/giac/headers/static.h
        src/giac/headers/static_extern.h
        src/giac/headers/static_help.h
        src/giac/headers/static_lexer.h
        src/giac/headers/static_lexer_.h
        src/giac/headers/subst.h
        src/giac/headers/sym2poly.h
        src/giac/headers/symbolic.h
        src/giac/headers/tex.h
        src/giac/headers/threaded.h
        src/giac/headers/ti89.h
        src/giac/headers/tinymt32.h
        src/giac/headers/tinymt32_license.h
        src/giac/headers/TmpFGLM.H
        src/giac/headers/TmpLESystemSolver.H
        src/giac/headers/trialdivtab.h
        src/giac/headers/unary.h
        src/giac/headers/usual.h
        src/giac/headers/vecteur.h
        src/giac/headers/vector.h
)

include_directories(src/giac/headers)

add_definitions(-DHAVE_NO_HOME_DIRECTORY -DGIAC_GGB -DIN_GIAC -DHAVE_LIB_PTHREAD
        -DGIAC_GENERIC_CONSTANTS -DTIMEOUT -DSIZEOF_VOID_P=8
        -DHAVE_LIBMPFR -DVERSION="1.2.3") # Version number seems hardwired, FIXME.

add_executable(minigiac
        ${GIAC_SOURCES}
        src/minigiac/cpp/minigiac.cc)

# Note the order: MPFR must precede GMP!
target_link_libraries(minigiac ${MPFR_STATIC} ${GMP_STATIC})
#target_link_libraries(minigiac mpfr gmp) # minigiac can also be built as a dynamic executable

# JNI/DLL

set(GIAC_DEF "")

if(WIN32)
    set(GIAC_DEF ${CMAKE_SOURCE_DIR}/src/jni/giac.def)
endif()

add_library(javagiac SHARED
        ${GIAC_SOURCES}
        src/jni/cpp/giac_wrap.cxx
        ${GIAC_DEF})

# Note the order: MPFR must precede GMP!
target_link_libraries(javagiac ${MPFR_STATIC} ${GMP_STATIC})

set_target_properties(javagiac PROPERTIES LINK_FLAGS -s) # strip

target_include_directories(javagiac PUBLIC
        ${CMAKE_SOURCE_DIR}/src/jni/jdkHeaders
        ${CMAKE_SOURCE_DIR}/src/giac/headers)

if(APPLE)
    # Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
    # link_directories(/usr/local/lib) # This is necessary only if you use Homebrew's libs.
    add_definitions(-DDONT_USE_LIBLAPLACK)
    target_include_directories(javagiac PUBLIC
        ${CMAKE_SOURCE_DIR}/src/jni/jdkHeaders/darwin)
    # target_link_directories(javagiac PUBLIC /usr/local/lib) # Only for Homebrew's libs.
    # target_link_directories(minigiac PUBLIC /usr/local/lib) # Only for Homebrew's libs.
endif()

if(UNIX)
    add_definitions(-DHAVE_SYS_TIMES_H -DHAVE_UNISTD_H -DHAVE_SYS_TIME_H -DHAVE_SYSCONF)
endif()

if(LINUX)
    target_include_directories(javagiac PUBLIC
        ${CMAKE_SOURCE_DIR}/src/jni/jdkHeaders/linux)
    target_link_options(javagiac PRIVATE -static-libgcc -static-libstdc++)
endif()

if(WIN32)
    add_definitions(-DGIAC_MPQS -D__MINGW_H -DMINGW32 -DHAVE_NO_SYS_TIMES_H
        -DHAVE_NO_SYS_RESOURCE_WAIT_H -DHAVE_NO_PWD_H -DHAVE_NO_CWD -DNO_CLOCK
        -Dusleep= -DYY_NO_UNISTD_H)
    target_include_directories(javagiac PUBLIC
        ${CMAKE_SOURCE_DIR}/src/jni/jdkHeaders/win)
    target_link_options(javagiac PRIVATE -static-libgcc -static-libstdc++)
endif()