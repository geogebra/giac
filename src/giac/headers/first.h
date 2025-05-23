/*
 *  Copyright (C) 2000,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#undef max
#undef min
//#define GIAC_VECTOR

#ifndef _GIAC_FIRST_H_
#define _GIAC_FIRST_H_

#define INT_MAXSHIFT (sizeof(int)*8-1)
#define INT_MAXSHIFTM1 (sizeof(int)*8-2)

#if __cplusplus >= 201103L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201103L)
// post-c++11 functional headers changes, thanks to George Huebner
#define CPP11
#endif
  
// register is deprecated in c++17
#define register 


#ifdef _GLIBCXX_ASSERTIONS
#undef _GLIBCXX_ASSERTIONS
#endif

#if defined NUMWORKS && !defined SDL_KHICAS
#define KHICAS 1
#ifdef NUMWORKS_SLOTBFR
#define NUMWORKS_SLOTB
#endif
#ifdef NUMWORKS_SLOTBEN
#define NUMWORKS_SLOTB
#endif
#endif

#ifndef GIAC_VERSION
#define GIAC_VERSION VERSION
#endif
//#include <stdint.h>

// mingw now defines x86_64
#if (defined(__x86_64__) || defined(__arm64__)) && !defined __MINGW_H
#define x86_64 1
#else
#ifdef __MINGW_H
 
#define MINGW32
#ifndef M_LN2
#define M_LN2 0.693147180559945310
#endif
#ifndef M_PI
#define M_PI       3.14159265358979323846
#define M_PI_2       (M_PI/2)
#endif
#ifndef M_E
#define M_E       2.71828182845904524
#endif
#ifndef M_SQRT2
#define M_SQRT2       1.41421356237309505
#endif
#endif
#endif

#ifdef __VISUALC__
#define M_SQRT2 1.4142135623730950
#define M_PI_2 1.5707963267948966
#define M_LN2 0.69314718055994531
#define M_1_PI 0.31830988618379067
#endif

#ifdef HP39
#include <time.h>
#define M_E 2.7182818284590452
#endif

// Thanks to Jason Papadopoulos, author of msieve
#ifdef BESTA_OS
#include <time.h>
#define PREFETCH(addr) /* nothing */
#elif (defined(__GNUC__) && __GNUC__ >= 3) || defined(__clang__)
	#define PREFETCH(addr) __builtin_prefetch(addr) 
#elif defined(_MSC_VER) && _MSC_VER >= 1400 && !defined(MS_SMART)
	#define PREFETCH(addr) PreFetchCacheLine(PF_TEMPORAL_LEVEL_1, addr)
#else
	#define PREFETCH(addr) /* nothing */
#endif

#ifdef PNACL
#define EMCC
#undef HAVE_LIBPTHREAD
#undef HAVE_PTHREAD_H
#undef x86_64
#undef SMARTPTR64
#undef HAVE_LONG_DOUBLE
#endif

#if defined __VISUALC__ 
#undef BIGENDIAN
#endif


#ifdef RTOS_THREADX
#define NO_STDEXCEPT 1
#endif

#define MAX_INTSTACK 32768 // maximal size for allocating an array by int tab[]


#ifdef FXCG
#define RAND_MAX 2147483647
#define clock() 0
#define CLOCK() 0
#define CLOCK_T int
#undef HAVE_LIBDL
#undef HAVE_LIBPTHREAD
struct Bidon {
  int i;
Bidon(int i_=0):i(i_){}
  flush(){}
};
template<class T> Bidon operator << (Bidon ,const T&){ return Bidon(); }
inline Bidon operator << (Bidon,const char *){return Bidon();}
// #define CIN 0 //std::cin
#define COUT Bidon(0) //std::cout
#define CERR Bidon(0) //std::cout
typedef unsigned pid_t;
extern "C" double lgamma(double);
#else // FXCG

