// -*- mode:C++ ; compile-command: "g++ -I.. -g -c -fno-strict-aliasing -DGIAC_GENERIC_CONSTANTS -DHAVE_CONFIG_H -DIN_GIAC solve.cc" -*-
#include "giacPCH.h" 

/*
 *  Copyright (C) 2001,14 B. Parisse, R. De Graeve
 *  Institut Fourier, 38402 St Martin d'Heres
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
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include "gen.h"
#include "solve.h"
#include "modpoly.h"
#include "unary.h"
#include "symbolic.h"
#include "usual.h"
#include "sym2poly.h"
#include "subst.h"
#include "derive.h"
#include "plot.h"
#include "prog.h"
#include "series.h"
#include "alg_ext.h"
#include "intg.h"
#include "rpn.h"
#include "lin.h"
#include "misc.h"
#include "cocoa.h"
#include "ti89.h"
#include "maple.h"
#include "csturm.h"
#include "sparse.h"
#include "giacintl.h"
#ifdef HAVE_LIBGSL
#include <gsl/gsl_roots.h>
#include <gsl/gsl_multiroots.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_vector.h>
#endif

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  // FIXME intvar_counter should be contextized
  int intvar_counter=0;
  int realvar_counter=0;
  string print_intvar_counter(GIAC_CONTEXT){
    if (intvar_counter<0)
      return print_INT_(-intvar_counter);
    string res=print_INT_(intvar_counter);
    ++intvar_counter;
    return res;
  }

  string print_realvar_counter(GIAC_CONTEXT){
    if (realvar_counter<0)
      return print_INT_(int(-realvar_counter));
    string res=print_INT_(int(realvar_counter));
    ++realvar_counter;
    return res;
  }

  gen _reset_solve_counter(const gen & args,const context * contextptr){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (is_zero(args,contextptr)){
      intvar_counter=0;
      return 1;
    }
    if (is_one(args)){
      realvar_counter=0;
      return 1;
    }
    if (args.type==_VECT && args._VECTptr->size()==2){
      intvar_counter=int(evalf_double(args._VECTptr->front(),1,contextptr)._DOUBLE_val);
      realvar_counter=int(evalf_double(args._VECTptr->back(),1,contextptr)._DOUBLE_val);      
    }
    else {
      intvar_counter=0;
      realvar_counter=0;
    }
    return 1;
  }
  static const char _reset_solve_counter_s []="reset_solve_counter";
  static define_unary_function_eval (___reset_solve_counter,&_reset_solve_counter,_reset_solve_counter_s);
  define_unary_function_ptr5( at_reset_solve_counter ,alias_at_reset_solve_counter,&___reset_solve_counter,0,true);

  void set_merge(vecteur & v,const vecteur & w){
    if (is_undef(w)){
      v=w;
      return;
    }
    const_iterateur it=w.begin(),itend=w.end();
    for (;it!=itend;++it)
      if (!equalposcomp(v,*it))
	v.push_back(*it);
  }

  static gen one_tour(GIAC_CONTEXT){
    if (angle_radian(contextptr)) 
      return cst_two_pi;
    else if(angle_degree(contextptr))
      return 360;
    //grad
    else
      return 400;
  }
  static gen one_half_tour(GIAC_CONTEXT){
    if (angle_radian(contextptr)) 
      return cst_pi;
    else if(angle_degree(contextptr))
      return 180;
    //grad
    else
      return 200;
  }
  static gen isolate_exp(const gen & e,int isolate_mode,GIAC_CONTEXT){
    if (isolate_mode &1)
      return ln(e,contextptr);
    if (e.type!=_VECT){
      if (is_strictly_positive(-e,contextptr))
	return vecteur(0);
      else
	return ln(e,contextptr);
    }
    // check in real mode for negative ln
    const_iterateur it=e._VECTptr->begin(),itend=e._VECTptr->end();
    vecteur res;
    for (;it!=itend;++it){
      if (!is_strictly_positive(-*it,contextptr))
	res.push_back(ln(*it,contextptr));
    }
    return res;
  }
  static gen isolate_ln(const gen & e,int isolate_mode,GIAC_CONTEXT){
    return simplify(exp(e,contextptr),contextptr);
  }
  static gen isolate_sin(const gen & e,int isolate_mode,GIAC_CONTEXT){
    gen asine=asin(e,contextptr);
    if (!(isolate_mode & 2))
      return makevecteur(asine,one_half_tour(contextptr)-asine);
    identificateur x(string("n_")+print_intvar_counter(contextptr));
    if (is_zero(e,contextptr))
      return asine+(x)*one_half_tour(contextptr);
    return makevecteur(asine+(x)*one_tour(contextptr),one_half_tour(contextptr)-asine+(x)*one_tour(contextptr));
  }
  static gen isolate_cos(const gen & e,int isolate_mode,GIAC_CONTEXT){
    gen acose=acos(e,contextptr);
    if (!(isolate_mode & 2))
      return makevecteur(acose,-acose);
    identificateur x(string("n_")+print_intvar_counter(contextptr));
    if (is_zero(e,contextptr))
      return acose+(x)*one_half_tour(contextptr);
    return makevecteur(acose+(x)*one_tour(contextptr),-acose+(x)*one_tour(contextptr));
  }
  static gen isolate_tan(const gen & e,int isolate_mode,GIAC_CONTEXT){
    if (!(isolate_mode & 2))
      return atan(e,contextptr);
    identificateur x(string("n_")+print_intvar_counter(contextptr));
    return atan(e,contextptr)+(x)*one_half_tour(contextptr);
  }
  static gen isolate_asin(const gen & e,int isolate_mode,GIAC_CONTEXT){
    return sin(e,contextptr);
  }
  static gen isolate_acos(const gen & e,int isolate_mode,GIAC_CONTEXT){
    return cos(e,contextptr);
  }
  static gen isolate_atan(const gen & e,int isolate_mode,GIAC_CONTEXT){
    return tan(e,contextptr);
  }

  static gen isolate_asinh(const gen & e,int isolate_mode,GIAC_CONTEXT){
    return sinh(e,contextptr);
  }
  static gen isolate_acosh(const gen & e,int isolate_mode,GIAC_CONTEXT){
    return cosh(e,contextptr);
  }
  static gen isolate_atanh(const gen & e,int isolate_mode,GIAC_CONTEXT){
    return tanh(e,contextptr);
  }

  static gen isolate_sinh(const gen & e,int isolate_mode,GIAC_CONTEXT){
    gen asine= asinh(e,contextptr);
    if ((isolate_mode & 3)!=3)
      return asine;
    identificateur * x=new identificateur(string("n_")+print_intvar_counter(contextptr));
    return makevecteur(asine+(*x)*one_tour(contextptr)*cst_i,(one_half_tour(contextptr)+(*x)*one_tour(contextptr))*cst_i-asine);
  }
  static gen isolate_cosh(const gen & e,int isolate_mode,GIAC_CONTEXT){
    gen acose=acosh(e,contextptr);
    if ((isolate_mode & 3)!=3)
      return makevecteur(acose,-acose);
    identificateur * x=new identificateur(string("n_")+print_intvar_counter(contextptr));
    return makevecteur(acose+(*x)*one_tour(contextptr)*cst_i,-acose+(*x)*one_tour(contextptr)*cst_i);
  }
  static gen isolate_tanh(const gen & e,int isolate_mode,GIAC_CONTEXT){
    if ((isolate_mode & 3)!=3)
      return atanh(e,contextptr);
    identificateur * x=new identificateur(string("n_")+print_intvar_counter(contextptr));
    return atanh(e,contextptr)+(*x)*one_half_tour(contextptr)*cst_i;
  }

  static gen (* const isolate_fcns[] ) (const gen &,int,GIAC_CONTEXT) = { isolate_exp,isolate_ln,isolate_sin,isolate_cos,isolate_tan,isolate_asin,isolate_acos,isolate_atan,isolate_sinh,isolate_cosh,isolate_tanh,isolate_asinh,isolate_acosh,isolate_atanh};

  static vecteur find_excluded(const gen & g,GIAC_CONTEXT){
    if (g.type!=_IDNT)
      return vecteur(0);
    gen g2=g._IDNTptr->eval(eval_level(contextptr),g,contextptr);
    if ((g2.type==_VECT) && (g2.subtype==_ASSUME__VECT)){
      vecteur v=*g2._VECTptr;
      if ( v.size()==3 && v[0]!=_INT_ && v[2].type==_VECT ){
	return *v[2]._VECTptr;
      }
    }
    return vecteur(0);
  }

  static bool check(const gen & id,const gen & value,const gen & ids,const gen & vals,GIAC_CONTEXT){
    if (is_inequation(value))
      return true; // FIXME!!
    if (id.type==_VECT && value.type==_VECT && id._VECTptr->size()==value._VECTptr->size()){
      for (unsigned i=0;i<id._VECTptr->size();++i){
	if (!check((*id._VECTptr)[i],(*value._VECTptr)[i],ids,vals,contextptr))
	  return false;
      }
      return true;
    }
    if (id.type!=_IDNT)
      return true;
    gen g,g2=id._IDNTptr->eval(1,g,contextptr);
    g2=subst(g2,ids,vals,false,contextptr);
    if ((g2.type==_VECT) && (g2.subtype==_ASSUME__VECT)){
      vecteur v=*g2._VECTptr;
      if (!v.empty() && v[0].type==_INT_ && (v[0].val==_INT_ || v[0].val==_ZINT) && value.type!=_IDNT && value.type!=_SYMB && !is_integer(value))
	return false;
      if ( v.size()==3 && v[1].type==_VECT && v[2].type==_VECT){
	for (unsigned i=0;i<v[2]._VECTptr->size();++i){
	  if (value==(*v[2]._VECTptr)[i])
	    return false;
	}
	int loupe=0;
	for (unsigned i=0;i<v[1]._VECTptr->size();++i){
	  gen tmp=(*v[1]._VECTptr)[i];
	  if (tmp.type==_VECT && tmp._VECTptr->size()==2){
	    gen a=tmp._VECTptr->front(),b=tmp._VECTptr->back();
	    if (is_strictly_greater(a,value,contextptr) || is_strictly_greater(value,b,contextptr))
	      loupe++;
	    else {
	      if (is_greater(value,a,contextptr) && is_greater(b,value,contextptr))
		break;
	    }
	  }
	}
	if (loupe==int(v[1]._VECTptr->size())) // all tests above returned false
	  return false;
      }
    }
    return true;
  }

  // Fix isolate_mode if g is assumed to be in a given interval
  static void ck_isolate_mode(int & isolate_mode,const gen & g,GIAC_CONTEXT){
    if ( (isolate_mode& 2) ||  g.type!=_IDNT)
      return ;
    gen g2=g._IDNTptr->eval(eval_level(contextptr),g,contextptr);
    if (g2.type==_VECT && g2.subtype==_ASSUME__VECT){
      vecteur v=*g2._VECTptr;
      if ( v.size()==3 && v[0]!= _INT_ && v[1].type==_VECT && !v[1]._VECTptr->empty()){
	gen a=v[1]._VECTptr->front(),b=v[1]._VECTptr->back();
	if (a.type==_VECT && !a._VECTptr->empty() && !is_inf(a._VECTptr->front()) && b.type==_VECT && !b._VECTptr->empty() && !is_inf(b._VECTptr->back()))
	  isolate_mode |= 2;
      }
    }
  }

  vecteur protect_sort(const vecteur & res,GIAC_CONTEXT){
#ifndef NO_STDEXCEPT
    try {
#endif
      gen tmp=_sort(res,contextptr);
      if (tmp.type==_VECT){
	vecteur w=*tmp._VECTptr,res;
	iterateur it=w.begin(),itend=w.end();
	for (;it!=itend;++it){
	  if (res.empty() || *it!=res.back())
	    res.push_back(*it);
	}
	return res;
      }
#ifndef NO_STDEXCEPT
    }
    catch (std::runtime_error & e){
      last_evaled_argptr(contextptr)=NULL;
      CERR << e.what() << '\n';
    }
#endif
    return res;
  }

  bool is_inequation(const gen & g){
    return g.is_symb_of_sommet(at_superieur_strict) || g.is_symb_of_sommet(at_superieur_egal) 
      || g.is_symb_of_sommet(at_inferieur_strict) || g.is_symb_of_sommet(at_inferieur_egal);
  }

  vecteur find_singularities(const gen & e,const identificateur & x,int cplxmode,GIAC_CONTEXT){
    vecteur lv(lvarxpow(e,x));
    if (cplxmode & 8){
      lv=mergevecteur(lv,lvarxwithinv(e,x,contextptr));
      cplxmode ^= 8;
    }
    int cplxmodeorig=cplxmode;
    bool add=(cplxmode & 0x10);
    if (add)
      cplxmode ^= 0x10;
    vecteur res;
    vecteur l(lvar(e));
    gen p=e2r(e,l,contextptr),n,d;
    vecteur pv=gen2vecteur(p);
    const_iterateur jt=pv.begin(),jtend=pv.end();
    for (;jt!=jtend;++jt){
      fxnd(*jt,n,d);
      if (d.type==_POLY){
	res=solve(r2e(d,l,contextptr),x,cplxmode,contextptr);
      }
      if (is_undef(res))
	return res;
    }
    const_iterateur it=lv.begin(),itend=lv.end();
    for (;it!=itend;++it){
      if (it->type!=_SYMB)
	continue;
      const unary_function_ptr & u=it->_SYMBptr->sommet;
      gen & f=it->_SYMBptr->feuille;
      res=mergeset(res,find_singularities(f,x,cplxmodeorig,contextptr));
      if (u==at_ln || u==at_sign)
	res=mergeset(res,solve(f,x,cplxmode,contextptr));
      if (add && (u==at_asin || u==at_acos)){
        res=mergeset(res,solve(f-1,x,cplxmode,contextptr));
        res=mergeset(res,solve(f+1,x,cplxmode,contextptr));
      }
      if (u==at_pow && f.type==_VECT && f._VECTptr->size()==2)
	res=mergeset(res,solve(f._VECTptr->front(),x,cplxmode,contextptr));	
      if (u==at_tan)
	res=mergeset(res,solve(cos(f,contextptr),x,cplxmode,contextptr));
      if (u==at_piecewise && f.type==_VECT){
	vecteur & v = *f._VECTptr;
	int s=int(v.size());
	for (int i=0;i<s-1;i+=2){
	  gen & e =v[i];
	  if (is_inequation(e)){
	    vecteur tmp=solve(e._SYMBptr->feuille._VECTptr->front()-e._SYMBptr->feuille._VECTptr->back(),x,cplxmode,contextptr);
	    // is *it continuous at tmp
	    gen etoileit=subst(*it,undef,identificateur("undef_"),false,contextptr);
	    const_iterateur jt=tmp.begin(),jtend=tmp.end();
	    for (;jt!=jtend;++jt){
	      if (!is_zero(limit(etoileit,x,*jt,1,contextptr)-limit(etoileit,x,*jt,-1,contextptr),contextptr))
		res.push_back(*jt);
	    }
	  }
	}
      }
    }
    if (cplxmode)
      return res;
    return protect_sort(res,contextptr);
  }

  vecteur protect_find_singularities(const gen & e,const identificateur & x,int cplxmode,GIAC_CONTEXT){
    //int C=calc_mode(contextptr);
    //calc_mode(0,contextptr);
    vecteur sp;
#ifdef NO_STDEXCEPT
    sp=find_singularities(e,x,cplxmode,contextptr);
    if (is_undef(sp)){
      *logptr(contextptr) << sp << '\n';      
      // sp.clear(); 
    }
#else
    try {
      sp=find_singularities(e,x,cplxmode,contextptr);
    }
    catch (std::runtime_error & e){
      last_evaled_argptr(contextptr)=NULL;
      *logptr(contextptr) << e.what() << '\n';
      sp=vecteur(1,undef);
      // sp.clear();
    }
#endif
    //calc_mode(C,contextptr);
    return sp;
  }

  static void solve_ckrange(const identificateur & x,vecteur & v,int isolate_mode,GIAC_CONTEXT){
    vecteur w,excluded(find_excluded(x,contextptr));
    // assumption on x, either range or integer
    int fr=find_range(x,w,contextptr);
    // optimization does not work because some tests are done after
    // examples assume(n>0);solve(((n)^(2))*(((x)/(n))^(log10(x)))=(x)^(2),x);
    if (0 && fr==1 && w.size()==1 && w.front().type==_VECT && w.front()._VECTptr->front()==minus_inf && w.front()._VECTptr->back()==plus_inf){
      return;
    }
    if (fr>=2){
      int s=int(v.size());
      for (int i=0;i<s;++i){
	if (is_integer(v[i]))
	  w.push_back(v[i]);
      }
      v=w;
      if (fr==2)
	return;
    }
    if (w.size()!=1 || w.front().type!=_VECT)
      return;
    gen wfront=w.front();
    w=*wfront._VECTptr;
    if (w.size()!=2)
      return;
    gen l,m;
    vecteur newv;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      l=w[0];m=w[1];
      *it=simplifier(*it,contextptr); *it=simplifier(*it,contextptr); // 2nd simplifier required for solve(asin(x)=acos(x))
      if (equalposcomp(excluded,*it))
	continue;
      gen sol=*it;
      if (l!=minus_inf && fastsign(l-sol,contextptr)==1)
	continue;
      if (m!=plus_inf && fastsign(sol-m,contextptr)==1)
	continue;
      sol=evalf(sol,eval_level(contextptr),contextptr);
      if (!(isolate_mode &1) && ( (sol.type==_CPLX && !is_zero(im(sol,contextptr),contextptr))
				  || (sol.type!=_CPLX && has_i(sol))))
	continue;
      if (sol.type!=_DOUBLE_){ // check for trig solutions
	newv.push_back(*it);
	vecteur lv(lidnt(sol));
	if (lv.size()!=1 || l==minus_inf || m==plus_inf)
	  continue;
	gen n(lv.front()),a,b,expr(*it);
	expr=pow2expln(expr,contextptr);
	// check linearity
	while (expr.type==_SYMB){
	  vecteur varn(lvarx(expr,n));
	  if (varn.size()!=1)
	    break;
	  if (!is_linear_wrt(expr,varn.front(),a,b,contextptr))
	    break;
	  expr=varn.front();
	  l=ratnormal((l-b)/a);
	  m=ratnormal((m-b)/a);
	  if (is_strictly_positive(-a,contextptr))
	    swapgen(l,m);
	  if (expr.is_symb_of_sommet(at_ln)){
	    l=exp(l,contextptr);
	    m=exp(m,contextptr);
	    expr=expr._SYMBptr->feuille;
	    continue;
	  }
	  if (expr.is_symb_of_sommet(at_exp)){
	    if (is_positive(l,contextptr)){
	      l=l==0?minus_inf:ln(l,contextptr);
	      m=m==0?minus_inf:ln(m,contextptr);
	    }
	    else
	      l=m=minus_inf;
	    expr=expr._SYMBptr->feuille;
	    continue;
	  }
	  break;
	}
	if (is_inf(l) || n.type!=_IDNT || n.print(contextptr).substr(0,2)!="n_" || !is_linear_wrt(expr,n,a,b,contextptr)){
	  *logptr(contextptr) << gettext("Warning: unable to find ") <<n << gettext(" integer solutions for ") << expr << ">=" << l << gettext(" and <=") << m << gettext(", answer may be wrong.\nIf you are computing an integral with exact boundaries, replace by approx. boundaries") << '\n';
	  if (v.size()!=1) v=vecteur(1,undef);
	  return;
	}
	newv.pop_back();
	a=normal(a,contextptr);
	b=normal(b,contextptr);
	if (!is_positive(a,contextptr))
	  swapgen(l,m);
	gen tmp=(l-b)/a;
#if defined HAVE_LIBMPFR && !defined NO_STDEXCEPT
	try {
	  if (tmp.type!=_FRAC && tmp.type!=_EXT)
	    tmp=accurate_evalf(tmp,1000);
	} catch (std::runtime_error & ) {
	  last_evaled_argptr(contextptr)=NULL;
	}
#endif
	tmp=evalf(tmp,eval_level(contextptr),contextptr);
	int n0(_ceil(tmp,contextptr).val);
	tmp=(m-b)/a;
#if defined HAVE_LIBMPFR && !defined NO_STDEXCEPT
	try {
	  if (tmp.type!=_FRAC && tmp.type!=_EXT)
	    tmp=accurate_evalf(tmp,1000);
	} catch (std::runtime_error & ) {
	  last_evaled_argptr(contextptr)=NULL;
	}
#endif
	tmp=evalf(tmp,eval_level(contextptr),contextptr);
	int n1(_floor(tmp,contextptr).val);
	for (;n0<=n1;++n0){
	  gen sol=ratnormal(subst(*it,n,n0,false,contextptr),contextptr);
	  if (!equalposcomp(excluded,sol))
	    newv.push_back(sol);
	}
      }
      else {
	if (is_strictly_greater(l,sol,contextptr))
	  continue;
	if (is_strictly_greater(sol,m,contextptr))
	  continue;
	newv.push_back(*it);
      }
    }
    v=newv;
  }

  // Helper for the solver, make a translation using x^(n-1) coeff
  // and find gcd of deg, return true if non-trivial gcd found
  static bool translate_gcddeg(const vecteur & v,vecteur & v_translated, gen & x_translation,int & gcddeg){
    int s=int(v.size());
    if (s<4)
      return false;
    x_translation=-v[1]/((s-1)*v[0]);
    v_translated=taylor(v,x_translation,0);
    gcddeg=0;
    for (int i=1;i<s;++i){
      if (!is_zero(v_translated[i]))
	gcddeg=gcd(gcddeg,i);
    }
    if (gcddeg<=1)
      return false;
    int newdeg=(s-1)/gcddeg+1;
    // compress v_translated, keep only terms with index multiple of gcddeg
    for (int i=1;i<newdeg;++i){
      v_translated[i]=v_translated[i*gcddeg];
    }
    v_translated=vecteur(v_translated.begin(),v_translated.begin()+newdeg);
    return true;
  }

  static vecteur solve_inequation(const gen & e0,const identificateur & x,int direction,GIAC_CONTEXT);

  static vecteur solve_piecewise(const gen & args_,const gen & value,const identificateur & x,int isolate_mode,GIAC_CONTEXT){
    gen args=_exp2pow(args_,contextptr);
    if (is_undef(args))
      args=args_;
    if (args.type!=_VECT)
      return vecteur(1,gensizeerr(contextptr));
    vecteur & piece_args=*args._VECTptr;
    vecteur failtest; // all these tests must fail to keep solution
    gen successtest,equation; // this test must succeed
    int s=int(piece_args.size());
    vecteur res;
    for (int i=0;i<s;i+=2){
      if (i)
	failtest.push_back(successtest);
      if (i+1==s){
	successtest=1;
	equation=piece_args[i];
      }
      else {
	successtest=piece_args[i];
	equation=piece_args[i+1];
      }
      int fails=int(failtest.size());
      vecteur sol=solve(equation-value,x,isolate_mode,contextptr);
      // now test whether solutions in sol are acceptable
      const_iterateur it=sol.begin(),itend=sol.end();
      for (;it!=itend;++it){
	const gen & g=*it;
	if (g==x){
	  if (fails){
	    gen tmp=symb_not(symbolic(at_ou,gen(failtest,_SEQ__VECT)));
	    res.push_back(is_one(successtest)?tmp:symb_and(tmp,successtest));
	  }
	  else
	    res.push_back(is_one(successtest)?g:successtest);
	  continue;
	}
	if (!is_zero(derive(g,x,contextptr),contextptr)){
	  if (fails){
	    gen tmp=symb_not(symbolic(at_ou,gen(failtest,_SEQ__VECT)));
	    tmp=is_one(successtest)?tmp:symb_and(tmp,successtest);
	    res.push_back(symb_and(tmp,g));
	  }
	  else
	    res.push_back(is_one(successtest)?g:symb_and(successtest,g));
	  continue;
	}
	int j;
	for (j=0;j<fails;++j){
	  if (is_one(subst(failtest[j],x,g,false,contextptr)))
	    break;
	}
	if (j==fails && is_one(subst(successtest,x,g,false,contextptr)))
	  res.push_back(g);
      }
    }
    return res;
  }

  // inner solver
  void in_solve(const gen & e,const identificateur & x,vecteur &v,int isolate_mode,GIAC_CONTEXT){
    if (has_op(e,*at_equal) || has_op(e,*at_equal2)){
      v=vecteur(1,gensizeerr(gettext("Bad equal in")+e.print(contextptr)));
      return;
    }
    if (is_inequation(e)){
      if (e._SYMBptr->sommet==at_inferieur_strict)
	v=solve_inequation(e,x,-2,contextptr);
      else if (e._SYMBptr->sommet==at_inferieur_egal)
	v=solve_inequation(e,x,-1,contextptr);
      else if (e._SYMBptr->sommet==at_superieur_strict)
	v=solve_inequation(e,x,2,contextptr);
      else if (e._SYMBptr->sommet==at_superieur_egal)
	v=solve_inequation(e,x,1,contextptr);
      return;
    }
    bool complexmode=isolate_mode & 1;
    vecteur lv(lvarx(e,x));
    int s=int(lv.size());
    if (!s)
      return;
    if (s>1){
      if (s==2 && lv[1]==x)
	swapgen(lv[0],lv[1]);
      gen m1(-exp(-1,contextptr));
      if (s==2 && lv[0]==x && (isolate_mode & 1)==0){
	gen a,b,newe(e);
	if (0 && lv[1].is_symb_of_sommet(at_pow)){ // not reached, not tested
	  gen f=lv[1]._SYMBptr->feuille;
	  if (f.type==_VECT && f._VECTptr->size()==2){
	    f=symb_exp(f._VECTptr->back()*ln(f._VECTptr->front(),contextptr));
	    newe=subst(e,lv[1],f,false,contextptr);
	    lv[1]=f;
	  }
	}
	if (lv[1].is_symb_of_sommet(at_ln) && is_linear_wrt(lv[1]._SYMBptr->feuille,x,a,b,contextptr) && a!=0){
	  // ln(ax+b)=t -> x=(exp(t)-b)/a, solve in t
	  gen newx=(symb_exp(x)-b)/a;
	  newe=subst(e,lv,makevecteur(newx,x),false,contextptr);
	  vecteur newv;
	  in_solve(newe,x,newv,isolate_mode,contextptr);
	  const_iterateur it=newv.begin(),itend=newv.end();
	  for (;it!=itend;++it){
	    v.push_back((exp(*it,contextptr)-b)/a);
	  }
	  return;
	}
	if (lv[1].is_symb_of_sommet(at_exp) && is_linear_wrt(lv[1]._SYMBptr->feuille,x,a,b,contextptr) && a!=0){
	  gen A,B,C,D,E,F;
	  if (is_linear_wrt(e,lv[1],A,B,contextptr) && A!=0 && is_linear_wrt(B,x,C,D,contextptr) && is_linear_wrt(A,x,E,F,contextptr)){
	    if (E==0){
	      // A*exp(a*x+b)+C*x+D=0, t=a*x+b
	      // A*exp(t)+C*(t-b)/a+D=0 -> A*exp(t)+C/a*t+D-C*b/a=0
	      gen a_(A),b_(C/a),c_(D-C*b/a);
	      gen delta=ratnormal(a_/b_*exp(-c_/b_,contextptr),contextptr);
	      if (is_strictly_greater(m1,delta,contextptr))
		return; // no solution
	      gen sol=-_LambertW(delta,contextptr)-c_/b_;
	      v.push_back((sol-b)/a);
	      if (is_positive(delta,contextptr)|| delta==m1)
		return;
	      sol=-_LambertW(makesequence(delta,-1),contextptr)-c_/b_;
	      v.push_back((sol-b)/a);
	      return;
	    }
	    if (C==0){
	      // (E*x+F)*exp(a*x+b)+D==0
	      // write a*x+b=a/E*(E*x+F)+b-a*F/E
	      // a/E*(E*x+F)*exp(a/E*(E*x+F))=-D*a/E*exp(a*F/E-b)
	      gen delta=-D*a/E*exp(a*F/E-b,contextptr);
	      if (is_strictly_greater(m1,delta,contextptr))
		return; // no solution
	      gen sol=_LambertW(delta,contextptr)/a-F/E;
	      v.push_back(sol);
	      if (is_positive(delta,contextptr)|| delta==m1)
		return;
	      sol=_LambertW(makesequence(delta,-1),contextptr)/a-F/E;
	      v.push_back(sol);
	      return;
	    }
	  }
	  // lv[1]=exp(a*x+b), set exp(ax+b)=t/(ax+b), 
	  // if equation does not depend on x, solve in t, then x=(W(t)-b)/a
	  gen t(identificateur(" t"));
	  gen rep=subst(newe,lv[1],t/lv[1]._SYMBptr->feuille,false,contextptr),xfact(1),tfact(1);
	  rep=_factors(rep,contextptr);
	  if (separate_variables(rep,x,t,xfact,tfact,0,contextptr)){
	    vecteur vt;
	    in_solve(tfact,*t._IDNTptr,vt,isolate_mode,contextptr);
	    const_iterateur it=vt.begin(),itend=vt.end();
	    for (;it!=itend;++it){
	      if (is_strictly_greater(m1,*it,contextptr))
		continue;
	      v.push_back((_LambertW(*it,contextptr)-b)/a);
	      if (is_strictly_positive(-*it,contextptr))
		v.push_back((_LambertW(makesequence(*it,-1),contextptr)-b)/a);
	    }
	    return;
	  }
	  // try with exp(ax+b)=-(ax+b)/t
	  rep=subst(e,lv[1],-lv[1]._SYMBptr->feuille/t,false,contextptr);
	  xfact=1;tfact=1;
	  rep=_factors(rep,contextptr);
	  if (separate_variables(rep,x,t,xfact,tfact,0,contextptr)){
	    vecteur vt;
	    in_solve(tfact,*t._IDNTptr,vt,isolate_mode,contextptr);
	    const_iterateur it=vt.begin(),itend=vt.end();
	    for (;it!=itend;++it){
	      if (is_strictly_greater(m1,*it,contextptr))
		continue;
	      v.push_back((b-_LambertW(*it,contextptr))/a);
	      if (is_strictly_positive(-*it,contextptr))
		v.push_back((b-_LambertW(makesequence(*it,-1),contextptr))/a);
	    }
	    return;
	  }
	}
      }
      for (int i=0;i<s;++i){
	gen xvar=lv[i];
	if (xvar._SYMBptr->sommet==at_sign){
	  gen new_e=subst(e,xvar,1,false,contextptr);
	  vecteur vplus;
	  in_solve(new_e,x,vplus,isolate_mode,contextptr);
	  const_iterateur it=vplus.begin(),itend=vplus.end();
	  for (;it!=itend;++it){
	    if (is_one(subst(xvar,x,*it,false,contextptr)))
	      v.push_back(*it);
	  }
	  new_e=subst(e,xvar,-1,false,contextptr);
	  vecteur vminus;
	  in_solve(new_e,x,vminus,isolate_mode,contextptr);
	  it=vminus.begin();
	  itend=vminus.end();
	  for (;it!=itend;++it){
	    if (is_one(-subst(xvar,x,*it,false,contextptr)))
	      v.push_back(*it);
	  }
	  return;
	}
      }
      if (lidnt(e)==vecteur(1,x)){
	// if the equation does not depend on parameters
	// and the variable is assumed to live in a finite interval 
	// try bisection solver
	vecteur a;
	gen a0,a1;
	if (find_range(x,a,contextptr)==1 && a.size()==1){
	  gen A=a.front();
	  if (A.type==_VECT && A._VECTptr->size()==2 && (a0=A._VECTptr->front())!=minus_inf && (a1=A._VECTptr->back())!=plus_inf){
	    int iszero=-1;
	    a=bisection_solver(e,x,a0,a1,iszero,contextptr);
	    if (iszero==1 || iszero==0){
	      *logptr(contextptr) << gettext("Unable to isolate ")+string(x.print(contextptr))+" in "+e.print(contextptr) << gettext(", switching to approx. solutions") << '\n';
	      v=mergevecteur(v,a);
	      return;
	    }
	  }
	}
      }
#if 1
      string msg=gettext("Unable to isolate ")+string(x.print(contextptr))+" in "+e.print(contextptr);
      if (calc_mode(contextptr)==1){ // for solve(ln(exp(x)+1)=x) 
	v.push_back(undef); // (string2gen(msg,false));
	return;
      }
      *logptr(contextptr) << msg+gettext(", switching to approx. solutions") << '\n';
      gen a=_fsolve(makesequence(e,x),contextptr);
      if (a.type==_VECT)
	v=mergevecteur(v,*a._VECTptr);
      else 
	if (!is_undef(a) && !a.is_symb_of_sommet(at_fsolve)) v.push_back(a);
      return;
#else
#ifndef NO_STDEXCEPT
      throw(std::runtime_error("Unable to isolate "+string(x.print(contextptr))+" in "+e.print(contextptr)));
#endif
      v=vecteur(1,undeferr(gettext("Unable to isolate ")+string(x.print(contextptr))+" in "+e.print(contextptr)));
      return;
#endif
    }
    gen xvar(lv.front());
    if (xvar!=x){ // xvar must be a unary function of x, except for a few special cases
      if (xvar.type!=_SYMB){
	v=vecteur(1,gentypeerr(contextptr));
	return;
      }
      if (xvar._SYMBptr->sommet!=at_piecewise && xvar._SYMBptr->feuille.type==_VECT){
	if ((xvar._SYMBptr->sommet==at_NTHROOT && xvar._SYMBptr->feuille.type==_VECT && xvar._SYMBptr->feuille._VECTptr->size()==2 && is_integer(xvar._SYMBptr->feuille._VECTptr->front())))
	  ;
	else {
#ifndef NO_STDEXCEPT
	  throw(std::runtime_error("Unable to isolate "+string(x.print(contextptr))+" in "+xvar.print(contextptr)));
#endif
	  v=vecteur(1,undeferr(gettext("Unable to isolate ")+string(x.print(contextptr))+" in "+xvar.print(contextptr)));
	  return;
	}
      }
      if (xvar._SYMBptr->sommet==at_sign){
	gen new_e=subst(e,xvar,1,false,contextptr);
	if (is_zero(new_e,contextptr)){
	  v=solve_inequation(symbolic(at_superieur_strict,makesequence(xvar._SYMBptr->feuille,0)),x,1,contextptr);
	}
	else {
	  new_e=subst(e,xvar,-1,false,contextptr);
	  if (is_zero(new_e,contextptr)){
	    v=solve_inequation(symbolic(at_inferieur_strict,makesequence(xvar._SYMBptr->feuille,0)),x,-1,contextptr);
	  }
	}
	return;
      }
      int pos=equalposcomp(solve_fcns_tab,xvar._SYMBptr->sommet);
      if (xvar._SYMBptr->sommet==at_piecewise)
	pos=-1;
      if (xvar._SYMBptr->sommet==at_NTHROOT)
	pos=-2;
      if (xvar._SYMBptr->sommet==at_factorial && e==xvar){
        return;
      }
      if (!pos){
#ifndef NO_STDEXCEPT
	throw(std::runtime_error(string(gettext("Unable to isolate function "))+xvar._SYMBptr->sommet.ptr()->print(contextptr)));
#endif
	v=vecteur(1,undeferr(string(gettext("Unable to isolate function "))+xvar._SYMBptr->sommet.ptr()->print(contextptr)));
	return;
      }
      // solve with respect to xvar
      identificateur localt(" t");
      // ck_parameter_t();
      gen new_e=subst(e,xvar,localt,false,contextptr);
      vecteur new_v=solve(new_e,localt,isolate_mode,contextptr);
      const_iterateur it=new_v.begin(),itend=new_v.end();
      for (;it!=itend;++it){
        if (pos==-2){
          set_merge(v,vecteur(1,pow(*it,xvar._SYMBptr->feuille[0],contextptr)));
          continue;
        }
        if (pos==-1){
          // solve piecewise()==*it
          set_merge(v,solve_piecewise(xvar._SYMBptr->feuille,*it,x,isolate_mode,contextptr));
          if (is_undef(v)) return;
          continue;
        }
        if ( (isolate_mode & 1)==0 && (pos==3 || pos==4) && is_strictly_greater(*it**it,1,contextptr)){
          continue;
        }
        gen res=isolate_fcns[pos-1](*it,isolate_mode,contextptr);
        if (res.type!=_VECT)
          set_merge(v,solve(xvar._SYMBptr->feuille-res,x,isolate_mode,contextptr));
        else {
          const_iterateur it=res._VECTptr->begin(),itend=res._VECTptr->end();
          for (;it!=itend;++it)
            if (xvar._SYMBptr->feuille==x)
              v.push_back(*it);
            else
              set_merge(v,solve(xvar._SYMBptr->feuille-*it,x,isolate_mode,contextptr));
        }
      }
      solve_ckrange(x,v,isolate_mode,contextptr);
      return;
    } // end xvar!=x
    // rewrite e as a univariate polynomial, first add other vars to x
    vecteur newv;
    lv=vecteur(1,vecteur(1,x));
    alg_lvar(e,lv);
    vecteur lvrat(1,x);
    lvar(e,lvrat);
    if (lvrat==lv.front())
      lv=lvrat;
    vecteur lv_(lv);
    // int lv_size=lv.size();
    gen num,den,f;
    f=e2r(e,lv,contextptr);
    fxnd(f,num,den);
    if (num.type!=_POLY || num._POLYptr->dim==0)
      return;
    vecteur w=polynome2poly1(*num._POLYptr,1);
    if (lv.front().type==_VECT){
      lv.front()=vecteur(lv.front()._VECTptr->begin()+1,lv.front()._VECTptr->end());
      if (lv.front()._VECTptr->empty())
	lv.erase(lv.begin()); // remove x from lv (CDR_VECT)	
    }
    else
      lv.erase(lv.begin()); // remove x from lv (CDR_VECT)
    int deg;
    vecteur w_translated;
    gen delta_x;
    if (translate_gcddeg(w,w_translated,delta_x,deg)){
      // composite polynomials
      gen invdeg=inv(deg,contextptr);
      identificateur compositex("tmp_x_solve_composite_");
      gen newe=symb_horner(*r2sym(w_translated,lv,contextptr)._VECTptr,compositex);
      delta_x=r2sym(delta_x,lv,contextptr);
      vecteur vtmp;
      in_solve(newe,compositex,vtmp,isolate_mode,contextptr);
      if (lop(vtmp,at_rootof).empty()){
	vecteur unitroot(1,plus_one),munitroot;
	if (complexmode){
	  for (int k=1;k<deg;++k)
	    unitroot.push_back(exp(2*k*cst_pi/deg*cst_i,contextptr));
	  for (int k=0;k<deg;++k)
	    munitroot.push_back(exp((1+2*k)*cst_pi/deg*cst_i,contextptr));
	}
	const_iterateur it=vtmp.begin(),itend=vtmp.end();
	for (;it!=itend;++it){
	  bool negatif=is_strictly_positive(-*it,contextptr);
	  gen tmp;
	  if (deg==2) 
	    tmp=sqrt((negatif?-*it:*it),contextptr);
	  else
	    tmp=pow((negatif?-*it:*it),invdeg,contextptr);
	  if (complexmode){
	    const_iterateur jt,jtend;
	    if (!negatif){
	      jt=unitroot.begin();
	      jtend=unitroot.end();
	    }
	    else {
	      jt=munitroot.begin();
	      jtend=munitroot.end();
	    }
	    for (;jt!=jtend;++jt)
	      newv.push_back(delta_x + (*jt) * tmp);
	  }
	  else {
	    if (deg%2)
	      newv.push_back(delta_x + (negatif?-tmp:tmp));
	    else {
	      if (!negatif){
		newv.push_back(delta_x + tmp);
		newv.push_back(delta_x - tmp);
	      }
	    }
	  }
	}
	solve_ckrange(x,newv,isolate_mode,contextptr);
	v=mergevecteur(v,newv);
	return;
      }
    }
    // if degree(w)=0, 1 or 2 solve it, otherwise error (should return ext)
    int d=int(w.size())-1;
    if (!d)
      return;
    if (d==1){
      gen tmp=rdiv(-r2sym(w.back(),lv,contextptr),r2sym(w.front(),lv,contextptr),contextptr);
      if (!complexmode && has_i(tmp))
	return;
      newv.push_back(tmp);
      solve_ckrange(x,newv,isolate_mode,contextptr);
      v=mergevecteur(v,newv);
      return;
    }
    if (d>2){
      if (has_num_coeff(w)){
	if (complexmode)
	  newv=proot(w,epsilon(contextptr),contextptr);
	else
	  newv=real_proot(w,epsilon(contextptr),contextptr);
	solve_ckrange(x,newv,isolate_mode,contextptr);
	v=mergevecteur(v,newv);
	return;
      }
      int n=is_cyclotomic(w,epsilon(contextptr));
      if (!n){
	if (
#ifdef GIAC_GGB
	    0 &&
#endif
	    calc_mode(contextptr)!=1 && abs_calc_mode(contextptr)!=38 && d==3 && lv_.size()==1
	    ){
	  gen W=r2sym(num,lv_,contextptr);
#if 1
	  // ALT: alpha*x^3+beta*x^2+gamma*x+delta
	  /*
	    A:=beta/alpha;
	    B:=gamma/alpha;
	    C:=delta/alpha;
	    P:=x^3+A*x^2+B*x+C;
	    // Check if discriminant is a square
	    d:=4*A^3*C-A^2*B^2-18*A*B*C+4*B^3+27*C^2;
	    // = (27*alpha^2*delta^2-18*alpha*beta*delta*gamma+4*alpha*gamma^3+4*beta^3*delta-beta^2*gamma^2)/alpha^4
	    // if discriminant is positive and not a square, the real root is > the conjugates real part if (27*alpha^2*delta-9*alpha*beta*gamma+2*beta^3)/(27*alpha^3) <0
            Q:=poly1[alpha^4,0,6*alpha^3*gamma-2*alpha^2*beta^2,0,9*alpha^2*gamma^2-6*alpha*beta^2*gamma+beta^4,0,27*alpha^2*delta^2-18*alpha*beta*delta*gamma+4*alpha*gamma^3+4*beta^3*delta-beta^2*gamma^2];
	    ro:=rootof([1,0],Q);
	    D:=27*alpha^2*delta-9*alpha*beta*gamma+2*beta^3;
	    P1:=[-3*alpha^4,0,-15*alpha^3*gamma+5*alpha^2*beta^2,0,-9*alpha^2*beta*delta-12*alpha^2*gamma^2+11*alpha*beta^2*gamma-2*beta^4];
	    R1:=rootof([P1,Q])/alpha/D;
	    P2:=[3*alpha^3,0,15*alpha^2*gamma-5*alpha*beta^2,27*alpha^2*delta-9*alpha*beta*gamma+2*beta^3,-18*alpha*beta*delta+12*alpha*gamma^2-2*beta^2*gamma];
	    R2:=rootof(P2,Q)/2/D;
	    P3:=[3*alpha^3,0,15*alpha^2*gamma-5*alpha*beta^2,-27*alpha^2*delta+9*alpha*beta*gamma-2*beta^3,-18*alpha*beta*delta+12*alpha*gamma^2-2*beta^2*gamma];
	    R3:=rootof(P3,Q)/2/D;
	    normal(subst(P,x,R1)); // ->0
	    normal(subst(P,x,R2)); // ->0
	    normal(subst(P,x,R3)); // ->0
	    normal(R1+R2+R3); // ok
	    normal(R1*R2+R2*R3+R3*R1); // ok
	    normal(R1*R2*R3); // ok
	  */
	  gen alpha=w[0],beta=w[1],gamma=w[2],delta=w[3];
	  gen alpha2=alpha*alpha,alpha3=alpha2*alpha,alpha4=alpha3*alpha,beta2=beta*beta,beta3=beta2*beta,beta4=beta3*beta,delta2=delta*delta,gamma2=gamma*gamma,gamma3=gamma2*gamma;
	  gen discriminant=(27*alpha2*delta2-18*alpha*beta*delta*gamma+4*alpha*gamma3+4*beta3*delta-beta2*gamma2);
	  gen test1=x*x-r2sym(discriminant,lv,contextptr);
	  bool b=withsqrt(contextptr);
	  withsqrt(false,contextptr);
	  bool bc=complex_mode(contextptr);
	  complex_mode(false,contextptr);
	  test1=_factors(test1,contextptr);
	  if (test1.type==_VECT && test1._VECTptr->size()==4){
	    // discriminant is a perfect square
	    gen ROOTOF=rootof(gen(makevecteur(
					  makevecteur(1,0),
					  _symb2poly(gen(makevecteur(W,x),_SEQ__VECT),contextptr)
					  ),_SEQ__VECT),contextptr);	    
	    gen F=_factor(gen(makevecteur(W,ROOTOF),_SEQ__VECT),contextptr);
	    newv=solve(F,x,isolate_mode,contextptr);
	  }
	  else {
	    vecteur Q=makevecteur(alpha4,0,6*alpha3*gamma-2*alpha2*beta2,0,9*alpha2*gamma2-6*alpha*beta2*gamma+beta4,0,27*alpha2*delta2-18*alpha*beta*delta*gamma+4*alpha*gamma3+4*beta3*delta-beta2*gamma2);
	    gen tmp=lgcd(Q);
	    divvecteur(Q,tmp,Q);
	    gen q=r2sym(Q,lv,contextptr),q0;
	    if (q.type==_VECT && !q._VECTptr->empty()){
	      q0=q._VECTptr->front();
	      if (!is_one(q0) && is_one(q0*q0*q0*q0)){
		q=q/q0;
		q0=1;
	      }
	    }
	    if (q.type==_VECT && !q._VECTptr->empty() && !is_one(q._VECTptr->front())){
	      // make change of variable so that Q becomes monic and solve again
	      gen e1=subst(e,x,x/q0,false,contextptr);
	      vecteur newv;
	      int is=isolate_mode;
	      isolate_mode |= 16;
	      in_solve(e1,x,newv,isolate_mode,contextptr);
	      isolate_mode = is;
	      multvecteur(inv(q0,contextptr),newv,newv);
	      solve_ckrange(x,newv,isolate_mode,contextptr);
	      v=mergevecteur(v,newv);
	      return;
	    }
	    gen D=r2sym(27*alpha2*delta-9*alpha*beta*gamma+2*beta3,lv,contextptr);
	    vecteur P1=makevecteur(-3*alpha4,0,-15*alpha3*gamma+5*alpha2*beta2,0,-9*alpha2*beta*delta-12*alpha2*gamma2+11*alpha*beta2*gamma-2*beta4);
	    gen R1=rootof(makevecteur(r2sym(P1,lv,contextptr),q),contextptr)/r2sym(alpha,lv,contextptr)/D;
	    vecteur P2=makevecteur(3*alpha3,0,15*alpha2*gamma-5*alpha*beta2,27*alpha2*delta-9*alpha*beta*gamma+2*beta3,-18*alpha*beta*delta+12*alpha*gamma2-2*beta2*gamma);
	    gen R2=rootof(makevecteur(r2sym(P2,lv,contextptr),q),contextptr)/2/D;
	    vecteur P3=makevecteur(3*alpha3,0,15*alpha2*gamma-5*alpha*beta2,-27*alpha2*delta+9*alpha*beta*gamma-2*beta3,-18*alpha*beta*delta+12*alpha*gamma2-2*beta2*gamma);
	    gen R3=rootof(makevecteur(r2sym(P3,lv,contextptr),q),contextptr)/2/D;
	    newv=makevecteur(R1,R2,R3);
	  }
	  // End ALT
#else
	  // w is of order 3, 
	  // find the 3 roots in term of an extension of order 6
	  // let r1,r2,r3 be the 3 roots, the extension min poly
	  // will have rj-rk j=1..3, k!=j has roots
	  // Let x=rj-rk, y=rk then w(x+y)=w(y)=0
	  // therefore resultant((w(x+y)-w(y))/x,w(y),y)=0
	  // and it has the right degree
	  // FIXME check complex_mode if there is only 1 real root!
	  gen Y(identificateur(" solve_y"));
	  gen WY=subst(W,x,Y,false,contextptr);
	  gen WXY=subst(W,x,x+Y,false,contextptr);
	  gen R=_resultant(gen(makevecteur((WXY-WY)/x,WY,Y),_SEQ__VECT),contextptr);
	  gen ROOTOF=rootof(gen(makevecteur(
					    makevecteur(1,0),
					    _symb2poly(gen(makevecteur(R,x),_SEQ__VECT),contextptr)
					    ),_SEQ__VECT),contextptr);
	  gen F=_factor(gen(makevecteur(W,ROOTOF),_SEQ__VECT),contextptr);
	  newv=solve(F,x,isolate_mode,contextptr);
#endif
	  withsqrt(b,contextptr);
	  complex_mode(bc,contextptr);
	  if ((isolate_mode & 16)==0)
	    solve_ckrange(x,newv,isolate_mode,contextptr);
	  v=mergevecteur(v,newv);
	  return ;
	}
	if (debug_infolevel) // abs_calc_mode(contextptr)!=38)
	  *logptr(contextptr) << gettext("Warning! Algebraic extension not implemented yet for poly ") << r2sym(w,lv,contextptr) << '\n';
	gen w_orig;
	w=*evalf((w_orig=r2sym(w,lv,contextptr)),1,contextptr)._VECTptr;
	if (has_num_coeff(w)){ // FIXME: test is always true...
#ifndef NO_STDEXCEPT
	  try {
#endif
	    if (complexmode)
	      newv=proot(w,epsilon(contextptr),contextptr);
	    else {
	      if (lvar(w_orig).empty() && !has_num_coeff(w_orig))
		newv=gen2vecteur(_realroot(gen(makevecteur(w_orig,epsilon(contextptr),at_evalf),_SEQ__VECT),contextptr));
	      else 
		newv=real_proot(w,epsilon(contextptr),contextptr);
	    }
	    solve_ckrange(x,newv,isolate_mode,contextptr);
	    v=mergevecteur(v,newv);
#ifndef NO_STDEXCEPT
	  }
	  catch (std::runtime_error & ){
	    last_evaled_argptr(contextptr)=NULL;
	  }
#endif
	  return;
	}
	return;
      }
      if (complexmode){
	for (int j=1;j<=n/2;++j){
	  if (gcd(j,n)==1){
	    if (n%2){
	      newv.push_back(exp(rdiv(gen(2*j)*cst_pi*cst_i,n,contextptr),contextptr));
	      newv.push_back(exp(rdiv(gen(-2*j)*cst_pi*cst_i,n,contextptr),contextptr));
	    }
	    else {
	      newv.push_back(exp(rdiv(gen(j)*cst_pi*cst_i,n/2,contextptr),contextptr));
	      newv.push_back(exp(rdiv(gen(-j)*cst_pi*cst_i,n/2,contextptr),contextptr));
	    }
	  }
	}
      }
      solve_ckrange(x,newv,isolate_mode,contextptr);
      v=mergevecteur(v,newv);
      return ;
    }
    if (w[0].type==_CPLX){
      w=multvecteur(conj(w[0],contextptr),w);
    }
    gen b_over_2=rdiv(w[1],plus_two,contextptr);
    if (b_over_2.type!=_FRAC){
      gen a=r2sym(w.front(),lv,contextptr);
      gen minus_b_over_2=r2sym(-b_over_2,lv,contextptr);
      gen delta_prime=r2sym(pow(b_over_2,2,contextptr)-w.front()*w.back(),lv,contextptr);
#if 1 // def NO_STDEXCEPT
      if (!complexmode && (lidnt(evalf(makevecteur(a,minus_b_over_2,delta_prime),1,contextptr)).empty() || lvar(delta_prime).size()==1)&& is_positive(-delta_prime,contextptr))
	return;      
#else
      if (!complexmode && is_positive(-delta_prime,contextptr))
	return;
#endif
      if (fastsign(delta_prime,contextptr)<0)
	delta_prime=cst_i*sqrt(-delta_prime,contextptr);
      else
	delta_prime=sqrt(delta_prime,contextptr);
      delta_prime=normalize_sqrt(delta_prime,contextptr,false); // no abs in sqrt
      newv.push_back(rdiv(minus_b_over_2+delta_prime,a,contextptr));
      if (!is_zero(delta_prime,contextptr))
	newv.push_back(rdiv(minus_b_over_2-delta_prime,a,contextptr));
    }
    else {
      gen two_a=r2sym(plus_two*w.front(),lv,contextptr);
      gen minus_b=r2sym(-w[1],lv,contextptr);
      gen delta=r2sym(w[1]*w[1]-gen(4)*w.front()*w.back(),lv,contextptr);
#if 1 // def NO_STDEXCEPT
      if (!complexmode && (lidnt(evalf(makevecteur(two_a,minus_b,delta),1,contextptr)).empty() || lvar(delta).size()==1) && is_positive(-delta,contextptr))
	return;      
#else
      if (!complexmode && is_positive(-delta,contextptr))
	return;
#endif
      if (complexmode && (lidnt(evalf(makevecteur(two_a,minus_b,delta),1,contextptr)).empty() || lvar(delta).size()==1) && is_positive(-delta,contextptr))
	delta=cst_i*normalize_sqrt(sqrt(-delta,contextptr),contextptr);
      else
	delta=normalize_sqrt(sqrt(delta,contextptr),contextptr);
      newv.push_back(rdiv(minus_b+delta,two_a,contextptr));
      newv.push_back(rdiv(minus_b-delta,two_a,contextptr));
    }
    solve_ckrange(x,newv,isolate_mode,contextptr);
    v=mergevecteur(v,newv);
  }

  // v assumed to represent an irreducible dense 1-d poly
  vecteur solve(const vecteur & v,bool complexmode,GIAC_CONTEXT){
    vecteur res;
    int d=int(v.size())-1;
    if (d<1)
      return res;
    if (d==1){
      res.push_back(rdiv(-v.back(),v.front(),contextptr));
      return res;
    }
    if (!is_one(v.front())){
      // if v is not monic, set Y=a*X
      gen a(v.front()),puissance(plus_one);
      vecteur w;
      w.reserve(d+1);
      for (int i=0;i<=d;++i,puissance=puissance*a)
	w.push_back(v[i]*puissance);
      return divvecteur(solve(divvecteur(w,a),complex_mode(contextptr),contextptr),a);
    }
    // should call sym2rxroot for extensions of extensions
    vecteur tmp(2,zero);
    tmp.front()=plus_one;
    if (d==2){
      gen b(v[1]),c(v[2]);
      gen bprime(rdiv(b,plus_two,contextptr));
      if (!has_denominator(bprime)){
	gen delta(bprime*bprime-c);
	if (!complexmode && is_positive(-delta,contextptr))
	  return res;
	vecteur w(3,zero);
	w.front()=plus_one;
	w.back()=-delta;
	tmp.back()=-bprime;
	res.push_back(algebraic_EXTension(tmp,w));
	tmp.front()=minus_one;
	tmp.back()=-bprime;
	res.push_back(algebraic_EXTension(tmp,w));	
      }
      else {
	if (!complexmode && is_positive(4*c-b*b,contextptr))
	  return res;
	tmp.back()=zero;
	res.push_back(algebraic_EXTension(tmp,v));
	tmp.front()=minus_one;
	tmp.back()=-b;
	res.push_back(algebraic_EXTension(tmp,v));
      }
      return res;
    }
    // should return a list of d algebraic extension with order number
    res.push_back(algebraic_EXTension(tmp,v));
    return res;
  }

  static vecteur in_solve_inequation(const gen & e0,const gen &e,const identificateur & x,int direction,const gen & rangeg,const vecteur & veq_excluded,const vecteur & veq_not_singu,const vecteur & excluded_not_singu,const vecteur & singu,GIAC_CONTEXT){
    if (rangeg.type!=_VECT)
      return vecteur(0);
    vecteur rangev = *rangeg._VECTptr,range=rangev;
    if (rangev.size()==2){
      gen &a=rangev.front();
      gen & b=rangev.back();
      // keep only values inside a,b
      range=vecteur(1,a);
      const_iterateur it=veq_excluded.begin(),itend=veq_excluded.end();
      for (;it!=itend;++it){
	if (is_strictly_greater(*it,a,contextptr))
	  break;
      }
      for (;it!=itend;++it){
	if (is_greater(*it,b,contextptr))
	  break;
	range.push_back(*it);
      }
      range.push_back(b);
    }
    else {
      range=mergevecteur(rangev,veq_excluded);
      range=protect_sort(range,contextptr);
    }
    vecteur res;
    int s=int(range.size());
    if (s<2)
      return vecteur(1,gensizeerr(contextptr));
    if (s==2 && range[0]==minus_inf && range[1]==plus_inf){
      gen test=sign(subst(e,x,0,false,contextptr),contextptr);
      if (direction<0)
	test=-test;
      if (is_one(test))
	return vecteur(1,x);
      if (is_one(-test))
	return vecteur(0);
      return vecteur(1,gensizeerr(gettext("Unable to check sign ")+test.print()));
    }
    vecteur add_eq,already_added; gen prevm=minus_inf;
    for (int i=0;i<s-1;++i){
      gen l=range[i],m=range[i+1];
      if (l==m)
	continue;
      gen testval;
      if (l==minus_inf)
	testval=m-1;
      else {
	if (m==plus_inf)
	  testval=l+1;
	else
	  testval=(l+m)/2;
      }
      if (has_op(testval,*at_rootof)){
	testval=evalf_double(testval,1,contextptr);
	if (testval.type==_CPLX && is_zero(*(testval._CPLXptr+1),contextptr))
	  testval=*testval._CPLXptr;
      }
      gen test=eval(subst(e0,x,testval,false,contextptr),eval_level(contextptr),contextptr);
      // additional numeric check
      if (e0.type==_SYMB && e0._SYMBptr->feuille.type==_VECT && e0._SYMBptr->feuille._VECTptr->size()==2){
	gen a=e0._SYMBptr->feuille[0];
	gen b=e0._SYMBptr->feuille[1];
	a=a-b;
	gen testnum=subst(a,x,evalf(testval,1,contextptr),false,contextptr);
	if (testnum.type==_CPLX){
	  if (l!=prevm && !is_inf(l) && !equalposcomp(singu,l)){
	    // maybe add l
	    gen test=eval(subst(e0,x,l,false,contextptr),eval_level(contextptr),contextptr);
	    if (test==1)
	      res.push_back(l);
	  }
	  continue;
	}
      }
      if (is_undef(test)){
	if (e0.type==_SYMB && e0._SYMBptr->feuille.type==_VECT && e0._SYMBptr->feuille._VECTptr->size()==2){
	  gen a=e0._SYMBptr->feuille[0];
	  a=limit(a,x,testval,0,contextptr);
	  gen b=e0._SYMBptr->feuille[1];
	  b=limit(b,x,testval,0,contextptr);
	  test=e0._SYMBptr->sommet(gen(makevecteur(a,b),_SEQ__VECT),contextptr);
	  if (is_undef(test))
	    a=limit(e0._SYMBptr->feuille[0]-e0._SYMBptr->feuille[1],x,testval,0,contextptr);
	  test=e0._SYMBptr->sommet(gen(makevecteur(a,0),_SEQ__VECT),contextptr);
	}
	if (is_undef(test))
	  return vecteur(1,gensizeerr(gettext("Unable to check test at x=")+test.print()));
      }
      if (test!=1){
	if (!equalposcomp(already_added,l) && equalposcomp(veq_not_singu,l)){
	  gen a=e0._SYMBptr->feuille[0];
	  a=subst(a,x,l,false,contextptr);
	  if (!has_i(a))
	    add_eq.push_back(l);
	}
	if (l!=prevm && !is_inf(l) && !equalposcomp(singu,l)){
	  // maybe add l
	  gen test=eval(subst(e0,x,l,false,contextptr),eval_level(contextptr),contextptr);
	  if (test==1)
	    res.push_back(l);
	}
	continue;
      }
      already_added.push_back(m);
      gen symb_sup,symb_inf;
      if (equalposcomp(singu,l) && e0.type==_SYMB && e0._SYMBptr->feuille.type==_VECT && e0._SYMBptr->feuille._VECTptr->size()==2){
	gen a=e0._SYMBptr->feuille[0];
	gen b=e0._SYMBptr->feuille[1];
	a=limit(a-b,x,l,1,contextptr);
	if (is_inf(a) || is_undef(a))
	  test=0;
	else
	  test=e0._SYMBptr->sommet(gen(makevecteur(a,0),_SEQ__VECT),contextptr);
      }
      else
	test=eval(subst(e0,x,l,false,contextptr),eval_level(contextptr),contextptr);
      gen testeq=abs(evalf(subst(e,x,l,false,contextptr),eval_level(contextptr),contextptr),contextptr);
      double eps=epsilon(contextptr); gen lf=_epsilon2zero(evalf_double(l,1,contextptr),contextptr);
      gen lsymb=l; 
      if (has_op(lsymb,*at_rootof) && abs_calc_mode(contextptr)==38) 
	lsymb=lf;
      if (lf.type==_DOUBLE_){
	double lfd=fabs(lf._DOUBLE_val);
	if (lfd>1)
	  eps=lfd*eps;
      }
      // 22 feb 2018: revert change ... || test!=1 to && test!=1 made for solve(abs(x^2)=abs(x)^2)
      if ((is_greater(eps,testeq,contextptr) || test!=1) &&
	  (equalposcomp(excluded_not_singu,l) || equalposcomp(singu,l) ||
	   ( !(direction %2) && equalposcomp(veq_not_singu,l))) 
	  )
	symb_inf=symb_superieur_strict(x,lsymb);
      else {
	if (is_undef(testeq) || equalposcomp(excluded_not_singu,l) || (test!=1 && equalposcomp(singu,l)))
	  symb_inf=symb_superieur_strict(x,lsymb);
	else
	  symb_inf=symb_superieur_egal(x,lsymb);
      }
      if (equalposcomp(singu,m) && e0.type==_SYMB && e0._SYMBptr->feuille.type==_VECT && e0._SYMBptr->feuille._VECTptr->size()==2){
	gen a=e0._SYMBptr->feuille[0];
	gen b=e0._SYMBptr->feuille[1];
	a=limit(a-b,x,m,-1,contextptr);
	if (is_inf(a) || is_undef(a))
	  test=0;
	else
	  test=e0._SYMBptr->sommet(gen(makevecteur(a,0),_SEQ__VECT),contextptr);
      }
      else
	test=eval(subst(e0,x,m,false,contextptr),eval_level(contextptr),contextptr);
      lf=_epsilon2zero(evalf_double(m,1,contextptr),contextptr);
      gen msymb=m;
      if (has_op(m,*at_rootof) && abs_calc_mode(contextptr)==38)
	msymb=lf;
      testeq=abs(evalf(subst(e,x,m,false,contextptr),eval_level(contextptr),contextptr),contextptr);
      eps=epsilon(contextptr);
      // if ( (lf.type!=_DOUBLE_ && lf.type!=_CPLX) || (testeq.type!=_DOUBLE_ || testeq.type != _CPLX && !is_undef(testeq) && !is_inf(testeq)) ) return vecteur(1,gensizeerr("Unable to solve inequation"));
      if (lf.type==_DOUBLE_){
	double lfd=fabs(lf._DOUBLE_val);
	if (lfd>1)
	  eps=lfd*eps;
      }
      if ( (is_greater(eps,testeq,contextptr) || test!=1) &&
	  (equalposcomp(excluded_not_singu,m) || equalposcomp(singu,m) ||
	   ( !(direction %2) && equalposcomp(veq_not_singu,m)) )
	  )
	symb_sup=symb_inferieur_strict(x,msymb);
      else {
	if (is_undef(testeq) || equalposcomp(excluded_not_singu,m) || (test!=1 && equalposcomp(singu,m)))
	  symb_sup=symb_inferieur_strict(x,msymb);
	else {
	  symb_sup=symb_inferieur_egal(x,msymb);
	  prevm=m;
	}
      }
      if (l==minus_inf)
	res.push_back(symb_sup);
      else {
	if (m==plus_inf)
	  res.push_back(symb_inf);
	else
	  res.push_back(symbolic(at_and,makesequence(symb_inf,symb_sup))); 
      }
    }
    if (direction % 2)
      res=mergevecteur(add_eq,res);
    return res;
  }

  static bool ck_sorted(const vecteur & v,GIAC_CONTEXT){
    int vs=int(v.size());
    for (int i=1;i<vs;++i){
      if (!ck_is_greater(v[i],v[i-1],contextptr))
	return false;
    }
    return true;
  }

  // works for continuous functions only
  static vecteur solve_inequation(const gen & e0,const identificateur & x,int direction,GIAC_CONTEXT){
    gen e=e0;
    if (has_num_coeff(e0)){
      *logptr(contextptr) << gettext("Unable to solve inequations with approx coeffs ") << '\n';
      e=exact(e0,contextptr);
    }
    vecteur xrange;
    if (find_range(x,xrange,contextptr)){
      gen T;
      if (xrange==vecteur(1,gen(makevecteur(minus_inf,plus_inf),_LINE__VECT)) && is_periodic(e,x,T,contextptr)){
        gen hyp=symb_and(symb_superieur_egal(x,0),symb_inferieur_strict(x,T));
        *logptr(contextptr) << gettext("Inequation on periodic expression without assumptions on variable, adding assumption ") << hyp << "\n";
        giac_assume(hyp,contextptr);
      }
    }
    gen a1=e._SYMBptr->feuille[0];
    gen a2=e._SYMBptr->feuille[1];
    vecteur w=lop(lvarx(makevecteur(a1,a2),x),at_pow);
    if (a2.type!=_VECT && a2!=0 && w.size()>1)
      e=lncollect(lnexpand(ln(simplify(a1,contextptr),contextptr)-ln(simplify(a2,contextptr),contextptr),contextptr),contextptr);
    else
      e=a1-a2;
    if (is_inequation(e))
      return vecteur(1,gensizeerr(gettext("Inequation inside inequation not implemented ")+e.print()));
    if (is_zero(ratnormal(derive(e,x,contextptr),contextptr),contextptr))
      *logptr(contextptr) <<gettext("Inequation is constant with respect to ")+string(x.print(contextptr)) << '\n';
    vecteur veq_not_singu,veq,singu;
    int cm=calc_mode(contextptr); 
    calc_mode(0,contextptr); // for solve(1/(log(abs(x))-x) > 0)
    singu=find_singularities(e,x,0x12,contextptr);
    veq_not_singu=solve(e,x,2,contextptr);
    calc_mode(cm,contextptr);
    for (unsigned i=0;i<singu.size();++i){	
      singu[i]=ratnormal(singu[i],contextptr);
      if (equalposcomp(veq_not_singu,singu[i])){
	gen tmp=subst(e0,x,singu[i],false,contextptr);
	if (eval(tmp,1,contextptr)==1)
	  singu.erase(singu.begin()+i);
      }
    }
    for (unsigned i=0;i<veq_not_singu.size();++i)
      veq_not_singu[i]=ratnormal(veq_not_singu[i],contextptr);
    // Check if trig equations have introduced infinitely many solutions depending on add. param.
    vecteur eid=lidnt(e),eids=eid;
    lidnt(evalf(veq_not_singu,1,contextptr),eids,false);
    gen singuf=evalf(singu,1,contextptr), veq_not_singuf=evalf(veq_not_singu,1,contextptr);
    if (singuf.type!=_VECT || veq_not_singuf.type!=_VECT || !is_numericv(*singuf._VECTptr) || !is_numericv(*veq_not_singuf._VECTptr)){
      if (eids.size()>eid.size())
	return vecteur(1,gensizeerr(gettext("Unable to find numeric values solving equation. For trigonometric equations this may be solved using assumptions, e.g. assume(x>-pi && x<pi)")));
      *logptr(contextptr) << gettext("Warning! Solving parametric inequation requires assumption on parameters otherwise solutions may be missed. The solutions of the equation are ") << veq_not_singu << '\n';
    }
    veq=mergevecteur(veq_not_singu,singu);
    vecteur range,excluded_not_singu(find_excluded(x,contextptr));
    vecteur excluded=mergevecteur(excluded_not_singu,singu);
    vecteur veq_excluded=mergevecteur(excluded,veq);
    veq_excluded=protect_sort(veq_excluded,contextptr);
    if (!ck_sorted(veq_excluded,contextptr))
      return vecteur(1,gensizeerr(gettext("Unable to sort ")+gen(veq_excluded).print(contextptr)));
    // From the lower bound of range to the higher bound
    // find the sign 
    vecteur res;
    if (!find_range(x,range,contextptr))
      return res;
    for (unsigned i=0;i<range.size();i++){
      res=mergevecteur(res,in_solve_inequation(e0,e,x,direction,range[i],veq_excluded,veq_not_singu,excluded_not_singu,singu,contextptr));
    }
    return res;
  }

  // modular roots, modulo p, p supposed to be prime
  // dogcd should be set to true except if you have already done gcd with x^p-x
  bool modpolyroot(const modpoly & a,const gen & p,vecteur & v,bool dogcd,GIAC_CONTEXT){
    environment env;
    env.moduloon=true;
    env.modulo=p;
    modpoly A,B(2,1),D,C;
    if (dogcd){
      C=modpoly(p.val+1);
      C[0]=1;
      C[p.val-1]=-1;
      gcdmodpoly(a,C,&env,A);
    }
    else
      A=a;
    if (A.size()==1)
      return true;
    if (A.size()==2){
      v.push_back(smod(invmod(-A.front(),p)*A.back(),p));
      return true;
    }
    // try to split a in 2 parts using gcd with (x+random)^((p-1)/2) mod p and a -1
    for (;;){
      gen r=smod(gen(giac_rand(contextptr)),p);
      B[1]=r;
      D=powmod(B,(p-1)/2,A,&env);
      D.back()=D.back()-1;
      if (is_zero(D.front(),contextptr))
	continue;
      gcdmodpoly(A,D,&env,C);
      if (C.size()>1 && C.size()<A.size()){
	return modpolyroot(C,p,v,false,contextptr) && modpolyroot(A/C,p,v,false,contextptr);
      }
    }
  }

  static bool modsolve(const gen & e,const identificateur & x,const gen & modulo,vecteur &v,GIAC_CONTEXT){
    vecteur l=lvar(e);
    if (modulo.type!=_INT_ || modulo.val> (1<<30))
      return false; // setdimerr(gettext("Modular equation with modulo too large"));
    if (l.size()==1 && l.front()==x && is_probab_prime_p(modulo)){
      // Convert e to polynomial wrt x 
      gen tmp=_symb2poly(gen(makevecteur(e,l.front()),_SEQ__VECT),contextptr);
      if (tmp.type==_FRAC)
	tmp=tmp._FRACptr->num;
      tmp=unmod(tmp);
      if (tmp.type!=_VECT)
	return false;
      vecteur w=*tmp._VECTptr;
      return modpolyroot(w,modulo,v,true,contextptr);
    }
    int m=modulo.val;
    for (int i=0;i<m;++i){
      gen tmp=subst(e,x,i,false,contextptr);
      if (is_zero(tmp.eval(eval_level(contextptr),contextptr),contextptr))
	v.push_back(i);
    }
    return true;
  }
  
  bool remove_neg(gen & g){
    if (g.type!=_SYMB || g._SYMBptr->sommet!=at_neg)
      return false;
    g=g._SYMBptr->feuille;
    return true;
  }

  bool is_rewritable_as_trigprod(gen & expr,GIAC_CONTEXT){
    if (expr.is_symb_of_sommet(at_plus) && expr._SYMBptr->feuille.type==_VECT && expr._SYMBptr->feuille._VECTptr->size()==2){
      const vecteur & v = *expr._SYMBptr->feuille._VECTptr;
      gen a=v[0],b=v[1];
      bool nega=remove_neg(a);
      bool negb=remove_neg(b);
      if (nega)
	negb=!negb;
      bool cosa=a.type==_SYMB && a._SYMBptr->sommet==at_cos;
      bool sina=a.type==_SYMB && a._SYMBptr->sommet==at_sin;
      bool cosb=b.type==_SYMB && b._SYMBptr->sommet==at_cos;
      bool sinb=b.type==_SYMB && b._SYMBptr->sommet==at_sin;
      if ( (cosa || sina) && (cosb || sinb)){
	a=a._SYMBptr->feuille;
	if (negb)
	  b=b._SYMBptr->feuille+cst_pi;
	else
	  b=b._SYMBptr->feuille;
	if (sina)
	  a=a-cst_pi/2;
	if (sinb)
	  b=b-cst_pi/2;
	// cos(a)+cos(b)=0 equivalent to cos((a+b)/2)*cos((a-b)/2)=0
	expr=cos((a+b)/2,contextptr)*cos((a-b)/2,contextptr);
	return true;
      }
    }
    return false;
  }

  static void clean(gen & e,const identificateur & x,GIAC_CONTEXT){
    if (e.type!=_SYMB)
      return;
#if 1
    gen z=fast_icontent(e);
    e=fast_divide_by_icontent(e,z);
    if (e.type!=_SYMB)
      return;
#endif
    if ( complex_mode(contextptr)==0 && lvarx(e,x).size()>1 ){
      gen es=e;
      if (has_op(es,*at_ln)){
	es=lncollect(es,contextptr);
	if (lvarx(es,x).size()==1){
	  e=es;
	  return;
	}
      }
      es=rationalize(e,x,contextptr);
      if (lvarx(es,x).size()==1){
	e=es;
	return;
      }
      es=simplify(e,contextptr);
      if (lvarx(es,x).size()==1){
	e=es;
	return;
      }
      e=factor(e,x,false,contextptr);
    }
    if (e.type!=_SYMB)
      return;
    if (e._SYMBptr->sommet==at_inv || (e._SYMBptr->sommet==at_pow && is_positive(-e._SYMBptr->feuille._VECTptr->back(),contextptr))){
      gen ef=e._SYMBptr->feuille;
      if (e._SYMBptr->sommet==at_pow)
	ef=ef._VECTptr->front();
      // search for a tan in the variables
      vecteur lv(lvarx(e,x));
      if (lv.size()!=1)
	e=1;
      else {
	gen xvar(lv.front());
	if (!xvar.is_symb_of_sommet(at_tan))
	  e=1;
      }
      return;
    }
    if (e._SYMBptr->sommet==at_prod){
      gen ef=e._SYMBptr->feuille;
      if (ef.type!=_VECT)
	return;
      vecteur v=*ef._VECTptr;
      int vs=int(v.size());
#if 0
      // find num and den, check if gcd is 1, otherwise simplify
      gen num(1),den(1);
      for (int i=0;i<vs;++i){
	if (v[i].is_symb_of_sommet(at_inv))
	  den = den*v[i]._SYMBptr->feuille;
	else {
	  if (v[i].is_symb_of_sommet(at_pow) && v[i]._SYMBptr->feuille[1].type==_INT_ && v[i]._SYMBptr->feuille[1].val<0)
	    den=den*inv(v[i],contextptr);
	  else
	    num= num*v[i];
	}
      }
      gen g=gcd(num,den,contextptr);
      if (!is_constant_wrt(g,x,contextptr)){
	v=makevecteur(num,inv(den,contextptr));
	vs=2;
      }
#endif
      for (int i=0;i<vs;++i)
	clean(v[i],x,contextptr);
      ef=gen(v,ef.subtype);
      e=symbolic(at_prod,ef);
    }
  }
  
  // detect product and powers
  void solve(const gen & e,const identificateur & x,vecteur &v,int isolate_mode,GIAC_CONTEXT){
    if (is_zero(e,contextptr)){
      v.push_back(x);
      return;
    }
    switch (e.type){
    case _IDNT:
      if (*e._IDNTptr==x){ 
	vecteur newv(1,0);
	solve_ckrange(x,newv,isolate_mode,contextptr);
	if (!newv.empty())
	  addtolvar(zero,v);
      }
      return;
    case _SYMB:
      if ( e._SYMBptr->sommet==at_pow && ck_is_strictly_positive(e._SYMBptr->feuille._VECTptr->back(),contextptr) ){
	vecteur tmpv;
	solve(e._SYMBptr->feuille._VECTptr->front(),x,tmpv,isolate_mode,contextptr);
	int ncopy=1;
	// make copies of the answer (xcas_mode(contextptr)==1 compatibility)
	if (xcas_mode(contextptr)==1 && e._SYMBptr->feuille._VECTptr->back().type==_INT_)
	  ncopy=e._SYMBptr->feuille._VECTptr->back().val;
	const_iterateur it=tmpv.begin(),itend=tmpv.end();
	for (;it!=itend;++it){
	  for (int i=0;i<ncopy;++i)
	    v.push_back(*it);
	}
	return;
      }
      if (e._SYMBptr->sommet==at_prod){
	const_iterateur it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
	for (;it!=itend;++it){
	  solve(*it,x,v,isolate_mode,contextptr);
	  if (is_undef(v)) break;
	}
	return;
      }
      if (e._SYMBptr->sommet==at_neg){
	solve(e._SYMBptr->feuille,x,v,isolate_mode,contextptr);
	return;
      }
      if (//!(isolate_mode & 2) && // commented for assume(x>0 et x<2*pi);simplifier(solve(cos(x)+sin(x)=-1)) 
	  (e._SYMBptr->sommet==at_inv || (e._SYMBptr->sommet==at_pow && ck_is_positive(-e._SYMBptr->feuille._VECTptr->back(),contextptr)))
	  ){
	gen ef=e._SYMBptr->feuille;
	if (e._SYMBptr->sommet==at_pow)
	  ef=ef._VECTptr->front();
	// search for a tan in the variables
	vecteur lv(lvarx(e,x));
	if (lv.size()!=1)
	  return;
	gen xvar(lv.front());
	if (!xvar.is_symb_of_sommet(at_tan))
	  return;
	gen arg=xvar._SYMBptr->feuille;
	// solve arg=pi/2[pi]
	in_solve(arg-isolate_tan(plus_inf,isolate_mode,contextptr),x,v,isolate_mode,contextptr);
	return;
      }
      in_solve(e,x,v,isolate_mode,contextptr);
      break;
    default:
      return;
    }
  }

  // find the arguments of fractional power inside expression e
  vecteur lvarfracpow(const gen & e){
    vecteur l0=lvar(e),l;
    iterateur it=l0.begin(),itend=l0.end();
    for (;it!=itend;++it){
      if (it->_SYMBptr->sommet==at_surd){
	vecteur & arg=*it->_SYMBptr->feuille._VECTptr;
	if (arg.size()==2 && arg.back().type==_INT_ && absint(arg.back().val)<=MAX_ALG_EXT_ORDER_SIZE){
	  l.push_back(arg[0]);
	  l.push_back(arg[1]);
	  l.push_back(*it);
	}
	continue;
      }
      if (it->_SYMBptr->sommet==at_NTHROOT){
	vecteur & arg=*it->_SYMBptr->feuille._VECTptr;
	if (arg.size()==2 && arg.front().type==_INT_ && absint(arg.front().val)<=MAX_ALG_EXT_ORDER_SIZE){
	  l.push_back(arg[1]);
	  l.push_back(arg[0]);
	  l.push_back(*it);
	}
	continue;
      }
      if (it->_SYMBptr->sommet!=at_pow)
	continue;
      vecteur & arg=*it->_SYMBptr->feuille._VECTptr;
      gen g=arg[1],expnum,expden;
      if (g.type==_FRAC){
	expnum=g._FRACptr->num;
	expden=g._FRACptr->den;
      }
      else {
	if ( (g.type!=_SYMB) || (g._SYMBptr->sommet!=at_prod) )
	  continue;
	gen & arg1=g._SYMBptr->feuille;
	if (arg1.type!=_VECT)
	  continue;
	vecteur & v=*arg1._VECTptr;
	if ( (v.size()!=2) || (v[1].type!=_SYMB) || (v[1]._SYMBptr->sommet==at_inv) )
	  continue;
	expnum=v[0];
	expden=v[1]._SYMBptr->feuille;
      }
      if (expden.type!=_INT_ || absint(expden.val)>MAX_ALG_EXT_ORDER_SIZE)
	continue;
      l.push_back(arg[0]);
      l.push_back(expden.val);
      l.push_back(*it);
      vecteur v=lvarfracpow(arg[0]);
      if (!v.empty())
	l=mergevecteur(v,l);
    }
    return l;
  }

  static vecteur lvarfracpow(const gen & g,const identificateur & x,GIAC_CONTEXT){
    vecteur l0=lvarfracpow(g),l;
    const_iterateur it=l0.begin(),itend=l0.end();
    for (;it!=itend;++it){
      if (!is_zero(derive(*it,x,contextptr),contextptr)){
	l.push_back(*it);
	++it;
	l.push_back(*it);
	++it;
	l.push_back(*it);
      }
      else
	it+=2;
    }
    return l;
  }

  static void solve_fracpow(const gen & e,const identificateur & x,const vecteur & eq,const vecteur & listvars,vecteur & fullres,int isolate_mode,GIAC_CONTEXT){
    vecteur equations(eq);
    if (e.type==_IDNT){
      if (*e._IDNTptr==x && !equalposcomp(find_excluded(x,contextptr),zero)){
	addtolvar(zero,fullres);
	return;
      }
    }
    if (e.type==_SYMB){
      if ( (e._SYMBptr->sommet==at_pow) && (ck_is_positive(e._SYMBptr->feuille._VECTptr->back(),contextptr)) ){
	solve_fracpow(e._SYMBptr->feuille._VECTptr->front(),x,equations,listvars,fullres,isolate_mode,contextptr);
	return;
      }
      if (e._SYMBptr->sommet==at_prod){
	const_iterateur it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
	for (;it!=itend;++it)
	  solve_fracpow(*it,x,equations,listvars,fullres,isolate_mode,contextptr);
	return;
      }
      if (e._SYMBptr->sommet==at_neg){
	solve_fracpow(e._SYMBptr->feuille,x,equations,listvars,fullres,isolate_mode,contextptr);
	return;
      }
    } // end if (e.type==_SYMB)
    vecteur tmp1=listvars;
    tmp1.push_back(x);
    tmp1.push_back(cst_pi);
    vecteur tmp2=tmp1;
    lvar(e,tmp1);
    if (tmp1==tmp2){
      // new code with resultant in all var except the first one
      // disadvantage: does not check that listvars[i] are admissible
      // example assume(M<0); solve(sqrt(x)=M);
      // hence can be used only if no parameter present
      gen expr(e);
      int s=int(listvars.size());
      for (int i=s-1;i>=1;--i){
	// expr must be rationnal wrt listvars[i]
	vecteur vtmp(1,listvars[i]);
	if (listvars[i].type!=_IDNT)
	  setsizeerr();
	rlvarx(expr,listvars[i],vtmp);
	// IMPROVE: maybe a function applied to expr is rationnal
	if (vtmp.size()!=1)
	  setsizeerr(gettext("Solve with fractional power:")+expr.print(contextptr)+gettext(" is not rational w.r.t. ")+listvars[i].print(contextptr));
	if (!is_zero(derive(expr,listvars[i],contextptr),contextptr)){
	  expr=_resultant(makevecteur(expr,equations[i-1],listvars[i]),contextptr);
	  if (is_zero(expr))
	    expr=_gcd(makesequence(expr,equations[i-1]),contextptr);
	}
      }
      expr=factor(expr,false,contextptr);
      if (is_zero(derive(expr,x,contextptr),contextptr))
	return;
      solve(pow2expln(expr,contextptr),x,fullres,isolate_mode,contextptr);
      return;
    }
    // old code with Groebner basis
    equations.push_back(e);      
    int evalf_after=approx_mode(contextptr)?1:0;
    vecteur res=gsolve(equations,listvars,complex_mode(contextptr),evalf_after,contextptr);
#ifndef NO_STDEXCEPT
    if (!res.empty() && res.front().type==_STRNG)
      setsizeerr(*res.front()._STRNGptr);
#endif
    iterateur it=res.begin(),itend=res.end();
    for (;it!=itend;++it)
      *it=(*it)[0];
    purgenoassume(vecteur(listvars.begin()+1,listvars.end()),contextptr);
    if (listvars[0].type==_IDNT){
      fullres=mergevecteur(res,fullres);
      return;
    }
    // recursive call to solve composevar=*it with respect to x
    for (it=res.begin();it!=itend;++it){
      fullres=mergevecteur(fullres,solve(*it-listvars[0],x,isolate_mode,contextptr));
    }
  }

  gen rationalize(const gen & g,const gen & x,GIAC_CONTEXT){
    gen expr=g;
    vecteur lv(lvarx(expr,x));
    int s=int(lv.size());
    if (s==1)
      return expr;
    expr=hyp2exp(expr,contextptr); // was halftan_hyp2exp, changed for solve(sin(2x)=sin(x))
    lv=lvarx(expr,x);
    s=int(lv.size());
    if (s==1)
      return expr;
    // solve(sin(3x)=cos(x))
    if (is_rewritable_as_trigprod(expr,contextptr))
      return expr;
    gen tmp;
    if (lv.size()==2 && lv[0].type==_SYMB && lv[1].type==_SYMB && lv[0]._SYMBptr->feuille==lv[1]._SYMBptr->feuille)
      tmp=expr;
    else
      tmp=_texpand(expr,contextptr);
    vecteur tmplv=lvarx(tmp,x);
    if (tmplv.size()==2 && tmplv[0].type==_SYMB && tmplv[1].type==_SYMB && tmplv[0]._SYMBptr->feuille==tmplv[1]._SYMBptr->feuille){
      gen a,b,c,d;
      if (is_linear_wrt(tmp,tmplv[0],a,b,contextptr) && is_zero(derive(a,x,contextptr)) && is_linear_wrt(b,tmplv[1],c,d,contextptr) && is_zero(derive(c,x,contextptr)) && is_zero(derive(d,x,contextptr))){
	// tmp=a*tmplv[0]+c*tmplv[1]+d
	if (tmplv[0]._SYMBptr->sommet==at_sin && tmplv[1]._SYMBptr->sommet==at_cos){
	  // a*sin(x)+c*cos(x)=C*sin(x+phi) where exp(i*phi)=a+i*c;
	  gen phi=is_zero(d)?atan(halftan(c/a,contextptr),contextptr):arg(halftan(a+cst_i*c,contextptr),contextptr);
	  tmp=sqrt(a*a+c*c,contextptr)*sin(tmplv[0]._SYMBptr->feuille+phi,contextptr)+d;
	}
	if (tmplv[0]._SYMBptr->sommet==at_cos && tmplv[1]._SYMBptr->sommet==at_sin){
	  // a*cos(x)+c*sin(x)=C*sin(x+phi) where exp(i*phi)=c+i*a;
	  gen phi=is_zero(d)?atan(halftan(a/c,contextptr),contextptr):arg(halftan(c+cst_i*a,contextptr),contextptr);
	  tmp=sqrt(a*a+c*c,contextptr)*sin(tmplv[0]._SYMBptr->feuille+phi,contextptr)+d;
	}
      }
      else {
	if (is_linear_wrt(tmp,tmplv[1],a,b,contextptr) && is_zero(derive(a,x,contextptr)) && is_linear_wrt(b,tmplv[0],c,d,contextptr) && is_zero(derive(c,x,contextptr)) && is_zero(derive(d,x,contextptr))){
	  // tmp=a*tmplv[0]+c*tmplv[1]+d
	  if (tmplv[1]._SYMBptr->sommet==at_sin && tmplv[0]._SYMBptr->sommet==at_cos){
	    // a*sin(x)+c*cos(x)=C*sin(x+phi) where exp(i*phi)=a+i*c;
	    gen phi=is_zero(d)?atan(halftan(c/a,contextptr),contextptr):arg(halftan(a+cst_i*c,contextptr),contextptr);
	    tmp=sqrt(a*a+c*c,contextptr)*sin(tmplv[1]._SYMBptr->feuille+phi,contextptr)+d;
	  }
	  if (tmplv[1]._SYMBptr->sommet==at_cos && tmplv[0]._SYMBptr->sommet==at_sin){
	    // a*cos(x)+c*sin(x)=C*sin(x+phi) where exp(i*phi)=c+i*a;
	    gen phi=is_zero(d)?atan(halftan(a/c,contextptr),contextptr):arg(halftan(c+cst_i*a,contextptr),contextptr);
	    tmp=sqrt(a*a+c*c,contextptr)*sin(tmplv[1]._SYMBptr->feuille+phi,contextptr)+d;
	  }
	}
      }
    }
    if (lvarx(tmp,x).size()==1)
      expr=tmp;
    else {
      gen tmp1=_trigtan(tmp,contextptr);
      if (lvarx(tmp1,x).size()==1)
	tmp=tmp1;
      else {
	tmp1=_trigcos(tmp,contextptr);
	if (lvarx(tmp1,x).size()==1)
	  tmp=tmp1;
	else
	  tmp=halftan(tmp,contextptr); // on Casio, problematic for tabvar([sin(2t),cos(3t)]), fixed by adding trigcos test
      }
      // change made on 6 dec 2014 for solve(-e^x*(-cos(x)+sin(x)),x);
      int tmps=int(lvarx(tmp,x).size());
      if (tmps==1)
	expr=tmp;
      else {
	tmp=_lncollect((tmps<s?tmp:expr),contextptr);
	int s1=int(lvarx(tmp,x).size());
	if (s1<s){
	  // Note: we are checking solutions numerically later
	  *logptr(contextptr) << gettext("Warning: solving in ") << x << gettext(" equation ") << tmp << "=0" << '\n';
	  expr=tmp;
	  s=s1;
	}
	// code added 11 october 2015 for solve(2^(3*x-1)+2^(6*x-2)-2^(3*x+3)-(4^(3*x-2))=0);
	tmp=_tsimplify(tmps<s?tmp:expr,contextptr);
	if (int(lvarx(tmp,x).size())<s){
	  expr=tmp;
	}
      }
    }
    return expr;
  }

  static vecteur solve_numeric_check(const gen & e,const gen & x,const vecteur & sol,bool real,GIAC_CONTEXT){
    if (is_undef(sol))
      return sol;
    vecteur res;
    vecteur eid=lidnt(e);
    if (eid.size()==1 && lvar(e)==eid)
      return sol; // it was a univariate polynomial equation, no need to check
    for (unsigned i=0;i<sol.size();++i){
      gen tmp=subst(e,x,sol[i],false,contextptr);
      if (real && has_i(tmp))
	continue;
#ifdef HAVE_LIBMPFR
      if (has_op(sol[i],*at_LambertW)) // workaround for solve([(a)*(k)=1, (a)*(exp((3)*(k)))=(3)*(e)],[a, k]);, eval with 100 digits in moyal.cc takes too much time
	tmp=_evalf(makesequence(tmp,30),contextptr);
      else
	tmp=_evalf(makesequence(tmp,100),contextptr);
#endif
      tmp=evalf_double(tmp,1,contextptr);
      if (real && tmp.type==_CPLX)
	continue;
      vecteur tmpid=lidnt(tmp); // find identifiers introduced by all_trig_sols=true
      for (unsigned j=0;j<tmpid.size();++j){
	if (!equalposcomp(eid,tmpid[j]))
	  tmp=subst(tmp,tmpid[j],0,false,contextptr);
      }
      tmp=evalf_double(tmp,1,contextptr);
      // the following test accepts undef, otherwise we might miss some solutions
      if ((tmp.type>_CPLX && tmp.type!=_FLOAT_) || is_greater(1e-6,abs(tmp,contextptr),contextptr))
	res.push_back(sol[i]);
    }
    return res;
  }

  // v are solutions in varx, solve for x
  static vecteur solve_subst(const vecteur & v,const identificateur & x,const gen & varx,int isolate_mode,GIAC_CONTEXT){
    if (x==varx)
      return v;
    vecteur res;
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      in_solve(varx-*it,x,res,isolate_mode,contextptr);
    }
    return res;
  }

  static vecteur solve_cleaned(const gen & e,const gen & e_check,const identificateur & x,int isolate_mode,GIAC_CONTEXT){
    if (e.is_symb_of_sommet(at_exp))
      return vecteur(0);
    gen expr(e),a,b;
    if (is_linear_wrt(e,x,a,b,contextptr)){
      if (contains(a,x))  
	a=ratnormal(a,contextptr);
      if (is_exactly_zero(ratnormal(a,contextptr))){
	if (is_exactly_zero(ratnormal(b,contextptr)))
	  return vecteur(1,x);
	return vecteur(0);
      }
      a=-rdiv(b,a,contextptr);
      if (rlvarx(a,x).empty()){
	vecteur res(1,a);
	solve_ckrange(x,res,isolate_mode,contextptr);
	return res;
      }
    }
    if (expr.is_symb_of_sommet(at_neg))
      expr=expr._SYMBptr->feuille;
    if (expr.is_symb_of_sommet(at_inv)){
      // if expr has tan inside, subst tan by 1/x then x->0 if it's 0 we have additional solutions if arg of tan=pi/2
      vecteur lv(lvarx(expr,x));
      if (lv.size()==1 && lv[0].is_symb_of_sommet(at_tan)){
	gen chk=subst(expr,lv[0],inv(x,contextptr),false,contextptr);
	chk=limit(chk,x,0,0,contextptr);
	if (chk==0){
	  chk=cos(lv[0]._SYMBptr->feuille,contextptr);
	  vecteur v1(solve_cleaned(chk,e_check,x,isolate_mode,contextptr));
	  return v1;
	}
      }
      return vecteur(0);
    }
    if (expr.is_symb_of_sommet(at_prod)){
      vecteur v=gen2vecteur(expr._SYMBptr->feuille),res;
      for (unsigned i=0;i<v.size();++i){
	res=mergevecteur(res,solve_cleaned(v[i],e_check,x,isolate_mode,contextptr));
      }
      return res;
    }
    if (expr.is_symb_of_sommet(at_pow)){
      gen & f =expr._SYMBptr->feuille;
      if (f.type==_VECT && f._VECTptr->size()==2 && is_strictly_positive(f._VECTptr->back(),contextptr))
	return solve_cleaned(f._VECTptr->front(),e_check,x,isolate_mode,contextptr);
    }
    // Check for re/im/conj in complexmode
    bool complexmode=isolate_mode & 1;
    if (complexmode){
      vecteur lc=mergevecteur(lop(expr,at_conj),mergevecteur(lop(expr,at_re),lop(expr,at_im)));
      int s=int(lc.size());
      for (int i=0;i<s;++i){
	gen f=lc[i]._SYMBptr->feuille;
	if (!is_zero(derive(f,x,contextptr),contextptr)){
	  identificateur xrei(" x"),ximi(" y");
	  gen xre(xrei),xim(ximi);
	  bool savec=complex_mode(contextptr);
	  bool savecv=complex_variables(contextptr);
	  complex_mode(false,contextptr);
	  complex_variables(false,contextptr);
	  gen tmp=subst(_numer(e,contextptr),x,xre+cst_i*xim,false,contextptr);
	  vecteur res=gsolve(makevecteur(re(tmp,contextptr),im(tmp,contextptr)),makevecteur(xre,xim),false,0,contextptr);
	  complex_mode(savec,contextptr);
	  complex_variables(savecv,contextptr);
	  s=int(res.size());
	  for (int j=0;j<s;++j){
	    if (res[j].type==_VECT && res[j]._VECTptr->size()==2){
	      gen a=res[j]._VECTptr->front();
	      gen b=res[j]._VECTptr->back();
	      if (is_zero(a,contextptr))
		res[j]=cst_i*b;
	      else {
		if (is_zero(b,contextptr))
		  res[j]=a;
		else
		  res[j]=symbolic(at_plus,gen(makevecteur(a,cst_i*b),_SEQ__VECT));
	      }
	    }
	  }
	  return res;
	}
      }
    }
    if ( (approx_mode(contextptr) || has_num_coeff(e)) && lidnt(e)==makevecteur(x)){
      vecteur vtmp=makevecteur(e,x),w;
      int fr=find_range(x,w,contextptr);
      if (w.size()==1)
        w=gen2vecteur(w.back());
      if (w.size()==2 && !is_inf(w.front()) && !is_inf(w.back()))
        vtmp[1]=symb_equal(x,symb_interval(w.front(),w.back()));
      vecteur res=gen2vecteur(in_fsolve(vtmp,contextptr));
      solve_ckrange(x,res,isolate_mode,contextptr);
      return res;
    }
    // should rewrite e in terms of a minimal number of variables
    // first factorization of e
    // Checking for abs
    vecteur la;
    if (!complexmode)
      la=lop(expr,at_abs);
    const_iterateur itla=la.begin(),itlaend=la.end();
    for (;itla!=itlaend;++itla){
      gen g=itla->_SYMBptr->feuille;
      if (is_zero(derive(g,x,contextptr),contextptr))
	continue;
      vecteur res;
      gen ee=subst(expr,*itla,g,false,contextptr);
      ee=simplifier(ee,contextptr);
      vecteur v1=solve(ee,x,isolate_mode,contextptr);
      const_iterateur it=v1.begin(),itend=v1.end();
      for (;it!=itend;++it){
	if (*it==x){
	  res=mergevecteur(res,solve_inequation(symbolic(at_superieur_egal,gen(makevecteur(g,0),_SEQ__VECT)),x,0,contextptr)); // > replaced by >= for solve((3*abs(6-x)+2*abs(3/2*x-5))=8,x)
	  continue;
	}
	if (contains(*it,x)){
	  *logptr(contextptr) << gettext("Warning, trying to solve ") << g << ">=0 with " << *it << '\n';
	  gen tmp=symbolic(at_solve,gen(makevecteur(symbolic(at_superieur_egal,gen(makevecteur(g,0),_SEQ__VECT)),x),_SEQ__VECT));
	  gen xval=eval(x,1,contextptr);
	  tmp=_tilocal(gen(makevecteur(tmp,*it),_SEQ__VECT),contextptr);
	  sto(xval,x,contextptr);
	  if (tmp.type==_VECT)
	    res=mergevecteur(res,*tmp._VECTptr);
	  continue;
	}
	gen g1=subst(g,x,*it,false,contextptr);
	gen g1f=evalf(g1,1,contextptr);
	if ( (g1f.type==_DOUBLE_ || g1f.type==_FLOAT_ || g1f.type==_REAL) && is_positive(g1,contextptr))
	  res.push_back(*it);
	else {
	  if (fastsign(simplifier(g1,contextptr),contextptr)==1 || normal(abs(g1,contextptr)-g1,contextptr)==0) // was ratnormal, but insufficient
	    res.push_back(*it);
	}
      }
      ee=subst(expr,*itla,-g,false,contextptr);
      v1=solve(ee,x,isolate_mode,contextptr);
      it=v1.begin(); itend=v1.end();
      for (;it!=itend;++it){
	if (*it==x){
	  res=mergevecteur(res,solve_inequation(symbolic(at_inferieur_strict,gen(makevecteur(g,0),_SEQ__VECT)),x,0,contextptr));
	  continue;
	}
	if (contains(*it,x)){
	  *logptr(contextptr) << gettext("Warning, trying to solve ") << g << "<=0 with " << *it << '\n';
	  gen tmp=symbolic(at_solve,gen(makevecteur(symbolic(at_inferieur_egal,gen(makevecteur(g,0),_SEQ__VECT)),x),_SEQ__VECT));
	  gen xval=eval(x,1,contextptr);
	  tmp=_tilocal(gen(makevecteur(tmp,*it),_SEQ__VECT),contextptr);
	  sto(xval,x,contextptr);
	  if (tmp.type==_VECT)
	    res=mergevecteur(res,*tmp._VECTptr);
	  continue;
	}
	gen g1=subst(g,x,*it,false,contextptr);
	gen g1f=evalf(g1,1,contextptr);
	if ( (g1f.type==_DOUBLE_ || g1f.type==_FLOAT_ || g1f.type==_REAL) && is_positive(-g1,contextptr))
	  res.push_back(*it);
	else {
	  if (normal(abs(g1,contextptr)+g1,contextptr)==0) 
	    res.push_back(*it);
	}
      }
      return res;
    }
    expr=rationalize(expr,x,contextptr);
    // Checking for fractional power
    //vecteur surd1,surd2; // ggb 4089
    //surd2pow(expr,surd1,surd2,contextptr);
    //if (!surd1.empty()) expr=subst(expr,surd1,surd2,false,contextptr);
    // Remark: algebraic extension could also be solved using resultant
    vecteur ls(lvarfracpow(expr,x,contextptr));
    if (!ls.empty()){  // Use auxiliary variables
      int s=int(ls.size())/3;
      vecteur substin,substout,equations,listvars(lvarx(expr,x,true));
      // remove ls from listvars, add aux var instead
      for (int i=0;i<s;++i){
	gen lsvar=ls[3*i+2];
	int j=equalposcomp(listvars,ls);
	if (j)
	  listvars.erase(listvars.begin()+j-1);
      }
      if (listvars.size()!=1){
	// try factorization before err, e.g. (sqrt(x))^-1*exp(x)+2*sqrt(x)*exp(x)
	gen tryf=factor(expr,false,contextptr);
	if (tryf.is_symb_of_sommet(at_neg))
	  tryf=tryf._SYMBptr->feuille;
	if (tryf.is_symb_of_sommet(at_prod) || tryf.is_symb_of_sommet(at_pow)){
	  // recurse
	  return solve_cleaned(tryf,e_check,x,isolate_mode,contextptr);
	}
	return vecteur(1,gensizeerr(gettext("Unable to isolate ")+gen(listvars).print(contextptr)+gettext(" solving equation ")+expr.print(contextptr)));
      }
      vecteur assumedvars;
      for (int i=0;i<s;++i){
	gen lsvar=ls[3*i+2];
	gen ls3i=subst(ls[3*i],substin,substout,false,contextptr);
	if (equalposcomp(substin,lsvar))
	  continue;
	substin.push_back(lsvar);
#ifdef GIAC_GGB
	gen tmp("d_"+print_intvar_counter(contextptr),contextptr);
#else
	gen tmp("c__"+print_intvar_counter(contextptr),contextptr);
#endif
	if (!(ls[3*i+1].val %2)){
	  assumesymbolic(symb_superieur_egal(tmp,0),0,contextptr); 
	  assumedvars.push_back(tmp);
	}
	listvars.push_back(tmp);
	substout.push_back(tmp);
	equations.push_back(pow(tmp,ls[3*i+1],contextptr)-ls3i);
      }
      gen expr1=subst(expr,substin,substout,false,contextptr);
      expr1=factor(expr1,false,contextptr);
      if (is_undef(expr1))
	return vecteur(1,expr1);
      vecteur fullres;
      solve_fracpow(expr1,x,equations,listvars,fullres,isolate_mode,contextptr);
      // Check that e_check at x=fullres is 0
      // Only if expr1 does not depend on other variables than x
      vecteur othervar(1,x),res;
      lidnt(expr1,othervar,false);
      int pospi;
      if ((pospi=equalposcomp(othervar,cst_pi))) 
	othervar.erase(othervar.begin()+pospi-1);
      if (othervar.size()<=listvars.size()){
	const_iterateur it=fullres.begin(),itend=fullres.end();
	for (;it!=itend;++it){
	  vecteur algv=alg_lvar(evalf(*it,1,contextptr));
	  if (!algv.empty() && algv.front().type==_VECT && !algv.front()._VECTptr->empty()){
	    *logptr(contextptr) << "Warning, " << *it << " not checked" << '\n';
	    res.push_back(*it);
	  }
	  else {
#ifdef HAVE_LIBMPFR
	    gen tmp=subst(e_check,x,*it,false,contextptr);
	    if ((isolate_mode & 1)==0 && tmp.is_symb_of_sommet(at_plus) && tmp._SYMBptr->feuille.type==_VECT && tmp._SYMBptr->feuille._VECTptr->size()==2){
	      gen a=evalf_double(tmp._SYMBptr->feuille._VECTptr->front(),1,contextptr);
	      if (a.type!=_DOUBLE_)
		continue;
	      a=evalf_double(tmp._SYMBptr->feuille._VECTptr->back(),1,contextptr);
	      if (a.type!=_DOUBLE_)
		continue;
	    }
	    tmp=_evalf(makesequence(tmp,100),contextptr);
	    tmp=abs(tmp,contextptr);
#else
	    gen tmp=evalf(subst(e_check,x,*it,false,contextptr),1,contextptr);
#endif
	    if ( (tmp.type==_DOUBLE_ || tmp.type==_REAL || tmp.type==_FLOAT_) && is_greater(1e-8,abs(tmp,contextptr),contextptr)){
	      if ( (calc_mode(contextptr)==1 || abs_calc_mode(contextptr)==38) && has_op(*it,*at_rootof))
		res.push_back(evalf(*it,1,contextptr));
	      else
		res.push_back(*it);
	      continue;
	    }
	    if (is_undef(tmp)){
#if defined(EMCC) || defined(EMCC2) // computation takes too long in emscripten, accept the solution without check
	      tmp=0;
	      *logptr(contextptr) << "Warning, " << *it << " not checked" << '\n';
#else
	      tmp=limit(e_check,x,*it,0,contextptr);
#endif
	    }
	    if (is_zero(tmp,contextptr))
	      res.push_back(*it);
	  }
	}
      }
      else {
	if (debug_infolevel)
	  *logptr(contextptr) << gettext("Warning, solutions were not checked!") << '\n';
	res=fullres;
      }
      purgenoassume(assumedvars,contextptr);
      return res;
    }
    vecteur lv=lvarx(expr,x);
    if (lv.size()>1){
      gen tmp=factor(simplify(expr,contextptr),false,contextptr);
      if (is_undef(tmp))
	return vecteur(1,tmp);
      int lvs=0;
      if (tmp.is_symb_of_sommet(at_prod) && tmp._SYMBptr->feuille.type==_VECT){
	vecteur & f=*tmp._SYMBptr->feuille._VECTptr;
	int fs=int(f.size());
	for (int i=0;i<fs;++i){
	  lvs=int(lvarx(f[i],x).size());
	  if (lvs>1)
	    break;
	}
      }
      else
	lvs=int(lvarx(tmp,x).size());
      if (lvs<2)
	expr=tmp;
    }
    // -> exp/ln
    expr=pow2expln(expr,x,contextptr);
    bool setcplx=complexmode && complex_mode(contextptr)==false;
    if (setcplx)
      complex_mode(true,contextptr);
#if 1
    vecteur lvarxexpr(lvarx(expr,x));
    if (lvarxexpr.size()==1){
      // quick check for expr=alpha*x^n+beta
      vecteur ll(1,lvarxexpr.front());
      lvar(expr,ll);
      fraction f=sym2r(expr,ll,contextptr);
      if (f.num.type==_POLY){
	polynome & p = *f.num._POLYptr;
	int n=p.lexsorted_degree();
	int n0=p.valuation(0);
	n -= n0;
	if (n>3){
	  std::vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end();
	  polynome p1=Tnextcoeff<gen>(it,itend);
	  p1.dim=p.dim-1;
	  if (it!=itend){
	    polynome p2=Tnextcoeff<gen>(it,itend);
	    p2.dim=p.dim-1;
	    if (it==itend){
	      ll.erase(ll.begin());
	      gen g1=r2sym(p1,ll,contextptr);
	      gen g2=r2sym(p2,ll,contextptr);
	      factorization f; gen an,extra_div; bool has_nthroot=false;
	      if ((isolate_mode & 1) && cfactor(p,an,f,true,extra_div) && f.size()>1){
		for (unsigned i=0;i<f.size();++i){
		  if (f[i].fact.lexsorted_degree()==1){
		    it=f[i].fact.coord.begin(),itend=f[i].fact.coord.end();
		    p1=Tnextcoeff<gen>(it,itend);
		    p2=Tnextcoeff<gen>(it,itend);
		    an=-r2sym(p2,ll,contextptr)/r2sym(p1,ll,contextptr);
		    an=r2sym(an,vecteur(1,vecteur(0)),contextptr);
		    has_nthroot=true;
		    break;
		  }
		}
	      }
	      if (isolate_mode & 1){
		vecteur res;
		if (is_positive(g2/g1,contextptr) && !has_nthroot){
		  gen g=pow(g2/g1,inv(n,contextptr),contextptr);
		  for (int i=n-1;i>=0;--i){
		    res.push_back(g*exp(cst_i*cst_pi*gen(2*i+1)/n,contextptr));
		  }
		}
		else {
		  gen g=has_nthroot?an:pow(-g2/g1,inv(n,contextptr),contextptr);
		  for (int i=n-1;i>=0;--i){
		    if (n%2)
		      res.push_back(g*exp(cst_i*cst_two_pi*gen(i)/n,contextptr));
		    else
		      res.push_back(g*exp(cst_i*cst_pi*gen(i)/(n/2),contextptr));
		  }
		}
		if (n0)
		  res.push_back(0);
		return solve_subst(res,x,lvarxexpr.front(),isolate_mode,contextptr);
	      }
	      vecteur res;
	      if (n0)
		res.push_back(0);
	      gen g2g1=g2/g1;
	      if (n%2){
		if (is_positive(g2g1,contextptr))
		  res.push_back(-pow(g2g1,inv(n,contextptr),contextptr));
		else
		  res.push_back(pow(-g2g1,inv(n,contextptr),contextptr));
		return solve_subst(res,x,lvarxexpr.front(),isolate_mode,contextptr);
	      }
	      if (is_positive(g2g1,contextptr))
		return solve_subst(res,x,lvarxexpr.front(),isolate_mode,contextptr);
	      gen g=simplifier(pow(-g2g1,inv(n,contextptr),contextptr),contextptr);
	      res.push_back(-g); res.push_back(g);
	      return solve_subst(res,x,lvarxexpr.front(),isolate_mode,contextptr);
	    }
	  }
	}
      }
    }
#endif
    gen ea,eb,ec;
    if (is_quadratic_wrt(expr,x,ea,eb,ec,contextptr)){
      gen tmp;
      bool doreal=complex_mode(contextptr) && is_real(ea,contextptr) && is_real(eb,contextptr) && is_real(ec,contextptr) && is_positive(eb*eb-4*ea*ec,contextptr);
      if (doreal)
	complex_mode(false,contextptr);
      tmp=factor(expr,false,contextptr); // factor in complex or real mode
      if (doreal)
	complex_mode(true,contextptr);
      if (!lop(tmp,at_rootof).empty())
	expr=_sqrfree(expr,contextptr);
      else
	expr=tmp;
    }
    else
      expr=factor(expr,false,contextptr); // factor in complex or real mode
    if (expr.is_symb_of_sommet(at_neg))
      expr=expr._SYMBptr->feuille;
    if (is_undef(expr))
      return vecteur(1,expr);
    if (setcplx)
      complex_mode(false,contextptr);
    lv=lvarx(expr,x);
    int s=int(lv.size());
    if (s==1 && lv[0].is_symb_of_sommet(at_tan) && expr.is_symb_of_sommet(at_prod) && expr._SYMBptr->feuille.type==_VECT){
      // remove denominator if limit!=0
      gen etan=limit(subst(expr,lv[0],x,false,contextptr),x,plus_inf,-1,contextptr);
      if (!is_zero(etan,contextptr)){
	const vecteur varg=*expr._SYMBptr->feuille._VECTptr;
	vecteur newarg;
	for (unsigned i=0;i<varg.size();++i){
	  if (varg[i].type==_SYMB && (varg[i]._SYMBptr->sommet==at_inv || (varg[i]._SYMBptr->sommet==at_pow && ck_is_positive(-varg[i]._SYMBptr->feuille._VECTptr->back(),contextptr))) )
	    ;
	  else
	    newarg.push_back(varg[i]);
	}
	expr=_prod(gen(newarg,_SEQ__VECT),contextptr);
      }
    }
    vecteur v;
    if (!s){
      if (is_zero(expr,contextptr))
	v.push_back(x);
      return v;
    }
    solve(expr,x,v,isolate_mode,contextptr);
    if (is_undef(v)) return v;
    v=solve_numeric_check(e_check,x,v,(isolate_mode & 1)==0,contextptr);
    if (0 && !(isolate_mode & 2)){
      // check solutions if there is a tan inside, commented now that we have the test above
      for (int i=0;i<s;++i){
	if (lv[i].is_symb_of_sommet(at_tan)){
	  vecteur res;
	  const_iterateur it=v.begin(),itend=v.end();
	  for (;it!=itend;++it){
	    if (has_num_coeff(*it) || is_zero(recursive_normal(limit(_tan2sincos2(expr,contextptr),x,*it,0,contextptr),contextptr),contextptr) || is_zero(recursive_normal(limit(expr,x,*it,0,contextptr),contextptr),contextptr))
	      res.push_back(*it);
	  }
	  return res;
	}
      }
    }
    return v;
  }

  vecteur protect_solve(const gen & e,const identificateur & x,int isolate_mode,GIAC_CONTEXT){
    vecteur res;
#ifdef NO_STDEXCEPT
    res=solve(e,x,isolate_mode,contextptr);
#else
    try {
      res=solve(e,x,isolate_mode,contextptr);
    } catch(std::runtime_error & e){
      last_evaled_argptr(contextptr)=NULL;
      res=vecteur(1,undef);
    }
#endif
    return res;
  }

  vecteur solve(const gen & e,const identificateur & x,int isolate_mode,GIAC_CONTEXT){
    ck_isolate_mode(isolate_mode,x,contextptr);
    if (has_i(e))
      isolate_mode |= 1;
    if (is_undef(e)) return vecteur(0);
    gen expr(exp2pow(e,contextptr));
    // keep e if x is isolable inside
    vecteur lv0(lvarx(e,x));
    int s0=int(lv0.size());
    if (s0==1){
      gen xvar(lv0.front());
      if (xvar!=x && xvar.type==_SYMB && equalposcomp(solve_fcns_tab,xvar._SYMBptr->sommet))
	expr=e;
    }
    gen modulo;
    if (has_mod_coeff(expr,modulo)){
      vecteur v;
      if (!modsolve(expr,x,modulo,v,contextptr))
	return vecteur(1,gensizeerr(gettext("Modulo too large")));
      return v;
    }
    // embedded inequation? Beware, needs revert if we evaluate later to check one solution 
    if (e.type==_SYMB){
      if (e._SYMBptr->sommet==at_prod){
        vecteur v=gen2vecteur(e._SYMBptr->feuille);
        int i=0;
        for (;i<v.size();++i){
          if (is_inequation(v[i]))
            break;
        }
        if (i<v.size()){
          vecteur res;
          for (i=0;i<v.size();++i)
            res=mergevecteur(res,solve(v[i],x,isolate_mode,contextptr));
          return res;
        }
      }
      if (e._SYMBptr->sommet==at_inferieur_strict)
	return solve_inequation(e,x,-2,contextptr);
      if (e._SYMBptr->sommet==at_inferieur_egal)
	return solve_inequation(e,x,-1,contextptr);
      if (e._SYMBptr->sommet==at_superieur_strict)
	return solve_inequation(e,x,2,contextptr);
      if (e._SYMBptr->sommet==at_superieur_egal)
	return solve_inequation(e,x,1,contextptr);
      if (e._SYMBptr->sommet==at_equal || e._SYMBptr->sommet==at_equal2 || e._SYMBptr->sommet==at_same)
	expr = e._SYMBptr->feuille._VECTptr->front()-e._SYMBptr->feuille._VECTptr->back();
    }
    vecteur v=lvarx(expr,x);
    v=lop(v,at_of);
    if (!v.empty())
      return vecteur(1,gensizeerr("Invalid function "+v.front().print(contextptr)+" perhaps a missing * for multiplication"));
    clean(expr,x,contextptr);
    vecteur res= solve_cleaned(expr,e,x,isolate_mode,contextptr);
    if (has_op(expr,*at_unit)){
      gen tmp=eval(res,1,contextptr);
      //tmp=mksa_reduce(tmp,contextptr);
      if (tmp.type==_VECT)
	res=*tmp._VECTptr;
    }
    return res;
  }

  gen remove_and(const gen & g,const unary_function_ptr * u){
    if (g.type==_VECT){
      vecteur res;
      const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      for (;it!=itend;++it){
	gen tmp=remove_and(*it,u);
	if (tmp.type!=_VECT){
	  tmp=remove_and(*it,at_and);
	  res.push_back(tmp);
	}
	else
	  res=mergevecteur(res,*tmp._VECTptr);
      }
      return res;
    }
    if (!g.is_symb_of_sommet(u)){
      // if (g.is_symb_of_sommet(at_and)) return g._SYMBptr->feuille;
      return g;
    }
    return remove_and(g._SYMBptr->feuille,u);
  }

  // revert inequations for checking because solve(x>0) returns x>0 but x>0 evals to 1, not 0
  gen solve_revert_inequations(const gen & g){
    if (g.type==_VECT){
      vecteur v=*g._VECTptr;
      for (int i=0;i<v.size();++i)
        v[i]=solve_revert_inequations(v[i]);
      return gen(v,g.subtype);
    }
    if (g.type!=_SYMB)
      return g;
    if (g._SYMBptr->sommet==at_superieur_strict)
      return symbolic(at_inferieur_egal,g._SYMBptr->feuille);
    if (g._SYMBptr->sommet==at_superieur_egal)
      return symbolic(at_inferieur_strict,g._SYMBptr->feuille);
    if (g._SYMBptr->sommet==at_inferieur_strict)
      return symbolic(at_superieur_egal,g._SYMBptr->feuille);
    if (g._SYMBptr->sommet==at_inferieur_egal)
      return symbolic(at_superieur_strict,g._SYMBptr->feuille);
    return symbolic(g._SYMBptr->sommet,solve_revert_inequations(g._SYMBptr->feuille));
  }

  bool taillesort(const gen & a,const gen & b){
    bool ai=is_inequation(a),bi=is_inequation(b);
    if (ai && !bi)
      return true;
    if (!ai &&bi)
      return false;
    return taille(a,RAND_MAX)<taille(b,RAND_MAX);
  }

  vecteur solve(const gen & e,const gen & x,int isolate_mode,GIAC_CONTEXT){
    bool complexmode=isolate_mode & 1;
    vecteur res;
    if (x.type!=_IDNT){
      if (x.type==_VECT && x._VECTptr->size()==1 && e.type==_VECT && e._VECTptr->size()==1){
	vecteur res=solve(e._VECTptr->front(),x._VECTptr->front(),isolate_mode,contextptr);
	iterateur it=res.begin(),itend=res.end();
	for (;it!=itend;++it)
	  *it=vecteur(1,*it);	
	return res;
      }
      if (x.type==_VECT )
	return gsolve(gen2vecteur(e),*x._VECTptr,complexmode,(approx_mode(contextptr)?1:0),contextptr);
      identificateur xx("x_solve");
      res=solve(subst(e,x,xx,false,contextptr),xx,isolate_mode,contextptr);
      res=subst(res,xx,x,false,contextptr);
      return res;
    }
    if (e.type==_VECT){
      if (x.type==_IDNT &&
          _is_polynomial(makesequence(e,x),contextptr)!=0
          // lvarx(e,x)==vecteur(1,x)
          )
	return solve(_gcd(e,contextptr),x,isolate_mode,contextptr);
      // sort e in asc. order of complexity
      vecteur ev=*e._VECTptr;
      sort(ev.begin(),ev.end(),taillesort);
      const_iterateur it=ev.begin(),itend=ev.end();
      gen curx=x._IDNTptr->eval(1,x,contextptr);
      res=vecteur(1,x); // everything is solution up to now
      double eps=epsilon(contextptr);
      int N=decimal_digits(contextptr);
#ifdef HAVE_LIBMPFR
      int addN=60; double muleps=std::pow(10.0,-addN);
#else
      int addN=1; double muleps=0.1;
#endif
      for (;it!=itend;++it){
	if (res==vecteur(1,x)){
          // temp. increase proot default precision, otherwise roots are discarded
          epsilon(muleps*eps,contextptr); decimal_digits(N+addN,contextptr);
	  res=solve(*it,*x._IDNTptr,isolate_mode,contextptr);
          epsilon(eps,contextptr); decimal_digits(N,contextptr);
        }
	else { // check every element of res
          gen eq=solve_revert_inequations(*it);
	  vecteur newres;
	  const_iterateur jt=res.begin(),jtend=res.end();
	  for (;jt!=jtend;++jt){
	    if (is_inequation(*jt) ||
		jt->is_symb_of_sommet(at_and)){
              gen xval=assumeeval(x,contextptr);
              assumesymbolic(*jt,0,contextptr); // assume and solve next equation
              // temp. increase proot default precision, otherwise roots are discarded
              epsilon(muleps*eps,contextptr); decimal_digits(N+addN,contextptr);
	      newres=mergevecteur(newres,solve(*it,*x._IDNTptr,isolate_mode,contextptr));
              epsilon(eps,contextptr); decimal_digits(N,contextptr);
              restorepurge(xval,x,contextptr);
	      // purgenoassume(x,contextptr);
	    }
	    else { 
	      gen tst=subst(eq,x,*jt,true,contextptr);
	      tst=eval(tst,1,contextptr);
	      tst=normal(tst,1,contextptr); // FIXME call a solve_simplify(tst,contextptr);
	      if (is_zero(tst,contextptr))
		newres.push_back(*jt);
	    }
	  } // end for (;jt!=jtend;++jt) loop on previous solutions
	  res=newres;
	} // end else
      } // end for (;it!=itend;++it) loop on equations
      if (curx!=x)
	sto(curx,x,contextptr);
      return res;
    }
    else
      res=solve(e,*x._IDNTptr,isolate_mode,contextptr);
    return res;
  }

  static gen symb_solution(const gen & g,const gen & var,GIAC_CONTEXT){
    if (var.type!=_VECT){
      if (var.type==_IDNT && g.type!=_IDNT && !lvarx(g,var).empty())
	return g;
      else 
	return symbolic(at_equal,makesequence(var,g));
    }
    vecteur v=*var._VECTptr;
    if (g.type!=_VECT || g._VECTptr->size()!=v.size())
      return gensizeerr(contextptr);
    const_iterateur it=v.begin(),itend=v.end(),jt=g._VECTptr->begin();
    vecteur res;
    res.reserve(itend-it);
    for (;it!=itend;++it,++jt){
      // if (*it!=*jt)
      if (jt->is_symb_of_sommet(at_equal) && jt->_SYMBptr->feuille[0]==*it)
	res.push_back(*jt);
      else
	res.push_back(symbolic(at_equal,makesequence(*it,*jt)));
    }
    if (xcas_mode(contextptr)==3)
      return symbolic(at_and,res);
    else
      return res;
  }

  static gen quote_inferieur_strict(const gen & g,GIAC_CONTEXT){
    return symbolic(at_quote,symbolic(at_inferieur_strict,eval(g,eval_level(contextptr),contextptr)));
  }

  static gen quote_superieur_strict(const gen & g,GIAC_CONTEXT){
    return symbolic(at_quote,symbolic(at_superieur_strict,eval(g,eval_level(contextptr),contextptr)));
  }

  static gen quote_inferieur_egal(const gen & g,GIAC_CONTEXT){
    return symbolic(at_quote,symbolic(at_inferieur_egal,eval(g,eval_level(contextptr),contextptr)));
  }

  static gen quote_superieur_egal(const gen & g,GIAC_CONTEXT){
    return symbolic(at_quote,symbolic(at_superieur_egal,eval(g,eval_level(contextptr),contextptr)));
  }

  static gen quote_different(const gen & g,GIAC_CONTEXT){
    vecteur v=gen2vecteur(g);
    gen d=eval(v[0]-v[1],eval_level(contextptr),contextptr);
    return symbolic(at_quote,symbolic(at_superieur_strict,makesequence(symb_pow(d,2),0)));
  }

  static gen quote_conj(const gen & g,GIAC_CONTEXT){
    return symbolic(at_quote,symbolic(at_conj,eval(g,eval_level(contextptr),contextptr)));
  }

  static gen quote_re(const gen & g,GIAC_CONTEXT){
    return symbolic(at_quote,symbolic(at_re,eval(g,eval_level(contextptr),contextptr)));
  }

  static gen quote_im(const gen & g,GIAC_CONTEXT){
    return symbolic(at_quote,symbolic(at_im,eval(g,eval_level(contextptr),contextptr)));
  }

  gen expand_not(const gen & g,GIAC_CONTEXT){
    if (g.type==_VECT){
      vecteur res(*g._VECTptr);
      for (int i=0;i<res.size();++i)
        res[i]=expand_not(res[i],contextptr);
      return gen(res,g.subtype);
    }
    if (!g.is_symb_of_sommet(at_not))
      return g;
    gen f=g._SYMBptr->feuille;
    if (f.is_symb_of_sommet(at_not))
      return f._SYMBptr->feuille;
    if (f.is_symb_of_sommet(at_and))
      return symbolic(at_ou,solve_revert_inequations(f._SYMBptr->feuille));
    if (f.is_symb_of_sommet(at_ou))
      return symbolic(at_and,solve_revert_inequations(f._SYMBptr->feuille));
    if (is_inequation(f))
      return solve_revert_inequations(f);
    return g;
  }

  vecteur solvepreprocess(const gen & args,bool complexmode,GIAC_CONTEXT){
    gen g(args);
    // change made 2024/08/02 to replace identifiers only in args by value
    // so that != < > are replaced by innert form before full evaluation
    if (g.type==_IDNT && g!=vx_var)
      g=eval(g,1,contextptr);
    else if (g.type==_VECT && g.subtype==_SEQ__VECT && g._VECTptr->size()>=2) {
      vecteur v(*g._VECTptr);
      gen g0=v[0];
      vecteur v1=gen2vecteur(v[1]);
      if (g0.type==_IDNT && !equalposcomp(v1,g0))
        g0=eval(g0,1,contextptr);
      else if (g0.type==_VECT){
        vecteur w(*g0._VECTptr);
        for (int i=0;i<w.size();++i){
          if (w[i].type==_IDNT && !equalposcomp(v1,w[i]))
            w[i]=eval(w[i],1,contextptr);
        }
        g0=gen(w,g0.subtype);
      }
      v[0]=g0;
      g=gen(v,_SEQ__VECT);
    }
    g=expand_not(g,contextptr);
    if (g.is_symb_of_sommet(at_and) && g._SYMBptr->feuille.type==_VECT)
      g=makesequence(expand_not(*g._SYMBptr->feuille._VECTptr,contextptr),vx_var);
    if (g.type==_VECT && !g._VECTptr->empty() && (g._VECTptr->front().is_symb_of_sommet(at_abs))){
      vecteur v(*g._VECTptr);
      v.front()=v.front()._SYMBptr->feuille;
      g=gen(v,g.subtype);
    }
    if (g.type==_VECT && !g._VECTptr->empty() && (g._VECTptr->front().is_symb_of_sommet(at_and) || g._VECTptr->front().is_symb_of_sommet(at_ou))){
      vecteur v(*g._VECTptr);
      if (v.front().is_symb_of_sommet(at_ou) && v.front()._SYMBptr->feuille.type==_VECT){
        // FIXME should merge if solving inequation
        v.front()=symbolic(at_prod,v.front()._SYMBptr->feuille);
      }
      else
        v.front()=expand_not(remove_and(v.front(),at_and),contextptr);
      g=gen(v,g.subtype);
    }
    // quote < <= > and >=
    vector<const unary_function_ptr *> quote_inf;
    quote_inf.push_back(at_inferieur_strict);
    quote_inf.push_back(at_inferieur_egal);
    quote_inf.push_back(at_superieur_strict);
    quote_inf.push_back(at_superieur_egal);
    quote_inf.push_back(at_different);
    if (complexmode){
      quote_inf.push_back(at_conj);
      quote_inf.push_back(at_re);
      quote_inf.push_back(at_im);
    }
    vector< gen_op_context > quote_inf_v;
    quote_inf_v.push_back(quote_inferieur_strict);
    quote_inf_v.push_back(quote_inferieur_egal);
    quote_inf_v.push_back(quote_superieur_strict);
    quote_inf_v.push_back(quote_superieur_egal);
    quote_inf_v.push_back(quote_different);
    if (complexmode){
      quote_inf_v.push_back(quote_conj);
      quote_inf_v.push_back(quote_re);
      quote_inf_v.push_back(quote_im);
    }
    g=subst(g,quote_inf,quote_inf_v,true,contextptr);
    return plotpreprocess(g,contextptr);
  }

  gen solvepostprocess(const gen & g,const gen & x,GIAC_CONTEXT){
    if (g.type!=_VECT)
      return g;
    vecteur res=*g._VECTptr;
    // convert solution to an expression
    iterateur it=res.begin(),itend=res.end();
    if (it==itend)
      return res;
    if (x.type==_VECT || xcas_mode(contextptr)==3 || calc_mode(contextptr)==1){
      for (;it!=itend;++it)
	*it=symb_solution(*it,x,contextptr);
    }
    if (xcas_mode(contextptr)==3)
      return symbolic(at_ou,res);
    if (xcas_mode(contextptr)==2 || calc_mode(contextptr)==1)
      return gen(res,_SET__VECT);
    return gen(res,_SEQ__VECT);
  }

  gen point2xy(const gen & g,GIAC_CONTEXT){
    if (g.type==_VECT)
      return apply(g,point2xy,contextptr);
    if (is_equal(g))
      return apply_to_equal(g,point2xy,contextptr);
    if (g.is_symb_of_sommet(at_pnt))
      return _coordonnees(g,contextptr);
    return g;
  }

  bool is_log10(const gen & g,gen & arg){
    if (g.type!=_SYMB || g._SYMBptr->sommet!=at_prod) return false;
    const gen & f = g._SYMBptr->feuille;
    if (f.type!=_VECT) return false;
    const vecteur & v = *f._VECTptr;
    if (!v.back().is_symb_of_sommet(at_inv))
      return false;
    const gen & va = v.back()._SYMBptr->feuille;
    if (va.is_symb_of_sommet(at_ln) && va._SYMBptr->feuille==10){
      if (v.size()==2)
	arg=v.front();
      else
	arg=symbolic(at_prod,gen(vecteur(v.begin(),v.end()-1),_SEQ__VECT));
      return true;
    }
    return false;
  }

  static gen postprocess_return(const vecteur & res,const gen &x,bool withequal,GIAC_CONTEXT){
    if (!withequal){
      vecteur res_(res);
      for (int i=0;i<res.size();++i){
        if (res_[i].is_symb_of_sommet(at_equal))
          res_[i]=res_[i][2];
      }
      return gen(res_,_LIST__VECT);
    }
    gen vres=solvepostprocess(res,x,contextptr);
    return vres;
  }    

  gen _solve_uncompressed(const gen & args,bool postprocess,GIAC_CONTEXT){
    if (args.type==_VECT && args.subtype==0 && ckmatrix(args))
      return _solve_uncompressed(change_subtype(args,_SEQ__VECT),postprocess,contextptr);
    if (args.type==_VECT && args.subtype==_SEQ__VECT && lidnt(args).empty())
      return _linsolve(args,contextptr);
    vecteur res;
    if (args.type==_VECT && !args._VECTptr->empty() && (args._VECTptr->back()==at_equal || args._VECTptr->back()==at_different)){
      int x=calc_mode(contextptr);
      calc_mode(1,contextptr);
      gen g=gen(vecteur(args._VECTptr->begin(),args._VECTptr->end()-1),args.subtype);
      postprocess=args._VECTptr->back()==at_equal;
      g=_solve_uncompressed(g,postprocess,contextptr);
      calc_mode(x,contextptr);
      if (x==1 && g.subtype==_LIST__VECT)
        g.subtype=0;
      return g;
    }
    int isolate_mode=int(complex_mode(contextptr)) | int(int(all_trig_sol(contextptr)) << 1);
    if (calc_mode(contextptr)==1){
      if (args.type==_VECT && args.subtype!=_SEQ__VECT){
	vecteur w(1,cst_pi);
	lidnt(args,w,false);
	w.erase(w.begin());
	return _solve(makesequence(args,w),contextptr);
      }
    }
    if (args.type!=_VECT) // change 9 dec 2019: x no more default var
      return _solve(makesequence(args,ggb_var(eval(args,1,contextptr))),contextptr);
    if (has_op(args,*at_irem)){
      vector<const unary_function_ptr *> v(1,at_irem);
      vector<gen_op_context> w(1,_normalmod);
      return _solve(subst(args,v,w,false,contextptr),contextptr);
    }
    vecteur v(solvepreprocess(args,complex_mode(contextptr),contextptr));
    if (v.size()>1 && v[1].is_symb_of_sommet(at_unquote))
      v[1]=eval(v[1],1,contextptr);
    int s=int(v.size());
    if (s && ckmatrix(v[0])){
      vecteur w;
      aplatir(*v[0]._VECTptr,w,true);
      v[0]=w;
    }
    if (s && v.back()==at_interval)
      return _fsolve(args,contextptr);
    if (s>=2 && ckmatrix(v[1]) && v[1]._VECTptr->front()._VECTptr->size()==1)
      v[1]=mtran(*v[1]._VECTptr).front();
    if (s==2 && is_equal(v[1]))
      return _fsolve(gen(makevecteur(v[0],v[1]._SYMBptr->feuille[0],v[1]._SYMBptr->feuille[1]),_SEQ__VECT),contextptr);
    if (s>2)
      return _fsolve(args,contextptr);
    gen arg1(point2xy(v.front(),contextptr));
    if (arg1.type==_VECT){ // Flatten equations which are list of equations
      vecteur w,w1,w2;
      const_iterateur it=arg1._VECTptr->begin(),itend=arg1._VECTptr->end();
      for (;it!=itend;++it){
	gen tmp=equal2diff(*it);
	if (tmp.type==_VECT){
	  const_iterateur jt=tmp._VECTptr->begin(),jtend=tmp._VECTptr->end();
	  for (;jt!=jtend;++jt){
	    if (is_inequation(*jt))
	      w1.push_back(*jt);
	    else
	      w2.push_back(*jt);
	  }
	}
	else {
	  if (is_inequation(tmp))
	    w1.push_back(tmp);
	  else
	    w2.push_back(tmp);
	}
      }
      // put inequations first
      w=mergevecteur(w1,w2);
      arg1=w;
    }
    if (arg1.is_symb_of_sommet(at_plus) && arg1._SYMBptr->feuille._VECTptr->size()==2)
      arg1=symb_equal(arg1,0);
    if (arg1.type!=_VECT && !is_equal(arg1) && !is_inequation(arg1))
      *logptr(contextptr) << gettext("Warning, argument is not an equation, solving ") << arg1 << "=0" << '\n';
    else {
#if 1 // ATESTER
      if (arg1.type==_VECT && arg1._VECTptr->size()==2 && v[1].type==_VECT && v[1]._VECTptr->size()==2){
	gen eq1(arg1._VECTptr->front()),eq2(arg1._VECTptr->back()),var1(v[1]._VECTptr->front()),var2(v[1]._VECTptr->back()),a,b,c,d;
	if (is_linear_wrt(eq1,var1,a,b,contextptr) && is_linear_wrt(eq2,var1,c,d,contextptr) && lvarxwithinv(makevecteur(a,b,c,d),var2,contextptr).size()>1 && !is_zero(recursive_normal(a*d-b*c,contextptr))){
	  // a*var1+b=c*var1+d=0 => b*c-a*d=0
	  vecteur V;
	  gen res=_solve(makesequence(symb_equal(a*d,b*c),var2),contextptr);
	  if (res.type==_VECT && !res._VECTptr->empty()){
	    for (unsigned i=0;i<res._VECTptr->size();++i){
	      gen v2=(*res._VECTptr)[i];
	      gen v2val=v2.is_symb_of_sommet(at_equal)?v2._SYMBptr->feuille._VECTptr->back():v2;
	      gen res2=derive(eq1,var1,contextptr);
	      if (!is_zero(res2))
		res2=_simplify(subst(eq1,var2,v2val,false,contextptr),contextptr);
	      if (!is_zero(res2)){
		if (taille(res2,RAND_MAX)>taille(v2val,RAND_MAX)*10){
		  res2=subst(eq1,var2,v2val,false,contextptr);
		  res2=ratnormal(_solve(makesequence(symb_equal(res2,0),var1),contextptr),contextptr);
		}
		else
		  res2=_simplify(_solve(makesequence(symb_equal(res2,0),var1),contextptr),contextptr);
	      }
	      gen res3=derive(eq2,var1,contextptr);
	      if (!is_zero(res3))
		res3=_simplify(subst(eq2,var2,v2val,false,contextptr),contextptr);
	      if (!is_zero(res3)){
		if (taille(res3,RAND_MAX)>taille(v2val,RAND_MAX)*10){
		  res3=subst(eq2,var2,v2val,false,contextptr);
		  res3=ratnormal(_solve(makesequence(symb_equal(res3,0),var1),contextptr),contextptr);
		}
		else
		  res3=_simplify(_solve(makesequence(symb_equal(res3,0),var1),contextptr),contextptr);
		if (is_zero(res2))
		  res2=res3;
		else
		  res2=_intersect(makesequence(res2,res3),contextptr);
	      }
	      if (res2.type==_VECT){
		for (unsigned j=0;j<res2._VECTptr->size();++j)
		  V.push_back(makevecteur((*res2._VECTptr)[j],v2));
	      }
	    }
	    return V;
	  } // if res.type==_VECT ...
	}
	if (is_linear_wrt(eq1,var2,a,b,contextptr) && is_linear_wrt(eq2,var2,c,d,contextptr) && lvarxwithinv(makevecteur(a,b,c,d),var1,contextptr).size()>1 && !is_zero(recursive_normal(a*d-b*c,contextptr))){
	  vecteur V;
	  gen res=_solve(makesequence(symb_equal(a*d,b*c),var1),contextptr);
	  if (res.type==_VECT && !res._VECTptr->empty()){
	    for (unsigned i=0;i<res._VECTptr->size();++i){
	      gen v1=(*res._VECTptr)[i];
	      gen v1val=v1.is_symb_of_sommet(at_equal)?v1._SYMBptr->feuille._VECTptr->back():v1;
	      gen res2=derive(eq1,var2,contextptr);
	      if (!is_zero(res2))
		res2=_simplify(subst(eq1,var1,v1val,false,contextptr),contextptr);
	      if (!is_zero(res2))
		res2=_simplify(_solve(makesequence(symb_equal(res2,0),var2),contextptr),contextptr);
	      gen res3=derive(eq2,var2,contextptr);
	      if (!is_zero(res3)){
		res3 = subst(eq2,var1,v1val,false,contextptr);
		res3=_simplify(res3,contextptr);
	      }
	      if (!is_zero(res3)){
		res3=_solve(makesequence(symb_equal(res3,0),var2),contextptr);
		res3=_simplify(res3,contextptr);
		if (is_zero(res2))
		  res2=res3;
		else
		  res2=_intersect(makesequence(res2,res3),contextptr);
	      }
	      if (res2.type==_VECT){
		for (unsigned j=0;j<res2._VECTptr->size();++j)
		  V.push_back(makevecteur(v1,(*res2._VECTptr)[j]));
	      }
	    }
	    return V;
	  } // if res.type==_VECT
	}
      }
      if (v[1].type!=_VECT){
	vecteur arg1l=lvarx(equal2diff(arg1),v[1]);
	if (arg1l.size()>1){
	  if (arg1.type==_VECT){
	    arg1l.clear();
	    for (int i=0;i<int(arg1._VECTptr->size());++i){
	      gen tmp=(*arg1._VECTptr)[i];
	      arg1l.push_back(is_inequation(tmp)?tmp:powneg2invpow(tmp,contextptr));
	    }
	    arg1=gen(arg1l,arg1.subtype);
	  }
	  else {
	    if (!is_inequation(arg1))
	      arg1=powneg2invpow(arg1,contextptr);
	  }
	}
      }
      if (is_equal(arg1) && arg1._SYMBptr->feuille.type==_VECT){
	gen a1=arg1._SYMBptr->feuille[0];
	gen a2=arg1._SYMBptr->feuille[1];
	if (a1.is_symb_of_sommet(at_program) || a2.is_symb_of_sommet(at_program))
	  return gensizeerr(contextptr);
	if (a2==0 && a1.is_symb_of_sommet(at_plus) && a1._SYMBptr->feuille._VECTptr->size()==2){
	  a2=-a1._SYMBptr->feuille._VECTptr->back();
	  a1=a1._SYMBptr->feuille._VECTptr->front();
	}
	if (a1.is_symb_of_sommet(at_neg)){
	  a1=a1._SYMBptr->feuille;
	  a2=-a2;
	}
	// solve(simplify(surd((5/10),570)^(x))=(8/10))
	if (a2.type!=_VECT && !is_zero(a2) && (!lvarx(a1,v.back()).empty() || !lvarx(a2,v.back()).empty())){
	  vecteur lv=lvarx(makevecteur(a1,a2),v.back());
	  vecteur wpow=lop(lvar(lop(lv,at_pow)),at_pow);
	  vecteur w=mergevecteur(wpow,lop(lv,at_exp));
	  if (!wpow.empty() || w.size()>1){
	    gen a12=gcd(a1,a2,contextptr);
	    if (!lvarx(a12,v.back()).empty()){
	      gen res1=_solve(makesequence(symb_equal(a12,0),v.back()),contextptr);
	      gen res2=_solve(makesequence(symb_equal(ratnormal(a1/a12,contextptr),ratnormal(a2/a12,contextptr)),v.back()),contextptr);
	      res=mergevecteur(gen2vecteur(res1),gen2vecteur(res2));
              return postprocess_return(res,v[1],postprocess,contextptr);
	    }
	    if (w.size()>=3){
	      gen tst=tsimplify(_lin(a1/a2,contextptr),contextptr);
	      w=lvarx(tst,v.back());
	      if (w.size()==1){
                res=gen2vecteur(_solve(makesequence(tst-1,v.back()),contextptr));
		return postprocess_return(res,v[1],postprocess,contextptr);;
              }
	    }
	    gen a1arg,a2arg; 
	    bool a1log=is_log10(a1,a1arg),a2log=is_log10(a2,a2arg);
	    if (a1log && a2log){
	      a1=a1arg; a2=a2arg;
	    }
	    else {
	      if (a1log){
		a1=a1arg; a2=a2*symbolic(at_ln,10);
	      }
	      if (a2log) {
		a2=a2arg; a1=a1*symbolic(at_ln,10);
	      }
	    }
	    bool doit=false;
	    bool a1ln=a1.is_symb_of_sommet(at_ln),a2ln=a2.is_symb_of_sommet(at_ln);
	    if (a1ln && a2ln){
	      a1=a1._SYMBptr->feuille;
	      a2=a2._SYMBptr->feuille;
	      a1ln=a1.is_symb_of_sommet(at_ln);a2ln=a2.is_symb_of_sommet(at_ln);
	    }
	    if (a1ln || a2ln){
	    }
	    else {
	      if (is_positive(-a1,contextptr) && is_positive(-a2,contextptr)){
		doit=true;
		arg1=ln(simplify(-a1,contextptr),contextptr)-ln(simplify(-a2,contextptr),contextptr);
	      }	    
	      if (!is_positive(-a1*a2,contextptr)){
		doit=true;
		arg1=ln(simplify(a1,contextptr),contextptr)-ln(simplify(a2,contextptr),contextptr);
	      }
	    }
	    if (doit && lvarx(arg1,v.back()).size()>1){
              bool l=do_lnabs(contextptr); // check for solve(log10(((n)^(2))*(((x)/(n))^(log10(x))))=log10((x)^(2)),x);
              if (l){
                // change for solve(x^4=2^x) where a1=x^4 and we want to keep ln(abs(x))
                if (lidnt(arg1).size()==1)
                  l=0;
                if (l)
                  do_lnabs(false,contextptr);
              }
	      arg1=lnexpand(arg1,contextptr);
	      if (!lop(arg1,at_pow).empty()){ 
		arg1=lnexpand(a1-a2,contextptr);
		if (lvarx(arg1,v.back()).size()>1)
		  arg1=a1-a2;
	      }
              if (l)
                do_lnabs(l,contextptr);
	    }
	  }
	  w=lop(lv,at_exp);
	  if (w.size()>1){
	    arg1=lnexpand(ln(simplify(a1,contextptr),contextptr)-ln(simplify(a2,contextptr),contextptr),contextptr);
	    // check if ln trick worked, for example it does not for:
	    // f(x):=exp(x);g(x):=2*exp(x/2)-1; solve(f(x)=g(x))
	    if (!lop(arg1,at_exp).empty()) 
	      arg1=a1-a2;
	  }
	}
      }
#endif
    }
    arg1=apply(arg1,equal2diff);
    if (arg1.is_symb_of_sommet(at_program))
      return gensizeerr(contextptr);
    arg1=subst(arg1,undef,identificateur("undef_"),true,contextptr);
    vecteur _res=solve(arg1,v.back(),isolate_mode,contextptr);
    if (_res.empty() || _res.front().type==_STRNG || is_undef(_res) || _res.back().type==_STRNG)
      return _res;
    // quick check if back substitution returns undef
    const_iterateur it=_res.begin(),itend=_res.end();
    for (;it!=itend;++it){
      if (is_inequation(*it) || it->is_symb_of_sommet(at_ou) || it->is_symb_of_sommet(at_and) || has_op(*it,*at_LambertW)){
	res.push_back(*it);
	continue;
      }
      if (!check(v.back(),*it,v.back(),*it,contextptr))
	continue;
      gen tmp=subst(arg1,v.back(),*it,false,contextptr);
      tmp=eval(tmp,1,contextptr);
      if (!is_undef(tmp) && !is_inf(tmp)){
	vecteur itv=lop(*it,at_ln); // check added so that solve(2^x=8,x) returns 3 instead of ln(8)/ln(2)
	if (itv.size()>1)
	  res.push_back(simplify(*it,contextptr));
	else
	  res.push_back(*it);
      }
    }
    // if (is_fully_numeric(res))
    if (!v.empty() && v.back().type!=_VECT && 
	lidnt(evalf(res,1,contextptr)).empty()
	// lidnt(res).empty() && is_zero(im(res,contextptr),contextptr)
	)
      res=protect_sort(res,contextptr);
    if (v[1].type==_IDNT && !res.empty() && is_inequation(res.front())){
      // collect all inequations
      vecteur resineq,other;
      for (int i=0;i<res.size();++i){
        if (is_inequation(res[i]) || res[i].is_symb_of_sommet(at_and))
          resineq.push_back(res[i]);
        else
          other.push_back(res[i]);
      }
      if (resineq.size()>1){
        const context * cptr=contextptr; // contextptr?contextptr->globalcontextptr:contextptr;
        gen xval=assumeeval(v[1],cptr);
        gen a0(assumesymbolic(resineq[0],0,cptr));
        gen a0about=undef;
        // merge inequation intervals
        vecteur merged;
        if (a0==v[1]){
          a0about=a0._IDNTptr->eval(1,a0,cptr);
          if (a0about.type==_VECT){
            merged=*a0about._VECTptr;
            int i=1;
            for (;i<resineq.size();++i){
              restorepurge(xval,v[1],cptr);
              gen a1=assumesymbolic(resineq[i],0,cptr);
              gen a1about=undef;
              if (a1!=v[1])
                break;
              a1about=a1._IDNTptr->eval(1,a1,cptr);
              if (a1about.type!=_VECT)
                break;
              vecteur oldmerged(merged);
              realset_glue(oldmerged,*a1about._VECTptr,merged,contextptr);
            }
            if (i==resineq.size() && merged.size()>=3){
              gen i=merged[1],e=merged[2];
              resineq.clear();
              if (i.type==_VECT && e.type==_VECT){
                for (int j=0;j<i._VECTptr->size();++j){
                  gen cur=i[j];
                  gen m=cur[0],M=cur[1];
                  gen tmp1=symbolic(equalposcomp(*e._VECTptr,m)?at_superieur_strict:at_superieur_egal,makesequence(v[1],m));
                  gen tmp2=symbolic(equalposcomp(*e._VECTptr,M)?at_inferieur_strict:at_inferieur_egal,makesequence(v[1],M));
                  if (m==minus_inf)
                    resineq.push_back(tmp2);
                  else if (M==plus_inf)
                    resineq.push_back(tmp1);
                  else
                    resineq.push_back(symb_and(tmp1,tmp2));
                }
                res=mergevecteur(resineq,other);
              }
            }
          } // end a0about of type vect
        } // end a0==v[1]
        restorepurge(xval,v[1],cptr);
      } // end if resineq.size()>1
    }
    if (!postprocess)
      return gen(res,_LIST__VECT);
    gen vres=solvepostprocess(res,v[1],contextptr);
    return vres;
  }
  gen _solve(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (is_equal(args) && args._SYMBptr->feuille[1]==0 && args._SYMBptr->feuille[0].type==_INT_){
      int v=args._SYMBptr->feuille[0].val;
      if (v==16 || v==10 || v==8 || v==2){
	*logptr(contextptr) << "Integer format set to " << v << '\n';
	integer_format(v,contextptr);
	return vecteur(0);
      }
    }
    bool postprocess=xcas_mode(contextptr) || calc_mode(contextptr)==1;
    gen res=_solve_uncompressed(args,postprocess,contextptr);
    if (res.type==_VECT){
      vecteur v=*res._VECTptr;
      comprim(v);
      res=gen(v,res.subtype);
    }
    return res;
  }
  static const char _solve_s []="solve";
  static define_unary_function_eval_quoted (__solve,&_solve,_solve_s);
  define_unary_function_ptr5( at_solve ,alias_at_solve,&__solve,_QUOTE_ARGUMENTS,true);

  gen _realproot(const gen & e,GIAC_CONTEXT) {
    gen g=_proot(e,contextptr);
    if (g.type!=_VECT)
      return g;
    vecteur w;
    for (unsigned i=0;i<g._VECTptr->size();++i){
      gen tmp=(*g._VECTptr)[i];
      if (is_zero(im(tmp,contextptr),contextptr))
	w.push_back(tmp);
    }
    return w;
  }
  static const char _realproot_s []="realproot";
  static define_unary_function_eval (__realproot,&_realproot,_realproot_s);
  define_unary_function_ptr5( at_realproot ,alias_at_realproot,&__realproot,0,true);

  // bisection solver on a0,b0 with a sign reversal inside
  static vecteur bisection_solver_sr(const gen & equation,const gen & var,const gen & a0,const gen &b0,int & iszero,double faorig,double fborig,double eps,GIAC_CONTEXT){
    gen a=a0,b=b0;
    gen fa=subst(equation,var,a,false,contextptr);
    fa=eval(fa,1,contextptr);
    gen fb=subst(equation,var,b,false,contextptr);
    fb=eval(fb,1,contextptr);
    if (is_exactly_zero(fa)){
      iszero=1;
      return vecteur(1,a);
    }
    if (is_exactly_zero(fb)){
      iszero=1;
      return vecteur(1,b);
    }
    // sign change in [a,b]
    // number of steps
    gen n;
    n=ln(abs(b-a,contextptr),contextptr)-ln(max(abs(b,contextptr),abs(a,contextptr),contextptr),contextptr)+53;
    if (eps>0 && eps<1){ // solve (b-a)/2^n<eps 2^n>(b-a)/eps
      gen N=ln(abs((b-a)/eps,contextptr),contextptr);
      if (is_greater(n,N,contextptr))
	n=N;
    }    
    n=_floor(n/0.69,contextptr);
    for (int i=0;i<n.val;i++){
      gen c=(a+b)/2,fc;
#ifndef NO_STDEXCEPT
      try {
#endif
	fc=subst(equation,var,c,false,contextptr);
	fc=eval(fc,1,contextptr);
#ifndef NO_STDEXCEPT
      } catch (std::runtime_error & ){
	last_evaled_argptr(contextptr)=NULL;
	iszero=-1;
	return vecteur(0);
      }
#endif
      if (fc.type!=_DOUBLE_){
	iszero=-1;
	if (fa.type==_DOUBLE_){
	  b=c; fb=fc;
	  continue;
	}
	if (fb.type==_DOUBLE_){
	  a=c; fa=fc;
	  continue;
	}
	return vecteur(1,c);
      }
      if (is_exactly_zero(fc)){
	iszero=1;
	return vecteur(1,c);
      }
      if (fa._DOUBLE_val*fc._DOUBLE_val>0){
	a=c;
	fa=fc;
      }
      else {
	b=c;
	fb=fc;
      }
    }
    iszero=2;
    if (fa.type==_DOUBLE_ && fb.type==_DOUBLE_ && fabs(fa._DOUBLE_val*fb._DOUBLE_val/faorig/fborig)<1e-10)
      iszero=1;
    return vecteur(1,(a+b)/2);
  }

  vecteur bisection_solver(const gen & equation,const gen & var,const gen & a0,const gen &b0,int & iszero,double eps,GIAC_CONTEXT){
    bool onlyone=iszero==0;
    int nstep=gnuplot_pixels_per_eval;
    if (iszero>0)
      nstep=iszero;
    else {
      if (has_op(equation,*at_tan))
	nstep *= 16;
    }
    iszero=0;
    gen a(is_inf(a0)?gnuplot_xmin:evalf_double(a0,1,contextptr)),b(is_inf(b0)?gnuplot_xmax:evalf_double(b0,1,contextptr));
    if (is_strictly_greater(a,b,contextptr))
      swapgen(a,b);
    gen fa,fb,decal=(b-a)/nstep;
    if (is_zero(decal,contextptr))
      return vecteur(0);
    while (a+decal==a || b-decal==b){
      decal=2*decal;
    }
    vecteur res;
#ifndef NO_STDEXCEPT
    try {
#endif
      for (;is_strictly_greater(b,a,contextptr);){
	fa=subst(equation,var,a,false,contextptr);
	fa=eval(fa,1,contextptr);
	if (!is_zero(fa,contextptr))
	  break;
	if (onlyone)
	  return vecteur(1,a);
	if (is_exactly_zero(a))
	  res.push_back(a);
	a +=decal;
      }
      fb=fa;
      for (;is_strictly_greater(b,a,contextptr);){
	fb=subst(equation,var,b,false,contextptr);
	fb=eval(fb,1,contextptr);
	if (!is_zero(fb,contextptr))
	  break;
	if (onlyone)
	  return vecteur(1,b);
	if (is_exactly_zero(b))
	  res.push_back(b);
	b -=decal;
      }
#ifndef NO_STDEXCEPT
    } catch (std::runtime_error & ){
      last_evaled_argptr(contextptr)=NULL;
      iszero=-1;
      return vecteur(0);
    }
#endif
    int ntries=40;
    gen ab=(b-a)/ntries;
    if (fb.type!=_DOUBLE_ || is_undef(fb)){      
      for (int i=0;i<ntries;++i){
	b -= ab;
	fb=subst(equation,var,b,false,contextptr);
	fb=eval(fb,1,contextptr);
	if (fb.type==_DOUBLE_)
	  break;
      }
    }
    ab=(b-a)/ntries;
    if (fb.type==_DOUBLE_ && (fa.type!=_DOUBLE_ || is_undef(fa))){
      for (int i=0;i<ntries;++i){
	a += ab;
	fa=subst(equation,var,a,false,contextptr);
	fa=eval(fa,1,contextptr);
	if (fa.type==_DOUBLE_)
	  break;
      }
    }
    if (fa.type!=_DOUBLE_ || fb.type!=_DOUBLE_){
      iszero=-1;
      return vecteur(0);
    }
    double faorig=fa._DOUBLE_val,fborig=fb._DOUBLE_val;
    if (onlyone){
      if (fa._DOUBLE_val*fb._DOUBLE_val>0){
	bool test1=fa._DOUBLE_val>0;
	bool found=false;
	gen b0=b;
	// discretization of [a,b] searching a sign reversal
	for (int i=1;i<=6;i++){
	  int ntest=1 << (i-1);
	  gen decal=(b0-a)/gen(1 << i);
	  b=a+decal;
	  // double bd=b._DOUBLE_val;
	  decal=2*decal;
	  for (int j=0;j<ntest;j++,b+=decal){
#ifndef NO_STDEXCEPT
	    try {
#endif
	      fb=subst(equation,var,b,false,contextptr);
	      fb=eval(fb,1,contextptr);
#ifndef NO_STDEXCEPT
	    } catch (std::runtime_error & ){
	      last_evaled_argptr(contextptr)=NULL;
	      iszero=-1;
	      return vecteur(0);
	    }
#endif
	    if (fb.type!=_DOUBLE_){
	      iszero=-1;
	      return vecteur(0);
	    }
	    double fbd=fb._DOUBLE_val;
	    bool test2=fbd>0;
	    if (test1 ^ test2){
	      found=true;
	      break;
	    }
	  }
	  if (found)
	    break;
	}
	if (!found){
	  iszero=-2;
	  return vecteur(0);
	}
      }
      return bisection_solver_sr(equation,var,a,b,iszero,faorig,fborig,eps,contextptr);
    }
    // we are searching many zeros in this interval, cutting it in small intervals
    // and searching a sign reversal in each
    decal=(b-a)/nstep;
    b=a+decal;
    for (int i=0;i<nstep;++i, a=b, fa=fb,b+=decal){
      if (calc_mode(contextptr)==1 && res.size()>=16)
        return res;
#ifndef NO_STDEXCEPT
      try {
#endif
	fb=subst(equation,var,b,false,contextptr);
	fb=eval(fb,1,contextptr);
#ifndef NO_STDEXCEPT
      } catch (std::runtime_error & ){
	last_evaled_argptr(contextptr)=NULL;
	continue;
      }
#endif
      if (fb.type!=_DOUBLE_)
	continue;
      if (fb._DOUBLE_val==0){
	res.push_back(b);
	continue;
      }
      if (fa._DOUBLE_val*fb._DOUBLE_val>0)
	continue;
      vecteur addres=bisection_solver_sr(equation,var,a,b,iszero,faorig,fborig,eps,contextptr);
      if (iszero==1)
	res=mergevecteur(res,addres);
    }
    comprim(res);
    return res;
  }

  // also sets iszero to -2 if endpoints have same sign, -1 if err or undef
  // 1 if zero found, 2 if sign reversal (no undef),
  // set iszero to 0 on entry if only one root
  // set to -1 or positive if you want many sign reversals 
  // -1 means no step specified, positive means nstep specified
  vecteur bisection_solver(const gen & equation,const gen & var,const gen & a0,const gen &b0,int & iszero,GIAC_CONTEXT){
    return bisection_solver(equation,var,a0,b0,iszero,0.0,contextptr);
  }

  static void set_nearest_first(const gen & guess,vecteur & res,GIAC_CONTEXT){
    int s=int(res.size());
    if (s<2)
      return;
    int pos=0,i;
    gen minabs=evalf_double(abs(res[0]-guess,contextptr),1,contextptr);
    for (i=1;i<s;++i){
      gen curabs=evalf_double(abs(res[i]-guess,contextptr),1,contextptr);
      if (is_strictly_greater(minabs,curabs,contextptr)){
	minabs=curabs;
	pos=i;
      }
    }
    if (pos){
      minabs=res[0];
      res[0]=res[pos];
      res[pos]=minabs;
    }
  }

  bool is_idnt_function38(const gen & g){
    if (g.type!=_IDNT)
      return false;
    const char * ch = g._IDNTptr->id_name;
    if (strlen(ch)==2 && ch[1]>='0' && ch[1]<='9'){
      switch (ch[0]){
      case 'F': case 'X': case 'Y': case 'R':
	return true;
      }
    }
    return false;
  }

  void lidnt_solve(const gen &g,vecteur & res){
    vecteur v=lidnt(g);
    for (unsigned i=0;i<v.size();++i){
      if (!is_idnt_function38(v[i]))
	res.push_back(v[i]);
    }
  }

  vecteur lidnt_solve(const gen & g){
    vecteur res;
    lidnt_solve(g,res);
    return res;
  }

  void lidnt_function38(const gen &g,vecteur & res){
    vecteur v=lidnt(g);
    for (unsigned i=0;i<v.size();++i){
      if (is_idnt_function38(v[i]))
	res.push_back(v[i]);
    }
  }

  vecteur lidnt_function38(const gen & g){
    vecteur res;
    lidnt_function38(g,res);
    return res;
  }

  // Find zero or extrema of equation for variable near guess in real mode
  // For polynomial input, returns all zeros or extrema
  // On entry type=0 for zeros, =1 for extrema
  //  guess might be a single value or vecteur with 2 values (an interval)
  //  bisection is used if guess is an interval
  //  if guess is a single value, guess is checked to be in [xmin,xmax]
  // returns 0 if zero(s) were found, 1 if extrema found, 2 if sign reversal found
  vecteur solve_zero_extremum(const gen & equation0,const gen & variable0,const gen & guess,double xmin, double xmax,int & type,GIAC_CONTEXT){
    if (variable0.type!=_IDNT)
      return vecteur(1,gentypeerr(contextptr));
    vecteur l0(1,variable0);
    lidnt(equation0,l0,false);
    vecteur l1=gen2vecteur(eval(l0,1,contextptr));
    identificateur id_solve("aspen_x");
    gen variable(id_solve);
    l1.front()=variable;
    gen eq0=subst(equation0,l0,l1,false,contextptr),eq;
    // ofstream of("log"); of << equation0 << '\n' << eq0 << '\n' << l0 << '\n' << l1 << '\n'; of.close();
    eq0=remove_equal(eval(eq0,1,contextptr));
    vecteur res;
    if (is_undef(eq0) || is_inf(eq0)){
      type=-2;
      return res;
    }
    gen a,b;
    if (is_linear_wrt(eq0,variable,a,b,contextptr)){
      a=ratnormal(a,contextptr);
      if (is_zero(a,contextptr)){
	type=-1;
	return 0;
      }
      type=0;
      a=-b/a;
      b=im(a,contextptr);
      if (is_zero(b,contextptr))
	res=vecteur(1,re(a,contextptr));
      else
	res=vecteur(1,undef);
      return res;
    }
    bool interval=false;
    a=xmin;b=xmax;
    if (guess.type==_VECT){
      if (guess._VECTptr->size()!=2)
	return vecteur(1,gendimerr(contextptr));
      // Find in [a,b]
      interval=true;
      a=guess._VECTptr->front();
      b=guess._VECTptr->back();
    }
    else {
      gen tmp=evalf_double(guess,1,contextptr);
      if (tmp.type==_DOUBLE_){
	if (tmp._DOUBLE_val>xmax)
	  b=tmp;
	if (tmp._DOUBLE_val<xmin)
	  a=tmp;
      }
    }
    // Check if equation is smooth, if not, find an interval for solving
#ifndef NO_STDEXCEPT
    try {
#endif
      eq=derive(eq0,variable,contextptr); 
      if (is_undef(eq))
	interval=true;
#ifndef NO_STDEXCEPT
    } catch (std::runtime_error &){
      last_evaled_argptr(contextptr)=NULL;
      eq=undef;
      interval=true;
    }
#endif
    // ofstream of("log"); of << eq << " " << diffeq << '\n'; of.close();
    if (is_zero(ratnormal(eq,contextptr),contextptr)){
      type=-1;
      return res;
    }
    if (type==0){ // Find zero
      if (interval){
	int iszero=0;
	res=bisection_solver(eq0,*variable._IDNTptr,a,b,iszero,contextptr);
	if (iszero<=0)
	  res.clear();
	if (iszero==2)
	  type=2;
      }
      else {
#ifndef NO_STDEXCEPT
	try {
#endif
	  if (lvar(eq0)==vecteur(1,variable)){
	    res=solve(eq0,*variable._IDNTptr,0,contextptr);
	    set_nearest_first(guess,res,contextptr);
	    if (res.empty())
	      type=1;
	  }
#ifndef NO_STDEXCEPT
	}
	catch (std::runtime_error & ){
	  last_evaled_argptr(contextptr)=NULL;
	  res.clear();
	}
#endif
      }
      if (!res.empty() && is_undef(res.front()))
	res.clear();
      if (type==0 && res.empty()){
	gen sol=_fsolve(gen(makevecteur(evalf(eq0,1,contextptr),symbolic(at_equal,makesequence(variable,guess))),_SEQ__VECT),contextptr);
	sol=evalf2bcd_nock(sol,1,contextptr);
	if (sol.type==_VECT){
	  res=*sol._VECTptr;
	  set_nearest_first(guess,res,contextptr);
	}
	else {
	  if (sol.type==_FLOAT_)
	    res=vecteur(1,sol);
	}
      }
      if (!res.empty() && !is_undef(res))
	return *eval(res,1,contextptr)._VECTptr;
    }
    if (type==0)
      type=1;
    if (type==1 && !is_undef(eq)){ // Find extremum
      if (interval){
	int iszero=0;
	res=bisection_solver(eq,variable,a,b,iszero,contextptr);
	if (iszero<=0)
	  res.clear();
      }
      else {
#ifndef NO_STDEXCEPT
	try {
#endif
	  if (lvar(eq)==vecteur(1,variable)){
	    res=solve(eq,*variable._IDNTptr,0,contextptr);
	    if (!res.empty() && is_undef(res.front()))
	      res.clear();
	    if (res.empty())
	      type=2;
	    else
	      set_nearest_first(guess,res,contextptr);
	  }
#ifndef NO_STDEXCEPT
	}
	catch (std::runtime_error & ){
	  last_evaled_argptr(contextptr)=NULL;
	  res.clear();
	}
#endif
      }
      if (!res.empty() && is_undef(res.front()))
	res.clear();
      if (type==1 && res.empty()){
	gen sol=_fsolve(gen(makevecteur(evalf(eq,1,contextptr),symbolic(at_equal,makesequence(variable,guess))),_SEQ__VECT),contextptr);
	sol=evalf2bcd_nock(sol,1,contextptr);
	if (sol.type==_VECT){
	  res=*sol._VECTptr;
	  set_nearest_first(guess,res,contextptr);
	}
	else {
	  if (sol.type==_FLOAT_)
	    res=vecteur(1,sol);
	}
      }
      if (!res.empty() && !is_undef(res))
	return *eval(res,1,contextptr)._VECTptr;
    }
    type=2; // Find singularities
    res=find_singularities(eq0,*variable._IDNTptr,0,contextptr);
    if (res.empty()) type=3;
    return *eval(res,1,contextptr)._VECTptr;
  }
  vecteur solve_zero_extremum(const gen & equation0,const gen & variable,const gen & guess,int & type,GIAC_CONTEXT){
#ifndef NO_STDEXCEPT
    try {
#endif
      return solve_zero_extremum(equation0,variable,guess,gnuplot_xmin,gnuplot_xmax,type,contextptr);
#ifndef NO_STDEXCEPT
    } catch(std::runtime_error & ){
      last_evaled_argptr(contextptr)=NULL;
      type=-2;
      return vecteur(1,undef);
    }
#endif
  }
  gen _solve_zero_extremum(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    vecteur v(solvepreprocess(args,complex_mode(contextptr),contextptr));
    int s=int(v.size());
    if (s<3 || v[1].type!=_IDNT)
      return gensizeerr(contextptr);
    int type=0;
    if (s==4 && v[3].type==_INT_)
      type=v[3].val;
    vecteur res=solve_zero_extremum(v[0],v[1],v[2],type,contextptr);
    return makevecteur(type,res);
  }
  static const char _solve_zero_extremum_s []="solve_zero_extremum";
  static define_unary_function_eval_quoted (__solve_zero_extremum,&_solve_zero_extremum,_solve_zero_extremum_s);
  define_unary_function_ptr5( at_solve_zero_extremum ,alias_at_solve_zero_extremum,&__solve_zero_extremum,_QUOTE_ARGUMENTS,true);

  double nan(){
    double x=0.0;
    return 0.0/x;
  }
#ifdef HAVE_LIBGSL
  // p should point a vector with elements the expression f(x) and x
  // OR with f(x), f'(x) and x
  static double my_f (double x0, void * p) {
    gen & params = * ((gen *)p) ;
#ifdef DEBUG_SUPPORT
    if ( (params.type!=_VECT) || (params._VECTptr->size()<2))
      setsizeerr(gettext("solve.cc/my_f"));
#endif	// DEBUG_SUPPORT
    gen & f=params._VECTptr->front();
    gen & x=params._VECTptr->back();
    gen res=evalf(subst(f,x,x0,false,context0),1,context0);
    if (res.type==_REAL)
      res=evalf_double(res,1,context0);
#ifdef DEBUG_SUPPORT
    if (res.type>_IDNT)
      setsizeerr();
#endif
    if (res.type!=_DOUBLE_)
      return nan();
    else
      return res._DOUBLE_val;
  }

  static double my_df (double x0, void * p) {
    gen & params = * ((gen *)p) ;
#ifdef DEBUG_SUPPORT
    if ( (params.type!=_VECT) || (params._VECTptr->size()!=3))
      setsizeerr(gettext("solve.cc/my_df"));
#endif	// DEBUG_SUPPORT
    gen & f=(*params._VECTptr)[1];
    gen & x=params._VECTptr->back();
    gen res=evalf_double(subst(f,x,x0,false,context0),1,context0);
#ifdef DEBUG_SUPPORT
    if (res.type>_IDNT)
      setsizeerr();
#endif
    if (res.type!=_DOUBLE_)
      return nan();
    else
      return res._DOUBLE_val;
  }

  static void my_fdf (double x0, void * p,double * fx,double * dfx) {
    gen & params = * ((gen *)p) ;
#ifdef DEBUG_SUPPORT
    if ( (params.type!=_VECT) || (params._VECTptr->size()!=3))
      setsizeerr(gettext("solve.cc/my_fdf"));
#endif	// DEBUG_SUPPORT
    gen & f=params._VECTptr->front();
    gen & df=(*params._VECTptr)[1];
    gen & x=params._VECTptr->back();
    gen res=evalf_double(subst(f,x,x0,false,context0),1,context0);
    if (res.type!=_DOUBLE_)
      *fx=nan();
    else
      *fx=res._DOUBLE_val;
    res=evalf_double(subst(df,x,x0,false,context0),1,context0);
    if (res.type!=_DOUBLE_)
      *dfx=nan();
    else
      *dfx=res._DOUBLE_val;
  }

  static int my_F (const gsl_vector * x0, void * p,gsl_vector * F) {
    gen & params = * ((gen *)p) ;
#ifdef DEBUG_SUPPORT
    if ( (params.type!=_VECT) || (params._VECTptr->size()<2))
      setsizeerr(gettext("solve.cc/my_F"));
#endif	// DEBUG_SUPPORT
    gen & f=params._VECTptr->front();
    gen & x=params._VECTptr->back();
    gen res=evalf_double(subst(f,x,gsl_vector2vecteur(x0),false,context0),1,context0);
    if (res.type!=_VECT)
      return !GSL_SUCCESS;
    return vecteur2gsl_vector(*res._VECTptr,F,context0);
  }

  static int my_dF (const gsl_vector *x0, void * p,gsl_matrix * J) {
    gen & params = * ((gen *)p) ;
#ifdef DEBUG_SUPPORT
    if ( (params.type!=_VECT) || (params._VECTptr->size()!=3))
      setsizeerr(gettext("solve.cc/my_dF"));
#endif	// DEBUG_SUPPORT
    gen & f=(*params._VECTptr)[1];
    gen & x=params._VECTptr->back();
    gen res=evalf_double(subst(f,x,gsl_vector2vecteur(x0),false,context0),1,context0);
    if (res.type!=_VECT)
      return !GSL_SUCCESS;
    else
      return matrice2gsl_matrix(*res._VECTptr,J,context0);
  }

  static int my_FdF (const gsl_vector * x0, void * p,gsl_vector * fx,gsl_matrix * dfx) {
    gen & params = * ((gen *)p) ;
#ifdef DEBUG_SUPPORT
    if ( (params.type!=_VECT) || (params._VECTptr->size()!=3))
      setsizeerr(gettext("solve.cc/my_FdF"));
#endif	// DEBUG_SUPPORT
    gen & f=params._VECTptr->front();
    gen & df=(*params._VECTptr)[1];
    gen & x=params._VECTptr->back();
    gen g0=gsl_vector2vecteur(x0);
    gen res=evalf_double(subst(f,x,g0,false,context0),1,context0);
    if (res.type!=_VECT)
      return !GSL_SUCCESS;
    int ires=vecteur2gsl_vector(*res._VECTptr,fx,context0);
    if (ires!=GSL_SUCCESS)
      return !GSL_SUCCESS;
    res=evalf_double(subst(df,x,g0,false,context0),1,context0);
    if (res.type!=_VECT)
      return !GSL_SUCCESS;
    return matrice2gsl_matrix(*res._VECTptr,dfx,context0);
  }

  gen msolve(const gen & f,const vecteur & vars,const vecteur & g,int method,double eps,GIAC_CONTEXT){
    vecteur guess(g);
    bool with_derivative=false;
    int dim=vars.size();
    switch (method){
    case _NEWTONJ_SOLVER: case _HYBRIDSJ_SOLVER: case _HYBRIDJ_SOLVER:
      with_derivative=true;
      break;
    case _DNEWTON_SOLVER: case _HYBRIDS_SOLVER: case _HYBRID_SOLVER:
      with_derivative=false;
      break;
    }
    if (with_derivative){
      gen difff=derive(f,vars,contextptr);
      if (is_undef(difff) || difff.type!=_VECT)
	return vecteur(vars.size(),undef);
      gen params(makevecteur(f,mtran(*difff._VECTptr),vars));
      gsl_multiroot_function_fdf FDF;
      FDF.f=&my_F;
      FDF.df=&my_dF;
      FDF.fdf=&my_FdF;
      FDF.n=dim;
      FDF.params=&params;
      const gsl_multiroot_fdfsolver_type * T=0;
      switch (method){
      case _NEWTONJ_SOLVER: 
	T=gsl_multiroot_fdfsolver_gnewton;
	break;
      case _HYBRIDSJ_SOLVER:
	T=gsl_multiroot_fdfsolver_hybridsj;
	break;
      case _HYBRIDJ_SOLVER:
	T=gsl_multiroot_fdfsolver_hybridj;
	break;
      }
      gsl_multiroot_fdfsolver * s= gsl_multiroot_fdfsolver_alloc (T, dim);
      gsl_vector * X=vecteur2gsl_vector(guess,contextptr);
      gsl_multiroot_fdfsolver_set (s, &FDF, X);
      int maxiter=SOLVER_MAX_ITERATE,res=0;
      vecteur oldguess;
      for (;maxiter;--maxiter){
	oldguess=guess;
	res=gsl_multiroot_fdfsolver_iterate(s);
	if ( (res==GSL_EBADFUNC) || (res==GSL_ENOPROG) )
	  break;
	guess=gsl_vector2vecteur(gsl_multiroot_fdfsolver_root(s));
	if (is_strictly_greater(eps,abs(guess-oldguess,contextptr),contextptr))
	  break;
      }
      gsl_multiroot_fdfsolver_free(s);
      if ( (res==GSL_EBADFUNC) || (res==GSL_ENOPROG) )
	return vecteur(dim,gensizeerr(contextptr));
      return guess;
    }
    else {
      gen params(makevecteur(f,vars));
      gsl_multiroot_function F;
      F.f=&my_F;
      F.n=dim;
      F.params=&params;
      const gsl_multiroot_fsolver_type * T=0;
      switch (method){
      case _DNEWTON_SOLVER: 
	T=gsl_multiroot_fsolver_dnewton;
	break;
      case _HYBRIDS_SOLVER:
	T=gsl_multiroot_fsolver_hybrids;
	break;
      case _HYBRID_SOLVER:
	T=gsl_multiroot_fsolver_hybrid;
	break;
      }
      gsl_multiroot_fsolver * s= gsl_multiroot_fsolver_alloc (T, dim);
      gsl_vector * X=vecteur2gsl_vector(guess,contextptr);
      gsl_multiroot_fsolver_set (s, &F, X);
      int maxiter=SOLVER_MAX_ITERATE,res=0;
      vecteur oldguess;
      for (;maxiter;--maxiter){
	oldguess=guess;
	res=gsl_multiroot_fsolver_iterate(s);
	if ( (res==GSL_EBADFUNC) || (res==GSL_ENOPROG) )
	  break;
	guess=gsl_vector2vecteur(gsl_multiroot_fsolver_root(s));
	if (is_strictly_greater(eps,abs(guess-oldguess,contextptr),contextptr))
	  break;
      }
      gsl_multiroot_fsolver_free(s);
      if ( (res==GSL_EBADFUNC) || (res==GSL_ENOPROG) )
	return vecteur(1,gensizeerr(contextptr));
      return guess;
    }
  }
#endif // HAVE_LIBGSL

  // fsolve(expr,var[,interval/guess,method])
  gen _fsolve(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (calc_mode(contextptr)==1 && args.type!=_VECT)
      return _fsolve(makesequence(args,ggb_var(args)),contextptr);
    vecteur v(plotpreprocess(args,contextptr));
    gen res=undef;
    res=in_fsolve(v,contextptr);
    if (calc_mode(contextptr)!=1)
      return res;
    // ggb always in a list
    if (res.type!=_VECT)
      res=vecteur(1,res);
    return res;
  }

  // same as lidnt but removes mute variable in int/sum/fsolve
  // FIXME: Home functions names should be removed from the list
  vecteur true_lidnt(const gen & g){
    vecteur v=lvar(g);
    vecteur w;
    for (unsigned i=0;i<v.size();++i){
      if (v[i].is_symb_of_sommet(at_fsolve) ||
	  v[i].is_symb_of_sommet(at_integrate) ||
	  v[i].is_symb_of_sommet(at_sum) ||
	  v[i].is_symb_of_sommet(at_product)
	  ){
	if (v[i]._SYMBptr->feuille.type!=_VECT || v[i]._SYMBptr->feuille._VECTptr->size()<2)
	  continue;
	gen v1=(*v[i]._SYMBptr->feuille._VECTptr)[1];
	if (v1.is_symb_of_sommet(at_equal) && v1._SYMBptr->feuille.type==_VECT && !v1._SYMBptr->feuille._VECTptr->empty())
	  v1=v1._SYMBptr->feuille._VECTptr->front();
	vecteur tmp=true_lidnt(v[i]._SYMBptr->feuille);
	if (int pos=equalposcomp(tmp,v1))
	  tmp.erase(tmp.begin()+pos-1);
	w=mergevecteur(w,tmp);
	continue;
      }
      w.push_back(v[i]);
    }
    v=lidnt(w);
    w.clear();
    for (unsigned i=0;i<v.size();++i){
      if (!is_inf(v[i]) && !is_undef(v[i]))
	w.push_back(v[i]);  
    }
    return w;
  }

  gen in_fsolve(vecteur & v,GIAC_CONTEXT){
    if (is_undef(v))
      return v;  
    bool interv=false;
    double gsl_eps=abs_calc_mode(contextptr)==38?1e-5:epsilon(contextptr);
    int s=int(v.size());
    if (s && v.back()==at_interval){
      --s;
      v.pop_back();
      interv=true;
    }
    if (s<2)
      return gentoofewargs("fsolve");
    if (v[0].is_symb_of_sommet(at_program))
      swapgen(v[0],v[1]);
    if (v[1].is_symb_of_sommet(at_program)){ // scilab-like syntax
      gen var(identificateur("fsolve_tmpvar"));
      v.insert(v.begin()+1,var);
      swapgen(v[0],v[2]);
      v[0]=v[0](v[1],contextptr);
      return in_fsolve(v,contextptr);
    }
    gen v0=remove_equal(v[0]);
    vecteur I1(lidnt(v[1]));
    vecteur I0(true_lidnt(v0)); // should remove embedded fsolve/sum/int
    I0=lidnt(makevecteur(evalf(I0,1,contextptr),I1));
    if (_sort(I0,contextptr)!=_sort(I1,contextptr))
      return symbolic(at_fsolve,gen(v,_SEQ__VECT));
    int evalf_after=interv?3:1;
    if (s>=2 && v0.type==_VECT && v[1].type==_VECT && !v[1]._VECTptr->empty()){
      // check v[1]
      vecteur w=*v[1]._VECTptr;
      unsigned i=0;
      if (w.front().is_symb_of_sommet(at_equal)){
	vecteur v1,v2;
	for (;i<w.size();++i){
	  if (w[i].is_symb_of_sommet(at_equal)){
	    v1.push_back(w[i]._SYMBptr->feuille[0]);
	    v2.push_back(w[i]._SYMBptr->feuille[1]);
	  }
	  else {
	    v1.push_back(w[i]);
	    v2.push_back(0);
	  }
	}
	v[1]=v1;
	v.insert(v.begin()+2,v2);
	return in_fsolve(v,contextptr);
      }
      for (;i<w.size();++i){
	if (w[i].type==_IDNT)
	  continue;
	if (!w[i].is_symb_of_sommet(at_at))
	  break;
      }
      if (i!=w.size())
	return gensizeerr(gettext("fsolve([equations],[variables],[guesses])"));
      if (s==2 && _sort(lvar(v0),contextptr)==_sort(v[1],contextptr)){
	vecteur _res=gsolve(*v0._VECTptr,*v[1]._VECTptr,complex_mode(contextptr),evalf_after,contextptr);
	const_iterateur it=_res.begin(),itend=_res.end();
	vecteur res;
	for (;it!=itend;++it){
	  gen tmp=subst(*v0._VECTptr,*v[1]._VECTptr,*it,false,contextptr);
	  tmp=eval(tmp,1,contextptr);
	  if (!is_undef(tmp) && !is_inf(tmp)){
	    res.push_back(*it);
	  }
	}
	return evalf(res,1,contextptr);
      }
    }
    if (s==2 && v[1].type==_IDNT){ 
      // no initial guess, check for poly-like equation
      vecteur lv(lvar(v0));
      if (contextptr && contextptr->quoted_global_vars){
	contextptr->quoted_global_vars->push_back(lv[1]);
	lv=lvar(evalf(lv,1,contextptr)); // this should not evalf v[1], otherwise x:=10; solve(x^(3/2)=3.,x); will fail
	contextptr->quoted_global_vars->pop_back();
      } else
	lv=lvar(evalf(lv,1,contextptr)); 
      int lvs=int(lv.size());
      bool poly=true;
      for (unsigned i=0;i<lv.size();++i){
	if (lv[i]==v[1] || lv[i]==cst_pi || lv[i]==cst_euler_gamma)
	  continue;
	poly=false;
	break;
      }
      if (poly){
	gen tmp=_e2r(makesequence(v0,v[1]),contextptr),tmp1=tmp;
	if (tmp.type==_FRAC)
	  tmp1=tmp=tmp._FRACptr->num;
	tmp=evalf(tmp,eval_level(contextptr),contextptr);
	if (tmp.type==_VECT && tmp._VECTptr->size()<1024){
	  // call realroot? this would be more accurate
	  gen res=complex_mode(contextptr)?proot(*tmp._VECTptr,epsilon(contextptr),contextptr):(lvar(tmp1).empty()?_realroot(gen(makevecteur(tmp1,epsilon(contextptr),at_evalf),_SEQ__VECT),contextptr):real_proot(*tmp._VECTptr,epsilon(contextptr),contextptr));
	  if (res.type==_VECT && res._VECTptr->size()==1)
	    return res._VECTptr->front();
	  return res;
	}
      }
      gen v0orig(v0); bool testpi=false;
      if (lvs>1){
	v0=halftan_hyp2exp(v0,contextptr);
	testpi=true;
      }
      lv=lvar(v0);
      lvs=int(lv.size());
      if (lvs==1 && lv[0].type==_SYMB && lv[0]._SYMBptr->feuille.type!=_VECT){
	int pos=equalposcomp(solve_fcns_tab,lv[0]._SYMBptr->sommet);
	if (pos){
	  gen tmp=_e2r(makesequence(v0,lv[0]),contextptr);
	  if (tmp.type==_FRAC)
	    tmp=tmp._FRACptr->num;
	  tmp=evalf(tmp,eval_level(contextptr),contextptr);
	  if (tmp.type==_VECT){
	    vecteur res0=complex_mode(contextptr)?proot(*tmp._VECTptr,epsilon(contextptr),contextptr):real_proot(*tmp._VECTptr,epsilon(contextptr),contextptr);
	    vecteur res;
	    if (testpi && is_zero(subst(v0orig,v[1],M_PI,false,contextptr),contextptr))
	      res.push_back(M_PI);
	    const_iterateur it=res0.begin(),itend=res0.end();
	    for (;it!=itend;++it){
	      vecteur res0val=gen2vecteur(isolate_fcns[pos-1](*it,complex_mode(contextptr),contextptr));
	      const_iterateur jt=res0val.begin(),jtend=res0val.end();
	      for (;jt!=jtend;++jt){
		gen fs=_fsolve(gen(makevecteur(lv[0]._SYMBptr->feuille-*jt,v[1]),_SEQ__VECT),contextptr);
		if (fs.type==_VECT)
		  res=mergevecteur(res,*fs._VECTptr);
		else
		  res.push_back(fs);
	      }
	    }
	    return res;
	  }
	}
      } // end lvs==1 etc.
      // no guess, try to root by bisection over a large interval of R 
      // x=tan(t) change of var
      if (1
	  //abs_calc_mode(contextptr)==38
	  ){
	*logptr(contextptr) << gettext("Solving by bisection with change of variable x=tan(t) and t=-1.57..1.57. Try fsolve(equation,x=guess) for iterative solver or fsolve(equation,x=xmin..xmax) for bisection.") << "\nEquation: " << v[0] << '\n';
	gen eq=subst(v[0],v[1],tan(v[1],contextptr),false,contextptr);
  //grad
	vecteur v_=makevecteur(eq,symb_equal(v[1],angle_radian(contextptr)?symb_interval(-1.57,1.57):(angle_degree(contextptr)?symb_interval(-89.97,89.97):symb_interval(-99.97,99.97))));
	gen res=in_fsolve(v_,contextptr);
	if (is_undef(res))
	  return res;
	if (res.type==_VECT && res._VECTptr->empty()){
	  *logptr(contextptr) << gettext("No solution found by bisection. Trying iterative method starting at 0") << '\n';
	  v_=makevecteur(v[0],v[1],0);
	  return in_fsolve(v_,contextptr);
	}
	return tan(res,contextptr);
      }
      *logptr(contextptr) << gettext("Solving with initial guess 0. Try fsolve(equation,x=guess) for iterative solver or fsolve(equation,x=xmin..xmax) for bisection.") << '\n';
    }
    gen gguess;
    if (v[1].type==_VECT && !v[1]._VECTptr->empty() && is_equal(v[1]._VECTptr->front())){
      vecteur v1=*v[1]._VECTptr;
      vecteur vguess(v1.size());
      for (unsigned i=0;i<v1.size();++i){
	if (is_equal(v1[i])){
	  vguess[i]=v1[i]._SYMBptr->feuille[1];
	  v1[i]=v1[i]._SYMBptr->feuille[0];
	}
      }
      v[1]=gen(v1);
      gguess=vguess;
    }
    if (is_equal(v[1])){
      gguess=v[1]._SYMBptr->feuille[1];
      v[1]=v[1]._SYMBptr->feuille[0];
      v.insert(v.begin()+2,gguess);
      ++s;
    }
    if (s>=3 && (v[2].type!=_INT_ || v[2].subtype!=_INT_SOLVER))
      gguess=v[2];
    if (is_equal(gguess))
      return gensizeerr(contextptr);
    if (gguess.type==_VECT && gguess._VECTptr->size()!=2 && v[1].type==_IDNT){
      int nvar=int(gguess._VECTptr->size());
      vecteur tmp(nvar);
      vecteur chk=lop(v[0],at_of);
      bool at=true;
      for (unsigned i=0;at && i<chk.size();++i){
	if (chk[i][1]==v[1])
	  at=false;
      }
      for (int i=0;i<nvar;++i)
	tmp[i]=at?symbolic(at_at,makesequence(v[1],i)):symb_of(v[1],i+1);
      v[1]=tmp;
    }
    if (s>=4 && v[3].type==_IDNT)
      *logptr(contextptr) << gettext("Unknown option ") << v[3] << "\n";
    if (gguess.is_symb_of_sommet(at_interval) && (s<4 || v[3].subtype!=_INT_PLOT)){
      int iszero=-1;
      gen a=gguess._SYMBptr->feuille[0],b=gguess._SYMBptr->feuille[1];
      if (s>=4){
	if (is_integer(v[3]))
	  iszero=v[3].val;
	if (is_equal(v[3])){
	  gen v30=v[3]._SYMBptr->feuille[0];
	  gen v31=v[3]._SYMBptr->feuille[1];
	  if (v30.subtype==_INT_PLOT && v30==_NSTEP)
	    v30=v31;
	  if (v30.subtype==_INT_PLOT && (v30==_XSTEP || v30==_TSTEP))
	    v30=_floor((b-a)/v31,contextptr);
	  if (v30.type==_INT_ && v30.val>0)
	    iszero=v30.val;
	}
      }
      // optional tolerance parameter for bisection
      double eps=0;
      if (s>3){
	for (int i=3;i<s;++i){
	  if (v[i].type==_DOUBLE_ && v[i]._DOUBLE_val<1 && v[i]._DOUBLE_val>0){
	    eps=v[i]._DOUBLE_val;
	    break;
	  }
	}
      }
      return bisection_solver(v0,v[1],a,b,iszero,eps,contextptr);
    }
    if (gguess.type==_VECT && gguess._VECTptr->size()>1){
      for (unsigned i=0;i<gguess._VECTptr->size();++i){
	gen tmp=evalf_double((*gguess._VECTptr)[i],1,contextptr);
	if (tmp.type!=_DOUBLE_ && tmp.type!=_CPLX)
	  return gensizeerr("Bad guess "+gguess.print(contextptr));
      }
    }
    if (v0.type==_VECT){
      if ( (v[1].type==_VECT && v0._VECTptr->size()!=v[1]._VECTptr->size())
	   || (v[1].type!=_VECT && v[1]._VECTptr->size()!=1))
	return gendimerr(contextptr);
    }
    // check method
    int method=_NEWTON_SOLVER;
    //int method=0;
    if ( (s>=5) && (v[4].type==_DOUBLE_) )
      gsl_eps=v[4]._DOUBLE_val;
    if ( (s>=4) && (v[3].type==_INT_) )
      method=v[3].val;
    if (v[1].type==_VECT){
      int dim=int(v[1]._VECTptr->size());
      if (!dim)
	return gensizeerr(contextptr);
      if (s>=3){
	if (gguess.type!=_VECT)
	  return gensizeerr(contextptr);
	if (gguess._VECTptr->size()!=unsigned(dim))
	  return gensizeerr(contextptr);
      }
      else {
	gguess=vecteur(dim);
	gguess[0]=(gnuplot_xmin+gnuplot_xmax)/2;
	if (dim>1)
	  gguess[1]=(gnuplot_ymin+gnuplot_ymax)/2;
	if (dim>2)
	  gguess[2]=(gnuplot_zmin+gnuplot_zmax)/2;
	if (dim>3)
	  gguess[3]=(gnuplot_tmin+gnuplot_tmax)/2;
      }
#ifdef HAVE_LIBGSL
      if (method!=_NEWTON_SOLVER)
	return msolve(v0,*v[1]._VECTptr,*gguess._VECTptr,method,gsl_eps,contextptr);
#endif
    }
#ifndef HAVE_LIBGSL
    if (v[1].type!=_VECT && gguess.type==_VECT && gguess._VECTptr->size()==2){
      int iszero=0;
      vecteur res= bisection_solver(v0,v[1],gguess[0],gguess[1],iszero,contextptr);
      if (!res.empty() && iszero!=1)
	*logptr(contextptr) << (iszero==-1?gettext("Warning: undefined"):gettext("Warning: sign reversal")) << '\n';
      return res;
    }
#endif
#ifdef HAVE_LIBGSL
    if (method!=_NEWTON_SOLVER){
      bool with_derivative=false;
      switch (method){
      case _BISECTION_SOLVER: case _FALSEPOS_SOLVER: case _BRENT_SOLVER:
	with_derivative=false;
	break;
      case _NEWTON_SOLVER: case _SECANT_SOLVER: case _STEFFENSON_SOLVER:
	with_derivative=true;
	break;
      }
      gen params;
      if (with_derivative){
	gen dv0=derive(v0,v[1],contextptr);
	if (is_undef(dv0))
	  return dv0;
	params= makevecteur(v0,dv0,v[1]);
	double guess((gnuplot_xmin+gnuplot_xmax)/2),oldguess;
	if (s>=3){
	  gen g=evalf(gguess,eval_level(contextptr),contextptr);
	  if (g.type==_DOUBLE_)
	    guess=g._DOUBLE_val;
	}
	gsl_function_fdf FDF ;     
	FDF.f = &my_f ;
	FDF.df = &my_df ;
	FDF.fdf = &my_fdf ;
	FDF.params = &params ;
	gsl_root_fdfsolver * slv=0;
	switch (method){
	case _NEWTON_SOLVER:
	  slv=gsl_root_fdfsolver_alloc (gsl_root_fdfsolver_newton);
	  break;
	case _SECANT_SOLVER: 
	  slv=gsl_root_fdfsolver_alloc (gsl_root_fdfsolver_secant);
	  break;
	case _STEFFENSON_SOLVER:
	  slv=gsl_root_fdfsolver_alloc (gsl_root_fdfsolver_steffenson);
	  break;
	}
	if (!slv)
	  return gensizeerr(contextptr);
	gsl_root_fdfsolver_set(slv,&FDF,guess);
	int maxiter=SOLVER_MAX_ITERATE,res=0;
	for (;maxiter;--maxiter){
	  oldguess=guess;
	  res=gsl_root_fdfsolver_iterate(slv);
	  guess=gsl_root_fdfsolver_root(slv);
	  if ( (res==GSL_EBADFUNC) || (res==GSL_EZERODIV) )
	    break;
	  if (fabs(guess-oldguess)<gsl_eps)
	    break;
	}
	gsl_root_fdfsolver_free(slv);
	if (!maxiter)
	  return gensizeerr(contextptr);
	if ( (res==GSL_EBADFUNC) || (res==GSL_EZERODIV) )
	  return undef;
	else
	  return guess;
      }
      else {
	params= makevecteur(v0,v[1]);
	double x_low,x_high;
	if (s>=3) {
	  vecteur w;
	  if (gguess.type==_VECT)
	    w=*gguess._VECTptr;
	  else {
	    if ( (gguess.type==_SYMB) && (gguess._SYMBptr->sommet==at_interval) )
	      w=*gguess._SYMBptr->feuille._VECTptr;
	  }
	  if (w.size()!=2)
	    return gentypeerr(contextptr);
	  gen low=w[0].evalf(eval_level(contextptr),contextptr);
	  gen high=w[1].evalf(eval_level(contextptr),contextptr);
	  if ( (low.type!=_DOUBLE_) || (high.type!=_DOUBLE_) )
	    return gensizeerr(contextptr);
	  x_low=low._DOUBLE_val;
	  x_high=high._DOUBLE_val;
	}
	else {
	  x_low=gnuplot_xmin;
	  x_high=gnuplot_xmax;
	}
	if (x_low>x_high){
	  double tmp=x_low;
	  x_low=x_high;
	  x_high=tmp;
	}
	gsl_function F ;
	F.function=&my_f;
	F.params = &params ;
	gsl_root_fsolver * slv =0 ;
	switch (method){
	case  _BISECTION_SOLVER:
	  slv=gsl_root_fsolver_alloc (gsl_root_fsolver_bisection);
	  break;
	case _FALSEPOS_SOLVER: 
	  slv=gsl_root_fsolver_alloc (gsl_root_fsolver_falsepos);
	  break;
	case _BRENT_SOLVER:
	  slv=gsl_root_fsolver_alloc (gsl_root_fsolver_brent);
	  break;
	}
	if (!slv)
	  return gensizeerr(contextptr);
	gsl_root_fsolver_set (slv,&F,x_low,x_high);
	int res=0;
	int maxiter=SOLVER_MAX_ITERATE;
	for (;maxiter && (x_high-x_low>gsl_eps);--maxiter){
	  res=gsl_root_fsolver_iterate(slv);
	  if (res==GSL_EBADFUNC)
	    break;
	  x_low=gsl_root_fsolver_x_lower(slv);
	  x_high= gsl_root_fsolver_x_upper(slv);
	}
	gsl_root_fsolver_free (slv);
	if (res==GSL_EBADFUNC)
	  return undef;
	return makevecteur(x_low,x_high);
      } // end if derivative
    }
#else // HAVE_LIBGSL
    if (method!=_NEWTON_SOLVER)
      return gensizeerr(gettext("Not linked with GSL"));
#endif    // HAVE_LIBGSL
    else  {// newton method, call newton
      gguess=newton(v0,v[1],gguess,NEWTON_DEFAULT_ITERATION,gsl_eps,1e-12,!complex_mode(contextptr),1,0,1,0,1,contextptr);
      if (is_greater(1e-8,abs(im(gguess,contextptr),contextptr)/abs(re(gguess,contextptr),contextptr),contextptr))
	return re(gguess,contextptr);
      return gguess;
    }
    return undef;
  } // end f_solve
  static const char _fsolve_s []="fsolve";
  static define_unary_function_eval_quoted (__fsolve,&_fsolve,_fsolve_s);
  define_unary_function_ptr5( at_fsolve ,alias_at_fsolve,&__fsolve,_QUOTE_ARGUMENTS,true);

  gen _cfsolve(const gen &args,GIAC_CONTEXT){
    bool b=complex_mode(contextptr);
    complex_mode(true,contextptr);
    gen res=_fsolve(args,contextptr);
    complex_mode(b,contextptr);
    return res;
  }
  static const char _cfsolve_s []="cfsolve";
  static define_unary_function_eval_quoted (__cfsolve,&_cfsolve,_cfsolve_s);
  define_unary_function_ptr5( at_cfsolve ,alias_at_cfsolve,&__cfsolve,_QUOTE_ARGUMENTS,true);

  vecteur sxa(const vecteur & sl_orig,const vecteur & x,GIAC_CONTEXT){
    vecteur sl;
    aplatir(sl_orig,sl,true);
    int d;
    d=int(x.size());
    int de;
    de=int(sl.size());
    for (int i=0;i<de;i++){
      //gen e:
      //e=sl[i];    
      if ( (sl[i].type==_SYMB) && ((*sl[i]._SYMBptr).sommet==at_equal || (*sl[i]._SYMBptr).sommet==at_equal2 || (*sl[i]._SYMBptr).sommet==at_same)){
	sl[i]=(*sl[i]._SYMBptr).feuille[0]-(*sl[i]._SYMBptr).feuille[1];
      }
    }
    vecteur A;
    for (int i=0;i<de;i++){
      vecteur li(d+1);
      gen lo=sl[i];
      for (int j=0;j<d;j++){
	lo=subst(lo,x[j],0,false,contextptr);
	li[j]=derive(sl[i],x[j],contextptr);
      }
      li[d]=lo;
      A.push_back(li);
    }
    return(A);
  }

  // return -2 invalid, -1 (unknown), 0 (no solution), 1 (solution exist)
  int linsolve_ineq(const vecteur & sl,const vecteur & x,vecteur & sol,GIAC_CONTEXT){
    // first extract equations, recursive solve and substitute
    for (int i=0;i<sl.size();++i){
      gen sli=sl[i],a,b,xj,XJ;
      if (!is_inequation(sli)){
        for (int j=0;j<x.size();++j){
          xj=x[j];
          if (is_linear_wrt(sli,xj,a,b,contextptr) && !is_zero(a)){
            XJ=-b/a;
            vecteur SL=sl;
            SL.erase(SL.begin()+i);
            SL=subst(SL,xj,XJ,true,contextptr);
            vecteur X(x);
            X.erase(X.begin()+j);
            int r=linsolve_ineq(SL,X,sol,contextptr);
            if (r<1)
              return r;
            sol.insert(sol.begin()+j,0);
            xj=subst(XJ,x,sol,false,contextptr);
            sol[j]=xj;
            return 1;
          }
        }
      }
    }
    // now we have only inequations, rewrite them as Ax>=0
    // and keep a list of indices for strict inequalities
    vector<int> strict; vecteur SL; gen g;
    for (int i=0;i<sl.size();++i){
      gen sli=sl[i];
      if (!is_inequation(sli))
        return -2;
      if (sli._SYMBptr->sommet==at_superieur_strict || sli._SYMBptr->sommet==at_inferieur_strict)
        strict.push_back(i);
      g=sli._SYMBptr->feuille[0]-sli._SYMBptr->feuille[1];
      if (sli._SYMBptr->sommet==at_inferieur_egal || sli._SYMBptr->sommet==at_inferieur_strict)
        g=-g;
      SL.push_back(g);
    }
    matrice A=sxa(SL,x,contextptr);
    int n=x.size();
    // if x is n-dimensional, search n independant inequalities
    // change basis to rewrite them as x>=0
    matrice Ared; vecteur pivots; gen det;
    vector<int> permutation; int R;
    for (;;){
      permutation.resize(A.size());
      for (int i=0;i<A.size();++i)
        permutation[i]=i;
      // reduction without last column (constants)
      mrref(A,Ared,permutation,pivots,det,0,A.size(),0,A[0]._VECTptr->size()-1,0,0,true,0,0,contextptr);
      R=A.size();
      for (;R>0;--R){
        const vecteur & v=*Ared[R-1]._VECTptr;
        int j=0;
        for (;j<v.size()-1;++j){
          if (!is_exactly_zero(v[j]))
            break;
        }
        if (j<v.size()-1)
          break;
      }
      if (R==0)
        return -2;
      if (R==x.size())
        break;
      // add a fake large inequalities
      // find which x_i we should add
      int i=0;
      for (;i<x.size() && i<Ared.size();++i){
        if (is_zero(Ared[i][i]))
          break;
      }
      vecteur line(x.size()+1);
      line[i]=1;
      A.push_back(line); // go reduce it
    } 
    // extrnact invertible matrix
    matrice P(R);
    for (int i=0;i<R;++i){
      vecteur & v=*A[permutation[i]]._VECTptr;
      P[i]=vecteur(v.begin(),v.begin()+R);
    }
    // invert
    matrice Pinv=minv(P,contextptr);
    vecteur translate(R);
    // origin translation
    for (int i=0;i<R;++i)
      translate[i]=A[permutation[i]][R];
    // multiply each line by Pinv : line <- Pinv*line
    for (int i=R;i<A.size();++i){
      vecteur & v =*A[permutation[i]]._VECTptr;
      vecteur w(v.begin(),v.begin()+R);
      w=multvecteurmat(w,Pinv);
      gen & cst=v[R];
      cst = -cst;
      for (int j=0;j<R;++j){
        v[j]=w[j];
        cst += v[j]*translate[j];
      }
    }
    sol.resize(R);
    if (A.size()==R){
      // no more inequalities, we can take [1...1] translated as solution
      for (int i=0;i<R;++i)
        sol[i]=-translate[i]+1;
      sol=multmatvecteur(Pinv,sol);
      return 1;
    }
    // there are additional remaining inequalities
    matrice Ab;
    for (int i=R;i<A.size();++i)
      Ab.push_back(-A[permutation[i]]);
    matrice Aborig(Ab);
    int m=Ab.size(); vecteur last2(m);
    for (int i=0;i<m;++i){
      if (is_strictly_positive(-Ab[i][R],contextptr)){
        Ab[i]=-Ab[i];
        // if b_i<0 we must optimize with respect to this variable
        // and the optimum of the artificial problem (phase I) used
        // to find a solution of the large inequalities system
        // optimum must be 0 (so that the artificial variable disappear)
        last2[i]=1; 
      }
    }
    // insert identity
    for (int i=0;i<m;++i){
      vecteur & v=*Ab[i]._VECTptr;
      gen cst=v[R];
      v.pop_back();
      for (int j=0;j<m;++j)
        v.push_back(i==j?1:0);
      v.push_back(cst);
    }
    vecteur last(mergevecteur(vecteur(R,0),last2));
    last.push_back(0);
    Ab.push_back(last);
    vecteur v; gen optimum;
    // if we have m additional inequalities between Ax and b, b>=0
    // search solutions for Ax=b under constraints x>=0
    // If b_i<0 change sign s.t. b>=0
    // Call simplex_reduce on (m additional slack variables y)
    // ( A | I_m | B )
    // (0.0| 1.1 | 0 )
    // if optimum is <0, there is no solution
    // otherwise extract m first components of the solution
    matrice res=simplex_reduce(Ab,v,optimum,true,false,contextptr);
    if (!is_zero(optimum))
      return 0;
    // now v fullfills large inequalities in translated new basis
    // update solution
    sol=vecteur(v.begin(),v.begin()+n);
    for (int i=0;i<sol.size();++i)
      sol[i] -= translate[i];
    bool nostrict=true;
    for (int i=0;i<strict.size();++i){
      // pos=strict[i] is a strict ineq
      // if v[permutation[pos]]!=0 large ineq is already strict
      int pos=strict[i];
      if (is_exactly_zero(v[pos])){
        nostrict=false;
        break;
      }
    }
    if (nostrict){
      sol=multmatvecteur(Pinv,sol);
      return 1;
    }    
    // In non degenerate situations, n inequalities (including x>=0)
    // will be equalities, with nomal vectors that form a basis,
    // in this basis increasing a little bit the coordinates will
    // keep all other inequalities strict
    vector<int> zeropos; gen minabs(1);
    for (int i=0;i<m+R;++i){
      if (is_exactly_zero(v[i])){
        zeropos.push_back(i);
      }
      else {
        gen curabs;
        if (i<R)
          curabs=abs(v[i],contextptr);
        else
          curabs=Aborig[i-R][R];
        if (is_greater(minabs,curabs,contextptr))
          minabs=curabs;
      }        
    }
    int z=zeropos.size();
    matrice P2(z),I(midn(n));
    vecteur strictv(z);
    // homogeneous strict inequalities: run a small translation
    // find direction by solving an optimization problem
    for (int i=0;i<z;++i){
      int pos=zeropos[i];
      bool is_strict=equalposcomp(strict,permutation[pos]);
      if (pos<R){
        P2[i]=I[pos];
      }
      else {
        vecteur &w=*Aborig[pos-R]._VECTptr;
        P2[i]=-vecteur(w.begin(),w.begin()+n);
      }
      // add identity parts
      for (int j=0;j<z;++j)
        P2[i]._VECTptr->push_back(i==j?-1:0);
      for (int j=0;j<z;++j)
        P2[i]._VECTptr->push_back(i==j?1:0);
      P2[i]._VECTptr->push_back(is_strict?1:0);
      if (is_strict)
        strictv[i]=1;
    }
    vecteur ligne(n+z,0); ligne=mergevecteur(ligne,strictv); ligne.push_back(0);
    P2.push_back(ligne);
    res=simplex_reduce(P2,v,optimum,true,false,contextptr);
    if (!is_zero(optimum)){
      sol=multmatvecteur(Pinv,sol);
      *logptr(contextptr) << "Unable to solve with strict constraints, large solution " << sol << "\n";
      return 0; 
    }
    // v is the direction
    v=vecteur(v.begin(),v.begin()+n);
    gen Pnorm=linfnorm(v,contextptr)*linfnorm(Pinv,contextptr);
    vecteur delta=minabs/Pnorm/gen(2*n)*v;
    sol=addvecteur(sol,delta);
    sol=multmatvecteur(Pinv,sol);
    return 1;
  }

  gen create_var(vecteur & X,int & count){
    for (;count<RAND_MAX;++count){
      identificateur xii("x"+print_INT_(count));
      gen xi(xii);
      if (equalposcomp(X,xi))
        continue;
      X.push_back(xi);
      return xi;
    }
    return undef;
  }

  vecteur remove_ineq(const vecteur & v){
    vecteur w(v);
    for (int i=0;i<w.size();++i){
      if (is_inequation(w[i]))
        w[i]=w[i]._SYMBptr->feuille[0]-w[i]._SYMBptr->feuille[1];
    }
    return w;
  }

  gen remove_neg_sq(const gen & g){
    if (g.is_symb_of_sommet(at_neg))
      return remove_neg_sq(g._SYMBptr->feuille);
    if (g.is_symb_of_sommet(at_pow) && g._SYMBptr->feuille[1]==2)
      return g._SYMBptr->feuille[0];
    return g;
  }

  void resize_solutions(vecteur & v, int s){
    for (int i=0;i<v.size();++i){
      if (v[i].type==_VECT && v[i]._VECTptr->size()>s)
        v[i]=vecteur(v[i]._VECTptr->begin(),v[i]._VECTptr->begin()+s);
    }
  }

  vecteur remove_sommet(const vecteur & v){
    vecteur w(v);
    for (int i=0;i<w.size();++i){
      if (w[i].type==_SYMB)
        w[i]=w[i]._SYMBptr->feuille;
    }
    return w;
  }

  // return -3 too many random tries, -2 invalid, -1 (unknown), 0 (no solution), 1 (solution exist)
  int solve_ineq(const vecteur & sl_,const vecteur &x,vecteur & res,bool allowrec,GIAC_CONTEXT){
    vecteur v(x);
    vecteur sl(gen2vecteur(exact(sl_,contextptr)));
    sl=gen2vecteur(hyp2exp(sl,contextptr));
    lvar(remove_ineq(sl),v);
    vecteur X(x);
    int count=0;
    // for each algebraic var add an equation
    if (v!=x){
      vecteur vt=mergevecteur(mergevecteur(lop(v,at_sin),lop(v,at_cos)),mergevecteur(lop(v,at_tan),lop(v,at_exp)));
      if (vt.size()>1){
        gen vtexp=_texpand(vt,contextptr);
        vecteur wt=mergevecteur(mergevecteur(lop(vtexp,at_sin),lop(vtexp,at_cos)),mergevecteur(lop(vtexp,at_tan),lop(vtexp,at_exp)));
        vecteur vtfeuille(remove_sommet(vt)),wtfeuille(remove_sommet(wt));
        comprim(wtfeuille); comprim(vtfeuille);
        if (wtfeuille.size()<vtfeuille.size()){
          sl=subst(sl,vt,gen2vecteur(vtexp),true,contextptr);
          v=x;
          lvar(remove_ineq(sl),v);
        }
      }
      vecteur vsincos,vexp;
      vecteur vin,vout;
      for (int i=0;i<v.size();++i){
        gen g=v[i];
        if (g.is_symb_of_sommet(at_sin) || g.is_symb_of_sommet(at_cos) || g.is_symb_of_sommet(at_tan)){
          g=g._SYMBptr->feuille;
          if (!equalposcomp(vsincos,g))
            vsincos.push_back(g);
          continue;
        }
        if (g.is_symb_of_sommet(at_exp)){
          g=g._SYMBptr->feuille;
          if (!equalposcomp(vexp,g))
            vexp.push_back(g);
          continue;
        }
        if (g.is_symb_of_sommet(at_pow)){
          gen base=g._SYMBptr->feuille[0];
          gen expo=g._SYMBptr->feuille[1];
          if (expo.type==_FRAC){ // g=base^(n/d), g^d=base^n
            gen xi=create_var(X,count);
            sl.push_back(pow(base,expo._FRACptr->num,contextptr)-symb_pow(xi,expo._FRACptr->den));
            vin.push_back(g);
            vout.push_back(xi);
            continue;
          }
        }
        if (equalposcomp(x,g))
          continue;
        return -3; // non polynomial
      }
      if (!vin.empty())
        sl=subst(sl,vin,vout,true,contextptr);
      if (!vsincos.empty() || !vexp.empty()){
        // FIXME sort phases and detect relationships
        // add two variables per phase
        vecteur vtrigin,vtrigout,vtrig,vexp_,vexpin,vexpout;
        for (int i=0;i<vsincos.size();++i){
          gen ci=create_var(X,count);
          gen si=create_var(X,count);
          gen ti=si/ci;
          gen g=vsincos[i];
          vtrig.push_back(ci);
          vtrig.push_back(si);
          vtrigin.push_back(symb_cos(g));
          vtrigin.push_back(symb_sin(g));
          vtrigin.push_back(symb_tan(g));
          vtrigout.push_back(ci);
          vtrigout.push_back(si);
          vtrigout.push_back(ti);
          sl.push_back(ci*ci+si*si-1);
        }
        sl=subst(sl,vtrigin,vtrigout,true,contextptr);
        for (int i=0;i<vexp.size();++i){
          gen ei=create_var(X,count);
          gen g=vexp[i];
          vexp_.push_back(ei);
          vexpin.push_back(symb_exp(g));
#ifdef INEQEXPINV
          vexpout.push_back(inv(ei*ei,contextptr));
#else
          vexpout.push_back(ei);
          sl.push_back(symb_superieur_strict(ei,0));
#endif
        }
        sl=subst(sl,vexpin,vexpout,true,contextptr);
        vecteur vars(mergevecteur(vtrig,vexp_));
        lidnt(sl,vars,true);
        int r=solve_ineq(sl,vars,res,allowrec,contextptr);
        if (r<=0)
          return r;
        // solve vars==res[0]
        vecteur sol=gen2vecteur(res.front());
        vecteur EQ;
        for (int i=0;i<vsincos.size();++i){
          EQ.push_back(vsincos[i]-arg(gen(sol[2*i],sol[2*i+1]),contextptr));
        }
        for (int i=0;i<vexp.size();++i){
          gen g=sol[i+vtrig.size()];
#ifdef INEQEXPINV
          EQ.push_back(vexp[i]+ln(normal(g*g,contextptr),contextptr));
#else
          EQ.push_back(vexp[i]-ln(g,contextptr));
#endif
        }
        for (int i=vtrig.size()+vexp_.size();i<vars.size();++i)
          EQ.push_back(vars[i]-sol[i]);
        vecteur V=gsolve(EQ,X,false,false,contextptr);
        if (V.empty()){
          res.clear();
          return -1;
        }
        resize_solutions(V,x.size());
        res[0]=V;
        return 1;
      }
    }
    res=vecteur(1,undef);
    int strict=0; // ineqs with index >= strict are strict
    vecteur eq,ineqs,ineqsnum;
    //vecteur ineqsvar,ineqadded;
    for (int i=0;i<sl.size();++i){
      gen g=sl[i];
      if (is_inequation(g)){
        bool stricteq=g._SYMBptr->sommet==at_superieur_strict || g._SYMBptr->sommet==at_inferieur_strict;
        if (g._SYMBptr->sommet==at_superieur_strict || g._SYMBptr->sommet==at_superieur_egal)
          g=g._SYMBptr->feuille[0]-g._SYMBptr->feuille[1];
        else
          g=g._SYMBptr->feuille[1]-g._SYMBptr->feuille[0];
        g=_fxnd(g,contextptr);
        if (g.type!=_VECT || g._VECTptr->size()!=2)
          return -2;
        gen num=g._VECTptr->front();
        g=num*g._VECTptr->back();
        if (0 && !stricteq){ // would be slower
          gen var;
          if (num.type==_IDNT) // num>0
            var=num;
          else if (num.is_symb_of_sommet(at_neg) && num._SYMBptr->feuille.type==_IDNT)
            var=-num; // num<0
          if (var.type==_IDNT && equalposcomp(X,var)){
            gen xi=create_var(X,count);
            gen xi2=symb_pow(xi,2);
            if (num.type==_IDNT){        
              eq.push_back(var-xi2);
              sl=subst(sl,var,xi2,true,contextptr);
            }
            else {
              eq.push_back(var+xi2);
              sl=subst(sl,var,-xi2,true,contextptr);
            }
            continue;
          }
        }
        // n/d>0 is equivalent to n*d>0 and n/d>=0 is equivalent to n*d>=0
        bool done=false;
        // detect here some simple ineqs that are equations, like -x^2-y^2>=0
        gen deg=_total_degree(makesequence(g,X),contextptr);
        if (deg==2){
          fraction p=sym2r(g,X,contextptr);
          if (p.num.type==_POLY){
            // extract homogeneous part of degree 2
            polynome q(p.num._POLYptr->dim);
            vector< monomial<gen> >::const_iterator it=p.num._POLYptr->coord.begin(),itend=p.num._POLYptr->coord.end();
            for (;it!=itend;++it){
              if (total_degree(it->index)==2)
                q.coord.push_back(*it);
            }
            gen Q=r2sym(q,X,contextptr);
            vecteur D,U,P;
            vecteur w=gauss(Q,X,D,U,P,contextptr);
            // find signature
            int i=0;
            for (;i<D.size();++i){
              if (is_strictly_positive(D[i],contextptr))
                break; // we have a true inequation
            }
            if (i==D.size()){
              // is it possible to translate?
              vecteur dg=gen2vecteur(derive(g,X,contextptr));
              vecteur delta=linsolve(dg,X,contextptr);
              if (delta.size()==X.size()){
                gen gt=subst(g,X,delta,false,contextptr);
                if (is_exactly_zero(ratnormal(gt,contextptr))){
                  // each element of w translated is an equation ==0
                  w=subst(w,X,X-delta,false,contextptr);
                  for (int i=0;i<w.size();++i){ if (!is_zero(w[i])) eq.push_back(remove_neg_sq(w[i])); } // eq=mergevecteur(eq,w);
                  done=true;
                }
                else if (is_positive(-gt,contextptr)){ // empty solution
                  res.clear();
                  return 0;
                }
              }
            }
          }
        }
        else if (deg%2==0){ // check for -(perfect square)
          gen var=create_var(X,count);
          --count; X.pop_back();
          for (int i=0;!done && i<X.size();++i){
            if (is_constant_wrt(g,X[i],contextptr))
              continue;
            gen gv=g-var;
            gen g1=derive(gv,X[i],contextptr);
            gen R=_resultant(makesequence(gv,g1,X[i]),contextptr);
            vecteur S=solve(R,var,0,contextptr);
            for (int r=0;!done && r<S.size();++r){
              if (!is_zero(derive(S[r],X,contextptr)))
                continue;
              bool iszero=is_exactly_zero(S[r]);
              if (!iszero && is_positive(S[r],contextptr))
                continue;
              fraction f=sym2r(g-S[r],X,contextptr);
              if (f.num.type==_POLY && is_integer(f.num._POLYptr->coord.front().value) && is_positive(-f.num._POLYptr->coord.front().value,contextptr) && is_integer(f.den) && is_positive(f.den,contextptr)){
                polynome p=-*f.num._POLYptr;
                vecteur S=sqrfree(p,X,1,contextptr),EQ; int I;
                // check degrees of sqrfree factorization
                for (I=0;I<S.size();++I){
                  const gen g=S[I];
                  if (g.type==_VECT && g._VECTptr->size()==2){
                    gen g0=g._VECTptr->front();
                    if (!is_zero(derive(g0,X,contextptr))){
                      if (g._VECTptr->back().val%2)
                        break; // even multiplicity
                      EQ.push_back(g0);
                    }
                  }
                }
                if (I==S.size()){
                  // all multiplicities are even
                  if (!iszero){
                    res.clear();
                    return 0;
                  }
                  // add equations
                  eq=mergevecteur(eq,EQ);
                  done=true;
                }
              }
            }
          }
        } // else if deg%2==0
        if (done){
          if (stricteq){
            res.clear();
            return 0;
          }
        }
        else {
          if (stricteq){
            ineqs.push_back(g);
            ineqsnum.push_back(num);
          }
          else {
            ineqs.insert(ineqs.begin()+strict,g);
            ineqsnum.insert(ineqsnum.begin()+strict,num);
            ++strict;
          }
        }
      }
      else 
        eq.push_back(_numer(equal2diff(g),contextptr));
    }
    if (ineqs.empty()){
      res=gsolve(eq,X,false/*complexmode*/,false/*evalfafter*/,contextptr);
      resize_solutions(res,x.size());
      return res.empty()?0:1;
    }
    if (eq.empty() && X.size()<=3){
      // only inequalities, try some points?
      if (X.size()==2){
        const int N=11; // 11^2=121
        for (int i=0;i<N*N;++i){
          vecteur Xval(makevecteur(i/11-5,i%11-5));
          for (int j=0;j<ineqs.size();++j){
            gen cur=normal(subst(ineqs[j],X,Xval,true,contextptr),contextptr);
            if (is_exactly_zero(cur) && j>=strict)
              break;
            if (is_positive(cur,contextptr)){
              if (j==ineqs.size()-1){
                res.clear();
                res.push_back(Xval);
                res.push_back(string2gen(gettext("Certificate of existence, more solutions may exist"),false));
                resize_solutions(res,x.size());
                return 1;
              }
              continue;
            }
            break;
          }
        }
      }
      else {
        const int N=5; // 5^3=125
        for (int i=0;i<N*N*N;++i){
          vecteur Xval(makevecteur(i/25-2,(i%25)/5-2,i%5-2));
          for (int j=0;j<ineqs.size();++j){
            gen cur=normal(subst(ineqs[j],X,Xval,true,contextptr),contextptr);
            if (is_exactly_zero(cur) && j>=strict)
              break;
            if (is_positive(cur,contextptr)){
              if (j==ineqs.size()-1){
                res.clear();
                res.push_back(Xval);
                res.push_back(string2gen(gettext("Certificate of existence, more solutions may exist"),false));
                resize_solutions(res,x.size());
                return 1;
              }
              continue;
            }
            break;
          }
        }
      }
    }
#if 1 // try replacing ineqs by eq
    // now we have a polynomial system of equations, call gbasis
    gen G;
    // find vertices,
    // this is done by increasing n, a number of ineqs to be replaced by eq
    // by setting any possible combination of n ineqs
    // until rur works
    int nmax=ineqs.size();
    res.clear();
    bool dim0=false;
    for (int n=0;n<=nmax;++n){
      if (n>=1 && !res.empty()){
        resize_solutions(res,x.size());
        return 1;
      }
#ifdef NO_STDEXCEPT
      if (n+eq.size()<X.size())
        continue;
#endif
      gen g=comb(nmax,n);
      if (g.type!=_INT_)
        return -3;
      int N=g.val;
      vector<int> pos(n);
      for (int i=0;i<n;++i)
        pos[i]=i;
      for (int i=0;i<N;i++){
        vecteur EQ(eq);
        for (int j=0;j<pos.size();++j)
          EQ.push_back(ineqsnum[pos[j]]);
#ifdef NO_STDEXCEPT
        G=_gbasis(makesequence(EQ,X,change_subtype(_RUR_REVLEX,_INT_GROEBNER)),contextptr);
#else
        try {
          G=_gbasis(makesequence(EQ,X,change_subtype(_RUR_REVLEX,_INT_GROEBNER)),contextptr);
        } 
        catch (std::runtime_error & err){
          G=undef;
        }
#endif
        if (G.type==_VECT && G._VECTptr->size()==X.size()+4 && G._VECTptr->front().type==_INT_ && G._VECTptr->front().val==_RUR_REVLEX){
          // system is 0 dimensional, find real solutions
          vecteur Gv=*G._VECTptr,S;
          gen pmin=G[2];
          gen rurvar=lidnt(G[2]).front();
          vecteur GX(Gv.begin()+3,Gv.end()),GX1(Gv.begin()+4,Gv.end());
          GX1=multvecteur(inv(GX.front(),contextptr),GX1);
          vecteur R;
          if (_degree(pmin,contextptr).val<=4)
            R=solve(pmin,rurvar,0,contextptr);
          if (R.empty())
            R=gen2vecteur(_realroot(makesequence(pmin,epsilon(contextptr)),contextptr));
          if (!R.empty() && !is_undef(R)){
            dim0=true;
            vector<bool> ineq_is_zero(ineqs.size());
            for (int k=0;k<ineqsnum.size();++k){
              gen cur=ineqsnum[k];
              cur=subst(cur,X,GX1,false,contextptr);
              cur=_numer(cur,contextptr);
              cur=_rem(makesequence(cur,pmin,rurvar),contextptr);
              ineq_is_zero[k]=is_exactly_zero(cur);
            }
            for (int j=0;j<R.size();++j){
              gen solrur=R[j]; if (solrur.type==_VECT) solrur=solrur[0]; // remove multiplicity
              vecteur GXsol=subst(GX,rurvar,solrur,false,contextptr);
              gen sol=normal(inv(GXsol.front(),contextptr)*vecteur(GXsol.begin()+1,GXsol.end()),contextptr);
              // check if R satisfies remaining ineq
              // should be done exactly if almost zero
              int k;
              vector<int> strictiszero,largezero;
              for (k=0;k<ineqs.size();++k){
                if (ineq_is_zero[k]){ 
                  largezero.push_back(k);
                  if (k<strict)
                    continue;
                  if (!equalposcomp(pos,k))
                    break;
                  strictiszero.push_back(k);
                  continue;
                }
                gen cur=ineqs[k];
                cur=subst(cur,X,sol,false,contextptr);
                //cur=normal(cur,contextptr);
                if (is_strictly_positive(cur,contextptr))
                  continue; // ineq is satisfied
                break;
              }
              if (k==ineqs.size()){ // all ineq are satisfied
                if (!strictiszero.empty()){
                  if (allowrec){
                    // for strict ineq, try to shift ineq
                    // linearization at sol
                    // derivatives of eq and large ineq|solution =0
                    // derivative of strict ineq|solution must be >0
                    vecteur SL;
                    for (int i=0;i<eq.size();++i){
                      gen cur=eq[i];
                      cur=derive(cur,X,contextptr);
                      cur=subst(cur,X,sol,false,contextptr);
                      cur=scalar_product(cur,X,contextptr);
                      SL.push_back(cur);
                    }
                    bool non0=true;
                    for (int i=0;i<largezero.size();++i){
                      gen cur=ineqs[largezero[i]];
                      cur=derive(cur,X,contextptr);
                      cur=subst(cur,X,sol,false,contextptr);
                      cur=normal(cur,contextptr);
                      if (is_zero(cur)){
                        non0=false;
                        break;
                      }
                      cur=scalar_product(cur,X,contextptr);
                      cur=symbolic(equalposcomp(strictiszero,pos[i])?at_superieur_strict:at_superieur_egal,makesequence(cur,zero));  
                      SL.push_back(cur);
                    }
                    if (non0){
                      vecteur linsol;
                      int lres=linsolve_ineq(SL,X,linsol,contextptr);
                      if (lres==1){
                        // if there is a solution to the linearized pb
                        // recursive call with these strict ineq replaced by large ineq with 0 replaced by value*epsilon for epsilon=1,1/2, 1/2^2, ... until it works
                        gen eps=1;
                        vecteur strictv;
                        for (int expo=0;expo<10;++expo,eps=eps/2){
                          vecteur newsys(eq);
                          int strictcount=0;
                          for (int i=0;i<ineqs.size();++i){
                            gen cur=ineqs[i];
                            if (equalposcomp(strictiszero,i)){
                              if (strictv.size()>strictcount)
                                cur=symbolic(at_superieur_egal,makesequence(cur,eps*strictv[strictcount]));
                              else {
                                gen dcur=derive(cur,X,contextptr);
                                dcur=subst(dcur,X,sol,false,contextptr);
                                dcur=scalar_product(dcur,linsol,contextptr);
                                dcur=normal(dcur,contextptr);
                                strictv.push_back(dcur);
                                cur=symbolic(at_superieur_egal,makesequence(cur,eps*dcur));
                              }
                              ++strictcount;
                            }
                            else
                              cur=symbolic(i>=strict?at_superieur_strict:at_superieur_egal,makesequence(cur,zero));
                            newsys.push_back(cur);
                          }
                          int ires=solve_ineq(newsys,x,res,false/*allowrec*/,contextptr);
                          if (ires==1){
                            resize_solutions(res,x.size());
                            return 1;
                          }
                        }
                      }
                    } // if non0
                  }
                  continue;
                }
                res.push_back(sol);
                if (n>0){
                  res.push_back(string2gen(gettext("Certificate of existence, more solutions may exist"),false));
                  return 1;
                }
              }
            }
          } // end R not empty
          if (n==0){
            resize_solutions(res,x.size());
            return res.empty()?0:1;
          }
        } // end 0 dimensional system
        // increment pos
        for (int j=n-1;j>=0;j--){
          ++pos[j];
          if (pos[j]<nmax-(n-1-j)){
            for (int k=j+1;k<n;++k){
              pos[k]=pos[k-1]+1;
            }
            break;
          }
        }
      } // end for i
    } // end for n
#endif
    // all attempts replacing ineqs by eq failed
    int n=X.size(),eqs=eq.size(),ins=ineqs.size(),s=eqs+ins;
    if (dim0 && s>3 && proba_epsilon(contextptr)){
      *logptr(contextptr) << "No solution found, run proba_epsilon:=0 to certify\n";
      res.clear();
      return 0;
    }
    vecteur V(s),W(s); // identifiers
    int vi=0,wi=0;
    for (int i=0;i<s;++i){
      for (;;){
        gen Vi=identificateur("v"+print_INT_(vi));
        ++vi;
        if (!equalposcomp(X,Vi)){
          V[i]=Vi;
          break;
        }
      }
      for (;;){
        gen Vi=identificateur("w"+print_INT_(wi));
        ++wi;
        if (!equalposcomp(X,Vi)){
          W[i]=Vi;
          break;
        }
      }
    }
    // create random 0 dimensional system
    // https://arxiv.org/pdf/1603.01183.pdf
    for (int nr=0;;++nr){
      if (nr==NEWTON_MAX_RANDOM_RESTART)
        return -3;
      gen J=0; int alpha=1,beta=0,gamma=1,delta=0;
      for (int i=0;i<n;++i){
        alpha=1+std_rand()%9;
        beta=std_rand()%9;
        J += alpha*pow(X[i]-beta,2,contextptr);
      }
      for (int i=0;i<s;++i){
        if (i<eqs)
          J += V[i]*eq[i];
        else if (i<eqs+strict)
          J += V[i]*(ineqs[i-eqs]-pow(W[i],2,contextptr));
        else
          J += V[i]*(pow(W[i],2,contextptr)*ineqs[i-eqs]-1);
        gamma=1+std_rand()%9;
        delta=std_rand()%9;
        J += gamma*pow(W[i]-delta,2,contextptr);
      }
      vecteur EQ,XVW(mergevecteur(mergevecteur(X,V),W));
      for (int i=0;i<XVW.size();++i){
        EQ.push_back(derive(J,XVW[i],contextptr));
      }
      gen G=_gbasis(makesequence(EQ,XVW,change_subtype(_RUR_REVLEX,_INT_GROEBNER)),contextptr);
      if (G.type==_VECT && G._VECTptr->size()==XVW.size()+4 && G._VECTptr->front().type==_INT_ && G._VECTptr->front().val==_RUR_REVLEX){
        // system is 0 dimensional, find real solutions
        vecteur Gv=*G._VECTptr,S;
        gen pmin=G[2];
        gen rurvar=lvar(G[2]).front();
        vecteur R=gen2vecteur(_realroot(makesequence(pmin,epsilon(contextptr)),contextptr));
        if (R.empty()){
          if (strict){
            *logptr(contextptr) << "No solution for strict inequalities.\n" ;
            return -1;
          }
          res.clear();
          return 0;
        }
        // extract solutions
        if (is_undef(R))
          return -2;
        gen rursol=R[0][0];
        vecteur Gx(G._VECTptr->begin()+3,G._VECTptr->begin()+4+X.size());
        Gx=subst(Gx,rurvar,rursol,false,contextptr);
        vecteur Gx1(Gx.begin()+1,Gx.end());
        gen sol=inv(Gx.front(),contextptr)*Gx1;
        res.clear();
        res.push_back(sol);
        res.push_back(string2gen("Certificate of existence, more solution may exist",false));
        resize_solutions(res,x.size());
        return 1;
      }
    }
    return -1;
  }

  vecteur ineq_addassume(const vecteur & sl_,const vecteur &x,GIAC_CONTEXT){
    vecteur sl(sl_);
    for (int i=0;i<x.size();++i){
      gen g=x[i];
      if (g.type!=_IDNT)
        continue;
      gen g2=g._IDNTptr->eval(1,g,contextptr);
      if (g2.type!=_VECT || g2.subtype!=_ASSUME__VECT)
        continue;
      vecteur v=*g2._VECTptr;
      if ( v.size()==3 && (v.front()==vecteur(0) || v.front()==_DOUBLE_ || v.front()==_ZINT || v.front()==_SYMB || v.front()==0) && v[1].type==_VECT){
        vecteur a=*v[1]._VECTptr;
        if (a.empty() || a.front().type!=_VECT)
          continue;
        if (a.size()>1)
          *logptr(contextptr) << "Assumption on" << x[i] << ": solving only for the first interval in " << a << "\n";
        a=*a.front()._VECTptr;
        if (a.size()==2 && v[2].type==_VECT){
          if (a[0]!=minus_inf)
            sl.push_back(symbolic(equalposcomp(*v[2]._VECTptr,a[0])?at_superieur_strict:at_superieur_egal,makesequence(x[i],a[0])));
          if (a[1]!=plus_inf)
            sl.push_back(symbolic(equalposcomp(*v[2]._VECTptr,a[1])?at_inferieur_strict:at_inferieur_egal,makesequence(x[i],a[1])));          
        }
      }
    }
    return sl;
  }

  // number of possible assumptions on x
  vector<int> ineq_countassume(const vecteur &x,GIAC_CONTEXT){
    vector<int> res(x.size(),1);
    for (int i=0;i<x.size();++i){
      gen g=x[i];
      if (g.type!=_IDNT)
        continue;
      gen g2=g._IDNTptr->eval(1,g,contextptr);
      if (g2.type!=_VECT || g2.subtype!=_ASSUME__VECT)
        continue;
      vecteur v=*g2._VECTptr;
      if ( v.size()==3 && (v.front()==vecteur(0) || v.front()==_DOUBLE_ || v.front()==_ZINT || v.front()==_SYMB || v.front()==0) && v[1].type==_VECT){
        vecteur a=*v[1]._VECTptr;
        if (a.empty() || a.front().type!=_VECT)
          continue;
        res[i]=a.size();
      }
    }
    return res;
  }
  
  vecteur ineq_addassume(const vecteur & sl_,const vecteur &x,const vector<int> & Nassume,int n,GIAC_CONTEXT){
    vector<int> N(Nassume.size());
    for (int i=0;i<Nassume.size();++i){
      N[i]=n%Nassume[i];
      n=n/Nassume[i];
    }
    vecteur sl(sl_);
    for (int i=0;i<x.size();++i){
      gen g=x[i];
      if (g.type!=_IDNT)
        continue;
      gen g2=g._IDNTptr->eval(1,g,contextptr);
      if (g2.type!=_VECT || g2.subtype!=_ASSUME__VECT)
        continue;
      vecteur v=*g2._VECTptr;
      if ( v.size()==3 && (v.front()==vecteur(0) || v.front()==_DOUBLE_ || v.front()==_ZINT || v.front()==_SYMB || v.front()==0) && v[1].type==_VECT){
        vecteur a=*v[1]._VECTptr;
        if (a.empty())
          continue;
        if (a.size()<N[i] || a[N[i]].type!=_VECT)
          *logptr(contextptr) << "ineq_addassume bug!\n";
        a=*a[N[i]]._VECTptr;
        if (a.size()==2 && v[2].type==_VECT){
          if (a[0]!=minus_inf)
            sl.push_back(symbolic(equalposcomp(*v[2]._VECTptr,a[0])?at_superieur_strict:at_superieur_egal,makesequence(x[i],a[0])));
          if (a[1]!=plus_inf)
            sl.push_back(symbolic(equalposcomp(*v[2]._VECTptr,a[1])?at_inferieur_strict:at_inferieur_egal,makesequence(x[i],a[1])));          
        }
      }
    }
    return sl;
  }

  vecteur linsolve(const vecteur & sl,const vecteur & x,GIAC_CONTEXT){
    if (sl.empty())
      return x;
    vecteur A; 
    if (ckmatrix(sl)){
      unsigned int n=unsigned(sl.size());
      if (n>=GIAC_PADIC && n==x.size() && is_integer_matrice(sl) && is_integer_vecteur(x)){
	gen p,det_mod_p,h2;
	int res=padic_linsolve(sl,x,A,p,det_mod_p,h2);
	if (res==1)
	  return A;
      }
      A=mtran(sl);
      if (ckmatrix(x)){
	if (x.size()==1){
	  if (x.front()._VECTptr->size()!=n)
	    return vecteur(1,gendimerr(contextptr));
	  A.push_back(-x.front());
	}
	else {
	  if (x.size()!=n)
	    return vecteur(1,gendimerr(contextptr));
	  matrice xm=mtran(x);
	  if (xm.size()!=1)
	    return vecteur(1,gensizeerr(contextptr));
	  A.push_back(-xm.front());
	}
      }
      else {
	if (x.size()!=n)
	  return vecteur(1,gendimerr(contextptr));
	A.push_back(-x);
      }
      A=mtran(A);
      //int c=int(A.front()._VECTptr->size());
      vecteur B=-mker(A,contextptr);
      if (is_undef(B) || B.empty())
	return B;
      // The last element of B must have a non-zero last component
      vecteur Bend=*B.back()._VECTptr;
      gen last=Bend.back();
      if (is_zero(last,contextptr))
	return vecteur(0);
      gen R=Bend/last;
      // The solution is sum(B[k]*Ck+Blast/last)
      int s=int(B.size());
      for (int k=0;k<s-1;k++)
	R=R+gen("C_"+print_INT_(k),contextptr)*B[k];
      vecteur res=*R._VECTptr;
      res.pop_back();
      return res;
    }
    if (ckmatrix(sl,false,false)) // check whether sl has undef inside
      return vecteur(x.size(),undef);
    // symbolic system. First check for inequalities
    if (has_op(sl,*at_superieur_strict) || has_op(sl,*at_inferieur_strict) ||
        has_op(sl,*at_superieur_egal) || has_op(sl,*at_inferieur_egal)){
      vecteur res;
      vector<int> Nassume(ineq_countassume(x,contextptr));
      int N=1;
      for (int i=0;i<Nassume.size();++i)
        N *= Nassume[i];
#if 0
      if (N==1){
        int val=linsolve_ineq(ineq_addassume(sl,x,contextptr),x,res,contextptr);
        if (val<0)
          return vecteur(1,gensizeerr(gettext("Unable to solve inequalities system")));
        if (val==0)
          return vecteur(0);
        //return res;
        return makevecteur(res,string2gen(gettext("Certificate of existence, more solutions may exist"),false));
      }
#endif
      for (int i=0;i<N;++i){
        int val=linsolve_ineq(ineq_addassume(sl,x,Nassume,i,contextptr),x,res,contextptr);
        if (val<-1)
          return vecteur(1,gensizeerr(gettext("Unable to solve inequalities system")));
        if (val>0)
          return makevecteur(res,string2gen(gettext("Certificate of existence, more solutions may exist"),false));
      }
      return vecteur(0);
    }
    A=sxa(sl,x,contextptr);
    vecteur B,R(x);
    gen rep;
    if (A.size()==2 && x.size()==2){
      gen a00=A[0][0]; a00=simplify(a00,contextptr);
      if (is_zero(a00,contextptr))
	B=makevecteur(A[1],A[0]);
      else 
	B=makevecteur(A[0],subvecteur(multvecteur(A[0][0],*A[1]._VECTptr),multvecteur(A[1][0],*A[0]._VECTptr)));
      B[1]._VECTptr->front()=0;
      gen b11=(*B[1]._VECTptr)[1];
      b11=simplify(b11,contextptr);
      if (!is_zero(b11)){
        gen y=-B[1][2]/B[1][1];
        // B[0][0]*x=-(B[0][1]*y+B[0][2])
        gen x=-(B[0][1]*y+B[0][2])/B[0][0];
        return makevecteur(x,y);
	B=makevecteur(subvecteur(multvecteur(b11,*B[0]._VECTptr),multvecteur(B[0][1],*B[1]._VECTptr)),B[1]);
	(*B[0]._VECTptr)[1]=0;
      }
      (*B[1]._VECTptr)[1]=b11;
      gen b12=simplify((*B[1]._VECTptr)[2],contextptr);
      (*B[1]._VECTptr)[2]=b12;
    }
    else
      B=mrref(A,contextptr);
    //COUT<<B<<'\n';
    int d=int(x.size());
    int de=int(sl.size());
    for (int i=0; i<de;i++){
      vecteur li(d+1);
      for(int k=0;k<d+1;k++){
	li[k]=B[i][k];
      }
      int j;
      j=i;
      while (j<d && li[j]==0){
	j=j+1;
      }
      if (j==d && !is_zero(li[d],contextptr)){
	return vecteur(0);
      } 
      else {
	if (j<d){
	  rep=-li[d];
	  for (int k=j+1;k<d;k++){
	    rep=rep-li[k]*x[k];
	  }
	  rep=rdiv(rep,li[j],contextptr);
	  R[j]=rep;
	}
      }
    }
    return R;
  }

  gen equal2diff(const gen & g){
    if ( (g.type==_SYMB) && (g._SYMBptr->sommet==at_equal || g._SYMBptr->sommet==at_equal2 || g._SYMBptr->sommet==at_same) ){
      vecteur & v=*g._SYMBptr->feuille._VECTptr;
      return v[0]-v[1];
    }
    else
      return g;
  }

  gen symb_linsolve(const gen & syst,const gen & vars){
    return symbolic(at_linsolve,makesequence(syst,vars));
  }
 
  gen linsolve(const gen & syst,const gen & vars,GIAC_CONTEXT){
    if ((syst.type!=_VECT)||(vars.type!=_VECT))
      return symb_linsolve(syst,vars);
    gen res=linsolve(*syst._VECTptr,*vars._VECTptr,contextptr);
    if (!has_i(syst) && has_i(res))
      res=_evalc(res,contextptr);
    else
      res=normal(res,contextptr);
    return res;
  }
  

  // solve triangular system l*a=y where l is the lower part of a lu decomp 
  void linsolve_l(const matrice & m,const vecteur & y,vecteur & a){
    // l*a=y: a1=y1, a2=y2-m_21*a1, ..., ak=yk-sum_{j=1..k-1}(m_kj*aj)
    int n=int(y.size());
    a.resize(n);
    gen * astart=&a[0];
    *astart=y[0]/m[0][0];
    for (int k=1;k<n;++k){
      const gen * mkj=&m[k]._VECTptr->front();
      gen *aj=astart,*ak=astart+k;
      gen res=y[k];
      for (;aj<ak;++mkj,++aj)
	res -= (*mkj)*(*aj); 
      *ak=res/(*mkj);
    }
  }

  // solve upper triangular system m*y=a
  void linsolve_u(const matrice & m,const vecteur & y,vecteur & a){
    // u*[a0,..,an-1]=[y0,...,yn]
    // a_{n-1}=y_{n-1}/u_{n-1,n-1}
    // a_{n-2}=(y_{n-2}-u_{n-2,n-1}*a_{n-1})/u_{n-2,n-2}
    // ...
    // a_k=(y_{k}-sum_{j=k+1..n-1} u_{k,j}a_j)/u_{k,k}
    int n=int(y.size());
    a.resize(n);
    for (int k=n-1;k>=0;--k){
      gen res=y[k];
      gen * mkj=&(*m[k]._VECTptr)[n-1],*colj=&a[n-1],*colend=&a[k];
      for (;colj>colend;--mkj,--colj){
	res -= (*mkj)*(*colj);
      }
      *colj=res/(*mkj);
    }
  }

  gen _linsolve(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    vecteur v(plotpreprocess(args,contextptr));
    if (is_undef(v))
      return v;
    int s=int(v.size());
    bool eq=false;
    if (s==3 && v[2]==at_equal){
      eq=true;
      --s;
      v.pop_back();
    }
    if (s==4){
      // P,L,U,B, solve A*X=B where P*A=L*U
      gen P=v[0],L=eval(v[1],1,contextptr),U=v[2],B=v[3];
      if (P.type!=_VECT || B.type!=_VECT)
	return gensizeerr(contextptr);
      vector<int> p;
      if (!is_permu(*P._VECTptr,p,contextptr))
	return gensizeerr(contextptr);
      matrice b; int n=int(B._VECTptr->size());
      bool mat=ckmatrix(B);
      if (!mat){
	b=vecteur(1,B);	
	if (!ckmatrix(b))
	  return gensizeerr(contextptr);
      }
      else 
	b=mtran(*B._VECTptr);
      bool map=L.type==_MAP && U.type==_MAP;
      if (!map && (!ckmatrix(L) || !ckmatrix(U)))
	return gensizeerr(contextptr);
      if (n!=int(p.size()) || (!map && (n!=int(L._VECTptr->size()) || n!=int(U._VECTptr->size()))))
	return gendimerr(contextptr);
      vecteur res; res.reserve(b.size());
      for (unsigned i=0;i<b.size();++i){
	const vecteur & Bv=*b[i]._VECTptr;
	vecteur c(n),y(n),x(n);
	for (int i=0;i<n;++i)
	  c[i]=Bv[p[i]];
	// now solve L*(U*X)=c
	if (map){
	  if (!sparse_linsolve_l(*L._MAPptr,c,y))
	    return gendimerr(contextptr);
	  if (!sparse_linsolve_u(*U._MAPptr,y,x))
	    return gendimerr(contextptr);
	}
	else {
	  linsolve_l(*L._VECTptr,c,y);
	  linsolve_u(*U._VECTptr,y,x);
	}
	if (!mat)
	  return x;
	res.push_back(x);
      }
      return gen(mtran(res),_MATRIX__VECT);
    }
    if (s!=2)
      return gentoomanyargs("linsolve");
    if (is_squarematrix(v[0]) && v[1].type==_VECT){
      // maybe it's a triangular system
      matrice & m=*v[0]._VECTptr;
      int n=int(m.size());
      bool mat=ckmatrix(v[1]);
      vecteur b,res;
      if (!mat){
	b=vecteur(1,v[1]);	
	if (!ckmatrix(b))
	  return gensizeerr(contextptr);
      }
      else 
	b=mtran(*v[1]._VECTptr);
      if (n>=2){
	if (is_zero(m[0][1],contextptr)){
	  // lower triangular?
	  bool lower=true;
	  for (int i=0;lower && i<n;++i){
	    vecteur & v=*m[i]._VECTptr;
	    for (int j=i+1;lower && j<n;++j){
	      lower=is_zero(v[j]);
	    }
	  }
	  if (lower){
	    for (unsigned i=0;i<b.size();++i){
	      vecteur y(n);
	      linsolve_l(m,*b[i]._VECTptr,y);
	      if (!mat)
		return y;
	      res.push_back(y);
	    }
	    return res;
	  }
	}
	// upper triangular?
	bool upper=true;
	for (int i=1;upper && i<n;++i){
	  vecteur & v=*m[i]._VECTptr;
	  for (int j=0;upper && j<i;++j){
	    upper=is_zero(v[j]);
	  }
	}
	if (upper){
	  for (unsigned i=0;i<b.size();++i){
	    vecteur y(n);
	    linsolve_u(m,*b[i]._VECTptr,y);
	    if (!mat)
	      return y;
	    res.push_back(y);
	  }
	  return res;
	}
      }
    }
    if (v[1].type==_IDNT)
      v[1]=eval(v[1],eval_level(contextptr),contextptr);
    gen syst=apply(v[0],equal2diff),vars=v[1];
    gen res= linsolve(syst,v[1],contextptr);
    if (eq)
      res=_list2exp(makesequence(res,v[1]),contextptr);
    return res;
  }
  static const char _linsolve_s []="linsolve";
  static define_unary_function_eval_quoted (__linsolve,&_linsolve,_linsolve_s);
  define_unary_function_ptr5( at_linsolve ,alias_at_linsolve,&__linsolve,_QUOTE_ARGUMENTS,true);

  static const char _resoudre_systeme_lineaire_s []="resoudre_systeme_lineaire";
  static define_unary_function_eval_quoted (__resoudre_systeme_lineaire,&_linsolve,_resoudre_systeme_lineaire_s);
  define_unary_function_ptr5( at_resoudre_systeme_lineaire ,alias_at_resoudre_systeme_lineaire,&__resoudre_systeme_lineaire,_QUOTE_ARGUMENTS,true);

  /*
  gen iter(const gen & f, const gen & x,const gen & arg,int maxiter,double eps, int & b){
    gen a=arg;
    complex<double> olda;
    complex<double> ad;
    b=0;
    ad=gen2complex_d(a);
    //COUT<<"a"<<a<<'\n';
    //COUT<<"ad"<<ad<<'\n';
    for (int j=1;j<=maxiter;j++){
      olda=ad;    
      // COUT << f << " " << x << " " << a << '\n';
      a=subst(f,x,a).evalf();
      // COUT<<"a"<<a<<'\n';
      //a=a.evalf();
      //ad=a._DOUBLE_val;
      ad=gen2complex_d(a);
      // COUT<<"a"<<a<<'\n';
      // COUT<<"ad"<<ad<<'\n';
      // COUT<<"j"<<j<<"abs"<<abs(ad-olda)<<'\n';
      if (eps>abs(ad-olda)) {
	b=1; return(a);
      }
    } 
    return(a); 
  }
  
  gen newtona(const gen & f, const gen & x, const gen & arg,int niter1, int niter2, double eps1,double eps2,double prefact1,double prefact2, int & b){
    if (x.type!=_IDNT)
      settypeerr(gettext("2nd arg must be an identifier"));
    //COUT<<a<<'\n';
    gen a=arg;
    gen g1;
    gen g;
    g1=x-gen(prefact1)*rdiv(f,derive(f,x));
    // sym_sub(x,sym_mult(rdiv(9,10),rdiv(f,derive(f,x))));
    try {
      a= iter(g1,x,a,niter1,eps1,b);
      g=x-gen(prefact2)*rdiv(f,derive(f,x));
      a= iter(g,x,a,niter2,eps2,b); 
    }
    catch (std::runtime_error & err){
      last_evaled_argptr(contextptr)=NULL;
      b=0;
    }
    return a;
  }
 
  gen newton(const gen & f, const gen & x,const gen & guess,int niter1,int niter2,double eps1,double eps2,double prefact1,double prefact2){
    bool guess_first=is_undef(guess);
    for (int j=1;j<5;j++,niter2 *=2, niter1 *=2){ 
      gen a;
      int b;
      //on prend un d�part au hasard (a=x0=un _DOUBLE_)
      // a=gen(2.0);
      if (guess_first)
	a=j*4*(std_rand()/(RAND_MAX+1.0)-0.5);
      else {
	a=guess;
	guess_first=true;
      }
      // COUT<<j<<"j"<<a<<'\n'; 
      gen e;
      e=newtona(f, x, a,niter1,niter2,eps1,eps2,prefact1,prefact2,b);
      if (b==1) return e;
      gen c;
      c=j*4*(std_rand()/(RAND_MAX+1.0)-0.5);
      // COUT<<j<<"j"<<c<<'\n';
      // g=x-gen(0.5)*rdiv(f,derive(f,x));
      gen ao(gen(0.0),c);
      // COUT<<"ao"<<ao<<'\n';
      gen e0= newtona(f, x, ao,niter1,niter2,eps1,eps2,prefact1,prefact2,b);
      if (b==1) 
	return(e0);
      gen a1(a,c);
      // COUT<<j<<"j,a1"<<a1<<'\n';
      e0= newtona(f, x, a1,niter1,niter2,eps1,eps2,prefact1,prefact2,b);
      if (b==1) 
	return(e0);
    }
    setsizeerr(gettext("nontrouve"));
    return(0);
  }
  */

  static gen newton_rand(int j,bool real,double xmin,double xmax,GIAC_CONTEXT){
    gen a=gen(giac_rand(contextptr));
    a=a/(gen(rand_max2)+1);
    if (xmin<xmax)
      return xmin+(xmax-xmin)*a;
    a-=plus_one_half; 
    a=evalf(j*4*a,1,contextptr);
    if (j>2 && complex_mode(contextptr) && !real)
      a=a+cst_i*evalf(j*4*(gen(giac_rand(contextptr))/(gen(rand_max2)+1)-plus_one_half),1,contextptr);
    return a;
  }

  static gen newton_rand(int j,bool real,double xmin,double xmax,const gen & f,GIAC_CONTEXT){
    if (f.type==_VECT){
      int s=int(f._VECTptr->size());
      vecteur v(s);
      for (int i=0;i<s;++i)
	v[i]=(newton_rand(j,real,xmin,xmax,contextptr));
      return v;
    }
    return newton_rand(j,real,xmin,xmax,contextptr);
  }
  
  gen newton(const gen & f0, const gen & x,const gen & guess_,int niter,double eps1, double eps2,bool real,double xmin,double xmax,double rand_xmin,double rand_xmax,double init_prefactor,GIAC_CONTEXT){
    if (real &&
        (has_i(f0) || has_i(guess_))
        // (!is_zero(im(f0,contextptr),contextptr) || !is_zero(im(guess_,contextptr),contextptr))
        )
      real=false;
    if (abs_calc_mode(contextptr)==38 && x.type==_VECT){
      vecteur AZin,AZout;
      int count=0;
      const vecteur & X=*x._VECTptr;
      for (int i=0;i<X.size();++i){
        if (X[i].type==_IDNT && strlen(X[i]._IDNTptr->id_name)==1 && X[i]._IDNTptr->id_name[0]>='A' && X[i]._IDNTptr->id_name[0]<='Z'){
          for (;;++count){
            identificateur newx_("x"+print_INT_(count));
            gen newx(newx_);
            if (!equalposcomp(X,newx)){
              AZin.push_back(X[i]);
              AZout.push_back(newx);
              ++count;
              break;
            }
          }
        }
      }
      if (!AZin.empty()){
        gen newf0=subst(f0,AZin,AZout,true,contextptr);
        gen newx=subst(X,AZin,AZout,true,contextptr);
        return newton(newf0,newx,guess_,niter,eps1,eps2,real,xmin,xmax,rand_xmin,rand_xmax,init_prefactor,contextptr);
      }
    }
    if (x.type!=_IDNT && x.type!=_VECT){
      if (x.type!=_SYMB || (x._SYMBptr->sommet!=at_at && x._SYMBptr->sommet!=at_of))
	return gensizeerr(contextptr);
    }
    bool out=niter!=NEWTON_DEFAULT_ITERATION;
    gen guess(guess_);
    // ofstream of("log"); of << f0 << '\n' << x << '\n' << guess << '\n' << niter ; 
    gen f=real?eval(f0,1,context0):f0;  // eval of f wrt context0 is intentionnal, replace UTPN by erf
    if (guess.is_symb_of_sommet(at_interval))
      guess=(guess._SYMBptr->feuille[0]+guess._SYMBptr->feuille[1])/2;
    bool inv_after=f.type==_VECT;
    gen a,b,d,fa,fb,invdf=inv_after?derive(f,x,contextptr):inv(derive(f,x,contextptr),contextptr),epsg1(eps1),epsg2(eps2);
    if (is_undef(invdf))
      return invdf;
    if (ckmatrix(invdf))
      invdf=mtran(*invdf._VECTptr);
    bool guess_first=is_undef(guess);
    // Main loop with random initialization
    int j=1; //NEWTON_MAX_RANDOM_RESTART=20;
    for (;j<=NEWTON_MAX_RANDOM_RESTART;j++,niter += 5){ 
      if (guess_first){
	a=newton_rand(j,real,rand_xmin,rand_xmax,f,contextptr);
      }
      else {
	a=guess;
	guess_first=true;
      }
#ifndef NO_STDEXCEPT
      try {
#endif
	fa=evalf(eval(subst(f,x,a,false,contextptr),eval_level(contextptr),contextptr),1,contextptr); 
	// First loop to localize the solution with prefactor
	gen lambda(init_prefactor);
#ifdef HAVE_LIBMPFR
	if (a.type==_REAL){
	  int prec=mpfr_get_prec(a._REALptr->inf);
	  lambda=accurate_evalf(exact(lambda,contextptr),prec);
	}
#endif
	int k;
	for (k=0;k<niter;++k){
#ifdef TIMEOUT
	  control_c();
#endif
	  if (ctrl_c || interrupted) { 
	    interrupted = true; ctrl_c=false;
	    return gensizeerr(gettext("Stopped by user interruption.")); 
	  }
	  d=subst(invdf,x,a,false,contextptr);
	  // of << k << " " << d << " " << invdf << " " << x << " " << a << " ";
	  d=eval(d,eval_level(contextptr),contextptr);
	  // of << d << " " << fa << " ";
	  if (inv_after)
	    d=linsolve(evalf(d,1,contextptr),-fa,contextptr);
	  else
	    d=-evalf(d*fa,1,contextptr);
	  if (is_undef(d) || (d.type==_VECT &&d._VECTptr->empty()) || !lidnt(d).empty()){
	    a=newton_rand(j,real,rand_xmin,rand_xmax,f,contextptr);
	    fa=evalf(eval(subst(f,x,a,false,contextptr),eval_level(contextptr),contextptr),1,contextptr); 
	    continue;
	  }	    
	  if (d.type!=_FLOAT_ && d.type!=_DOUBLE_ && d.type!=_CPLX && d.type!=_REAL && d.type!=_VECT && !is_undef(d) && !is_inf(d))
	    return gensizeerr(contextptr);
	  if (k==0 && is_zero(d,contextptr) && is_greater(abs(fa,contextptr),std::sqrt(eps2),contextptr)){
	    a=newton_rand(j,real,rand_xmin,rand_xmax,f,contextptr);
	    fa=evalf(eval(subst(f,x,a,false,contextptr),eval_level(contextptr),contextptr),1,contextptr); 
	    continue;
	  }
	  // of << d << " " << '\n';
	  // of << k << " " << invdf << " " << " " << f << " " << x << " " << a << " " << fa << " " << d << " " << epsg1 << '\n';
	  // CERR << k << " " << invdf << " " << " " << f << " " << x << " " << a << " " << fa << " " << d << " " << epsg1 << '\n';
	  b=a+lambda*d;
	  if (xmin<xmax){
	    if (!is_zero(im(b,contextptr),contextptr) || is_greater(xmin,b,contextptr) || is_greater(b,xmax,contextptr)){
	      for (;;) {
		a=newton_rand(j,real,rand_xmin,rand_xmax,contextptr);
		if (is_greater(a,xmin,contextptr) && is_greater(xmax,a,contextptr))
		break;
	      }
	      fa=evalf(eval(subst(f,x,a,false,contextptr),eval_level(contextptr),contextptr),1,contextptr); 
	      continue;
	    }
	  }
	  else {
	    if(real && !is_zero(im(b,contextptr),contextptr)){
	      a=newton_rand(j,real,rand_xmin,rand_xmax,contextptr);
	      fa=evalf(eval(subst(f,x,a,false,contextptr),eval_level(contextptr),contextptr),1,contextptr); 
	      continue;
	    }
	  }
	  gen babs=_l2norm(b,contextptr);
	  if (is_inf(babs) || is_undef(babs)){
	    guess_first=true;
	    k=niter;
	    break;
	  }
	  if (is_positive(epsg1-_l2norm(d,contextptr)/max(1,babs,contextptr),contextptr)){
	    a=b;
	    break;
	  }
	  fb=evalf(eval(subst(f,x,b,false,contextptr),eval_level(contextptr),contextptr),1,contextptr);
	  if ( (real && !is_zero(im(fb,contextptr),contextptr)) ||
	       is_positive(_l2norm(fb,contextptr)-_l2norm(fa,contextptr),contextptr)){
	    // Decrease prefactor and try again
	    lambda=evalf(plus_one_half*lambda,1,contextptr);
	  }
	  else {
	    // Save new value of a and increase the prefactor slightly
	    if (is_positive(lambda-0.9,contextptr))
	      lambda=1;
	    else
	      lambda=evalf(gen(12)/gen(10),1,contextptr)*lambda;
	    a=b;
	    fa=fb;
	  }
	} // end for (k<niter)
	if (k==niter){
	  if (out)
	    return a;
	  continue;
	}
	// Second loop to improve precision (prefactor 1)
	for (k=0;k<niter;++k){
#ifdef TIMEOUT
	  control_c();
#endif
	  if (ctrl_c || interrupted) { 
	    interrupted = true; ctrl_c=false;
	    return gensizeerr(gettext("Stopped by user interruption.")); 
	  }
	  if (inv_after)
	    d=linsolve(evalf(subst(invdf,x,a,false,contextptr),1,contextptr),-subst(f,x,a,false,contextptr),contextptr);
	  else
	    d=-evalf(subst(invdf,x,a,false,contextptr)*subst(f,x,a,false,contextptr),1,contextptr);
	  if (is_undef(d) || (d.type==_VECT &&d._VECTptr->empty()) || !lidnt(d).empty()){
	    return gensizeerr(gettext("Unable to solve numerically, perhaps a multiple root?"));             
          }          
	  a=a+d;
	  if (is_positive(epsg2-_l2norm(d,contextptr)/max(1,abs(a,contextptr),contextptr),contextptr))
	    break;
	}
	if (k!=niter || is_positive(epsg1-_l2norm(d,contextptr)/max(1,abs(a,contextptr),contextptr),contextptr))
	  break;
#ifndef NO_STDEXCEPT
      } catch (std::runtime_error & ){
	last_evaled_argptr(contextptr)=NULL;
	continue; // start with a new initial point
      }
#endif
    } // end for
    if (j>5)
      return undef;
    return a;
  }

  gen _newton(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    double gsl_eps=epsilon(contextptr);
    if (args.type!=_VECT)
      return newton(args,vx_var,undef,NEWTON_DEFAULT_ITERATION,gsl_eps,1e-12,!complex_mode(contextptr),1,0,1,0,1,contextptr);
    vecteur v=*args._VECTptr;
    int s=int(v.size());
    v[0]=apply(v[0],equal2diff);
    if (s<2)
      return gensizeerr(contextptr);
    if (s==2){
      if (is_equal(v[1]))
	return newton(v[0],v[1]._SYMBptr->feuille[0],v[1]._SYMBptr->feuille[1],NEWTON_DEFAULT_ITERATION,gsl_eps,1e-12,!complex_mode(contextptr),1,0,1,0,1,contextptr);
      return newton(v[0],v[1],undef,NEWTON_DEFAULT_ITERATION,gsl_eps,1e-12,!complex_mode(contextptr),1,0,1,0,1,contextptr);
    }
    int niter=NEWTON_DEFAULT_ITERATION;
    double eps=epsilon(contextptr);
    for (int j=3;j<s;++j){
      if (v[j].type==_INT_)
	niter=v[j].val;
      else {
	gen tmp=evalf_double(v[j],1,contextptr);
	if (tmp.type==_DOUBLE_)
	  eps=tmp._DOUBLE_val;
      }
    }
    gen res=newton(v[0],v[1],v[2],niter,1e-10,eps,!complex_mode(contextptr),1,0,1,0,1,contextptr);
    if (debug_infolevel)
      *logptr(contextptr) << res << '\n';
    return res;
    // return gentoomanyargs("newton");
  }
  static const char _newton_s []="newton";
  static define_unary_function_eval (__newton,&_newton,_newton_s);
  define_unary_function_ptr5( at_newton ,alias_at_newton,&__newton,0,true);
  
  bool has_num_coeff(const vecteur & v){
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (has_num_coeff(*it))
	return true;
    }
    return false;
  }
  
  bool has_num_coeff(const polynome & p){
    vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end();
    for (;it!=itend;++it){
      if (has_num_coeff(it->value))
	return true;
    }
    return false;
  }

  bool has_num_coeff(const gen & e){
    switch (e.type){
    case _ZINT: case _INT_: case _IDNT: case _USER:
      return false;
    case _DOUBLE_: case _REAL: case _FLOAT_:
      return true;
    case _CPLX:
      return (e._CPLXptr->type==_DOUBLE_ || e._CPLXptr->type==_REAL || e._CPLXptr->type==_FLOAT_) || ((e._CPLXptr+1)->type==_DOUBLE_ || (e._CPLXptr+1)->type==_REAL || (e._CPLXptr+1)->type==_FLOAT_);
    case _SYMB:
      return has_num_coeff(e._SYMBptr->feuille);
    case _VECT:
      return has_num_coeff(*e._VECTptr);
    case _POLY:
      return has_num_coeff(*e._POLYptr);
    case _FRAC:
      return has_num_coeff(e._FRACptr->num) || has_num_coeff(e._FRACptr->den);
    default:
      return false;
    }
    return 0;
  }

  bool has_mod_coeff(const vecteur & v,gen & modulo){
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (has_mod_coeff(*it,modulo))
	return true;
    }
    return false;
  }

  bool has_mod_coeff(const polynome & p,gen & modulo){
    vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end();
    for (;it!=itend;++it){
      if (has_mod_coeff(it->value,modulo))
	return true;
    }
    return false;
  }

  bool has_mod_coeff(const gen & e,gen & modulo){
    switch (e.type){
    case _MOD:
      modulo = *(e._MODptr+1);
      return true;
    case _SYMB:
      return has_mod_coeff(e._SYMBptr->feuille,modulo);
    case _VECT:
      return has_mod_coeff(*e._VECTptr,modulo);
    case _POLY:
      return has_mod_coeff(*e._POLYptr,modulo);
    default:
      return false;
    }
  }

#if 1
#define inplace_ppz ppz
#else
  // find gcd of coefficients of p but aborts and returns 1 if it is small
  // otherwise divides
  gen zint_ppz(polynome & p){
    vector< monomial<gen> >::iterator it=p.coord.begin(),itend=p.coord.end();
    if (it==itend)
      return 1;
    for (;it!=itend;++it){
      if (it->value.type==_INT_)
	return 1;
    }
    gen res=(itend-1)->value;
    for (it=p.coord.begin();it!=itend;++it){
      res=gcd(res,it->value);
      if (res.type==_INT_)
	return 1;
    }
    for (it=p.coord.begin();it!=itend;++it){
      if (it->value.type!=_ZINT || it->value.ref_count()>1)
	it->value=it->value/res; 
      else
	mpz_divexact(*it->value._ZINTptr,*it->value._ZINTptr,*res._ZINTptr);
    }
    return res;
  }

  gen inplace_ppz(polynome & p,bool divide=true){
    vector< monomial<gen> >::iterator it=p.coord.begin(),itend=p.coord.end();
    if (it==itend)
      return 1;
    gen res=(itend-1)->value;
    for (it=p.coord.begin();it!=itend-1;++it){
      res=gcd(res,it->value);
      if (is_one(res))
	return 1;
    }
    if (!divide)
      return res;
    if (res.type==_INT_ && res.val>0){
      for (it=p.coord.begin();it!=itend;++it){
	if (it->value.type!=_ZINT || it->value.ref_count()>1)
	  it->value=it->value/res; 
	else
	  mpz_divexact_ui(*it->value._ZINTptr,*it->value._ZINTptr,res.val);
      }
      return res;
    }
    if (res.type==_ZINT){
      for (it=p.coord.begin();it!=itend;++it){
	if (it->value.type!=_ZINT || it->value.ref_count()>1)
	  it->value=it->value/res; 
	else
	  mpz_divexact(*it->value._ZINTptr,*it->value._ZINTptr,*res._ZINTptr);
      }
      return res;
    }
    for (it=p.coord.begin();it!=itend;++it){
      it->value=it->value/res; 
    }
    return res;
  }
#endif

  polynome spoly(const polynome & p,const polynome & q,environment * env){
    if (p.coord.empty())
      return q;
    if (q.coord.empty())
      return p;
    const index_t & pi = p.coord.front().index.iref();
    const index_t & qi = q.coord.front().index.iref();
    index_t lcm = index_lcm(pi,qi);
    polynome tmp=p.shift(lcm-pi,q.coord.front().value)-q.shift(lcm-qi,p.coord.front().value);
    // gen g=zint_ppz(tmp); if (debug_infolevel>1) CERR << "spoly ppz " << g << '\n';
    return (env && env->moduloon)?smod(tmp,env->modulo):tmp;
  }

  // this version of reduce returns in rem the reduction of m*p
  // other version of reduce do not care about m
  void reduce(const polynome & p,const polynome * it0,const polynome * itend,polynome & rem,gen & m,environment * env,vector<polynome> * quo=0){
    if (quo){
      quo->resize(itend-it0);
      for (int i=0;i<int(quo->size());++i){
	(*quo)[i].dim=p.dim;
	(*quo)[i].coord.clear();
      }
    }
    m=1;
    if (&p!=&rem)
      rem=p;
    if (p.coord.empty())
      return ;
    polynome TMP1(p.dim,p),TMP2(p.dim,p);
    std::vector< monomial<gen> >::const_iterator pt=p.coord.begin(),ptend;
    const polynome * it;
    for (;;){
      ptend=rem.coord.end();
      // look in rem for a monomial >= to a monomial in it0, then it0+1 
      for (it=it0; it!=itend ;++it){
	for (pt=rem.coord.begin();pt!=ptend;++pt){
	  if (pt->index>=it->coord.front().index)
	    break;
	}
	if (pt!=ptend)
	  break;
      }
      if (it==itend) // no monomial of rem are divisible by LT(b): finished
	break;
      gen a(pt->value),b(it->coord.front().value) ;
      if (env && env->moduloon){
	gen q=a*invmod(b,env->modulo);
	polynome temp=it->shift(pt->index-it->coord.front().index,q);
	rem = smod(rem - temp,env->modulo) ; // FIXME: improve!
	if (quo){
	  (*quo)[it-it0].coord.push_back(monomial<gen>(q,pt->index-it->coord.front().index));
	}
      }
      else {
	simplify(a,b);
	m=b*m;
	if (quo){
	  for (int i=0;i<int(quo->size());++i){
	    (*quo)[i]=b*((*quo)[i]);
	  }
	  (*quo)[it-it0].coord.push_back(monomial<gen>(a,pt->index-it->coord.front().index));
	}
#if 0
	polynome temp=it->shift(pt->index-it->coord.front().index,a);
	if (is_one(b))
	  rem = rem-temp;
	else {
	  rem = b*rem - temp;
	  inplace_ppz(rem);
	}
#else
	TMP1.coord.clear();
	TMP2.coord.clear();
	Shift(it->coord,pt->index-it->coord.front().index,a,TMP1.coord);
	if (!is_one(b))
	  rem *= b;
	rem.TSub(TMP1,TMP2);
	swap(rem.coord,TMP2.coord);
#endif
      }
    }
    if (quo && (!env || !env->moduloon)){
      for (int i=0;i<int(quo->size());++i){
	(*quo)[i]=(*quo)[i]/m;
      }
    }
    m=m/inplace_ppz(rem);
  }

  polynome reduce(const polynome & p,const polynome * it0,const polynome * itend,environment * env){
    polynome rem(p.dim,p);
    gen m;
    reduce(p,it0,itend,rem,m,env);
    return rem;
  }

  polynome reduce(const polynome & p,const vectpoly & v,environment * env){
    const polynome * it=&v.front(),* itend=it+v.size();
    return reduce(p,it,itend,env);
  }

  struct sort_vectpoly_t {
    sort_vectpoly_t() {}
    bool operator () (const tensor<gen> & p,const tensor<gen> & q){
#if 1
      if (p.coord.empty())
	return false;
      if (q.coord.empty())
	return true;
      return p.is_strictly_greater(p.coord.front().index,q.coord.front().index);
      // return p.is_strictly_greater(q.coord.front().index,p.coord.front().index);
#else
      return tensor_is_strictly_greater<gen>(p,q);
#endif
    }
  };

  struct sort_vect_vectpoly_t {
    sort_vect_vectpoly_t() {}
    bool operator () (const vector< tensor<gen> > & vp,const vector< tensor<gen> > & vq){
      const tensor<gen> & p=vp.back();
      const tensor<gen> & q=vq.back();
      if (p.coord.empty())
	return false;
      if (q.coord.empty())
	return true;
      return p.is_strictly_greater(p.coord.front().index,q.coord.front().index);
    }
  };

  void sort_vectpoly(vectpoly::iterator it,vectpoly::iterator itend){
#if 1
    sort(it,itend,sort_vectpoly_t());
#else
    sort(it,itend,tensor_is_strictly_greater<gen>);
#endif
  }

  struct polyptr_int {
    polynome * polyptr;
    int i;
  };

  bool operator < (const polyptr_int & a,const polyptr_int & b){
    const polynome & p=*a.polyptr;
    const polynome & q=*b.polyptr;
    if (p.coord.empty())
      return false;
    if (q.coord.empty())
      return true;
    return p.is_strictly_greater(p.coord.front().index,q.coord.front().index);
  }

  void reduce(vectpoly & res,environment * env){
    if (res.empty())
      return;
    polynome pred(res.front().dim,res.front());
    sort_vectpoly(res.begin(),res.end());
    // reduce res
    for (int i=int(res.size())-2;i>=0;){
      polynome & p=res[i];
      gen m;
      reduce(p,&res.front()+i+1,&res.front()+res.size(),pred,m,env);
      if (pred.coord.empty()){
	res.erase(res.begin()+i);
	--i;
	continue;
      }
      if (pred.coord.size()==p.coord.size()){
	gen & p0=p.coord.front().value;
	gen & pred0=pred.coord.front().value;
	vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end(),jt=pred.coord.begin();
	for (;it!=itend;++jt,++it){
	  if (it->index!=jt->index || it->value*pred0 != jt->value*p0)
	    break;
	}
	if (it==itend){
	  --i;
	  continue;
	}
      }
      // find where we must insert pred
      unsigned j;
      for (j=i+1;j<res.size();++j){
	if (pred.is_strictly_greater(pred.coord.front().index,res[j].coord.front().index))
	  break;
	else
	  swap(res[j-1].coord,res[j].coord);
      }
      // now we can overwrite res[j-1] (=original res[i]) with pred
      swap(res[j-1].coord,pred.coord);
      i=j-2;
    }
  }

  // Will work for a few order only
  // add total degree for faster comparisons
  struct heap_index {
#if 0
    unsigned short order:4;
    unsigned short resi:22; // position in res[G[i]], up to 2^22 monomials allowed
    unsigned short qi:22; // same for quotients[i]
    unsigned short tdeg; // total degree of the product of monomial
#else
    unsigned short resi; 
    unsigned short qi; 
    unsigned short order; 
    unsigned short tdeg; // total degree of the product of monomial
#endif
    unsigned short i; // position in G
    index_m lm; // records the leading monomial
    heap_index():resi(0),qi(0),order(0),tdeg(0),i(0) {}
    heap_index(unsigned _resi,unsigned _qi,unsigned _i):resi(_resi),qi(_qi),order(_REVLEX_ORDER),tdeg(0),i(_i){}
    void dbgprint() { CERR << "index" << lm << " res[G[" << i << "]][" << resi << "], quotients[" << i << "][" << qi << "]" << '\n'; }
  };

  bool operator < (const heap_index & b,const heap_index & a){
    switch(a.order){
    case _TDEG_ORDER:
      if (b.tdeg!=a.tdeg)
	return b.tdeg<a.tdeg;
      return i_total_lex_is_strictly_greater(a.lm,b.lm);
    case _PLEX_ORDER:
      return i_lex_is_strictly_greater(a.lm,b.lm);
    case _3VAR_ORDER:
      return i_3var_is_strictly_greater(a.lm,b.lm);
    case _7VAR_ORDER:
      return i_7var_is_strictly_greater(a.lm,b.lm);
    case _11VAR_ORDER:
      return i_11var_is_strictly_greater(a.lm,b.lm);
    case _16VAR_ORDER:
      return i_16var_is_strictly_greater(a.lm,b.lm);
    case _32VAR_ORDER:
      return i_32var_is_strictly_greater(a.lm,b.lm);
    case _64VAR_ORDER:
      return i_64var_is_strictly_greater(a.lm,b.lm);
    default:
      if (b.tdeg!=a.tdeg)
	return b.tdeg<a.tdeg;
      return i_total_revlex_is_strictly_greater(a.lm,b.lm);
    }
  }

#ifdef HEAP_REDUCE
  void heap_reduce(const polynome & p0,const vectpoly & res,const vector<unsigned> & G,unsigned excluded,polynome & rem,polynome & p,polynome & p2,environment * env){
    p=p0;
    vectpoly quotients(G.size(),polynome(p.dim,p)); // init quotients to null poly
    // first compute quotients using a heap, the heap is the sum_i res[G[i]]*quotients[i]
    vector<index_m> reslm(G.size());
    vecteur reslc(G.size());
    unsigned heapsize=0;
    for (unsigned j=0;j<G.size();++j){
      if (j==excluded)
	continue;
      heapsize+=res[G[j]].coord.size();
      if (!res[G[j]].coord.empty()){
	reslm[j]=res[G[j]].coord.front().index;
	reslc[j]=res[G[j]].coord.front().value;
      }
      if (debug_infolevel>100)
	reslm[0].dbgprint();
    }
    vector<heap_index> heap;
    heap.reserve(heapsize);
    for (unsigned j=0;j<G.size();++j){
      if (j==excluded)
	continue;
      for (unsigned k=0;k<res[G[j]].coord.size();++k)
	heap.push_back(heap_index(k,0,j));
      if (debug_infolevel>100)
	heap.front().dbgprint();
    }
    unsigned heappos=0;
    unsigned ppos=0;
    for (;;){
      gen topcoeff=0;
      // find largest monomial between the heap and p.coord[pos]
      index_m topindex;
      if (heappos==0){
	if (ppos>=p.coord.size())
	  break; // nothing more to do, except copying the rest of p into rem
	topindex=p.coord[ppos].index;
	topcoeff=p.coord[ppos].value;
	++ppos;
      }
      else {
	bool popheap=true;
	topindex=heap.front().lm;
	if (ppos<p.coord.size()){
	  if (topindex==p.coord[ppos].index){
	    topcoeff=p.coord[ppos].value;
	    ++ppos;
	  }
	  else {
	    if (p.is_strictly_greater(p.coord[ppos].index,topindex)){
	      topindex=p.coord[ppos].index;
	      topcoeff=p.coord[ppos].value;
	      ++ppos;
	      popheap=false;
	    }
	  }
	}
	if (popheap){ // add all coefficients of the heap which have the same leading monomial
	  for (;;){
	    heap_index hf=heap.front();
	    std::pop_heap(heap.begin(),heap.begin()+heappos);
	    topcoeff -= res[G[hf.i]].coord[hf.resi].value*quotients[hf.i].coord[hf.qi].value;
	    // replace heap term 
	    ++hf.qi;
	    if (hf.qi<quotients[hf.i].coord.size()){
	      hf.lm = res[G[hf.i]].coord[hf.resi].index + quotients[hf.i].coord[hf.qi].index;
	      hf.tdeg = total_degree(res[G[hf.i]].coord[hf.resi].index)+total_degree(quotients[hf.i].coord[hf.qi].index);
	      heap[heappos-1]=hf;
	      std::push_heap(heap.begin(),heap.begin()+heappos);
	    }
	    else { // quotient term unknown
	      heap[heappos-1]=hf;
	      --heappos;
	    }
	    if (heappos==0 || heap.front().lm!=topindex)
	      break;
	  } // end for
	} // end if popheap
      } // end else heap.empty()
      if (is_zero(topcoeff,contextptr)){
	continue;
      }
      // now we have collected the top coeff and monomial of p-sum_i res[G[i]]*quotients[i]
      // if we can find a leading monomial in res[G[i]] that is <= to this monomial
      // add a new quotient term, update the heap
      // otherwise move the coeff/monomial to rem
      unsigned j;
      for (j=0;j<G.size();++j){
	if (j==excluded)
	  continue;
	if (topindex >= reslm[j])
	  break;
      }
      if (j==G.size()){
	rem.coord.push_back(monomial<gen>(topcoeff,topindex));
	continue;
      }
      // Add a quotient term, 
      // FIXME, take care of env
      gen s=reslc[j];
      simplify(s,topcoeff);
      if (is_minus_one(s)){ // should check also for i and -i
	s=-s;
	topcoeff=-topcoeff;
      }
      if (!is_one(s)){ // multiply everything by s, so that no fraction appear
	rem *= s;
	p *= s;
	for (unsigned k=0;k<G.size();++k)
	  quotients[k] *= s;
      }
      index_m qlm=topindex-reslm[j];
      quotients[j].coord.push_back(monomial<gen>(topcoeff,qlm));
      // look after the heap for terms with
      // i==j and qi=quotients[j].coord.size()-1
      // if multiplied by res[G[i]][0] increment qi, otherwise
      // their index must be computed and they must be pushed on the heap
      for (unsigned k=heappos;k<heapsize;++k){
	heap_index & hf =heap[k];
	if (hf.i==j && hf.qi==quotients[j].coord.size()-1){
	  if (hf.resi==0)
	    ++hf.qi;
	  else {
	    hf.lm=qlm+res[G[hf.i]].coord[hf.resi].index;
	    hf.tdeg=total_degree(qlm)+total_degree(res[G[hf.i]].coord[hf.resi].index);
	    swap(heap[heappos],hf);
	    ++heappos;
	    push_heap(heap.begin(),heap.begin()+heappos);
	  }
	}
      }
    } // end of division loop
    gen g=inplace_ppz(rem);
    if (debug_infolevel>1)
      CERR << "ppz is " << g << '\n';
  }
#endif

  // #define LINEAR_COMB
#ifdef LINEAR_COMB // it's slower, perhaps because a==1 makes new elements
  // a*A+b*B_shifted -> res
  void linear_combination(const polynome & A,const gen & a,const polynome & B,const gen & b,const index_m & bshift,polynome & res){
    vector< monomial<gen> >::const_iterator ait=A.coord.begin(),ait_end=A.coord.end(),
      bit=B.coord.begin(),bit_end=B.coord.end();
    for (;;){
      // If A is finished, fill up with elements from B and stop
      if (ait == ait_end) {
	while (bit != bit_end) {
	  res.coord.push_back(monomial<gen>(b*bit->value,bit->index+bshift));
	  ++bit;
	}
	break;
      } 
      // If A is finished, fill up with elements from a and stop
      if (bit == bit_end) {
	while (ait != ait_end) {
	  res.coord.push_back(monomial<gen>(a*ait->value,ait->index));
	  ++ait;
	}
	break;
      } 
      index_m pow_b = bit->index+bshift;
      // ait and b are non-empty, compare powers
      if (ait->index==pow_b){
	gen diff = a* ait->value + b* bit->value;
	if (!is_zero(diff,contextptr))
	  res.coord.push_back(monomial<gen>(diff,ait->index));
	++ait;
	++bit;
      }
      else {
	while (ait!=ait_end && A.is_strictly_greater(ait->index, pow_b)) {
	  // a has greater power, get coefficient from a
	  res.coord.push_back(monomial<gen>(a*ait->value,ait->index));
	  ++ait;
	} 
	if (ait==ait_end || ait->index!=pow_b){
	  // b has greater power, get coefficient from b
	  res.coord.push_back(monomial<gen>(b*bit->value,pow_b));
	  ++bit;
	} 
      }
    }  
  }
#endif

  void reduce(const polynome & p,const vectpoly & res,const vector<unsigned> & G,unsigned excluded,polynome & rem,polynome & TMP1, polynome & TMP2,environment * env){
#ifdef HEAP_REDUCE
    TMP2.coord.clear();
    heap_reduce(p,res,G,excluded,TMP2,TMP1,TMP2,env);
    swap(rem.coord,TMP2.coord);
    return;
#endif
    if (&p!=&rem)
      rem=p;
    if (p.coord.empty())
      return ;
    std::vector< monomial<gen> >::const_iterator pt,ptend;
    unsigned i,rempos=0;
    for (unsigned count=0;;++count){
      ptend=rem.coord.end();
#if 1 // this branch search first in all leading coeff of G for a monomial 
      // <= to the current rem monomial
      pt=rem.coord.begin()+rempos;
      if (pt>=ptend)
	break;
      for (i=0;i<G.size();++i){
	if (i==excluded || res[G[i]].coord.empty())
	  continue;
	if (pt->index>=res[G[i]].coord.front().index)
	  break;
      }
      if (i==G.size()){ // no leading coeff of G is smaller than the current coeff of rem
	++rempos;
	continue;
      }
#else
      // look in rem for a monomial >= to a monomial in it0, then it0+1 
      for (i=0; i<G.size() ;++i){
	if (i==excluded || res[G[i]].coord.empty())
	  continue;
	const index_m & Gi=res[G[i]].coord.front().index;
	for (pt=rem.coord.begin();pt!=ptend;++pt){
	  if (pt->index>=Gi)
	    break;
	}
	if (pt!=ptend)
	  break;
      }
      if (i==G.size()) // no monomial of rem are divisible by LT(b): finished
	break;
#endif
      gen a(pt->value),b(res[G[i]].coord.front().value);
      if (env && env->moduloon){
	polynome temp=res[G[i]].shift(pt->index-res[G[i]].coord.front().index,a*invmod(b,env->modulo));
	rem = smod(rem - temp,env->modulo) ; // FIXME: improve!
      }
      else {
	simplify(a,b);
	if (b==-1){
	  b=-b;
	  a=-a;
	}
	TMP1.coord.clear();
	TMP2.coord.clear();
#if 0
	linear_combination(rem,b,res[G[i]],-a,pt->index-res[G[i]].coord.front().index,TMP2);
#else
	Shift(res[G[i]].coord,pt->index-res[G[i]].coord.front().index,a,TMP1.coord);
	if (!is_one(b)){
	  rem *= -b;
	  rem.TAdd(TMP1,TMP2);
	}
	else
	  rem.TSub(TMP1,TMP2);
#endif
	swap(rem.coord,TMP2.coord);
	// zint_ppz(rem);
      }
    }
    gen g=inplace_ppz(rem);
    if (debug_infolevel>1)
      CERR << "ppz was " << g << '\n';
  }

  void reduce(const polynome & p,const vectpoly & res,const vector<unsigned> & G,unsigned excluded,polynome & rem,environment * env){
    polynome TMP1(p.dim,p),TMP2(p.dim,p);
    reduce(p,res,G,excluded,rem,TMP1,TMP2,env);
  }

  // reduce with respect to itself the elements of res with index in G
  void reduce(vectpoly & res,vector<unsigned> G,environment * env){
    if (res.empty() || G.empty())
      return;
    polynome pred(res.front().dim,res.front());
    polynome TMP1(res.front().dim,res.front()),TMP2(res.front().dim,res.front());
    // reduce res
    for (unsigned i=0;i<G.size();++i){
      polynome & p=res[i];
      reduce(p,res,G,i,pred,TMP1,TMP2,env);
      swap(res[i].coord,pred.coord);
    }
  }

  void ppz(vectpoly & res){
    vectpoly::iterator it=res.begin(),itend=res.end();
    for (;it!=itend;++it)
      inplace_ppz(*it);
  }

  static void gbasis_update(vector<unsigned> & G,vector< pair<unsigned,unsigned> > & B,vectpoly & res,unsigned pos,environment * env){
    const polynome & h = res[pos];
    vector<unsigned> C;
    C.reserve(G.size());
    const index_m & h0=h.coord.front().index;
    index_t tmp1,tmp2;
    // C is used to construct new pairs
    // create pairs with h and elements g of G, then remove
    // -> if g leading monomial is prime with h, remove the pair
    // -> if g leading monomial is not disjoint from h leading monomial
    //    keep it only if lcm of leading monomial is not divisible by another one
    for (unsigned i=0;i<G.size();++i){
      if (res[G[i]].coord.empty() || disjoint(h0,res[G[i]].coord.front().index))
	continue;
      index_lcm(h0,res[G[i]].coord.front().index,tmp1); // h0 and G[i] leading monomial not prime together
      unsigned j;
      for (j=0;j<G.size();++j){
	if (i==j || res[G[j]].coord.empty())
	  continue;
	index_lcm(h0,res[G[j]].coord.front().index,tmp2);
	if (tmp1>=tmp2){
	  // found another pair, keep the smallest, or the first if equal
	  if (tmp1!=tmp2)
	    break; 
	  if (i>j)
	    break;
	}
      } // end for j
      if (j==G.size())
	C.push_back(G[i]);
    }
    vector< pair<unsigned,unsigned> > B1;
    B1.reserve(B.size()+C.size());
    for (unsigned i=0;i<B.size();++i){
      if (res[B[i].first].coord.empty() || res[B[i].second].coord.empty())
	continue;
      index_lcm(res[B[i].first].coord.front().index,res[B[i].second].coord.front().index,tmp1);
      if (!(tmp1>=h0)){
	B1.push_back(B[i]);
	continue;
      }
      index_lcm(res[B[i].first].coord.front().index,h0,tmp2);
      if (tmp2==tmp1){
	B1.push_back(B[i]);
	continue;
      }
      index_lcm(res[B[i].second].coord.front().index,h0,tmp2);
      if (tmp2==tmp1){
	B1.push_back(B[i]);
	continue;
      }
    }
    // B <- B union pairs(h,g) with g in C
    for (unsigned i=0;i<C.size();++i)
      B1.push_back(pair<unsigned,unsigned>(pos,C[i]));
    swap(B1,B);
    // Update G by removing elements with leading monomial >= leading monomial of h
    C.clear();
    C.reserve(G.size());
#if 0 // sort G by leading monomial increasing order
    bool pospushed=false;
    for (unsigned i=0;i<G.size();++i){
      if (!res[G[i]].coord.empty() && !(res[G[i]].coord.front().index>=h0)){
	// reduce res[G[i]] with respect to h
	reduce(res[G[i]],&h,&h+1,res[G[i]],env);
	if (!pospushed && res[G[i]].is_strictly_greater(res[G[i]].coord.front().index,h0)){
	  pospushed=true;
	  C.push_back(pos);
	}
	C.push_back(G[i]);
      }
      // NB: removing all pairs containing i in it does not work
    }
    if (!pospushed)
      C.push_back(pos);
#else // without sorting G
    for (unsigned i=0;i<G.size();++i){
      if (!res[G[i]].coord.empty() && !(res[G[i]].coord.front().index>=h0)){
	// reduce res[G[i]] with respect to h
	gen m;
	reduce(res[G[i]],&h,&h+1,res[G[i]],m,env);
	C.push_back(G[i]);
      }
      // NB: removing all pairs containing i in it does not work
    }
    C.push_back(pos);
#endif
    swap(C,G);
  }

  // first occurrence in v: i<0 not found, i>=0 means v[i]==idx
  int find(const vector<index_m> & v,const index_m & idx){
    unsigned debut=0,fin=unsigned(v.size()); // search in [debut,fin[
    if (v.empty() || i_lex_is_strictly_greater(v[0],idx))
      return -1;
    if (i_lex_is_strictly_greater(idx,v.back()))
      return -int(fin);
    for (;fin-debut>1;){
      unsigned i=(fin+debut)/2;
      if (i_lex_is_greater(v[i],idx)){
	if (v[i]==idx)
	  return i;
	fin=i;
      }
      else
	debut=i;
    }
    if (v[debut]==idx)
      return debut;
    return -int(fin);
  }

  void inplace_division(gen & a,const gen & b){
#if !defined USE_GMP_REPLACEMENTS && !defined BF2GMP_H
    if (a.type==_ZINT && a.ref_count()==1){
      if (b.type==_INT_ && mpz_divisible_ui_p(*a._ZINTptr,b.val)){
	if (b.val>0)
	  mpz_divexact_ui(*a._ZINTptr,*a._ZINTptr,b.val);
	else {
	  mpz_divexact_ui(*a._ZINTptr,*a._ZINTptr,-b.val);
	  mpz_neg(*a._ZINTptr,*a._ZINTptr);
	}
	return;
      }
      if (b.type==_ZINT && mpz_divisible_p(*a._ZINTptr,*b._ZINTptr)){
	mpz_divexact(*a._ZINTptr,*a._ZINTptr,*b._ZINTptr);
	return;
      }
    }
    if (a.type==_POLY && a.ref_count()==1){
      *a._POLYptr /= b;
      return;
    }
#endif
    a = rdiv(a,b,context0);
  }

  void inplace_multpoly(const gen & a,polynome & p){
    vector< monomial<gen> >::iterator jt=p.coord.begin(),jtend=p.coord.end();
    for (;jt!=jtend;++jt)
      type_operator_times(a,jt->value,jt->value);
  }

  void inplace_divpoly(polynome & p,const gen & a){
    vector< monomial<gen> >::iterator jt=p.coord.begin(),jtend=p.coord.end();
    for (;jt!=jtend;++jt)
      inplace_division(jt->value,a);
  }

  // (a*A+b*B)/c->B, in-place
  static void inplace_linear_combination(const gen & a,const vecteur & A,const gen & b,vecteur & B,const gen & c,int start,polynome & TMP1, polynome & TMP2){
    const_iterateur it=A.begin()+start,itend=A.end()-1;
    iterateur jt=B.begin()+start;
    gen tmp;
    for (;it!=itend;++jt,++it){
      type_operator_times(b,*jt,*jt);
      type_operator_times(a,*it,tmp);
      *jt += tmp;
      inplace_division(*jt,c);
    }
    // last operation is polynomial
    if (it->type==_POLY && jt->type==_POLY){
      *jt->_POLYptr *= b;
      TMP1 = *it->_POLYptr;
      inplace_multpoly(a,TMP1); // TMP1 *= a;
      TMP2.coord.clear();
      TMP1.TAdd(*jt->_POLYptr,TMP2);
      inplace_divpoly(TMP2,c); // TMP2 /= c;
      swap(TMP2.coord,jt->_POLYptr->coord);
    }
    else {
      type_operator_times(b,*jt,*jt);
      type_operator_times(a,*it,tmp);
      *jt += tmp;
      *jt = *jt/c;
    }
  }

  static void inplace_multvecteur(const gen & a,vecteur & A,int start=0){
    iterateur it=A.begin()+start,itend=A.end()-1;
    for (;it!=itend;++it){
      type_operator_times(a,*it,*it);
    }
    if (it->type==_POLY){
      inplace_multpoly(a,*it->_POLYptr);
      // *it->_POLYptr *= a;
    }
    else
      type_operator_times(a,*it,*it);      
  }

  static void inplace_divvecteur(vecteur & A,const gen & a,int start=0){
    iterateur it=A.begin()+start,itend=A.end()-1;
    for (;it!=itend;++it){
      inplace_division(*it,a);
    }
    if (it->type==_POLY){
      inplace_divpoly(*it->_POLYptr,a);
      // *it->_POLYptr /= a;
    }
    else
      inplace_division(*it,a);
  }

  vecteur coeffs(const polynome & p,vector<index_m> rmonomials,environment * env){
    vecteur res(rmonomials.size());
    for (unsigned k=0;k<p.coord.size();++k){
      int pos=find(rmonomials,p.coord[k].index);
      if (pos<0 || pos>=int(res.size()))
	return res; // FIXME error (should not happen)
      res[pos]=p.coord[k].value;
    }
    return res;
  }

  // Does not work if env->moduloon
  bool fglm_lex(const vectpoly & G,vectpoly & Glex,unsigned maxpow,environment * env,GIAC_CONTEXT){
    Glex.clear();
    if (G.empty())
      return true;
    const polynome & G0=G.front();
    unsigned dim=G0.dim;
    vector<index_m> rmonomials; 
    // rmonomials contains the list of indexes of monomials of reducted poly
    // they are sorted
    vector<unsigned> positions;
    // positions[k] is the column of the matrix corresponding to rmonomials[k]
    // that way we can quickly find a monomial in rmonomials (sorted) and
    // find the corresponding column in the matrix mat
    // the two last columns of mat are non-reduced and reduced polynomials
    matrice mat,matr; vecteur ligne; vecteur pivots;
    // rows of mat are made of coefficients wrt monomials of reduction wrt G
    // of the non reduced monomial (last col of G)
    // then new monomials may be added to rmonomials to take in account reduced[i]
    // adding new columns of 0 to mat
    // then reduction of last row by previous ones
    // ? and reduction of a column by the last row (TODO?)
    // with same linear combination on the (nonreduced) last column
    // if the last line is 0 -> new element nonreduced in Glex
    index_m idxm(dim);
    index_t idxt(dim),prev;
    polynome M(G0.dim,G0),R(G0.dim,G0),Rlex(G0.dim),TMP1(G0.dim),TMP2(G0.dim);
    M.coord.push_back(monomial<gen>(1,idxm));
    gen m;
    reduce(M,&G.front(),&G.back()+1,R,m,env);
    if (R.coord.empty()){
      Glex.push_back(M);
      return true;
    }
    rmonomials.push_back(idxm);
    positions.push_back(0);
    ligne.push_back(1);
    ligne.push_back(M);
    mat.push_back(ligne);
    idxt[dim-1]=1;
    for (;;){
      if (sum_degree(idxt)>int(maxpow))
	return false;
      // bool found=false;
      // reduce monomial w.r.t. G (G order)
      M.coord.clear();
      idxm=index_t(idxt);
      M.coord.push_back(monomial<gen>(1,idxm));
      if (debug_infolevel>0)
	CERR << CLOCK() << " reduce begin " << M << '\n';
      gen mprev=m;
      m=1;
      if (prev.empty())
	reduce(M,&G.front(),&G.back()+1,R,m,env); // m*M=<G>+R
      else {
	vector< monomial<gen> >::iterator it=R.coord.begin(),itend=R.coord.end();
	for (;it!=itend;++it){
	  *it=it->shift(idxt-prev);
	}
	reduce(R,&G.front(),&G.back()+1,R,m,env); // m*R=<G>+R
	m=mprev*m;
      }
      if (debug_infolevel>0)
	CERR << CLOCK() << " reduce end " << '\n';
      // 1st check if we need to add new monomials
      int pos;
      bool inserted=false;
      for (unsigned i=0;i<R.coord.size();++i){
	pos=find(rmonomials,R.coord[i].index);
	if (pos<0){
	  // set this monomial at column mat.size()
	  rmonomials.insert(rmonomials.begin()-pos,R.coord[i].index);
	  int c=int(mat.size());
	  for (unsigned j=0;j<positions.size();++j){
	    if (int(positions[j])>=c)
	      ++positions[j];
	  }
	  positions.insert(positions.begin()-pos,c);
	  for (unsigned j=0;j<mat.size();++j){
	    vecteur & l=*mat[j]._VECTptr;
	    l.insert(l.begin()+c,0);
	  }
	  inserted=true;
	}
      }
      if (debug_infolevel>0)
	CERR << CLOCK() << " end insert monomials" << '\n';
      // now make last matrix line
      ligne.clear();
      for (unsigned i=0;i<positions.size();++i)
	ligne.push_back(0);
      for (unsigned i=0;i<R.coord.size();++i){
	int pos=find(rmonomials,R.coord[i].index);
	if (pos<0 || pos>=int(ligne.size()))
	  return false; // (should not happen)
	ligne[positions[pos]]=R.coord[i].value;
      }
      swap(Rlex.coord,M.coord);
      Rlex *= m; // no need to sort here
      ligne.push_back(Rlex);
      mat.push_back(ligne);
      // Gauss row reduction on mat
      gen det,bareiss=1,piv,coeff;
      int li=0,lmax=int(mat.size()),c=0,cmax=int(mat.front()._VECTptr->size())-1;
      if (debug_infolevel>0)
	CERR << CLOCK() << " reduce line" << '\n';
      for (;li<lmax-1 && c<cmax;){
	vecteur & v=*mat[li]._VECTptr;
	piv=v[c];
	if (is_zero(piv,contextptr)){
	  // ERROR
	  CERR << "error" << '\n';
	  break;
	}
	vecteur & w =*mat[lmax-1]._VECTptr;
	coeff=w[c];
	// row combination of mat[lmax-1] and mat[p]
	if (is_zero(coeff,contextptr)){
	  gen x=piv/bareiss,num,den;
	  if (!is_one(x) && !is_minus_one(x)){
	    fxnd(x,num,den);
	    inplace_multvecteur(num,w,c+1);
	    if (!is_one(den))
	      inplace_divvecteur(w,den,c+1);
	  }
	}
	else {
	  w[c]=0;
	  inplace_linear_combination(-coeff,v,piv,w,bareiss,c+1,TMP1,TMP2);
	  // linear_combination(piv,*mat[lmax-1]._VECTptr,-coeff,*mat[li]._VECTptr,bareiss,*mat[lmax-1]._VECTptr,0.0,0);
	}
	bareiss=piv;
	++li;
	++c;
      }
#if 0 // creates 0 in column c==lmax-1
      if (li==lmax-1 && c==li && !is_zero(piv=(*mat[li]._VECTptr)[c],contextptr)){
	if (c)
	  bareiss=(*mat[c-1]._VECTptr)[c-1];
	else
	  bareiss=1;
	for (li=0;li<lmax-1;++li){
	  vecteur & w=*mat[li]._VECTptr;
	  coeff=w[c];
	  vecteur & v =*mat[lmax-1]._VECTptr;
	  if (is_zero(coeff,contextptr)){
	    gen x=piv/bareiss,num,den;
	    if (!is_one(x) && !is_minus_one(x)){
	      fxnd(x,num,den);
	      inplace_multvecteur(num,w,c+1);
	      if (!is_one(den))
		inplace_divvecteur(w,den,c+1);
	    }
	  }
	  else {
	    w[c]=0;
	    inplace_linear_combination(-coeff,v,piv,w,bareiss,c+1,TMP1,TMP2);
	  }
	}
      }
#endif
      const vecteur & l=*mat.back()._VECTptr;
      if (li==lmax-1 && c<cmax){
	// search in current line for first non-zero pivot
	// exchange columns
	if (is_zero(l[c],contextptr)){
	  for (pos=c+1;pos<cmax;++pos){
	    if (!is_zero(l[pos],contextptr)){ // if it does not happen, add to Glex
	      for (unsigned k=0;k<positions.size();++k){
		if (int(positions[k])==c)
		  positions[k]=pos;
		else {
		  if (int(positions[k])==pos)
		    positions[k]=c;
		}
	      }
	      for (unsigned k=0;k<mat.size();++k){
		vecteur & w = *mat[k]._VECTptr;
		swapgen(w[c],w[pos]);
	      }
	      break;
	    }
	  }
	}
      }
      if (li<lmax-1 && c<cmax){
	for (unsigned i=0;i<mat.size();++i){
	  vecteur & v=*mat[i]._VECTptr;
	  gen g=lgcd(v);
	  divvecteur(v,g,v);
	}
	mrref(mat,matr,pivots,det,0,int(mat.size()),0,int(mat.front()._VECTptr->size())-2,
	      /* fullreduction */0,0,true,RREF_BAREISS,0,context0);
	swap(mat,matr);
      }
      if (debug_infolevel>0)
	CERR << CLOCK() << " reduce line end" << '\n';
      // if last line is 0, add element to Glex and remove last line from mat
      for (pos=0;pos<int(l.size())-1;++pos){
	if (!is_zero(l[pos],contextptr))
	  break;
      }
      if (pos==int(l.size())-1){
	if (l.back().type!=_POLY)
	  return false; // should not happen
	Glex.push_back(*l.back()._POLYptr);
	ppz(Glex.back());
	if (debug_infolevel>0){
	  CERR << "Found element " << Glex.back() << '\n';
	}
	index_t tmp=l.back()._POLYptr->coord.front().index.iref();
	index_t tmp1(dim);
	tmp1[0]=tmp[0];
	if (tmp==tmp1){
	  reduce(Glex,env);
	  reverse(Glex.begin(),Glex.end());
	  return true;
	}
	mat.pop_back();
      }
      // compute next monomial using lex ordering
      pos=dim-1;
      prev=idxt;
      for (;pos>=0;--pos){
	++idxt[pos];
	idxm=idxt;
	// compare to Glex leading monomial, if >= to one of them -> change var
	unsigned j=0;
	for (;j<Glex.size();++j){
	  if (idxm>=Glex[j].coord.front().index)
	    break;
	}
	if (j==Glex.size())
	  break;
	prev.clear();
	idxt[pos]=0;
      }
      if (pos<0) // should not happen
	return true;
    }
    return true;
  }

#if 0
  // try to convert a basis G to a lex basis Glex
  bool fglm1_lex(const vectpoly & G,vectpoly & Glex,unsigned maxpow,environment * env,GIAC_CONTEXT){
    Glex.clear();
    if (G.empty())
      return true;
    const polynome & G0=G.front();
    unsigned dim=G0.dim;
    vector<index_m> monomials,rmonomials; 
    // monomials contains the list of indexes of input monomials
    vectpoly reduced; 
    // reduced[i] is the reduction wrt G of monomials[i]
    // rmonomials is the list of monomials of all reduced[i]
    // they are sorted in increasing lex order 
    index_m idxm(dim);
    index_t idxt(dim);
    polynome M(G0.dim,G0),R(G0.dim,G0);
    M.coord.push_back(monomial<gen>(1,idxm));
    gen m;
    reduce(M,&G.front(),&G.back()+1,R,m,env);
    if (R.coord.empty()){
      Glex.push_back(M);
      return true;
    }
    monomials.push_back(idxm);
    rmonomials.push_back(idxm);
    reduced.push_back(R);
    matrice lignes,syst,syst0;
    idxt[dim-1]=1;
    for (;;){
      if (sum_degree(idxt)>maxpow)
	return false;
      bool found=false;
      // reduce monomial w.r.t. G (G order)
      idxm=index_t(idxt);
      M.coord.front().index=idxm;
      gen m;
      if (debug_infolevel>0)
	CERR << CLOCK() << " reduce begin " << '\n';
      reduce(M,&G.front(),&G.back()+1,R,m,env);
      if (debug_infolevel>0)
	CERR << CLOCK() << " reduce end " << '\n';
      if (R.coord.empty()){
	Glex.push_back(M);
	break;
      }
      R /= m;
      // can we express the reduction as a linear combination of the preceding ones?
      // 1st check by updating rmonomial, if we need to add a monomial there answer is no
      int pos;
      bool inserted=false;
      for (unsigned i=0;i<R.coord.size();++i){
	// CERR << rmonomials << " " << R.coord[i].index << '\n';
	pos=find(rmonomials,R.coord[i].index);
	if (pos<0){
	  rmonomials.insert(rmonomials.begin()-pos,R.coord[i].index);
	  inserted=true;
	}
      }
      // if i==R.coord.size(), solve linear system to find linear. comb.
      if (!inserted){
	if (debug_infolevel>0){
	  if (R==M)
	    CERR << "R=M " ;
	  CERR << CLOCK() << " fill matrix " << '\n';
	}
	lignes.clear();
	lignes.reserve(reduced.size()+1);
	for (unsigned k=0;k<reduced.size();k++){
	  lignes.push_back(coeffs(reduced[k],rmonomials,env));
	}
	lignes.push_back(coeffs(R,rmonomials,env));
	int nunknown=lignes.size();
	vecteur B;
	mtran(lignes,syst);
	int neq=syst.size();
	bool checked=false;
#if 0	
	// scan lines of syst to simplify the system
	// if a line contains only 1 non-zero coeff (except last col)
	// we can determine the unknown of that column
	vecteur sol(nunknown,undef);
	vecteur syst1(syst);
	unsigned totalfound=0;
	for (;;){
	  unsigned found=0;
	  for (unsigned i=0;i<syst1.size();++i){
	    unsigned pos=-1;
	    const vecteur & current=*syst1[i]._VECTptr;
	    gen somme;
	    for (unsigned j=0;j<nunknown-1;j++){
	      if (is_zero(current[j],contextptr))
		continue;
	      if (sol[j]==undef){
		if (pos==-1)
		  pos=j;
		else {
		  pos=-1;
		  break;
		}
	      }
	      else
		somme += current[j]*sol[j];
	    }
	    if (pos!=-1){
	      sol[pos]=(current[nunknown-1]-somme)/current[pos];
	      syst1.erase(syst1.begin()+i);
	      --i;
	      ++found;
	      ++totalfound;
	      if (totalfound==nunknown-1)
		break;
	    }
	  }
	  if (found==0 || totalfound==nunknown-1)
	    break;
	}
	if (totalfound==nunknown-1){
	  sol[nunknown-1]=1;
	  checked=true;
	  if (is_zero(multmatvecteur(syst,sol),contextptr))
	    B=vecteur(1,sol);
	}
	// lignes.size()== number of unknowns, syst.size()=numbers of equations
	// first try to solve with number of equations=number of unknowns -1 ?
	// if the ker is dim 1 we can check that full_syst*ker[0]=0
	if (neq>nunknown){
	  syst0=vecteur(syst.begin(),syst.begin()+nunknown-1);
	  mker(syst0,B,contextptr);
	  if (B.size()!=1)
	    B.clear();
	  else {
	    checked=true;
	    if (!is_zero(multmatvecteur(syst,B),contextptr))
	      B.clear();
	  }
	}
#endif
	if (!checked){
	  gen m;
	  for (unsigned i=0;i<syst.size();++i){
	    lcmdeno(*syst[i]._VECTptr,m,context0);
	  }
	  if (debug_infolevel>0)
	    CERR << CLOCK() << " ker begin " << neq << "*" << nunknown << '\n';
	  mker(syst,B,contextptr);
	  if (debug_infolevel>0)
	    CERR << CLOCK() << " ker end " << '\n';
	}
	if (is_undef(B) || B.empty())
	  ;
	else {
	  // The last element of B must have a non-zero last component
	  vecteur Bend=*B.back()._VECTptr;
	  gen last=Bend.back();
	  if (!is_zero(last,contextptr)){
	    // solution found!
	    // make scalar product of Bend with reduced
	    polynome res(dim);
	    res.coord.push_back(monomial<gen>(last,idxm));
	    for (unsigned k=0;k<reduced.size();++k){
	      if (!is_zero(Bend[k],contextptr))
		res.coord.push_back(monomial<gen>(Bend[k],monomials[k]));
	    }
	    res.tsort();
	    m=1;
	    lcmdeno(res,m);
	    res *= m;
	    Glex.push_back(res);
	    if (debug_infolevel>0)
	      CERR << "Found element beginning with [x1,x2,...]^" << idxt << '\n';
	    // check if we are finished
	    index_t tmp=res.coord.front().index.iref();
	    index_t tmp1(dim);
	    tmp1[0]=tmp[0];
	    if (tmp==tmp1)
	      return true;
	    found=true;
	  }
	}
      }
      // if monomial not found
      // add idxm to the list of monomials and R to the list of reduced
      if (!found){
	monomials.push_back(idxm);
	reduced.push_back(R);
	change_monomial_order(reduced.back(),_PLEX_ORDER);
      }
      // compute next monomial using lex ordering
      pos=dim-1;
      for (;pos>=0;--pos){
	++idxt[pos];
	idxm=idxt;
	// compare to Glex leading monomial, if >= to one of them -> change var
	unsigned j=0;
	for (;j<Glex.size();++j){
	  if (idxm>=Glex[j].coord.front().index)
	    break;
	}
	if (j==Glex.size())
	  break;
	idxt[pos]=0;
      }
      if (pos<0) // should not happen
	return true;
    }
    return true;
  }
#endif

  bool is_zero_dim(vectpoly & G){
    if (G.empty())
      return false;
    unsigned dim=G.front().dim,count=0;
    for (unsigned i=0;i<G.size();++i){
      const index_m & idxm=G[i].coord.front().index;
      // check if idx is a power of an indeterminate
      for (unsigned j=0;j<dim;++j){
	if (idxm[j]==0)
	  continue;
	index_t idxt(dim);
	idxt[j]=idxm[j];
	if (idxm.iref()==idxt)
	  ++count;
	else
	  break;
      }
    }
    return count==dim;
  }

  bool gbasis_coeffs_merge(const vectpoly & gb,const vector<vectpoly> & coeffs,vectpoly & res){
    if (gb.size()!=coeffs.size() || coeffs.empty())
      return false;
    size_t gs=gb.size(),s=coeffs.front().size();
    res.reserve(gs*(s+1));
    for (size_t i=0;i<gs;++i){
      res.push_back(gb[i]);
      const vectpoly & src=coeffs[i];
      for (size_t j=0;j<s;j++)
        res.push_back(src[j]);
    }
    return true;
  }
  
  static bool gbasis_coeffs_sort(vectpoly & gb,vector<vectpoly> & coeffs,GIAC_CONTEXT){
    if (gb.size()!=coeffs.size() || coeffs.empty())
      return false;
    size_t gs=gb.size(),s=coeffs.front().size();
    for (size_t i=0;i<gs;++i){
      vectpoly & src=coeffs[i];
      if (gb[i].coord.empty()){
        src.erase(src.begin()+i);
        --i; --gs;
      }
      else
        src.push_back(gb[i]);
    }
    // sort
    sort(coeffs.begin(),coeffs.end(),sort_vect_vectpoly_t());
    if (increasing_power(contextptr))
      reverse(coeffs.begin(),coeffs.end());
    // recreate gb
    gb.resize(gs);
    for (size_t i=0;i<gs;++i){
      vectpoly & src=coeffs[i];
      gb[i]=src.back();
      src.pop_back();
    }
    return true;
  }
  
  static bool giac_gbasis(vectpoly & res,const gen & order_,environment * env,int modularcheck,int & rur,GIAC_CONTEXT,gbasis_param_t gbasis_param,vector<vectpoly> * coeffsptr=0){
    if (res.empty()) return true;
    int order,lexvars=0;
    if (order_.type==_VECT && order_._VECTptr->size()==2){
      if (order_._VECTptr->front().type==_INT_ && order_._VECTptr->back().type==_INT_){
	order=order_._VECTptr->front().val;
	lexvars=order_._VECTptr->back().val;
      }
      else return false;
    }
    else {
      if (order_.type!=_INT_)
	return false;
      order=order_.val;
    }
    if (order==_PLEX_ORDER || order==0){
      // try first a 0-dim ideal with REVLEX and conversion
      vectpoly resrev(res),reslex;
      for (unsigned k=0;k<resrev.size();++k)
	change_monomial_order(resrev[k],_REVLEX_ORDER);
      gbasis_param.eliminate_flag=false;
      if (!giac_gbasis(resrev,_REVLEX_ORDER,env,modularcheck,rur,contextptr,gbasis_param))
	return false;
      if (resrev.size()==1){
	reslex.swap(resrev);
	return true;
      }
      if (is_zero_dim(resrev) && fglm_lex(resrev,reslex,1024,env,context0)){
	reslex.swap(res);
	return true;
      }
    }
    if (order<0){
      order=-order;
      rur=1;
    }
    vector<int> coeffsperm(res.size());
    if (gbasis_param.reinject_begin==-1 && gbasis_param.reinject_end==-1){
      if (coeffsptr){ // sort, track permutation for coeffs reordering
        vector<polyptr_int> v(res.size());
        for (int i=0;i<v.size();++i){
          v[i].polyptr=&res[i];
          v[i].i=i;
        }
        sort(v.begin(),v.end());
        for (int i=0;i<v.size();++i)
          coeffsperm[i]=v[i].i;
        reverse(coeffsperm.begin(),coeffsperm.end());
        apply_permutation(res,coeffsperm);
      }
      else {
#ifdef GIAC_REDUCEMODULO
        //if (res.size()<=2*res.front().dim) reduce(res,env);
#else
        reduce(res,env);
#endif
        sort_vectpoly(res.begin(),res.end());
        reverse(res.begin(),res.end());
        if (debug_infolevel>6)
          res.dbgprint();
      }
    }
#if !defined GIAC_HAS_STO_38 && !defined FXCG && !defined KHICAS && !defined SDL_KHICAS // CAS38_DISABLED
    if (
#ifdef GIAC_64VARS
	1 || 
#endif
	 res.front().dim<=GROEBNER_VARS+1-(order!=_PLEX_ORDER)){
      vectpoly tmp;
      order_t order_={static_cast<short>(order),(unsigned char)(lexvars/256),(unsigned char)(lexvars)};
      if (!gbasis8(res,order_,tmp,env,modularcheck!=0,modularcheck>=2,rur,contextptr,gbasis_param,coeffsptr))
	return false;
      if (coeffsptr){ // ordering and null elements
        vector<int> invp(perminv(coeffsperm));
        for (int i=0;i<coeffsptr->size();++i)
          apply_permutation((*coeffsptr)[i],invp);
        gbasis_coeffs_sort(tmp,*coeffsptr,contextptr);
        res.swap(tmp);
        return true;
      }
      int i;
      for (i=0;i<tmp.size();++i){
	if (tmp[i].coord.empty())
	  break;
      }
      if (rur || i==tmp.size())
	tmp.swap(res);
      else {
	// remove 0
	res.clear();
	for (int i=0;i<tmp.size();++i){
	  if (!tmp[i].coord.empty())
	    res.push_back(tmp[i]);
	}
      }
      // reduce(res,env);
      if (!rur){
	sort_vectpoly(res.begin(),res.end());
	if (increasing_power(contextptr))
	  reverse(res.begin(),res.end());
      }
      return true;
    }
#endif
#if 0 // def BESTA_OS 
    bool notfound=true;
    for (;notfound && !interrupted;){
      if (debug_infolevel>6)
	res.dbgprint();
      notfound=false;
      vectpoly::const_iterator it=res.begin(),itend=res.end(),jt;
      vectpoly newres(res);
      for (;it!=itend && !interrupted;++it){
	for (jt=it+1;jt!=itend && !interrupted;++jt){
	  if (disjoint(it->coord.front().index,jt->coord.front().index))
	    continue;
	  polynome toadd(spoly(*it,*jt,env));
	  toadd=reduce(toadd,newres,env);
	  if (!toadd.coord.empty()){
	    newres.push_back(toadd); // should be at the right place
	    notfound=true;
	  }
	}
      }
      reduce(newres,env);
      swap(res,newres);
    }
#else
    // BP: What's wrong for besta here?
    vector<unsigned> G;
    vector< pair<unsigned,unsigned> > B;
    for (unsigned l=0;l<res.size();++l){
      gbasis_update(G,B,res,l,env);
    }
    for (;!B.empty() && !interrupted;){
      if (debug_infolevel>1)
	CERR << CLOCK() << " number of pairs: " << B.size() << ", base size: " << G.size() << '\n';
      // find smallest lcm pair in B
      index_t small0,cur;
      unsigned smallpos;
      int smalltd=RAND_MAX;
      for (smallpos=0;smallpos<B.size();++smallpos){
	if (!res[B[smallpos].first].coord.empty() && !res[B[smallpos].second].coord.empty())
	  break;
      }
      index_lcm(res[B[smallpos].first].coord.front().index,res[B[smallpos].second].coord.front().index,small0);
      for (unsigned i=smallpos+1;i<B.size();++i){
	if (res[B[i].first].coord.empty() || res[B[i].second].coord.empty())
	  continue;
	index_lcm(res[B[i].first].coord.front().index,res[B[i].second].coord.front().index,cur);
	int curtd=RAND_MAX; // total_degree(cur); // commented otherwise lex is '\n'ess
	if (curtd<smalltd 
	    || (curtd==smalltd && res.front().is_strictly_greater(small0,cur))
	    ){
	  smalltd=curtd;
	  swap(small0,cur); // small0=cur;
	  smallpos=i;
	}
      }
      pair<unsigned,unsigned> bk=B[smallpos];
      if (debug_infolevel>1 && (equalposcomp(G,bk.first)==0 || equalposcomp(G,bk.second)==0))
	CERR << CLOCK() << " reducing pair with 1 element not in basis " << bk << " from " << B << '\n';
      B.erase(B.begin()+smallpos);
      polynome h=spoly(res[bk.first],res[bk.second],env);
      if (debug_infolevel>1)
	CERR << CLOCK() << " reduce begin, pair " << bk << " remainder size " << h.coord.size() << '\n';
      reduce(h,res,G,unsigned(-1),h,env);
      if (debug_infolevel>1){
	if (debug_infolevel>2){ CERR << h << '\n'; }
	CERR << CLOCK() << " reduce end, remainder size " << h.coord.size() << '\n';
      }
      if (!h.coord.empty()){
	res.push_back(h);
	gbasis_update(G,B,res,int(res.size())-1,env);
	if (debug_infolevel>2)
	  CERR << CLOCK() << " basis indexes " << G << " pairs indexes " << B << '\n';
      }
    }
    vectpoly newres(G.size(),polynome(res.front().dim,res.front()));
    for (unsigned i=0;i<G.size();++i)
      swap(newres[i].coord,res[G[i]].coord);
    swap(res,newres);
    reduce(res,env);
    if (!env || !env->moduloon)
      ppz(res);
#endif
    sort_vectpoly(res.begin(),res.end());
    if (increasing_power(contextptr))
      reverse(res.begin(),res.end());
    return true;
  }

  vectpoly gbasis(const vectpoly & v,const gen & order,bool with_cocoa,int modular,environment * env,int & rur,GIAC_CONTEXT,gbasis_param_t gbasis_param,vector< vectpoly> * coeffsptr){
    if (v.size()<=1){
      return v;
    }
    vectpoly res(v);
#ifndef NO_STDEXCEPT
    try {
#endif
      if (with_cocoa && order.type==_INT_){
	// modular used as a synonym for with_f5
	bool ok=modular?f5(res,order):cocoa_gbasis(res,order);
	if (ok){
	  if (debug_infolevel>1)
	    CERR << res << '\n';
	  return res;
	}
      }
#ifndef NO_STDEXCEPT
    } catch (...){
      last_evaled_argptr(contextptr)=NULL;
      if (debug_infolevel)
	CERR << "Unable to compute gbasis with CoCoA" << '\n';
    }
#endif
    if (!giac_gbasis(res,order,env,modular,rur,contextptr,gbasis_param,coeffsptr))
      gensizeerr(gettext("Unable to compute gbasis with giac, perhaps dimension is too large"));
    return res;
  }

  gen remove_equal(const gen & f){
    if ( (f.type==_SYMB) && (f._SYMBptr->sommet==at_equal || f._SYMBptr->sommet==at_equal2 || f._SYMBptr->sommet==at_same ) ){
      vecteur & v=*f._SYMBptr->feuille._VECTptr;
      return v.front()-v.back();
    }
    if (f.type==_VECT)
      return apply(f,remove_equal);
    return f;
  }

  vecteur remove_equal(const_iterateur it,const_iterateur itend){
    vecteur conditions;
    conditions.reserve(itend-it);
    for (;it!=itend;++it){
      conditions.push_back(remove_equal(*it));
    }
    return conditions;
  }

  bool vecteur2vector_polynome(const vecteur & eq_in,const vecteur & l,vectpoly & eqp){
    // remove all denominators
    const_iterateur it=eq_in.begin(),itend=eq_in.end();
    for (;it!=itend;++it){
      gen n,d;
      fxnd(*it,n,d);
      if (n.type==_POLY){
	// should reordre n with total degree+revlex order here
	eqp.push_back(*n._POLYptr);
	continue;
      }
      if (!is_zero(n))
	return false;
    }
    return true;
  }

  vecteur gen2poly(const gen &g){
    if (g.type==_FRAC && g._FRACptr->num.type==_VECT)
      return multvecteur(inv(g._FRACptr->den,context0),*g._FRACptr->num._VECTptr);
    return gen2vecteur(g);
  }

  // Max number of attempt to find a random separation form in rur computation
  int rur_separate_max_tries=100;
  gen _rur_separate_max_tries(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG &&  g.subtype==-1) return  g;
    if (g.type==_VECT && g._VECTptr->empty())
      return rur_separate_max_tries;
    if (g.type==_INT_ && g.val>0)
      return rur_separate_max_tries=g.val;
    return gensizeerr(contextptr);
  }
  static const char _rur_separate_max_tries_s []="rur_separate_max_tries";
  static define_unary_function_eval2 (__rur_separate_max_tries,&_rur_separate_max_tries,_rur_separate_max_tries_s,&printasDigits);
  define_unary_function_ptr5( at_rur_separate_max_tries ,alias_at_rur_separate_max_tries ,&__rur_separate_max_tries,0,true);

  vecteur gsolve(const vecteur & eq_orig,const vecteur & var_orig,bool complexmode,int evalf_after,GIAC_CONTEXT){
    // replace variables in var_orig by true identificators
    vecteur var(var_orig);
    if (!lop(eq_orig,*at_unit).empty())
      *logptr(contextptr) << "Units are not supported"<<'\n';
    // check if the whole system is linear
    if (is_zero(derive(derive(remove_ineq(eq_orig),var,contextptr),var,contextptr),contextptr)){
      gen sol=_linsolve(makesequence(eq_orig,var),contextptr);
      if (sol.type==_VECT && (sol._VECTptr->empty() || sol._VECTptr->back().type==_STRNG))
	return *sol._VECTptr;
      return vecteur(1,sol);
    }
    iterateur it=var.begin(),itend=var.end();
    int s=int(itend-it); // # of unknowns
    if (s==0)
      return vecteur(1,gendimerr(contextptr));
    if (s==1){
      vecteur v=solve(eq_orig,var[0],complexmode,contextptr);
      for (unsigned i=0;i<v.size();++i){
	v[i]=makevecteur(v[i]);
      }
      return v;
    }
    bool ineq=has_op(eq_orig,*at_superieur_strict) || has_op(eq_orig,*at_inferieur_strict) || has_op(eq_orig,*at_superieur_egal) || has_op(eq_orig,*at_inferieur_egal);
    if (ineq){
      vector<int> Nassume(ineq_countassume(var,contextptr));
      int N=1;
      for (int i=0;i<Nassume.size();++i)
        N *= Nassume[i];
      for (int i=0;i<N;++i){
        vecteur eq(ineq_addassume(eq_orig,var,Nassume,i,contextptr)),sol;
        int r=solve_ineq(eq,var,sol,true/*allow recursion*/,contextptr);
        if (r<-1)
          return vecteur(1,gensizeerr(gettext("Unable to solve inequalities system")));
        if (r>0)
          return sol;
      }
      return vecteur(0);
    }
#if 0
    if (s>int(eq_orig.size())){
      *logptr(contextptr) << gettext("Warning: solving by reducing number of unknowns to number of equations: ") << var_orig << " -> " << vecteur(it,it+eq_orig.size()) << '\n';
      vecteur remvars=vecteur(it+eq_orig.size(),itend);
      vecteur res=gsolve(eq_orig,vecteur(it,it+eq_orig.size()),complexmode,evalf_after,contextptr);
      for (unsigned i=0;i<res.size();++i){
	if (res[i].type==_VECT)
	  res[i]=mergevecteur(*res[i]._VECTptr,remvars);
      }
      return res;
    }
#endif
    bool need_subst=false;
    vector<identificateur> tab_idnt(s);
    for (int i=0;it!=itend;++it,++i){
      if (it->type!=_IDNT){
	*it=tab_idnt[i]; 
	need_subst=true;
      }
    }
    vecteur eq(remove_equal(eq_orig.begin(),eq_orig.end()));
    if (need_subst)
      eq=subst(eq,var_orig,var,false,contextptr);
    if (approx_mode(contextptr)){
      if (_sort(lvar(eq),contextptr)!=_sort(lidnt(eq),contextptr)){
#ifdef HAVE_LIBGSL
	return makevecteur(msolve(eq,var,multvecteur(zero,var),_HYBRID_SOLVER,epsilon(contextptr),contextptr));
#else
	return vecteur(1,undef);
#endif
      }
      eq=*apply(eq,exact,contextptr)._VECTptr;
      evalf_after |= 1;
    }
    if (var.size()>1){
#if 1 // ATESTER
      if (eq.size()<=lidnt(eq).size()+3){
	// first check for linear dependencies -> substitutions
	gen a,b;
	for (unsigned i=0;i<eq.size();++i){
	  gen numeqi=_numer(eq[i],contextptr);
	  for (unsigned j=0;j<var.size();++j){
	    if (is_linear_wrt(numeqi,var[j],a,b,contextptr)){
	      if (j==0 && is_zero(a) && is_zero(b)){
		// suppress eq[i]
		eq.erase(eq.begin()+i);
		--i;
		break;
	      }
	      if (is_zero(derive(a,var,contextptr),contextptr) 
		  && !is_zero(simplify(a,contextptr),contextptr)){
		if (a!=1 && a!=-1){
		  // maybe eq[i] is linear wrt var[jj] for jj>j with a simpler a coeff
		  for (unsigned jj=j+1;jj<var.size();++jj){
		    gen aa,bb;
		    if (is_linear_wrt(numeqi,var[jj],aa,bb,contextptr) 
			&& is_zero(derive(aa,var,contextptr),contextptr) 
			&& !is_zero(simplify(aa,contextptr),contextptr)){
		      if (aa.islesscomplexthan(a)){
			j=jj;
			a=aa;
			b=bb;
		      }
		    }
		  }
		}
		// eq[i]=a*var[j]+b
		// replace var[j] by -b/a
		gen elimj=-b/a;
		vecteur eqs(eq);
		vecteur elim(var);
		eqs.erase(eqs.begin()+i);
		for (unsigned k=0;k<eqs.size();++k){
		  eqs[k]=_numer(subst(eqs[k],elim[j],elimj,false,contextptr),contextptr); // FIXME a solve_simplify should be called to simplify more, example solve([a=b*c,ln(a)=ln(b)+ln(c)],[a,b,c]);
		}
		elim.erase(elim.begin()+j);
		vecteur res=gsolve(eqs,elim,complexmode,evalf_after,contextptr);
		for (unsigned k=0;k<res.size();++k){
		  gen & resk=res[k];
		  if (resk.type==_VECT && resk._VECTptr->size()==elim.size()){
		    vecteur resmodif(*resk._VECTptr);
		    gen resval=subst(elimj,elim,resk,false,contextptr);
		    if (is_undef(resval)){
		      res.erase(res.begin()+k);
		      --k;
		      continue;
		    }
		    resmodif.insert(resmodif.begin()+j,resval);
		    resk=gen(resmodif,resk.subtype);
		  }
		  else
		    resk=gensizeerr(contextptr);
		}
                // quick check for undef in initial equations
                for (int k=0;k<res.size();++k){
                  gen resk=res[k];
                  gen resval=subst(eq,var,resk,false,contextptr);
                  if (has_undef(resval))
                    res.erase(res.begin()+k);
                }
		return res;
	      }
	      if (eq.size()==2 && var.size()==2 && is_zero(derive(derive(eq,var[j],contextptr),var[j],contextptr))){
		// add resultant and solve again
		gen resu=_resultant(makesequence(eq[0],eq[1],var[j]),contextptr);
		eq.push_back(resu);
	      }
	    } // end if is_linear
	  }
	}
      }
#endif
      vecteur ls=lvarfracpow(eq);
      if ( ((evalf_after & 4)==0) && !ls.empty()){
	// add equations and variables
	int s=int(ls.size())/3;
	vecteur substin,substout,equations,listvars;
	vector<int> poscheck;
	for (unsigned i=0;int(i)<s;++i){
	  gen lsvar=ls[3*i+2];
	  gen ls3i=subst(ls[3*i],substin,substout,false,contextptr);
	  if (equalposcomp(substin,lsvar))
	    continue;
	  substin.push_back(lsvar);
	  gen tmp("c__"+print_intvar_counter(contextptr),contextptr);
	  if (!(ls[3*i+1].val %2))
	    poscheck.push_back(int(var.size()+listvars.size()));
	  listvars.push_back(tmp);
	  substout.push_back(tmp);
	  equations.push_back(pow(tmp,ls[3*i+1],contextptr)-ls3i);
	}
	vecteur neweq=subst(eq,substin,substout,false,contextptr);
	vecteur newvar=mergevecteur(var,listvars);
	vecteur sol=gsolve(mergevecteur(neweq,equations),newvar,complexmode,evalf_after | 4,contextptr);
	// extract var part
	for (unsigned i=0;i<sol.size();++i){
	  if (sol[i].type==_VECT && sol[i]._VECTptr->size()==newvar.size()){
	    // check positivity for sqrt variables and so on
	    vecteur w=*sol[i]._VECTptr;
	    unsigned j;
	    for (j=0;j<poscheck.size();++j){
	      if (is_strictly_positive(-w[poscheck[j]],contextptr))
		break;
	    }
	    if (j<poscheck.size()){
	      sol.erase(sol.begin()+i);
	      --i;
	    }
	    else
	      sol[i]=vecteur(w.begin(),w.begin()+var.size());
	  }
	}
	return sol;
	//return vecteur(1,gensizeerr(contextptr));
      } // end fractional power code
      if (eq.size()<=3 &&
          (evalf_after & 4)==0){
        // check if one equation depends only on one unknown
        // disabled, otherwise can't solve
        // S:=[b^2+c^2+5*b*c-85,10*a*b^3+22*a^2*b^2+10*a^3*b-10a^3-10b^3-25*a*b^2-25a^2*b+5a^2*c+5b^2*c+25a*b*c-32a^2-32b^2+c^2-125a*b-35a-35b-7,a^4+c^4-d^4+10a*c^3+27a^2*c^2+10a^3*c-15d^3+5a^2+5c^2-53d^2+25a*c-15d,a^4-b^4-d^4-10b*d^3-27b^2*d^2-10b^3*d+70a^3+1328a^2-35b^2-35d^2-175b*d+3605a+2598,48d^4-10d^3-417d^2-2135d-7650];
        // re-enabled for solve([67000=(c)*((a)^(2007)), 3=(c)*((a)^(9))],[c, a]);
	for (unsigned i=0;i<eq.size();++i){
	  vecteur curv=lidnt(eq[i]);
	  gen curvvar=_intersect(makesequence(curv,var),contextptr);
	  if (curvvar.type==_VECT && curvvar._VECTptr->size()==1){
	    gen curvar= curvvar._VECTptr->front();
	    int varn=equalposcomp(var,curvar);
	    var.erase(var.begin()+varn-1);
	    vecteur sol=solve(eq[i],curvar,complexmode,contextptr);
	    vecteur res;
	    if (!is_undef(res)){
	      eq.erase(eq.begin()+i);
	      for (unsigned j=0;j<sol.size();++j){
		gen eq1=subst(eq,curvar,sol[j],false,contextptr),tmp; eq1=ratnormal(eq1);
		if (var.size()==1){
		  if (eq1.type==_VECT && eq1._VECTptr->size()==1)
		    tmp=solve(eq1._VECTptr->front(),var.front(),complexmode,contextptr);
		  else
		    tmp=solve(eq1,var.front(),complexmode,contextptr);
		}
		else
		  tmp=solve(eq1,var,complexmode,contextptr);
		if (tmp.type==_VECT){
		  for (unsigned k=0;k<tmp._VECTptr->size();++k){
		    vecteur solv=gen2vecteur((*tmp._VECTptr)[k]);
		    if (int(solv.size())<varn-1)
		      return vecteur(1,gensizeerr(contextptr));
		    solv.insert(solv.begin()+varn-1,sol[j]);
		    res.push_back(solv);
		  }
		}
	      } // for j (solutions in the single variable)
	      return res;
	    } // end if !is_undef(res)
	  } // end curvar size==1
	} // end for on all equations
      } // end if evalf_after | 4 == 0
    } // end var size >1
    bool convertapprox=has_num_coeff(eq);
    gen eqs=eq;
    if (convertapprox)
      eq=*exact(evalf(eq,1,contextptr),contextptr)._VECTptr;
    // check rational
    int varn=0;
    for (it=var.begin();it!=itend;++it,++varn){
      if (it->type!=_IDNT) // should not occur!
	return vecteur(1,gensizeerr(gettext("Bad var ")+it->print(contextptr)));
      vecteur l(rlvarx(eq,*it)),lerr(l);
      if (l.size()>1){
	gen tmp=rationalize(eq,*it,contextptr);
	if (tmp.type==_VECT){
	  eq=*tmp._VECTptr;
          // if you change below, check with solve([z^3=y^2*x, sqrt(x)=log10(3*z^2), 5*z*x=y^x], [x, y, z]);solve([((2)*(exp(((x)^(2))+((y)^(2)))))-(16), ((-16)*((y)^(2)))+((2)*(exp(((x)^(2))+((y)^(2)))))],[x,y]);
	  l=lvarx(eq,*it); 
	  if (l.size()==1
              && l[0]!=*it
              ){
	    // solve with respect to l[0] then extract *it
	    gen newvar=l.front();
	    gen tmpeq=subst(eq,newvar,*it,false,contextptr);
	    if (tmpeq.type==_VECT){
	      vecteur res0=gsolve(*tmpeq._VECTptr,var,complexmode,0,contextptr);
	      // solve newvar=varn-th component of each solution in res
	      vecteur res;
	      int i=0;
	      for (;i<int(res0.size());++i){
		gen cur=res0[i];
		if (cur.type!=_VECT || cur._VECTptr->size()<varn)
		  break;
		vecteur curv=*cur._VECTptr;
		gen val=curv[varn];
		vecteur resval=solve(newvar-val,*it,complexmode,contextptr);
		for (int j=0;j<int(resval.size());++j){
		  curv[varn]=resval[j];
		  res.push_back(curv);
		}
	      }
	      if (i==res0.size()){
		// subst original var with result, for example for solve([ exp(x^2 + y^2) =8, exp(x^2 + y^2)=8*y^2],[x,y]);
		for (int j=0;j<res.size();++j){
		  res[j]=subst(res[j],var_orig,res[j],false,contextptr);
		}
		return res;
	      }
	    }
	  }
	}
	return vecteur(1,gensizeerr(gen(lerr).print(contextptr)+gettext(" is not rational w.r.t. ")+it->print(contextptr)));
      }
    }
    // extract gcd for systems like eq1:=4*v5*v6^2-6*v6^3-12*v5*v6+23*v6^2-15*v6; eq2:=12*v5^2-10*v5*v6-12*v6^2-9*v5+46*v6-30; solve([eq1,eq2],[v5,v6]);
    if (eq.size()>1){
      gen G=0;
      for (int i=0;i<eq.size();++i){
        G=gcd(G,eq[i],contextptr);
        if (G.type!=_SYMB)
          break;
      }
      if (G.type==_SYMB){
        int i;
        for (i=0;i<var.size();++i){
          if (!is_constant_wrt(G,var[i],contextptr))
            break;
        }
        if (i<var.size()){
          vecteur res=gsolve(vecteur(1,G),var,complexmode,evalf_after,contextptr);
          for (int i=0;i<eq.size();++i)
            eq[i]=normal(eq[i]/G,contextptr);
          vecteur sol=gsolve(eq,var,complexmode,evalf_after,contextptr);
          res=mergevecteur(res,sol);
          return res;
        }
      }
    }
    // if one equation factors recurse with each factor
    for (size_t i=0;i<eq.size();++i){
      vecteur vi=factors(eq[i],var[0],contextptr);
      if (vi.size()/2==0) continue;
      if (is_cinteger(vi[vi.size()-2])){
	vi.pop_back();
	vi.pop_back();
      }
      if (vi.size()>2){
	vecteur res;
	for (size_t j=0;j<vi.size();j+=2){
	  if (is_strictly_positive(vi[j+1],contextptr)){
	    eq[i]=vi[j];
	    vecteur sol=gsolve(eq,var,complexmode,evalf_after,contextptr);
	    res=mergevecteur(res,sol);
	  }
	}
	return res;
      }
    }
    // end recurse if one equation factors
    int varsize=int(var.size());
#if 1 // trying with rational univariate rep., assuming radical ideal of dim 0
    double eps=epsilon(contextptr);
    if (varsize==2 && eq.size()>=2){ // try by resultant
      //eq=*eqs._VECTptr;
      gen r=0;
      for (size_t i=1;i<eq.size();++i)
	r=gcd(r,_resultant(makesequence(eq[0],eq[i],var[0]),contextptr),contextptr);
      if (!is_zero(r)){
	// solve r w.r.t. var[1]
	vecteur S,res;
	if (convertapprox){
	  gen p=_symb2poly(makesequence(r,var[1]),contextptr);
	  p=evalf(p,1,contextptr);
	  p=_proot(p,contextptr);
	  if (p.type!=_VECT) return vecteur(1,gensizeerr(contextptr));
	  S=*p._VECTptr;
	}
	else
	  S=solve(r,var[1],complexmode,contextptr);
	for (int i=0;i<int(S.size());++i){
	  gen y=S[i];
	  if (has_num_coeff(y)){
	    vecteur T=solve(subst(eq[0],var[1],y,false,contextptr),var[0],complexmode,contextptr);
	    for (size_t k=1;k<eq.size();++k){
	      vecteur newT;
	      for (int j=0;j<int(T.size());++j){
		gen x=T[j];
		gen tst=subst(subst(eq[k],var[1],y,false,contextptr),var[0],x,false,contextptr);
		if (is_greater(1e-6,abs(tst,contextptr),contextptr))
		  newT.push_back(x);
	      }	    
	      newT.swap(T);
	    }
	    for (int k=0;k<T.size();++k)
	      res.push_back(makevecteur(T[k],y));
	  }
	  else {
	    gen G=subst(eq[0],var[1],y,false,contextptr);
	    for (size_t j=1;j<eq.size();++j)
	      G=gcd(G,subst(eq[j],var[1],y,false,contextptr),contextptr);
	    vecteur T=solve(G,var[0],complexmode,contextptr);
	    for (int j=0;j<int(T.size());++j){
	      res.push_back(makevecteur(T[j],y));
	    }
	  }
	}
	return res;
      } // end resultant not 0
    } // end #var=2
    if (varsize<=GROEBNER_VARS && varsize<=int(eq.size())){
      gen G=_gbasis(makesequence(eq,var,change_subtype(_RUR_REVLEX,_INT_GROEBNER)),contextptr);
      if (G.type==_VECT && G._VECTptr->size()==1){
	if (!is_zero(G._VECTptr->front()))
	  return vecteur(0); // system was equivalent to 1=0
	gensizeerr("solve.cc internal error");
      }
      if (G.type==_VECT && G._VECTptr->size()==var.size()+4 && G._VECTptr->front().type==_INT_ && G._VECTptr->front().val==_RUR_REVLEX){
	vecteur Gv=*G._VECTptr,S;
	gen rurvar=var.front();
	if (Gv[1].type==_IDNT) 
	  rurvar=Gv[1];
#if 1
	if (rur_do_certify!=0)
	  *logptr(contextptr) << "Rational univariate representation is not certified, run rur_certify(1) to certify" << '\n';
#else
	if (proba_epsilon(contextptr)<1e-16){
	  // check the solution replace var by G[4..end]/G[3] in eq and divide by G[2]
	  for (unsigned i=0;i<eq.size();++i){
	    gen check=subst(eq[i],var,divvecteur(vecteur(Gv.begin()+4,Gv.end()),Gv[3]),false,contextptr);
	    check=_numer(check,contextptr);
	    check=_rem(makesequence(check,Gv[2],rurvar),contextptr);
	    if (!is_zero(check,contextptr))
	      *logptr(contextptr) << "Warning, solution does not seem to cancel " << eq[i] << '\n';
	  }
	}
	else
	  *logptr(contextptr) << "Rational univariate representation is not certified, set proba_epsilon:=0 to certify" << '\n';
#endif
	int deg=_degree(makesequence(Gv[2],rurvar),contextptr).val;
	if (evalf_after & 1){
	  gen pol=Gv[2],tmp;
	  if (complexmode){
	    // FIXME: implement interval arit. for complexes
	    if (0 && (evalf_after & 2)){
	      tmp=complexroot(makesequence(pol,eps),true,contextptr);
	    }
	    else {
	      if (deg>28)
		tmp=_proot(makesequence(pol,rurvar,deg/2),contextptr);
	      else
		tmp=_proot(makesequence(pol,rurvar),contextptr);
	    }
	  }
	  else {
	    if (deg>28){
	      double eps2=std::pow(2.0,-deg/2);
	      if (eps2<eps)
		eps=eps2;
	    }
	    tmp=complexroot(makesequence(pol,eps),false,contextptr); // realroots
	  }
	  // CERR << tmp << '\n';
	  if (tmp.type==_VECT)
	    S=*tmp._VECTptr;
	}
	if (S.empty()){
	  // G[1] separating, G[2]=minpoly, G[3]=derivative, G[4..end]=solution
	  if (debug_infolevel)
	    *logptr(contextptr) << "Solutions = substitute roots of " << Gv[2] << " in " << vecteur(Gv.begin()+4,Gv.end()) << "/(" << Gv[3] << ")" << '\n';
          if (deg>14){
            // FIXME precision should be adapted for large degrees (and intervals should be used, like above with evalf_after)
            int d=decimal_digits(contextptr);
            decimal_digits(d+deg,contextptr);
            S=solve(Gv[2],rurvar,complexmode,contextptr);
            decimal_digits(d,contextptr);
          }
          else
            S=solve(Gv[2],rurvar,complexmode,contextptr); 
	}
	vecteur res;
	modpoly minp=gen2poly(_symb2poly(makesequence(Gv[2],rurvar),contextptr));
	modpoly minp1=derivative(minp);
	modpoly denp=gen2poly(_symb2poly(makesequence(Gv[3],rurvar),contextptr));
	vector<modpoly> numv;
	for (unsigned i=4;i<Gv.size();++i){
	  numv.push_back(gen2poly(_symb2poly(makesequence(Gv[i],rurvar),contextptr)));
	}
	for (unsigned i=0;i<S.size();++i){
	  gen s=S[i];
	  if (s.type==_VECT && s._VECTptr->size()==2){
	    s=s._VECTptr->front();
	    if (s.type==_VECT && s._VECTptr->size()==2){
#if 1 // arithmetic interval
	      gen l=s._VECTptr->front(),r=s._VECTptr->back();
	      vecteur den;
	      // accuracy
	      int nbits=0;
	      gen deuxn=1,lr=r-l;
	      gen Eps=pow(gen(10),int(std::floor(std::log10(eps))),contextptr);
	      for (;is_greater(1,deuxn*lr,contextptr);){
		++nbits;
		deuxn=plus_two*deuxn;
	      }
	      for (bool stopnextiter=false;;){
		if (!stopnextiter)
		  den=horner_interval(denp,l,r);
		gen den0=den[0],den1=den[1];
		bool lpos=is_positive(den0,contextptr),rpos=is_positive(den1,contextptr);
		if ( !(lpos ^ rpos) && is_greater(Eps/3,abs(den[1]/den[0]-1,contextptr),contextptr)){
		  // try numerators
		  unsigned pos=0;
		  vecteur Hs;
		  for (;pos<numv.size();++pos){
		    vecteur num=horner_interval(numv[pos],l,r);
		    if (!stopnextiter && is_greater(abs(num[1]/num[0]-1,contextptr),Eps/3,contextptr))
		      break;
		    gen num0=num[0],num1=num[1],a=makesequence(num0/den0,num0/den1,num1/den0,num1/den1);
#ifdef HAVE_LIBMPFI
		    if (evalf_after & 2)
		      Hs.push_back(eval(gen(makevecteur(_min(a,contextptr),_max(a,contextptr)),_INTERVAL__VECT),1,contextptr));
		    else
		      Hs.push_back(accurate_evalf((num0+num1)/(den0+den1),nbits));
#else
		      Hs.push_back(accurate_evalf((num0+num1)/(den0+den1),nbits));
#endif
		  }
		  if (pos==numv.size()){
		    res.push_back(Hs);
		    break;
		  }
		} // end if
		s=(l+r)/2;
		in_round2(s,deuxn,nbits);
		gen minps=horner(minp,s,0,false);
		if (is_exactly_zero(minps))
		  break;
		s=s-minps/horner(minp1,s,0,false);
		nbits*=2;
		deuxn=deuxn*deuxn;
		in_round2(s,deuxn,nbits);
		lr=int(minp.size()-1)*abs(horner(minp,s,0,false)/horner(minp1,s,0,false),contextptr); // warranted bounds
		double lrd=evalf_double(lr,1,contextptr)._DOUBLE_val;
		if (is_greater(lrd,1e-300,contextptr))
		  lr=pow(plus_two,int(std::log(lrd)/std::log(2.))+1);
		if (lrd==0)
		  stopnextiter=true;
		else {
		  l=s-lr;
		  r=s+lr;
		  lr=2*lr;
		}
	      } // end for(;;)
	      continue;
#else
	      s=(s._VECTptr->front()+s._VECTptr->back())/2;
	      if (eps<1e-14)
		s=accurate_evalf(s,int(-3.2*std::log(eps)));
	      else
		s=evalf_double(s,1,contextptr);
#endif
	    }
	  }
	  if (!complexmode && !is_zero(im(s,contextptr),contextptr))
	    continue;
	  vecteur Hs;
#ifdef HAVE_LIBMPFR
	  bool sdouble=(s.type==_DOUBLE_ || (s.type==_CPLX && s._CPLXptr->type==_DOUBLE_));
	  if (sdouble)
	    s=accurate_evalf(s,60); // ,giacmax(deg,60));
	  if ((s.type==_REAL || (s.type==_CPLX && s._CPLXptr->type==_REAL)) )
	    sdouble=true;
#else
	  bool sdouble=false;
#endif
	  gen den=_horner(makesequence(Gv[3],s,rurvar),contextptr);
	  for (unsigned j=4;j<Gv.size();++j){
	    Hs.push_back(recursive_normal(_horner(makesequence(Gv[j],s,rurvar),contextptr)/den,contextptr));
	    if (sdouble)
	      Hs.back()=evalf_double(Hs.back(),1,contextptr);
	  }
	  res.push_back(Hs);
	}
	return res;
      }
    }
#endif
#if 0
    // Trying with a partial elimination
    // Add a variable t linear combination of x1,...,xn with random small integers coeffs
    // eliminate all variables except t (and params) with revlex/revlex ordering
    // This returns a polynomial f in t in the Groebner basis
    // and in generic situations, it will return a basis of size n+1
    // where f is first basis element, and all other are degree 1 in xn...x1
    // therefore can be used to express xn...x1 in terms of t a root of f
    /* Example solve [24*u*z-u^2-z^2-u^2*z^2-13,24*y*z-y^2-z^2-y^2*z^2-13,24*u*y-u^2-y^2-u^2*y^2-13] for [u,y,z]:
       I:=[24*u*z-u^2-z^2-u^2*z^2-13,24*y*z-y^2-z^2-y^2*z^2-13,24*u*y-u^2-y^2-u^2*y^2-13,t-u-2y+z];
       G:=gbasis(I,[u,y,z]); // eliminates t
       H:=greduce([u,y,z],G,[u,y,z]); // u,y,z in terms of t
       rem(subst(I[0],[u,y,z],H),G[0],t); // check that rem is indeed 0
       S:=solve(G[0],t);
       sol:=map(S,s->normal(map(H,h->horner(h,s,t))));
       size(sol);
       normal(subst(I[0..2],[u,y,z],sol[0])); // check a solution
    */
    // Should be improved using global revlex ordering and 
    // partial FGLM i.e. find min poly on t, then express f'(T)*each variable
    // in term of powers of T up to degree(f)-1
    // first try, with the variables
    if (varsize<=11 && varsize==eq.size()){
      vecteur H,res; gen G,caspart;
      // retry with a random linear combination of the variables
      gen vart=identificateur("gsolve_t");
      for (unsigned k=0;k<varsize;++k){
	gen T=vart-var[k];
	vecteur eqv=gen2vecteur(eq);
	eqv.push_back(T);
	G=_gbasis(makesequence(eqv,var),contextptr);
	if (G.type==_VECT && G._VECTptr->size()>=varsize+1){ 
	  H = vecteur(G._VECTptr->begin()+1,G._VECTptr->begin()+varsize+1);
	  // check whether we have a linear system
	  gen M=derive(H,var,contextptr);
	  if (!is_zero(derive(M,var,contextptr),contextptr))
	    H.clear();
	  else {
	    // additional cases if det(M)==0
	    M=_det(M,contextptr);
	    M=ratnormal(M/gcd(M,derive(M,*vart._IDNTptr,contextptr)),contextptr);
	    if (_degree(makesequence(M,vart),contextptr)>2)
	      H.clear();
	    else {
	      caspart=_solve(makesequence(M,vart),contextptr);
	      if (caspart.type!=_VECT)
		H.clear();
	      else {
		vecteur casv=*caspart._VECTptr;
		for (unsigned j=0;j<casv.size();++j){
		  // solve with var[k]=casv[j]
		  gen eqpart=subst(eq,var[k],casv[j],false,contextptr);
		  vecteur varpart(var);
		  varpart.erase(varpart.begin()+k);
		  vecteur solpart=solve(eqpart,varpart,complexmode,contextptr);
		  for (unsigned l=0;l<solpart.size();++l){
		    gen solpartl=solpart[l];
		    if (solpartl.type==_VECT)
		      solpartl._VECTptr->insert(solpartl._VECTptr->begin()+k,casv[j]);
		  }
		  res=mergevecteur(res,solpart);
		}
	      }
	    }
	  }
	}
	if (H.size()==varsize)
	  break;
      }
      if (H.size()!=varsize){
	caspart=vecteur(0);
	for (unsigned essai=0;essai<10;++essai){
	  H.clear();
	  gen hasard;
	  if (essai==0){
	    vecteur v;
	    for (unsigned j=0;j<varsize;++j){
	      v.push_back(int(j)%3-1);
	    }
	    hasard=v;
	  }
	  else {
	    hasard=_ranm(int(var.size()),contextptr);
	    hasard=_iquo(makesequence(hasard,int(100/(essai+1))),contextptr);
	  }
	  gen T=vart-dotvecteur(hasard,var);
	  vecteur eqv=gen2vecteur(eq);
	  eqv.push_back(T);
	  *logptr(contextptr) << "Trying " << eqv << '\n';
	  G=_gbasis(makesequence(eqv,var),contextptr);
	  if (G.type==_VECT && G._VECTptr->size()>=varsize+1){ // bingo (probably)
	    H = vecteur(G._VECTptr->begin()+1,G._VECTptr->begin()+varsize+1);
	    gen M=derive(H,var,contextptr);
	    vecteur var1(var);
	    var1.push_back(vart);
	    if (!is_zero(derive(M,var1,contextptr),contextptr))
	      H.clear();
	  }
	  if (H.size()==varsize)
	    break;
	}
      }
      if (H.size()==varsize){
	H=linsolve(H,var,contextptr);
	*logptr(contextptr) << "map(proot(" <<subst(G[0],vart,vx_var,false,contextptr) << "),r->map(" << subst(H ,vart,vx_var,false,contextptr) << ",h->horner(h,r,"<<vx_var<<"))" << '\n';
	vecteur S=solve(G[0],vart,complexmode,contextptr);
	for (unsigned i=0;i<S.size();++i){
	  gen s=S[i];
	  if (caspart.type==_VECT && equalposcomp(*caspart._VECTptr,s))
	    continue;
	  vecteur Hs;
	  for (unsigned j=0;j<H.size();++j){
	    Hs.push_back(recursive_normal(_horner(makesequence(H[j],s,vart),contextptr),contextptr));
	  }
	  res.push_back(Hs);
	}
	return res;
      }
    }
#endif
    vecteur l(1,var);
    alg_lvar(eq,l);
    // convert eq to polynomial
    vecteur eq_in(*e2r(eq,l,contextptr)._VECTptr);
    vectpoly eqp;
    // remove all denominators
    it=eq_in.begin();
    itend=eq_in.end();
    for (;it!=itend;++it){
      gen n,d;
      fxnd(*it,n,d);
      if (n.type==_POLY){
	// should reordre n with total degree+revlex order here
	eqp.push_back(*n._POLYptr);
	continue;
      }
      if (!is_zero(n,contextptr))
	return vecteur(0); // no solution since cst equation
    }
    int rur=0;
    gbasis_param_t gbasis_param={false,-1,-1,-1,true,false,false,-1,vector<int>(0)};
    vectpoly eqpr(gbasis(eqp,_PLEX_ORDER,/* cocoa */false,/* f5 */ false,/*environment * */0,rur,contextptr,gbasis_param,0));
    // should reorder eqpr with lex order here
    // solve from right to left
    sort_vectpoly(eqpr.begin(),eqpr.end());
    reverse(eqpr.begin(),eqpr.end());
    // reverse(eqpr.begin(),eqpr.end());
    vecteur sols(1,vecteur(0)); // sols=[ [] ]
    vectpoly::const_iterator jt=eqpr.begin(),jtend=eqpr.end();
    for (;jt!=jtend;++jt){
      // the # of found vars is the size of sols.front()
      if (sols.empty())
	break;
      vecteur newsols;
      gen g(r2e(*jt,l,contextptr));
      const_iterateur st=sols.begin(),stend=sols.end();
      for (;st!=stend;++st){
	vecteur & stv = *st->_VECTptr;
	if (equalposcomp(lidnt(stv),undef))
	  continue;
	int foundvars=int(stv.size());
        gen tmpg=subst(g,vecteur(var.end()-foundvars,var.end()),*st,false,contextptr);
        tmpg=ratnormal(tmpg,contextptr);
        tmpg=ratnormal(tmpg,contextptr);
        if (tmpg.type!=_SYMB){
          if (is_zero(tmpg)){ // 0==0 is verified
            newsols.push_back(*st);
            continue;
          }
          return vecteur(0); // no solution
        }
	gen curgf=_factors(tmpg,contextptr);
	if (curgf.type!=_VECT) return vecteur(1,gensizeerr(contextptr));
	const_iterateur curgfit=curgf._VECTptr->begin(),curgfend=curgf._VECTptr->end();
	for (;curgfit!=curgfend;curgfit+=2){
	  vecteur current=*st->_VECTptr;
	  foundvars=int(st->_VECTptr->size());
	  gen curg=*curgfit;
          curg=normalize_sqrt(curg,contextptr);
	  gen x;
	  int xpos=0;
	  // First search in current an identifier curg depends on
	  for (;xpos<foundvars;++xpos){
	    x=current[xpos];
	    if (x==var[s-foundvars+xpos] && !is_zero(derive(curg,x,contextptr),contextptr) )
	      break;
	  }
	  if (xpos==foundvars){
	    xpos=0;
	    // find next var g depends on 
	    for (;foundvars<s;++foundvars){
	      x=var[s-foundvars-1];
	      current.insert(current.begin(),x);
	      if (!is_zero(derive(curg,x,contextptr),contextptr))
		break;
	    }
	    if (s==foundvars){
	      if (is_zero(simplify(curg,contextptr),contextptr))
		newsols.push_back(current);
	      continue;
	    }
	  }
	  // solve
	  vecteur xsol(solve(curg,*x._IDNTptr,complexmode,contextptr));
	  const_iterateur xt=xsol.begin(),xtend=xsol.end();
	  for (;xt!=xtend;++xt){
	    // current[xpos]=*xt;
	    if (is_inequation(*xt) || xt->is_symb_of_sommet(at_and) || xt->is_symb_of_sommet(at_ou)){ // FIXME is_inequation
	      gen id=lidnt(*xt);
	      if (id.type==_VECT && id._VECTptr->size()==1){
		id=id._VECTptr->front();
		gen idval=assumeeval(id,contextptr);
		giac_assume(*xt,contextptr);
		gen curval=eval(recursive_normal(current,contextptr),1,contextptr);
		restorepurge(idval,id,contextptr);
		newsols.push_back(curval);
	      }
	      else
		newsols.push_back(current);
	    }
	    else
	      newsols.push_back(subst(current,*x._IDNTptr,*xt,false,contextptr));
	  }
	} // end for curfit!=curfitend
      } // end for (;st!=stend;)
      sols=newsols;
    } // end for jt!=jtend
    // Add var at the beginning of each solution of sols if needed
    it=sols.begin(); 
    itend=sols.end();
    for (;it!=itend;++it){
      int ss=int(it->_VECTptr->size());
      if (ss<s)
	*it=mergevecteur(vecteur(var.begin(),var.begin()+s-ss),*it->_VECTptr);
    }
    if (need_subst)
      sols=subst(sols,var,var_orig,false,contextptr);
#if 1
    // Do a fast subst in eq_orig and check if there is an undef, if not consider it a solution
    vecteur sol0(sols);
    sols.clear();
    for (unsigned i=0;i<sol0.size();++i){
      gen val=subst(eq_orig,var_orig,sol0[i],false,contextptr);
      if (!equalposcomp(lidnt(val),undef))
	sols.push_back(sol0[i]);
    }
#endif
    if (convertapprox)
      sols=*evalf_VECT(sols,0,1,contextptr)._VECTptr;    
    return sols;
  }

  gen _gbasis_reinject(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG &&  g.subtype==-1) return  g;
    if (g==0){
      int tmp1=gbasis_stop;
      int tmp2=gbasis_logz_age_sort;
      gbasis_stop=0;
      gbasis_logz_age_sort=0;
      return makevecteur(tmp1,tmp2);
    }
    if (g.type==_INT_ && g.val<=2){
      if (g.val<0){
	int tmp=gbasis_stop;
	gbasis_stop=g.val;
	return tmp;
      }
      int tmp=gbasis_logz_age_sort;
      gbasis_logz_age_sort=g.val;
      return tmp;
    }
    gen args(evalf_double(g,1,contextptr));
    double old=gbasis_reinject_ratio,oldtime=gbasis_reinject_speed_ratio;
    if (g.type==_DOUBLE_){
      gbasis_reinject_ratio=g._DOUBLE_val<=0?0:g._DOUBLE_val;
      return old;
    }
    if (g.type==_VECT && g._VECTptr->size()==2){
      gen a=g._VECTptr->front(),b=g._VECTptr->back();
      if (a.type==_DOUBLE_ && b.type==_DOUBLE_){
	gbasis_reinject_ratio=a._DOUBLE_val<=0?0:a._DOUBLE_val;
	gbasis_reinject_speed_ratio=b._DOUBLE_val<=0?0:b._DOUBLE_val;
	return makevecteur(old,oldtime);
      }
    }
    if (g.type==_VECT && g._VECTptr->empty())
      return makevecteur(old,oldtime);
    return gensizeerr(contextptr);
  }
  static const char _gbasis_reinject_s []="gbasis_reinject";
  static define_unary_function_eval2 (__gbasis_reinject,&_gbasis_reinject,_gbasis_reinject_s,&printasDigits);
  define_unary_function_ptr5( at_gbasis_reinject ,alias_at_gbasis_reinject ,&__gbasis_reinject,0,true);

  gen _rur_certify(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG &&  g.subtype==-1) return  g;
    if (g.type==_INT_){
      if (g.val<0){
	rur_certify_maxthreads=-g.val;
	*logptr(contextptr) << "rur certification max number of threads " << rur_certify_maxthreads << "\n";
	return -g.val;
      }
      if (g.val==0) *logptr(contextptr) << "rur: no certification\n";
      if (g.val==1) *logptr(contextptr) << "rur: certify all equations\n";
      if (g.val>1) *logptr(contextptr) << "rur: certify equations of total degree <=" << g.val << "\n";
      return rur_do_certify=g.val-1;
    }
    if (g.type==_VECT || g._VECTptr->empty())
      return rur_do_certify+1;      
    return gensizeerr(contextptr);
  }
  static const char _rur_certify_s []="rur_certify";
  static define_unary_function_eval (__rur_certify,&_rur_certify,_rur_certify_s);
  define_unary_function_ptr5( at_rur_certify ,alias_at_rur_certify ,&__rur_certify,0,true);
  gen _rur_gbasis(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG &&  g.subtype==-1) return  g;
    if (g.type==_INT_){
      if (g.val==-1){ 
	rur_error_ifnot0dimensional=true;
	return string2gen("rur: if not 0 dimensional return error code",false);
      }
      if (g.val==-2){ 
	rur_error_ifnot0dimensional=false;
	return string2gen("rur: if not 0 dimensional, compute gbasis",false);
      }
      if (g.val<=0) *logptr(contextptr) << "rur: do not compute gbasis over Q\n";
      if (g.val==1) *logptr(contextptr) << "rur: compute gbasis over Q\n";
      if (g.val>1) *logptr(contextptr) << "rur: compute gbasis over Q if total nmumber of monomials is <=" << g.val << "\n";
      return rur_do_gbasis=g.val-1;
    }
    if (g.type==_VECT || g._VECTptr->empty())
      return rur_do_gbasis+1;      
    return gensizeerr(contextptr);
  }
  static const char _rur_gbasis_s []="rur_gbasis";
  static define_unary_function_eval (__rur_gbasis,&_rur_gbasis,_rur_gbasis_s);
  define_unary_function_ptr5( at_rur_gbasis ,alias_at_rur_gbasis ,&__rur_gbasis,0,true);

  // return true if v contains coeffs or matrix, false otherwise
  static bool read_gbargs(vecteur & v,int start,int s,gen & order,bool & with_cocoa,bool & with_f5,int & modular,gbasis_param_t & gbasis_param){
    bool ret=false;
    for (int i=start;i<s;++i){
      if (v[i]==at_gbasis)
        gbasis_param.gbasis=true;
      if (v[i]==at_eliminate)
	gbasis_param.eliminate_flag=true;
      if (v[i].is_symb_of_sommet(at_equal)){
        gen a=v[i]._SYMBptr->feuille[0];
        gen b=v[i]._SYMBptr->feuille[1];
        if (a==at_select)
          gbasis_param.buchberger_select_strategy=b.val;
      }
      if (v[i]==at_coeffs){
        gbasis_param.interred=false;
        gbasis_param.rawcoeffs=true;
      }
      if (v[i]==at_coeffs || v[i]==at_matrix)
	ret=true;
      if (v[i]==at_irem || v[i]==at_chinrem){
	modular=1;
	with_f5=false;
	with_cocoa=false;
      }
      if (is_equal(v[i])){
	gen & tmp=v[i]._SYMBptr->feuille;
	if (v[1].type==_VECT && tmp.type==_VECT && tmp._VECTptr->size()==2){
          gen tmp0=tmp._VECTptr->front(),tmp1=tmp._VECTptr->back();
          if (tmp0==_RUR_REVLEX){
            tmp1=derive(tmp1,v[1],context0);
            if (tmp1.type==_VECT){
              vector<int> sep;
              vecteur & d=*tmp1._VECTptr;
              for (int i=0;i<d.size();++i){
                if (d[i].type==_INT_)
                  sep.push_back(d[i].val);
                else
                  break;
              }
              if (sep.size()==d.size()){
                gbasis_param.initsep=sep;
                order=_RUR_REVLEX;
              }
            }
          }
        }
	if (tmp.type==_VECT && v[0].type==_VECT && tmp._VECTptr->front()==at_gbasis_reinject){
	  if (tmp._VECTptr->back().type==_VECT){
	    gbasis_param.reinject_begin=v[0]._VECTptr->size();
	    v[0]=mergevecteur(*v[0]._VECTptr,*tmp._VECTptr->back()._VECTptr);
	    gbasis_param.reinject_end=v[0]._VECTptr->size();
	  }
	  if (tmp._VECTptr->back().type==_INT_)
	    gbasis_param.reinject_for_calc=(gbasis_param.reinject_begin>=0?gbasis_param.reinject_begin:v[0]._VECTptr->size())+tmp._VECTptr->back().val;
	  continue;
	}
	if (tmp.type==_VECT && (tmp._VECTptr->front()==at_irem || tmp._VECTptr->front()==at_chinrem) && tmp._VECTptr->back().type==_INT_){
	  modular=tmp._VECTptr->back().val;
	}
	if (tmp.type==_VECT && tmp._VECTptr->front()==at_eliminate && tmp._VECTptr->back().type==_INT_){
	  gbasis_param.eliminate_flag=tmp._VECTptr->back().val!=0;
	}
	if (tmp.type==_VECT && tmp._VECTptr->front().type==_INT_ && tmp._VECTptr->back().type==_INT_){
	  switch (tmp._VECTptr->front().val){
	  case _WITH_COCOA:
	    with_cocoa=tmp._VECTptr->back().val!=0;
	    modular=!with_cocoa;
	    break;
	  case _WITH_F5: case _MODULAR_CHECK:
	    with_f5=tmp._VECTptr->back().val!=0;
	    break;
	  }
	}
      }
      if (v[i].type==_INT_ && v[i].subtype==_INT_GROEBNER){
	switch (v[i].val){
	case _WITH_COCOA:
	  with_cocoa=true;
	  break;
	case _WITH_F5: case _MODULAR_CHECK:
	  with_f5=true;
	  break;
	default:
	  order=v[i].val;
	}
      }
    }
#ifndef HAVE_LIBCOCOA
    with_cocoa=false;
#endif
    return ret;
  }


  void change_monomial_order(polynome & p,const gen & order){
    switch (order.val){
    case _PLEX_ORDER: 
      p.is_strictly_greater=i_lex_is_strictly_greater;
#ifdef CPP11
      p.m_is_strictly_greater=m_lex_is_strictly_greater<gen>;
#else
      p.m_is_strictly_greater=std::ptr_fun(m_lex_is_strictly_greater<gen>);
#endif
      break;
    case _REVLEX_ORDER: 
      p.is_strictly_greater=i_total_revlex_is_strictly_greater;
#ifdef CPP11
      p.m_is_strictly_greater=m_total_revlex_is_strictly_greater<gen>;
#else
      p.m_is_strictly_greater=std::ptr_fun(m_total_revlex_is_strictly_greater<gen>);
#endif
      break;
    case _TDEG_ORDER:
      p.is_strictly_greater=i_total_lex_is_strictly_greater;
#ifdef CPP11
      p.m_is_strictly_greater=m_total_lex_is_strictly_greater<gen>;
#else
      p.m_is_strictly_greater=std::ptr_fun(m_total_lex_is_strictly_greater<gen>);
#endif
      break;
    case _3VAR_ORDER:
      p.is_strictly_greater=i_3var_is_strictly_greater;
#ifdef CPP11
      p.m_is_strictly_greater=m_3var_is_strictly_greater<gen>;
#else
      p.m_is_strictly_greater=std::ptr_fun(m_3var_is_strictly_greater<gen>);
#endif
      break;      
    case _7VAR_ORDER:
      p.is_strictly_greater=i_7var_is_strictly_greater;
#ifdef CPP11
      p.m_is_strictly_greater=m_7var_is_strictly_greater<gen>;
#else
      p.m_is_strictly_greater=std::ptr_fun(m_7var_is_strictly_greater<gen>);
#endif
      break;      
    case _11VAR_ORDER:
      p.is_strictly_greater=i_11var_is_strictly_greater;
#ifdef CPP11
      p.m_is_strictly_greater=m_11var_is_strictly_greater<gen>;
#else
      p.m_is_strictly_greater=std::ptr_fun(m_11var_is_strictly_greater<gen>);
#endif
      break;      
    case _16VAR_ORDER:
      p.is_strictly_greater=i_16var_is_strictly_greater;
#ifdef CPP11
      p.m_is_strictly_greater=m_16var_is_strictly_greater<gen>;
#else
      p.m_is_strictly_greater=std::ptr_fun(m_16var_is_strictly_greater<gen>);
#endif
      break;      
    case _32VAR_ORDER:
      p.is_strictly_greater=i_32var_is_strictly_greater;
#ifdef CPP11
      p.m_is_strictly_greater=m_32var_is_strictly_greater<gen>;
#else
      p.m_is_strictly_greater=std::ptr_fun(m_32var_is_strictly_greater<gen>);
#endif
      break;      
    case _64VAR_ORDER:
      p.is_strictly_greater=i_64var_is_strictly_greater;
#ifdef CPP11
      p.m_is_strictly_greater=m_64var_is_strictly_greater<gen>;
#else
      p.m_is_strictly_greater=std::ptr_fun(m_64var_is_strictly_greater<gen>);
#endif
      break;      
    }
    p.tsort();
  }

  static void change_monomial_order(vectpoly & eqp,const gen & order){
    // change polynomial order
    if (order.type==_INT_ && order.val){
      vectpoly::iterator it=eqp.begin(),itend=eqp.end();
      for (;it!=itend;++it){
	change_monomial_order(*it,order);
      }
    }
  }

  // l=variables, l0=parameters, add 0 to l
  // returns first fake variable, i.e. value of l with a 0 (or l.size())
  int revlex_parametrize(vecteur & l,const vecteur &l0,int & order){
    bool rur=order<0;
    int res=int(l.size());
#if GROEBNER_VARS==15
    // split variables and parameters for revlex
    if (!l.empty() && l!=l0 && l0.size()<=64 && (order==_REVLEX_ORDER || order==_RUR_REVLEX)){
      if (l.size()>11 || (l0.size()+3-l.size()%4)>14){
	if (l.size()<=11){
#ifdef GIAC_CHARDEGTYPE
	  while (l.size()<16) l.push_back(0);
	  order=_16VAR_ORDER; // improve: could be less
#else
	  while (l.size()<12) l.push_back(0);
	  order=_11VAR_ORDER; // improve: could be less
#endif
	}
	else {
	  int j=nextpow2(int(l.size()));
	  if (j==16) order=_16VAR_ORDER;
	  if (j==32) order=_32VAR_ORDER;
	  if (j==64) order=_64VAR_ORDER;
	  for (;int(l.size())<j;)
	    l.push_back(0);
	}
      }
      else { // l.size()<=11 and l0.size() small enough
	// add fake variables
	if (l.size()/4==0)
	  order=_3VAR_ORDER;
	if (l.size()/4==1)
	  order=_7VAR_ORDER;
	if (l.size()/4==2)
	  order=_11VAR_ORDER;
	for (int j=l.size()%4;j<3;j++){
	  l.push_back(0);
	}
      }
      if (rur)
	order=-order;
    }
#endif
    return res;
  }

  vecteur gbasis_vars(const vecteur & args,GIAC_CONTEXT){
    vecteur l(lvar(args));
    vector<double> pideg(l.size(),1.0);
    for (int i=0;i<args.size();++i){
      gen g=args[i];
      g=_degree(makesequence(g,l),contextptr);
      if (g.type!=_VECT || g._VECTptr->size()!=l.size())
        return l;
      vecteur & v= *g._VECTptr;
      for (int j=0;j<v.size();++j)
        pideg[j] *= (v[j].val+1);
    }
    vector< pair<double,int> > w(l.size());
    for (int i=0;i<l.size();++i)
      w[i]=pair<double,int>(pideg[i],i);
    sort(w.begin(),w.end());
    vecteur L(l);
    for (int i=0;i<l.size();++i)
      L[i]=l[w[i].second];
    return L;
  }

  gen _gbasis_vars(const gen & args,GIAC_CONTEXT){
    if (args.type!=_VECT)
      return vecteur(0);
    return gbasis_vars(*args._VECTptr,contextptr);
  }
  static const char _gbasis_vars_s []="gbasis_vars";
  static define_unary_function_eval (__gbasis_vars,&_gbasis_vars,_gbasis_vars_s);
  define_unary_function_ptr5( at_gbasis_vars ,alias_at_gbasis_vars,&__gbasis_vars,0,true);

  gen _syzygy(const gen & args,GIAC_CONTEXT){
    if (args.type!=_VECT)
      return vecteur(0);
    // compute a gbasis to reduce syzygyies of [f1,..,fk] 
    // compute gbasis for all pairs 1<=i<j<=k [0...0,-fj,0..0,fi,0..0] 
    vecteur & v = *args._VECTptr;
    int d=v.size();
    vecteur L; L.reserve(d*(d-1)/2); vecteur D(d);
    for (int i=0;i<d;++i){
      for (int j=i+1;j<d;++j){
        D.clear(); D.resize(d);
        D[i]=-v[j]; D[j]=v[i];
        L.push_back(D);
      }
    }
    gen G=_gbasis(L,contextptr);
    vecteur V=gen2vecteur(eval(gen("lastv",contextptr),1,contextptr));
    if (V.size()<d)
      return makesequence(G,V,string2gen("syzygy internal bug",false));
    vecteur A=vecteur(V.begin(),V.begin()+d);
    vecteur X=vecteur(V.begin()+d,V.end());
    return makesequence(G,V,A,X);
  }
  static const char _syzygy_s []="syzygy";
  static define_unary_function_eval (__syzygy,&_syzygy,_syzygy_s);
  define_unary_function_ptr5( at_syzygy ,alias_at_syzygy,&__syzygy,0,true);

  // gbasis([Pi],[vars]) -> [Pi']
  gen _gbasis(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    vecteur v;
    if (args.type==_VECT && args.subtype==_SEQ__VECT){
      v=*args._VECTptr;
      if (v.size()>=2 && v[1].type!=_VECT)
        v=vecteur(1,change_subtype(args,0));
    }
    else
      v=vecteur(1,args);
    int s=int(v.size());
    if (s==0)
      return args;
    if (v[0].type!=_VECT)
      return gensizeerr(contextptr);
    if (s<2 || v[1].type!=_VECT){
      v.insert(v.begin()+1,gbasis_vars(*v[0]._VECTptr,contextptr));
      sto(v[1],gen("lastv",contextptr),contextptr); // save variables 
      *logptr(contextptr) << "Variables saved in lastv " << v[1] << "\n";
      s++;
    }
    if (debug_infolevel)
      CERR << CLOCK()*1e-6 << " gbasis begin :" << memory_usage()*1e-6 << '\n';
    v[0]=remove_equal(v[0]);
    gen order=_REVLEX_ORDER; // 0 assumes plex and 0-dimension ideal so that FGLM applies
    // v[2] will serve for ordering
    bool with_f5=false,with_cocoa=false;
    gbasis_param_t gbasis_param={false,-1,-1,-1,true,false,false,-1,vector<int>(0)};
    int modular=1;
    vector<vectpoly> gbasiscoeff; vector<vectpoly> * coeffsptr=0;
    if (read_gbargs(v,2,s,order,with_cocoa,with_f5,modular,gbasis_param))
      coeffsptr=&gbasiscoeff;
    vecteur l1=*v[1]._VECTptr;
    vecteur l0;
    if (s>2 && v[2].type==_VECT)
      lidnt(v[2],l0,true); // ordering for remaining variables
    bool rurparam=false;
    for (int i=0;i<s;++i){
      if (v[i]==_RUR_REVLEX || (is_equal(v[i])&&v[i]._SYMBptr->feuille[0]==_RUR_REVLEX)){
        rurparam=true;
	lidnt(v[1],l0,true); // insure all variables are here for rur
      }
    }
    lidnt(v[0],l0,true);
    rurparam = rurparam && l0.size()>l1.size();
    if (rurparam){
      // parametric rur, interpolate
      vecteur lparam(l1);
      lidnt(l0,lparam,true);
      lparam=vecteur(lparam.begin()+l1.size(),lparam.end());
      int nparam=lparam.size();
      vecteur vargs=makevecteur(v[0],v[1],0);
      if (0 &&
          nparam==1){
        *logptr(contextptr) << "Warning, parametric rur: experimental code\n";
        gen cursep=0;
        for (int iparam=0;iparam<nparam;++iparam){
          vecteur Lx,Ly,Ldiff,Llast;
          int pmindeg=0,interpcurdeg=0;
          for (int j=0;;++j){
            if (interpcurdeg==RUR_PARAM_MAX_DEG)
              return gensizeerr("Parametric rur interpolation degree too large");
            gen x=j;
            if (cursep==0)
              vargs[2]=change_subtype(_RUR_REVLEX,_INT_GROEBNER);
            else
              vargs[2]=symb_equal(_RUR_REVLEX,cursep);
            gen curarg=subst(vargs,lparam[iparam],j,false,contextptr);
            gen y=_gbasis(change_subtype(curarg,_SEQ__VECT),contextptr);
            if (y.type!=_VECT || y._VECTptr->size()<3 ){
              cursep=0;
              continue;
            }
            if (y._VECTptr->front()!=_RUR_REVLEX){
              cursep=0;
              continue;
            }
            cursep=y[1];
            // check pmin degree and compare to previous degree
            gen pmin=y[2],var=lidnt(pmin)[0];
            int deg=_degree(makesequence(pmin,var),contextptr).val;
            if (deg<pmindeg)
              continue;
            if (deg>pmindeg){
              Lx.clear(); Ly.clear(); Ldiff.clear(); Llast.clear();
              interpcurdeg=0; pmindeg=deg;
            }
            interpcurdeg++;
            Lx.push_back(x); Ly.push_back(y);
            // update divided differences
            gen cur=y;
            vecteur newline; newline.reserve(Llast.size()+1);
            newline.push_back(y);
            for (int k=0;k<Llast.size();++k){
              cur=ratnormal((cur-Llast[k])/(Lx[k+1]-Lx[0]),contextptr); 
              newline.push_back(cur);
            }
            Ldiff.push_back(cur);
            Llast.swap(newline);
            if (is_zero(cur)){
              // build G from Ldiff and Lx, and variable lparam[iparam]
              int s=Ldiff.size()-1;
              gen x=lparam[iparam],G=Ldiff[s-1];
              for (int i=s-2;i>=0;--i){
                G=ratnormal((x-Lx[i])*G+Ldiff[i],contextptr);
              }
              // FIXME check G
              return G;
            }
          }
        }
      }
    }
    // remove variables not in args0
    vecteur l;
    for (unsigned i=0;i<l1.size();++i){
      if (equalposcomp(l0,l1[i]))
	l.push_back(l1[i]);
    }
    l0=lidnt_with_at(makevecteur(l,l0)); // this sorts l0 with l variables first
    int faken=revlex_parametrize(l,l0,order.val),lsize=int(l.size());
    gen v0=v[0];
    if (ckmatrix(v0)){
      // gbasis for a module
      // "naive" computation, add dim variables s1,..,sd
      // then every element [p1,...,pk] is mapped to p1*s1+...+pk*sk
      // then compute gbasis(mapped elements union all pairs si*sj)
      vecteur w=*v0._VECTptr,S;
      gen w0=w[0]; int dim=w0._VECTptr->size();
      // find unused char root variable in A..Z
      short tab[256]={0};
      char rootvar=0;
      for (int i=0;i<l.size();++i){
        tab[l[i].print(contextptr)[0]]++;
      }
      for (int i=65;i<65+26;++i){
        if (tab[i]==0){
          rootvar=i;
          break;
        }
        if (tab[i+32]==0){
          rootvar=i+32;
          break;
        }
      }
      if (rootvar==0){
        return gensizeerr("Please rename some variables");
      }
      // generate additional variables
      for (int i=0;i<dim;++i){
        gen si(rootvar+print_INT_(i),contextptr);
        S.push_back(si);
      }
      l=mergevecteur(S,l);
      sto(l,gen("lastv",contextptr),contextptr); // save variables       
      for (int i=0;i<v0._VECTptr->size();++i){
        w[i]=dotvecteur(w[i],S,contextptr);
      }
      for (int i=0;i<dim;++i){
        for (int j=i;j<dim;++j){
          w.push_back(S[i]*S[j]);
        }
      }
      v0=w;
    }
    l=vecteur(1,l);
    if (s>2 && v[2].type==_VECT)
      alg_lvar(v[2],l); // ordering for remaining variables
    alg_lvar(v0,l);
    
    // if (l.front()._VECTptr->size()==15 && order.val==11) l.front()._VECTptr->insert(l.front()._VECTptr->begin()+11,0);
    // convert eq to polynomial
    if (debug_infolevel)
      CERR << CLOCK()*1e-6 << " memory before convert :" << memory_usage()*1e-6 << "M\n";
    vectpoly eqp;
    {
      // all negative integers will be duplicated in e2r, adding about 50% mem
      gen eqtmp=e2r(v0,l,contextptr);
      const vecteur & eq_in=*eqtmp._VECTptr;
      if (debug_infolevel)
	CERR << CLOCK()*1e-6 << " after convert :" << memory_usage()*1e-6 << "M\n";
      if (!vecteur2vector_polynome(eq_in,l,eqp))
	return vecteur(1,plus_one);
    }
    if (eqp.empty()) return vecteur(0);
    // add fake polynomials for fake variables added by revlex_parametrize
    int dim=eqp.front().dim;
#if 1
    for (;faken<lsize;++faken){
      polynome fakep(dim,vector< monomial<gen> >(1,monomial<gen>(1,faken+1,dim)));
      eqp.push_back(fakep);
    }
#endif
    gen coeff;
    environment env ;
    if (modular){
      env.modulo=0;
      with_cocoa=false;
    }
    env.moduloon = false;    
    for (unsigned i=0;i<eqp.size();++i){
      int ct=coefftype(eqp[i],coeff);
      if (ct==_EXT)
        return gensizeerr(gettext("Non polynomial system"));
      if (ct==_MOD){
	with_cocoa = false;
	env.moduloon = true;
	env.modulo = *(coeff._MODptr+1);
	env.pn=env.modulo;
	vectpoly::iterator it=eqp.begin(),itend=eqp.end();
	for (;it!=itend;++it)
	  *it=unmodularize(*it);
	break;
      }
    }
    if (!with_cocoa)
      change_monomial_order(eqp,abs(order,contextptr));
    int rur=0;
    vectpoly eqpr(gbasis(eqp,order,with_cocoa,with_cocoa?with_f5:modular,&env,rur,contextptr,gbasis_param,coeffsptr));
    if (debug_infolevel)
      CERR << CLOCK()*1e-6 << " gbasis symbolic conversion begin\n";
    vecteur res,coeffs;
    vectpoly::const_iterator it=eqpr.begin(),itend=eqpr.end();
    res.reserve(itend-it);
    for (int i=0;it!=itend;++i,++it){
      gen tmp=r2e(*it,l,contextptr);
      if (is_zero(tmp) && !is_zero(*it))
	continue;
      res.push_back(tmp);
      if (coeffsptr && coeffsptr->size()>i){
        vecteur ligne;
        const vectpoly & cur=(*coeffsptr)[i];
        for (unsigned j=0;j<cur.size();++j)
          ligne.push_back(r2e(cur[j],l,contextptr));
        coeffs.push_back(ligne);
      }
    }
    if (debug_infolevel)
      CERR << CLOCK()*1e-6 << " gbasis symbolic conversion end\n";
    if (order.val<0 && rur){
      // subst l[0] by another variable name to avoid confusion in res[2..]?
      if (res[0].type==_IDNT && l.front().type==_VECT && !l.front()._VECTptr->empty()){
	for (int i=1;i<=dim+2;++i)
          res[i]=subst(res[i],l.front()[0],res[0],false,contextptr);
      }
      res.insert(res.begin(),change_subtype(order,_INT_GROEBNER));
    }
    return coeffsptr?makevecteur(res,coeffs):res;
  }
  gen _gbasis_(const gen & args,GIAC_CONTEXT){
    gen res=_gbasis(args,contextptr);
    sto(res,gen("lastgb",contextptr),contextptr);
    return res;
  }
  static const char _gbasis_s []="gbasis";
  static define_unary_function_eval (__gbasis,&_gbasis_,_gbasis_s);
  define_unary_function_ptr5( at_gbasis ,alias_at_gbasis,&__gbasis,0,true);
  
  gen _gbasis_max_pairs(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG &&  g.subtype==-1) return  g;
    gen args(g);
    if (g.type==_DOUBLE_)
      args=int(g._DOUBLE_val);    
    if (args.type!=_INT_)
      return int(max_pairs_by_iteration);
    int old=max_pairs_by_iteration;
    max_pairs_by_iteration=args.val<256?(1<<30):args.val;
    return old;
  }
  static const char _gbasis_max_pairs_s []="gbasis_max_pairs";
  static define_unary_function_eval2 (__gbasis_max_pairs,&_gbasis_max_pairs,_gbasis_max_pairs_s,&printasDigits);
  define_unary_function_ptr5( at_gbasis_max_pairs ,alias_at_gbasis_max_pairs ,&__gbasis_max_pairs,0,true);

  gen _gbasis_simult_primes(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG &&  g.subtype==-1) return  g;
    gen args(g);
    int old=simult_primes;
    if (g.type==_VECT && g._VECTptr->size()==5 && is_integer_vecteur(*g._VECTptr)){
      const vecteur & v=*g._VECTptr;
      simult_primes=giacmax(1,v[0].val);
      simult_primes_seuil2=v[1].val;
      simult_primes2=giacmax(1,v[2].val);
      simult_primes_seuil3=v[3].val;
      simult_primes3=giacmax(1,v[4].val);
      *logptr(contextptr) << simult_primes << ", n>" << simult_primes_seuil2 << ":" << simult_primes2 << " ,n>" << simult_primes_seuil3 << ":" << simult_primes3 << '\n';
      return old;
    }
    if (g.type==_DOUBLE_)
      args=int(g._DOUBLE_val);    
    if (args.type!=_INT_)
      return int(simult_primes);
    simult_primes=args.val<1?1:args.val;
    return old;
  }
  static const char _gbasis_simult_primes_s []="gbasis_simult_primes";
  static define_unary_function_eval2 (__gbasis_simult_primes,&_gbasis_simult_primes,_gbasis_simult_primes_s,&printasDigits);
  define_unary_function_ptr5( at_gbasis_simult_primes ,alias_at_gbasis_simult_primes ,&__gbasis_simult_primes,0,true);

  static gen in_greduce(const gen & eq,const vecteur & l,const vectpoly & eqp,const gen & order,bool with_cocoa,GIAC_CONTEXT,vector<polynome> * quo=0){
    if (eq.type!=_POLY)
      return r2e(eq,l,contextptr);
    gen coeff;
    environment env ;
    if (coefftype(*eq._POLYptr,coeff)==_MOD){
      with_cocoa = false;
      env.moduloon = true;
      env.modulo = *(coeff._MODptr+1);
      env.pn=env.modulo;
    }
    else
      env.moduloon = false;
    polynome p(*eq._POLYptr);
    change_monomial_order(p,order);
    vectpoly rescocoa;
    if (!env.moduloon && with_cocoa && cocoa_greduce(vectpoly(1,p),eqp,order,rescocoa))
      return r2e(rescocoa.front(),l,contextptr);
    // FIXME: get constant term, subtract one to get the correct constant
    // gen C(p.constant_term());
    // eq=eq-C+plus_one;
    // p=*eq._POLYptr;
    // change_monomial_order(p,order);
    // polynome res(env.moduloon?reduce(p,eqp.begin(),eqp.end(),&env):reducegb(p,eqp.begin(),eqp.end(),&env));
    gen C1;
    if (debug_infolevel>1)
      COUT << CLOCK() << "begin reduce poly #monomials " << p.coord.size() << '\n';
    reduce(p,&eqp.front(),&eqp.front()+eqp.size(),p,C1,&env,quo);
    if (debug_infolevel>1)
      COUT << CLOCK() << "end reduce poly #monomials " << p.coord.size() << '\n';
    // gen C1(res.constant_term());
    if (env.moduloon){
      p=invmod(C1,env.modulo)*p;
      modularize(p,env.modulo);
    }
    else
      p=p/C1;
    return r2e(p,l,contextptr);
  }

  static gen in_ideal(const vectpoly & r,const vecteur &l,const vectpoly & v,const gen & order,bool with_cocoa,bool with_f5,environment * env){
#ifndef NO_STDEXCEPT
    try {
      if (with_cocoa){
	return cocoa_in_ideal(r,v,order);
      }
    } catch (...){
     return -1;
    }
#endif
    vecteur res;
    for (int i=0;i<int(r.size());++i){
      res.push_back(is_zero(in_greduce(r[i],l,v,order,with_cocoa,context0)));
    }
    return res;
  }

  static gen greduce(const gen & g,const vecteur & l,const vectpoly & eqp,const gen & order,bool with_cocoa,GIAC_CONTEXT,vecteur * quo=0){
    gen eq(e2r(g,l,contextptr));
    vector<polynome> q;
    if (eq.type==_FRAC){
      gen den=in_greduce(eq._FRACptr->den,l,eqp,order,with_cocoa,contextptr,0);
      gen res=in_greduce(eq._FRACptr->num,l,eqp,order,with_cocoa,contextptr,quo?&q:0)/den;
      if (quo){
	for (int i=0;i<int(quo->size());++i){
	  (*quo)[i]=r2e(q[i],l,contextptr)/den;
	}
      }
      reverse(quo->begin(),quo->end()); // the gbasis was reversed
      return res;
    }
    gen res=in_greduce(eq,l,eqp,order,with_cocoa,contextptr,quo?&q:0);
    if (quo){
      quo->clear();
      quo->resize(q.size());
      for (int i=0;i<int(quo->size());++i){
	(*quo)[i]=r2e(q[i],l,contextptr);
      }
      reverse(quo->begin(),quo->end()); // the gbasis was reversed
    }
    return res;
  }

  // greduce(P,[gbasis],[vars])
  gen _greduce(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT)
      return symbolic(at_gbasis,args);
    vecteur v = *args._VECTptr;
    int s=int(v.size());
    vecteur quo;
    vecteur * quoptr=0;
    if (s && (v.back()==at_quo || v.back()==at_quorem)){
      quoptr=&quo;
      v.pop_back();
      --s;
    }
    if (s<2)
      return gentoofewargs("greduce");
    if (s<3)
      v.push_back(lidnt_with_at(v[1]));
    if (v[1].type!=_VECT) 
      v[1]=vecteur(1,v[1]);
    v[1]=remove_equal(v[1]);
    if (v[2].type!=_VECT)
      return gensizeerr(contextptr);
    // v[3] will serve for ordering
    gen order=_REVLEX_ORDER;// _PLEX_ORDER; // FIXME for parameters!
    bool with_f5=false,with_cocoa=false;
    gbasis_param_t gbasis_param={false,-1,-1,-1,true,false,false,-1,vector<int>(0)};
    int modular=1;
    read_gbargs(v,3,s,order,with_cocoa,with_f5,modular,gbasis_param);
    vecteur l1=gen2vecteur(v[2]),l0=lidnt_with_at(makevecteur(v[0],v[1]));
    // remove variables not in args0
    vecteur l;
    for (unsigned i=0;i<l1.size();++i){
      if (equalposcomp(l0,l1[i]))
	l.push_back(l1[i]);
    }
    int faken=revlex_parametrize(l,l0,order.val),lsize=int(l.size());
    l=vecteur(1,l);
    if (s>3 && v[3].type==_VECT)
      alg_lvar(v[3],l); // ordering for remaining variables
    alg_lvar(makevecteur(v[0],v[1]),l);
    if (v[1].type==_VECT && v[1]._VECTptr->size()==1 && v[1]._VECTptr->front()==1){
      if (v[0].type==_VECT){
        vecteur & v0=*v[0]._VECTptr;
        vecteur res0(v0.size()),res1;
        if (quoptr){
          for (int i=0;i<v0.size();++i){
            res1.push_back(makevecteur(v0[i]));
          }
          return makevecteur(res0,res1);
        }
        return res0;
      }
      if (quoptr)
        return makevecteur(0,makevecteur(v[0]));
      return 0;
    }   
    vecteur eq_in(*e2r(v[1],l,contextptr)._VECTptr);
    vectpoly eqp;
    if (!vecteur2vector_polynome(eq_in,l,eqp))
      return gensizeerr("Bad second argument, expecting a Groebner basis");
    change_monomial_order(eqp,order);
    reverse(eqp.begin(),eqp.end());
#if !defined CAS38_DISABLED && !defined FXCG && !defined KHICAS && !defined SDL_KHICAS 
    vecteur red_in_(gen2vecteur(v[0])),deno(red_in_.size());
    for (int i=0;i<int(red_in_.size());++i){
      gen eq(e2r(red_in_[i],l,contextptr));
      if (eq.type!=_FRAC) 
	deno[i]=1;
      else {
	deno[i]=eq._FRACptr->den;
	eq=eq._FRACptr->num;
      }
      red_in_[i]=eq;
    }
    vectpoly red_in,red_out;
    if (!vecteur2vector_polynome(red_in_,l,red_in))
      return gensizeerr("Bad first argument, expecting polynomial or list of polynomials");
    change_monomial_order(red_in,order);
    order_t order_={static_cast<short>(order.val),0};
    environment env;
    env.moduloon=false;
    if (!quoptr && greduce8(red_in,eqp,order_,red_out,&env,contextptr)){
      vecteur red_out_;
      for (int i=0;i<int(red_out.size());++i)
	red_out_.push_back(r2e(red_out[i],l,contextptr)/r2e(deno[i],l,contextptr));
      if (v[0].type==_VECT || red_out_.size()!=1)
	return red_out_;
      return red_out_.front();
    }
#endif
    if (v[0].type==_VECT){
      vecteur res(v[0]._VECTptr->size());
      if (debug_infolevel>1)
	COUT << CLOCK() << " begin reduce vector size " << res.size() << '\n';
      for (unsigned i=0;i<v[0]._VECTptr->size();++i){
	res[i]=greduce((*v[0]._VECTptr)[i],l,eqp,order,with_cocoa,contextptr,quoptr);
	if (quoptr)
	  res[i]=makevecteur(res[i],*quoptr);
      }
      if (debug_infolevel>1)
	COUT << CLOCK() << " end reduce vector size " << res.size() << '\n';
      return res;
    }
    gen res=greduce(v[0],l,eqp,order,with_cocoa,contextptr,quoptr);
    if (quoptr) 
      return makevecteur(res,*quoptr);
    return res;
  }

  static const char _greduce_s []="greduce";
  static define_unary_function_eval (__greduce,&_greduce,_greduce_s);
  define_unary_function_ptr5( at_greduce ,alias_at_greduce,&__greduce,0,true);

  // eliminate/algsubs (very first version adapted from Reinhard Oldenburg user code)
  // eliminate(eqs,vars)
  gen _eliminate(const gen & args,GIAC_CONTEXT){
    if (args.type!=_VECT || args._VECTptr->size()<2)
      return gensizeerr(contextptr);
    int returngb=0;
    if (args._VECTptr->back()==at_gbasis)
      returngb=1;
    if (args._VECTptr->back()==at_lcoeff)
      returngb=2;
    if (args._VECTptr->back()==at_resultant)
      returngb=3;
    bool with_f5=false,with_cocoa=false; int modular=1; gen o;
    gbasis_param_t gbasis_param={epsilon(contextptr)!=0,-1,-1,-1,true,false,false,-1,vector<int>(0)};
    read_gbargs(*args._VECTptr,2,int(args._VECTptr->size()),o,with_cocoa,with_f5,modular,gbasis_param);
    vecteur eqs=gen2vecteur(remove_equal(args._VECTptr->front()));
    vecteur elim=gen2vecteur((*args._VECTptr)[1]);
    vecteur eqsidnt=lidnt(eqs);
    for (int i=0;i<elim.size();++i){
      if (!equalposcomp(eqsidnt,elim[i])){
	elim.erase(elim.begin()+i);
	--i;
      }
    }
    if (elim.empty()){
      for (int i=0;i<eqs.size();++i){
	if (eqs[i].type<_POLY && !is_zero(eqs[i]))
	  eqs[i]=1;
      }
      comprim(eqs);
      if (eqs.size()==1 && is_zero(eqs[0]))
	eqs.clear();
      return eqs;
    }
    vecteur l(elim);
    if (args._VECTptr->size()>2 && (*args._VECTptr)[2].type==_VECT)
      lvar((*args._VECTptr)[2],l);
    lvar(eqs,l); // add other vars after vars to eliminate
    vecteur remainvars(l.begin()+elim.size(),l.end());
    if (!returngb && eqs.size()<=l.size()+3){
      // eliminate variables with linear dependency 
      // (in order to lower the number of vars, since <= 11 vars is handled faster)
      for (unsigned i=0;i<eqs.size();++i){
	for (unsigned j=0;j<elim.size();++j){
	  gen a,b;
	  if (is_linear_wrt(eqs[i],elim[j],a,b,contextptr) && !is_zero(simplify(a,contextptr),contextptr) && 
	      is_zero(derive(a,l,contextptr),contextptr)
	      ){
	    // Warning: a is not identically 0 but may vanish for some values of elim...
	    // eqs[i]=a*elim[j]+b
	    // replace elim[j] by -b/a
	    gen elimj=-b/a;
	    eqs.erase(eqs.begin()+i);
	    for (unsigned k=0;k<eqs.size();++k){
	      eqs[k]=_numer(subst(eqs[k],elim[j],elimj,false,contextptr),contextptr);
	      if (is_zero(eqs[k])){
		eqs.erase(eqs.begin()+k);
		--k;
	      }
	    }
	    elim.erase(elim.begin()+j);
	    gen res=_eliminate(makesequence(eqs,elim,symb_equal(at_irem,modular),symb_equal(at_eliminate,gbasis_param.eliminate_flag)),contextptr);
	    // additional check for 
	    // eliminate([v49+-1*v49*v48+-1*v47+v50*v47,-1*v49+-1*v49*v48+v47+v50*v47,v59+-1*v55,v60+-1*v56,2*v63+-1*v59,-1+v50,v48,-1+2*v64+-1*v60,-4*v56+v55^2,-1*v47,-1*v49],revlist([v47,v48,v49,v50,v55,v56,v59,v60]));
	    // eliminate([-2+2*v7+-1*v5,-4+2*v8+-1*v6,4*v6+-1*v6^2+2*v5+-1*v5^2,-4*v6+v6^2+-2*v5+v5^2],revlist([v5,v6]));
	    if (elim.empty())
	      res= _eliminate(makesequence(res,gen2vecteur((*args._VECTptr)[1])),contextptr);
	    return res;
	  }
	}
      }
    }
    if (elim.size()==1 && returngb!=1 && eqs.size()>1) {
      *logptr(contextptr) << "1 variable to eliminate, using resultant. Run with last optional parameter gbasis if you want to force gbasis\n";
      returngb=3; // for example for eliminate([((-((t)^(2)))*((((t)^(2))+(1))^(3)))+((((((t)^(4))+((x)*((((t)^(2))+(1))^(2))))+((6)*((t)^(2))))-(3))^(2)), (-((((t)^(2))+(1))^(3)))+((((-(8))*((t)^(3)))+((y)*((((t)^(2))+(1))^(2))))^(2))],[t])
    }
    vecteur linelim;
#ifdef GIAC_GBASISLEX
    if (returngb==3 && eqs.size()<=l.size()+3){
      // eliminate variables with linear dependency 
      // (in order to lower the number of vars, since <= 11 vars is handled faster)
      // not faster
      // Perhaps better: find revlex gbasis and do something similar to FGLM
      for (unsigned i=0;i<eqs.size();++i){
	for (unsigned j=0;j<elim.size();++j){
	  gen a,b;
	  if (!equalposcomp(linelim,elim[j]) && is_linear_wrt(eqs[i],elim[j],a,b,contextptr) && !is_zero(simplify(a,contextptr),contextptr) 
	      && is_zero(derive(a,remainvars,contextptr),contextptr)
	      ){
	    linelim.push_back(elim[j]);
	  }
	}
      }
    }
#endif
    // put linear dependent variables first
    int lexvars=int(linelim.size());
    lvar(elim,linelim);
    elim=linelim;
    int es=int(elim.size()),rs=int(l.size()-elim.size()),neq=int(eqs.size());
#if 1
    // check if we should eliminate linear dependency with resultant
    // to fit inside 3/11 or 7/7 or 11/3
    if (returngb==3 && neq>=2 && neq<=l.size()+3){
      bool ok=es>=1;
      if (ok){
	*logptr(contextptr) << "Eliminating with resultant. Original equations may reduce further."<<'\n';
	vector<int> vtdeg;
	// Choose lowest degree pivot 
	int curdeg=_total_degree(makesequence(eqs.front(),l),contextptr).val;
	vtdeg.push_back(curdeg);
	vector<int> pos(1,0);
	for (int i=1;i<neq;++i){
	  int tdeg=_total_degree(makesequence(eqs[i],l),contextptr).val;
	  vtdeg.push_back(tdeg);
	  if (tdeg>curdeg)
	    continue;
	  if (tdeg<curdeg){
	    curdeg=tdeg;
	    pos=vector<int>(1,i);
	    continue;
	  }
	  pos.push_back(i);
	}
	// Choose lowest degree variable in pos
	curdeg=RAND_MAX; 
	vector<int> poselim; 
	for (int i=0;i<int(pos.size());++i){
	  gen eq=eqs[pos[i]];
	  gen eqdeg=_degree(makesequence(eq,elim),contextptr);
	  if (eqdeg.type==_VECT){
	    const vecteur & v =*eqdeg._VECTptr;
	    for (int j=0;j<int(v.size());++j){
	      if (v[j].type!=_INT_ || v[j].val==0 || v[j].val>curdeg)
		continue;
	      if (v[j].val<curdeg){
		curdeg=v[j].val;
		poselim=vector<int>(1,j);
	      }
	      if (!equalposcomp(poselim,j))
		poselim.push_back(j);
	    }
	  }
	}
	if (poselim.empty()) // do not call resultant, var is not there
	  returngb=0;
	else if (1 || curdeg==1
		 ){
	  // Choose lowest number of dependent equations in poselim
	  gen besteq(0),bestvar(0); int bestpos=-1,n0deps=-1;
	  for (int i=0;i<int(poselim.size());++i){
	    gen curvar=elim[poselim[i]];
	    gen curdiff=derive(eqs,curvar,contextptr);
	    gen cur0deps=_count_eq(makesequence(0,curdiff),contextptr);
	    if (cur0deps.type==_INT_ && cur0deps.val>n0deps && curdiff.type==_VECT){
	      n0deps=cur0deps.val;
	      bestvar=curvar;
	      // find smallest total degree equation depending on bestvar
	      bestpos=-1; besteq=0;
	      int besttdeg=RAND_MAX;
	      for (int j=0;j<int(curdiff._VECTptr->size());++j){
		if (is_zero((*curdiff._VECTptr)[j],contextptr)) continue;
		if (vtdeg[j]<besttdeg){
		  besttdeg=vtdeg[j];
		  besteq=eqs[j];
		  bestpos=j;
		}
	      }
	    } 
	  }
	  // make resultant of all equations except posi with cureq, curvar
	  gen G=_gcd(eqs,contextptr);
	  // if G depends on bestvar, then we can not deduce anything
	  if (!is_zero(derive(G,bestvar,contextptr))){
	    *logptr(contextptr) << gettext("No relation found, if following equation holds 0=") << G << '\n';
	    return vecteur(0);
	  }
	  vecteur neweq;
	  for (int i=0;i<neq;++i){
	    if (i==bestpos) continue;
	    gen a=_simp2(makesequence(eqs[i],besteq),contextptr);
	    if (a.type!=_VECT || a._VECTptr->size()!=2)
	      return gensizeerr(contextptr);
	    gen G=_gcd(makesequence(eqs[i],besteq),contextptr);
	    gen r=_resultant(makesequence(a._VECTptr->front(),a._VECTptr->back(),bestvar),contextptr)*G;
	    vecteur vr(lidnt(r));
	    if (!vr.empty()){
	      gen dr=_diff(makesequence(r,vr[0]),contextptr);
	      dr=_gcd(makesequence(r,dr),contextptr);
	      r=_quo(makesequence(r,dr,vr[0]),contextptr);
	    }
	    vecteur rv=lvar(r);
	    r=_primpart(makesequence(r,rv),contextptr);
	    neweq.push_back(r);
	  }
	  vecteur newelim;
	  if (contains(lidnt(neweq),bestvar))
	    newelim.push_back(bestvar);
	  for (int i=0;i<int(elim.size());++i){
	    if (elim[i]!=bestvar)
	      newelim.push_back(elim[i]);
	  }
	  // recursive call
	  if (newelim.empty())
	    return neweq;
	  gen res=_eliminate(makesequence(neweq,newelim,at_resultant),contextptr);
	  return res;
	}
      }
    }
#endif
    vecteur gb,res;
    int order=_PLEX_ORDER;
#if GROEBNER_VARS==15
    if (es<=64){
      unsigned i=0;
      for (;i<l.size();++i){
	if (!equalposcomp(elim,l[i]))
	  break;
      }
      if (1 
	  // ||(l.size()+3-(i%4)<=14)
	  ){
	for (;i%4<3;++i)
	  l.insert(l.begin()+i,0);
	if (l.size()>=
#ifdef GIAC_CHARDEGTYPE
	    15
#else
	    16
#endif
	    ){
	  int lim=nextpow2(es);
#ifdef GIAC_CHARDEGTYPE
	  if (es<=7)
	    lim=8;
#else
	  if (es<=11)
	    lim=12;
	  if (es<=7)
	    lim=8;
	  if (es<=3)
	    lim=4;
#endif
	  for (;int(i)<lim;++i)
	    l.insert(l.begin()+i,0);
	  if (lim<16) i--;
	}
	if (l.size()==15) l.insert(l.begin()+i,0); // insure that the fast algo in cocoa.cc is not called because it would fail
	order = i; // double revlex ordering of type 3/7/11/16/32/64
	l=vecteur(1,l);
	alg_lvar(eqs,l);
	// convert eq to polynomial
	vecteur eq_in(*e2r(eqs,l,contextptr)._VECTptr);
	vectpoly eqp;
	if (!vecteur2vector_polynome(eq_in,l,eqp)){
	  for (int i=0;i<int(eq_in.size());++i){
	    gen tmp=eq_in[i];
	    if (is_integer(tmp) || tmp.type==_FRAC)
	      return vecteur(1,1);	      
	  }
	  return gensizeerr(contextptr);
	}
	gen coeff;
	environment env ;
	env.moduloon = false;    
	for (unsigned i=0;i<eqp.size();++i){
	  if (coefftype(eqp[i],coeff)==_MOD){
	    env.moduloon = true;
	    env.modulo = *(coeff._MODptr+1);
	    env.pn=env.modulo;
	    vectpoly::iterator it=eqp.begin(),itend=eqp.end();
	    for (;it!=itend;++it)
	      *it=unmodularize(*it);
	    break;
	  }
	}
	// add "x_k=0" equation for fake variables x_k in eqp
	if (l.front().type==_VECT){
	  vecteur lf=*l.front()._VECTptr;
	  for (int i=0;i<int(lf.size());++i){
	    if (is_zero(lf[i])){
	      index_t idx(lf.size());
	      idx[i]=1;
	      eqp.push_back(polynome(int(lf.size())));
	      eqp.back().coord.push_back(monomial<gen>(1,idx));
	    }
	  }
	}
	change_monomial_order(eqp,order);
	if (debug_infolevel)
	  CERR << "eliminate revlex/revlex with " << order << " variables " << '\n';
	int rur=0;
	vectpoly eqpr;
	if (gbasis_param.eliminate_flag && !eqp.empty() && eqp.front().dim==order+1){
	  rur=2;
	  eqpr=gbasis(eqp,makevecteur(_REVLEX_ORDER,0),false,modular,&env,rur,contextptr,gbasis_param);
	}
	if (rur==0)
	  eqpr=gbasis(eqp,makevecteur(order,lexvars),false,modular,&env,rur,contextptr,gbasis_param);
	vectpoly::const_iterator it=eqpr.begin(),itend=eqpr.end();
	gb.reserve(itend-it);
	if (returngb && returngb!=3){
	  for (;it!=itend;++it){
	    gb.push_back(r2e(*it,l,contextptr));
	  }
	}
	else {
	  for (;it!=itend;++it){
	    // keep *it if it does not depend on elim
	    if (it->coord.empty())
	      continue;
	    const index_m & i=it->coord.front().index;
	    index_t::const_iterator jt=i.begin(),jtend=jt+order;
	    for (;jt!=jtend;++jt){
	      if (*jt!=0)
		break;
	    }
	    if (jt==jtend){
	      gen tmp=r2e(*it,l,contextptr);
	      if (!is_zero(tmp))
		gb.push_back(tmp);
	    }
	  }
	  if (debug_infolevel)
	    COUT << CLOCK() << " end eliminate" << '\n';
	  return gb;
	}
      }
    }
#endif
    if (order==_PLEX_ORDER)
      gb=gen2vecteur(_gbasis(gen(makevecteur(eqs,l,change_subtype(order,_INT_GROEBNER)),_SEQ__VECT),contextptr));
    // keep in gb values that do not depend on elim
    for (unsigned i=0;i<gb.size();++i){
      vecteur v=lidnt_with_at(gb[i]);
      if (is_zero(derive(v,elim,contextptr),contextptr)){
	res.push_back(gb[i]);
      }
      if (returngb==2 && gb[i].is_symb_of_sommet(at_plus)){
	gb[i]=gb[i][1];
      }
    }
    if (returngb)
      return makevecteur(res,gb);
#if 0 // def GIAC_ELIMINATE1
    vecteur othervars=lidnt_with_at(res),addres;
    gen gres=_gbasis(makesequence(res,othervars),contextptr);
    if (gres.type==_VECT){
      res=*gres._VECTptr;
      for (unsigned i=0;i<gb.size();++i){
	vecteur v=lidnt_with_at(gb[i]);
	if (!is_zero(derive(v,elim,contextptr),contextptr)){
	  gen c=_content(makesequence(gb[i],elim),contextptr);
	  c=_greduce(makesequence(c,res,othervars),contextptr);
	  if (!lidnt_with_at(c).empty()){
	    addres.push_back(c);
	  }
	}
      }
    }
    return mergevecteur(res,addres);
#endif
    //return _gbasis(makesequence(res,lidnt_with_at(res)),contextptr);
    return res;
  }
  static const char _eliminate_s []="eliminate";
  static define_unary_function_eval (__eliminate,&_eliminate,_eliminate_s);
  define_unary_function_ptr5( at_eliminate ,alias_at_eliminate,&__eliminate,0,true);

  // algsubs(eqs,vars)
  gen _algsubs(const gen & args,GIAC_CONTEXT){
    if (args.type!=_VECT || args._VECTptr->size()!=2)
      return gensizeerr(contextptr);
    gen eq=args._VECTptr->front();
    vecteur term=gen2vecteur(_fxnd(args._VECTptr->back(),contextptr));
    if (term.size()!=2 || !is_equal(eq))
      return gensizeerr();
    gen idnt(identificateur(" algsubs"));
    gen ee=term[0]-term[1]*idnt;
    gen lhs=eq._SYMBptr->feuille[0],rhs=eq._SYMBptr->feuille[1];
    term=gen2vecteur(_fxnd(lhs,contextptr));
    if (term.size()!=2) return gensizeerr(contextptr);
    gen eq1=term[0]-term[1]*rhs;
    vecteur ids(lidnt_with_at(eq));
    vecteur sol;
    for (;!ids.empty();){
      sol=gen2vecteur(_eliminate(makevecteur(makevecteur(eq1,ee),ids),contextptr));
      if (!sol.empty())
	break;
      ids.pop_back();
    }
    gen solu=_solve(gen(makevecteur(sol,vecteur(1,idnt)),_SEQ__VECT),contextptr);
    if (equalposcomp(lidnt_with_at(solu),idnt))
      return gensizeerr(gettext("Error solving equations. Check that your variables are purged"));
    if (solu.type!=_VECT)
      return gensizeerr(contextptr);
    if (solu._VECTptr->empty())
      return args._VECTptr->back();
    if (solu._VECTptr->size()>1)
      *logptr(contextptr) << gettext("Warning: algsubs selected one branch. Consider running G:=gbasis(") << gen2vecteur(eq) << ","<< ids << ");greduce("<<args._VECTptr->back()<<",G," << ids << ");" << '\n';
    return normal(solu[0][0],contextptr);
  }
  static const char _algsubs_s []="algsubs";
  static define_unary_function_eval (__algsubs,&_algsubs,_algsubs_s);
  define_unary_function_ptr5( at_algsubs ,alias_at_algsubs,&__algsubs,0,true);

  // in_ideal([Pi],[gb],[vars]) -> true/false
  gen _in_ideal(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    gen res=_greduce(args,contextptr);
    if (res.type==_VECT){
      vecteur v=*res._VECTptr;
      for (int i=0;i<int(v.size());++i){
	v[i]=is_zero(v[i])?1:0;
      }
      return v;
    }
    return is_zero(res);
#if 0
    if (args.type!=_VECT)
      return gensizeerr(contextptr);
    vecteur & v = *args._VECTptr;
    int s=int(v.size());
    if (s<3)
      return gentoofewargs("in_ideal");
    if ( v[1].type!=_VECT || v[2].type!=_VECT )
      return gensizeerr(contextptr);
    vecteur atester=gen2vecteur(v[0]);
    vecteur l=vecteur(1,v[2]);
    alg_lvar(v[1],l);
    alg_lvar(v[0],l);
    gen order=_PLEX_ORDER; // _REVLEX_ORDER;
    bool with_f5=false,with_cocoa=false;
    gbasis_param_t gbasis_param={false,-1,-1,-1,true,false,false,-1,vector<int>(0)};
    int modular=1;
    read_gbargs(v,3,s,order,with_cocoa,with_f5,modular,gbasis_param);
    // convert eq to polynomial
    vecteur eq_in(*e2r(v[1],l,contextptr)._VECTptr);
    vecteur r(*e2r(atester,l,contextptr)._VECTptr);
    vectpoly eqp,eqr;
    if (!vecteur2vector_polynome(eq_in,l,eqp) || !vecteur2vector_polynome(r,l,eqr))
      return gensizeerr(contextptr);
    gen coeff;
    environment env ;
    if (!eqp.empty() && coefftype(eqp.front(),coeff)==_MOD){
      with_cocoa = false;
      env.moduloon = true;
      env.modulo = *(coeff._MODptr+1);
      env.pn=env.modulo;
      vectpoly::iterator it=eqp.begin(),itend=eqp.end();
      for (;it!=itend;++it)
	*it=unmodularize(*it);
    }
    else
      env.moduloon = false;
    if (!with_cocoa){
      change_monomial_order(eqp,order);
      change_monomial_order(eqr,order);
    }
    // is r in ideal eqp?
    gen res=in_ideal(eqr,l,eqp,order,with_cocoa,with_f5,&env);
    if (res.type==_VECT && res._VECTptr->size()==1 && v[0].type!=_VECT)
      return res._VECTptr->front();
    return res;
#endif
  }
  static const char _in_ideal_s []="in_ideal";
  static define_unary_function_eval (__in_ideal,&_in_ideal,_in_ideal_s);
  define_unary_function_ptr5( at_in_ideal ,alias_at_in_ideal,&__in_ideal,0,true);

  // returns 0 for 0 solution, 1 for 1 solution, 2 for infinity solution
  // -1 on error
  int aspen_linsolve(const matrice & m,GIAC_CONTEXT){
    gen k=_ker(exact(gen(m),contextptr),contextptr);
    if (is_undef(k) || k.type!=_VECT) return -1;
    if (k._VECTptr->empty()) return 0;
    if (is_zero(k._VECTptr->back()._VECTptr->back(),contextptr))
      return 0;
    if (k._VECTptr->size()==1)
      return 1;
    return 2;
  }
  // returns 0 for 0 solution, 1 for 1 solution, 2 for infinity solution
  // -1 on error
  int aspen_linsolve_2x2(const gen & a,const gen &b,const gen &c,
			  const gen &d,const gen & e,const gen & f,GIAC_CONTEXT){
    matrice m(makevecteur(makevecteur(a,b,c),makevecteur(d,e,f)));
    return aspen_linsolve(m,contextptr);
  }
  // returns 0 for 0 solution, 1 for 1 solution, 2 for infinity solution
  // -1 on error
  int aspen_linsolve_3x3(const gen & a,const gen &b,const gen &c,const gen &d,
			  const gen & e,const gen &f,const gen & g,const gen &h,
			  const gen & i,const gen & j,const gen &k,const gen &l,GIAC_CONTEXT){
    matrice m(makevecteur(makevecteur(a,b,c,d),makevecteur(e,f,g,h),makevecteur(i,j,k,l)));
    return aspen_linsolve(m,contextptr);
  }

#if defined(GIAC_HAS_STO_38) || defined(ConnectivityKit)
  gen fmin_cobyla(const gen & f,const vecteur & constraints,const vecteur & variables,const vecteur & guess,const gen & eps0,const gen & maxiter0,GIAC_CONTEXT){
    return gensizeerr(contextptr);
  }
#else // GIAC_HAS_STO_38
  // state is a pointer of type gen_context
  int cobyla_giac_function(int n, int m, double *x, double *f, double *con,void *state){
    cobyla_gc * gptr=(cobyla_gc *)state;
    if (gptr->g.type!=_VECT || gptr->g._VECTptr->size()!=3)
      return 1; //error
    gen F=(*gptr->g._VECTptr)[0];
    vecteur conditions=gen2vecteur((*gptr->g._VECTptr)[1]);
    vecteur variables=gen2vecteur((*gptr->g._VECTptr)[2]);
    if (int(conditions.size())!=m || int(variables.size())!=n)
      return 1;
    vecteur values(n);
    for (int i=0;i<n;++i)
      values[i]=x[i];
    gen Fx=subst(F,variables,values,false,gptr->contextptr);
    Fx=_evalf(Fx,gptr->contextptr); // changed evalf_double to _evalf which allows for quoted objectives (L. Marohnić)
    if (Fx.type!=_DOUBLE_)
      return 1;
    *f=Fx._DOUBLE_val;
    gen conditionsx=subst(conditions,variables,values,false,gptr->contextptr);
    if (conditionsx.type!=_VECT || int(conditionsx._VECTptr->size())!=m)
      return 1;
    vecteur & conditionsv=*conditionsx._VECTptr;
    for (int i=0;i<m;++i){
      gen cx=evalf_double(conditionsv[i],1,gptr->contextptr);
      if (cx.type!=_DOUBLE_)
	return 1;
      con[i]=cx._DOUBLE_val;
    }
    return 0;
  }

  // COBYLA will try to make all the values of the constraints positive.
  // So if you want to input a constraint j such as x[i] <= MAX, set:
  // con[j] = MAX - x[i]
  gen fmin_cobyla(const gen & f,const vecteur & constraints,const vecteur & variables,const vecteur & guess,const gen & eps0,const gen & maxiter0,GIAC_CONTEXT){
    vecteur con;
    const_iterateur ct=constraints.begin(),ctend=constraints.end();
    for (;ct!=ctend;++ct){
      if (ct->type!=_SYMB || ct->_SYMBptr->feuille.type!=_VECT || ct->_SYMBptr->feuille._VECTptr->size()!=2 || ct->_SYMBptr->sommet!=at_equal){
	con.push_back(*ct);
	continue;
      }
      con.push_back(ct->_SYMBptr->feuille._VECTptr->back()-ct->_SYMBptr->feuille._VECTptr->front());
      con.push_back(ct->_SYMBptr->feuille._VECTptr->front()-ct->_SYMBptr->feuille._VECTptr->back());
    }
    iterateur it=con.begin(),itend=con.end();
    for (;it!=itend;++it){
      if (it->type!=_SYMB || it->_SYMBptr->feuille.type!=_VECT || it->_SYMBptr->feuille._VECTptr->size()!=2)
	continue;
      if (it->_SYMBptr->sommet==at_superieur_strict || it->_SYMBptr->sommet==at_superieur_egal)
	*it=it->_SYMBptr->feuille._VECTptr->front()-it->_SYMBptr->feuille._VECTptr->back();
      if (it->_SYMBptr->sommet==at_inferieur_strict || it->_SYMBptr->sommet==at_inferieur_egal)
	*it=it->_SYMBptr->feuille._VECTptr->back()-it->_SYMBptr->feuille._VECTptr->front();
    }
    gen fcv=makevecteur(f,con,variables);
    cobyla_gc gc={fcv,contextptr};
    int n=variables.size(),m=con.size(),message(debug_infolevel),maxfun(1000);
    gen maxiter(maxiter0);
    gen eps0d=evalf_double(eps0,1,contextptr);
    if (is_greater(1,maxiter,contextptr) || is_greater(eps0d,1,contextptr))
      swapgen(maxiter,eps0d);
    if (is_integral(maxiter))
      maxfun=maxiter.val;
    if (int(guess.size())!=n)
      return gendimerr(contextptr);
/*
#ifdef VISUALC
    double x[100];
#else
    double x[n];
#endif
*/
    double *x=new double[n]; // added by L. Marohnić
    for (int i=0;i<n;++i){
      gen tmp=evalf_double(guess[i],1,contextptr);
      if (tmp.type!=_DOUBLE_)
	return gensizeerr(contextptr);
      x[i]=tmp._DOUBLE_val;
    }
    double eps;
    if (eps0d.type==_DOUBLE_)
      eps=eps0d._DOUBLE_val;
    else {
      eps=(x[0]>1e-10?absdouble(x[0]):1)*epsilon(contextptr);
      if (eps<1e-13)
	eps=1e-13;
    }
    int cres=cobyla(n,m,x,x[0]/100,eps,message,&maxfun,cobyla_giac_function,&gc);
    vecteur res(n);
    for (int i=0;i<n;++i)
      res[i]=x[i];
    delete[] x;
    if (cres==0)
      return res;
    return makevecteur(string2gen(gettext("Unable to minimize at given precision, last value "),false),res); // changed 2021/feb/22 to be able to process the result further
    *logptr(contextptr) << gettext("Unable to minimize at given precision, last value ") << res << '\n';
    return undef;
  }

  int giac_cobyla(cobyla_gc *gc,vecteur &x0,int &maxiter,double eps,int msg) {
    int n = gc->g._VECTptr->at(2)._VECTptr->size(), m = gc->g._VECTptr->at(1)._VECTptr->size();
    double *x;
#ifndef NO_STDEXCEPT
    try {
     x = new double[n];
    } catch (const std::bad_alloc &e) {
      return COBYLA_ENOMEM;
    }
#else
    x = (double *) malloc (n*sizeof(double));
    if (!x) return COBYLA_ENOMEM;
#endif
    for (int i = 0; i < n; ++i)
    {
      gen tmp = evalf_double(x0[i], 1, gc->contextptr);
      if (tmp.type != _DOUBLE_)
        return COBYLA_EINVAL;
      x[i] = tmp._DOUBLE_val;
    }
    int cres = cobyla(n, m, x, x[0] / 100, eps, msg, &maxiter, cobyla_giac_function, gc);
    vecteur res(n);
    for (int i = 0; i < n; ++i)
      x0[i] = x[i];
#ifndef NO_STDEXCEPT
    delete[] x;
#else
    free(x);
#endif
    return cres;
  }

#endif // GIAC_HAS_STO_38


#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC

#if !defined(GIAC_HAS_STO_38) && !defined(ConnectivityKit)
/* cobyla : contrained optimization by linear approximation */

/*
 * Copyright (c) 1992, Michael J. D. Powell (M.J.D.Powell@damtp.cam.ac.uk)
 * Copyright (c) 2004, Jean-Sebastien Roy (js@jeannot.org)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * This software is a C version of COBYLA2, a contrained optimization by linear
 * approximation package developed by Michael J. D. Powell in Fortran.
 * 
 * The original source code can be found at :
 * http://plato.la.asu.edu/topics/problems/nlores.html
 */

static char const rcsid[] =
  "@(#) $Jeannot: cobyla.c,v 1.11 2004/04/18 09:51:36 js Exp $";

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define abs(x) ((x) >= 0 ? (x) : -(x))

/*
 * Return code strings
 */
const char *cobyla_rc_string[6] =
{
  "N<0 or M<0",
  "Memory allocation failed",
  "Normal return from cobyla",
  "Maximum number of function evaluations reached",
  "Rounding errors are becoming damaging",
  "User requested end of minimization"
};

static int cobylb(int *n, int *m, int *mpp, double *x, double *rhobeg,
  double *rhoend, int *iprint, int *maxfun, double *con, double *sim,
  double *simi, double *datmat, double *a, double *vsig, double *veta,
  double *sigbar, double *dx, double *w, int *iact, cobyla_function *calcfc,
  void *state);
static int trstlp(int *n, int *m, double *a, double *b, double *rho,
  double *dx, int *ifull, int *iact, double *z__, double *zdota, double *vmultc,
  double *sdirn, double *dxnew, double *vmultd);

/* ------------------------------------------------------------------------ */

int cobyla(int n, int m, double *x, double rhobeg, double rhoend, int iprint,
  int *maxfun, cobyla_function *calcfc, void *state)
{
  int icon, isim, isigb, idatm, iveta, isimi, ivsig, iwork, ia, idx, mpp, rc;
  int *iact;
  double *w;
  if (rhobeg<0) rhobeg=-rhobeg;
  if (rhobeg<100*rhoend) rhobeg=100*rhoend;
/*
 * This subroutine minimizes an objective function F(X) subject to M
 * inequality constraints on X, where X is a vector of variables that has 
 * N components. The algorithm employs linear approximations to the 
 * objective and constraint functions, the approximations being formed by 
 * linear interpolation at N+1 points in the space of the variables. 
 * We regard these interpolation points as vertices of a simplex. The 
 * parameter RHO controls the size of the simplex and it is reduced 
 * automatically from RHOBEG to RHOEND. For each RHO the subroutine tries 
 * to achieve a good vector of variables for the current size, and then 
 * RHO is reduced until the value RHOEND is reached. Therefore RHOBEG and 
 * RHOEND should be set to reasonable initial changes to and the required 
 * accuracy in the variables respectively, but this accuracy should be 
 * viewed as a subject for experimentation because it is not guaranteed. 
 * The subroutine has an advantage over many of its competitors, however, 
 * which is that it treats each constraint individually when calculating 
 * a change to the variables, instead of lumping the constraints together 
 * into a single penalty function. The name of the subroutine is derived 
 * from the phrase Constrained Optimization BY Linear Approximations. 
 *
 * The user must set the values of N, M, RHOBEG and RHOEND, and must 
 * provide an initial vector of variables in X. Further, the value of 
 * IPRINT should be set to 0, 1, 2 or 3, which controls the amount of 
 * printing during the calculation. Specifically, there is no output if 
 * IPRINT=0 and there is output only at the end of the calculation if 
 * IPRINT=1. Otherwise each new value of RHO and SIGMA is printed. 
 * Further, the vector of variables and some function information are 
 * given either when RHO is reduced or when each new value of F(X) is 
 * computed in the cases IPRINT=2 or IPRINT=3 respectively. Here SIGMA 
 * is a penalty parameter, it being assumed that a change to X is an 
 * improvement if it reduces the merit function 
 *      F(X)+SIGMA*MAX(0.0,-C1(X),-C2(X),...,-CM(X)), 
 * where C1,C2,...,CM denote the constraint functions that should become 
 * nonnegative eventually, at least to the precision of RHOEND. In the 
 * printed output the displayed term that is multiplied by SIGMA is 
 * called MAXCV, which stands for 'MAXimum Constraint Violation'. The 
 * argument MAXFUN is an int variable that must be set by the user to a 
 * limit on the number of calls of CALCFC, the purpose of this routine being 
 * given below. The value of MAXFUN will be altered to the number of calls 
 * of CALCFC that are made. The arguments W and IACT provide real and 
 * int arrays that are used as working space. Their lengths must be at 
 * least N*(3*N+2*M+11)+4*M+6 and M+1 respectively. 
 *
 * In order to define the objective and constraint functions, we require 
 * a subroutine that has the name and arguments 
 *      SUBROUTINE CALCFC (N,M,X,F,CON) 
 *      DIMENSION X(*),CON(*)  . 
 * The values of N and M are fixed and have been defined already, while 
 * X is now the current vector of variables. The subroutine should return 
 * the objective and constraint functions at X in F and CON(1),CON(2), 
 * ...,CON(M). Note that we are trying to adjust X so that F(X) is as 
 * small as possible subject to the constraint functions being nonnegative. 
 *
 * Partition the working space array W to provide the storage that is needed 
 * for the main calculation.
 */

  if (n == 0)
  {
    if (iprint>=1) fprintf(stderr, "cobyla: N==0.\n");
    *maxfun = 0;
    return 0;
  }

  if (n < 0 || m < 0)
  {
    if (iprint>=1) fprintf(stderr, "cobyla: N<0 or M<0.\n");
    *maxfun = 0;
    return -2;
  }

  /* workspace allocation */
  w = (double *) malloc((n*(3*n+2*m+11)+4*m+6)*sizeof(*w));
  if (w == NULL)
  {
    if (iprint>=1) fprintf(stderr, "cobyla: memory allocation error.\n");
    *maxfun = 0;
    return -1;
  }
  iact = (int *) malloc((m+1)*sizeof(*iact));
  if (iact == NULL)
  {
    if (iprint>=1) fprintf(stderr, "cobyla: memory allocation error.\n");
    free(w);
    *maxfun = 0;
    return -1;
  }
  
  /* Parameter adjustments */
  --iact;
  --w;
  --x;

  /* Function Body */
  mpp = m + 2;
  icon = 1;
  isim = icon + mpp;
  isimi = isim + n * n + n;
  idatm = isimi + n * n;
  ia = idatm + n * mpp + mpp;
  ivsig = ia + m * n + n;
  iveta = ivsig + n;
  isigb = iveta + n;
  idx = isigb + n;
  iwork = idx + n;
  rc = cobylb(&n, &m, &mpp, &x[1], &rhobeg, &rhoend, &iprint, maxfun,
      &w[icon], &w[isim], &w[isimi], &w[idatm], &w[ia], &w[ivsig], &w[iveta],
      &w[isigb], &w[idx], &w[iwork], &iact[1], calcfc, state);

  /* Parameter adjustments (reverse) */
  ++iact;
  ++w;

  free(w);
  free(iact);
  
  return rc;
} /* cobyla */

/* ------------------------------------------------------------------------- */
int cobylb(int *n, int *m, int *mpp, double 
    *x, double *rhobeg, double *rhoend, int *iprint, int *
    maxfun, double *con, double *sim, double *simi, 
    double *datmat, double *a, double *vsig, double *veta,
     double *sigbar, double *dx, double *w, int *iact, cobyla_function *calcfc,
     void *state)
{
  /* System generated locals */
  int sim_dim1, sim_offset, simi_dim1, simi_offset, datmat_dim1, 
      datmat_offset, a_dim1, a_offset, i__1, i__2, i__3;
  double d__1, d__2;

  /* Local variables */
  double alpha, delta, denom, tempa, barmu;
  double beta, cmin = 0.0, cmax = 0.0;
  double cvmaxm, dxsign, prerem = 0.0;
  double edgmax, pareta, prerec = 0.0, phimin, parsig = 0.0;
  double gamma;
  double phi, rho, sum = 0.0;
  double ratio, vmold, parmu, error, vmnew;
  double resmax, cvmaxp;
  double resnew, trured;
  double temp, wsig, f;
  double weta;
  int i__, j, k, l;
  int idxnew;
  int iflag = 0;
  int iptemp;
  int isdirn, nfvals, izdota;
  int ivmc;
  int ivmd;
  int mp, np, iz, ibrnch;
  int nbest, ifull, iptem, jdrop;
  int rc = 0;

/* Set the initial values of some parameters. The last column of SIM holds */
/* the optimal vertex of the current simplex, and the preceding N columns */
/* hold the displacements from the optimal vertex to the other vertices. */
/* Further, SIMI holds the inverse of the matrix that is contained in the */
/* first N columns of SIM. */

  /* Parameter adjustments */
  a_dim1 = *n;
  a_offset = 1 + a_dim1 * 1;
  a -= a_offset;
  simi_dim1 = *n;
  simi_offset = 1 + simi_dim1 * 1;
  simi -= simi_offset;
  sim_dim1 = *n;
  sim_offset = 1 + sim_dim1 * 1;
  sim -= sim_offset;
  datmat_dim1 = *mpp;
  datmat_offset = 1 + datmat_dim1 * 1;
  datmat -= datmat_offset;
  --x;
  --con;
  --vsig;
  --veta;
  --sigbar;
  --dx;
  --w;
  --iact;

  /* Function Body */
  iptem = min(*n,4);
  iptemp = iptem + 1;
  np = *n + 1;
  mp = *m + 1;
  alpha = .25;
  beta = 2.1;
  gamma = .5;
  delta = 1.1;
  rho = *rhobeg;
  parmu = 0.;
  if (*iprint >= 2) {
    fprintf(stderr,
      "cobyla: the initial value of RHO is %12.6E and PARMU is set to zero.\n",
      rho);
  }
  nfvals = 0;
  temp = 1. / rho;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    sim[i__ + np * sim_dim1] = x[i__];
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
      sim[i__ + j * sim_dim1] = 0.;
      simi[i__ + j * simi_dim1] = 0.;
    }
    sim[i__ + i__ * sim_dim1] = rho;
    simi[i__ + i__ * simi_dim1] = temp;
  }
  jdrop = np;
  ibrnch = 0;

/* Make the next call of the user-supplied subroutine CALCFC. These */
/* instructions are also used for calling CALCFC during the iterations of */
/* the algorithm. */

L40:
  if (nfvals >= *maxfun && nfvals > 0) {
    if (*iprint >= 1) {
      fprintf(stderr,
        "cobyla: maximum number of function evaluations reach.\n");
    }
    rc = 1;
    goto L600;
  }
  ++nfvals;
  if (calcfc(*n, *m, &x[1], &f, &con[1], state))
  {
    if (*iprint >= 1) {
      fprintf(stderr, "cobyla: user requested end of minimization.\n");
    }
    rc = 3;
    goto L600;
  }
  resmax = 0.;
  if (*m > 0) {
    i__1 = *m;
    for (k = 1; k <= i__1; ++k) {
      d__1 = resmax, d__2 = -con[k];
      resmax = max(d__1,d__2);
    }
  }
  if (nfvals == *iprint - 1 || *iprint == 3) {
    fprintf(stderr, "cobyla: NFVALS = %4d, F =%13.6E, MAXCV =%13.6E\n",
      nfvals, f, resmax);
    i__1 = iptem;
    fprintf(stderr, "cobyla: X =");
    for (i__ = 1; i__ <= i__1; ++i__) {
      if (i__>1) fprintf(stderr, "  ");
      fprintf(stderr, "%13.6E", x[i__]);
    }
    if (iptem < *n) {
      i__1 = *n;
      for (i__ = iptemp; i__ <= i__1; ++i__) {
        if (!((i__-1) % 4)) fprintf(stderr, "\ncobyla:  ");
        fprintf(stderr, "%15.6E", x[i__]);
      }
    }
    fprintf(stderr, "\n");
  }
  con[mp] = f;
  con[*mpp] = resmax;
  if (ibrnch == 1) {
    goto L440;
  }

/* Set the recently calculated function values in a column of DATMAT. This */
/* array has a column for each vertex of the current simplex, the entries of */
/* each column being the values of the constraint functions (if any) */
/* followed by the objective function and the greatest constraint violation */
/* at the vertex. */

  i__1 = *mpp;
  for (k = 1; k <= i__1; ++k) {
    datmat[k + jdrop * datmat_dim1] = con[k];
  }
  if (nfvals > np) {
    goto L130;
  }

/* Exchange the new vertex of the initial simplex with the optimal vertex if */
/* necessary. Then, if the initial simplex is not complete, pick its next */
/* vertex and calculate the function values there. */

  if (jdrop <= *n) {
    if (datmat[mp + np * datmat_dim1] <= f) {
      x[jdrop] = sim[jdrop + np * sim_dim1];
    } else {
      sim[jdrop + np * sim_dim1] = x[jdrop];
      i__1 = *mpp;
      for (k = 1; k <= i__1; ++k) {
        datmat[k + jdrop * datmat_dim1] = datmat[k + np * datmat_dim1]
            ;
        datmat[k + np * datmat_dim1] = con[k];
      }
      i__1 = jdrop;
      for (k = 1; k <= i__1; ++k) {
        sim[jdrop + k * sim_dim1] = -rho;
        temp = 0.f;
        i__2 = jdrop;
        for (i__ = k; i__ <= i__2; ++i__) {
          temp -= simi[i__ + k * simi_dim1];
        }
        simi[jdrop + k * simi_dim1] = temp;
      }
    }
  }
  if (nfvals <= *n) {
    jdrop = nfvals;
    x[jdrop] += rho;
    goto L40;
  }
L130:
  ibrnch = 1;

/* Identify the optimal vertex of the current simplex. */

L140:
  phimin = datmat[mp + np * datmat_dim1] + parmu * datmat[*mpp + np * 
      datmat_dim1];
  nbest = np;
  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    temp = datmat[mp + j * datmat_dim1] + parmu * datmat[*mpp + j * 
        datmat_dim1];
    if (temp < phimin) {
      nbest = j;
      phimin = temp;
    } else if (temp == phimin && parmu == 0.) {
      if (datmat[*mpp + j * datmat_dim1] < datmat[*mpp + nbest * 
          datmat_dim1]) {
        nbest = j;
      }
    }
  }

/* Switch the best vertex into pole position if it is not there already, */
/* and also update SIM, SIMI and DATMAT. */

  if (nbest <= *n) {
    i__1 = *mpp;
    for (i__ = 1; i__ <= i__1; ++i__) {
      temp = datmat[i__ + np * datmat_dim1];
      datmat[i__ + np * datmat_dim1] = datmat[i__ + nbest * datmat_dim1]
          ;
      datmat[i__ + nbest * datmat_dim1] = temp;
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      temp = sim[i__ + nbest * sim_dim1];
      sim[i__ + nbest * sim_dim1] = 0.;
      sim[i__ + np * sim_dim1] += temp;
      tempa = 0.;
      i__2 = *n;
      for (k = 1; k <= i__2; ++k) {
        sim[i__ + k * sim_dim1] -= temp;
        tempa -= simi[k + i__ * simi_dim1];
      }
      simi[nbest + i__ * simi_dim1] = tempa;
    }
  }

/* Make an error return if SIGI is a poor approximation to the inverse of */
/* the leading N by N submatrix of SIG. */

  error = 0.;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
      temp = 0.;
      if (i__ == j) {
        temp += -1.;
      }
      i__3 = *n;
      for (k = 1; k <= i__3; ++k) {
        temp += simi[i__ + k * simi_dim1] * sim[k + j * sim_dim1];
      }
      d__1 = error, d__2 = abs(temp);
      error = max(d__1,d__2);
    }
  }
  if (error > .1) {
    if (*iprint >= 1) {
      fprintf(stderr, "cobyla: rounding errors are becoming damaging.\n");
    }
    rc = 2;
    goto L600;
  }

/* Calculate the coefficients of the linear approximations to the objective */
/* and constraint functions, placing minus the objective function gradient */
/* after the constraint gradients in the array A. The vector W is used for */
/* working space. */

  i__2 = mp;
  for (k = 1; k <= i__2; ++k) {
    con[k] = -datmat[k + np * datmat_dim1];
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
      w[j] = datmat[k + j * datmat_dim1] + con[k];
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      temp = 0.;
      i__3 = *n;
      for (j = 1; j <= i__3; ++j) {
        temp += w[j] * simi[j + i__ * simi_dim1];
      }
      if (k == mp) {
        temp = -temp;
      }
      a[i__ + k * a_dim1] = temp;
    }
  }

/* Calculate the values of sigma and eta, and set IFLAG=0 if the current */
/* simplex is not acceptable. */

  iflag = 1;
  parsig = alpha * rho;
  pareta = beta * rho;
  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    wsig = 0.;
    weta = 0.;
    i__2 = *n;
    for (i__ = 1; i__ <= i__2; ++i__) {
      d__1 = simi[j + i__ * simi_dim1];
      wsig += d__1 * d__1;
      d__1 = sim[i__ + j * sim_dim1];
      weta += d__1 * d__1;
    }
    vsig[j] = 1. / std::sqrt(wsig);
    veta[j] = std::sqrt(weta);
    if (vsig[j] < parsig || veta[j] > pareta) {
      iflag = 0;
    }
  }

/* If a new vertex is needed to improve acceptability, then decide which */
/* vertex to drop from the simplex. */

  if (ibrnch == 1 || iflag == 1) {
    goto L370;
  }
  jdrop = 0;
  temp = pareta;
  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    if (veta[j] > temp) {
      jdrop = j;
      temp = veta[j];
    }
  }
  if (jdrop == 0) {
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
      if (vsig[j] < temp) {
        jdrop = j;
        temp = vsig[j];
      }
    }
  }

/* Calculate the step to the new vertex and its sign. */

  temp = gamma * rho * vsig[jdrop];
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    dx[i__] = temp * simi[jdrop + i__ * simi_dim1];
  }
  cvmaxp = 0.;
  cvmaxm = 0.;
  i__1 = mp;
  for (k = 1; k <= i__1; ++k) {
    sum = 0.;
    i__2 = *n;
    for (i__ = 1; i__ <= i__2; ++i__) {
      sum += a[i__ + k * a_dim1] * dx[i__];
    }
    if (k < mp) {
      temp = datmat[k + np * datmat_dim1];
      d__1 = cvmaxp, d__2 = -sum - temp;
      cvmaxp = max(d__1,d__2);
      d__1 = cvmaxm, d__2 = sum - temp;
      cvmaxm = max(d__1,d__2);
    }
  }
  dxsign = 1.;
  if (parmu * (cvmaxp - cvmaxm) > sum + sum) {
    dxsign = -1.;
  }

/* Update the elements of SIM and SIMI, and set the next X. */

  temp = 0.;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    dx[i__] = dxsign * dx[i__];
    sim[i__ + jdrop * sim_dim1] = dx[i__];
    temp += simi[jdrop + i__ * simi_dim1] * dx[i__];
  }
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    simi[jdrop + i__ * simi_dim1] /= temp;
  }
  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    if (j != jdrop) {
      temp = 0.;
      i__2 = *n;
      for (i__ = 1; i__ <= i__2; ++i__) {
        temp += simi[j + i__ * simi_dim1] * dx[i__];
      }
      i__2 = *n;
      for (i__ = 1; i__ <= i__2; ++i__) {
        simi[j + i__ * simi_dim1] -= temp * simi[jdrop + i__ * 
            simi_dim1];
      }
    }
    x[j] = sim[j + np * sim_dim1] + dx[j];
  }
  goto L40;

/* Calculate DX=x(*)-x(0). Branch if the length of DX is less than 0.5*RHO. */

L370:
  iz = 1;
  izdota = iz + *n * *n;
  ivmc = izdota + *n;
  isdirn = ivmc + mp;
  idxnew = isdirn + *n;
  ivmd = idxnew + *n;
  trstlp(n, m, &a[a_offset], &con[1], &rho, &dx[1], &ifull, &iact[1], &w[
      iz], &w[izdota], &w[ivmc], &w[isdirn], &w[idxnew], &w[ivmd]);
  if (ifull == 0) {
    temp = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      d__1 = dx[i__];
      temp += d__1 * d__1;
    }
    if (temp < rho * .25 * rho) {
      ibrnch = 1;
      goto L550;
    }
  }

/* Predict the change to F and the new maximum constraint violation if the */
/* variables are altered from x(0) to x(0)+DX. */

  resnew = 0.;
  con[mp] = 0.;
  i__1 = mp;
  for (k = 1; k <= i__1; ++k) {
    sum = con[k];
    i__2 = *n;
    for (i__ = 1; i__ <= i__2; ++i__) {
      sum -= a[i__ + k * a_dim1] * dx[i__];
    }
    if (k < mp) {
      resnew = max(resnew,sum);
    }
  }

/* Increase PARMU if necessary and branch back if this change alters the */
/* optimal vertex. Otherwise PREREM and PREREC will be set to the predicted */
/* reductions in the merit function and the maximum constraint violation */
/* respectively. */

  barmu = 0.;
  prerec = datmat[*mpp + np * datmat_dim1] - resnew;
  if (prerec > 0.) {
    barmu = sum / prerec;
  }
  if (parmu < barmu * 1.5) {
    parmu = barmu * 2.;
    if (*iprint >= 2) {
      fprintf(stderr, "cobyla: increase in PARMU to %12.6E\n", parmu);
    }
    phi = datmat[mp + np * datmat_dim1] + parmu * datmat[*mpp + np * 
        datmat_dim1];
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
      temp = datmat[mp + j * datmat_dim1] + parmu * datmat[*mpp + j * 
          datmat_dim1];
      if (temp < phi) {
        goto L140;
      }
      if (temp == phi && parmu == 0.f) {
        if (datmat[*mpp + j * datmat_dim1] < datmat[*mpp + np * 
            datmat_dim1]) {
          goto L140;
        }
      }
    }
  }
  prerem = parmu * prerec - sum;

/* Calculate the constraint and objective functions at x(*). Then find the */
/* actual reduction in the merit function. */

  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    x[i__] = sim[i__ + np * sim_dim1] + dx[i__];
  }
  ibrnch = 1;
  goto L40;
L440:
  vmold = datmat[mp + np * datmat_dim1] + parmu * datmat[*mpp + np * 
      datmat_dim1];
  vmnew = f + parmu * resmax;
  trured = vmold - vmnew;
  if (parmu == 0. && f == datmat[mp + np * datmat_dim1]) {
    prerem = prerec;
    trured = datmat[*mpp + np * datmat_dim1] - resmax;
  }

/* Begin the operations that decide whether x(*) should replace one of the */
/* vertices of the current simplex, the change being mandatory if TRURED is */
/* positive. Firstly, JDROP is set to the index of the vertex that is to be */
/* replaced. */

  ratio = 0.;
  if (trured <= 0.f) {
    ratio = 1.f;
  }
  jdrop = 0;
  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    temp = 0.;
    i__2 = *n;
    for (i__ = 1; i__ <= i__2; ++i__) {
      temp += simi[j + i__ * simi_dim1] * dx[i__];
    }
    temp = abs(temp);
    if (temp > ratio) {
      jdrop = j;
      ratio = temp;
    }
    sigbar[j] = temp * vsig[j];
  }

/* Calculate the value of ell. */

  edgmax = delta * rho;
  l = 0;
  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    if (sigbar[j] >= parsig || sigbar[j] >= vsig[j]) {
      temp = veta[j];
      if (trured > 0.) {
        temp = 0.;
        i__2 = *n;
        for (i__ = 1; i__ <= i__2; ++i__) {
          d__1 = dx[i__] - sim[i__ + j * sim_dim1];
          temp += d__1 * d__1;
        }
        temp = std::sqrt(temp);
      }
      if (temp > edgmax) {
        l = j;
        edgmax = temp;
      }
    }
  }
  if (l > 0) {
    jdrop = l;
  }
  if (jdrop == 0) {
    goto L550;
  }

/* Revise the simplex by updating the elements of SIM, SIMI and DATMAT. */

  temp = 0.;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    sim[i__ + jdrop * sim_dim1] = dx[i__];
    temp += simi[jdrop + i__ * simi_dim1] * dx[i__];
  }
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    simi[jdrop + i__ * simi_dim1] /= temp;
  }
  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    if (j != jdrop) {
      temp = 0.;
      i__2 = *n;
      for (i__ = 1; i__ <= i__2; ++i__) {
        temp += simi[j + i__ * simi_dim1] * dx[i__];
      }
      i__2 = *n;
      for (i__ = 1; i__ <= i__2; ++i__) {
        simi[j + i__ * simi_dim1] -= temp * simi[jdrop + i__ * 
            simi_dim1];
      }
    }
  }
  i__1 = *mpp;
  for (k = 1; k <= i__1; ++k) {
    datmat[k + jdrop * datmat_dim1] = con[k];
  }

/* Branch back for further iterations with the current RHO. */

  if (trured > 0. && trured >= prerem * .1) {
    goto L140;
  }
L550:
  if (iflag == 0) {
    ibrnch = 0;
    goto L140;
  }

/* Otherwise reduce RHO if it is not at its least value and reset PARMU. */

  if (rho > *rhoend) {
    rho *= .5;
    if (rho <= *rhoend * 1.5) {
      rho = *rhoend;
    }
    if (parmu > 0.) {
      denom = 0.;
      i__1 = mp;
      for (k = 1; k <= i__1; ++k) {
        cmin = datmat[k + np * datmat_dim1];
        cmax = cmin;
        i__2 = *n;
        for (i__ = 1; i__ <= i__2; ++i__) {
          d__1 = cmin, d__2 = datmat[k + i__ * datmat_dim1];
          cmin = min(d__1,d__2);
          d__1 = cmax, d__2 = datmat[k + i__ * datmat_dim1];
          cmax = max(d__1,d__2);
        }
        if (k <= *m && cmin < cmax * .5) {
          temp = max(cmax,0.) - cmin;
          if (denom <= 0.) {
            denom = temp;
          } else {
            denom = min(denom,temp);
          }
        }
      }
      if (denom == 0.) {
        parmu = 0.;
      } else if (cmax - cmin < parmu * denom) {
        parmu = (cmax - cmin) / denom;
      }
    }
    if (*iprint >= 2) {
      fprintf(stderr, "cobyla: reduction in RHO to %12.6E and PARMU =%13.6E\n",
        rho, parmu);
    }
    if (*iprint == 2) {
      fprintf(stderr, "cobyla: NFVALS = %4d, F =%13.6E, MAXCV =%13.6E\n",
        nfvals, datmat[mp + np * datmat_dim1], datmat[*mpp + np * datmat_dim1]);

      fprintf(stderr, "cobyla: X =");
      i__1 = iptem;
      for (i__ = 1; i__ <= i__1; ++i__) {
        if (i__>1) fprintf(stderr, "  ");
        fprintf(stderr, "%13.6E", sim[i__ + np * sim_dim1]);
      }
      if (iptem < *n) {
        i__1 = *n;
        for (i__ = iptemp; i__ <= i__1; ++i__) {
          if (!((i__-1) % 4)) fprintf(stderr, "\ncobyla:  ");
          fprintf(stderr, "%15.6E", x[i__]);
        }
      }
      fprintf(stderr, "\n");
    }
    goto L140;
  }

/* Return the best calculated values of the variables. */

  if (*iprint >= 1) {
    fprintf(stderr, "cobyla: normal return.\n");
  }
  if (ifull == 1) {
    goto L620;
  }
L600:
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    x[i__] = sim[i__ + np * sim_dim1];
  }
  f = datmat[mp + np * datmat_dim1];
  resmax = datmat[*mpp + np * datmat_dim1];
L620:
  if (*iprint >= 1) {
    fprintf(stderr, "cobyla: NFVALS = %4d, F =%13.6E, MAXCV =%13.6E\n",
      nfvals, f, resmax);
    i__1 = iptem;
    fprintf(stderr, "cobyla: X =");
    for (i__ = 1; i__ <= i__1; ++i__) {
      if (i__>1) fprintf(stderr, "  ");
      fprintf(stderr, "%13.6E", x[i__]);
    }
    if (iptem < *n) {
      i__1 = *n;
      for (i__ = iptemp; i__ <= i__1; ++i__) {
        if (!((i__-1) % 4)) fprintf(stderr, "\ncobyla:  ");
        fprintf(stderr, "%15.6E", x[i__]);
      }
    }
    fprintf(stderr, "\n");
  }
  *maxfun = nfvals;
  return rc;
} /* cobylb */

/* ------------------------------------------------------------------------- */
int trstlp(int *n, int *m, double *a, 
    double *b, double *rho, double *dx, int *ifull, 
    int *iact, double *z__, double *zdota, double *vmultc,
     double *sdirn, double *dxnew, double *vmultd)
{
#ifndef MS_SMART // https://connect.microsoft.com/VisualStudio/feedback/details/1028781/crash-c1001-on-relase-build
  /* System generated locals */
  int a_dim1, a_offset, z_dim1, z_offset, i__1, i__2;
  double d__1, d__2;

  /* Local variables */
  double alpha, tempa;
  double beta;
  double optnew, stpful, sum, tot, acca, accb;
  double ratio, vsave, zdotv, zdotw, dd;
  double sd;
  double sp, ss, resold = 0.0, zdvabs, zdwabs, sumabs, resmax, optold;
  double spabs;
  double temp, step;
  int icount;
  int iout, i__, j, k;
  int isave;
  int kk;
  int kl, kp, kw;
  int nact, icon = 0, mcon;
  int nactx = 0;


/* This subroutine calculates an N-component vector DX by applying the */
/* following two stages. In the first stage, DX is set to the shortest */
/* vector that minimizes the greatest violation of the constraints */
/*   A(1,K)*DX(1)+A(2,K)*DX(2)+...+A(N,K)*DX(N) .GE. B(K), K=2,3,...,M, */
/* subject to the Euclidean length of DX being at most RHO. If its length is */
/* strictly less than RHO, then we use the resultant freedom in DX to */
/* minimize the objective function */
/*      -A(1,M+1)*DX(1)-A(2,M+1)*DX(2)-...-A(N,M+1)*DX(N) */
/* subject to no increase in any greatest constraint violation. This */
/* notation allows the gradient of the objective function to be regarded as */
/* the gradient of a constraint. Therefore the two stages are distinguished */
/* by MCON .EQ. M and MCON .GT. M respectively. It is possible that a */
/* degeneracy may prevent DX from attaining the target length RHO. Then the */
/* value IFULL=0 would be set, but usually IFULL=1 on return. */

/* In general NACT is the number of constraints in the active set and */
/* IACT(1),...,IACT(NACT) are their indices, while the remainder of IACT */
/* contains a permutation of the remaining constraint indices. Further, Z is */
/* an orthogonal matrix whose first NACT columns can be regarded as the */
/* result of Gram-Schmidt applied to the active constraint gradients. For */
/* J=1,2,...,NACT, the number ZDOTA(J) is the scalar product of the J-th */
/* column of Z with the gradient of the J-th active constraint. DX is the */
/* current vector of variables and here the residuals of the active */
/* constraints should be zero. Further, the active constraints have */
/* nonnegative Lagrange multipliers that are held at the beginning of */
/* VMULTC. The remainder of this vector holds the residuals of the inactive */
/* constraints at DX, the ordering of the components of VMULTC being in */
/* agreement with the permutation of the indices of the constraints that is */
/* in IACT. All these residuals are nonnegative, which is achieved by the */
/* shift RESMAX that makes the least residual zero. */

/* Initialize Z and some other variables. The value of RESMAX will be */
/* appropriate to DX=0, while ICON will be the index of a most violated */
/* constraint if RESMAX is positive. Usually during the first stage the */
/* vector SDIRN gives a search direction that reduces all the active */
/* constraint violations by one simultaneously. */

  /* Parameter adjustments */
  z_dim1 = *n;
  z_offset = 1 + z_dim1 * 1;
  z__ -= z_offset;
  a_dim1 = *n;
  a_offset = 1 + a_dim1 * 1;
  a -= a_offset;
  --b;
  --dx;
  --iact;
  --zdota;
  --vmultc;
  --sdirn;
  --dxnew;
  --vmultd;

  /* Function Body */
  *ifull = 1;
  mcon = *m;
  nact = 0;
  resmax = 0.;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
      z__[i__ + j * z_dim1] = 0.;
    }
    z__[i__ + i__ * z_dim1] = 1.;
    dx[i__] = 0.;
  }
  if (*m >= 1) {
    i__1 = *m;
    for (k = 1; k <= i__1; ++k) {
      if (b[k] > resmax) {
        resmax = b[k];
        icon = k;
      }
    }
    i__1 = *m;
    for (k = 1; k <= i__1; ++k) {
      iact[k] = k;
      vmultc[k] = resmax - b[k];
    }
  }
  if (resmax == 0.) {
    goto L480;
  }
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    sdirn[i__] = 0.;
  }

/* End the current stage of the calculation if 3 consecutive iterations */
/* have either failed to reduce the best calculated value of the objective */
/* function or to increase the number of active constraints since the best */
/* value was calculated. This strategy prevents cycling, but there is a */
/* remote possibility that it will cause premature termination. */

L60:
  optold = 0.;
  icount = 0;
L70:
  if (mcon == *m) {
    optnew = resmax;
  } else {
    optnew = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      optnew -= dx[i__] * a[i__ + mcon * a_dim1];
    }
  }
  if (icount == 0 || optnew < optold) {
    optold = optnew;
    nactx = nact;
    icount = 3;
  } else if (nact > nactx) {
    nactx = nact;
    icount = 3;
  } else {
    --icount;
    if (icount == 0) {
      goto L490;
    }
  }

/* If ICON exceeds NACT, then we add the constraint with index IACT(ICON) to */
/* the active set. Apply Givens rotations so that the last N-NACT-1 columns */
/* of Z are orthogonal to the gradient of the new constraint, a scalar */
/* product being set to zero if its nonzero value could be due to computer */
/* rounding errors. The array DXNEW is used for working space. */

  if (icon <= nact) {
    goto L260;
  }
  kk = iact[icon];
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    dxnew[i__] = a[i__ + kk * a_dim1];
  }
  tot = 0.;
  k = *n;
L100:
  if (k > nact) {
    sp = 0.;
    spabs = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      temp = z__[i__ + k * z_dim1] * dxnew[i__];
      sp += temp;
      spabs += abs(temp);
    }
    acca = spabs + abs(sp) * .1;
    accb = spabs + abs(sp) * .2;
    if (spabs >= acca || acca >= accb) {
      sp = 0.;
    }
    if (tot == 0.) {
      tot = sp;
    } else {
      kp = k + 1;
      temp = std::sqrt(sp * sp + tot * tot);
      alpha = sp / temp;
      beta = tot / temp;
      tot = temp;
      i__1 = *n;
      for (i__ = 1; i__ <= i__1; ++i__) {
        temp = alpha * z__[i__ + k * z_dim1] + beta * z__[i__ + kp * 
            z_dim1];
        z__[i__ + kp * z_dim1] = alpha * z__[i__ + kp * z_dim1] - 
            beta * z__[i__ + k * z_dim1];
        z__[i__ + k * z_dim1] = temp;
      }
    }
    --k;
    goto L100;
  }

/* Add the new constraint if this can be done without a deletion from the */
/* active set. */

  if (tot != 0.) {
    ++nact;
    zdota[nact] = tot;
    vmultc[icon] = vmultc[nact];
    vmultc[nact] = 0.;
    goto L210;
  }

/* The next instruction is reached if a deletion has to be made from the */
/* active set in order to make room for the new active constraint, because */
/* the new constraint gradient is a linear combination of the gradients of */
/* the old active constraints. Set the elements of VMULTD to the multipliers */
/* of the linear combination. Further, set IOUT to the index of the */
/* constraint to be deleted, but branch if no suitable index can be found. */

  ratio = -1.;
  k = nact;
L130:
  zdotv = 0.;
  zdvabs = 0.;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    temp = z__[i__ + k * z_dim1] * dxnew[i__];
    zdotv += temp;
    zdvabs += abs(temp);
  }
  acca = zdvabs + abs(zdotv) * .1;
  accb = zdvabs + abs(zdotv) * .2;
  if (zdvabs < acca && acca < accb) {
    temp = zdotv / zdota[k];
    if (temp > 0. && iact[k] <= *m) {
      tempa = vmultc[k] / temp;
      if (ratio < 0. || tempa < ratio) {
        ratio = tempa;
        iout = k;
      }
    }
    if (k >= 2) {
      kw = iact[k];
      i__1 = *n;
      for (i__ = 1; i__ <= i__1; ++i__) {
        dxnew[i__] -= temp * a[i__ + kw * a_dim1];
      }
    }
    vmultd[k] = temp;
  } else {
    vmultd[k] = 0.;
  }
  --k;
  if (k > 0) {
    goto L130;
  }
  if (ratio < 0.) {
    goto L490;
  }

/* Revise the Lagrange multipliers and reorder the active constraints so */
/* that the one to be replaced is at the end of the list. Also calculate the */
/* new value of ZDOTA(NACT) and branch if it is not acceptable. */

  i__1 = nact;
  for (k = 1; k <= i__1; ++k) {
    d__1 = 0., d__2 = vmultc[k] - ratio * vmultd[k];
    vmultc[k] = max(d__1,d__2);
  }
  if (icon < nact) {
    isave = iact[icon];
    vsave = vmultc[icon];
    k = icon;
L170:
    kp = k + 1;
    kw = iact[kp];
    sp = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      sp += z__[i__ + k * z_dim1] * a[i__ + kw * a_dim1];
    }
    d__1 = zdota[kp];
    temp = std::sqrt(sp * sp + d__1 * d__1);
    alpha = zdota[kp] / temp;
    beta = sp / temp;
    zdota[kp] = alpha * zdota[k];
    zdota[k] = temp;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      temp = alpha * z__[i__ + kp * z_dim1] + beta * z__[i__ + k * 
          z_dim1];
      z__[i__ + kp * z_dim1] = alpha * z__[i__ + k * z_dim1] - beta * 
          z__[i__ + kp * z_dim1];
      z__[i__ + k * z_dim1] = temp;
    }
    iact[k] = kw;
    vmultc[k] = vmultc[kp];
    k = kp;
    if (k < nact) {
      goto L170;
    }
    iact[k] = isave;
    vmultc[k] = vsave;
  }
  temp = 0.;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    temp += z__[i__ + nact * z_dim1] * a[i__ + kk * a_dim1];
  }
  if (temp == 0.) {
    goto L490;
  }
  zdota[nact] = temp;
  vmultc[icon] = 0.;
  vmultc[nact] = ratio;

/* Update IACT and ensure that the objective function continues to be */
/* treated as the last active constraint when MCON>M. */

L210:
  iact[icon] = iact[nact];
  iact[nact] = kk;
  if (mcon > *m && kk != mcon) {
    k = nact - 1;
    sp = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      sp += z__[i__ + k * z_dim1] * a[i__ + kk * a_dim1];
    }
    d__1 = zdota[nact];
    temp = std::sqrt(sp * sp + d__1 * d__1);
    alpha = zdota[nact] / temp;
    beta = sp / temp;
    zdota[nact] = alpha * zdota[k];
    zdota[k] = temp;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      temp = alpha * z__[i__ + nact * z_dim1] + beta * z__[i__ + k * 
          z_dim1];
      z__[i__ + nact * z_dim1] = alpha * z__[i__ + k * z_dim1] - beta * 
          z__[i__ + nact * z_dim1];
      z__[i__ + k * z_dim1] = temp;
    }
    iact[nact] = iact[k];
    iact[k] = kk;
    temp = vmultc[k];
    vmultc[k] = vmultc[nact];
    vmultc[nact] = temp;
  }

/* If stage one is in progress, then set SDIRN to the direction of the next */
/* change to the current vector of variables. */

  if (mcon > *m) {
    goto L320;
  }
  kk = iact[nact];
  temp = 0.;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    temp += sdirn[i__] * a[i__ + kk * a_dim1];
  }
  temp += -1.;
  temp /= zdota[nact];
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    sdirn[i__] -= temp * z__[i__ + nact * z_dim1];
  }
  goto L340;

/* Delete the constraint that has the index IACT(ICON) from the active set. */

L260:
  if (icon < nact) {
    isave = iact[icon];
    vsave = vmultc[icon];
    k = icon;
L270:
    kp = k + 1;
    kk = iact[kp];
    sp = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      sp += z__[i__ + k * z_dim1] * a[i__ + kk * a_dim1];
    }
    d__1 = zdota[kp];
    temp = std::sqrt(sp * sp + d__1 * d__1);
    alpha = zdota[kp] / temp;
    beta = sp / temp;
    zdota[kp] = alpha * zdota[k];
    zdota[k] = temp;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      temp = alpha * z__[i__ + kp * z_dim1] + beta * z__[i__ + k * 
          z_dim1];
      z__[i__ + kp * z_dim1] = alpha * z__[i__ + k * z_dim1] - beta * 
          z__[i__ + kp * z_dim1];
      z__[i__ + k * z_dim1] = temp;
    }
    iact[k] = kk;
    vmultc[k] = vmultc[kp];
    k = kp;
    if (k < nact) {
      goto L270;
    }
    iact[k] = isave;
    vmultc[k] = vsave;
  }
  --nact;

/* If stage one is in progress, then set SDIRN to the direction of the next */
/* change to the current vector of variables. */

  if (mcon > *m) {
    goto L320;
  }
  temp = 0.;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    temp += sdirn[i__] * z__[i__ + (nact + 1) * z_dim1];
  }
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    sdirn[i__] -= temp * z__[i__ + (nact + 1) * z_dim1];
  }
  goto L340;

/* Pick the next search direction of stage two. */

L320:
  temp = 1. / zdota[nact];
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    sdirn[i__] = temp * z__[i__ + nact * z_dim1];
  }

/* Calculate the step to the boundary of the trust region or take the step */
/* that reduces RESMAX to zero. The two statements below that include the */
/* factor 1.0E-6 prevent some harmless underflows that occurred in a test */
/* calculation. Further, we skip the step if it could be zero within a */
/* reasonable tolerance for computer rounding errors. */

L340:
  dd = *rho * *rho;
  sd = 0.;
  ss = 0.;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    if ((d__1 = dx[i__], abs(d__1)) >= *rho * 1e-6f) {
      d__2 = dx[i__];
      dd -= d__2 * d__2;
    }
    sd += dx[i__] * sdirn[i__];
    d__1 = sdirn[i__];
    ss += d__1 * d__1;
  }
  if (dd <= 0.) {
    goto L490;
  }
  temp = std::sqrt(ss * dd);
  if (abs(sd) >= temp * 1e-6f) {
    temp = std::sqrt(ss * dd + sd * sd);
  }
  stpful = dd / (temp + sd);
  step = stpful;
  if (mcon == *m) {
    acca = step + resmax * .1;
    accb = step + resmax * .2;
    if (step >= acca || acca >= accb) {
      goto L480;
    }
    step = min(step,resmax);
  }

/* Set DXNEW to the new variables if STEP is the steplength, and reduce */
/* RESMAX to the corresponding maximum residual if stage one is being done. */
/* Because DXNEW will be changed during the calculation of some Lagrange */
/* multipliers, it will be restored to the following value later. */

  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    dxnew[i__] = dx[i__] + step * sdirn[i__];
  }
  if (mcon == *m) {
    resold = resmax;
    resmax = 0.;
    i__1 = nact;
    for (k = 1; k <= i__1; ++k) {
      kk = iact[k];
      temp = b[kk];
      i__2 = *n;
      for (i__ = 1; i__ <= i__2; ++i__) {
        temp -= a[i__ + kk * a_dim1] * dxnew[i__];
      }
      resmax = max(resmax,temp);
    }
  }

/* Set VMULTD to the VMULTC vector that would occur if DX became DXNEW. A */
/* device is included to force VMULTD(K)=0.0 if deviations from this value */
/* can be attributed to computer rounding errors. First calculate the new */
/* Lagrange multipliers. */

  k = nact;
L390:
  zdotw = 0.;
  zdwabs = 0.;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    temp = z__[i__ + k * z_dim1] * dxnew[i__];
    zdotw += temp;
    zdwabs += abs(temp);
  }
  acca = zdwabs + abs(zdotw) * .1;
  accb = zdwabs + abs(zdotw) * .2;
  if (zdwabs >= acca || acca >= accb) {
    zdotw = 0.;
  }
  vmultd[k] = zdotw / zdota[k];
  if (k >= 2) {
    kk = iact[k];
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      dxnew[i__] -= vmultd[k] * a[i__ + kk * a_dim1];
    }
    --k;
    goto L390;
  }
  if (mcon > *m) {
    d__1 = 0., d__2 = vmultd[nact];
    vmultd[nact] = max(d__1,d__2);
  }

/* Complete VMULTC by finding the new constraint residuals. */

  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    dxnew[i__] = dx[i__] + step * sdirn[i__];
  }
  if (mcon > nact) {
    kl = nact + 1;
    i__1 = mcon;
    for (k = kl; k <= i__1; ++k) {
      kk = iact[k];
      sum = resmax - b[kk];
      sumabs = resmax + (d__1 = b[kk], abs(d__1));
      i__2 = *n;
      for (i__ = 1; i__ <= i__2; ++i__) {
        temp = a[i__ + kk * a_dim1] * dxnew[i__];
        sum += temp;
        sumabs += abs(temp);
      }
      acca = sumabs + abs(sum) * .1f;
      accb = sumabs + abs(sum) * .2f;
      if (sumabs >= acca || acca >= accb) {
        sum = 0.f;
      }
      vmultd[k] = sum;
    }
  }

/* Calculate the fraction of the step from DX to DXNEW that will be taken. */

  ratio = 1.;
  icon = 0;
  i__1 = mcon;
  for (k = 1; k <= i__1; ++k) {
    if (vmultd[k] < 0.) {
      temp = vmultc[k] / (vmultc[k] - vmultd[k]);
      if (temp < ratio) {
        ratio = temp;
        icon = k;
      }
    }
  }

/* Update DX, VMULTC and RESMAX. */

  temp = 1. - ratio;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    dx[i__] = temp * dx[i__] + ratio * dxnew[i__];
  }
  i__1 = mcon;
  for (k = 1; k <= i__1; ++k) {
    d__1 = 0., d__2 = temp * vmultc[k] + ratio * vmultd[k];
    vmultc[k] = max(d__1,d__2);
  }
  if (mcon == *m) {
    resmax = resold + ratio * (resmax - resold);
  }

/* If the full step is not acceptable then begin another iteration. */
/* Otherwise switch to stage two or end the calculation. */

  if (icon > 0) {
    goto L70;
  }
  if (step == stpful) {
    goto L500;
  }
L480:
  mcon = *m + 1;
  icon = mcon;
  iact[mcon] = mcon;
  vmultc[mcon] = 0.;
  goto L60;

/* We employ any freedom that may be available to reduce the objective */
/* function before returning a DX whose length is less than RHO. */

L490:
  if (mcon == *m) {
    goto L480;
  }
  *ifull = 0;
L500:
#endif // MS_SMART
  return 0;
} /* trstlp */

#endif // GIAC_HAS_STO_38
