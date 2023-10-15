// -*- mode:C++ ; compile-command: "g++ -I.. -I../include -g -c -Wall -DHAVE_CONFIG_H -DIN_GIAC csturm.cc" -*-
#include "giacPCH.h"

/*
 *  Copyright (C) 2000,14 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
using namespace std;
#include <cmath>
#include <stdexcept>
#include <map>
#include "gen.h"
#include "csturm.h"
#include "vecteur.h"
#include "modpoly.h"
#include "unary.h"
#include "symbolic.h"
#include "usual.h"
#include "sym2poly.h"
#include "solve.h"
#include "prog.h"
#include "subst.h"
#include "permu.h"
#include "series.h"
#include "alg_ext.h"
#include "ti89.h"
#include "plot.h"
#include "modfactor.h"
#include"giacintl.h"
#include <fstream>
#define MPFI_CERT
#ifdef HAVE_LIBMPS
#include <mps/mps.h>
#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <string.h>
#endif

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  // compute Sturm sequence of r0 and r1,
  // returns gcd (without content)
  // and compute list of quotients, coeffP, coeffR
  // such that coeffR*r_(k+2) = Q_k*r_(k+1) - coeffP_k*r_k
  gen csturm_seq(modpoly & r0,modpoly & r1,vecteur & listquo,vecteur & coeffP, vecteur & coeffR,GIAC_CONTEXT){
    listquo.clear();
    coeffP.clear();
    coeffR.clear();
    if (r0.empty())
      return r1;
    if (r1.empty())
      return r0;
    gen tmp;
    lcmdeno(r0,tmp,contextptr);
    if (ck_is_positive(-tmp,contextptr))
      r0=-r0;
    r0=r0/abs(lgcd(r0),contextptr);
    lcmdeno(r1,tmp,contextptr);
    if (ck_is_positive(-tmp,contextptr))
      r1=-r1;
    r1=r1/abs(lgcd(r0),contextptr);
    // set auxiliary constants g and h to 1
    gen g(1),h(1);
    modpoly a(r0),b(r1),quo,r;
    gen b0(1);
    for (int loop_counter=0;;++loop_counter){
      int m=int(a.size())-1;
      int n=int(b.size())-1;
      int ddeg=m-n; // should be 1 generically
      if (!n) { // if b is constant, gcd=1
	return 1;
      }
      b0=b.front();
      if (b.front().type==_VECT) {
	// ddeg should be even if b0 is a _POLY1
	if (ddeg%2==0)
	  *logptr(contextptr) << gettext("Singular parametric Sturm sequence ") << a << "/" << b << '\n';
      }
      else
	b0=abs(b.front(),contextptr); 
      coeffP.push_back(pow(b0,ddeg+1));
      DivRem(coeffP.back()*a,b,0,quo,r);
      listquo.push_back(quo);
      coeffR.push_back(g*pow(h,ddeg));
      if (r.empty()){
	return b/abs(lgcd(b),contextptr);
      }
      // remainder is non 0, loop continue: a <- b
      a=b;
      // now divides r by g*h^(m-n) and change sign, result is the new b
      b= -r/coeffR.back();
      g=b0;
      h=pow(b0,ddeg)/pow(h,ddeg-1);
    } // end while loop
  }

  static gen csturm_horner(const modpoly & p,const gen & a){
    if (p.size()==1 && p.front().type==_POLY && p.front()._POLYptr->dim==1){
      // patch for "sparse modpoly"
      vector< monomial<gen> >::const_iterator it=p.front()._POLYptr->coord.begin(),itend=p.front()._POLYptr->coord.end();
      gen res=0,anum=a,aden=1,den=1;
      int oldpui=0,pui;
      if (a.type==_FRAC){
	anum=a._FRACptr->num;
	aden=a._FRACptr->den;
      }
      for (;it!=itend;++it){
	pui=it->index.front();
	if (oldpui){
	  res = res * pow(anum,oldpui-pui,context0);
	  den = den * pow(aden,oldpui-pui,context0);
	}
	res += it->value*den;
	oldpui=pui;
      }
      if (oldpui){
	res = res *pow(a,oldpui,context0);
	den = den *pow(a,oldpui,context0);
      }
      return res/den;
    }
    else
      return horner(p,a);
  }

  static int csturm_vertex_ab(const modpoly & r0,const modpoly & r1,const vecteur & listquo,const vecteur & coeffP, const vecteur & coeffR,const gen & a,int start,GIAC_CONTEXT){
    int n=int(listquo.size()),j,k,res=0;
    vecteur R(n+2);
    R[0]=csturm_horner(r0,a);
    R[1]=csturm_horner(r1,a);
    for (j=0;j<n;j++)
      R[j+2]=(-coeffP[j]*R[j]+R[j+1]*horner(listquo[j],a))/coeffR[j];
    // signes
    for (j=start;j<n+2;j++){
      if (R[j]!=0) break;
    }
    for (k=j+1;k<n+2;k++){
      if (is_zero(R[k])) continue;
      if (fastsign(R[j],context0)*fastsign(R[k],context0)<0 
	  // is_positive(-R[j]*R[k],contextptr)
	  ){
	res++;
	j=k;
      }
    }
    return res;
  }

#if 0
  // compute vertex index at a (==0 unless s(a)==0) 
  static int csturm_vertex_a(const modpoly & s,const modpoly & r,const gen & a,int direction,GIAC_CONTEXT){
    int j;
    modpoly s1,s2;
    gen sa=horner(s,a,0,s1);
    if (!is_zero(sa)) return 0;
    for (j=1;;j++){
      sa=horner(s1,a,0,s2);
      if (!is_zero(sa))
	break;
      s1=s2;
    }
    if (direction==1) j=0;
    gen tmp=sign(sa,contextptr)*sign(horner(r,a),contextptr);
    return tmp.val*((j%2)?-1:1);
  }
#endif

  void change_scale(modpoly & p,const gen & l,longlong lb){
    //vecteur chk(p),save(p);
    int n=int(p.size());
    if (lb!=(1<<31)){
      if (lb==0) return;
      mpz_t pi;
      mpz_init(pi);
      for (int i=n-2;i>=0;--i){
	if (p[i].type==_INT_)
	  mpz_set_si(pi,p[i].val);
	else
	  mpz_set(pi,*p[i]._ZINTptr);
	if (lb<0)
	  mpz_tdiv_q_2exp(pi,pi,(-lb)*(n-1-i)); // was mpz_fdiv_q_2exp
	else
	  mpz_mul_2exp(pi,pi,lb*(n-1-i));
	p[i] = pi;
      }
      mpz_clear(pi);
      //chk=p; p=save;
      return;
    }
    gen lton(l);
    for (int i=n-2;i>=0;--i){
      p[i] = p[i] * lton;
      lton = lton * l;
    }
    //if (p!=chk) CERR << "bug\n";
  }

  void back_change_scale(modpoly & p,const gen & l,longlong lb){
    if (lb!=(1<<31)){
      change_scale(p,l,-lb);
      return;
    }
    int n=int(p.size());
    gen lton(l);
    for (int i=n-2;i>=0;--i){
      p[i] = p[i] / lton;
      lton = lton * l;
    }
  }

  // p(x)->p(a*x+b)
  modpoly linear_changevar(const modpoly & p,const gen & a,const gen & b){
    modpoly res(taylor(p,b));
    change_scale(res,a);
    return res;
  }

  // p(a*x+b)->p(x)
  // t=a*x+b -> pgcd(t)=g((t-b)/a)
  modpoly inv_linear_changevar(const modpoly & p,const gen & a,const gen & b){
    gen A=inv(a,context0); 
    gen B=-b/a;
    modpoly res(taylor(p,B));
    change_scale(res,A);
    return res;
  }

  // Find roots of R, S=R' at precision eps, returns number of roots
  // if eps==0 does not compute intervals for roots
  static int csturm_realroots(const modpoly & S,const modpoly & R,const vecteur & listquo,const vecteur & coeffP, const vecteur & coeffR,const gen & a,const gen & b,const gen & t0, const gen & t1,vecteur & realroots,double eps,GIAC_CONTEXT){
    if (is_inf(t0)) // replace with max(R)
      return csturm_realroots(S,R,listquo,coeffP,coeffR,a,b,-linfnorm(R,contextptr),t1,realroots,eps,contextptr);
    if (is_inf(t1)) // replace with max(R)
      return csturm_realroots(S,R,listquo,coeffP,coeffR,a,b,t0,linfnorm(R,contextptr),realroots,eps,contextptr);    
    int n1=csturm_vertex_ab(S,R,listquo,coeffP,coeffR,t0,1,contextptr);
    int n2=csturm_vertex_ab(S,R,listquo,coeffP,coeffR,t1,1,contextptr);
    int n=(n2-n1);
    if (!eps || !n)
      return n;
    /* disabled localization of roots, do isolation of roots instead
       if (is_strictly_greater(eps,(t1-t0)*abs(b,contextptr),contextptr)){
       realroots.push_back(makevecteur(makevecteur(a+t0*b,a+t1*b),n));
       return n;
       }
    */
    if (n==1){
      gen T0=t0,T1=t1,T2;
      int s0=fastsign(csturm_horner(R,T0),contextptr);
      // int s1=fastsign(csturm_horner(R,T1),contextptr);
      int s2;
      gen delta=evalf_double(log((T1-T0)*abs(b,contextptr)/eps,contextptr)/log(2.,contextptr),1,contextptr);
      if (delta.type!=_DOUBLE_){
	realroots=vecteur(1,gentypeerr(contextptr));
	return -2;
      }
      int nstep=int(delta._DOUBLE_val+1);
      for (int step=0;step<nstep;++step){
	T2=(T0+T1)/2;
	s2=fastsign(csturm_horner(R,T2),contextptr);
	if (!s2){
	  realroots.push_back(makevecteur(a+T2*b,n));
	  return n;
	}
	if (s2==s0)
	  T0=T2;
	else
	  T1=T2;
      }
      realroots.push_back(makevecteur(makevecteur(a+T0*b,a+T1*b),n));
      return n;
    }
    gen T0=t0,T1=t1,t01;
    for (;;){
      t01=(T0+T1)/2;
      int n01=csturm_vertex_ab(S,R,listquo,coeffP,coeffR,t01,1,contextptr);
      if (n01!=n1 && n01!=n2)
	break;
      if (n01==n1)
	T0=t01;
      else
	T1=t01;
    }
    if (csturm_realroots(S,R,listquo,coeffP,coeffR,a,b,T0,t01,realroots,eps,contextptr)==-2)
      return -2;
    if (csturm_realroots(S,R,listquo,coeffP,coeffR,a,b,t01,T1,realroots,eps,contextptr)==-2)
      return -2;
    return n;
  }

  // Find complex sturm sequence for P(a+(b-a)*x)
  // If P is "pseudo"-real on [a,b] and eps>0 put roots in [a,b]
  // at precision eps inside realroots
  // returns a,b,R,S,g,listquo,coeffP,coeffR,typeseq
  // with typeseq=0 (complex Sturm) or 1 (limit)
  // If b-a is real and horiz_sturm is not empty, it tries to replace
  // the variable by im(a)*i in horiz_sturm and if no quotient in horiz_sturm
  // has a leading 0 coefficient, 
  // it returns im(a)*i,im(a)*i+1,R,S,g,listquo,coeffP,coeffR,typeseq
  // If b-a is pure imaginary and vert_sturm is not empty, it tries to replace
  // the variable by re(a) and returns re(a),re(a)+i,R,S,g,listquo,coeffP,coeffR,typeseq
  static vecteur csturm_segment_seq(const modpoly & P,const gen & a,const gen & b,vecteur & realroots,double eps,vecteur & horiz_sturm,vecteur & vert_sturm,GIAC_CONTEXT){
    // try with horiz_sturm and vert_sturm
    gen ab(b-a);
    /* // Optimization fails for sturmab(x^3-1,-1-i,1+i)
       if (is_zero(re(ab,contextptr))){ // b-a is pure imaginary
       if (vert_sturm.empty()){
       gen A=gen(makevecteur(1,0),_POLY1__VECT);
       vert_sturm.push_back(undef);
       vecteur tmp;
       vert_sturm=csturm_segment_seq(P,A,A+cst_i,tmp,eps,horiz_sturm,vert_sturm,contextptr);
       if (is_undef(vert_sturm))
       return vert_sturm;
       }
       if (vert_sturm.size()==9){
       vecteur res(vert_sturm);
       gen A=re(a,contextptr);
       res[0]=A; // re(a)
       res[1]=A+cst_i; // re(a)+i
       res[2]=apply1st(res[2],A,horner); // R 
       res[3]=apply1st(res[3],A,horner); // S
       res[4]=horner(res[4],A); // g
       vecteur tmp(*res[5]._VECTptr);
       int tmps=tmp.size();
       for (int j=0;j<tmps;++j)
       tmp[j]=apply1st(tmp[j],A,horner); 
       res[5]=tmp; // listquo
       res[6]=apply1st(res[6],A,horner); // coeffP
       res[7]=apply1st(res[7],A,horner); // coeffR
       if (res[6].type==_VECT && !equalposcomp(*res[6]._VECTptr,0))
       return res;
       else
       CERR << "list of quotients is not regular" << '\n';
       }
       }
    */
    modpoly Q(taylor(P,a));
    change_scale(Q,b-a);
    // now x is in 0..1
    gen gtmp=apply(Q,re,contextptr);
    if (gtmp.type!=_VECT)
      return vecteur(1,gensizeerr(contextptr));
    modpoly R=trim(*gtmp._VECTptr,0);
    gtmp=apply(Q,im,contextptr);
    if (gtmp.type!=_VECT)
      return vecteur(1,gensizeerr(contextptr));
    modpoly S=trim(*gtmp._VECTptr,0);
    modpoly listquo,coeffP,coeffR;
    gen g=csturm_seq(S,R,listquo,coeffP,coeffR,contextptr);
    int typeseq=-1;
    if (debug_infolevel)
      *logptr(contextptr)  << "segment " << a << ".." << b << ", im/re:" << S << "|" << R << ", gcd:" << g << '\n';
    if (g.type==_VECT && g._VECTptr->size()==P.size()){
      // if g==P (up to a constant), use real Sturm sequences
      if (debug_infolevel)
	*logptr(contextptr)  << "Real-kind roots: " << g << '\n';
      R=*g._VECTptr;
      S=derivative(R);
      g=csturm_seq(S,R,listquo,coeffP,coeffR,contextptr);
      typeseq=csturm_realroots(S,R,listquo,coeffP,coeffR,a,b-a,0,1,realroots,eps,contextptr);
      if (typeseq==-2)
	return realroots;
    }
    if (g.type==_VECT)
      g=inv_linear_changevar(*g._VECTptr,b-a,a);
    vecteur res= makevecteur(a,b,R,S,g,listquo,coeffP,coeffR,typeseq);
    return res;
  }

  // index for segment a,b (2* number of roots when summed over a closed
  // polygon). Note that if S=ImP along the segment is 0 we remove
  // the roots on [a,b] using real Sturm sequences
  // If S=0 at a or b, this is simply ignored
  // Indeed the computed index is then the same as if S was of the
  // sign of R, and since R!=0 if S is 0 this is a property of the vertex
  // not of the segment (note that contrary to counting real roots
  // on an interval, S can vanish as many times as long as R keeps
  // the same sign, without modifying the algebraic number of Im=0
  // cuts if S has the same sign on both end)
  static int csturm_segment(const vecteur & seq,const gen & a,const gen & b,GIAC_CONTEXT){
    gen t0,t1;
    if (seq.size()!=9)
      return -(RAND_MAX/2);
    gen aseq=seq[0];
    gen bseq=seq[1];
    gen directeur=(b-a)/(bseq-aseq);
    t0=(a-aseq)/(bseq-aseq);
    if ( !is_zero(im(directeur,contextptr)) || !is_zero(im(t0,contextptr)) )
      return -(RAND_MAX/2);
    t0=re(t0,contextptr); // t0=normal(t0);
    t1=re(t0+directeur,contextptr); // t1=normal(t0+directeur);
    int signe=1;
    if (is_strictly_greater(t0,t1,contextptr)){
      signe=-1;
      swapgen(t0,t1);
    } 
    const modpoly & R=*seq[2]._VECTptr;
    const modpoly & S=*seq[3]._VECTptr;
    gen g=seq[4];
    const modpoly & listquo=*seq[5]._VECTptr;
    const modpoly & coeffP=*seq[6]._VECTptr;
    const modpoly & coeffR=*seq[7]._VECTptr;
    int debut=(seq[8].val==-1)?0:1;
    int tmp = csturm_vertex_ab(S,R,listquo,coeffP,coeffR,t0,debut,contextptr);
    int res = tmp;
    tmp = csturm_vertex_ab(S,R,listquo,coeffP,coeffR,t1,debut,contextptr);
    res -= tmp;
    // tmp = (-csturm_vertex_a(S,R,t0,1,contextptr)+csturm_vertex_a(S,R,t1,-1,contextptr));
    // res += tmp;
    res=(debut?1:signe)*res;
    if (debug_infolevel)
      *logptr(contextptr)  << "segment " << a << ".." << b << " index contribution " << res << '\n';
    return res;
  }

  static bool csturm_square_seq(const modpoly & P,const gen & a0,const gen & b0,const gen & a1,const gen & b1,gen & pgcd,vecteur & realroots,double eps,vecteur & seq1,vecteur & seq2,vecteur & seq3,vecteur & seq4,vecteur & horiz_sturm,vecteur & vert_sturm,GIAC_CONTEXT){
    gen A=a0+cst_i*b0,B=a1+cst_i*b0;
    vecteur rroots;
    seq1=csturm_segment_seq(P,A,B,rroots,eps,horiz_sturm,vert_sturm,contextptr);
    if (is_undef(seq1))
      return false;
    pgcd=seq1[4];
    if (!is_one(pgcd)){
      return false;
    }
    A=a1+cst_i*b0; B=a1+cst_i*b1;
    seq2=csturm_segment_seq(P,A,B,rroots,eps,horiz_sturm,vert_sturm,contextptr);
    if (is_undef(seq2))
      return false;
    pgcd=seq2[4];
    if (!is_one(pgcd)){
      return false;
    }
    A=a1+cst_i*b1; B=a0+cst_i*b1;
    seq3=csturm_segment_seq(P,A,B,rroots,eps,horiz_sturm,vert_sturm,contextptr);
    if (is_undef(seq3))
      return false;
    pgcd=seq3[4];
    if (!is_one(pgcd)){
      return false;
    }
    A=a0+cst_i*b1; B=a0+cst_i*b0;
    seq4=csturm_segment_seq(P,A,B,rroots,eps,horiz_sturm,vert_sturm,contextptr);
    if (is_undef(seq4))
      return false;
    pgcd=seq4[4];
    if (!is_one(pgcd)){
      return false;
    }
    realroots=mergevecteur(realroots,rroots);
    return true;
  }

  // find 2* number of roots of P inside the square of vertex of affixes a,b
  // roots on the square are not counted. P must not vanish at the vertices.
  // The complex Sturm sequences must be known
  // returns -1 on error
  static int csturm_square(const modpoly & P,const gen & a0,const gen & b0,const gen & a1,const gen & b1,const vecteur & seq1,const vecteur & seq2,const vecteur & seq3,const vecteur & seq4,GIAC_CONTEXT){
    int ind,tmp;
    ind = 0;
    gen A=a0+cst_i*b0,B=a1+cst_i*b0;
    tmp = csturm_segment(seq1,A,B,contextptr);
    if (tmp==-(RAND_MAX/2))
      return -1;
    ind += tmp;
    A=a1+cst_i*b0; B=a1+cst_i*b1;
    tmp = csturm_segment(seq2,A,B,contextptr);
    if (tmp==-(RAND_MAX/2))
      return -1;
    ind += tmp;
    A=a1+cst_i*b1; B=a0+cst_i*b1;
    tmp = csturm_segment(seq3,A,B,contextptr);
    if (tmp==-(RAND_MAX/2))
      return -1;
    ind += tmp;
    A=a0+cst_i*b1; B=a0+cst_i*b0;
    tmp = csturm_segment(seq4,A,B,contextptr);
    if (tmp==-(RAND_MAX/2))
      return -1;
    ind += tmp;
    return ind;
  }

  static void csturm_normalize(modpoly & p,const gen & a0,const gen & b0,const gen & a1,const gen & b1,vecteur & roots){
    int n=int(p.size())-1;
    // Make sure that x->a+i*x does not return a multiple 
    // of a real polynomial with the multiple non real
    // If degree of p is even the multiple will be a real (because of lcoeff)
    if (n%2){
      // If degree is odd then look at q=p(x-a_{n-1}/n*an)
      // it has the same property
      // if its cst coeff is zero remove
      gen an=p.front();
      gen b=p[1];
      gen shift=-b/n/an;
      modpoly q(taylor(p,shift));
      gen q0;
      // remove valuation
      int qs=int(q.size());
      int n1=0;
      for (;qs>0;--qs,++n1){
	if (!is_zero(q0=q[qs-1]))
	  break;
      }
      if (is_zero(re(q0,context0))){
	q=cst_i*q;
	p=cst_i*p;
      }
      if (n1){
	q=modpoly(q.begin(),q.begin()+qs);
	gen a=re(shift,context0),b=im(shift,context0);
	if (is_greater(a,a0,context0) && is_greater(b,b0,context0) && is_greater(a1,a,context0) && is_greater(b1,b,context0))
	  roots.push_back(makevecteur(shift,n1));
	p=taylor(q,-shift);
      }
    }
  }

  void ab2a0b0a1b1(const gen & a,const gen & b,gen & a0,gen & b0,gen & a1,gen & b1,GIAC_CONTEXT){
    a0=re(a,contextptr); b0=im(a,contextptr);
    a1=re(b,contextptr); b1=im(b,contextptr);
    if (ck_is_greater(a0,a1,contextptr)) swapgen(a0,a1);
    if (ck_is_greater(b0,b1,contextptr)) swapgen(b0,b1);
  }

  // find 2* number of roots of P inside the square of vertex of affixes a,b
  // excluding those on the square
  // returns -1 on error
  int csturm_square(const gen & p,const gen & a,const gen & b,gen& pgcd,GIAC_CONTEXT){
    if (p.type==_POLY){
      int res=0;
      factorization f(sqff(*p._POLYptr));
      factorization::const_iterator it=f.begin(),itend=f.end();
      for (;it!=itend;++it){
	int tmp=csturm_square(polynome2poly1(it->fact),a,b,pgcd,contextptr);
	if (tmp==-1)
	  return -1;
	res += it->mult*tmp;
      }
      return res;
    }
    if (p.type!=_VECT)
      return 0;
    modpoly P=*p._VECTptr;
    vecteur realroots;
    gen a0,b0,a1,b1;
    ab2a0b0a1b1(a,b,a0,b0,a1,b1,contextptr);
    csturm_normalize(P,a0,b0,a1,b1,realroots);
    int evident=0;
    if (!realroots.empty()){
      gen r=realroots.front();
      if (r.type==_VECT && r._VECTptr->size()==2)
	r=r._VECTptr->front();
      gen rx=re(r,contextptr),ry=im(r,contextptr);
      if ( ( is_zero(ry) && (rx==a0 || rx==a1) ) ||
	   ( is_zero(rx) && (ry==b0 || ry==b1) ) )
	;
      else
	evident=1;
    }
    if (P.size()<2)
      return evident;
    vecteur seq1,seq2,seq3,seq4,horiz_seq,vert_seq;    
    if (!csturm_square_seq(P,a0,b0,a1,b1,pgcd,realroots,0.0,seq1,seq2,seq3,seq4,horiz_seq,vert_seq,contextptr)){
      if (pgcd.type!=_VECT)	      
	return -1;
      modpoly g=(*pgcd._VECTptr)/pgcd[0];
      // true factorization found, restart with each factor
      modpoly p1=P/g;
      int n1=csturm_square(p1,a,b,pgcd,contextptr);
      if (n1==-1)
	return -1;
      int n2=csturm_square(g,a,b,pgcd,contextptr);
      if (n2==-1)
	return -1;
      return evident+n1+n2;
    }
    int tmp=csturm_square(P,a0,b0,a1,b1,seq1,seq2,seq3,seq4,contextptr);
    if (tmp==-1)
      return tmp;
    return evident+tmp;
  }

  static void complex_roots(const modpoly & P,const gen & a0,const gen & b0,const gen & a1,const gen & b1,vecteur & realroots,vecteur & complexroots,double eps);

  static bool complex_roots_split(const modpoly & P,const gen & pgcd,const gen & a0,const gen & b0,const gen & a1,const gen & b1,vecteur & realroots,vecteur & complexroots,double eps){
    if (pgcd.type!=_VECT)
      return false;
    modpoly g=(*pgcd._VECTptr)/pgcd[0];
    // true factorization found, restart with each factor
    modpoly p1=P/g;
    csturm_normalize(p1,a0,b0,a1,b1,realroots);
    csturm_normalize(g,a0,b0,a1,b1,realroots);
    complex_roots(p1,a0,b0,a1,b1,realroots,complexroots,eps);
    complex_roots(g,a0,b0,a1,b1,realroots,complexroots,eps);
    return true;
  }