#ifdef NSPIRE
#define clock() 0
#undef HAVE_LIBDL
#undef HAVE_LIBPTHREAD
#include <os.h>
#define CIN (*std::console_cin_ptr)
#define COUT (*std::console_cin_ptr)
#define CERR (*std::console_cin_ptr)
#else // NSPIRE
#define CIN std::cin
#define COUT std::cout
#if defined(EMCC) || defined(EMCC2)
#define CERR std::cout
extern "C" double emcctime(); 
extern "C" int glinit(int,int,int,int,int);
extern "C" void glcontext(int);
#define CLOCK emcctime
#define CLOCK_T clock_t
#else // EMCC
#define CERR std::cerr
#if defined(MS_SMART) || defined(NO_CLOCK) || defined __MINGW_H
#define CLOCK() 0
#define CLOCK_T int
#else
#define CLOCK clock
#define CLOCK_T clock_t
#endif // MS_SMART
#endif // EMCC
#endif // NSPIRE
#endif // FXCG

#ifdef __sparc__
#define DOUBLEVAL
#define GIAC_NO_OPTIMIZATIONS
#endif

#if defined(BUILDING_NODE_EXTENSION) && defined(_WIN64)
#define DOUBLEVAL
#endif

#ifndef DOUBLEVAL
// #define IMMEDIATE_VECTOR 12 // 48 extra bytes = 6 gens or 4 monomials or 3 sparse
#define IMMEDIATE_VECTOR 6 // 24 extra bytes = 3 gens or 2 monomials or 1 sparse
#endif
#define GIAC_MPZ_INIT_SIZE 128 // initial number of bits for mpz

#if 0 // def HAVE_LONG_DOUBLE
typedef long double giac_double;
#else
typedef double giac_double;
#endif

typedef long double  long_double;

// sprintf replacement
// calls snprintf instead of sprintf with n=512, assumes s bufsize>=512
int sprintf512(char *s,const char *format,...);
int my_sprintf(char * s, const char * format, ...);
#ifdef GIAC_HAS_STO_38
//#define WITH_MYOSTREAM
#endif
#ifdef TICE
#if 0 // if we find out that the OS sprintf is faster than nanoprintf, use it instead
#include <ti/sprintf.h>
#else
// fallback
#define boot_sprintf sprintf
#endif
#endif

#ifdef WITH_MYOSTREAM
#include "myostream.h"
#else
#if defined KHICAS || defined SDL_KHICAS //&& defined STATIC_BUILTIN_LEXER_FUNCTION
#include "stdstream"
#define my_ostream stdostream
#else
#define my_ostream std::ostream
#endif
#endif

#ifdef x86_64
#define alias_type ulonglong
#else
#define alias_type size_t
#endif

#if defined(RTOS_THREADX) || defined(BESTA_OS) || defined NSPIRE || defined KHICAS
#define NO_TEMPLATE_MULTGCD
#endif

#ifdef BESTA_OS
#undef CLOCK
#undef CLOCK_T
#define CLOCK() PrimeGetNow()
#define CLOCK_T int
#endif

#if !defined HAVE_ALLOCA_H && !defined GIAC_HAS_STO_38 && !defined KHICAS
#define alloca _alloca
#endif

#ifdef NO_UNARY_FUNCTION_COMPOSE

#define define_unary_function_eval(name,ptr,name_s) const alias_unary_function_eval name={name_s,0,taylor,0,0,0,ptr,0}
#define define_unary_function_eval2(name,ptr,name_s,printptr) const alias_unary_function_eval name={name_s,0,taylor,printptr,0,0,ptr,0}
#define define_unary_function_eval3(name,ptr,derivee,name_s) const alias_unary_function_eval name={name_s,derivee,taylor,0,0,0,ptr,0}
#define define_unary_function_eval4(name,ptr,name_s,printptr,texprintptr) const alias_unary_function_eval name={name_s,0,taylor,printptr,texprintptr,0,ptr,0}
#define define_unary_function_eval5(name,ptr,derive,name_s,printptr,texprintptr) const alias_unary_function_eval name={name_s,derive,taylor,printptr,texprintptr,0,ptr,0}
#define define_unary_function_eval_taylor(name,ptr,derive,taylors,name_s) const alias_unary_function_eval name={name_s,derive,taylors,0,0,0,ptr,0}
#define define_unary_function_eval_taylor2(name,ptr,derive,taylors,name_s,printptr,texprintptr) const alias_unary_function_eval name={name_s,derive,taylors,printptr,texprintptr,0,ptr,0}
#define define_unary_function_eval_quoted(name,ptr,name_s) const alias_unary_function_eval name={name_s,0,taylor,0,0,0,ptr,1}
#define define_unary_function_eval2_quoted(name,ptr,name_s,printptr) const alias_unary_function_eval name={name_s,0,taylor,printptr,0,0,ptr,1}
#define define_unary_function_eval3_quoted(name,ptr,derivee,name_s) const alias_unary_function_eval name={name_s,derivee,taylor,0,0,0,ptr,1}
#define define_unary_function_eval4_quoted(name,ptr,name_s,printptr,texprintptr) const alias_unary_function_eval name={name_s,0,taylor,printptr,texprintptr,0,ptr,1}
#define define_unary_function_eval5_quoted(name,ptr,derive,name_s,printptr,texprintptr) const alias_unary_function_eval name={name_s,derive,taylor,printptr,texprintptr,0,ptr,1}
#define define_unary_function_eval_taylor_quoted(name,ptr,derive,taylors,name_s) const alias_unary_function_eval name={name_s,derive,taylors,0,0,0,ptr,1}
#define define_unary_function_eval_taylor2_quoted(name,ptr,derive,taylors,name_s,printptr,texprintptr) const alias_unary_function_eval name={name_s,derive,taylors,printptr,texprintptr,0,ptr,1}

