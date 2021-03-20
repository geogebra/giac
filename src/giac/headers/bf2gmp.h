#ifndef BF2GMP_H
#define BF2GMP_H
#include "config.h"
#define INT64_MIN 9223372036854775808UL
#define INT64_MAX 9223372036854775807LL 
#define INT32_MIN -2147483648
#define INT32_MAX 2147483647
#if SIZEOF_VOID_P==8
#define INTPTR_MAX 9223372036854775807LL
#else
#define INTPTR_MAX 2147483647
#endif
extern "C" { 
#include "libbf.h"
}
#include "stdlib.h"
#include "string.h"
#include "math.h"

extern "C" int ctrl_c_interrupted(int exception);

typedef bf_t mpz_t;

extern "C" void * bf_ctx_ptr;

inline int mpz_get_si(const mpz_t & a){
  int64_t pres;
  bf_get_int64(&pres,&a,BF_GET_INT_MOD);
  return pres;
} 
inline uint64_t mpz_get_ui(const mpz_t & a){
  uint64_t pres;
  bf_get_uint64(&pres,&a);
  return pres;
} 
inline void mpz_init(mpz_t &  a){ bf_init((bf_context_t *)bf_ctx_ptr,&a);}
inline void mpz_init2(mpz_t &  a,int size){ bf_init((bf_context_t *)bf_ctx_ptr,&a);}
inline void mpz_init_set(mpz_t &  a,const mpz_t & b){ bf_init((bf_context_t *)bf_ctx_ptr,&a); bf_set(&a,&b);}
inline void mpz_init_set_si(mpz_t &  a,long b){ bf_init((bf_context_t *)bf_ctx_ptr,&a); bf_set_si(&a,b);}
inline int mpz_init_set_ui(mpz_t &  a,unsigned long b){ bf_init((bf_context_t *)bf_ctx_ptr,&a); bf_set_ui(&a,b);}
inline void mpz_clear(mpz_t & a){ bf_delete(&a);}
inline int mpz_sgn(const mpz_t & a){ if (bf_is_zero(&a)) return 0; return a.sign==0?1:-1;}
inline void mpz_set(mpz_t &  a,const mpz_t & b){ bf_set(&a,&b);}
inline void mpz_set_ui(mpz_t &  a,unsigned long ui){ bf_set_ui(&a,ui); }
inline void mpz_set_si(mpz_t &  a,long ui){ bf_set_si(&a,ui); }
inline void mpz_swap(mpz_t & a,mpz_t &b){
#if 0
  mpz_t tmp; mpz_init_set(tmp,a);
  mpz_set(a,b);
  mpz_set(b,tmp);
  mpz_clear(tmp);
#else
  bf_t tmp=a;
  a=b;
  b=tmp;
#endif
}
inline int mpz_mod(mpz_t & r,const mpz_t & a,const mpz_t & b){ bf_rem(&r, &a, &b, BF_PREC_INF, BF_RNDZ,BF_DIVREM_EUCLIDIAN); }
inline int mpz_add(mpz_t & c,const mpz_t & a,const mpz_t & b){ return bf_add(&c,&a,&b,BF_PREC_INF, BF_RNDZ); }
inline void mpz_add_ui(mpz_t & c,const mpz_t & a,unsigned long B){ bf_add_si(&c,&a,B,BF_PREC_INF, BF_RNDZ); } // ASSUMES that B<1ULL<<63
inline int mpz_sub(mpz_t & c,const mpz_t & a,const mpz_t & b){ return bf_sub(&c,&a,&b,BF_PREC_INF, BF_RNDZ);  }
inline void mpz_sub_ui(mpz_t & c,const mpz_t & a,long B){ bf_add_si(&c,&a,-B,BF_PREC_INF, BF_RNDZ); } // ASSUMES that B<1ULL<<63
inline int mpz_mul(mpz_t & c,const mpz_t & a,const mpz_t & b){ return bf_mul(&c,&a,&b,BF_PREC_INF, BF_RNDZ); }
inline void mpz_mul_ui(mpz_t & c,const mpz_t & a,unsigned long B){ bf_mul_ui(&c,&a,B,BF_PREC_INF, BF_RNDZ); } 
inline void mpz_mul_si(mpz_t & c,const mpz_t & a,long B){ bf_mul_si(&c,&a,B,BF_PREC_INF, BF_RNDZ); } 
inline int mpz_mul_2exp(mpz_t & c,const mpz_t & a,long B){ if (&c!=&a) mpz_set(c,a); return bf_mul_2exp(&c,B,BF_PREC_INF, BF_RNDZ); }
inline void mpz_tdiv_q_2exp(mpz_t & c,const mpz_t & a,long B){ 
  if (&c!=&a) mpz_set(c,a); 
  bf_mul_2exp(&c,-B,BF_PREC_INF, BF_RNDZ); 
  bf_rint(&c,BF_RNDD);
}
inline void mpz_tdiv_r_2exp(mpz_t & d,const mpz_t & a,long B){ 
  mpz_t q; mpz_init(q); 
  mpz_tdiv_q_2exp(q,a,B); 
  mpz_mul_2exp(q,q,B); 
  mpz_sub(d,a,q); 
  mpz_clear(q);
}
inline int mpz_even_p (const mpz_t &p){int i; bf_get_int32(&i,&p,BF_GET_INT_MOD); return i%2==0;}
inline int mpz_odd_p (const mpz_t &p){int i; bf_get_int32(&i,&p,BF_GET_INT_MOD); return i%2;}
inline int mpz_pow_ui(mpz_t & c,const mpz_t & a,unsigned B){ return bf_pow_ui(&c,&a,B,BF_PREC_INF, BF_RNDZ); }
inline void mpz_ui_pow_ui(mpz_t & c,unsigned A,unsigned B){ mpz_t a ; mpz_init_set_ui(a,A); mpz_pow_ui(c,a,B); mpz_clear(a);}
inline int mpz_ior(mpz_t & c,const mpz_t & a,const mpz_t & b){ return bf_logic_or(&c,&a,&b); }
inline int mpz_xor(mpz_t & c,const mpz_t & a,const mpz_t & b){ return bf_logic_xor(&c,&a,&b); }
inline int mpz_and(mpz_t & c,const mpz_t & a,const mpz_t & b){ return bf_logic_and(&c,&a,&b); }
inline void mpz_addmul(mpz_t & c,const mpz_t & a,const mpz_t & b){ mpz_t ab; mpz_init(ab); mpz_mul(ab,a,b); mpz_add(c,c,ab); mpz_clear(ab);}
inline void mpz_addmul_ui(mpz_t & c,const mpz_t & a,unsigned int B){ mpz_t ab; mpz_init(ab); mpz_mul_ui(ab,a,B); mpz_add(c,c,ab); mpz_clear(ab); }
inline void mpz_submul(mpz_t & c,const mpz_t & a,const mpz_t & b){mpz_t ab; mpz_init(ab); mpz_mul(ab,a,b); mpz_sub(c,c,ab); mpz_clear(ab);} 
inline void mpz_submul_ui(mpz_t & c,const mpz_t & a,unsigned int B){ mpz_t ab; mpz_init(ab); mpz_mul_ui(ab,a,B); mpz_sub(c,c,ab); mpz_clear(ab); }
inline int mpz_fdiv_qr(mpz_t & c,mpz_t & d,const mpz_t & a,const mpz_t & b){ return bf_divrem(&c,&d,&a,&b,BF_PREC_INF,BF_RNDZ,BF_RNDD); }
inline int mpz_tdiv_qr(mpz_t & c,mpz_t & d,const mpz_t & a,const mpz_t & b){ return bf_divrem(&c,&d,&a,&b,BF_PREC_INF,BF_RNDZ,BF_DIVREM_EUCLIDIAN); }
inline int mpz_fdiv_r(mpz_t & c,const mpz_t & a,const mpz_t & b){ return bf_rem(&c,&a,&b,BF_PREC_INF,BF_RNDZ,BF_RNDD); }
inline int mpz_tdiv_r(mpz_t & c,const mpz_t & a,const mpz_t & b){ return bf_rem(&c,&a,&b,BF_PREC_INF,BF_RNDZ,BF_DIVREM_EUCLIDIAN);  }
inline void mpz_fdiv_r_ui(mpz_t & c,const mpz_t & a,unsigned B){ mpz_t b; mpz_init_set_ui(b,B); mpz_fdiv_r(c,a,b); mpz_clear(b); }
inline long mpz_fdiv_ui(const mpz_t & a,unsigned B){ 
  mpz_t b,r; 
  mpz_init_set_ui(b,B); mpz_init(r); 
  mpz_fdiv_r(r,a,b); 
  int res=mpz_get_ui(r); 
  mpz_clear(b); mpz_clear(r); 
  return res;
}
inline void mpz_tdiv_r_ui(mpz_t & c,const mpz_t & a,unsigned B){ mpz_t b; mpz_init_set_ui(b,B); mpz_tdiv_r(c,a,b); mpz_clear(b); }
inline long mpz_fdiv_qr_ui(mpz_t & c,mpz_t & d,const mpz_t & a,unsigned B){ mpz_t b; mpz_init_set_ui(b,B); mpz_fdiv_qr(c,d,a,b); mpz_clear(b); return mpz_get_ui(d);}
inline long mpz_tdiv_qr_ui(mpz_t & c,mpz_t & d,const mpz_t & a,unsigned B){ mpz_t b; mpz_init_set_ui(b,B); mpz_tdiv_qr(c,d,a,b); mpz_clear(b); return mpz_get_ui(d);}
inline void mpz_fdiv_q_ui(mpz_t & c,const mpz_t & a,unsigned B){ mpz_t b,r; mpz_init_set_ui(b,B); mpz_init(r); mpz_fdiv_qr(c,r,a,b); mpz_clear(b); mpz_clear(r);}
inline void mpz_tdiv_q_ui(mpz_t & c,const mpz_t & a,unsigned B){ mpz_t b,r; mpz_init_set_ui(b,B); mpz_init(r); mpz_tdiv_qr(c,r,a,b); mpz_clear(b); mpz_clear(r);}
inline void mpz_fdiv_q(mpz_t & c,const mpz_t & a,const mpz_t & b){ mpz_t r; mpz_init(r); mpz_fdiv_qr(c,r,a,b); mpz_clear(r); }
inline void mpz_tdiv_q(mpz_t & c,const mpz_t & a,const mpz_t & b){ mpz_t r; mpz_init(r); mpz_tdiv_qr(c,r,a,b); mpz_clear(r); }
inline void mpz_divexact(mpz_t & c,const mpz_t & a,const mpz_t &b){  mpz_fdiv_q(c,a,b);  }
inline void mpz_divexact_ui(mpz_t & c,const mpz_t & a,unsigned B){ mpz_t b,r; mpz_init_set_ui(b,B);  mpz_fdiv_q(c,a,b); mpz_clear(b); }
inline int mpz_divisible_ui_p(const mpz_t &c,unsigned long a){ return mpz_fdiv_ui(c,a)==0;}
inline int mpz_powm(mpz_t & d,const mpz_t &  a,const mpz_t & b_,const mpz_t & c){
  mpz_t a2i; mpz_init(a2i); mpz_tdiv_r(a2i,a,c);
  mpz_t b,r; mpz_init_set(b,b_); mpz_init(r);
  mpz_set_ui(d,1);
  while (!bf_is_zero(&b)){
    if (mpz_odd_p(b)){
      mpz_mul(d,d,a2i);
      mpz_tdiv_r(r,d,c);
      mpz_swap(r,d);
      mpz_sub_ui(b,b,1);
    }
    bf_mul_2exp(&b,-1,BF_PREC_INF, BF_RNDZ);
    mpz_mul(r,a2i,a2i);
    mpz_tdiv_r(a2i,r,c);
  }
  mpz_clear(b); mpz_clear(a2i); mpz_clear(r);
}
inline void mpz_powm_ui(mpz_t & d,const mpz_t &  a,unsigned int B,const mpz_t & c){ mpz_t b; mpz_init_set_ui(b,B); mpz_powm(d,a,b,c); mpz_clear(b);}
inline int mpz_sqrt(mpz_t &  a,const mpz_t & b){ 
  mpz_t r; mpz_init(r);
  int res=bf_sqrtrem(&a,&r,&b);
  mpz_clear(r);
  return res;
}
inline int mpz_sqr(mpz_t &  a,const mpz_t & b){ return bf_mul(&a,&b,&b,BF_PREC_INF, BF_RNDZ);}
inline void mpz_neg(mpz_t &  a,const mpz_t & b){ mpz_set(a, b); bf_neg(&a);}
inline int mpz_abs(mpz_t &  a,const mpz_t & b){ 
  mpz_set(a,b);
  if (a.sign==1)
    bf_neg(&a);
  return 0;
}
inline int mpz_gcd(mpz_t & z,const mpz_t & A,const mpz_t & B){ 
  mpz_t a,b;
  mpz_init_set(a,A);
  mpz_init_set(b,B);
  while (!bf_is_zero(&b)){
    mpz_tdiv_r(z,a,b);
    mpz_swap(a,b);
    mpz_swap(b,z);
  }
  mpz_abs(z,a);
  mpz_clear(a);
  mpz_clear(b);
  return 0;
}
inline int gcdint(int a,int b){
  int r;
  while (b){
    r=a%b;
    a=b;
    b=r;
  }
  return a<0?-a:a;
}
inline int mpz_lcm(mpz_t & c,const mpz_t & a,const mpz_t & b){ 
  mpz_t d; mpz_init(d);
  mpz_gcd(d,a,b);
  mpz_tdiv_q(c,a,d);
  mpz_mul(c,c,d);
  mpz_clear(d);
  return 0;
}
inline int mpz_cmp(const mpz_t &  a,const mpz_t & b){ 
  return bf_cmp(&a,&b);
}
inline int mpz_cmp_ui(const mpz_t &  a,unsigned int B){ 
  mpz_t b; int res; 
  mpz_init_set_ui(b,B); 
  res=mpz_cmp(a,b); 
  mpz_clear(b); 
  return res;
}
inline int mpz_cmp_si(const mpz_t &  a,int B){ 
  mpz_t b; int res; mpz_init(b); 
  mpz_set_si(b,B); 
  res=mpz_cmp(a,b); 
  mpz_clear(b); 
  return res;
}
inline int mpz_gcd_ui(mpz_t * c,const mpz_t & a,unsigned B){ 
  mpz_t b,r; mpz_init_set_ui(b,B);  mpz_init(r);
  mpz_mod(r,a,b);
  int res=mpz_get_ui(r); 
  mpz_clear(b); mpz_clear(r);
  return gcdint(B,res);
}
inline double mpz_get_d(const mpz_t & z){ 
  double d;
  bf_get_float64(&z,&d,BF_RNDD);
  return d;
}
inline void mpz_set_d(mpz_t & z,double d){ 
  bf_set_float64(&z,d);
}
inline int mpz_sizeinbase(const mpz_t & a,int radix){ 
  if (radix==2)
    return a.expn;
  return ceil(a.expn*log(2.0)/log((double)radix));
}
inline int mpz_set_str(mpz_t &  z,const char * s,int base){
  slimb_t exponent;
  const char * next=0;
  bf_atof2(&z,&exponent,s,&next,base,BF_PREC_INF,BF_RNDZ);
}
inline int mpz_get_str(char * s,int base,const mpz_t &  z){
  char * ptr=bf_ftoa(NULL, &z, base, 0,BF_RNDZ | BF_FTOA_FORMAT_FRAC);
  strcpy(s,ptr);
  free(ptr);
}
inline void mpz_fac_ui(mpz_t & z,unsigned int i){
  mpz_set_ui(z,1);
  for (unsigned long int j=2;j<=i;j++){
    ctrl_c_interrupted(1);
    mpz_mul_ui(z,z,j);
  }
}
inline int mpz_gcdext(mpz_t & d,mpz_t & u, mpz_t & v,const mpz_t & a,const mpz_t & b){ 
  mpz_t q,r0,r1,r2,u1,u2,v1,v2;
  // mpz_t r3,u3,v3;
  mpz_init_set(r1,a); mpz_init_set(r2,b);
  mpz_init_set_ui(u1,1); mpz_init_set_ui(u2,0);
  mpz_init_set_ui(v1,0); mpz_init_set_ui(v2,1);
  mpz_init(q); mpz_init(r0);
  // mpz_init(r3); mpz_init(u3); mpz_init(v3);
  while (mpz_cmp_si(r2,0)){
    // CERR << "iegcd " << gen(r1) << " " << gen(r2) << '\n';
    mpz_set(r0,r1);
    mpz_tdiv_qr(q,r1,r0,r2);
    mpz_swap(r1,r2);
    mpz_submul(u1,q,u2);
    mpz_swap(u1,u2);
    mpz_submul(v1,q,v2);
    mpz_swap(v1,v2);
  }
  if (mpz_cmp_si(r1,0)<0){
    mpz_neg(r1,r1); mpz_neg(u1,u1); mpz_neg(v1,v1);
  }
  mpz_swap(d,r1); mpz_swap(u,u1); mpz_swap(v,v1);
  mpz_clear(q); mpz_clear(r1); mpz_clear(r2); mpz_clear(r0);
  mpz_clear(u1); mpz_clear(u2); 
  mpz_clear(v1); mpz_clear(v2); 
}
// might use int mp_recip(bf_context_t *s, limb_t *tabr, const limb_t *taba, limb_t n);
inline int mpz_invert(mpz_t & ainv,const mpz_t & a,const mpz_t & m){
  mpz_t d,v;
  mpz_init(d); mpz_init(v);
  mpz_gcdext(d,ainv,v,a,m);
  mpz_clear(v);
  int ok=mpz_cmp_si(d,1)==0;
  mpz_clear(d);
  return ok;
}