#if 0
  // check that arg is >=pi/8 (assumes im(g)>=0)
  static bool arg_geq_pi_8(const gen & g){
    gen gr=re(g,context0),gi=im(g,context0);
    if (is_positive(-gr,context0))
      return true;
    // ? gi/gr>=sqrt(2)-1
    gen r=gi/gr+1;
    if (is_positive(r*r-2,context0))
      return true;
    return false;
  }

  // is im(b/a)>=0, tested without quotient
  static bool arg_in_0_pi(const gen & a,const gen & b){
    gen A(a),B(b);
    if (A.type==_FRAC && is_integer(A._FRACptr->den) && is_positive(A._FRACptr->den,context0))
      A=A._FRACptr->num;
    if (B.type==_FRAC && is_integer(B._FRACptr->den) && is_positive(B._FRACptr->den,context0))
      B=B._FRACptr->num;
    gen c=re(A,context0)*im(B,context0)+re(B,context0)*im(A,context0);
    return is_positive(c,context0);
  }

  static gen hornerarg(const modpoly & p,const gen & x){
    if (p.empty())
      return 0;
    if (x.type!=_FRAC || !is_integer(x._FRACptr->den))
      return horner(p,x);
    fraction & f =*x._FRACptr;
    gen num=f.num,den=f.den,d=den;
    if (is_positive(-f.den,context0)){
      num=-num; den=-den; d=den;
    }
    modpoly::const_iterator it=p.begin(),itend=p.end();
    gen res(*it);
    ++it;
    if (it==itend)
      return res;
    for (;;){
      res=res*num+(*it)*d;
      ++it;
      if (it==itend)
	break;
      d=d*den;   
    }
    return res;
  }  

  // Find one complex root inside a0,b0->a1,b1, return false if not found
  static bool complex_1root(const modpoly & P,const gen & a0,const gen & b0,const gen & a1,const gen & b1,vecteur & complexroots,double eps){
    return false; // disabled since it is not faster!!
    int step,nstep =int(evalf_double(log(max(a1-a0,b1-b0,context0)/eps,context0)/log(2.0,context0),1,context0)._DOUBLE_val+0.5);
    if (nstep<4)
      return false;
    // First compute P at the 4 vertex and check whether P[vertex_n+1]/P[vertex_n] is in C^+
    gen P0=hornerarg(P,a0+cst_i*b0),P2=hornerarg(P,a1+cst_i*b0),
      P4=hornerarg(P,a1+cst_i*b1),P6=hornerarg(P,a0+cst_i*b1);
    if (!(arg_in_0_pi(P0,P2) && arg_in_0_pi(P2,P4) && arg_in_0_pi(P4,P6) && arg_in_0_pi(P6,P0)))
      return false;
    gen A0(a0),A2(a1),B0(b0),B2(b1),A1,B1;
    for (step=0;step<2*nstep;step++){
      A1=(A0+A2)/2;
      B1=(B0+B2)/2;
      gen P1=hornerarg(P,A1+cst_i*B0),P7=hornerarg(P,A0+cst_i*B1),P8=hornerarg(P,A1+cst_i*B1),P3,P5;
      bool found=false;
      /*
        P6(A0,B2) - P5(A1,B2) - P4(A2,B2)  
        |            |           |      
        P7(A0,B1) - P8(A1,B1) - P3(A2,B1) 
        |            |            |      
        P0(A0,B0) - P1(A1,B0) - P2(A2,B0)  
      */
      // ? P0, P1, P8, P7
      if (arg_in_0_pi(P0,P1) && arg_in_0_pi(P1,P8) && arg_in_0_pi(P8,P7) && arg_in_0_pi(P7,P0)){
	A2=A1;
	B2=B1;
	P2=P1;
	P4=P8;
	P6=P7;
	if (step<nstep)
	  continue;
	found=true;
      }
      if (!found){
	P3=hornerarg(P,A2+cst_i*B1);
	// ? P1, P2, P3, P8
	if (arg_in_0_pi(P1,P2) && arg_in_0_pi(P2,P3) && arg_in_0_pi(P3,P8) && arg_in_0_pi(P8,P1)){
	  A0=A1;
	  B2=B1;
	  P0=P1;
	  P4=P3;
	  P6=P8;
	  if (step<nstep)
	    continue;
	  found=true;
	}
      }
      if (!found){
	// P8, P3, P4, P5
	P5=hornerarg(P,A1+cst_i*B2);
	if (arg_in_0_pi(P8,P3) && arg_in_0_pi(P3,P4) && arg_in_0_pi(P4,P5) && arg_in_0_pi(P5,P8)){
	  A0=A1;
	  B0=B1;
	  P0=P8;
	  P2=P3;
	  P6=P5;
	  if (step<nstep)
	    continue;
	  found=true;
	}
      }
      if (!found){
	// P7 P8 P5 P6
	if (arg_in_0_pi(P7,P8) && arg_in_0_pi(P8,P5) && arg_in_0_pi(P5,P6) && arg_in_0_pi(P6,P7)){
	  A2=A1;
	  B0=B1;
	  P0=P7;
	  P2=P8;
	  P4=P5;
	  if (step<nstep)
	    continue;
	  found=true;
	}
      }
      if (!found)
	return false;
      // Final check that there is indeed a root inside rectangle
      // args must be >= pi/8 and degree of (P)*max square length/distance to original square <= pi/8
      gen dist=min(min(A0-a0,a1-A2,context0),min(B0-b0,b1-B2,context0),context0);
      if (is_zero(dist))
	continue;
      gen max_sq=max(A2-A0,B2-B0,context0);
      if (is_greater((int(P.size())-2)*max_sq/dist,cst_pi/8,context0))
	continue;
      gen r1=P2/P0, r2=P4/P2, r3=P6/P4, r4=P0/P6;
      if (arg_geq_pi_8(r1) && arg_geq_pi_8(r2) && arg_geq_pi_8(r3) && arg_geq_pi_8(r4)){
	complexroots.push_back(makevecteur(makevecteur(A0+cst_i*B0,A2+cst_i*B2),1));
	return true;
      }
    }
    return false;
  }
#endif

  static gen round2util(const gen & num,const gen & den,int n){
    if (num.type==_CPLX){
      gen r=round2util(*num._CPLXptr,den,n);
      gen i=round2util(*(num._CPLXptr+1),den,n);
      return r+cst_i*i;
    }
    // num must be a _ZINT
    mpz_t tmp1,tmp2;
    mpz_init_set(tmp1,*num._ZINTptr);
    mpz_mul_2exp(tmp1,tmp1,n+1); // tmp1=2^(n+1)*num
    mpz_add(tmp1,tmp1,*den._ZINTptr); //      + den
    mpz_init_set(tmp2,*den._ZINTptr);
    mpz_mul_ui(tmp2,tmp2,2); // tmp2=2*den
    mpz_fdiv_q(tmp1,tmp1,tmp2);
    gen res=tmp1;
    mpz_clear(tmp1); mpz_clear(tmp2);
    return res;
  }

  void in_round2(gen & x,const gen & deuxn, int n){
    if (x.type==_INT_ || x.type==_ZINT)
      return ;
    if (x.type==_FRAC && x._FRACptr->den.type==_CPLX)
      x=fraction(x._FRACptr->num*conj(x._FRACptr->den,context0),x._FRACptr->den.squarenorm(context0));
    if (x.type==_FRAC && x._FRACptr->den.type==_ZINT && 
	(x._FRACptr->num.type==_ZINT || 
	 (x._FRACptr->num.type==_CPLX && x._FRACptr->num._CPLXptr->type==_ZINT && (x._FRACptr->num._CPLXptr+1)->type==_ZINT)) 
	){
      gen num=x._FRACptr->num,d=x._FRACptr->den;
      x=round2util(num,d,n);
      x=x/deuxn;
      return;
    }
    x=_floor(x*deuxn+plus_one_half,context0)/deuxn;
  }

  void round2(gen & x,int n){
    if (x.type==_INT_ || x.type==_ZINT)
      return ;
    gen deuxn;
    if (n<30)
      deuxn = (1<<n);
    else {
      mpz_t tmp;
      mpz_init_set_si(tmp,1);
      mpz_mul_2exp(tmp,tmp,n);
      deuxn=tmp;
      mpz_clear(tmp);
    }
    in_round2(x,deuxn,n);
  }

  void round2(gen & x,const gen & deuxn,GIAC_CONTEXT){
    if (x.type==_INT_ || x.type==_ZINT)
      return;
    if (x.type!=_FRAC)
      x=_floor(x*deuxn+plus_one_half,context0)/deuxn;
    else {
      gen n=x._FRACptr->num,d=x._FRACptr->den;
      if (d.type==_INT_){
	int di=d.val,ni=1;
	while (di>1){ di=di>>1; ni=ni<<1;}
	if (ni==d.val)
	  return;
      }
      n=2*n*deuxn+d;
      x=iquo(n,2*d)/deuxn;
    }
  }

  // Find one complex root inside a0,b0->a1,b1, return false if not found
  // algo: Newton method in exact mode starting from center
  bool newton_complex_1root(const modpoly & P,const gen & a0,const gen & b0,const gen & a1,const gen & b1,vecteur & complexroots,double eps){
    if (is_positive(a1-a0-0.01,context0) ||
	is_positive(b1-b0-0.01,context0))
      return false;
    gen x0=(a0+a1)/2+cst_i*(b0+b1)/2;
    modpoly Pprime=derivative(P);
    int n=int(-std::log(eps)/std::log(2.0)+.5); // for rounding
    gen eps2=pow(2,-(n+1),context0);
    for (int ii=0;ii<n;ii++){
      gen Pprimex0=horner(Pprime,x0,0,false);
      if (is_zero(Pprimex0,context0))
	return false;
      gen dx=horner(P,x0,0,false)/Pprimex0;
      gen absdx=dx*conj(dx,context0);
      x0=x0-dx;
      gen r=re(x0,context0),i=im(x0,context0);
      if (is_positive(a0-r,context0) || is_positive(r-a1,context0) || 
	  is_positive(b0-i,context0) || is_positive(i-b1,context0))
	return false;
      round2(r,n+4);
      round2(i,n+4);
      x0=r+cst_i*i;
      if (is_positive(absdx-eps2*eps2,context0))
	continue;
      // make a small square around x0 
      // and check that there is indeed a root inside
      gen A0=r-eps2;
      gen A1=r+eps2;
      gen B0=i-eps2;
      gen B1=i+eps2;
      gen tmp;
      if (csturm_square(P,A0+cst_i*B0,A1+cst_i*B1,tmp,context0)==2){
	complexroots.push_back(makevecteur(makevecteur(A0+cst_i*B0,A1+cst_i*B1),1));
	return true;
      }
    }
    return false;
  }

  // Find complex roots of P in a0,b0 -> a1,b1
  static int complex_roots(const modpoly & P,const gen & a0,const gen & b0,const gen & a1,const gen & b1,const vecteur & seq1,const vecteur & seq2,const vecteur & seq3,const vecteur & seq4,vecteur & realroots,vecteur & complexroots,double eps,vecteur & horiz_sturm,vecteur & vert_sturm){
    int n=csturm_square(P,a0,b0,a1,b1,seq1,seq2,seq3,seq4,context0);
    if (debug_infolevel && n)
      CERR << a0 << "," << b0 << ".." << a1 << "," << b1 << ":" << n/2 << '\n';
    if (n<=0)
      return 2*n;
    if (eps<=0){
      *logptr(context0) << gettext("Bad precision, using 1e-12 instead of ")+print_DOUBLE_(eps,14) << '\n';
      eps=1e-12;
    }
    if (is_strictly_greater(eps,a1-a0,context0) && is_strictly_greater(eps,b1-b0,context0)){
      gen r(makevecteur(a0+cst_i*b0,a1+cst_i*b1));
      complexroots.push_back(makevecteur(r,gen(n)/2));
      return n;
    }
    if (n==2 && newton_complex_1root(P,a0,b0,a1,b1,complexroots,eps))
      return n;
    gen a01=(a0+a1)/2,b01=(b0+b1)/2,pgcd;
    vecteur seqvert,seqhoriz;
    gen A=a0+cst_i*b01,B=a1+cst_i*b01;
    seqhoriz=csturm_segment_seq(P,A,B,realroots,eps,horiz_sturm,vert_sturm,context0);
    if (is_undef(seqhoriz)){
      realroots=seqhoriz;
      return -2;
    }
    pgcd=seqhoriz[4];
    if (is_one(pgcd)){
      A=a01+cst_i*b0; B=a01+cst_i*b1;
      seqvert=csturm_segment_seq(P,A,B,realroots,eps,horiz_sturm,vert_sturm,context0);
      if (is_undef(seqvert)){
	realroots=seqvert;
	return -2;
      }
      pgcd=seqvert[4];
    }
    if (!is_one(pgcd)){
      complex_roots_split(P,pgcd,a0,b0,a1,b1,realroots,complexroots,eps);
      return n;
    }
    /*
      (a0,b1)  - (a01,b1)  -  (a1,b1)         seq3                seq3
      |     n4   |     n3    |        seq4   n4      seqvert    n3     seq2
      (a0,b01) - (a01,b01) -  (a1,b01)        seqhoriz           seqhoriz
      |     n1   |     n2    |        seq4   n1      seqvert   n2      seq2
      (a0,b0)  - (a01,b0)  -  (a1,b0)         seq1                seq1
    */
    int n1=complex_roots(P,a0,b0,a01,b01,seq1,seqvert,seqhoriz,seq4,realroots,complexroots,eps,horiz_sturm,vert_sturm),nadd;
    if (n1==-2)
      return -2;
    if (n1==n)
      return n;
    n1 += (nadd=complex_roots(P,a01,b0,a1,b01,seq1,seq2,seqhoriz,seqvert,realroots,complexroots,eps,horiz_sturm,vert_sturm));
    if (nadd==-2)
      return -2;
    if (n1==n)
      return n;
    n1 += (nadd=complex_roots(P,a01,b01,a1,b1,seqhoriz,seq2,seq3,seqvert,realroots,complexroots,eps,horiz_sturm,vert_sturm));
    if (nadd==-2)
      return -2;
    if (n1==n)
      return n;
    n1 += (nadd=complex_roots(P,a0,b01,a01,b1,seqhoriz,seqvert,seq3,seq4,realroots,complexroots,eps,horiz_sturm,vert_sturm));
    if (nadd==-2)
      return -2;
    return n;
  }

  // Find complex roots of P in a0,b0 -> a1,b1
  static void complex_roots(const modpoly & P,const gen & a0,const gen & b0,const gen & a1,const gen & b1,vecteur & realroots,vecteur & complexroots,double eps){
    if (P.size()<2)
      return;
    vecteur Seq1,Seq2,Seq3,Seq4,horiz_sturm,vert_sturm;
    gen pgcd;
    if (!csturm_square_seq(P,a0,b0,a1,b1,pgcd,realroots,eps,Seq1,Seq2,Seq3,Seq4,horiz_sturm,vert_sturm,context0))
      complex_roots_split(P,pgcd,a0,b0,a1,b1,realroots,complexroots,eps);
    else
      complex_roots(P,a0,b0,a1,b1,Seq1,Seq2,Seq3,Seq4,realroots,complexroots,eps,horiz_sturm,vert_sturm);
  }

  // Find complex roots of P in a0,b0 -> a1,b1
  bool complex_roots(const modpoly & P,const gen & a0,const gen & b0,const gen & a1,const gen & b1,gen & pgcd,vecteur & roots,double eps){
    vecteur realroots,complexroots;
    complex_roots(P,a0,b0,a1,b1,realroots,complexroots,eps);
    if (is_undef(realroots))
      return false;
    roots=mergevecteur(roots,mergevecteur(realroots,complexroots));
    return true;
  }

  vecteur crationalroot(polynome & p,bool complexe){
    vectpoly v;
    int i=1;
    polynome qrem;
    environment * env= new environment;
    env->complexe=complexe || !is_zero(im(p,context0));
    vecteur w;
    if (!do_linearfind(p,env,qrem,v,w,i))
      w.clear();
    delete env;
    p=qrem;
    return w;
  }

  vecteur keep_in_rectangle(const vecteur & croots,const gen A0,const gen & B0,const gen & A1,const gen & B1,bool embed,GIAC_CONTEXT){
    vecteur roots;
    const_iterateur it=croots.begin(),itend=croots.end();
    for (;it!=itend;++it){
      gen a=re(*it,contextptr),b=im(*it,contextptr);
      if (is_greater(a,A0,contextptr)&&is_greater(A1,a,contextptr)&&is_greater(b,B0,contextptr)&&is_greater(B1,b,contextptr))
	roots.push_back(embed?makevecteur(*it,1):*it);
    }
    return roots;
  }

  gen square_modulus(const gen & g,GIAC_CONTEXT){
    return g.squarenorm(contextptr);
  }

  // P is the polynomial, P1 derivative, v list of approx roots 
  // (initially should have at least n bits precision), 
  // epsn is the target number of bits precision int(std::log(eps)/std::log(2.)-.5);
  // epsg2surdeg2 is eps^2/degree(P)^2 as a gen, epsg is the target precision
  // v[i] is set by newton_improve to be at distance at most vradius[i] of a root
  // kmax is the maximal number of Newton iterations
  bool newton_improve(const vecteur & P,const vecteur & P1,bool Preal,vecteur & v,vecteur & vradius,int i,int kmax,int n,int epsn,const gen & epsg2surdeg2,const gen & epsg){
    gen r=v[i];
    bool nextconj=false;
    if (Preal && i+1<int(v.size()))
      nextconj=is_exactly_zero(r-conj(v[i+1],context0));
    if (r.type==_FRAC || is_cinteger(r))
      return true;
    // find nearest root from v
    gen deltar=plus_inf,delta;
    for (unsigned j=0;j<v.size();++j){
      if (int(j)==i)
	continue;
      gen tmp=abs(r-v[j],context0);
      if (is_strictly_greater(deltar,tmp,context0))
	deltar=tmp;
    }
    if (is_zero(deltar))
      return false;
    deltar=deltar/3;
    gen sumdr2=0;
    int N=n; // effective value of number of bits for computation
#ifdef HAVE_LIBMPFR
    if (r.type==_REAL && mpfr_get_prec(r._REALptr->inf)>N)
      N=mpfr_get_prec(r._REALptr->inf);
    if (r.type==_CPLX && r._CPLXptr->type==_REAL && mpfr_get_prec(r._CPLXptr->_REALptr->inf)>N)
      N=mpfr_get_prec(r._CPLXptr->_REALptr->inf);
#endif
#if 0 // def HAVE_LIBMPFI
    gen deuxN=pow(2,N,context0);
    gen rr,ri,dr,di;
    reim(r,rr,ri,context0);
    if (Preal && !nextconj)
      r=eval(gen(makevecteur(rr-plus_one/deuxN,rr+plus_one/deuxN),_INTERVAL__VECT),1,context0);
    else
      r=eval(gen(makevecteur(rr-plus_one/deuxN,rr+plus_one/deuxN),_INTERVAL__VECT),1,context0)+cst_i*eval(gen(makevecteur(ri-plus_one/deuxN,ri+plus_one/deuxN),_INTERVAL__VECT),1,context0);
    for (int k=0;k<kmax;++k){
      // check if root precision is ok
      // otherwise try to improve root precision with Newton method
      gen P1r=horner(P1,r,0,false);
      if (is_exactly_zero(P1r)){
	delta=plus_inf;
	break;
      }
      gen Pr=horner(P,r,0,false);
      delta=Pr/P1r;
      bool test;
      if (Preal && ! nextconj){
	test=contains(delta,dr);
	delta=abs(delta,context0);
      }
      else {
	reim(delta,dr,di,context0);
	test= contains(rr,dr) && contains(ri,di);
	delta=square_modulus(delta,context0);
      }
      if (test){
	v[i]=r; // we can certify there is a root in r by Brouwer fixed thm
	vradius[i]=-1;
	if (nextconj){
	  v[i+1]=conj(r,context0);
	  vradius[i+1]=vradius[i];
	  ++i;
	}
	break;
      }
      if (delta.type==_REAL){
	if (real_interval * ptr=dynamic_cast<real_interval *>(delta._REALptr)){
	  mpfr_t tmp; mpfr_init(tmp);
	  mpfi_get_right(tmp,ptr->infsup);
	  delta=real_object(tmp);
	  mpfr_clear(tmp);
	}
      }
      sumdr2 += delta;
      if (!is_greater(deltar*deltar,sumdr2,context0)){
	CERR << "Unable to certify " << v[i] << '\n' ;
	return false;
      }
      if (N<P.size()-epsn){
	// add 10 bits of precision or double it
	if (N<-epsn/2){
	  deuxN=deuxN*deuxN;
	  N*=2;
	}
	else {
	  deuxN=1024*deuxN;
	  N+=10;
	}
      }
      r -= Pr/P1r;
      // change precision to N
      reim(r,rr,ri,context0);
      if (rr.type==_REAL){
	if (real_interval * ptr=dynamic_cast<real_interval *>(rr._REALptr))
	  mpfi_set_prec(ptr->infsup,N);
      }
      if (ri.type==_REAL){
	if (real_interval * ptr=dynamic_cast<real_interval *>(ri._REALptr))
	  mpfi_set_prec(ptr->infsup,N);
      }
      r=rr+cst_i*ri;
    } // end for k
#else
    if (N>int(P.size())/4-epsn/2)
      N=int(P.size())/4-epsn/2;
    gen deuxN=pow(2,N,context0);
    for (int k=0;k<kmax;++k){
      in_round2(r,deuxN,N); // round2(r,deuxN,context0);
      // check if root precision is ok
      // otherwise try to improve root precision with Newton method
      gen P1r=horner(P1,r,0,false);
      if (is_exactly_zero(P1r)){
	delta=plus_inf;
	break;
      }
      gen Pr=horner(P,r,0,false);
      delta=square_modulus(Pr,context0)/square_modulus(P1r,context0);
      if (is_greater(epsg2surdeg2,delta,context0)){
	v[i]=r; // we can certify there is a root at distance <= eps from r
	if (is_exactly_zero(Pr))
	  vradius[i]=0;
	else
	  vradius[i]=n*sqrt(accurate_evalf(delta,100),context0);
	// problem with double underflow
	if (!is_exactly_zero(vradius[i]))
	  vradius[i]=min(epsg,pow(plus_two,int(evalf_double(ln(vradius[i],context0),1,context0)._DOUBLE_val/std::log(2.))+1),context0);
	if (debug_infolevel)
	  CERR << CLOCK() << " isolated " << r << " radius " << vradius[i] << '\n';
	if (nextconj){
	  v[i+1]=conj(r,context0);
	  vradius[i+1]=vradius[i];
	  ++i;
	}
	break;
      }
      sumdr2 += delta;
      if (!is_greater(deltar*deltar,sumdr2,context0)){
	CERR << "Unable to certify " << v[i] << '\n' ;
	return false;
      }
      if (N<int(P.size())-epsn){
	// add 10 bits of precision or double it
	if (N<-epsn){
	  deuxN=deuxN*deuxN;
	  N*=2;
	}
	else {
	  deuxN=1024*deuxN;
	  N+=10;
	}
      }
      in_round2(Pr,deuxN,N); in_round2(P1r,deuxN,N); // round2(Pr,deuxN,context0); round2(P1r,deuxN,context0);
      r -= Pr/P1r;
    } // end for k
#endif
    if (!is_greater(epsg*epsg,delta,context0))
      return false;
    return true;
  }