#define define_unary_function_eval_index(u,name,ptr,name_s) const alias_unary_function_eval name={name_s,0,taylor,0,0,0,ptr,u}
#define define_unary_function_eval2_index(u,name,ptr,name_s,printptr) const alias_unary_function_eval name={name_s,0,taylor,printptr,0,0,ptr,u}
#define define_unary_function_eval3_index(u,name,ptr,derivee,name_s) const alias_unary_function_eval name={name_s,derivee,taylor,0,0,0,ptr,u}
#define define_unary_function_eval4_index(u,name,ptr,name_s,printptr,texprintptr) const alias_unary_function_eval name={name_s,0,taylor,printptr,texprintptr,0,ptr,u}
#define define_unary_function_eval5_index(u,name,ptr,derive,name_s,printptr,texprintptr) const alias_unary_function_eval name={name_s,derive,taylor,printptr,texprintptr,0,ptr,u}
#define define_unary_function_eval_taylor_index(u,name,ptr,derive,taylors,name_s) const alias_unary_function_eval name={name_s,derive,taylors,0,0,0,ptr,u}
#define define_unary_function_eval_taylor2_index(u,name,ptr,derive,taylors,name_s,printptr,texprintptr) const alias_unary_function_eval name={name_s,derive,taylors,printptr,texprintptr,0,ptr,u}

#define define_partial_derivative_onearg_unary_function_ptr(name,fcn) const size_t name=(const size_t) &fcn
#define define_partial_derivative_onearg_genop(name,name_s,genop) const alias_unary_function_eval name##unary_function_eval={name_s,0,taylor,0,0,0,genop,0}; const size_t name##unary_function_ptr = (const size_t)(&name##unary_function_eval); const size_t name=(const size_t) &name##unary_function_ptr

#else //  NO_UNARY_FUNCTION_COMPOSE