inline int mpz_probab_prime_p(const mpz_t & a,int){
  return INT32_MIN; // not implemented 
}
inline int mpz_legendre(const mpz_t & a,const mpz_t & n){
  return INT32_MIN; // not implemented 
}
inline int mpz_jacobi(const mpz_t & a,const mpz_t & n){
  return INT32_MIN; // not implemented 
}
inline bool mpz_perfect_square_p(const mpz_t & a){
  if (a.sign==1) return false;
  mpz_t q,r; mpz_init(q); mpz_init(r);
  bf_sqrtrem(&q,&r,&a);
  bool b=bf_is_zero(&r);
  mpz_clear(q); mpz_clear(r);
  return b;
}
inline int mpz_popcount(const mpz_t & a){
  return -1; // FIXME, should be numbers of bits set to 1
}
inline int mpz_hamdist(const mpz_t & a,const mpz_t & b){
  mpz_t c;
  mpz_init(c);
  mpz_xor(c,a,b);
  int res=mpz_popcount(c);
  mpz_clear(c);
  return res;
}

#define LONGFLOAT_DOUBLE
typedef double mpf_t;
#define mpf_clear(x) 
#define mpf_init(x) 
#define mpf_init_set(x,y) (x=y)
#define mpf_init_set_d(x,y) (x=y)
#define mpf_init_set_si(x,y) (x=y)
#define mpf_set_z(x,y) 
#define mpf_set(x,y) (x=y)
inline int mpf_set_str(double & x,const char * s,int base){ if (base!=10) return 1; x=strtod(s,0); return 0; } 
#define mpf_get_d(x) (x)
#define mpf_add(x,y,z) (x=y+z)
#define mpf_sub(x,y,z) (x=y-z)
#define mpf_mul(x,y,z) (x=y*z)
#define mpf_neg(x,y) (x=-y)
#define mpf_ui_div(z,x,y) (z=x/y)
#define mpf_sqrt(x,y) (x=sqrt(y))
#define mpf_sgn(x) (x>0?1:-1)

#endif