#ifdef HAVE_LIBMPFR
  void round1downup(const gen & a,const gen & r,gen & ad,gen & au){
    if (a.type==_REAL && r.type==_REAL){
      int n=mpfr_get_prec(a._REALptr->inf);
      ad=real_object(a._REALptr->inf);
      au=real_object(a._REALptr->inf);
      mpfr_set_prec(ad._REALptr->inf,n+1);
      mpfr_set_prec(au._REALptr->inf,n+1);
      mpfr_set(ad._REALptr->inf,a._REALptr->inf,MPFR_RNDD);
      mpfr_sub(ad._REALptr->inf,ad._REALptr->inf,r._REALptr->inf,MPFR_RNDD);
      mpfr_set(au._REALptr->inf,a._REALptr->inf,MPFR_RNDU);
      mpfr_add(au._REALptr->inf,au._REALptr->inf,r._REALptr->inf,MPFR_RNDU);
    }
    else {
      ad=a-r;
      au=a+r;
    }
  }
#else
  void round1downup(const gen & a,const gen & r,gen & ad,gen & au){
    ad=au=a;
  }
#endif

  // find roots of polynomial P at precision eps using proot or 
  // complex Sturm sequences
  // P must have numeric coefficients, in Q[i] and should be squarefree
  vecteur complex_roots(const modpoly & P,const gen & a0,const gen & b0,const gen & a1,const gen & b1,bool complexe,double eps,bool use_proot){
    if (P.empty())
      return P;
    bool mps=eps<0;
    eps=absdouble(eps);
    if (eps>1e-6)
      eps=1e-6;
    {
      vecteur v,res,vradius;
      bool b;
      if (mps)
        b=mps_solve(P,v,vradius,-eps,1/* isolate*/,true/*secular algo*/,context0)==0;
      else
        b=aberth(P,v,vradius,ABERTH_NMAX,eps,3/* isolate*/,false/* exact*/,context0);      
      if (b){
        for (unsigned j=0;j<v.size();++j){
          gen a,b;
          reim(v[j],a,b,context0);
          if (is_greater(a,a0,context0) && is_greater(a1,a,context0) && is_greater(b,b0,context0) && is_greater(b1,b,context0)){
            if (is_strictly_positive(-vradius[j],context0))
              res.push_back(makevecteur(v[j],1));
            else {
              gen ad,au;
              round1downup(a,vradius[j],ad,au);
              a=eval(change_subtype(makevecteur(ad,au),_INTERVAL__VECT),1,context0);
              gen bd,bu;
              round1downup(b,vradius[j],bd,bu);
              b=eval(change_subtype(makevecteur(bd,bu),_INTERVAL__VECT),1,context0);
              res.push_back(makevecteur(gen(a,b),1));
            }
          }
        }
        return res;
      }
    }
    if (use_proot){
      int epsn=int(std::log(eps)/std::log(2.)-.5);
      gen epsg=pow(2,epsn,context0);
      gen epsg2surdeg2=(epsg*epsg)/int((P.size()+1)*(P.size()+1));	
      // first try proot with double precision, if it's not enough increase
      int n=45;
      bool Preal=is_zero(im(P,context0));
      modpoly P1=derivative(P);
      for (;n<400;n*=2){
	double cureps=std::pow(2.0,-n);
	if (debug_infolevel)
	  CERR << CLOCK() << " proot at precision " << cureps << '\n';
	vecteur v=proot(P,cureps,n,context0);
	if (debug_infolevel)
	  CERR << CLOCK() << " proot end at precision " << cureps << '\n';
	vecteur vradius(v.size());
	unsigned i=0;
	int kmax=int(std::log(eps)/std::log(cureps))+4;
	for (;i<v.size();++i){
	  newton_improve(P,P1,Preal,v,vradius,i,kmax,n,epsn,epsg2surdeg2,epsg);
	} // end for i
	if (i==v.size()){
	  vecteur res;
	  for (unsigned j=0;j<v.size();++j){
	    if (Preal && is_zero(im(v[j],context0))){
	      if (is_exactly_zero(vradius[j]) || vradius[j]==-1){
		if (is_greater(v[j],a0,context0) && is_greater(a1,v[j],context0) && is_greater(0,b0,context0) && is_greater(b1,0,context0))
		  res.push_back(makevecteur(v[j],1));
		continue;
	      }
	      gen P1=horner(P,v[j]-vradius[j],0,false),P2=horner(P,v[j]+vradius[j],0,false);
	      if (P1.type==_FRAC) P1=P1._FRACptr->num;
	      if (P2.type==_FRAC) P2=P2._FRACptr->num;
	      if (is_strictly_positive(-P1*P2,context0)){
		if (is_greater(v[j],a0,context0) && is_greater(a1,v[j],context0) && is_greater(0,b0,context0) && is_greater(b1,0,context0))
		  res.push_back(makevecteur(eval(change_subtype(makevecteur(v[j]-vradius[j],v[j]+vradius[j]),_INTERVAL__VECT),1,context0),1));
		continue;
	      }
	    }
	    gen R,I;
	    reim(v[j],R,I,context0);
	    if (is_greater(R,a0,context0) && is_greater(a1,R,context0) && is_greater(I,b0,context0) && is_greater(b1,I,context0)){
	      if (is_exactly_zero(vradius[j]))
		res.push_back(makevecteur(v[j],1));
	      else {
#ifdef HAVE_LIBMPFI
		gen a,b;
		reim(v[j],a,b,context0);
		res.push_back(makevecteur(eval(change_subtype(makevecteur(a-vradius[j],a+vradius[j]),_INTERVAL__VECT),1,context0)+cst_i*eval(change_subtype(makevecteur(b-vradius[j],b+vradius[j]),_INTERVAL__VECT),1,context0),1));
#else
		res.push_back(makevecteur(makevecteur(ratnormal(v[j]-vradius[j]*(1+cst_i)),ratnormal(v[j]+vradius[j]*(1+cst_i))),1));
#endif
	      }
	    }
	  }
	  return res;
	} // end if i==v.size()
      } // end for n
      CERR << "proot isolation did not work, trying complex Sturm sequences" << '\n';
    }
    bool aplati=(a0==a1) && (b0==b1);
    if (!aplati && complexe && (a0==a1 || b0==b1) )
      return vecteur(1,gensizeerr(gettext("Square is flat!")));
    gen A0(a0),B0(b0),A1(a1),B1(b1);
    {
      // initial rectangle: |roots|< 1+ max(|a_i|)/|a_n|
      gen maxai=_max(*apply(P,abs,context0)._VECTptr,context0);
      gen tmp=1+maxai/abs(P.front(),context0);
      if (aplati){
	A0=-tmp;
	B0=-tmp;
	A1=tmp;
	B1=tmp;
      }
      if (is_inf(A0)) A0=-tmp;
      if (is_inf(B0)) B0=-tmp;
      if (is_inf(A1)) A1=tmp;
      if (is_inf(B1)) B1=tmp;
    }
    gen tmp;
    modpoly p(*apply(P,exact,context0)._VECTptr);
    lcmdeno(p,tmp,context0);
    polynome pp(poly12polynome(p));
    if (!complexe){
      gen tmp=gcd(re(pp,context0),im(pp,context0));
      if (tmp.type!=_POLY)
	return vecteur(0);
      pp=*tmp._POLYptr;
    }
    vecteur croots=crationalroot(pp,complexe);
    vecteur roots=keep_in_rectangle(croots,A0,B0,A1,B1,true,context0);
    p=polynome2poly1(pp);
    gen an=p.front();
    if (!is_zero(im(an,context0)))
      p=conj(p.front(),context0)*p;
    if (!complexe){ // real root isolation
      modpoly R=p;
      modpoly S=derivative(R);
      vecteur listquo,coeffP,coeffR;
      csturm_seq(S,R,listquo,coeffP,coeffR,context0);
      // sparse polynomial patch
      if (pp.coord.size()<p.size()/10.){
	R=vecteur(1,poly12polynome(R,1,1));
	S=vecteur(1,poly12polynome(S,1,1));
      }
      csturm_realroots(S,R,listquo,coeffP,coeffR,0,1,A0,A1,roots,eps,context0);
      return roots;
    }
    csturm_normalize(p,A0,B0,A1,B1,roots);
    gen pgcd;
    if (!complex_roots(p,A0,B0,A1,B1,pgcd,roots,eps))
      return vecteur(1,gensizeerr(context0));
    return roots;
  }


  gen complexroot(const gen & g,bool complexe,GIAC_CONTEXT){
    vecteur v=gen2vecteur(g);
    if (g.subtype==_POLY1__VECT)
      v=makevecteur(v);
    bool use_vas=!complexe ,use_proot=true;
#ifndef HAVE_LIBMPFR
    use_proot=false;
#endif
    bool isolation=false;
    if (!v.empty() && v[0]==at_sturm){
      use_vas=false;
      use_proot=false;
      v.erase(v.begin());
    }
    if (v.empty())
      return gensizeerr(contextptr);
    if (v.size()<2){
      isolation=true;
      v.push_back(epsilon(contextptr));
    }
    if (v.size()==3)
      v.insert(v.begin()+1,epsilon(contextptr));
    gen p=v.front(),prec=evalf_double(v[1],1,contextptr);
    if (prec.type!=_DOUBLE_)
      return gentypeerr(contextptr);
    double eps=prec._DOUBLE_val;
    if (eps>=1.0)
      eps=std::pow(10.,-eps);
    if (v[0].type==_VECT && has_num_coeff(v[0])){
      v=proot(*v[0]._VECTptr,eps,contextptr);
      vecteur w;
      for (unsigned i=0;i<v.size();++i){
	if (is_zero(im(v[i],contextptr)))
	  w.push_back(makevecteur(re(v[i],contextptr),1));
      }
      return w;
    }
    unsigned vs=unsigned(v.size());
    gen A(0),B(0),a0(minus_inf),b0(minus_inf),a1(plus_inf),b1(plus_inf);
    if (vs>3){
      A=v[2];
      B=v[3];
      a0=re(A,contextptr); b0=im(A,contextptr);
      a1=re(B,contextptr);b1=im(B,contextptr);
    }
    if (is_greater(a0,a1,contextptr))
      swapgen(a0,a1);
    if (is_greater(b0,b1,contextptr))
      swapgen(b0,b1);
    vecteur vas_res;
    if (p.type==_VECT){
      if (use_vas && vas(*p._VECTptr,a0,a1,isolation?1e300:eps,vas_res,true,contextptr))
	return vas_res;
      return complex_roots(*p._VECTptr,a0,b0,a1,b1,complexe,eps,use_proot);
    }
    if (use_vas && vas(symb2poly_num(v[0],contextptr),a0,a1,isolation?1e300:eps,vas_res,true,contextptr))
      return vas_res;
    vecteur l,l0;
    lidnt(p,l0,false);
    if (l0.size()!=1)
      return gentypeerr(contextptr);
    l=alg_lvar(p);
    gen px=_e2r(makesequence(p,l),contextptr);
    if (px.type==_FRAC)
      px=px._FRACptr->num;
    if (px.type!=_POLY)
      return vecteur(0);
    factorization f(sqff(*px._POLYptr));
    factorization::const_iterator it=f.begin(),itend=f.end();
    vecteur res;
    for (;it!=itend;++it){
      gen P=_poly2symb(makesequence(it->fact,l),contextptr);
      P=_e2r(makesequence(P,l0.front()),contextptr);
      if (P.type!=_VECT)
	continue;
      vecteur tmp=complex_roots(*P._VECTptr,a0,b0,a1,b1,complexe,eps,use_proot);
      if (is_undef(tmp))
	return tmp;
      iterateur jt=tmp.begin(),jtend=tmp.end();
      for (;jt!=jtend;++jt){
	if (jt->type==_VECT && jt->_VECTptr->size()==2)
	  jt->_VECTptr->back()=it->mult*jt->_VECTptr->back();
      }
      res=mergevecteur(res,tmp);
    }
    return res;
  }

  gen _complexroot(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    gen res=complexroot(g,true,contextptr);
    if (res.type==_VECT)
      gen_sort_f_context(res._VECTptr->begin(),res._VECTptr->end(),complex_sort,contextptr);
    return res;
    // return _sorta(complexroot(g,true,contextptr),contextptr);
  }
  static const char _complexroot_s []="complexroot";
  static define_unary_function_eval (__complexroot,&_complexroot,_complexroot_s);
  define_unary_function_ptr5( at_complexroot ,alias_at_complexroot,&__complexroot,0,true);

  gen _realroot(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    gen res; bool evalf_after=false;
    if (g.type==_VECT && !g._VECTptr->empty() && g._VECTptr->back()==at_evalf){
      res=complexroot(gen(vecteur(g._VECTptr->begin(),g._VECTptr->end()-1),g.subtype),false,contextptr);
      evalf_after=true;
    }
    else
      res=complexroot(g,false,contextptr);
    if (res.type!=_VECT)
      return res;
    vecteur v=*res._VECTptr;
    for (unsigned i=0;i<v.size();++i){
      if (v[i].type==_VECT && v[i]._VECTptr->size()==2){
	gen a=v[i]._VECTptr->front(),b=v[i]._VECTptr->back();
	if (a.type==_VECT && a.subtype==_INTERVAL__VECT){
	  if (evalf_after)
	    v[i]=evalf((a._VECTptr->front()+a._VECTptr->back())/2,1,contextptr);
	  else {
	    a=eval(a,1,contextptr);
	    v[i]=makevecteur(a,b);
	  }
	}
	else {
	  if (evalf_after)
	    v[i]=evalf(a,1,contextptr);
	}
      }
    }
    return v;
  }
  static const char _realroot_s []="realroot";
  static define_unary_function_eval (__realroot,&_realroot,_realroot_s);
  define_unary_function_ptr5( at_realroot ,alias_at_realroot,&__realroot,0,true);

  static vecteur crationalroot(const gen & g0,bool complexe){
    gen g(g0),a,b;
    if (g.type==_VECT){
      if (g.subtype==_SEQ__VECT){
	vecteur & tmp=*g._VECTptr;
	if (tmp.size()!=3)
	  return vecteur(1,gendimerr(context0));
	g=tmp[0];
	a=tmp[1];
	b=tmp[2];
      }
      else {
	g=poly12polynome(*g._VECTptr);
      }
    }
    gen a0,b0,a1,b1;
    ab2a0b0a1b1(a,b,a0,b0,a1,b1,context0);
    vecteur l;
    lvar(g,l);
    if (l.empty())
      return vecteur(0);
    if (l.size()!=1)
      return vecteur(1,gentypeerr(context0));
    gen px=_e2r(makevecteur(g,l),context0);
    if (px.type==_FRAC)
      px=px._FRACptr->num;
    if (px.type!=_POLY)
      return vecteur(0);
    factorization f(sqff(*px._POLYptr));
    factorization::const_iterator it=f.begin(),itend=f.end();
    vecteur res;
    for (;it!=itend;++it){
      polynome p=it->fact;
      vecteur tmp=crationalroot(p,complexe);
      res=mergevecteur(res,tmp);
    }
    if (a0!=a1 || b0!=b1)
      res=keep_in_rectangle(res,a0,b0,a1,b1,false,context0);
    return res;
  }
  gen _crationalroot(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    return crationalroot(g,true);
  }
  static const char _crationalroot_s []="crationalroot";
  static define_unary_function_eval (__crationalroot,&_crationalroot,_crationalroot_s);
  define_unary_function_ptr5( at_crationalroot ,alias_at_crationalroot,&__crationalroot,0,true);

  gen _rationalroot(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    return crationalroot(g,false);
  }
  static const char _rationalroot_s []="rationalroot";
  static define_unary_function_eval (__rationalroot,&_rationalroot,_rationalroot_s);
  define_unary_function_ptr5( at_rationalroot ,alias_at_rationalroot,&__rationalroot,0,true);

  // convert numerator of g to a list
  vecteur symb2poly_num(const gen & g_,GIAC_CONTEXT){
    gen g(g_);
    if (g.type!=_VECT)
      g=makesequence(g,ggb_var(g));
    gen tmp=_symb2poly(g,contextptr);
    if (tmp.type==_FRAC)
      tmp=tmp._FRACptr->num;
    if (tmp.type!=_VECT)
      return vecteur(1,gensizeerr(contextptr));
    return *tmp._VECTptr;
  }
  // VAS implementation. Based on Xcas implementation by  Alkiviadis G. Akritas,
  // A first C++ implementation was written by Spyros Kehagias and others
  // but it was too close to the Xcas code 
  // This implementation is much faster, using basic data structures of giac
  // number of sign changes of the coefficients of P, returns -1 on error
  int variations(const modpoly & P,GIAC_CONTEXT){
    int res=0,n=int(P.size());
    if (!n)
      return -1;
    int previous=fastsign(P.front(),contextptr);
    if (previous==0)
      return -1;
    for (int i=1;i<n;i++){
      if (is_exactly_zero(P[i]))
	continue;
      int current=fastsign(P[i],contextptr);
      if (!current)
	return -1;
      if (current!=previous){
	++res;
	previous=current;
      }
    }
    return res;
  }