#define define_unary_function_eval(name,ptr,name_s) const unary_function_eval name(0,ptr,name_s)
#define define_unary_function_eval2(name,ptr,name_s,printptr) const unary_function_eval name(0,ptr,name_s,printptr)
#define define_unary_function_eval3(name,ptr,derivee,name_s) const unary_function_eval name(0,ptr,derivee,name_s)
#define define_unary_function_eval4(name,ptr,name_s,printptr,texprintptr) const unary_function_eval name(0,ptr,name_s,printptr,texprintptr)
#define define_unary_function_eval5(name,ptr,derive,name_s,printptr,texprintptr) const unary_function_eval name(0,ptr,derive,name_s,printptr,texprintptr)
#define define_unary_function_eval_taylor(name,ptr,derive,taylors,name_s) const unary_function_eval name(0,ptr,derive,taylors,name_s)
#define define_unary_function_eval_taylor2(name,ptr,derive,taylors,name_s,printptr,texprintptr) const unary_function_eval name(0,ptr,derive,taylors,name_s,printptr,texprintptr)
#define define_unary_function_eval_quoted(name,ptr,name_s) const unary_function_eval name(1,ptr,name_s)
#define define_unary_function_eval2_quoted(name,ptr,name_s,printptr) const unary_function_eval name(1,ptr,name_s,printptr)
#define define_unary_function_eval3_quoted(name,ptr,derivee,name_s) const unary_function_eval name(1,ptr,derivee,name_s)
#define define_unary_function_eval4_quoted(name,ptr,name_s,printptr,texprintptr) const unary_function_eval name(1,ptr,name_s,printptr,texprintptr)
#define define_unary_function_eval5_quoted(name,ptr,derive,name_s,printptr,texprintptr) const unary_function_eval name(1,ptr,derive,name_s,printptr,texprintptr)
#define define_unary_function_eval_taylor_quoted(name,ptr,derive,taylors,name_s) const unary_function_eval name(1,ptr,derive,taylors,name_s)
#define define_unary_function_eval_taylor2_quoted(name,ptr,derive,taylors,name_s,printptr,texprintptr) const unary_function_eval name(1,ptr,derive,taylors,name_s,printptr,texprintptr)
#define define_unary_function_eval_index(u,name,ptr,name_s) const unary_function_eval name(u,ptr,name_s)
#define define_unary_function_eval2_index(u,name,ptr,name_s,printptr) const unary_function_eval name(u,ptr,name_s,printptr)
#define define_unary_function_eval3_index(u,name,ptr,derivee,name_s) const unary_function_eval name(u,ptr,derivee,name_s)
#define define_unary_function_eval4_index(u,name,ptr,name_s,printptr,texprintptr) const unary_function_eval name(u,ptr,name_s,printptr,texprintptr)
#define define_unary_function_eval5_index(u,name,ptr,derive,name_s,printptr,texprintptr) const unary_function_eval name(u,ptr,derive,name_s,printptr,texprintptr)
#define define_unary_function_eval_taylor_index(u,name,ptr,derive,taylors,name_s) const unary_function_eval name(u,ptr,derive,taylors,name_s)
#define define_unary_function_eval_taylor2_index(u,name,ptr,derive,taylors,name_s,printptr,texprintptr) const unary_function_eval name(u,ptr,derive,taylors,name_s,printptr,texprintptr)

#define define_partial_derivative_onearg_genop(name,name_s,genop) static const unary_function_eval name##unary_function_eval(0,genop,name_s); static const unary_function_ptr name##unary_function_ptr(&name##unary_function_eval,0,0); static const partial_derivative_onearg name##partial_derivative_onearg(name##unary_function_ptr); const partial_derivative_onearg * name = &name##partial_derivative_onearg

#endif //  NO_UNARY_FUNCTION_COMPOSE

#ifdef x86_64
#define define_unary_function_ptr(name,alias_name,ptr) const ulonglong alias_name = (ulonglong)(ptr); const unary_function_ptr * const name = (const unary_function_ptr *) &alias_name
#else
#define define_unary_function_ptr(name,alias_name,ptr) const size_t alias_name = (size_t)(ptr); const unary_function_ptr * const name = (const unary_function_ptr *) &alias_name
#endif

#ifdef DOUBLEVAL 
#undef STATIC_BUILTIN_LEXER_FUNCTIONS
// otherwise change the definition of charptr_gen in input_lexer.ll
#endif

#ifdef STATIC_BUILTIN_LEXER_FUNCTIONS
#ifdef x86_64
#define define_unary_function_ptr5(name,alias_name,ptr,quoted,token) const ulonglong alias_name = ulonglong(ptr); const unary_function_ptr * const name = (const unary_function_ptr *) &alias_name;
#else
#define define_unary_function_ptr5(name,alias_name,ptr,quoted,token) const size_t alias_name = (size_t)(ptr); const unary_function_ptr * const name = (const unary_function_ptr *) &alias_name;
#endif
#else
#ifdef x86_64
#define define_unary_function_ptr5(name,alias_name,ptr,quoted,token) static const unary_function_ptr alias_name##_(ptr,quoted,token); const ulonglong alias_name=(ulonglong)ptr; const unary_function_ptr * const name = &alias_name##_;
#else
#define define_unary_function_ptr5(name,alias_name,ptr,quoted,token) static const unary_function_ptr alias_name##_(ptr,quoted,token); const size_t alias_name=(size_t)ptr; const unary_function_ptr * const name = &alias_name##_;
#endif
#endif

#ifdef GIAC_HAS_STO_38
#ifndef GIAC_GENERIC_CONSTANTS
#define GIAC_GENERIC_CONSTANTS
#endif
#endif

