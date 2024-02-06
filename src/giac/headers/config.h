// This file has been taylored after autogenerating it by the autoconf machinery.
// Which is, actually, no longer used.

/* Additional settings or overrides should be put
 * into build.gradle. This file should be considered as
 * as a "common base" for all platforms.
 */

/* Set if debugging is enabled */
#define DEBUG_SUPPORT

/* Name of package */
#define PACKAGE "giac"

/* Define to the address where bug reports for this package should be
sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "giac"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "giac 1.9.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "giac"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.9.0"

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#ifdef __x86_64__
#define SIZEOF_LONG 8
#define GIAC_GENERIC_CONSTANTS
#define SMARTPTR64
#else
#define SIZEOF_LONG 4
#endif

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8



/* Version number of package */
#define VERSION "1.9.0-xx"


#define GIAC_NO_OPTIMIZATIONS
#define HAVE_NO_HOME_DIRECTORY

#define HAVE_SYSCONF
#if 0
#define BF2GMP 1
#else
#define HAVE_LIBMPFR
#define HAVE_MPFR_H 1
//#define HAVE_LIBMPFI
#endif
//#define HAVE_LIBPARI // uncomment if you want to include pari, currently buggy
#define HAVE_LIBGLPK 1
#define TIMEOUT
#define HAVE_UNISTD_H
//#define HAVE_SYS_TIMES_H 1
//#define EMCC_FETCH
//#define HAVE_LIBSAMPLERATE 1
#define DBL_MIN (-DBL_MAX)