#ifndef M_LN2
#define M_LN2 0.6931471805599454
#endif

  // like (ln(n/d)+shift*ln2)/expo, but faster for large integers
  gen LMQ_evalf(const gen & n,const gen & d,double shift,int expo,GIAC_CONTEXT){
#if !defined USE_GMP_REPLACEMENTS && !defined BF2GMP_H
    if (is_integer(n) && is_integer(d)){
      long int nexp=0,dexp=0;
      double nmant,dmant;
      if (n.type==_INT_)
	nmant=n.val;
      else
	nmant=mpz_get_d_2exp (&nexp,*n._ZINTptr);
      if (d.type==_INT_)
	dmant=d.val;
      else
	dmant=mpz_get_d_2exp (&dexp,*d._ZINTptr);
      return ( std::log(-nmant/dmant) + (nexp-dexp+shift)*M_LN2 )/expo;
    }
#endif
    return ( ln(evalf(-n/d,1,contextptr),contextptr) + shift*M_LN2 )/gen(expo);
  }

  static bool compute_lnabsmantexpo(const vecteur & cl,vector<double> & cllnabsmant,vector<long int> & clexpo,vector<short int> & clsign,GIAC_CONTEXT){
    int k=int(cl.size());
    cllnabsmant.resize(k);
    clexpo.resize(k);
    clsign.resize(k);
    for (int i=0;i<k;++i){
      gen tmp=sign(cl[i],contextptr);
      if (tmp.type!=_INT_)
	return false;
      clsign[i]=tmp.val;
      double mant;
      long int expo=0;
      if (is_integer(cl[i])){
	if (cl[i].type==_ZINT){
#if defined USE_GMP_REPLACEMENTS || defined BF2GMP_H
	  mant=evalf_double(cl[i],1,contextptr)._DOUBLE_val;
#else
	  mant=mpz_get_d_2exp (&expo,*cl[i]._ZINTptr);
#endif
	}
	else mant=cl[i].val;
      }
      else {
	if (cl[i].type==_DOUBLE_){
	  mant=cl[i]._DOUBLE_val;
	}
	else {
	  mant=evalf_double(cl[i],1,contextptr)._DOUBLE_val;
	}
      }
      mant=std::log(absdouble(mant));
      cllnabsmant[i]=mant;
      clexpo[i]=expo;
    }
    return true;
  }


  gen posubLMQ(const modpoly & P,GIAC_CONTEXT){
    //---implements the Local_Max_Quadratic method (LMQ) to compute an
    //---upper bound on the values of the POSITIVE roots of p(x).
    
    //---Reference:"Linear and Quadratic Complexity Bounds on the Values of the 
    //---Positive Roots of Polynomials" by Alkiviadis G. Akritas. 
    //---Journal of Universal Computer Science, Vol. 15, No. 3, 523-537, 2009. 
    int k=int(P.size());
    if (k<=1)
      return 0;
    vecteur cl;
    if (is_strictly_positive(P.front(),contextptr))
      cl=P;
    else
      cl=-P;
    reverse(cl.begin(),cl.end());
    vector<double> cllnabsmant; 
    vector<long int> clexpo;
    vector<short int> clsign;
    if (!compute_lnabsmantexpo(cl,cllnabsmant,clexpo,clsign,contextptr))
      return gensizeerr(contextptr);
    gen tempmax=minus_inf;
    vector<int> timesused(k+1,1);
    for (int m=k-1;m>=1;--m){
      if (clsign[m-1]==-1){ // is_strictly_positive(-cl[m-1],contextptr)
	gen tempmin=plus_inf;
	for (int n=k;n>m;--n){
	  if (clsign[n-1]==1){ // is_strictly_positive(cl[n-1],contextptr)
	    gen temp= (cllnabsmant[m-1]-cllnabsmant[n-1] + (clexpo[m-1]-clexpo[n-1]+timesused[n-1])*M_LN2)/(n-m);// LMQ_evalf(cl[m-1],cl[n-1],timesused[n-1],n-m,contextptr);
	    // gen temp=pow(-cl[m-1]/cl[n-1]*pow(plus_two,timesused[n-1]),inv(n-m,contextptr),contextptr);
	    // temp=evalf(temp,1,contextptr);
	    ++timesused[n-1];
	    if (is_strictly_greater(tempmin,temp,contextptr))
	      tempmin=temp;
	  }
	}
	if (is_strictly_greater(tempmin,tempmax,contextptr))
	  tempmax=tempmin;
      }
    }
    return _ceil(65*exp(tempmax,contextptr)/64,contextptr); // small margin
  }

  gen _posubLMQ(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    vecteur v;
    if (g.type==_VECT && g.subtype!=_SEQ__VECT)
      v=*g._VECTptr;
    else
      v=symb2poly_num(g,contextptr);
    return posubLMQ(v,contextptr);
  }
  static const char _posubLMQ_s []="posubLMQ";
  static define_unary_function_eval (__posubLMQ,&_posubLMQ,_posubLMQ_s);
  define_unary_function_ptr5( at_posubLMQ ,alias_at_posubLMQ,&__posubLMQ,0,true);

  gen poslbdLMQ(const modpoly & P,int & res,GIAC_CONTEXT){
    //---implements the Local_Max_Quadratic method (LMQ) to compute a
    //---lower bound on the values of the POSITIVE roots of p(x).
    
    //---Reference:"Linear and Quadratic Complexity Bounds on the Values of the 
    //---Positive Roots of Polynomials" by Alkiviadis G. Akritas. 
    //---Journal of Universal Computer Science, Vol. 15, No. 3, 523-537, 2009. 
    int k=int(P.size());
    if (k<=1)
      return 0;
    vecteur cl(P);
    reverse(cl.begin(),cl.end());
    if (is_strictly_positive(-cl.front(),contextptr))
      cl=-cl;
    vector<double> cllnabsmant; 
    vector<long int> clexpo;
    vector<short int> clsign;
    if (!compute_lnabsmantexpo(cl,cllnabsmant,clexpo,clsign,contextptr))
      return gensizeerr(contextptr);
    gen tempmax=symbolic(at_neg,_IDNT_infinity());
    vector<int> timesused(k,1);
    for (int m=1;m<k;++m){
      if (clsign[m]==-1){ // is_strictly_positive(-cl[m],contextptr)
	gen tempmin=symbolic(at_plus,_IDNT_infinity());
	for (int n=0;n<m;++n){
	  if (clsign[n]==1){ // is_strictly_positive(cl[n],contextptr)
	    // gen temp=pow(-cl[m]/cl[n]*pow(plus_two,timesused[n]),inv(m-n,contextptr),contextptr);
	    // temp=evalf(temp,1,contextptr);
	    gen temp= (cllnabsmant[m]-cllnabsmant[n] + (clexpo[m]-clexpo[n]+timesused[n])*M_LN2)/(m-n);// LMQ_evalf(cl[m],cl[n],timesused[n],m-n,contextptr);
	    ++timesused[n];
	    if (is_strictly_greater(tempmin,temp,contextptr))
	      tempmin=temp;
	  }
	}
	if (is_strictly_greater(tempmin,tempmax,contextptr))
	  tempmax=tempmin;
      }
    }
    tempmax=tempmax/M_LN2;
    tempmax=-_ceil(tempmax,contextptr);
    if (tempmax.type==_INT_)
      res=tempmax.val;
    else
      res=1<<31;
    tempmax=pow(plus_two,tempmax,contextptr);
    return tempmax; 
  }
  
  gen _poslbdLMQ(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    vecteur v;
    if (g.type==_VECT && g.subtype!=_SEQ__VECT)
      v=*g._VECTptr;
    else
      v=symb2poly_num(g,contextptr);
    int res;
    return poslbdLMQ(v,res,contextptr);
  }
  static const char _poslbdLMQ_s []="poslbdLMQ";
  static define_unary_function_eval (__poslbdLMQ,&_poslbdLMQ,_poslbdLMQ_s);
  define_unary_function_ptr5( at_poslbdLMQ ,alias_at_poslbdLMQ,&__poslbdLMQ,0,true);

  vecteur makeinterval(const gen & a,const gen & b){
    if (is_strictly_greater(a,b,context0))
      return makevecteur(b,a);
    else
      return makevecteur(a,b);
  }

  bool vas_sort(const gen & a,const gen &b){
    gen a1(a),b1(b);
    if (a.type==_VECT && a._VECTptr->size()==2)
      a1=a._VECTptr->front();
    if (b.type==_VECT && b._VECTptr->size()==2)
      b1=b._VECTptr->front();
    return is_strictly_greater(b1,a1,context0);
  }

  // P is assumed to be squarefree and without rational roots
  // find roots of P((ax+b)/(cx+d))
  vecteur VAS_positive_roots(const modpoly & P,const gen & ap,const gen & bp,const gen & cp,const gen & dp,GIAC_CONTEXT){
    matrice Pascal;
    //---The steps below correspond to the steps described in the reference below.
    
    //---Reference:	"A Comparative Study of Two Real Root Isolation Methods" 
    //---by Alkiviadis G. Akritas and Adam W. Strzebonski. 
    //---Nonlinear Analysis: Modelling and Control, Vol. 10, No. 4, 297-304, 2005.
    vecteur res; // root isolation intervals
    vecteur intervals_to_process;
    // STEP 1
    int v0=variations(P,contextptr);
    if (!v0)
      return res;
    gen ub=posubLMQ(P,contextptr);
    if (v0==1)
      return vecteur(1,makeinterval(0,ap*ub));
    intervals_to_process.push_back(makevecteur(ap, bp, cp, dp, P,v0));

    // STEP 2
    while (!intervals_to_process.empty()){
      gen tmp=intervals_to_process.back();
      intervals_to_process.pop_back();
      if (tmp.type!=_VECT || tmp._VECTptr->size()!=6)
	return vecteur(1,gensizeerr("VAS interval"+tmp.print()));
      vecteur & tmpv=*tmp._VECTptr;
      gen a=tmpv[0],b=tmpv[1],c=tmpv[2],d=tmpv[3], genf=tmpv[4],genv=tmpv[5];
      if (genf.type!=_VECT || genv.type!=_INT_)
	return vecteur(1,gensizeerr("VAS interval"+tmp.print()));
      int v=genv.val;
      modpoly f = *genf._VECTptr;

      // STEP 3
      int lbi;
      gen lb=poslbdLMQ(f,lbi,contextptr);

      // STEP 4
      if (is_strictly_greater(lb,16,contextptr)){
	change_scale(f,lb,lbi);
	a=lb*a; c=lb*c; lb=1; lbi=0;
      }
      
      // STEP 5
      if (is_greater(lb,1,contextptr)){
	// f=taylor(f,lb);
	change_scale(f,lb,lbi);
	f=taylor(f,1,0,&Pascal);
	back_change_scale(f,lb,lbi);
	b = lb*a + b; d = lb*c + d;
	if (is_zero(f.back())){
	  res.push_back(b/d);
	  f.pop_back();
	}
	v=variations(f,contextptr);
	if (!v)
	  continue;
	if (v==1){
	  if (!is_zero(c))
	    res.push_back(makeinterval(a/c,b/d));
	  else
	    res.push_back(makeinterval(b,b+a*posubLMQ(f,contextptr)));
	  continue;
	}
      }

      // STEP 6
      modpoly f1=taylor(f,1,0,&Pascal),f2;
      gen a1=a, b1=a+b, c1=c, d1=c+d;
      int r=0;
      if (is_zero(f1.back())){
	f1.pop_back();
	res.push_back(b1/d1);
	r=1;
      }
      int v1=variations(f1,contextptr);
      int v2=v-v1-r; 
      gen a2=b, b2=a+b, c2=d, d2=c+d;
      
      // STEP 7
      if (v2>1){
	f2=f;
	reverse(f2.begin(),f2.end());
	f2=taylor(f2,1,0,&Pascal);
	if (is_zero(f2.back()))
	  f2.pop_back();
	v2=variations(f2,contextptr);
      }

      // STEP 8
      if (v1<v2){
	swapgen(a1,a2);
	swapgen(b1,b2);
	swapgen(c1,c2);
	swapgen(d1,d2);
	swap(f1,f2);
	int i=v1; v1=v2; v2=i;
      }

      // STEP 9
      if (v1==0) continue;
      if (v1==1){
	if (!is_zero(c1))
	  res.push_back(makeinterval(a1/c1,b1/d1));
	else 
	  res.push_back(makeinterval(b1,b1 + a1*posubLMQ(f1,contextptr)));
      }
      else 
	intervals_to_process.push_back(makevecteur(a1,b1,c1,d1,f1,v1));

      // STEP 10
      if (v2==0) continue;
      if (v2==1){
	if (!is_zero(c2))
	  res.push_back(makeinterval(a2/c2,b2/d2));
	else 
	  res.push_back(makeinterval(b2,b2 + a2*posubLMQ(f2,contextptr)));
      }
      else 
	intervals_to_process.push_back(makevecteur(a2,b2,c2,d2,f2,v2));
    }
    gen_sort_f(res.begin(),res.end(),vas_sort);
    return res;
  }

  // P((ax+b)/(cx+d))
  struct thread_vas_t {
    modpoly * P;
    vecteur * v;
    gen a,b,c,d;
    const context * contextptr;
  };

  void * do_thread_vas_t(void * ptr_){
    thread_vas_t * ptr=(thread_vas_t *) ptr_;
    *ptr->v=VAS_positive_roots(*ptr->P,ptr->a,ptr->b,ptr->c,ptr->d,ptr->contextptr);
    return ptr_;
  }

  gen _VAS_positive(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    vecteur v;
    if (g.type==_VECT && g.subtype!=_SEQ__VECT)
      v=*g._VECTptr;
    else
      v=symb2poly_num(g,contextptr);
    return VAS_positive_roots(v,1,0,0,1,contextptr);
  }
  static const char _VAS_positive_s []="VAS_positive";
  static define_unary_function_eval (__VAS_positive,&_VAS_positive,_VAS_positive_s);
  define_unary_function_ptr5( at_VAS_positive ,alias_at_VAS_positive,&__VAS_positive,0,true);

  // square-free factorization of p, then remove all exponents
  // optionally remove factors with even multiplicities
  modpoly remove_multiplicities(const modpoly & p,factorization & f,bool odd_only,GIAC_CONTEXT){
    vecteur res(1,1),tmp;
    polynome P;
    poly12polynome(p,1,P,1);
    P=P/lgcd(P);
    f=sqff(P);
    factorization::const_iterator it=f.begin(),itend=f.end();
    for (;it!=itend;++it){
      if (odd_only && it->mult%2==0)
	continue;
      polynome2poly1(it->fact,1,tmp);
      res=operator_times(res,tmp,0);
    }
    return res;
  }

  gen vas(const modpoly & p,GIAC_CONTEXT){
    vecteur v(p);
    vecteur res1,res2;
    bool has_zero=false;
    if (is_zero(v.back())){
      has_zero=true;
      v.pop_back();
    }
    vecteur w(v);
    change_scale(w,-1);
    if (w.size()%2==0)
      w=-w;
    if (w==v){
      res1=VAS_positive_roots(v,1,0,0,1,contextptr);
      res2=-res1;
      reverse(res2.begin(),res2.end());
      iterateur it=res2.begin(),itend=res2.end();
      for (;it!=itend;++it){
	if (it->type==_VECT)
	  reverse(it->_VECTptr->begin(),it->_VECTptr->end());
      }
      if (has_zero)
	res2.push_back(0);
      res1=mergevecteur(res2,res1);
    }
    else {
#ifdef HAVE_LIBPTHREAD
      int nthreads=threads_allowed?threads:1;
      if (nthreads>1 && p.size()>64){
	pthread_t tab0;
	thread_vas_t tmp0={&v,&res1,1,0,0,1,contextptr};
	for (int i=0;i<v.size();++i){
	  if (v[i].type==_ZINT)
	    v[i]=*v[i]._ZINTptr;
	}
	thread_vas_t tmp1={&w,&res2,-1,0,0,1,contextptr};
	bool res=true;
	res=pthread_create(&tab0,(pthread_attr_t *) NULL,do_thread_vas_t,(void *) &tmp0);
	if (res)
	  do_thread_vas_t(&tmp0);
	do_thread_vas_t(&tmp1);
	void *ptr=&nthreads;
	pthread_join(tab0,&ptr);
      }
      else 
#endif
	{
          res1=VAS_positive_roots(v,1,0,0,1,contextptr);
          res2=VAS_positive_roots(w,-1,0,0,1,contextptr);
        }
      if (has_zero)
	res2.push_back(0);
      res1=mergevecteur(res2,res1);
    }
    return res1;
  }
  gen _VAS(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    vecteur v;
    if (g.type==_VECT && g.subtype!=_SEQ__VECT)
      v=*g._VECTptr;
    else
      v=symb2poly_num(g,contextptr);
    factorization f;
    v=remove_multiplicities(v,f,false,contextptr);
    return vas(v,contextptr);
  }
  static const char _VAS_s []="VAS";
  static define_unary_function_eval (__VAS,&_VAS,_VAS_s);
  define_unary_function_ptr5( at_VAS ,alias_at_VAS,&__VAS,0,true);

  static const double bisection_newton_eps=1e-3;

  // returns true if a root of p is found by Newton method, such that res-eps>a0
  // res+eps<b0 and sign of p changes between res-eps and res+eps
  static bool bisection_newton(const modpoly & P,const modpoly & Pprime,const gen & a0,const gen & a1,gen & x0,double eps,gen & eps2,GIAC_CONTEXT){
    if (is_greater(bisection_newton_eps,x0-a0,contextptr) || is_greater(bisection_newton_eps,a1-x0,contextptr))
      return false; // bisection is faster if the root is too close to the isolation interval boundaries
    int n=int(-std::log(eps)/std::log(2.0)+.5); // for rounding
    eps2=pow(2,-(n+1),contextptr);
    gen deuxn4(pow(2,n+4,contextptr));
    in_round2(x0,deuxn4,n+4); // round2(x0,deuxn4,contextptr);
    for (int ii=0;ii<n;ii++){
      gen Pprimex0=horner(Pprime,x0,0,false);
      if (is_zero(Pprimex0,contextptr))
	return false;
      gen Px0=horner(P,x0,0,false);
      in_round2(Px0,deuxn4,n+4); // round2(Px0,deuxn4,contextptr);
      in_round2(Pprimex0,deuxn4,n+4); // round2(Pprimex0,deuxn4,contextptr);
      gen dx=Px0/Pprimex0;
      in_round2(dx,deuxn4,n+4); // round2(dx,deuxn4,contextptr);
      x0=x0-dx;
      if (is_positive(a0-x0,contextptr) || is_positive(x0-a1,contextptr))
	return false;
      if (is_greater(abs(dx,contextptr),eps2,contextptr))
	continue;
      if (is_positive(-horner(P,x0-eps2,0,false)*horner(P,x0+eps2,0,false),contextptr))
	return true;
    }
    return false;
    
  }

  gen bisection(const modpoly & p,const gen & a0,const gen &b0,double eps,GIAC_CONTEXT){
    int nsteps=int(std::ceil(std::log(evalf_double(b0-a0,1,contextptr)._DOUBLE_val/eps)/M_LN2));
    int trynewtonstep=int(nsteps-std::log(bisection_newton_eps/eps)/M_LN2);
    gen a(a0),b(b0),m,eps2;
    modpoly dp=derivative(p);
    int s1=fastsign(horner(p,a,0,false),contextptr);
    if (s1==0)
      s1=fastsign(horner(dp,a,0,false),contextptr);
    int s2=fastsign(horner(p,b,0,false),contextptr);
    if (s2==0)
      s2=-fastsign(horner(dp,b,0,false),contextptr);
    if (s1==s2)
      return undef;
    for (int i=0;i<nsteps;++i){
      m=(a+b)/2;
      if (i==trynewtonstep && bisection_newton(p,dp,a0,b0,m,eps,eps2,contextptr))
	return makevecteur(m-eps2,m+eps2);
      int s=fastsign(horner(p,m,0,false),contextptr);
      if (s==0)
	return m;
      if (s==s1)
	a=m;
      else
	b=m;
    }
    return makevecteur(a,b);
  }

  int multiplicity(const factorization & f,const gen & interval,GIAC_CONTEXT){
    factorization::const_iterator it=f.begin(),itend=f.end();
    if (interval.type==_VECT && interval._VECTptr->size()==2){
      for (;it!=itend;++it){
	if (is_strictly_positive(-it->fact(interval._VECTptr->front())*it->fact(interval._VECTptr->back()),contextptr))
	  return it->mult;
      }
      for (it=f.begin();it!=itend;++it){
	if (is_positive(-it->fact(interval._VECTptr->front())*it->fact(interval._VECTptr->back()),contextptr))
	  return it->mult;
      }
    }
    else {
      for (;it!=itend;++it){
	if (is_zero(it->fact(interval)))
	  return it->mult;
      }
    }
    return 0;
  }

  static void add_vasres(vecteur & vasres,const gen & a,const gen & a0,const gen & b0,int mult,bool with_mult,GIAC_CONTEXT){
    if (a0==b0 || (is_greater(a,a0,contextptr) && is_greater(b0,a,contextptr)) )
      vasres.push_back(with_mult?gen(makevecteur(a,mult)):a);
  }

  // isolate and find real roots of P at precision eps between a and b
  // returns a list of intervals or of rationals
  bool vas(const modpoly & P,const gen & a0,const gen &b0,double eps,vecteur & vasres,bool with_mult,GIAC_CONTEXT){
    if (eps<=0)
      eps=1e-12;
    if (P.size()<=3){
      if (P.size()<2)
	return true;
      gen a(P[0]),b(P[1]);
      if (P.size()==2){
	a=-b/a;
	add_vasres(vasres,a,a0,b0,1,with_mult,contextptr);
	return true;
      }
      gen c(P[2]);
      gen delta=b*b-4*a*c;
      if (is_zero(delta)){
	a=-b/a/2;
	add_vasres(vasres,a,a0,b0,2,with_mult,contextptr);
	return true;	
      }
      if (is_positive(delta,contextptr)){
	delta=sqrt(delta,contextptr)/a/2;
	c=-b/a/2;
	add_vasres(vasres,c-delta,a0,b0,1,with_mult,contextptr);
	add_vasres(vasres,c+delta,a0,b0,1,with_mult,contextptr);
      }
      return true;
    }
    gen a(a0),b(b0);
    if (a==b){
      a=minus_inf;
      b=plus_inf;
    }
    // check and convert coeffs of P
    modpoly p(P);
    iterateur it=p.begin(),itend=p.end();
    for (;it!=itend;++it){
      *it=exact(*it,contextptr);
    }
    gen tmp;
    lcmdeno(p,tmp,contextptr);
    for (it=p.begin();it!=itend;++it){
      if (!is_integer(*it))
	return false;
    }
    p=divvecteur(p,lgcd(p));
    factorization f;
    p=remove_multiplicities(p,f,false,contextptr);
    tmp=vas(p,contextptr);
    if (tmp.type!=_VECT)
      return false;
    vecteur v=*tmp._VECTptr;
    // now improve precision by bisection 
    it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (it->type!=_VECT){
	if (is_greater(*it,a,contextptr) && is_greater(b,*it,contextptr)){
	  if (with_mult){
	    int n=multiplicity(f,*it,contextptr);
	    vasres.push_back(makevecteur(*it,n));
	  }
	  else
	    vasres.push_back(*it);
	}
	continue;
      }
      if (it->_VECTptr->size()!=2)
	return false;
      gen A=it->_VECTptr->front(),B=it->_VECTptr->back();
      if (is_strictly_greater(a,B,contextptr) || is_strictly_greater(A,b,contextptr))
	continue;
      gen interval=bisection(p,max(A,a,contextptr),min(B,b,contextptr),eps,contextptr);
      if (is_undef(interval))
	continue;
      if (interval.type==_VECT)
	interval.subtype=_INTERVAL__VECT;
      if (with_mult)
	vasres.push_back(makevecteur(interval,multiplicity(f,interval,contextptr)));
      else
	vasres.push_back( (interval.type==_VECT && interval._VECTptr->size()==2)?evalf((interval._VECTptr->front()+interval._VECTptr->back())/2,1,contextptr):interval);
    }
    return true;
  }

  /* 
*********************************
numerical root localization 
*********************************     
*/
  
  double cluster_step=1e-4; // if step>cluster_step, no cluster analysis, export GIAC_STEP=1e-3
  double cluster_dp=0.2; // if step*sum_{j!=i}(inv(approx_root[i]-approx_root[j])>cluster_dp, then do cluster analysis, export GIAC_DP=.1
  //int debug_infolevel=0; // export GIAC_DEBUG=1 or 2
  double MINREAL=-1e307;

// single precision
#if defined __x86_64__ || defined __i386__ // || defined EMCC
#define LDBL80 // 80 bit long double
  typedef long double longdouble;
#else
  typedef double longdouble;
#endif

typedef complex<longdouble> fdbl;
//typedef complex<float> fdbl;

inline double absdbl(const fdbl & x){
  return abs(x);
}
inline fdbl re(const fdbl & x){
  return fdbl(x.real(),0);
}
inline fdbl conj(const fdbl & x){
  return fdbl(x.real(),x.imag());
}
inline double redbl(const fdbl & x){
  return x.real();
}
inline double imdbl(const fdbl & x){
  return x.imag();
}
inline bool is_exactly_zero(const fdbl & x){
  return x.real()==0 && x.imag()==0;
}
fdbl inv(const fdbl z){
  return fdbl(1.0)/z;
}

bool fdbl_less(const fdbl & x,const fdbl & y){
  if (x.real()!=y.real())
    return x.real()<y.real();
  return x.imag()<y.imag();
}



typedef vector<fdbl> vfdbl;
#ifdef LDBL80
ostream & operator << (ostream & os,const vfdbl & P){
  os << "[";
  for (int i=0;i<P.size();++i){
    os << P[i].real() << "+" << P[i].imag() << "*i";
    if (i!=P.size()-1)
      os << ",";
  }
  os << "]";
  return os;
}
#endif

void print(const vfdbl & P){
  cout << P << "\n";
}

fdbl horner_rem(const vfdbl & P,fdbl x,vfdbl * Q){
  if (Q)
    Q->clear(); 
  if (P.empty())
    return 0.0;
  size_t s=P.size();
  if (Q)
    Q->reserve(s-1);
  fdbl r=0;
  if (Q){
    for (size_t i=0;;){
      r=r*x+P[i];
      ++i;
      if (i==s)
	break;
      Q->push_back(r);
    }
  }
  else {
    for (size_t i=0;i<s;++i){
      r=r*x+P[i];
    }
  }
  return r;
}

double l1(const vfdbl & P,const fdbl & z){
  double r=absdbl(z),rn=1,res=0;
  for (size_t i=1;i<=P.size();++i){
    double anrn=absdbl(P[P.size()-i])*rn;
    rn *= r;
    if (anrn<=res)
      continue;
    res=anrn;
  }
  return res;
}

double l1(const vfdbl & P){
  double res=0;
  for (size_t i=1;i<=P.size();++i){
    double an=absdbl(P[P.size()-i]);
    if (an<=res)
      continue;
    res=an;
  }
  return res;
}

// shift roots such that sum of roots=0
fdbl find_shift(const vfdbl & R){
  int n=R.size()-1;
  if (n==0)
    return 0;
  fdbl d=R[1]/fdbl(n)/R[0];
  return fdbl(floor(d.real()),floor(d.imag()));
}

vfdbl shift(const vfdbl & P0,fdbl & d,bool l1chk=true){
  vfdbl P(P0),Q,R;
  double D=l1(P0);
  if (isinf(D)){
    d=0;
    return P0;
  }
  int n=P.size();
  for (int i=0;i<n;++i){
    fdbl Pz=horner_rem(P,d,&Q);
    if (l1chk && absdbl(Pz)>D){
      d=0;
      return P0; // no shift, it would increase the l1 norm
    }
    R.push_back(Pz);
    P.swap(Q);
  }
  reverse(R.begin(),R.end());
  return R;
}

fdbl sum(const vfdbl & P){
  fdbl r(0.0);
  for (size_t i=0;i<P.size();++i)
    r += P[i];
  return r;
}

double sumabs(const vfdbl & P){
  double r(0.0);
  for (size_t i=0;i<P.size();++i)
    r += absdbl(P[i]);
  return r;
}

bool linreg(const vector<double> & x,const vector<double> & y,double & a,double &b,double & r){
  size_t n=x.size();
  if (n!=y.size())
    return false;
  double X=0,Y=0,XY=0,X2=0,Y2=0;
  for (size_t i=0;i<n;++i){
    X += x[i];
    Y += y[i];
  }
  X /= n; Y /= n;
  for (size_t i=0;i<n;++i){
    double xx=x[i]-X,yy=y[i]-Y;
    XY += xx*yy;
    X2 += xx*xx;
    Y2 += yy*yy;
  }
  if (X2==0)
    return false;
  XY /= n; X2 /= n; Y2 /= n;
  a=XY/X2;
  b=Y-a*X;
  r=Y2==0?1:XY/std::sqrt(X2)/std::sqrt(Y2);
  return true;
}

double find_scale(const vfdbl & P){
  int n=P.size()-1;
  vector<double> x,y;
  for (int i=0;i<=n;++i){
    double a=absdbl(P[i]);
    if (a==0)
      continue;
    if (isinf(a))
      return 1;
    x.push_back(n-i);
    y.push_back(std::log(a));
  }
  if (x.empty())
    return 1;
  double a,b,r;
  linreg(x,y,a,b,r);
  return std::exp(-a);
}

void rescale(vfdbl & P, fdbl l){
  if (l==fdbl(1)) return;
  fdbl ll=l;
  for (int i=P.size()-2;i>=0;--i){
    P[i] = ll*P[i];
    ll = ll*l;
  }
  fdbl c=P[0];
  for (size_t i=0;i<P.size();++i)
    P[i] = P[i]/c;
}



struct int_2double {
  int i;
  double theta,norm;
};

struct int_double {
  int i;
  double d;
};

bool norm_sort(const int_double & a,const int_double & b){
  if (a.d!=b.d)
    return a.d<b.d;
  return a.i<b.i;
}

double norm(double dx,double dy){
  if (dx==0 && dy==0)
    return 0;
  if (dx<0) dx=-dx;
  if (dy<0) dy=-dy;
  if (dx>dy){
    double z=dy/dx;
    return dx*std::sqrt(1+z*z);
  }
  else {
    double z=dx/dy;
    return dy*std::sqrt(1+z*z);
  }
}

bool graham_sort_function(const int_2double & a,const int_2double & b){
  if (a.theta==b.theta)
    return b.norm>a.norm;
  return b.theta>a.theta;
}