#ifdef __VISUALC__ 
#define _ITERATOR_DEBUG_LEVEL 0
#define GIAC_GENERIC_CONSTANTS
// Visual C++ is compiling
#define VISUALC
typedef long pid_t;
typedef __int64 longlong ;
typedef unsigned __int64 ulonglong ;
#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif
#define YY_NO_UNISTD_H
#ifndef _SECURE_SCL
#define _SECURE_SCL 0
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif
#else // __VISUALC__
typedef long long longlong;
typedef unsigned long long ulonglong;
#ifdef x86_64
typedef int int128_t __attribute__((mode(TI)));
typedef unsigned int uint128_t __attribute__((mode(TI)));
#ifndef INT128
#define INT128 1
#endif
#endif // x86_64

// do not define PSEUDO_MOD if a negative unsigned longlong >> 63 is != 0xffffffffffffffff
// #define PSEUDO_MOD accelerates cyclic* gbasis computation significantly
// from int_multilinear_combination in vecteur.cc (from rref?)
#ifdef FIR
#if !(defined(BESTA_OS) || defined(WINDOWS) || defined(OSXIOS) || defined(FIR_LINUX) || defined(FIR_ANDROID) || defined(FREERTOS) || defined(PRIMEWEBASM))
// was #if !(defined(IOS) || defined(__ANDROID__)) && !defined(OSX) && !defined(LINUX)
#define PSEUDO_MOD 
#endif
#else
#define PSEUDO_MOD 
#endif

#endif // __VISUALC__


#ifdef VISUALC
inline void swap_giac_double(double & a,double & b){ double c=a; a=b; b=c; }
#else
#define swap_giac_double(a,b) std::swap<giac_double>(a,b)
#endif

#if defined x86_64
typedef longlong ref_count_t;
#else
typedef int ref_count_t;
#endif

#ifdef WINSTORE
//tw  **NOTE** this is pulled out of winnt.h!!! I don't know why it is not found there.
//             there is some sort of interaction in windows ARM builds... 
//#define CP15_TPIDRURW          15, 0, 13,  0, 2         // Software Thread ID Register, User Read/Write
#endif

#ifndef SIZEOF_VOID_P
#ifdef __APPLE__
#ifdef __LP64__
#define SIZEOF_VOID_P 8
#else
#define SIZEOF_VOID_P 4
#endif
#endif //__APPLE__

#ifndef SIZEOF_VOID_P
//#error "No SIZEOF_VOID_P defined on this system!!!!"
#endif
#endif

#if __BYTE_ORDER__ ==__ORDER_LITTLE_ENDIAN__ && SIZEOF_VOID_P==8 //(defined(__LP64__) || defined(_WIN64) || (defined(x86_64) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__))
#if !defined(BUILDING_NODE_EXTENSION) || !defined(_WIN64)
#define SMARTPTR64
#endif
#else // x86_64
#ifdef SMARTPTR64
#undef SMARTPTR64
#endif // SMARTPTR64
#ifdef _I386_
#undef _I386_
#endif // _I386_
#endif // x86_64

#ifdef USE_GMP_REPLACEMENTS
//#define GIAC_TYPE_ON_8BITS
#undef HAVE_GMPXX_H
#undef HAVE_LIBMPFR
#include "gmp_replacements.h"
#else // USE_GMP_REPLACEMENTS
#include <cstddef>
#ifdef BF2GMP
#include "bf2gmp.h"
// #undef HAVE_LIBMPFR // to be replaced by defined later
#undef HAVE_LIBMPFI // to be replaced by defined later
#undef HAVE_GMPXX_H
#else
#include "gmp.h"
#endif // BF2GMP
#endif // USE_GMP_REPLACEMENTS

#ifndef FXCG
#include <cassert>
#endif

class init_gmp_memory 
{
	static int refcount;
	public:
	init_gmp_memory();
	~init_gmp_memory();
};
extern init_gmp_memory init_gmp_memory_instance;

#ifdef BCD
// #define ASPEN_GEOMETRY // required also here because of name resolution in identificateur.cc
// #define CAS38_DISABLED