double cross_prod(const vfdbl & v,int a,int b,int c){
  fdbl ab=v[b]-v[a],ac=v[c]-v[a];
  double A=redbl(ab),B=imdbl(ab),C=redbl(ac),D=imdbl(ac);
  return A*D-B*C;
}

vector<int> convexhull(const vfdbl & v){
  int s=v.size(),imin=0;
  if (s==1)
    return vector<int>(1,0);
  // find origin
  double ymin=imdbl(v[0]),ycur,xmin=redbl(v[0]),xcur;
  for (int i=1;i<s;++i){
    ycur=imdbl(v[i]);xcur=redbl(v[i]);
    if (ymin>ycur || (ymin==ycur && xmin>xcur) ){
      imin=i; ymin=ycur; xmin=xcur;
    }
  }
  vector<int_2double> ls;
  for (int j=0;j<s;++j){
    if (j!=imin){
      double dx=redbl(v[j])-xmin,dy=imdbl(v[j])-ymin;
      int_2double s={j,atan2(dy,dx),norm(dx,dy)};
      ls.push_back(s);
    }
  }
  sort(ls.begin(),ls.end(),graham_sort_function);
  vector<int> res; res.push_back(imin); res.push_back(ls[0].i);
  int ress=2;
  for (int j=1;j<s-1;++j){
    int icur=ls[j].i;
    double o=cross_prod(v,res[ress-2],res[ress-1],icur);
    if (o==0)
      res[ress-1]=icur;
    else {
      if (o>0){
        res.push_back(icur);
        ++ress;
      }
      else {
        while (ress>2 && o<0){
          res.pop_back();
          ress--;
          o=cross_prod(v,res[ress-2],res[ress-1],icur);
        }
        res.push_back(icur);
        ++ress;
      }
    }
  }
  return res;
}

double init_R(const vfdbl & P,vfdbl & R){
  R.clear();
  int n=P.size()-1;
  if (n && is_exactly_zero(P[n])){
    vfdbl P1(P);
    P1.pop_back();
    double res=init_R(P1,R);
    R.insert(R.begin(),0);
    return res;
  }
  vfdbl l;
  vector<int> lpos;
  double M=0;
  for (int i=0;i<=n;++i){
    longdouble ai=abs(P[n-i]);
    if (ai==0)
      continue;
    l.push_back(fdbl(double(i),std::log(ai)));
    lpos.push_back(i);
  }
  vector<int> pos=convexhull(l);
  // find real positions (since coeffs==0 were removed)
  for (int i=0;i<pos.size();++i){
    pos[i]=lpos[pos[i]];
  }
  // remove lower part of convexhull, pos[0] has the minimal y, 
  for (int i=0;i<pos.size();++i){
    if (i+1==pos.size() || pos[i]>pos[i+1]){
      pos.erase(pos.begin(),pos.begin()+i); 
      break;
    }
  }
  // now pos starts with highest value in x
  for (int i=0;i+1<pos.size();++i){
    if (pos[i]<pos[i+1]){
      // pos[i] is the lowest value in x
      pos.erase(pos.begin()+i+1,pos.end());
      break;
    }
  }
  sort(pos.begin(),pos.end());
  if (pos[0]!=0)
    pos.insert(pos.begin(),0);
  if (pos.back()!=n)
    pos.push_back(n);
  int count=0;
  for (int i=1;i<pos.size();++i){
    int dk=pos[i]-pos[i-1];
    double uk=std::pow(std::abs(P[n-pos[i]]/P[n-pos[i-1]]),-1.0/dk);
    if (uk==0) return 0;
    if (uk>M)
      M=uk;
    double sigma=0.7;
    for (int j=0;j<dk;++j){
      double theta=sigma+2*M_PI*i/n+2*M_PI*j/dk;
      fdbl z(uk*std::cos(theta),uk*std::sin(theta));
      R.push_back(z);
    }
  }
  return M;
}

// find r=P(x) and r1=diff(P)(x)
// if |x|>1 this may overflow
void horner2(const vfdbl & P,fdbl x,fdbl & r,fdbl & r1){
  r=r1=0;
  if (P.empty())
    return ;
  size_t s=P.size()-1;
  for (size_t i=0;i<s;++i){
    r=r*x+P[i];
    r1=r1*x+r;
  }
  r=r*x+P[s];
}

double l1norm(const vfdbl & v){
  double r=0;
  for (int i=0;i<v.size();++i)
    r += absdbl(v[i]);
  return r;
}

void div(vfdbl & v,double r){
  for (int i=0;i<v.size();++i)
    v[i] /= r;
}

// approx largest root in norm, using a few iterations of power method
double largest(const vfdbl & fP,int maxiter=100,double eps=1e-6){
  int deg=fP.size()-1;
  //if (debug_infolevel) *logptr(contextptr) << "power method " << fP << "\n";
  vfdbl v(deg);
  for (int i=0;i<deg;++i)
    v[i]=fdbl(std_rand()/(1.0+RAND_MAX),std_rand()/(1.0+RAND_MAX));
  div(v,l1norm(v));
  vfdbl w(deg);
  double oldR(0.0);
  for (int j=0;j<maxiter;++j){
    for (int i=0;i<deg-1;++i){
      w[i+1]=v[i];
    }
    fdbl r(0,0);
    for (int i=0;i<deg;++i){
      r += fP[i+1]*v[i];
    }
    w[0]= -r/fP[0];
    double R=l1norm(w);
    if (debug_infolevel)
      *logptr(context0) << "power method " << j << " " << R << "\n";
    div(w,R);
    if (j>5 && abs(1-R/oldR)<eps)
      return R;
    v.swap(w);
    oldR=R;
  }
  return oldR;
}

#define REV 1

void translate_shift(vfdbl & R,const fdbl & l,const fdbl & dr){
  if (l==fdbl(1) && dr==fdbl(0))
    return;
  // rescale and translate
  for (size_t j=0;j<R.size();++j){
    R[j] = l*R[j] + dr;
  }
}

bool read_poly(const string & filename,vfdbl & P){
  P.clear();
  ifstream i(filename.c_str());
  while (1){
    fdbl z;
    i >> z;
    if (i.eof())
      break;
    P.push_back(z);
  }
  return true;
}

vfdbl p_coeff(const vfdbl & R){
  vfdbl P;
  P.reserve(R.size()+1);
  P.push_back(fdbl(1));
  for (size_t i=0;i<R.size();++i){
    fdbl z=R[i];
    // P*(x-z)
    P.push_back(-z*P.back());
    for (size_t j=P.size()-2;j>=1;--j){
      P[j] -= z*P[j-1];
    }
  }
  return P;
}

// max distance between 2 elements of R
double ecart(const vfdbl & R){
  int n=R.size();
  double d=1e307;
  for (int i=0;i<n;++i){
    const fdbl & Ri=R[i];
    for (int j=0;j<n;++j){
      if (j==i) continue;
      double dd=absdbl(Ri-R[j]);
      if (dd<d)
        d=dd;
    }
  }
  return d;
}

// multi-precision
typedef gen dbl;
struct int_dbl {
  int i;
  dbl d;
};

bool norm_sort_dbl(const int_dbl & a,const int_dbl & b){
  if (a.d!=b.d)
    return is_strictly_greater(b.d,a.d,context0);
  return is_strictly_greater(b.i,a.i,context0);
}

inline double absdbl(const dbl & x){
  dbl g=abs(evalf_double(x,1,context0),context0);
  return g.type==_DOUBLE_?g._DOUBLE_val:0;
}
inline double logabsdbl(const dbl & x){
  if (is_exactly_zero(x))
    return MINREAL; // -707;
  dbl g=x;
  if (g.type!=_REAL)
    g=accurate_evalf(g,128);
  g=ln(abs(g,context0),context0);
  g=evalf_double(g,1,context0);
  return g.type==_DOUBLE_?g._DOUBLE_val:0;
}
inline double redbl(const dbl & x){
  return evalf_double(re(x,context0),1,context0)._DOUBLE_val;
}
inline double imdbl(const dbl & x){
  return evalf_double(im(x,context0),1,context0)._DOUBLE_val;
}
inline dbl conj(const dbl & x){
  return conj(x,context0);
}
inline dbl re(const dbl & x){
  return re(x,context0);
}
inline dbl im(const dbl & x){
  return im(x,context0);
}
inline dbl exp(const dbl & x){
  return exp(x,context0);
}
inline dbl inv(const dbl & x){
  return inv(x,context0);
}
bool dbl_less(const dbl & x,const dbl & y){
  gen xr,xi,yr,yi;
  reim(x,xr,xi,context0);
  reim(y,yr,yi,context0);
  if (xi==0 && yi!=0)
    return true;
  if (xi!=0 && yi==0)
    return false;
  if (xr!=yr)
    return is_strictly_greater(yr,xr,context0);
  return is_strictly_greater(yi,xi,context0);
  //return !islesscomplexthanf(x,y);
}

bool dblrayon_less(const dbl & x,const dbl & y){
  return dbl_less(x[0],y[0]);
}

typedef vecteur vdbl;
#if 0
ostream & operator << (ostream & os,const vdbl & P){
  for (int i=0;i<P.size();++i)
    os << P[i] << " ";
  return os;
}
#endif

void accurate_evalf(vdbl & P,int nbits){
  for (int i=0;i<P.size();++i)
    P[i]=accurate_evalf(P[i],nbits);
}

dbl horner_rem(const vdbl & P,dbl x,vdbl * Q){
  if (Q)
    Q->clear(); 
  if (P.empty())
    return 0.0;
  size_t s=P.size();
  if (Q)
    Q->reserve(s-1);
  dbl r=0;
  if (Q){
    for (size_t i=0;;){
      r=r*x+P[i];
      ++i;
      if (i==s)
	break;
      Q->push_back(r);
    }
  }
  else {
    for (size_t i=0;i<s;++i){
      r=r*x+P[i];
    }
  }
  return r;
}

double loglinf(const vdbl & P,const dbl & z){
  double r=logabsdbl(z),rn=1,res=MINREAL;
  for (size_t i=1;i<=P.size();++i){
    double anrn=logabsdbl(P[P.size()-i])*rn;
    rn *= r;
    if (anrn<=res)
      continue;
    res=anrn;
  }
  return res;
}

double loglinf(const vdbl & P){
  double res=MINREAL;
  for (size_t i=1;i<=P.size();++i){
    double an=logabsdbl(P[P.size()-i]);
    if (an<=res)
      continue;
    res=an;
  }
  return res;
}

// shift roots such that sum of roots=0
dbl find_shift(const vdbl & R,GIAC_CONTEXT){
  int n=R.size()-1;
  if (n==0)
    return 0;
  return _floor(R[1]/dbl(n)/R[0],contextptr);
}

vdbl shift(const vdbl & P0,dbl & d,bool l1chk=true){
  double D=loglinf(P0);
#if 0
  dbl chk=horner_rem(P0,d,0);
  if (l1chk && logabsdbl(chk)>D){
    d=0;
    return P0;
  }
  vdbl res=taylor(P0,d);
  for (int i=0;i<res.size();++i){
    if (l1chk && logabsdbl(res[i])>D){
      d=0;
      return P0; // no shift, it would increase the linf norm of the polynomial
    }
  }
  return res;
#endif
  vdbl P(P0),Q,R;
  int n=P.size();
  for (int i=0;i<n;++i){
    dbl Pz=horner_rem(P,d,&Q);
    if (l1chk && logabsdbl(Pz)>D){
      d=0;
      return P0; // no shift, it would increase the linf norm of the polynomial
    }
    R.push_back(Pz);
    P.swap(Q);
  }
  reverse(R.begin(),R.end());
  return R;
}

dbl sum(const vdbl & P){
  dbl r(0.0);
  for (size_t i=0;i<P.size();++i)
    r += P[i];
  return r;
}

double sumabs(const vdbl & P){
  double r(0.0);
  for (size_t i=0;i<P.size();++i)
    r += absdbl(P[i]);
  return r;
}

double find_scale(const vdbl & P){
  int n=P.size()-1;
  vector<double> x,y;
  for (int i=0;i<=n;++i){
    double a=logabsdbl(P[i]);
    if (a==MINREAL)
      continue;
    x.push_back(n-i);
    y.push_back(a);
  }
  if (x.empty())
    return 1;
  double a,b,r;
  linreg(x,y,a,b,r);
  return std::exp(-a);
}

void rescale(vdbl & P, dbl l){
  if (l==dbl(1)) return;
  dbl ll=l;
  for (int i=P.size()-2;i>=0;--i){
    P[i] = ll*P[i];
    ll = ll*l;
  }
  dbl c=P[0];
  for (size_t i=0;i<P.size();++i)
    P[i] = P[i]/c;
}

// find r=P(x) and r1=diff(P)(x)
// if |x|>1 this may overflow
void horner2(const vdbl & P,dbl x,dbl & r,dbl & r1){
  r=r1=0;
  if (P.empty())
    return ;
  size_t s=P.size()-1;
  for (size_t i=0;i<s;++i){
    r=r*x+P[i];
    r1=r1*x+r;
  }
  r=r*x+P[s];
}

#ifdef HAVE_LIBMPFR
// computes an estimate of the number of bits of precision lost
void add(mpfr_t & rr,mpfr_t & ri,const gen & Pi,int nbits,long & loss,long & size){
  long rr0,rr1,delta=0;
  mpfr_get_d_2exp(&rr0,rr,MPFR_RNDN);
  if (rr0>size)
    size=rr0;
  if (Pi.type==_REAL){
    mpfr_t & inf=Pi._REALptr->inf;
    if (mpfr_zero_p(inf))
      return ;
    mpfr_get_d_2exp(&rr1,inf,MPFR_RNDN);
    if (rr1>rr0){
      if (rr1>size)
        size=rr1;
      delta=rr1-rr0;
      rr0=rr1;
    }
    mpfr_add(rr,rr,Pi._REALptr->inf,MPFR_RNDN);
  }
  else if (Pi.type==_CPLX){
    mpfr_t & rinf=Pi._CPLXptr->_REALptr->inf;
    mpfr_get_d_2exp(&rr1,rinf,MPFR_RNDN);
    if (rr1>rr0){
      if (rr1>size)
        size=rr1;
      delta=rr1-rr0;
      rr0=rr1;
    }
    mpfr_add(rr,rr,rinf,MPFR_RNDN);
    mpfr_add(ri,ri,(Pi._CPLXptr+1)->_REALptr->inf,MPFR_RNDN);
  }
  else if (Pi.type==_INT_){
    mpfr_add_si(rr,rr,Pi.val,MPFR_RNDN);
    return ;
  }
  else exit(1);
  mpfr_get_d_2exp(&rr1,rr,MPFR_RNDN);
  if (delta>nbits)
    delta=nbits;
  if (loss>delta)
    loss -= delta;
  else
    loss = 0;
  if (rr1<rr0){
    loss += rr0-rr1;
  }
}

void mult(mpfr_t & rr,mpfr_t & ri,const gen & x,mpfr_t & tmp,mpfr_t & tmp1, mpfr_t & tmp2){
  if (x.type==_REAL){
    mpfr_mul(rr,rr,x._REALptr->inf,MPFR_RNDN);
    //if (!mpfr_zero_p(ri))
      mpfr_mul(ri,ri,x._REALptr->inf,MPFR_RNDN);
    return;
  }
  if (x.type==_CPLX){
#if 0
    mpfr_fmms(tmp1,rr,x._CPLXptr->_REALptr->inf,ri,(x._CPLXptr+1)->_REALptr->inf,MPFR_RNDN);
    mpfr_fmma(ri,rr,(x._CPLXptr+1)->_REALptr->inf,ri,x._CPLXptr->_REALptr->inf,MPFR_RNDN);
    mpfr_swap(tmp1,rr);
    return;
#endif
#if 0
    // (rr+i*ri)*(xr+i*xi)=rr*xr-ri*xi+i*(rr*xi+ri*xr)
    mpfr_mul(tmp1,rr,x._CPLXptr->_REALptr->inf,MPFR_RNDN);
    mpfr_mul(tmp2,ri,(x._CPLXptr+1)->_REALptr->inf,MPFR_RNDN);
    // imag part = (rr+ri)*(xr+xi)-(rr*xr+ri*xi)
    mpfr_add(rr,rr,ri,MPFR_RNDN);
    mpfr_add(ri,x._CPLXptr->_REALptr->inf,(x._CPLXptr+1)->_REALptr->inf,MPFR_RNDN);
    mpfr_mul(tmp,rr,ri,MPFR_RNDN);
    mpfr_sub(tmp,tmp,tmp1,MPFR_RNDN);
    mpfr_sub(ri,tmp,tmp2,MPFR_RNDN);
    mpfr_sub(rr,tmp1,tmp2,MPFR_RNDN);
    return;
#endif
    mpfr_mul(tmp1,rr,x._CPLXptr->_REALptr->inf,MPFR_RNDN);
    mpfr_mul(tmp2,ri,(x._CPLXptr+1)->_REALptr->inf,MPFR_RNDN);
    mpfr_sub(tmp,tmp1,tmp2,MPFR_RNDN);
    mpfr_mul(tmp1,rr,(x._CPLXptr+1)->_REALptr->inf,MPFR_RNDN);
    mpfr_mul(tmp2,ri,x._CPLXptr->_REALptr->inf,MPFR_RNDN);
    mpfr_swap(tmp,rr);
    mpfr_add(ri,tmp1,tmp2,MPFR_RNDN);
    return;
  }
  if (x.type==_INT_){
    mpfr_mul_si(rr,rr,x.val,MPFR_RNDN);
    //if (!mpfr_zero_p(ri))
      mpfr_mul_si(ri,ri,x.val,MPFR_RNDN);
    return;
  }
  exit(1);
}

void mult(mpfr_t & rr,mpfr_t & ri,const mpfr_t & xr,const mpfr_t & xi,mpfr_t & tmp,mpfr_t & tmp1, mpfr_t & tmp2){
  if (mpfr_zero_p(xi)){
    mpfr_mul(rr,rr,xr,MPFR_RNDN);
    mpfr_mul(ri,ri,xr,MPFR_RNDN);
    return;
  }
  mpfr_mul(tmp1,rr,xr,MPFR_RNDN);
  mpfr_mul(tmp2,ri,xi,MPFR_RNDN);
  mpfr_sub(tmp,tmp1,tmp2,MPFR_RNDN);
  mpfr_mul(tmp1,rr,xi,MPFR_RNDN);
  mpfr_mul(tmp2,ri,xr,MPFR_RNDN);
  mpfr_swap(tmp,rr);
  mpfr_add(ri,tmp1,tmp2,MPFR_RNDN);
}

// find r=P(x) and r1=diff(P)(x)
// returns largest number of bits of mantissa in intermediate computations
long horner2_mpfr(const vdbl & P,const dbl & x,dbl & r,dbl & r1,int nbits,long & size,bool pdiff){
  if (P.empty())
    return 0;
  long loss=0;
  size=-RAND_MAX;
  size_t s=P.size()-1;
  mpfr_t rr,ri,r1r,r1i,tmp,tmp1,tmp2;
  mpfr_init2(rr,nbits); mpfr_set_si(rr,0,MPFR_RNDN);
  mpfr_init2(ri,nbits); mpfr_set_si(ri,0,MPFR_RNDN);
  mpfr_init2(r1r,nbits); mpfr_set_si(r1r,0,MPFR_RNDN);
  mpfr_init2(r1i,nbits); mpfr_set_si(r1i,0,MPFR_RNDN);
  mpfr_init2(tmp,nbits);
  mpfr_init2(tmp1,nbits);
  mpfr_init2(tmp2,nbits);
  for (size_t i=0;i<s;++i){
    // r=r*x+P[i];
    mult(rr,ri,x,tmp,tmp1,tmp2);
    add(rr,ri,P[i],nbits,loss,size);
    if (pdiff){
      // r1=r1*x+r;
      mult(r1r,r1i,x,tmp,tmp1,tmp2);
      mpfr_add(r1r,r1r,rr,MPFR_RNDN);
      mpfr_add(r1i,r1i,ri,MPFR_RNDN);
    }
  }
  // r=r*x+P[s];
  mult(rr,ri,x,tmp,tmp1,tmp2);
  add(rr,ri,P[s],nbits,loss,size);
  // end
  r=gen(real_object(rr),real_object(ri));
  if (pdiff)
    r1=gen(real_object(r1r),real_object(r1i));
  mpfr_clear(rr);
  mpfr_clear(ri);
  mpfr_clear(r1r);
  mpfr_clear(r1i);
  mpfr_clear(tmp);
  mpfr_clear(tmp1);
  mpfr_clear(tmp2);
  return loss;
}

#endif

bool convert(const gen & g,longdouble & z){
  if (g.type==_INT_){
    z=g.val;
    return true;
  }
  if (g.type==_DOUBLE_){
    z=g._DOUBLE_val;
    return true;
  }
#ifndef USE_GMP_REPLACEMENTS
  if (g.type==_REAL){
#ifdef HAVE_LIBMPFR
    z=mpfr_get_ld(g._REALptr->inf,MPFR_RNDN);
#else
    z=mpf_get_d(g._REALptr->inf);
#endif
    return true;
  }
#endif
  if (g.type==_FRAC){
    longdouble n,d;
    if (convert(g._FRACptr->num,n) && convert(g._FRACptr->den,d)){
      z=n/d;
      return true;
    }
    return false;
  }
  if (g.type!=_ZINT)
    return false;
  int s=mpz_cmp_si(*g._ZINTptr,0);
  int l=mpz_sizeinbase(*g._ZINTptr,2);
  if (l>=(1<<15))
    return false;
  mpz_t zz; mpz_init(zz);
  if (l>64){
    // we have 64 bits of mantissa
#ifdef USE_GMP_REPLACEMENTS
      mpz_tdiv_q_2exp(zz,*g._ZINTptr,l-64);
#else
      mpz_div_2exp(zz,*g._ZINTptr,l-64);
#endif
  }
  else
    mpz_set(zz,*g._ZINTptr);
  ulonglong u;
  u=mpz_get_ui(zz);
  mpz_clear(zz);
  z=u;
  if (l>64)
    z=z*std::pow((longdouble) 2,l-64);
  if (s<0)
    z=-z;
  return true;
}
      
bool convert(const vdbl & P,vfdbl & fP,GIAC_CONTEXT){
  int s=P.size();
  fP.clear(); fP.reserve(s);
  for (int i=0;i<s;++i){
    dbl real,imag;
    reim(P[i],real,imag,contextptr);
#ifdef LDBL80
    longdouble zr,zi;
    if (!convert(real,zr) || !convert(imag,zi))
      return false;
    fP.push_back(fdbl(zr,zi));
#else
    if (real.type==_ZINT && mpz_sizeinbase(*real._ZINTptr,2)>1000)
      return false;
    if (imag.type==_ZINT && mpz_sizeinbase(*imag._ZINTptr,2)>1000)
      return false;
    fP.push_back(fdbl(evalf_double(real,1,contextptr)._DOUBLE_val,evalf_double(imag,1,contextptr)._DOUBLE_val));
#endif
  }
  return true;
}

void Convert(const vfdbl & fP,vdbl & P){
  int s=fP.size();
  P.clear(); P.reserve(s);
  for (int i=0;i<s;++i){
#if defined HAVE_LIBMPFR && defined LDBL80
    dbl re,im;
    re=accurate_evalf(re,64); im=accurate_evalf(im,64);
    mpfr_set_ld(re._REALptr->inf,fP[i].real(),MPFR_RNDN);
    mpfr_set_ld(im._REALptr->inf,fP[i].imag(),MPFR_RNDN);
    P.push_back(dbl(re,im));
#else
    P.push_back(dbl(double(fP[i].real()),double(fP[i].imag())));
#endif
  }
}

double l1norm(const vdbl & v){
  double r=0;
  for (int i=0;i<v.size();++i)
    r += absdbl(v[i]);
  return r;
}

void div(vdbl & v,double r){
  r=1/r;
  for (int i=0;i<v.size();++i)
    v[i] = r*v[i];
}