#include <stdlib.h>
#include "bcd_float.h" 
// abstract structure for BCD float operations
// bcd_float.h should define a type for bcd_float, 
// that fits into a gen (it can be a 64 bits integer but only 56 bits avail.)
// For accuracy, the BCD floats should have an expanded form
// of type accurate_bcd_float, that one can get using 
//   accurate_bcd_float * fExpand(bcd_float,accurate_bcd_float *)
// This accurate value is returned from a gen g using
//   gentobcd(g,accurate_bcd_float *)
// bcd_float.h should define the following operations 
//   fradd_g, frsub_g, fmul_g, fdiv_g, fchs_g, fpow_g
//   fLT_g, fGT_g (strict comparison, return bool), fEQ_g,
//   fabs_g, fsqrt_g, fZero_g (is ==0)
//   bcd_zero, bcd_set_double, bcd_get_double, bcd_set_int, bcd_get_int, 
struct giac_bcdfloat {
  bcd_float f;
  giac_bcdfloat(const bcd_float & _f){f=_f;}
  giac_bcdfloat(int i) {f=bcd_set_int(i);}
  giac_bcdfloat(mpz_t * m) {f=bcd_set_mpz(m);}
  giac_bcdfloat(const giac_bcdfloat & _f) {f=_f.f;}
  giac_bcdfloat(){f=bcd_zero;}
  giac_bcdfloat(double d){ f=bcd_set_double(d);}
};
typedef giac_bcdfloat giac_float;
inline void print_float(const giac_bcdfloat & f,char * ch, CHPPrintData const &pd) { bcd_print(f.f,ch, pd); }
void print_float(const giac_bcdfloat & f,char * ch);

inline giac_bcdfloat operator + (const giac_bcdfloat & f1,const giac_bcdfloat & f2){
  return fradd_g(f1.f,f2.f);
}
inline giac_bcdfloat operator += (giac_bcdfloat & f1,const giac_bcdfloat & f2){
  f1.f=fradd_g(f1.f,f2.f);
  return f1;
}
inline giac_bcdfloat operator - (const giac_bcdfloat & f1,const giac_bcdfloat & f2){
  return frsub_g(f1.f,f2.f);
}
inline giac_bcdfloat operator -= (giac_bcdfloat & f1,const giac_bcdfloat & f2){
  f1.f=frsub_g(f1.f,f2.f);
  return f1;
}
inline giac_bcdfloat operator - (const giac_bcdfloat & f1){
  return fchs_g(f1.f);
}
inline giac_bcdfloat operator * (const giac_bcdfloat & f1,const giac_bcdfloat & f2){
  return fmul_g(f1.f,f2.f);
}
inline giac_bcdfloat operator *= (giac_bcdfloat & f1,const giac_bcdfloat & f2){
  f1.f=fmul_g(f1.f,f2.f);
  return f1;
}
inline giac_bcdfloat operator / (const giac_bcdfloat & f1,const giac_bcdfloat & f2){
  return fdiv_g(f1.f,f2.f);
}
inline giac_bcdfloat operator /= (giac_bcdfloat & f1,const giac_bcdfloat & f2){
  f1.f=fdiv_g(f1.f,f2.f);
  return f1;
}
inline bool operator == (const giac_bcdfloat & f1,const giac_bcdfloat & f2){
  return fEQ_g(f1.f,f2.f);
}
inline bool operator > (const giac_bcdfloat & f1,const giac_bcdfloat & f2){ return fGT_g(f1.f,f2.f); }
inline bool operator < (const giac_bcdfloat & f1,const giac_bcdfloat & f2){ return fLT_g(f1.f,f2.f); }
inline bool operator >= (const giac_bcdfloat & f1,const giac_bcdfloat & f2){ return fGE_g(f1.f,f2.f); }
inline bool operator <= (const giac_bcdfloat & f1,const giac_bcdfloat & f2){ return fLE_g(f1.f,f2.f); }
inline int get_int(const giac_bcdfloat & f) { return bcd_get_int(f.f);}
inline giac_bcdfloat fabs (const giac_bcdfloat & f1){ return fabs_g(f1.f);}
inline int fsign (const giac_bcdfloat & f1){ return fZero_g(f1.f)?0:(fPos_g(f1.f)?1:-1); }
inline giac_bcdfloat fsqrt (const giac_bcdfloat & f1){ return fsqrt_g(f1.f); }
inline giac_bcdfloat finv (const giac_bcdfloat & f1){ return finv_g(f1.f); }
inline giac_bcdfloat fsquare (const giac_bcdfloat & f1){ return fsquare_g(f1.f); }
inline giac_bcdfloat fpow(const giac_bcdfloat & f1,const giac_bcdfloat & f2) {   return fpow_g(f1.f,f2.f); }
inline giac_bcdfloat fround(const giac_bcdfloat & f1,int digits) {   if (digits>=0) return frnd_g(f1.f,digits,false); else return frnd_g(f1.f,-digits,true); }
inline giac_bcdfloat ftrunc(const giac_bcdfloat & f1,int digits) {   return ftrunc_g(f1.f,digits,false); }
inline giac_bcdfloat ffloor(const giac_bcdfloat & f){ return ffloor_g(f.f);}
inline giac_bcdfloat fceil(const giac_bcdfloat & f){ return fceil_g(f.f);}
inline giac_bcdfloat fsin(const giac_bcdfloat & f,int angle_mode){ return fsin_g(f.f,TAngleMode(angle_mode));}
inline giac_bcdfloat fcos(const giac_bcdfloat & f,int angle_mode){ return fcos_g(f.f,TAngleMode(angle_mode));}
inline giac_bcdfloat ftan(const giac_bcdfloat & f,int angle_mode){ return ftan_g(f.f,TAngleMode(angle_mode));}
inline giac_bcdfloat fcot(const giac_bcdfloat & f,int angle_mode){ return fcot_g(f.f,TAngleMode(angle_mode));}
inline giac_bcdfloat fasin(const giac_bcdfloat & f,int angle_mode){ return fasin_g(f.f,TAngleMode(angle_mode));}
inline giac_bcdfloat facos(const giac_bcdfloat & f,int angle_mode){ return facos_g(f.f,TAngleMode(angle_mode));}
inline giac_bcdfloat fatan(const giac_bcdfloat & f,int angle_mode){ return fatan_g(f.f,TAngleMode(angle_mode));}
inline giac_bcdfloat facot(const giac_bcdfloat & f,int angle_mode){ return facot_g(f.f,TAngleMode(angle_mode));}
inline giac_bcdfloat fsinh(const giac_bcdfloat & f){ return fsinh_g(f.f);}
inline giac_bcdfloat fcosh(const giac_bcdfloat & f){ return fcosh_g(f.f);}
inline giac_bcdfloat ftanh(const giac_bcdfloat & f){ return ftanh_g(f.f);}
inline giac_bcdfloat fasinh(const giac_bcdfloat & f){ return fasinh_g(f.f);}
inline giac_bcdfloat facosh(const giac_bcdfloat & f){ return facosh_g(f.f);}
inline giac_bcdfloat fatanh(const giac_bcdfloat & f){ return fatanh_g(f.f);}
inline giac_bcdfloat flog(const giac_bcdfloat & f){ return fln_g(f.f);}
inline giac_bcdfloat fln(const giac_bcdfloat & f){ return fln_g(f.f);}
inline giac_bcdfloat flnp1(const giac_bcdfloat & f){ return flnp1_g(f.f);}
inline giac_bcdfloat fexp(const giac_bcdfloat & f){ return fexp_g(f.f);}
inline giac_bcdfloat fexpm(const giac_bcdfloat & f){ return fexpm_g(f.f);}
inline giac_bcdfloat flog10(const giac_bcdfloat & f){ return flog_g(f.f);}
inline giac_bcdfloat falog10(const giac_bcdfloat & f){ return falog_g(f.f);}
inline giac_bcdfloat fgamma(const giac_bcdfloat & f){ return fgamma_g(f.f); }
inline giac_bcdfloat fmod(const giac_bcdfloat & f1,const giac_bcdfloat & f2){ return fmod_g(f1.f,f2.f);}
inline giac_bcdfloat fnthroot(const giac_bcdfloat & f1,const giac_bcdfloat & f2){ return fnthroot_g(f1.f,f2.f);}
inline giac_bcdfloat fmax(const giac_bcdfloat & f1,const giac_bcdfloat & f2){ return fmax_g(f1.f,f2.f);}
inline giac_bcdfloat fmin(const giac_bcdfloat & f1,const giac_bcdfloat & f2){ return fmin_g(f1.f,f2.f);}
inline bool fis_exactly_zero(const giac_bcdfloat & f){ return fZero_g(f.f);}
inline bool fis_integer(const giac_bcdfloat & f){ return fInteger_g(f.f); }
inline bool fis_positive(const giac_bcdfloat & f){ return fPos_g(f.f);}
inline bool fis_nan(const giac_bcdfloat & f){ return fNaN_g(f.f); }
inline bool fis_inf(const giac_bcdfloat & f){ return finfmax_g(f.f); }
inline bool fis_inf_notmax(const giac_bcdfloat & f){ return fInf_g(f.f); }
inline giac_bcdfloat fpi(){ return fpi_g(); }
inline giac_bcdfloat atan2f(const giac_bcdfloat & re,const giac_bcdfloat & im,int anglemode=0){ return fatan2_g(re.f,im.f,anglemode); }