#if 0
// approx largest root in norm, using a few iterations of power method
double largest(const vdbl & fP,int maxiter=100,double eps=1e-6){
  int deg=fP.size()-1;
  vdbl v(deg);
  for (int i=0;i<deg;++i)
    v[i]=dbl(std_rand()/(1.0+RAND_MAX),std_rand()/(1.0+RAND_MAX));
  div(v,l1norm(v));
  vdbl w(deg);
  double oldR(0.0);
  for (int j=0;j<maxiter;++j){
    for (int i=0;i<deg-1;++i){
      w[i+1]=v[i];
    }
    dbl r(0);
    for (int i=0;i<deg;++i){
      r += fP[i+1]*v[i];
    }
    w[0]= -r/fP[0];
    double R=l1norm(w);
    if (debug_infolevel)
      *logptr(contextptr) << "power method " << j << " " << R << "\n";
    div(w,R);
    if (j>5 && abs(1-R/oldR)<eps)
      return R;
    v.swap(w);
    oldR=R;
  }
  return oldR;
}

#else
double largest(const vdbl & P,int maxiter=100,double eps=1e-6){
  vfdbl fP; convert(P,fP,context0);
  return largest(fP,maxiter,eps);
}
#endif

double cross_prod(const vdbl & v,int a,int b,int c){
  dbl ab=v[b]-v[a],ac=v[c]-v[a];
  double A=redbl(ab),B=imdbl(ab),C=redbl(ac),D=imdbl(ac);
  return A*D-B*C;
}

vector<int> convexhull(const vdbl & v){
  int s=v.size(),imin=0;
  if (s==1)
    return vector<int>(1,0);
  // find origin
  double ymin=imdbl(v[0]),ycur,xmin=redbl(v[0]),xcur;
  for (int i=1;i<s;++i){
    ycur=imdbl(v[i]);xcur=redbl(v[i]);
    if (ymin>ycur || (ymin==ycur && xmin>xcur) ){
      imin=i; ymin=ycur; xmin=xcur;
    }
  }
  vector<int_2double> ls;
  for (int j=0;j<s;++j){
    if (j!=imin){
      double dx=redbl(v[j])-xmin,dy=imdbl(v[j])-ymin;
      int_2double s={j,atan2(dy,dx),norm(dx,dy)};
      ls.push_back(s);
    }
  }
  sort(ls.begin(),ls.end(),graham_sort_function);
  vector<int> res; res.push_back(imin); res.push_back(ls[0].i);
  int ress=2;
  for (int j=1;j<s-1;++j){
    int icur=ls[j].i;
    double o=cross_prod(v,res[ress-2],res[ress-1],icur);
    if (o==0)
      res[ress-1]=icur;
    else {
      if (o>0){
        res.push_back(icur);
        ++ress;
      }
      else {
        while (ress>2 && o<0){
          res.pop_back();
          ress--;
          o=cross_prod(v,res[ress-2],res[ress-1],icur);
        }
        res.push_back(icur);
        ++ress;
      }
    }
  }
  return res;
}

void init_R(const vdbl & P,vdbl & R){
  R.clear();
  int n=P.size()-1;
  if (n && is_exactly_zero(P[n])){
    vdbl P1(P);
    P1.pop_back();
    init_R(P1,R);
    R.insert(R.begin(),0);
    return;
  }
  vdbl l;
  vector<int> lpos;
  for (int i=0;i<=n;++i){
    double ai=logabsdbl(P[n-i]);
    if (ai==MINREAL)
      continue;
    l.push_back(dbl(double(i),ai));
    lpos.push_back(i);
  }
  vector<int> pos=convexhull(l);
  // find real positions (since coeffs==0 were removed)
  for (int i=0;i<pos.size();++i){
    pos[i]=lpos[pos[i]];
  }
  // remove lower part of convexhull, pos[0] has the minimal y, 
  for (int i=0;i<pos.size();++i){
    if (i+1==pos.size() || pos[i]>pos[i+1]){
      pos.erase(pos.begin(),pos.begin()+i); 
      break;
    }
  }
  // now pos starts with highest value in x
  for (int i=0;i+1<pos.size();++i){
    if (pos[i]<pos[i+1]){
      // pos[i] is the lowest value in x
      pos.erase(pos.begin()+i+1,pos.end());
      break;
    }
  }
  sort(pos.begin(),pos.end());
  if (pos[0]!=0)
    pos.insert(pos.begin(),0);
  if (pos.back()!=n)
    pos.push_back(n);
  int count=0;
  vector<int_dbl> dkuk;
  for (int i=1;i<pos.size();++i){
    int dk=pos[i]-pos[i-1];
    const dbl & tmp =P[n-pos[i-1]];
    dbl uk=0;
    if (!is_exactly_zero(tmp)){
      dbl argexp=accurate_evalf(gen(-logabsdbl(P[n-pos[i]]/tmp)),64);
      uk=exp(argexp/dk,context0);//pow(absdbl(P[n-pos[i]]/P[n-pos[i-1]]),-1.0/dk);
    }
    int_dbl id={dk,uk};
    dkuk.push_back(id);
  }
  sort(dkuk.begin(),dkuk.end(),norm_sort_dbl);
  for (int i=0;i<dkuk.size();++i){
    int dk=dkuk[i].i;
    dbl uk=dkuk[i].d;
    double sigma=0.7;
    for (int j=0;j<dk;++j){
      double theta=sigma+2*M_PI*i/n+2*M_PI*j/dk;
      dbl z(uk*accurate_evalf(gen(std::cos(theta)),64),uk*accurate_evalf(gen(std::sin(theta)),64));
      R.push_back(z);
    }
  }
}

bool sum_inv_diff(const gen & zi,const vdbl & R,int i,gen & p){
  p=0;
  for (int j=0;j<R.size();++j){
    if (j==i) continue;
    dbl dz=(zi-R[j]);
    if (is_exactly_zero(dz))
      return false;
    p += inv(dz);
  }
  return true;
}

#ifdef HAVE_LIBMPFR
void sub(mpfr_t & rr,mpfr_t & ri,const gen & Pi){
  if (Pi.type==_CPLX){
    // const gen * Pir=Pi._CPLXptr; const gen * Pii=(Pi._CPLXptr+1);
    mpfr_sub(rr,rr,Pi._CPLXptr->_REALptr->inf,MPFR_RNDN);
    mpfr_sub(ri,ri,(Pi._CPLXptr+1)->_REALptr->inf,MPFR_RNDN);
  }
  else if (Pi.type==_REAL)
    mpfr_sub(rr,rr,Pi._REALptr->inf,MPFR_RNDN);
  else if (Pi.type==_INT_)
    mpfr_sub_si(rr,rr,Pi.val,MPFR_RNDN);
  else exit(1);
}

// product of R[i]-R[j] for j!=i
bool product(mpfr_t & rr,mpfr_t & ri,const vdbl & R,int i,mpfr_t & tmp,mpfr_t & tmp1,mpfr_t & tmp2,mpfr_t & tmp3,mpfr_t & tmp4){
  mpfr_set_si(rr,1,MPFR_RNDN);
  mpfr_set_si(ri,0,MPFR_RNDN);
  for (int j=0;j<R.size();++j){
    if (j==i) continue;
    if (R[i].type==_REAL){
      mpfr_set(tmp3,R[i]._REALptr->inf,MPFR_RNDN);
      mpfr_set_si(tmp4,0,MPFR_RNDN);
    }
    else if (R[i].type==_CPLX){
      mpfr_set(tmp3,R[i]._CPLXptr->_REALptr->inf,MPFR_RNDN);
      mpfr_set(tmp4,(R[i]._CPLXptr+1)->_REALptr->inf,MPFR_RNDN);
    }
    else return false;
    sub(tmp3,tmp4,R[j]);
    mult(rr,ri,tmp3,tmp4,tmp,tmp1,tmp2);
  }
  return true;
}

bool product(gen & p,const vdbl & R,int i,int nbits){
  mpfr_t rr,ri,tmp,tmp1,tmp2,tmp3,tmp4;
  mpfr_init2(rr,nbits); 
  mpfr_init2(ri,nbits); 
  mpfr_init2(tmp,nbits);
  mpfr_init2(tmp1,nbits);
  mpfr_init2(tmp2,nbits);
  mpfr_init2(tmp3,nbits);
  mpfr_init2(tmp4,nbits);
  bool b=product(rr,ri,R,i,tmp,tmp1,tmp2,tmp3,tmp4);
  p=gen(real_object(rr),real_object(ri));
  mpfr_clear(rr);
  mpfr_clear(ri);
  mpfr_clear(tmp);
  mpfr_clear(tmp1);
  mpfr_clear(tmp2);
  mpfr_clear(tmp3);
  mpfr_clear(tmp4);
  return b;  
}

void inv(mpfr_t & r,mpfr_t & i,mpfr_t & tmp,mpfr_t & tmp1, mpfr_t & tmp2){
  if (mpfr_zero_p(i)){
    mpfr_set_si(tmp,1,MPFR_RNDN);
    mpfr_div(r,tmp,r,MPFR_RNDN);
  }
  else {   // gen dbg; dbg=gen(real_object(r),real_object(i));
    mpfr_sqr(tmp1,r,MPFR_RNDN);
    mpfr_sqr(tmp2,i,MPFR_RNDN); //dbg=gen(real_object(tmp1),real_object(tmp2));
    mpfr_add(tmp,tmp1,tmp2,MPFR_RNDN);
    //dbg=gen(real_object(tmp),0);
#if 0
    mpfr_ui_div(tmp1,1,tmp,MPFR_RNDN);
    mpfr_mul(r,r,tmp1,MPFR_RNDN);
    mpfr_neg(tmp1,tmp1,MPFR_RNDN);
    mpfr_mul(i,i,tmp1,MPFR_RNDN);
#else
    mpfr_div(r,r,tmp,MPFR_RNDN);
    // dbg=gen(real_object(r),0);
    mpfr_neg(tmp,tmp,MPFR_RNDN);
    mpfr_div(i,i,tmp,MPFR_RNDN);
#endif
  }
}

bool mpfr_sum_inv_diff(const gen & z,const vdbl & R,int i,gen & p,int nbits,GIAC_CONTEXT){
  mpfr_t tmp,tmp1,tmp2,zr,zi,pr,pi;
  mpfr_init2(pr,nbits); mpfr_set_si(pr,0,MPFR_RNDN);
  mpfr_init2(pi,nbits); mpfr_set_si(pi,0,MPFR_RNDN);
  mpfr_init2(zr,nbits); 
  mpfr_init2(zi,nbits); 
  mpfr_init2(tmp,nbits);
  mpfr_init2(tmp1,nbits);
  mpfr_init2(tmp2,nbits);
  for (int j=0;j<R.size();++j){
    if (j==i) continue;
    // dbl dz=(z-R[j]);
    if (z.type==_REAL){
      mpfr_set(zr,z._REALptr->inf,MPFR_RNDN);
      mpfr_set_si(zi,0,MPFR_RNDN);
    }
    else if (z.type==_CPLX){
      mpfr_set(zr,z._CPLXptr->_REALptr->inf,MPFR_RNDN);
      mpfr_set(zi,(z._CPLXptr+1)->_REALptr->inf,MPFR_RNDN);
    }
    else if (z.type==_INT_){
      mpfr_set_si(zr,z.val,MPFR_RNDN);
      mpfr_set_si(zi,0,MPFR_RNDN);
    }
    else exit(1);
    sub(zr,zi,R[j]); 
    if (mpfr_zero_p(zr) && mpfr_zero_p(zi)){
      mpfr_clear(pr);
      mpfr_clear(pi);
      mpfr_clear(zr);
      mpfr_clear(zi);
      mpfr_clear(tmp);
      mpfr_clear(tmp1);
      mpfr_clear(tmp2);
      if (debug_infolevel>1)
        *logptr(contextptr) << "sum_inv_diff equal R index i=" << i << ", j=" << j << ", z=" << z << "\n";
      return false;
    }
    inv(zr,zi,tmp,tmp1,tmp2);
    // p += inv(dz);
    mpfr_add(pr,pr,zr,MPFR_RNDN);
    mpfr_add(pi,pi,zi,MPFR_RNDN);
    //p=gen(real_object(pr),real_object(pi)); // debug
  }
  p=gen(real_object(pr),real_object(pi));
  mpfr_clear(pr);
  mpfr_clear(pi);
  mpfr_clear(zr);
  mpfr_clear(zi);
  mpfr_clear(tmp);
  mpfr_clear(tmp1);
  mpfr_clear(tmp2);  
  return true;
}

// sum(1/(z-bi)), -1+sum(a_i/(z-b_i)), -sum(a_i/(z-b_i)^2)
bool mpfr_node_sum(const vdbl & A,const vdbl & B,const gen & z,gen & A0,gen & A1,gen & A2,int nbits,GIAC_CONTEXT){
  mpfr_t tmp,tmp1,tmp2,tmp3,tmp4,zr,zi,a0r,a0i,a1r,a1i,a2r,a2i,a1rcorr,a1icorr;
  mpfr_init2(zr,nbits); 
  mpfr_init2(zi,nbits); 
  mpfr_init2(a0r,nbits); mpfr_set_si(a0r,0,MPFR_RNDN);
  mpfr_init2(a0i,nbits); mpfr_set_si(a0i,0,MPFR_RNDN);
  mpfr_init2(a1r,nbits); mpfr_set_si(a1r,-1,MPFR_RNDN);
  mpfr_init2(a1i,nbits); mpfr_set_si(a1i,0,MPFR_RNDN);
  mpfr_init2(a1rcorr,nbits); mpfr_set_si(a1rcorr,0,MPFR_RNDN);
  mpfr_init2(a1icorr,nbits); mpfr_set_si(a1icorr,0,MPFR_RNDN);
  mpfr_init2(a2r,nbits); mpfr_set_si(a2r,0,MPFR_RNDN);
  mpfr_init2(a2i,nbits); mpfr_set_si(a2i,0,MPFR_RNDN);
  mpfr_init2(tmp,nbits);
  mpfr_init2(tmp1,nbits);
  mpfr_init2(tmp2,nbits);
  mpfr_init2(tmp3,nbits);
  mpfr_init2(tmp4,nbits);
  for (int j=0;j<B.size();++j){
    // dbl dz=(z-R[j]);
    if (z.type==_REAL){
      mpfr_set(zr,z._REALptr->inf,MPFR_RNDN);
      mpfr_set_si(zi,0,MPFR_RNDN);
    }
    else if (z.type==_CPLX){
      mpfr_set(zr,z._CPLXptr->_REALptr->inf,MPFR_RNDN);
      mpfr_set(zi,(z._CPLXptr+1)->_REALptr->inf,MPFR_RNDN);
    }
    else if (z.type==_INT_){
      mpfr_set_si(zr,z.val,MPFR_RNDN);
      mpfr_set_si(zi,0,MPFR_RNDN);
    }
    else exit(1);
    sub(zr,zi,B[j]); 
    if (mpfr_zero_p(zr) && mpfr_zero_p(zi)){
      mpfr_clear(a0r);
      mpfr_clear(a0i);
      mpfr_clear(a1r);
      mpfr_clear(a1i);
      mpfr_clear(a1rcorr);
      mpfr_clear(a1icorr);
      mpfr_clear(a2r);
      mpfr_clear(a2i);
      mpfr_clear(zr);
      mpfr_clear(zi);
      mpfr_clear(tmp);
      mpfr_clear(tmp1);
      mpfr_clear(tmp2);
      mpfr_clear(tmp3);
      mpfr_clear(tmp4);
      if (debug_infolevel>1)
        *logptr(contextptr) << "mpfr_node_sum equal B index j=" << j << ", z=" << z << "\n";
      return false;
    }
    inv(zr,zi,tmp,tmp1,tmp2);
    mpfr_set(tmp3,zr,MPFR_RNDN);
    mpfr_set(tmp4,zi,MPFR_RNDN);
    // A0=sum(1/(z-zi)
    // FIXME increase sum accuracy using
    // s :=0; c :=0;
    // loop: x=1/(z-zi), c += (x-((s+x)-s);  s +=x;
    // s += c
    mpfr_add(a0r,a0r,zr,MPFR_RNDN);
    mpfr_add(a0i,a0i,zi,MPFR_RNDN);
    //A0=gen(real_object(a0r),real_object(a0i));
    mult(zr,zi,A[j],tmp,tmp1,tmp2);
    // A1 precision correction, requires 8 more + or - in O(n) bits
    // vs multiplications that are slower
    // A1corr += (b-((A1+b)-A1));
    // prepare by keeping a copy of A1
    mpfr_set(tmp1,a1r,MPFR_RNDN);
    mpfr_set(tmp2,a1i,MPFR_RNDN);
    // A1 += b where b=a/(z-zi) is in zr,zi
    mpfr_add(a1r,a1r,zr,MPFR_RNDN);
    mpfr_add(a1i,a1i,zi,MPFR_RNDN);
    // A1=gen(real_object(a1r),real_object(a1i));
#if 1
    // precision correction: compute (A1+b)-A1 in tmp1/tmp2,
    // A1+b was just computed in a1r/a1i
    // tmp1/tmp2 contains a copy of A1 
    mpfr_sub(tmp1,a1r,tmp1,MPFR_RNDN);
    mpfr_sub(tmp2,a1i,tmp2,MPFR_RNDN);
    // precision correction: compute b-((A1+b)-A1) in tmp1/tmp2
    mpfr_sub(tmp1,zr,tmp1,MPFR_RNDN);
    mpfr_sub(tmp2,zi,tmp2,MPFR_RNDN);
    // precision correction: add correction tmp1/tmp2 to A1corr
    mpfr_add(a1rcorr,a1rcorr,tmp1,MPFR_RNDN);
    mpfr_add(a1icorr,a1icorr,tmp2,MPFR_RNDN);
#endif
    // A2=-sum(a/(z-zi)^2)
    mult(zr,zi,tmp3,tmp4,tmp,tmp1,tmp2);
    mpfr_sub(a2r,a2r,zr,MPFR_RNDN);
    mpfr_sub(a2i,a2i,zi,MPFR_RNDN);
    // A2=gen(real_object(a2r),real_object(a2i));    
  }
  A0=gen(real_object(a0r),real_object(a0i));
  mpfr_add(a1r,a1r,a1rcorr,MPFR_RNDN);
  mpfr_add(a1i,a1i,a1icorr,MPFR_RNDN);
  A1=gen(real_object(a1r),real_object(a1i));
  A2=gen(real_object(a2r),real_object(a2i));
  mpfr_clear(a0r);
  mpfr_clear(a0i);
  mpfr_clear(a1r);
  mpfr_clear(a1i);
  mpfr_clear(a1rcorr);
  mpfr_clear(a1icorr);
  mpfr_clear(a2r);
  mpfr_clear(a2i);
  mpfr_clear(zr);
  mpfr_clear(zi);
  mpfr_clear(tmp);
  mpfr_clear(tmp1);
  mpfr_clear(tmp2);  
  mpfr_clear(tmp3);
  mpfr_clear(tmp4);
  return true;
}
bool secular_mpfr(const vdbl & A,const vdbl & B,const dbl & x,dbl & d,int nbits,GIAC_CONTEXT){
  if (A.empty())
    return 0;
  gen A0,A1,A2;
  if (!mpfr_node_sum(A,B,x,A0,A1,A2,nbits,contextptr))
    return false;
  d=A1/(A1*A0+A2);
  return true;
}
#endif // MPFR

// sum(1/(z-bi)), -1+sum(a_i/(z-b_i)), -sum(a_i/(z-b_i)^2)
bool singleprec_node_sum(const vfdbl & A,const vfdbl & B,const fdbl & z,fdbl & A0,fdbl & A1,fdbl & A2,GIAC_CONTEXT){
  A2=A0=fdbl(0.0);
  A1=fdbl(-1.0);
  fdbl A0corr=0,A1corr=0,A2corr=0;
  // FIXME increase sum accuracy using
  // s :=0; c :=0;
  // loop: x=1/(z-zi), c += (x-((s+x)-s);  s +=x;
  // s += c
  for (size_t i=0;i<A.size();++i){
    fdbl zb(z-B[i]);
    longdouble n=norm(zb);
    if (n==0)
      return false;
    zb=conj(zb)/(n);
    //A0corr += (zb-((A0+zb)-A0));
    A0 += zb;
    fdbl b(A[i]*zb);
    A1corr += (b-((A1+b)-A1));
    A1 += b;
    b *= -zb;
    //A2corr += (b-((A2+b)-A2));
    A2 += b;
  }
  A0 += A0corr;
  A1 += A1corr;
  A2 += A2corr;
  return true;
}

bool secular_singleprec(const vfdbl & A,const vfdbl & B,const fdbl & x,fdbl & d,GIAC_CONTEXT){
  if (A.empty())
    return 0;
  fdbl A0,A1,A2;
  if (!singleprec_node_sum(A,B,x,A0,A1,A2,contextptr))
    return false;
  d=A1/(A1*A0+A2);
  return true;
}

bool aberth_singleprec(const vfdbl & P0,int N,double eps,vfdbl & R,int cluster_start,int cluster_afterend,vector<short int> & zi_done,int afteriter,bool secular,GIAC_CONTEXT){
  int deg=P0.size()-1;
  bool doing_cluster=cluster_start>0 || cluster_afterend<deg;
  vfdbl P(P0);
  // init R if not already done
  fdbl dr(0);
  fdbl l(1);
  if (R.size()!=deg){
    dr=-find_shift(P);
    P=shift(P,dr);
    l=1;//find_scale(P);
    rescale(P,l);
    double res=init_R(P,R);
    if (res==0 || isinf(res)){
      R.clear();
      return false;
    }
  }
  vfdbl Prev(P),A,B(R);
  bool firstiterhorner=!secular;
#ifdef HAVE_LIBMPFR
  if (secular){
    firstiterhorner=true;
    int nbitsP2=3*64; 
    vdbl P2,R2; Convert(P,P2); Convert(R,R2);
    accurate_evalf(P2,nbitsP2);
    accurate_evalf(R2,nbitsP2);
    // init secular nodes from R and P2
    A.resize(deg); gen d,d1,tmp; long size;
    for (int i=0;i<deg;++i){
      long loss=horner2_mpfr(P2,R2[i],d,d1,nbitsP2,size,false);
      product(d1,R2,i,nbitsP2);
      tmp=-d/(d1*P2[0]);
      reim(tmp,d,d1,contextptr);
      longdouble zr,zi;
      if (!convert(d,zr) || !convert(d1,zi))
        return false;
      A[i]=fdbl(zr,zi); // A[i]=accurate_evalf(-d/(d1*P2[0]),nbitsP2-loss);
    }
  }
#else
  secular=false;
  firstiterhorner=true;
#endif
  reverse(Prev.begin(),Prev.end());
  vfdbl newR(deg);
  int ok=0; double delta;
  for (int k=0;k<N;++k){
    if (k)
      firstiterhorner=false;
    delta=0;
    if (doing_cluster){
      int K=cluster_start;
      for (;K<cluster_afterend;++K){
        if (!zi_done[K])
          break;
      }
      if (K==cluster_afterend)
        ++ok;
    }
    for (int i=cluster_start;i<cluster_afterend;++i){
      if (!ok && zi_done[i]){ // always do the last iteration for accuracy
        newR[i]=R[i];
        continue;
      }
      fdbl & zi=R[i];
      fdbl d,d1;
      if (secular && !firstiterhorner && secular_singleprec(A,B,zi,d,contextptr))
        d1=1;
      else if (absdbl(zi)>1){
        fdbl gamma(inv(zi));
        horner2(Prev,gamma,d,d1);
        if (!is_exactly_zero(d)){
          d=gamma*(fdbl(deg)-gamma*d1/d);
          if (is_exactly_zero(d)){
            translate_shift(R,l,dr);
            if (debug_infolevel)
              *logptr(contextptr) << "Aberth_single precision |z|>1 trying to invert 0\n";
            return false;
          }
          d=inv(d);
        }
      }
      else {
        horner2(P,zi,d,d1);
        if (is_exactly_zero(d1)){
          translate_shift(R,l,dr);
          if (debug_infolevel)
            *logptr(contextptr) << "Aberth_single precision |z|<=1 trying to invert 0\n";
          return false;
        }
        d=d/d1;
      }
      fdbl p(0); bool binv=true;
      for (int j=0;j<deg;++j){
	if (j==i) continue;
        fdbl dz=(zi-R[j]);
        if (is_exactly_zero(dz)){
          binv=false;
          break; // return false;
        }
	p += inv(dz);
      }
      double dd=absdbl(d),pp=absdbl(p);
      double abszi=absdbl(zi);
      if (debug_infolevel>2)
        *logptr(contextptr) << "cluster? i=" << i << ", delta=" << dd << ", cluster_step=" << cluster_step << ", p=" << pp << ", d*p=" << dd*pp << " " << cluster_dp << "\n";
      if (!ok
          && k>8
          && !doing_cluster && dd<=cluster_step*abszi && dd*pp>=cluster_dp){
        // if (ok) continue;
        // cluster of roots, find all roots in this cluster
        int cend=i+1;
        fdbl sumR=zi;
        for (int k=cend;k<deg;++k){
          if (abs(zi-R[k])<2*deg/pp){
            swap(R[cend],R[k]);
            swap(zi_done[cend],zi_done[k]);
            sumR+=R[cend];
            ++cend;
          }
        }
        if (cend-i>1){
          int Nc=cend-i;
          // cluster is from i to cend-1 included
          fdbl z=sumR/fdbl(Nc); // center of gravity of cluster
          vfdbl Pdiff;
          --Nc;
          for (int l=0;l<=deg-Nc;++l){
            fdbl h=P[l];
            for (int k=deg-l;k>deg-l-Nc;--k){
              h=((longdouble) k)*h;
            }
            Pdiff.push_back(h);
          }
          fdbl z1,z2;
          horner2(Pdiff,z,z1,z2);
          if (!is_exactly_zero(z2)){
            fdbl delta=z1/z2;
            z=z-delta;
            for (;0;){ // disabled
              horner2(P,z,z1,z2);
              if (!is_exactly_zero(z1))
                break;
              z += fdbl(1-1e-17)*z;
            }
          }
          // shift P and reverse (roots become inverse of roots)
          vfdbl Pcluster(shift(P,z,false)),Rcluster(deg),initR(deg);
          reverse(Pcluster.begin(),Pcluster.end());
          if (is_exactly_zero(Pcluster[0])){
            Pcluster[0]=z/pow(fdbl(2),64);
          }
          init_R(Pcluster,initR);
          int count=deg-1;
          for (int k=0;k<deg;++k){
            if (k>=i && k<cend){
              Rcluster[k]=initR[count];
              --count;
              //dbl a(0.0,0.7+(k-i)*2*M_PI/(cend-i));
              //Rcluster[k]=D*exp(a);
            }
            else
              Rcluster[k]=inv(R[k]-z);
          }
          vector<short int> old(zi_done);        
          if (debug_infolevel)
            *logptr(contextptr) << "aberth single cluster=" << i << "," << cend << "\n";
          // recursive call of aberth
          bool b=aberth_singleprec(Pcluster,N,eps,Rcluster,i,cend,zi_done, 2/* afteriter*/,false,contextptr);
          for (int k=i;k<cend;++k){
            R[k]=newR[k]=inv(Rcluster[k])+z;
          }
          if (b)
            continue;
          translate_shift(R,l,dr);
          if (debug_infolevel)
            *logptr(contextptr) << "Aberth_single precision cluster split failure\n";
          return false;
        }
      }
      double absdz=abszi==0?absdbl(d):absdbl(d)/abszi;
      if (binv)
        d=d/(fdbl(1)-d*p);
      else
        d=d/(fdbl(1-std_rand()/2.0/RAND_MAX));
      if (absdz<eps/deg){
#ifndef KHICAS
        if (debug_infolevel>1 && !zi_done[i])
          *logptr(contextptr) << "New root found " << zi-d << "\n";
#endif
        zi_done[i]=1;
      }
      delta += absdz;
      newR[i]=zi-d;
      R[i]=newR[i]; // comment to avoid immediate update
    }
    newR.swap(R);
    int count=0;
    for (int k=0;k<deg;k++){
      if (zi_done[k])
        ++count;
    }
#ifndef KHICAS
    if (debug_infolevel>0){
      *logptr(contextptr) << "Aberth single prec iter " << k <<  " found=" << count << " cluster_search_start=" << cluster_start << ".." << cluster_afterend << " delta=" << delta << " time " << clock()*1e-6 << "\n";
      if (debug_infolevel>2){
        *logptr(contextptr) << "Current approx roots " << R << "\n";
      }
    }
#endif
    if (doing_cluster){
      if (ok>=afteriter)
        return true;
      continue;
    }
    if (//count==deg
        delta<=eps*(ok?deg:deg-count) || count==deg
        )
      ++ok;
    else
      ok=0;
    if (ok>=afteriter){
      translate_shift(R,l,dr);
      sort(R.begin(),R.end(),fdbl_less);
      return true;
    }
  }
  if (debug_infolevel)
    *logptr(contextptr) << "Aberth single precision: too many iterations " << N << " delta " << delta << "\n";
  return false;  
}


// check that roots are isolated or have precision eps, set zi_done[i] to 2 or 0
bool chk_isol(vdbl & z,bool realpoly,int isolate,const vdbl & rz,double eps,vector<short int> & zi_done,GIAC_CONTEXT){
  int deg=z.size();
  if (debug_infolevel && isolate)
    *logptr(contextptr) << "Certifying roots begin " << CLOCK()*1e-6 << "\n";
  dbl big(1LL<<62); // take care of almost complete cancellation in -
  for (int i=0;i<deg;++i){
    dbl & zi=z[i];
    // dbl err=abs(zi)/big;
    const dbl & rzi=rz[i]; // +err
    if (1 || zi_done[i]){
      zi_done[i]=2;
      for (int j=i+1;j<deg;++j){
        dbl zij=sqrt((z[j]-zi).squarenorm(contextptr),contextptr);
        if (is_greater(rzi+rz[j],zij,contextptr))
          zi_done[i]=zi_done[j]=0;
      }
    }
    if (zi_done[i] && realpoly && zi.type==_CPLX){
      dbl I=*(zi._CPLXptr+1);
      dbl c=conj(zi,contextptr);
      bool maybereal=is_positive(rzi-I,contextptr);
      int nconj=0; // 0 after loop if root is real
      bool unique=true;
      for (int j=i+1;j<deg;++j){
        dbl zij=(z[j]-c);
        // Note that zij may be = to abs(c)*2^-63*sqrt(2) if we loose all precision
        zij=rzi+rz[j]-abs(zij,contextptr);
        if (is_positive(zij,contextptr)){
          zi_done[i]=4; // potential conjugate
          if (nconj==0)
            nconj=j;
          else
            unique=false; // more than one potential conjugate
        }
      }
      if (nconj==0){
        if (maybereal) // clean it
          zi=*zi._CPLXptr;
      }
      else {
        swap(z[i+1],z[nconj]);
        swap(zi_done[i+1],zi_done[nconj]);
        zi_done[i]=4;
        if (unique){
          zi_done[i+1]=5;
          z[i+1]=c;
        }
        ++i;
      }
    }
  }
  // isolate==1 if goal is isolation only
  if (isolate!=1){ 
    bool approx=true;
    for (int i=0;i<deg;++i){
      dbl & zi=z[i];
      // dbl err=abs(zi)/big;
      const dbl & rzi=rz[i]; // +err
      if (zi_done[i]!=5){
        gen chk=is_exactly_zero(zi)?eps:eps*abs(zi,contextptr);
        if (is_greater(rzi,chk,contextptr)){
          zi_done[i]=0;
          approx=false;
        }
      }
    }
    if (!isolate)
      return approx;
  }
  for (int i=0;i<deg;++i){
    if (!zi_done[i]){
      if (debug_infolevel)
        *logptr(contextptr) << "Failure certifying roots " << CLOCK()*1e-6 << "\n";
      return false;
    }
  }
  if (debug_infolevel)
    *logptr(contextptr) << "Certifying roots end " << CLOCK()*1e-6 << "\n";
  return true;
}

void clear_zi_done(vector<short int> & zi_done,bool clearall){
  if (clearall){
    for (int i=0;i<zi_done.size();++i){
      if (zi_done[i]!=5)
        zi_done[i]=0;
    }
  }
  else {
    for (int i=0;i<zi_done.size();++i){
      if (zi_done[i]<2)
        zi_done[i]=0;
    }
  }
}

#if defined HAVE_LIBMPFI && !defined NO_RTTI
bool maybe_zero(const dbl & g){
  dbl a=abs(g,context0);
  if (a.type==_REAL){
    if (real_interval * ptr=dynamic_cast<real_interval *>(a._REALptr))
      return ptr->maybe_zero();
  }
  return is_exactly_zero(a);
}
void add(mpfi_t & rr,mpfi_t & ri,const gen & Pi){
  if (Pi.type==_CPLX){
    mpfi_add(rr,rr,dynamic_cast<real_interval *>(Pi._CPLXptr->_REALptr)->infsup);
    mpfi_add(ri,ri,dynamic_cast<real_interval *>((Pi._CPLXptr+1)->_REALptr)->infsup);
  }
  else if (Pi.type==_REAL)
    mpfi_add(rr,rr,dynamic_cast<real_interval *>(Pi._REALptr)->infsup);
  else if (Pi.type==_INT_)
    mpfi_add_si(rr,rr,Pi.val);
  else exit(1);
}

void mult(mpfi_t & rr,mpfi_t & ri,const gen & x,mpfi_t & tmp,mpfi_t & tmp1, mpfi_t & tmp2){
  if (x.type==_CPLX){
    mpfi_mul(tmp1,rr,dynamic_cast<real_interval *>(x._CPLXptr->_REALptr)->infsup);
    mpfi_mul(tmp2,ri,dynamic_cast<real_interval *>((x._CPLXptr+1)->_REALptr)->infsup);
    mpfi_sub(tmp,tmp1,tmp2);
    mpfi_mul(tmp1,rr,dynamic_cast<real_interval *>((x._CPLXptr+1)->_REALptr)->infsup);
    mpfi_mul(tmp2,ri,dynamic_cast<real_interval *>(x._CPLXptr->_REALptr)->infsup);
    mpfi_swap(tmp,rr);
    mpfi_add(ri,tmp1,tmp2);
  }
  else if (x.type==_REAL){
    mpfi_mul(rr,rr,dynamic_cast<real_interval *>(x._REALptr)->infsup);
    mpfi_mul(ri,ri,dynamic_cast<real_interval *>(x._REALptr)->infsup);
  }
  else if (x.type==_INT_){
    mpfi_mul_si(rr,rr,x.val);
    mpfi_mul_si(ri,ri,x.val);
  }
  else exit(1);
}

// find r=P(x) and r1=diff(P)(x)
bool horner2_mpfi(const vdbl & P,dbl x,dbl & r,dbl & r1,int nbits,bool pdiff=true){
  if (P.empty())
    return false;
  size_t s=P.size()-1;
  mpfi_t rr,ri,r1r,r1i,tmp,tmp1,tmp2;
  mpfi_init2(rr,nbits); mpfi_set_si(rr,0);
  mpfi_init2(ri,nbits); mpfi_set_si(ri,0);
  mpfi_init2(r1r,nbits); mpfi_set_si(r1r,0);
  mpfi_init2(r1i,nbits); mpfi_set_si(r1i,0);
  mpfi_init2(tmp,nbits);
  mpfi_init2(tmp1,nbits);
  mpfi_init2(tmp2,nbits);
  for (size_t i=0;i<s;++i){
    // r=r*x+P[i];
    mult(rr,ri,x,tmp,tmp1,tmp2);
    add(rr,ri,P[i]);
    if (pdiff){
      // r1=r1*x+r;
      mult(r1r,r1i,x,tmp,tmp1,tmp2);
      mpfi_add(r1r,r1r,rr);
      mpfi_add(r1i,r1i,ri);
    }
  }
  // r=r*x+P[s];
  mult(rr,ri,x,tmp,tmp1,tmp2);
  add(rr,ri,P[s]);
  // end
  r=gen(real_interval(rr),real_interval(ri));
  r1=gen(real_interval(r1r),real_interval(r1i));
  mpfi_clear(rr);
  mpfi_clear(ri);
  mpfi_clear(r1r);
  mpfi_clear(r1i);
  mpfi_clear(tmp);
  mpfi_clear(tmp1);
  mpfi_clear(tmp2);
  return true;
}


// find r=P(x) and r1=diff(P)(x)
bool horner2_mpfi(const vdbl & P,const vdbl & Pdiff,dbl x,dbl & r,dbl & r1,int nbits){
  if (P.empty())
    return false;
  size_t s=P.size()-1;
  mpfi_t rr,ri,r1r,r1i,tmp,tmp1,tmp2;
  mpfi_init2(rr,nbits); mpfi_set_si(rr,0);
  mpfi_init2(ri,nbits); mpfi_set_si(ri,0);
  mpfi_init2(r1r,nbits); mpfi_set_si(r1r,0);
  mpfi_init2(r1i,nbits); mpfi_set_si(r1i,0);
  mpfi_init2(tmp,nbits);
  mpfi_init2(tmp1,nbits);
  mpfi_init2(tmp2,nbits);
  for (size_t i=0;i<s;++i){
    // r=r*x+P[i];
    mult(rr,ri,x,tmp,tmp1,tmp2);
    add(rr,ri,P[i]);
    // r1=r1*x+Pdiff[i];
    mult(r1r,r1i,x,tmp,tmp1,tmp2);
    add(r1r,r1i,Pdiff[i]);
  }
  // r=r*x+P[s];
  mult(rr,ri,x,tmp,tmp1,tmp2);
  add(rr,ri,P[s]);
  // end
  r=gen(real_interval(rr),real_interval(ri));
  r1=gen(real_interval(r1r),real_interval(r1i));
  mpfi_clear(rr);
  mpfi_clear(ri);
  mpfi_clear(r1r);
  mpfi_clear(r1i);
  mpfi_clear(tmp);
  mpfi_clear(tmp1);
  mpfi_clear(tmp2);
  return true;
}

#else
bool maybe_zero(const dbl & g){
  return is_exactly_zero(g);
}

#endif

#ifdef HAVE_LIBMPFR
// returns 0 on failure (e.g. division by maybe 0), -1 too many iterations, 1 ok, 2 isolated
int aberth_mpfr(const vdbl & P0,bool realpoly,int & nbits,int N,double eps,vdbl & A,vdbl & B,vdbl & R,vdbl & rayon,int cluster_start,int cluster_afterend,vector<short int> & zi_done,bool certify_lastiter,int isolate,bool secular,GIAC_CONTEXT){
  int neps=std::ceil(-log2(eps));
  if (neps>nbits)
    nbits=64*((neps+63)/64);
  int afteriter=2;
  int deg=P0.size()-1,prevcount=0;
  bool doing_cluster=cluster_start>0 || cluster_afterend<deg;
  vdbl P(P0);
  dbl dr(0);
  dbl l(1);
  if (R.size()!=deg){
    dr=-find_shift(P,contextptr);
    P=shift(P,dr);
    //l=find_scale(P);
    //rescale(P,l);
    // init R if not already done, using single precision Aberth
    vfdbl fP,fR;
    if (convert(P,fP,contextptr)){
      double eps2=1e-4;
      bool ok;
      if (secular){
	aberth_singleprec(fP,N,eps2,fR,0,deg,zi_done,1,false,contextptr);
	ok=aberth_singleprec(fP,N,eps2/16,fR,0,deg,zi_done,4,true /* secular*/ ,contextptr);	
      }
      else
	ok=aberth_singleprec(fP,2*N,eps2,fR,0,deg,zi_done,4,false,contextptr);
      Convert(fR,R);
      if (ok &&
          //0 &&
	  !doing_cluster){
        vdbl P_cert(P);
        int nbits=64;
        accurate_evalf(P_cert,nbits);
#if defined MPFI_CERT && defined HAVE_LIBMPFI && !defined NO_RTTI
        P_cert=*convert_interval(P_cert,nbits,contextptr)._VECTptr;
#endif
        // find rayon with exact computation or MPFI
        int bits=nbits;
        vdbl P1(P);
        accurate_evalf(P1,bits);
        for (int i=0;ok && i<deg;++i){
#if 1 // def HAVE_LIBMPFR
          gen z=accurate_evalf(R[i],nbits),d,d1;
#if defined MPFI_CERT && defined HAVE_LIBMPFI && !defined NO_RTTI
          z=convert_interval(z,nbits,contextptr);
          horner2_mpfi(P_cert,convert_interval(z,nbits,contextptr),d,d1,nbits);
          if (maybe_zero(d1)){
            ok=false;
          }
          else {
            d=d/d1;
            rayon[i]=deg*abs(d,contextptr);
            if (rayon[i].type==_REAL)
              rayon[i]=_right(rayon[i],contextptr);
          }
#else // MPFI
          for (;;){
            long size; int loss=horner2_mpfr(P1,z,d,d1,bits,size,true);
            if (loss<bits-16){
              rayon[i]=abs(accurate_evalf(d/d1,nbits),contextptr);
              break;
            }
            bits*=2;
            z=accurate_evalf(R[i],bits);
            P1=P;
            accurate_evalf(P1,bits);
          }
#endif // MPFI
#else // MPFR
          gen z=exact(R[i],contextptr),d,d1;
          horner2(P,z,d,d1);
          rayon[i]=abs(evalf_double(d/d1,1,contextptr));
#endif
          if (is_greater(rayon[i],eps*abs(R[i],contextptr),contextptr))
            ok=false;
        }
        // early termination in single precision?
        if (ok &&
            chk_isol(R,realpoly,isolate,rayon,eps,zi_done,contextptr)
            ){
          for (size_t j=0;j<R.size();++j){
            R[j] += dr;
            if (!isolate && eps>1e-14)
              R[j]=evalf_double(R[j],1,contextptr);
          }
          return 2;
        }
      }
    }
    if (R.empty())
      init_R(P,R);
  }
  accurate_evalf(P,nbits);
  clear_zi_done(zi_done,true);
  vdbl P_cert; int nbitscert=nbits+64;
  if (certify_lastiter){
#if defined MPFI_CERT && defined HAVE_LIBMPFI && !defined NO_RTTI
    P_cert=*convert_interval(P,nbitscert,contextptr)._VECTptr;
#else
    P_cert=P;
#endif
  }
  accurate_evalf(R,nbits);
  vdbl P2(P0); int nbitsP2=2*nbits+64; 
  accurate_evalf(P2,nbitsP2);
  bool firstiterhorner=!secular;
  if (secular){
    bool refresh_nodes=A.empty();
    if (!refresh_nodes){
      if (A[0].type==_CPLX && A[0]._CPLXptr->type==_REAL)
        refresh_nodes=mpfr_get_prec(A[0]._CPLXptr->_REALptr->inf)<nbits;
      else if (A[0].type==_REAL)
        refresh_nodes=mpfr_get_prec(A[0]._REALptr->inf)<nbits;
    }
    if (refresh_nodes){
      firstiterhorner=true;
      // init secular nodes from R and P2
      A.resize(deg); B.resize(deg); gen d,d1; long size;
      for (int i=0;i<deg;++i){
        B[i]=R[i]; // accurate_evalf(R[i],nbitsP2);
        long loss=horner2_mpfr(P2,accurate_evalf(R[i],nbitsP2),d,d1,nbitsP2,size,false);
        product(d1,R,i,nbitsP2);
        A[i]=accurate_evalf(-d/(d1*P2[0]),nbits); // A[i]=accurate_evalf(-d/(d1*P2[0]),nbitsP2-loss);
      }
    }
  }
#if 0
  vdbl P_cert_diff(derivative(P0)); 
  if (certify_lastiter){
#if 0 // defined MPFI_CERT && defined HAVE_LIBMPFI && !defined NO_RTTI
    P_cert_diff=*convert_interval(P_cert_diff,nbits,contextptr)._VECTptr;
#else
    accurate_evalf(P_cert_diff,nbits);
#endif
  }
#endif
  vdbl newR(R); // init with R is required in clusters
  int ok=0; long size; double delta; bool isol;
  for (int k=0;k<N;++k){
    if (k)
      firstiterhorner=false;
    if (debug_infolevel>0)
      *logptr(contextptr) << clock()*1e-6 << " Aberth bits=" << nbits << " iter="<<  k << " ";
    delta=0; isol=true;
    long maxloss=0,minloss=0;
    if (doing_cluster){
      int K=cluster_start;
      for (;K<cluster_afterend;++K){
        if (!zi_done[K])
          break;
      }
      if (K==cluster_afterend)
        ok=1;
    }
    for (int i=cluster_start;i<cluster_afterend;++i){
      if (i && zi_done[i]==5){
        newR[i]=R[i]=conj(R[i-1],contextptr);
        rayon[i]=rayon[i-1];
        continue;
      }
      if (zi_done[i]>=2 ||
          (!ok && zi_done[i])
          ){
        // if root is not isolated, do the last iteration
        newR[i]=R[i];
        continue;
      }
      dbl zi=R[i];
      if (debug_infolevel>2)
        *logptr(contextptr) << CLOCK()*1e-6 << " computing d\n"; 
      dbl d,d1;
      long loss=0;
      if (ok && certify_lastiter){
#if defined MPFI_CERT && defined HAVE_LIBMPFI && !defined NO_RTTI
        int loss2=RAND_MAX;
        if (!isolate || secular)
          loss2=horner2_mpfr(P2,accurate_evalf(zi,nbitsP2),d,d1,nbitsP2,size,true);
        if (secular){
          B[i]=R[i];
          gen p; product(p,R,i,nbitsP2);
          A[i]=-d/(p*P[0]); // update secular nodes for next iteration
        }
        if (!isolate && loss2>=nbitsP2-16){
          gen zicert=convert_interval(zi,nbitscert,contextptr),dd,dd1;        
          horner2_mpfi(P_cert,zicert,d,d1,nbitscert);
          if (maybe_zero(d1)){
            if (debug_infolevel)
              *logptr(contextptr) << "MPFI unable to certify radius, root " << i << zi << "\n";
            return -1;
          }
        }
#else
        loss=horner2_mpfr(P_cert,accurate_evalf(zi,nbitscert),d,d1,nbitscert,size,true);
        while (loss>nbitscert-16){
          nbitscert*=2;
          P_cert=P0;
          accurate_evalf(P_cert,nbitscert);
          loss=horner2_mpfr(P_cert,accurate_evalf(zi,nbitscert),d,d1,nbitscert,size,true);
        }
        if (is_exactly_zero(d1))
          return -1;
        if (secular){
          B[i]=R[i];
          gen p; product(p,R,i,nbitscert);
          A[i]=-d/(p*P[0]); // update secular nodes for next iteration
        }
#endif
      }          
      else {
        if (secular && !firstiterhorner && secular_mpfr(A,B,zi,d,nbits,contextptr))
          d1=1;
        else
          loss=horner2_mpfr(P,zi,d,d1,nbits,size,true);
        if (is_exactly_zero(d1))
          return -1;
      }
      d=d/d1;
      if (ok)
        rayon[i]=deg*abs(d,contextptr);
      if (ok && certify_lastiter && rayon[i].type==_REAL){        
#if defined MPFI_CERT && defined HAVE_LIBMPFI && !defined NO_RTTI        
        rayon[i]=_right(rayon[i],contextptr);
        d=gen(_milieu(re(d,contextptr),contextptr),_milieu(im(d,contextptr),contextptr));
#else
        rayon[i]=accurate_evalf(rayon[i],nbits);
#endif
      }
      dbl p(0);
      if (debug_infolevel>2)
        *logptr(contextptr) << CLOCK()*1e-6 << " computing p\n"; 
      bool binv=mpfr_sum_inv_diff(zi,R,i,p,nbits,contextptr);
      if (debug_infolevel>2)
        *logptr(contextptr) << CLOCK()*1e-6 << " end computing p\n"; 
      if (!binv && doing_cluster){
        *logptr(contextptr) << "Root estimates collision \n"; return 0; }
#if 0
      dbl pc(0); sum_inv_diff(zi,R,i,pc);
      if (p!=pc)
        *logptr(contextptr) << "err\n";
#endif
      double dd=absdbl(d),pp=absdbl(p);
      double abszi=absdbl(zi);
      if (debug_infolevel>2)
        *logptr(contextptr) << CLOCK()*1e-6 << " cluster? i=" << i << ", delta=" << dd << ", cluster_step=" << cluster_step << ", p=" << pp << ", d*p=" << dd*pp << " " << cluster_dp << "\n";
      if (!binv ||
          (!ok && k>N/2 && !doing_cluster && dd<=cluster_step*abszi && dd*pp>=cluster_dp)){
        isol=false;
        // cluster of roots, find all roots in this cluster
        int cend=i+1;
        dbl sumR=zi;
        for (int k=cend;k<deg;++k){
          if (absdbl(zi-R[k])<2*deg/pp){
            swap(R[cend],R[k]);
            swap(zi_done[cend],zi_done[k]);
            sumR+=R[cend];
            ++cend;
            if (k<deg-1 && zi_done[k+1]==5)
              zi_done[k+1]=0;
          }
        }
        if (cend-i==deg){
          if (debug_infolevel)
            *logptr(contextptr) << "All estimates are close together\n";
        }
        if (cend-i>1){
          int nbits2=2*nbits,Nc=cend-i;
          // cluster is from i to cend-1 included
          dbl z=sumR/dbl(Nc); // center of gravity of cluster
          // improve z with a Newton iteration on the derivative of order size of cluster -1
          vdbl Pdiff;
          --Nc;
          for (int l=0;l<=deg-Nc;++l){
            gen h=1;
            for (int k=deg-l;k>deg-l-Nc;--k){
              h=k*h;
            }
            Pdiff.push_back(h*P[l]);
          }
          gen z1,z2;
          horner2_mpfr(Pdiff,z,z1,z2,nbits,size,true);
          if (!is_exactly_zero(z2))
            z=z-z1/z2; 
          // shift P and reverse (roots become inverse of roots)
          vdbl Pcluster(P),Rcluster(deg),initR(deg),Acluster(deg),Bcluster(deg);
          accurate_evalf(Pcluster,nbits2);
          z=accurate_evalf(z,nbits2);
          Pcluster=shift(Pcluster,z,false);
          reverse(Pcluster.begin(),Pcluster.end());
          if (is_exactly_zero(Pcluster[0])){
            Pcluster[0]=z/pow(2,nbits2,contextptr);
          }
          init_R(Pcluster,initR);
          // dbl D(dbl(cend-i)/d);
          int count=deg-1;
          for (int k=0;k<deg;++k){
            if (k>=i && k<cend){
              Rcluster[k]=initR[count];
              --count;
              //dbl a(0.0,0.7+(k-i)*2*M_PI/(cend-i));
              //Rcluster[k]=D*exp(a);
            }
            else
              Rcluster[k]=inv(R[k]-z);
          }
          vector<short int> old(zi_done);
          // recursive call of aberth
          if (debug_infolevel)
            *logptr(contextptr) << "aberth mpfr cluster=" << i << "," << cend << "\n";
          int b=aberth_mpfr(Pcluster,false,nbits2,N,eps,Acluster,Bcluster,Rcluster,rayon,i,cend,zi_done,false,false,false,contextptr);
          for (int k=i;k<cend;++k){
            R[k]=newR[k]=accurate_evalf(inv(Rcluster[k])+z,nbits);
          }
          if (b){
            i=cend-1;
            //++afteriter;
            continue;
          }
          return -1;
        }
      }
      double absdz=abszi==0?absdbl(d):absdbl(d)/abszi;
      if (binv){
        d=d/(dbl(1)-d*p);
      }
      else {
        isol=false;
        if (ok)
          return 0;
        d=d/(1-std_rand()/2.0/RAND_MAX);
      }
      delta += absdz;
      newR[i]=zi-d;
      R[i]=newR[i]; // comment to avoid immediate update of the root
      if (absdz<eps/deg){
        if (!zi_done[i]){
          if (debug_infolevel>1)
            *logptr(contextptr) << "New root found " << newR[i] << "\n";
          if (debug_infolevel)
            *logptr(contextptr) << "[" << i << "] ";
        }
        zi_done[i]=1;
      }
      else {
        if (loss<nbits && dd*pp<=0.5){
          if (debug_infolevel)
            *logptr(contextptr) << i << " ";
        }
        else if (!doing_cluster && !secular)
          zi_done[i]=-1;
        if (maxloss<loss)
          maxloss=loss;
        if (minloss==0)
          minloss=loss;
        if (minloss>loss)
          minloss=loss;
      }        
    }
    newR.swap(R);
    int count=0,skipped=0;
    for (int k=0;k<deg;++k){
      if (zi_done[k]>0)
        ++count;
      else if (zi_done[k]==-1)
        ++skipped;      
    }
    if (debug_infolevel>0){
      *logptr(contextptr) << " found=" << count << " skipped " << skipped << " delta=" << delta <<   " precision loss " << double(minloss)/nbits << " (" <<  minloss << "/" << nbits << "), cluster_search=" << cluster_start <<  ".." <<cluster_afterend << " time=" << CLOCK()*1e-6 << "\n";
      if (debug_infolevel>2){
        vdbl RR(R);
        accurate_evalf(RR,45);
        *logptr(contextptr) << "Current approx roots " << RR << "\n";
      }
    }
    if (doing_cluster){
      if (ok)
        return 1;
      continue;
    }
    if (
        delta<=eps*(ok?deg:deg-count) || count==deg
        ){
      if (debug_infolevel)
        *logptr(contextptr) << CLOCK()*1e-6 << " Aberth all roots found " << delta << "\n";      
      ++ok;
    }
    // else ok=0;
    if (ok==afteriter){
      // rescale and translate
      for (size_t j=0;j<R.size();++j){
        R[j] = l*R[j] + dr;
      }
      return 1;
    }
    if (minloss>nbits && count==prevcount)
      return -1;
    prevcount=count;
  }
  if (debug_infolevel)
    *logptr(contextptr) << "Aberth mpfr: too many iterations " << N << " delta " << delta << "\n";
  return -1;  
}
#endif