inline giac_bcdfloat strtobcd(const char *nptr,const char **endptr, CHPPrintData const &pd){ return fstrtobcd(nptr,endptr,pd); }
giac_bcdfloat strtobcd(const char *nptr,const char **endptr);
inline double get_double(const giac_bcdfloat & f) { 
  // FIXME
#ifdef GIAC_HAS_STO_38
  return bcd_get_float(f.f);
#else
  char ch[32]; char * endchar;
  bool pos=fis_positive(f) || fis_exactly_zero(f);
  if (fis_inf(f)){ double d=0.0; return 1./d; }
  if (fis_nan(f)){ double d=0.0; return 0./d; }
  if (!pos) ch[0]='-';
  print_float(pos?f.f:(-f).f,pos?ch:ch+1,'.'|('E'<<16)|(' '<<24),12+(15<<8),0);
  return strtod(ch,&endchar); 
#endif
}
#else // defined BCD

#include <cmath>
typedef float giac_float;
#define fis_inf_notmax fis_inf
inline bool fis_exactly_zero(float f){ return f==0;}
inline double get_double(float f) { return f;}
inline int get_int(float f) { return int(f);}
inline int fsign (float f1){return f1==0?0:(f1>0?1:-1);}
float fsqrt (float f1);
void print_float(const giac_float & f,char * ch);
inline float fpow(float f1,float f2){ 
#if defined NSPIRE || defined FXCG
  return pow(f1,f2); 
#else
  return std::pow(f1,f2); 
#endif
}
inline float ffloor(float f1){ 
#if defined NSPIRE || defined FXCG
  return floor(f1); 
#else
  return std::floor(f1); 
#endif
}
inline float finv(float f1){ return 1/f1; }
#ifdef FXCG
inline float atan2f(float f1,float f2,int rad){ if (rad) return std::atan2(f1,f2); else return std::atan2(f1,f2)*180/3.14159265358979323846;}
#else
inline float atan2f(float f1,float f2,int rad){ if (rad) return atan2f(f1,f2); else return atan2f(f1,f2)*180/3.14159265358979323846;}
#endif
#define fis_nan my_isnan
#define fis_inf my_isinf
#endif // BCD

#ifdef FIR_ANDROID
#undef B0 //this conflicts with a define
#undef bcopy //this conflicts with a define
#endif

#ifdef FXCG
namespace ustl {
  inline double abs(double d){ return ::fabs(d); }
  inline double tan(double d){ return ::tan(d); }
  inline double atan(double d){ return ::atan(d); }
  inline double asin(double d){ return ::asin(d); }
  inline double sin(double d){ return ::sin(d); }
  inline double acos(double d){ return ::acos(d); }
  inline double cos(double d){ return ::cos(d); }
  inline double tanh(double d){ return ::tanh(d); }
  inline double atanh(double d){ return ::atanh(d); }
  inline double asinh(double d){ return ::asinh(d); }
  inline double sinh(double d){ return ::sinh(d); }
  inline double acosh(double d){ return ::acosh(d); }
  inline double cosh(double d){ return ::cosh(d); }
  inline double log(double d){ return ::log(d); }
  inline double log10(double d){ return ::log10(d); }
  inline double exp(double d){ return ::exp(d); }
  inline double sqrt(double d){ return ::sqrt(d); }
  inline double floor(double d){ return ::floor(d); }
  inline double ceil(double d){ return ::ceil(d); }
  inline double pow(double d1,double d2){ return ::pow(d1,d2); }
  inline double atan2(double d1,double d2){ return ::atan2(d1,d2); }
}
#endif

//#ifdef __MINGW_H
#undef RAND_MAX
#define RAND_MAX 2147483647
//#endif
#endif // _GIAC_FIRST_H_