dbl round(const dbl & z,const gen & pow2,int nbits){
  dbl res(z);
  round2(res,nbits);
  return res;
  gen n,d;
  fxnd(z,n,d);
  return iquo(n*pow2,d)/pow2;
}

int aberth_z(const vdbl & P0,int nbits,int N,double eps,vdbl & R,int cluster_start,int cluster_afterend,vector<short int> & zi_done,GIAC_CONTEXT){
  int deg=P0.size()-1;
  bool doing_cluster=cluster_start>0 || cluster_afterend<deg;
  gen pow2=pow(2,nbits,contextptr);
  vdbl P(P0);
  dbl dr(0);
  dbl l(1);
  if (R.size()!=deg){
    dr=-find_shift(P,contextptr);
    P=shift(P,dr);
    //l=find_scale(P);
    //rescale(P,l);
    // init R if not already done, using single precision Aberth
    vfdbl fP,fR;
    if (convert(P,fP,contextptr)){
      vector<short int> zi_done(deg,false);
      double eps2=1e-4;
      bool ok;
      if (1){ // secular algorithm
	aberth_singleprec(fP,N,eps2,fR,0,deg,zi_done,1,false,contextptr);
	ok=aberth_singleprec(fP,N,eps2/16,fR,0,deg,zi_done,4,true /* secular*/ ,contextptr);	
      }
      else
	ok=aberth_singleprec(fP,2*N,eps2,fR,0,deg,zi_done,4,false,contextptr);
      Convert(fR,R);
#if 1 // exact computations are way too slow
      for (size_t j=0;j<R.size();++j)
        R[j] += dr;
      return 2;
#endif
    }
    else
      init_R(P,R);
  }
  P=*exact(P,contextptr)._VECTptr;
  R=*exact(R,contextptr)._VECTptr;
  vdbl Prev(P);
  reverse(Prev.begin(),Prev.end());
  vdbl newR(R);
  int ok=0; double delta;
  for (int k=0;k<N;++k){
    delta=0;
    if (doing_cluster){
      int K=cluster_start;
      for (;K<cluster_afterend;++K){
        if (!zi_done[K])
          break;
      }
      if (K==cluster_afterend)
        ok=1;
    }
    for (int i=0;i<deg;++i){
      if (zi_done[i]>=2 || (!ok && zi_done[i])){
        // do the last iteration if root is not already isolated
        newR[i]=R[i];
        continue;
      }
      dbl & zi=R[i];
      dbl d,d1;
      if (logabsdbl(zi)>0){
        dbl gamma(round(inv(zi),pow2,nbits));
        horner2(Prev,gamma,d,d1);
        if (!is_exactly_zero(d)){
          d=gamma*(dbl(deg)-gamma*d1/d);
          if (is_exactly_zero(d))
            return 0;
          d=inv(d);
        }
      }
      else {
        horner2(P,zi,d,d1);
        if (is_exactly_zero(d1))
          return 0;
        d=d/d1;
      }
      d=round(d,pow2,nbits);
      if (is_exactly_zero(d)){
        newR[i]=zi;
        zi_done[i]=1;
        continue;
      }
      dbl p(0); bool binv=true;
      for (int j=0;j<deg;++j){
	if (j==i) continue;
        dbl dz=zi-R[j];
        if (is_exactly_zero(dz)){
          if (doing_cluster)
            return 0;
          binv=false;
          break;
        }
	p += round(inv(dz),pow2,nbits);
      }
      double dd=absdbl(d),pp=absdbl(p);
      double abszi=absdbl(zi);
      if (debug_infolevel>2)
        *logptr(contextptr) << "cluster? i=" << i << ", delta=" << dd << ", cluster_step=" << cluster_step << ", p=" << pp << ", d*p=" << dd*pp << " " << cluster_dp << "\n";      
      if (!binv ||
          (!ok && k>N/2 && !doing_cluster && dd<=cluster_step*abszi && dd*pp>=cluster_dp)){
        // cluster of roots, find all roots in this cluster
        int cend=i+1;
        dbl sumR=zi;
        for (int k=cend;k<deg;++k){
          if (absdbl(zi-R[k])<2*deg/pp){
            swap(R[cend],R[k]);
            swap(zi_done[cend],zi_done[k]);
            sumR+=R[cend];
            ++cend;
          }
        }
        if (cend-i>1){
          // cluster is from i to cend-1 included
          dbl z=sumR/dbl(cend-i); // center of gravity of cluster
          // shift P and reverse (roots become inverse of roots)
          vdbl Pcluster(shift(P,z,false)),Rcluster(deg),initR(deg);;
          reverse(Pcluster.begin(),Pcluster.end());
          // dbl D(dbl(cend-i)/d);
          int count=deg-1;
          for (int k=0;k<deg;++k){
            if (k>=i && k<cend){
              Rcluster[k]=initR[count];
              --count;
              //dbl a(0.0,0.7+(k-i)*2*M_PI/(cend-i));
              //Rcluster[k]=D*exp(a);
            }
            else
              Rcluster[k]=inv(R[k]-z);
          }
          vector<short int> old(zi_done);
          // recursive call of aberth
          if (debug_infolevel)
            *logptr(contextptr) << "aberth exact cluster=" << i << "," << cend << "\n";
          bool b=aberth_z(Pcluster,nbits,N,eps,Rcluster,i,cend,zi_done,contextptr);
          for (int k=0;k<deg;++k){
            if (!old[k])
              R[k]=l*(inv(Rcluster[k])+z)+dr;
          }
          if (b)
            continue;
          return 0;
        }
      }
      double absdz=abszi==0?absdbl(d):absdbl(d)/abszi;
      if (binv)
        d=d/(dbl(1)-round(d*p,pow2,nbits));
      else {
        if (ok)
          return 0;
        d=d/(1-exact(std_rand()/2.0/RAND_MAX,contextptr));
      } 
      delta += absdz;
      newR[i]=round(zi-d,pow2,nbits);
      if (absdz<eps/deg){
        if (debug_infolevel>1 && !zi_done[i])
          *logptr(contextptr) << "New root found " << newR[i] << "\n";
        zi_done[i]=1;
      }
    }
    newR.swap(R);
    int count=0;
    for (int k=0;k<deg;++k){
      if (zi_done[k])
        ++count;
    }
    if (debug_infolevel>0){
      *logptr(contextptr) << "Aberth exact bits="<< nbits << " iter="<<  k  << " found=" << count << " delta=" << delta << " cluster_search=" << cluster_start <<  ".." <<cluster_afterend << " time " << clock()*1e-6 << "\n";
      if (debug_infolevel>2){
        vdbl RR(R);
        accurate_evalf(RR,45);
        *logptr(contextptr) << "Current approx roots " << RR << "\n";
      }
    }
    if (doing_cluster){
      if (ok)
        return 1;
      continue;
    }
    if (delta<=eps*(ok?deg:deg-count) || count==deg)
      ++ok;
    // else ok=0;
    if (ok==2){
      // rescale and translate
      for (size_t j=0;j<R.size();++j){
        R[j] = l*R[j] + dr;
      }
      return 1;
    }
  }
  *logptr(contextptr) << "Aberth exact: too many iterations " << N << " delta " << delta << "\n";
  return 0;  
}

bool aberth(const vdbl & P0,vdbl & R,vdbl & rayon,int N,double eps,int isolate,bool do_exact,GIAC_CONTEXT){
  if (P0.size()==2){
    R.clear();
    R.push_back(-P0[1]/P0[0]);
    rayon.clear();
    rayon.push_back(-1);
    return true;
  }
  vdbl P(P0);
  int deg=P.size()-1;
  if (deg && is_exactly_zero(P[deg])){
    P.pop_back();
    bool b=aberth(P,R,rayon,N,eps,isolate,do_exact,contextptr);
    R.insert(R.begin(),0);
    rayon.insert(rayon.begin(),0);    
    return b;
  }
  dbl dr=-find_shift(P,contextptr);
  P=shift(P,dr);
  bool realpoly=true;
  for (int i=0;realpoly && i<deg;++i){
    if (!is_exactly_zero(im(P[i],contextptr)))
      realpoly=false;
  }
  int bits=128; // nbits_start; // 512;
  R.clear();
  int Nmax=N;
  rayon.resize(P.size()-1);
  vector<short int> zi_done(P.size()-1,0);
  int bit2=0,N2=0,eps2=0;
  double eps0=eps;
  vdbl A,B;
  while (bits<=giac::ABERTH_NBITSMAX){
    //zi_done=vector<short int>(deg,0);
#ifdef HAVE_LIBMPFR    
    int b=do_exact?aberth_z(P,bits,Nmax,eps,R,0,deg,zi_done,contextptr):aberth_mpfr(P,realpoly,bits,Nmax,eps,A,B,R,rayon,0,P.size()-1,zi_done,true,isolate,true/* secular*/,contextptr);
#else
    int b=aberth_z(P,bits,Nmax,eps,R,0,deg,zi_done,contextptr);
#endif
    bool bisol=b==2;
    if (!bisol && b!=-1)
      bisol=chk_isol(R,realpoly,isolate,rayon,eps0,zi_done,contextptr);
    if (b>0){
      // if isolate is true, chk_isol will set zi_done[i] to 2 (root i is isolated) or 0
      if (b==2 || bisol){
        for (size_t j=0;j<R.size();++j){
          R[j] += dr;
        }
        return true;
      }
      if (isolate){
        ++eps2;
        if (eps>1e-4)
          eps=1e-8;
        else if (eps>1e-40)
          eps=eps*eps;
        else if (eps<=1e-280)
          return false;
        else
          eps=eps*1e-40;
        *logptr(contextptr) << "Root isolation: setting epsilon to " << eps << "\n";
        if (eps2>bit2){
          ++bit2;
          bits *=2;
          clear_zi_done(zi_done,true);
        }
        continue;
      }
    }
    ++bit2;
    bits *= 2;
    clear_zi_done(zi_done,false);
    ++N2; Nmax*=1.2;
  }
  return false;
}

  bool read_poly(const string & s,vdbl & P,GIAC_CONTEXT){
#ifndef NO_STDEXCEPT
    try {
#endif
      gen g(s,contextptr);
      g=eval(g,1,contextptr);
      if (g.type!=_IDNT){
        if (g.type==_SYMB)
          g=_symb2poly(g,contextptr);
        if (g.type==_VECT){
          P=*g._VECTptr;
          return true;
        }
      }
#ifndef NO_STDEXCEPT
    }
    catch (std::runtime_error & e){
    }
#endif
    P.clear();
    FILE * f=fopen(s.c_str(),"r");
    if (!f)
      return false;
    string S;
    while (1){
      char ch=fgetc(f);
      if (feof(f))
        break;
      S += ch;
    }
    fclose(f);
    gen g2(S,contextptr);
    g2=eval(g2,1,contextptr);
    if (g2.type==_SYMB)
      g2=_symb2poly(g2,contextptr);
    if (g2.type==_VECT){
      P=*g2._VECTptr;
    }
    return true;
  }

vdbl p_coeff(const vdbl & R){
  vdbl P;
  P.reserve(R.size()+1);
  P.push_back(dbl(1));
  for (size_t i=0;i<R.size();++i){
    dbl z=R[i];
    // P*(x-z)
    P.push_back(-z*P.back());
    for (size_t j=P.size()-2;j>=1;--j){
      P[j] -= z*P[j-1];
    }
  }
  return P;
}

// max distance between 2 elements of R
double ecart(const vdbl & R){
  int n=R.size();
  double d=1e307;
  for (int i=0;i<n;++i){
    const dbl & Ri=R[i];
    for (int j=0;j<n;++j){
      if (j==i) continue;
      double dd=absdbl(Ri-R[j]);
      if (dd<d)
        d=dd;
    }
  }
  return d;
}


#if defined HAVE_LIBMPS && defined HAVE_LIBMPFR
// time mpsolve -au -o30 -Ga mand255.pol (Aberth)
// time mpsolve -as -o30 -Ga mand255.pol (secular)

bool gen2mpq(const gen & R,mpq_t & rq){
  if (R.type==_INT_){
    mpq_set_si(rq,R.val,1);
    return true;
  }
  if (R.type==_DOUBLE_){
    mpq_set_d(rq,R._DOUBLE_val);
    return true;
  }
  if (R.type==_ZINT){
    mpq_set_z(rq,*R._ZINTptr);
    return true;
  }
  if (R.type==_FRAC){
    gen num=R._FRACptr->num,den=R._FRACptr->den;
    num.uncoerce(); den.uncoerce();
    mpq_set_num(rq,*num._ZINTptr);
    mpq_set_den(rq,*den._ZINTptr);
    return true;
  }
  return false;
}

  int mps_solve(const vdbl & P,vdbl & R,vdbl & rayon,double eps,int isolate,bool secular,GIAC_CONTEXT){
  int n=P.size()-1,nmps=53; R.clear(); rayon.clear();
  mps_context *s;
  s = mps_context_new ();
  mps_context_select_algorithm(s, secular?MPS_ALGORITHM_SECULAR_GA:MPS_ALGORITHM_STANDARD_MPSOLVE);
  nmps=eps==0?giac::ABERTH_NBITSMAX:int(ceil(-log2(std::abs(eps))));;
  if (isolate){
    mps_context_set_output_goal (s, MPS_OUTPUT_GOAL_ISOLATE);
    *logptr(contextptr) << "MPS " << (secular?"secular":"Aberth") << " goal isolate, output bits=" << nmps << "\n";
  }
  else { 
    mps_context_set_output_goal (s, MPS_OUTPUT_GOAL_APPROXIMATE);
    *logptr(contextptr) << "MPS " << (secular?"secular":"Aberth") << " goal approximate, output bits=" << nmps << "\n";
  }
  mps_context_set_output_prec (s, nmps);
  //mps_context_set_input_prec(s,0);
  int I;
  /*
  for (I=0;I<=n;++I){
    if (!is_cinteger(P[I]))
      break;
  }
  */
  mps_monomial_poly *p=0;
  if (0 && I<=n){
    string S=symb_horner(P,vx_var).print(contextptr);
    p = MPS_MONOMIAL_POLY (mps_parse_inline_poly_from_string (s,S.c_str()));
  }
  else {
    p = mps_monomial_poly_new (s, n);
    mpq_t rq,iq;
    mpq_init(rq); mpq_init(iq);
    for (int i=0;i<=n;++i){
      dbl R,I;
      reim( P[n-i],R,I,contextptr);
      if (R.type==_DOUBLE_ || I.type==_DOUBLE_ || R.type==_REAL || I.type==_REAL){
        R=evalf_double(R,1,contextptr);
        I=evalf_double(I,1,contextptr);
        mps_monomial_poly_set_coefficient_d (s, p, i,R._DOUBLE_val,I._DOUBLE_val);
        continue;
      }
      if (!gen2mpq(R,rq))
        return -1;
      if (!gen2mpq(I,iq))
        return -1;
      mps_monomial_poly_set_coefficient_q (s, p, i,rq,iq);
    }
    mpq_clear(rq); mpq_clear(iq);
    /* 
       
       mps_monomial_poly_set_coefficient_q (s, p, 0, m_one, zero); 
       mps_monomial_poly_set_coefficient_q (s, p, n, one, zero);  
       
       for (int i=0;i<=n;++i){
       dbl R,I;
       reim( evalf_double(P[n-i],1,contextptr),R,I,contextptr);
       mps_monomial_poly_set_coefficient_d (s, p, i,R._DOUBLE_val, I._DOUBLE_val); 
       }
    */
  }
  /* Set the input polynomial */
  mps_context_set_input_poly (s, MPS_POLYNOMIAL (p));
  
  /* Actually solve the polynomial */
  double t1=CLOCK()*1e-6;
  mps_mpsolve (s);
  double t2=CLOCK()*1e-6;
  *logptr(contextptr) << "MPS solve time " << t2-t1 << "\n";
  
  R.clear();
  rdpe_t * drad = rdpe_valloc (n); // disk radius
  mpc_t * mroot = mpc_valloc (n);
  mpc_vinit2 (mroot, n, nmps);   
  mps_context_get_roots_m (s, &mroot, &drad);
  mpfr_t tmp; mpfr_init2(tmp,64);
  for (int i=0;i<n;++i){
    mpfr_t mr,mi; dbl gr,gi;
    double d1=rdpe_log(drad[i]);
    rayon.push_back(exp(d1));
    mpfr_set_f(tmp,mpc_Re(mroot[i]),MPFR_RNDN);
    gr=gen(real_object(tmp));
    if (!is_exactly_zero(gr)){
      double d2=logabsdbl(gr);
      double d=d2-d1;
      if (d<0)
        gr=0;
      else {
        int prec=d/M_LN2; // mpf_get_prec(mpc_Re(mroot[i]));
        mpfr_init2(mr,prec); 
        mpfr_set_f(mr,mpc_Re(mroot[i]),MPFR_RNDN);
        gr=gen(real_object(mr));
        mpfr_clear(mr);
      }
    }
    mpfr_set_f(tmp,mpc_Im(mroot[i]),MPFR_RNDN);
    gi=gen(real_object(tmp));
    if (!is_exactly_zero(gi)){
      // cout << i << " " << gi << "\n";
      double d2=logabsdbl(gi);
      double d=d2-d1;
      if (d<0)
        gi=0;
      else {
        int prec=d/M_LN2; // mpf_get_prec(mpc_Im(mroot[i]));
        mpfr_init2(mi,prec);
        mpfr_set_f(mi,mpc_Im(mroot[i]),MPFR_RNDN);
        gi=gen(real_object(mi));
        mpfr_clear(mi);
      }
    }
    R.push_back(gen(gr,gi));
  }
  mpfr_clear(tmp);
  mpc_vclear (mroot, n);
  free (mroot);
  free (drad);
  // mps_monomial_poly_free(s,p); // expects a mps_polynomial * ??
  mps_context_free(s);
  return 0;
}
#else
  int mps_solve(const vdbl & P,vdbl & R,vdbl & rayon,double eps,int isolate,bool secular,GIAC_CONTEXT){
    return -1;
  }
#endif
  

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
