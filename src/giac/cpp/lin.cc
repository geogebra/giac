// -*- mode:C++ ; compile-command: "g++ -I.. -g -c lin.cc -DHAVE_CONFIG_H -DIN_GIAC" -*-
#include "giacPCH.h"
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
using namespace std;
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include "sym2poly.h"
#include "usual.h"
#include "lin.h"
#include "subst.h"
#include "modpoly.h"
#include "prog.h"
#include "giacintl.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  // Should be rewritten with a map container for better efficiency!

  bool contains(const gen & e,const unary_function_ptr & mys){
    if (e.type!=_SYMB)
      return false;
    if (e._SYMBptr->sommet==mys)
      return true;
    if (e._SYMBptr->feuille.type!=_VECT)
      return contains(e._SYMBptr->feuille,mys);
    vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
    for (;it!=itend;++it)
      if (contains(*it,mys))
	return true;
    return false;
  }

  void compress(vecteur & res,GIAC_CONTEXT){
    if (res.size()==2) return;
    vecteur v,w;
    const_iterateur it=res.begin(),itend=res.end();
    v.reserve(itend-it);
    w.reserve((itend-it)/2);
    int pos;
    for (;it!=itend;++it){
      pos=equalposcomp(w,*(it+1));
      if (pos){
	v[2*(pos-1)] = recursive_normal(v[2*(pos-1)] + *it,false,contextptr);
	++it;
      }
      else {
	v.push_back(*it);
	++it;
	w.push_back(*it);
	v.push_back(*it);
      }
    }
    swap(res,v);
  }

  // back conversion
  gen unlin(vecteur & v,GIAC_CONTEXT){
    vecteur w;
    gen coeff;
    vecteur::const_iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      coeff = *it;
      ++it;
      if (!is_zero(coeff))
	w.push_back(coeff*exp(*it,contextptr));
    }
    if (w.empty())
      return 0;
    return _plus(w,contextptr);
  }

  void convolution(const gen & coeff, const gen & arg,const vecteur & w,vecteur & res,GIAC_CONTEXT){
    vecteur::const_iterator it=w.begin(),itend=w.end();
    for (;it!=itend;++it){
      res.push_back(coeff*(*it));
      ++it;
      res.push_back(recursive_normal(arg+(*it),false,contextptr));
    }
    compress(res,contextptr);
  }

  void convolution(const vecteur & v,const vecteur & w, vecteur & res,GIAC_CONTEXT){
    res.clear();
    res.reserve(res.size()+v.size()*w.size()/2);
    vecteur::const_iterator it=v.begin(),itend=v.end();
    gen coeff;
    for (;it!=itend;++it){
      coeff = *it;
      ++it;
      convolution(coeff,*it,w,res,contextptr);
    }
  }

  void convolutionpower(const vecteur & v,int k,vecteur & res,GIAC_CONTEXT){
    res.clear();
    // should be improved for efficiency!
    if (k==0){
      res.push_back(1);
      res.push_back(0);
      return;
    }
    if (k==1){
      res=v;
      return;
    }
    convolutionpower(v,k/2,res,contextptr);
    vecteur tmp=res;
    convolution(tmp,tmp,res,contextptr);
    if (k%2){
      tmp=res;
      convolution(tmp,v,res,contextptr);
    }
  }

  // coeff & argument of exponential
  void lin(const gen & e,vecteur & v,bool convert_sqrt,GIAC_CONTEXT){
    if (e.type!=_SYMB){
      v.push_back(e);
      v.push_back(0);
      return ; // e*exp(0)
    }
    // e is symbolic, look for exp, cosh, sinh, +, *, neg and inv, ^
    unary_function_ptr s=e._SYMBptr->sommet;
    if ((s==at_plus) && (e._SYMBptr->feuille.type==_VECT)){
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
      for (;it!=itend;++it)
	lin(*it,v,convert_sqrt,contextptr);
      compress(v,contextptr);
      return;
    }
    if (s==at_neg){
      vecteur tmp;
      lin(e._SYMBptr->feuille,tmp,convert_sqrt,contextptr);
      const_iterateur it=tmp.begin(),itend=tmp.end();
      for (;it!=itend;++it){
	v.push_back(-*it);
	++it;
	v.push_back(*it);
      }
      return;
    }
    if (s==at_inv){
      vecteur w;
      lin(e._SYMBptr->feuille,w,convert_sqrt,contextptr);
      if (w.size()==2){
	v.push_back(inv(w[0],contextptr));
	v.push_back(-w[1]);
      }
      else {
	gen coeff(unlin(w,contextptr));
	v.push_back(inv(coeff,contextptr));
	v.push_back(0);
      }
      return ;
    }
    if (s==at_prod){
      if (e._SYMBptr->feuille.type!=_VECT){
	lin(e._SYMBptr->feuille,v,convert_sqrt,contextptr);
	return;
      }
      vecteur w;
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
      lin(*it,w,convert_sqrt,contextptr);
      ++it;
      for (;it!=itend;++it){
	vecteur v0;
	lin(*it,v0,convert_sqrt,contextptr);
	vecteur res;
	convolution(w,v0,res,contextptr);
	w=res;
      }
      v=mergevecteur(v,w);
      return;
    }
    if (s==at_pow){
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin();
      vecteur w;
      lin(*it,w,convert_sqrt,contextptr);
      ++it;
      if (w.size()==2){
	if ( is_zero(w[1]) && (w[0].type==_INT_ && convert_sqrt) ){
	  w[1]=ln(w[0],contextptr);
	  w[0]=plus_one;
	}
	v.push_back(pow(w[0],*it,contextptr));
	v.push_back(w[1]*(*it));
	return ;
      }
      if ((it->type==_INT_) && (it->val>=0)){
	vecteur z(w),tmp;
	convolutionpower(z,it->val,tmp,contextptr);
	v=mergevecteur(v,tmp);
	compress(v,contextptr);
	return ;
      }
      gen coeff=unlin(w,contextptr);
      v.push_back(pow(coeff,*it,contextptr));
      v.push_back(0);
      return ;
    }
    gen f=_lin(convert_sqrt?e._SYMBptr->feuille:makesequence(e._SYMBptr->feuille,at_sqrt),contextptr);
    if (s==at_exp){
      v.push_back(1);
      v.push_back(f);
      return ; // 1*exp(arg)
    }
    if (s==at_cosh || s==at_sinh){
      v.push_back(plus_one_half);
      v.push_back(f);
      v.push_back(s==at_cosh?plus_one_half:minus_one_half);
      v.push_back(-f);
      return ; // 1/2*exp(arg)+-1/2*exp(-arg)
    }
    v.push_back(symbolic(s,f));
    v.push_back(0);
  }

  void lin(const gen & e,vecteur & v,GIAC_CONTEXT){
    lin(e,v,true,contextptr);
  }

  symbolic symb_lin(const gen & a){
    return symbolic(at_lin,a);
  }

  // "unary" version
  gen _lin(const gen & args_,GIAC_CONTEXT){
    gen args(args_);
    bool convert_sqrt=true;
    if (args.type==_VECT && args.subtype==_SEQ__VECT && args._VECTptr->back()==at_sqrt){
      args=args._VECTptr->front();
      convert_sqrt=false;
    }
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    gen var,res;
    if (is_algebraic_program(args,var,res))
      return symbolic(at_program,makesequence(var,0,_lin(res,contextptr)));
    if (is_equal(args))
      return apply_to_equal(args,_lin,contextptr);
    vecteur v;
    if (args.type!=_VECT){
      lin(args,v,convert_sqrt,contextptr);
      return unlin(v,contextptr);
    }
    return apply(args,_lin,contextptr);
  }
  static const char _lin_s []="lin";
  static define_unary_function_eval (__lin,&_lin,_lin_s);
  define_unary_function_ptr5( at_lin ,alias_at_lin,&__lin,0,true);

  static const char _lineariser_s []="lineariser";
  static define_unary_function_eval (__lineariser,&_lin,_lineariser_s);
  define_unary_function_ptr5( at_lineariser ,alias_at_lineariser,&__lineariser,0,true);

  // back conversion
  gen tunlin(vecteur & v,GIAC_CONTEXT){
    vecteur w;
    gen coeff;
    vecteur::const_iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      coeff = *it;
      ++it;
      coeff=coeff*(*it);
      if (!is_zero(coeff))
	w.push_back(coeff);
    }
    if (w.empty())
      return 0;
    if (w.size()==1)
      return w.front();
    return symbolic(at_plus,gen(w,_SEQ__VECT));
  }

  static void tadd(vecteur & res,const gen & coeff,const gen & angle,GIAC_CONTEXT){
    gen newangle=angle,newcoeff=coeff;
    if ( (newangle.type==_SYMB) && (newangle._SYMBptr->sommet==at_neg)){
      newcoeff=-coeff;
      newangle=-newangle;
    }
    if ( (newangle.type==_SYMB) && ( (newangle._SYMBptr->sommet==at_sin) || (newangle._SYMBptr->sommet==at_cos) ) ){
      res.push_back(newcoeff);
      res.push_back(newangle);
    }
    else {
      newcoeff=newcoeff*newangle;
      if (!is_zero(newcoeff)){
	res.push_back(newcoeff);
	res.push_back(plus_one);
      }
    }
  }

  void tconvolution(const gen & coeff, const gen & arg,const vecteur & w,vecteur & res,GIAC_CONTEXT){
    gen newcoeff,tmp;
    if ((arg.type==_SYMB) && (arg._SYMBptr->sommet==at_cos)){
      vecteur::const_iterator it=w.begin(),itend=w.end();
      for (;it!=itend;++it){
	newcoeff=coeff*(*it);
	++it;
	bool iscos=it->type==_SYMB && it->_SYMBptr->sommet==at_cos;
	if ( (it->type==_SYMB) && (iscos || it->_SYMBptr->sommet==at_sin) ){
	  newcoeff=recursive_normal(rdiv(newcoeff,plus_two,contextptr),false,contextptr);
	  gen tmp1(normal(it->_SYMBptr->feuille+arg._SYMBptr->feuille,false,contextptr));
	  gen tmp2(normal(it->_SYMBptr->feuille-arg._SYMBptr->feuille,false,contextptr));
	  tadd(res,newcoeff,iscos?cos(tmp2,contextptr):sin(tmp1,contextptr),contextptr);
	  tadd(res,newcoeff,iscos?cos(tmp1,contextptr):sin(tmp2,contextptr),contextptr);
	  continue;
	}
	res.push_back(recursive_normal(newcoeff*(*it),false,contextptr));
	res.push_back(arg);
      }
      compress(res,contextptr);
      return;
    }
    if ((arg.type==_SYMB) && (arg._SYMBptr->sommet==at_sin)){
      vecteur::const_iterator it=w.begin(),itend=w.end();
      for (;it!=itend;++it){
	newcoeff=coeff*(*it);
	++it;
	bool iscos=it->type==_SYMB && it->_SYMBptr->sommet==at_cos;
	if ( (it->type==_SYMB) && (iscos || it->_SYMBptr->sommet==at_sin) ){
	  newcoeff=recursive_normal(rdiv(newcoeff,plus_two,contextptr),false,contextptr);
	  gen tmp1(normal(arg._SYMBptr->feuille+it->_SYMBptr->feuille,false,contextptr));
	  gen tmp2(normal(arg._SYMBptr->feuille-it->_SYMBptr->feuille,false,contextptr));
	  if (iscos){
	    tadd(res,newcoeff,sin(tmp1,contextptr),contextptr);
	    tadd(res,newcoeff,sin(tmp2,contextptr),contextptr);
	  }
	  else {
	    tadd(res,newcoeff,cos(tmp2,contextptr),contextptr);
	    tadd(res,-newcoeff,cos(tmp1,contextptr),contextptr);
	  }
	  continue;
	}
	res.push_back(recursive_normal(newcoeff*(*it),false,contextptr));
	res.push_back(arg);
      }
      compress(res,contextptr);
      return;
    }
    const_iterateur it=w.begin(),itend=w.end();
    newcoeff=coeff*arg;
    for (;it!=itend;++it){
      res.push_back(recursive_normal(*it*newcoeff,false,contextptr));
      ++it;
      res.push_back(*it);
    }
  }

  void tconvolution(const vecteur & v,const vecteur & w, vecteur & res,GIAC_CONTEXT){
    res.clear();
    res.reserve(res.size()+v.size()*w.size()/2);
    vecteur::const_iterator it=v.begin(),itend=v.end();
    gen coeff;
    for (;it!=itend;++it){
      coeff = *it;
      ++it;
      tconvolution(coeff,*it,w,res,contextptr);
    }
  }

  void tconvolutionpower(const vecteur & v,int k,vecteur & res,GIAC_CONTEXT){
    res.clear();
    // should be improved for efficiency!
    if (k==0){
      res.push_back(1);
      res.push_back(1);
      return;
    }
    if (k==1){
      res=v;
      return;
    }
    tconvolutionpower(v,k/2,res,contextptr);
    vecteur tmp=res;
    tconvolution(tmp,tmp,res,contextptr);
    if (k%2){
      tmp=res;
      tconvolution(tmp,v,res,contextptr);
    }
  }

  // coeff & argument of sin/cos
  void tlin(const gen & e,vecteur & v,GIAC_CONTEXT){
    if (e.type!=_SYMB){
      v.push_back(e);
      v.push_back(1);
      return ; // e*1
    }
    // e is symbolic, look for cos, sin, +, *, neg and inv, ^
    unary_function_ptr s=e._SYMBptr->sommet;
    if ( (s==at_cos) || (s==at_sin)){
      v.push_back(1);
      v.push_back(e);
      return ; 
    }
    if ((s==at_plus) && (e._SYMBptr->feuille.type==_VECT)){
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
      for (;it!=itend;++it)
	tlin(*it,v,contextptr);
      compress(v,contextptr);
      return;
    }
    if (s==at_neg){
      vecteur w;
      tlin(e._SYMBptr->feuille,w,contextptr);
      const_iterateur it=w.begin(),itend=w.end();
      for (;it!=itend;++it){
	v.push_back(-*it);
	++it;
	v.push_back(*it);
      }
      return;
    }
    if (s==at_prod){
      if (e._SYMBptr->feuille.type!=_VECT){
	tlin(e._SYMBptr->feuille,v,contextptr);
	return;
      }
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
      vecteur w;
      tlin(*it,w,contextptr);
      ++it;
      for (;it!=itend;++it){
	vecteur v0;
	tlin(*it,v0,contextptr);
	vecteur res;
	tconvolution(w,v0,res,contextptr);
	w=res;
      }
      v=mergevecteur(v,w);
      return;
    }
    if (s==at_pow){
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin();
      /*
      if ( (v.size()==2) && ((it+1)->type==_INT_) && ((it+1)->val>=0) ){
	tlin(*it,v);
	++it;
	return tpow(v,it->val);
      }
      */
      if (((it+1)->type==_INT_) && ((it+1)->val>=0)){
	vecteur w;
	tlin(*it,w,contextptr);
	vecteur z(w);
	++it;
	tconvolutionpower(z,it->val,w,contextptr);
	v=mergevecteur(v,w);
	return ;
      }
    }
    gen te=_tlin(e._SYMBptr->feuille,contextptr);
    if (s==at_pow && te.type==_VECT && te._VECTptr->size()==2 && te._VECTptr->back()==plus_one_half)
      v.push_back(sqrt(te._VECTptr->front(),contextptr));
    else
      v.push_back(s(te,contextptr));
    v.push_back(1);
  }

  symbolic symb_tlin(const gen & a){
    return symbolic(at_tlin,a);
  }

  // "unary" version
  gen _tlin(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    gen var,res;
    if (is_algebraic_program(args,var,res))
      return symbolic(at_program,makesequence(var,0,_tlin(res,contextptr)));
    if (is_equal(args))
      return apply_to_equal(args,_tlin,contextptr);
    vecteur v;
    if (args.type!=_VECT){
      tlin(args,v,contextptr);
      return tunlin(v,contextptr);
    }
    return apply(args,_tlin,contextptr);
  }
  static const char _tlin_s []="tlin";
  static define_unary_function_eval (__tlin,&_tlin,_tlin_s);
  define_unary_function_ptr5( at_tlin ,alias_at_tlin,&__tlin,0,true);

  static const char _lineariser_trigo_s []="lineariser_trigo";
  static define_unary_function_eval (__lineariser_trigo,&_tlin,_lineariser_trigo_s);
  define_unary_function_ptr5( at_lineariser_trigo ,alias_at_lineariser_trigo,&__lineariser_trigo,0,true);

  // Expand and texpand
  static void split(const gen & e, gen & coeff, gen & arg,GIAC_CONTEXT){
    if (e.type==_INT_){
      coeff=e;
      arg=plus_one;
    }
    if ( (e.type==_SYMB) && (e._SYMBptr->sommet==at_neg)){
      split(e._SYMBptr->feuille,coeff,arg,contextptr);
      coeff=-coeff;
      return;
    }
    if ( (e.type!=_SYMB) || (e._SYMBptr->sommet!=at_prod) ) {
      coeff=plus_one;
      arg=e;
      return;
    }
    coeff = plus_one;
    arg = plus_one;
    const_iterateur it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
    for (;it!=itend;++it){
      if (it->type==_INT_)
	coeff = coeff * (*it);
      else {
	if ( (it->type==_SYMB)  && (it->_SYMBptr->sommet==at_neg)){
	  coeff = -coeff;
	  arg = arg * (it->_SYMBptr->feuille);
	}
	else
	  arg= arg * (*it);
      }
    }
    if ( (coeff.type==_INT_) && (coeff.val<0) ){
      coeff=-coeff;
      arg=-arg;
    }
  }

  /*
  gen sigma(const vecteur & v){
    // an "intelligent" version of return symbolic(at_plus,v);
    // split each element of v as an integer coeff * an arg
    vecteur vcoeff,varg,res;
    int pos;
    gen coeff,arg;
    const_iterateur it=v.begin(),itend=v.end();
    vcoeff.reserve(itend-it);
    varg.reserve(itend-it);
    for (;it!=itend;++it){
      split(*it,coeff,arg);
      pos=equalposcomp(varg,arg);
      if (!pos){
	vcoeff.push_back(coeff);
	varg.push_back(arg);
      }
      else 
	vcoeff[pos-1]=vcoeff[pos-1]+coeff;
    }
    it=vcoeff.begin(),itend=vcoeff.end();
    res.reserve(itend-it);
    const_iterateur vargit=varg.begin();
    for (;it!=itend;++it,++vargit)
      if (!is_zero(*it))
	res.push_back((*it)*(*vargit));
    if (res.empty())
      return zero;
    if (res.size()>1)
      return symbolic(at_plus,res);
    return res.front();
  }
  */

  gen prod_expand(const gen & a,const gen & b,GIAC_CONTEXT){
    bool a_is_plus= (a.type==_SYMB) && (a._SYMBptr->sommet==at_plus);
    bool b_is_plus= (b.type==_SYMB) && (b._SYMBptr->sommet==at_plus);
    if ( (!a_is_plus) && (!b_is_plus) )
      return a*b;
    if (!a_is_plus) // distribute wrt b
      return symbolic(at_plus,a*(*b._SYMBptr->feuille._VECTptr));
    if (!b_is_plus)
      return symbolic(at_plus,b*(*a._SYMBptr->feuille._VECTptr));
    // distribute wrt a AND b
    const_iterateur ita=a._SYMBptr->feuille._VECTptr->begin(),itaend=a._SYMBptr->feuille._VECTptr->end();
    const_iterateur itb=b._SYMBptr->feuille._VECTptr->begin(),itbend=b._SYMBptr->feuille._VECTptr->end();
    if ((itbend-itb)*(itaend-ita)>MAX_PROD_EXPAND_SIZE)
      return a*b;
    vecteur v;
    v.reserve((itbend-itb)*(itaend-ita));
    for (;ita!=itaend;++ita){
      for (itb=b._SYMBptr->feuille._VECTptr->begin();itb!=itbend;++itb)
	v.push_back( (*ita) * (*itb) );
    }
    return symbolic(at_plus,gen(v,_SEQ__VECT));
  }

  static gen prod_expand(const const_iterateur it,const const_iterateur itend,GIAC_CONTEXT){
    int s=int(itend-it);
    if (s==0)
      return plus_one;
    if (s==1)
      return *it;
    return _simplifier(prod_expand(prod_expand(it,it+s/2,contextptr),prod_expand(it+s/2,itend,contextptr),contextptr),contextptr);
  }
  static gen prod_expand(const gen & e_orig,GIAC_CONTEXT){
    int te=taille(e_orig,MAX_PROD_EXPAND_SIZE);
    if (te>MAX_PROD_EXPAND_SIZE)
      return symbolic(at_prod,e_orig);
    gen e=aplatir_fois_plus(expand(e_orig,contextptr));
    if (taille(e,MAX_PROD_EXPAND_SIZE)>MAX_PROD_EXPAND_SIZE)
      return symbolic(at_prod,e_orig);
    if (e.type!=_VECT)
      return e;
    // look for sommet=at_plus inside e
    return prod_expand(e._VECTptr->begin(),e._VECTptr->end(),contextptr);
  }

  static gen prod_expand_nosimp(const const_iterateur it,const const_iterateur itend,GIAC_CONTEXT){
    int s=int(itend-it);
    if (s==0)
      return plus_one;
    if (s==1)
      return *it;
    return prod_expand(prod_expand_nosimp(it,it+s/2,contextptr),prod_expand_nosimp(it+s/2,itend,contextptr),contextptr);
  }
  static gen prod_expand_nosimp(const gen & e_orig,GIAC_CONTEXT){
    gen e=aplatir_fois_plus(e_orig);
    if (e.type!=_VECT)
      return e;
    // look for sommet=at_plus inside e
    return prod_expand_nosimp(e._VECTptr->begin(),e._VECTptr->end(),contextptr);
  }

  static void pow_expand_add_res(vector<gen> & factn,int pos,int sumexpo,const gen & coeff,const vecteur & w,const gen & p, int k,int n,vecteur & res,GIAC_CONTEXT){
    if (sumexpo==k){
      // End recursion
      res.push_back(coeff*p);
      return;
    }
    if (pos==n-1){
      // End recursion
      res.push_back(coeff/factn[k-sumexpo]*p*expand(pow(w[pos],k-sumexpo),contextptr));
      return;
    }
    for (int i=k-sumexpo;i>=0;--i){
      pow_expand_add_res(factn,pos+1,sumexpo+i,coeff/factn[i],w,expand(p*pow(w[pos],i),contextptr),k,n,res,contextptr);
    }
  }

  static gen expand_pow_expand(const gen & e,GIAC_CONTEXT){
    if (e.type!=_VECT || e._VECTptr->size()!=2)
      return e;
    vecteur & v=*e._VECTptr;
    gen base=expand(v[0],contextptr);
    gen exponent=expand(v[1],contextptr);
    if (v[1].is_symb_of_sommet(at_plus) && v[1]._SYMBptr->feuille.type==_VECT){
      vecteur & w=*v[1]._SYMBptr->feuille._VECTptr;
      const_iterateur it=w.begin(),itend=w.end();
      vecteur prodarg;
      prodarg.reserve(itend-it);
      for (;it!=itend;++it){
	prodarg.push_back(pow(base,*it,contextptr));
      }
      return _prod(prodarg,contextptr);
    }
    if (v[1].type==_INT_ ){
      if (v[0].is_symb_of_sommet(at_prod)&& v[0]._SYMBptr->feuille.type==_VECT){
	vecteur w(*v[0]._SYMBptr->feuille._VECTptr);
	iterateur it=w.begin(),itend=w.end();
	for (;it!=itend;++it){
	  *it=pow(expand(*it,contextptr),v[1],contextptr);
	}
	return _prod(w,contextptr);
      }
    }
    if (v[0].is_symb_of_sommet(at_plus) && v[0]._SYMBptr->feuille.type==_VECT && v[1].type==_INT_ && v[1].val>=0){
      int k=v[1].val;
      if (!k)
	return plus_one;
      if (k==1)
	return base;
      vector<gen> factn(k+1);
      factn[0]=1;
      for (int i=1;i<=k;++i){
	factn[i]=i*factn[i-1];
      }
      // (x1+...+xn)^k -> sum_{j1+...+jn=k} k!/(j1!j2!...jn!) x^j1 *... *x^jk
      vecteur & w=*v[0]._SYMBptr->feuille._VECTptr;
      int n=int(w.size());
      if (!n)
	return gensizeerr(contextptr);
      if (std::pow(double(n),double(k))>MAX_PROD_EXPAND_SIZE)
	return pow(v[0],v[1],contextptr);
      vecteur res;
      gen p;
      for (int i=k;i>=0;--i){
	p=expand(pow(w[0],i),contextptr);
	pow_expand_add_res(factn,1,i,factn[k]/factn[i],w,p,k,n,res,contextptr);
      }
      return symbolic(at_plus,res);
    }
    if (v[0].is_symb_of_sommet(at_prod) && v[0]._SYMBptr->feuille.type==_VECT){
      const vecteur & vb=*v[0]._SYMBptr->feuille._VECTptr;
      gen r1(1),r2(1);
      for (int i=0;i<vb.size();++i){
	if (fastsign(vb[i],contextptr)==1)
	  r1=r1*pow(vb[i],exponent,contextptr);
	else
	  r2=r2*vb[i];
      }
      if (r1!=1)
	return r1*pow(r2,exponent,contextptr);
    }
    return symb_pow(base,exponent);
  }

  static gen expand_neg_expand(const gen & g_orig,GIAC_CONTEXT){
    gen g=expand(g_orig,contextptr);
    return -g;
  }

  vecteur tchebycheff(int n,bool first_kind){
    vecteur v0,v1,vtmp;
    if (first_kind) {
      v0.push_back(1);
      v1.push_back(1);
      v1.push_back(0);
    }
    else
      v1.push_back(1);
    if (!n)
      return v0;
    if (n==1)
      return v1;
    for (--n;n;--n){
      multvecteur(2,v1,vtmp);
      vtmp.push_back(0);
      vtmp=vtmp-v0;
      v0=v1;
      v1=vtmp;
    }
    return v1; 
  }

  gen exp_expand(const gen & e,GIAC_CONTEXT){
    if (e.type!=_SYMB)
      return exp(e,contextptr);
    if (e._SYMBptr->sommet==at_plus)
      return symbolic(at_prod,apply(e._SYMBptr->feuille,exp_expand,contextptr));
    gen coeff,arg;
    split(e,coeff,arg,contextptr);
    return pow(exp(arg,contextptr),coeff,contextptr);
  }

  static gen ln_expand0(const gen & e,GIAC_CONTEXT){
    if (e.type==_FRAC)
      return ln(e._FRACptr->num,contextptr)-ln(e._FRACptr->den,contextptr);
    if (e.type!=_SYMB)
      return ln(e,contextptr);
    if (e._SYMBptr->sommet==at_prod)
      return _plus(apply(e._SYMBptr->feuille,ln_expand0,contextptr),contextptr);//symbolic(at_plus,apply(e._SYMBptr->feuille,ln_expand0,contextptr));
    if (e._SYMBptr->sommet==at_inv)
      return -ln_expand0(e._SYMBptr->feuille,contextptr);
    if (e._SYMBptr->sommet==at_pow){
      gen & tmp=e._SYMBptr->feuille;
      if (tmp.type==_VECT && tmp._VECTptr->size()==2){
	gen base=tmp._VECTptr->front(),expo=tmp._VECTptr->back();
	if (!complex_mode(contextptr) && expo.type==_INT_ && expo.val%2==0)
	  base=abs(base,contextptr);
	return expo*ln_expand0(base,contextptr);
      }
    }
    return ln(e,contextptr);
  }

  gen ln_expand(const gen & e0,GIAC_CONTEXT){
    gen e(factor(e0,false,contextptr));
    return ln_expand0(e,contextptr);
  }

  gen symhorner(const vecteur & v, const gen & e){
    if (v.empty())
      return zero;
    if (is_zero(e))
      return v.back();
    gen res=zero;
    const_iterateur it=v.begin(),itend=v.end();
    int n=int(itend-it)-1;
    for (;it!=itend;++it,--n)
      res = res + (*it)*pow(e,n);
    return res;
  }

  static gen cos_expand(const gen & e,GIAC_CONTEXT);
  static gen sin_expand(const gen & e,GIAC_CONTEXT){
    if (e.type!=_SYMB)
      return sin(e,contextptr);
    if (lidnt(e)==vecteur(1,cst_pi)){
      gen sine=sin(e,contextptr);
      if (!contains(lidnt(sine),cst_pi))
	return sine;
    }
    if (e._SYMBptr->sommet==at_plus){
      vecteur v=*e._SYMBptr->feuille._VECTptr;
      gen last=v.back(),first;
      v.pop_back();
      if (v.size()==1)
	first=v.front();
      else
	first=symbolic(at_plus,v);
      return sin_expand(first,contextptr)*cos_expand(last,contextptr)+cos_expand(first,contextptr)*sin_expand(last,contextptr);
    }
    if (e._SYMBptr->sommet==at_neg)
      return -sin_expand(e._SYMBptr->feuille,contextptr);
    gen coeff,arg;
    split(e,coeff,arg,contextptr);
    if (!is_one(coeff) && coeff.type==_INT_ && coeff.val<max_texpand_expansion_order)
      return symhorner(tchebycheff(coeff.val,false),cos(arg,contextptr))*sin(arg,contextptr);
    else
      return sin(e,contextptr);
  }

  static gen cos_expand(const gen & e,GIAC_CONTEXT){
    if (e.type!=_SYMB)
      return cos(e,contextptr);
    if (lidnt(e)==vecteur(1,cst_pi)){
      gen cose=cos(e,contextptr);
      if (!contains(lidnt(cose),cst_pi))
	return cose;
    }
    if (e._SYMBptr->sommet==at_plus){
      vecteur v=*e._SYMBptr->feuille._VECTptr;
      gen last=v.back(),first;
      v.pop_back();
      if (v.size()==1)
	first=v.front();
      else
	first=symbolic(at_plus,v);
      return cos_expand(first,contextptr)*cos_expand(last,contextptr)-sin_expand(first,contextptr)*sin_expand(last,contextptr);
    }
    if (e._SYMBptr->sommet==at_neg)
      return cos_expand(e._SYMBptr->feuille,contextptr);
    gen coeff,arg;
    split(e,coeff,arg,contextptr);
    if (!is_one(coeff) && coeff.type==_INT_ && coeff.val<max_texpand_expansion_order)
      return symhorner(tchebycheff(coeff.val,true),cos(arg,contextptr));
    else
      return cos(e,contextptr);
  }

  static gen sin2tancos(const gen & g,GIAC_CONTEXT){
    return symb_cos(g)*symb_tan(g);
  }

  static gen even_pow_cos2tan(const gen & g,GIAC_CONTEXT){
    if ( (g.type!=_VECT) || (g._VECTptr->size()!=2) )
      return g;
    gen a(g._VECTptr->front()),b(g._VECTptr->back());
    if ( (b.type!=_INT_) || (b.val%2) || (a.type!=_SYMB) || (a._SYMBptr->sommet!=at_cos) )
      return symbolic(at_pow,g);
    int i=b.val/2;
    return pow(plus_one+pow(symb_tan(a._SYMBptr->feuille),plus_two,contextptr),-i,contextptr);
  }

  static gen tan_expand(const gen & e,GIAC_CONTEXT){
    if (e.type!=_SYMB)
      return tan(e,contextptr);
    if (lidnt(e)==vecteur(1,cst_pi)){
      gen tane=tan(e,contextptr);
      if (!contains(lidnt(tane),cst_pi))
	return tane;
    }
    if (e._SYMBptr->sommet==at_plus){
      vecteur v=*e._SYMBptr->feuille._VECTptr;
      gen last=v.back(),first;
      v.pop_back();
      if (v.size()==1)
	first=v.front();
      else
	first=symbolic(at_plus,v);
      gen ta=tan_expand(first,contextptr);
      gen tb=tan_expand(last,contextptr);
      if (is_inf(ta))
	return -inv(tb,contextptr);
      if (is_inf(tb))
	return -inv(ta,contextptr);
      return rdiv(ta+tb,1-ta*tb,contextptr);
    }
    if (e._SYMBptr->sommet==at_neg)
      return -tan_expand(e._SYMBptr->feuille,contextptr);
    gen coeff,arg;
    split(e,coeff,arg,contextptr);
    if (!is_one(coeff) && coeff.type==_INT_ && coeff.val<max_texpand_expansion_order){
      gen g=rdiv(symhorner(tchebycheff(coeff.val,false),cos(arg,contextptr))*sin(arg,contextptr),symhorner(tchebycheff(coeff.val,true),cos(arg,contextptr)),contextptr);
      vector<const unary_function_ptr *> v;
      vector< gen_op_context > w;
      v.push_back(at_sin);
      w.push_back(&sin2tancos);
      g=subst(g,v,w,false,contextptr);
      v[0]=at_pow;
      w[0]=(&even_pow_cos2tan);
      g=subst(recursive_normal(g,false,contextptr),v,w,false,contextptr);      
      return recursive_normal(g,false,contextptr);
    }
    else
      return tan(e,contextptr);
  }

  symbolic symb_texpand(const gen & a){
    return symbolic(at_texpand,a);
  }

  // "unary" version
  gen _texpand(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    gen var,res;
    if (is_algebraic_program(args,var,res))
      return symbolic(at_program,makesequence(var,0,_texpand(res,contextptr)));
    if (is_equal(args))
      return apply_to_equal(args,_texpand,contextptr);
    vector<const unary_function_ptr *> v;
    vector< gen_op_context > w;
    v.push_back(at_exp);
    w.push_back(&exp_expand);
    v.push_back(at_ln);
    w.push_back(&ln_expand);
    v.push_back(at_prod);
    w.push_back(&prod_expand);
    v.push_back(at_sin);
    w.push_back(&sin_expand);
    v.push_back(at_cos);
    w.push_back(&cos_expand);
    v.push_back(at_tan);
    w.push_back(&tan_expand);
    return subst(args,v,w,false,contextptr);
  }
  static const char _texpand_s []="texpand";
  static define_unary_function_eval (__texpand,&_texpand,_texpand_s);
  define_unary_function_ptr5( at_texpand ,alias_at_texpand,&__texpand,0,true);

  static const char _developper_transcendant_s []="developper_transcendant";
  static define_unary_function_eval (__developper_transcendant,&_texpand,_developper_transcendant_s);
  define_unary_function_ptr5( at_developper_transcendant ,alias_at_developper_transcendant,&__developper_transcendant,0,true);

  vecteur andor2list(const gen & g,GIAC_CONTEXT){
    if (g.type!=_SYMB)
      return vecteur(1,vecteur(1,g));
    if (g._SYMBptr->sommet==at_ou){
      vecteur args(gen2vecteur(g._SYMBptr->feuille));
      int n=int(args.size());
      vecteur res;
      for (int i=0;i<n;++i){
	vecteur v(andor2list(args[i],contextptr));
	int l=int(v.size());
	for (int j=0;j<l;++j)
	  res.push_back(v[j]);
      }
      return res;
    }
    if (g._SYMBptr->sommet==at_and){
      vecteur args(gen2vecteur(g._SYMBptr->feuille));
      int n=int(args.size());
      vecteur res;
      longlong N=1;
      for (int i=0;i<n;++i){
	vecteur v(andor2list(args[i],contextptr));
	N*=v.size(); // res.size() at end of iteration
	if (N>RAND_MAX)
	  return vecteur(1,vecteur(1,gendimerr(contextptr)));
	if (i==0){
	  res.swap(v); continue;
	}
	vecteur newres; newres.reserve(N);
	// "multiply" element by element between res and v
	for (size_t I=0;I<res.size();++I){
	  for (size_t J=0;J<v.size();++J){
	    newres.push_back(mergevecteur(*res[I]._VECTptr,*v[J]._VECTptr));
	  }
	}
	res.swap(newres);
      }
      return res;
    }
    return vecteur(1,vecteur(1,g));
  }

  gen list2and(const gen & g){
    if (g.type!=_VECT)
      return g;
    if (g._VECTptr->empty())
      return 1;
    if (g._VECTptr->size()==1)
      return g;
    gen G=g; G.subtype=_SEQ__VECT;
    return symbolic(at_and,G);
  }

  gen list2orand(const vecteur & v){
    if (v.empty())
      return 1;
    if (v.size()==1)
      return list2and(v.front());
    vecteur w(v);
    for (int i=0;i<w.size();++i){
      w[i]=list2and(w[i]);
    }
    return symbolic(at_ou,gen(w,_SEQ__VECT));
  }

  bool are_inequations(const gen & g){
    if (g.type!=_VECT)
      return is_inequation(g);
    const vecteur & v=*g._VECTptr;
    size_t N=v.size();
    for (size_t i=0;i<N;++i){
      if (!are_inequations(v[i]))
	return false;
    }
    return true;
  }

  gen ineq2diff(const gen & g){
    if (g.type!=_SYMB) return g;
    if (g._SYMBptr->sommet==at_superieur_strict || g._SYMBptr->sommet==at_superieur_egal){
      vecteur & v=*g._SYMBptr->feuille._VECTptr;
      return v[0]-v[1];
    }
    if (g._SYMBptr->sommet==at_inferieur_strict || g._SYMBptr->sommet==at_inferieur_egal){
      vecteur & v=*g._SYMBptr->feuille._VECTptr;
      return v[1]-v[0];
    }
    return g;
  }

  // returns true if v is a list of linear inequations, write them in a matrix
  // w=[...,[a,b,c],...] where a*x+b*y+c>=0 (> is replaced by >=) 
  bool and2mat(const vecteur & v,const gen &x,const gen &y,matrice &w,GIAC_CONTEXT){
    w.clear();
    for (size_t i=0;i<v.size();++i){
      gen g=v[i];
      if (!is_inequation(g))
	return false;
      g=ineq2diff(g);
      gen a,tmp,b,c;
      if (!is_linear_wrt(g,x,a,tmp,contextptr))
	return false;
      if (!is_linear_wrt(tmp,y,b,c,contextptr))
	return false;
      if (evalf_double(a,1,contextptr).type!=_DOUBLE_ || evalf_double(b,1,contextptr).type!=_DOUBLE_ || evalf_double(c,1,contextptr).type!=_DOUBLE_)
	return false;
      w.push_back(makevecteur(a,b,c));
    }
    return true;
  }

  // compute list of intersections points [x,y]
  // update x/ymin..max,1st call set xmin/ymin to 1e307 and xmax/ymax to -1e307 
  vecteur lin_ineq_inter(const matrice & m,double & xmin,double &xmax,double & ymin,double & ymax,GIAC_CONTEXT){
    size_t N=m.size();
    vecteur res; res.reserve((N*(N+1))/2);
    for (size_t i=0;i<N;++i){
      vecteur v1(gen2vecteur(m[i]));
      if (v1.size()!=3) return vecteur(1,gensizeerr(contextptr));
      gen a(v1[0]),b(v1[1]),c1(v1[2]);
      for (size_t j=i+1;j<N;++j){
	vecteur v2(gen2vecteur(m[j]));
	if (v2.size()!=3) return vecteur(1,gensizeerr(contextptr));
	gen c(v2[0]),d(v2[1]),c2(v2[2]);
	gen D=ratnormal(a*d-b*c,contextptr);
	if (is_zero(D)) continue; // parallel
	// solve([a*x+b*y+c1,c*x+d*y+c2],[x,y])
	gen x=(b*c2-c1*d)/D,y=(c*c1-a*c2)/D;
	// check that x,y verifies equations
	vecteur mxy=multmatvecteur(m,makevecteur(x,y,1));
	size_t pos=0;
	for (;pos<mxy.size();++pos){
	  if (!is_positive(mxy[pos],contextptr))
	    break;
	}
	if (pos<mxy.size())
	  continue;
	gen add(makevecteur(x,y));
	if (!equalposcomp(res,add))
	  res.push_back(add);
	x=evalf_double(x,1,contextptr);
	double xd=x._DOUBLE_val;
	if (xd>xmax)
	  xmax=xd;
	if (xd<xmin)
	  xmin=xd;
	y=evalf_double(y,1,contextptr);
	double yd=y._DOUBLE_val;
	if (yd>ymax)
	  ymax=yd;
	if (yd<ymin)
	  ymin=yd;
      }
    }
    return res;
  }

  // v should be a union of intersections, as returned by andor2list
  bool lin_ineq_plot(const vecteur & vsymb,const gen & x,const gen &y,const vecteur & attr_,vecteur & res,GIAC_CONTEXT){
    vecteur attr(attr_);
    if (!attr.empty() && attr[0].type==_INT_)
      attr[0] = attr[0].val | _FILL_POLYGON;
    double xmin=1e307,ymin=1e307,xmax=-1e307,ymax=-1e307;
    vecteur v(vsymb.size());
    for (size_t i=0;i<v.size();++i){
      matrice vi;
      if (!and2mat(gen2vecteur(vsymb[i]),x,y,vi,contextptr))
	return false;
      v[i]=vi;
    }
    matrice inter(vsymb.size()); // one list of intersection points for each element of v
    for (size_t i=0;i<v.size();++i){
      inter[i]=lin_ineq_inter(gen2vecteur(v[i]),xmin,xmax,ymin,ymax,contextptr);
    }
    if (xmin==1e307) xmin=gnuplot_xmin;
    if (ymin==1e307) ymin=gnuplot_ymin;
    if (xmax==-1e307) xmax=gnuplot_xmax;
    if (ymax==-1e307) ymax=gnuplot_ymax;
    double dx=gnuplot_xmax-gnuplot_xmin,dy=gnuplot_ymax-gnuplot_ymin;
    if (xmax>xmin)
      dx=xmax-xmin;
    if (dx<1e-300)
      dx=1e-300;
    if (ymax>ymin)
      dy=ymax-ymin;
    if (dy<1e-300)
      dy=1e-300;
    // axes zoomeout factor
    int z=1.5;
    xmin -= z*dx; xmax += z*dx;
    ymin -= z*dy; ymax += z*dy;
    res.push_back(symb_equal(change_subtype(_GL_X,_INT_PLOT),symb_interval(xmin,xmax)));
    res.push_back(symb_equal(change_subtype(_GL_Y,_INT_PLOT),symb_interval(ymin,ymax)));
    // zoomout factor
    z=6;
    xmin -= z*dx; xmax += z*dx;
    ymin -= z*dy; ymax += z*dy;
    // add border equations, find again intersections (lazy version, should be optimized)
    for (size_t i=0;i<v.size();++i){
      vecteur w(gen2vecteur(v[i]));
      w.push_back(makevecteur(1,0,-xmin)); // x-xmin>=0
      w.push_back(makevecteur(-1,0,xmax)); // -x+xmax>=0
      w.push_back(makevecteur(0,1,-ymin)); // y-ymin>=0
      w.push_back(makevecteur(0,-1,ymax)); // -y+ymax>=0
      inter[i]=lin_ineq_inter(w,xmin,xmax,ymin,ymax,contextptr);
    }
    // compute convexhull for each v[i]
    for (size_t i=0;i<inter.size();++i){
      vecteur cur(gen2vecteur(inter[i]));
      for (size_t j=0;j<cur.size();++j){
	cur[j]=cur[j][0]+cst_i*cur[j][1];
      }
      gen convhull=_convexhull(gen(cur,_SEQ__VECT),contextptr);
      vecteur argv=gen2vecteur(convhull);
      argv.push_back(argv.front());
      convhull=pnt_attrib(gen(argv,_GROUP__VECT),attr,contextptr);
      res.push_back(convhull);
    }
    return true;
  }

  gen expand(const gen & e,GIAC_CONTEXT){
    if (is_equal(e))
      return apply_to_equal(e,expand,contextptr);
    if (e.type==_SYMB && (e._SYMBptr->sommet==at_and || e._SYMBptr->sommet==at_ou)){
#if 0 // for testing plot
      return lin_ineq_plot(andor2list(e,contextptr),x__IDNT_e,y__IDNT_e,vecteur(1,56),contextptr); 
#else
      return list2orand(andor2list(e,contextptr));
#endif
    }
    gen var,res;
    if (e.type!=_VECT && is_algebraic_program(e,var,res))
      return symbolic(at_program,makesequence(var,0,expand(res,contextptr)));
    if (e.type==_VECT && e.subtype==_SEQ__VECT && e._VECTptr->size()==2){
      gen last=e._VECTptr->back();
      if (last.type==_STRNG || last.type==_FUNC){
	vector<const unary_function_ptr *> v;
	vector< gen_op_context > w;
	if (contains(last,gen(at_prod)) || (last.type==_STRNG && !strcmp(last._STRNGptr->c_str(),"*"))){ // expand * with no further simplification
	  v.push_back(at_prod);
	  w.push_back(prod_expand_nosimp);
	}
	if (contains(last,gen(at_ln))){
	  v.push_back(at_ln);
	  w.push_back(&ln_expand);
	}
	if (contains(last,gen(at_exp))){
	  v.push_back(at_exp);
	  w.push_back(&exp_expand);
	}
	if (contains(last,gen(at_sin))){ 
	  v.push_back(at_sin);
	  w.push_back(&sin_expand);
	}
	if (contains(last,gen(at_cos))){
	  v.push_back(at_cos);
	  w.push_back(&cos_expand);
	}
	if (contains(last,gen(at_tan))){
	  v.push_back(at_tan);
	  w.push_back(&tan_expand);
	}
	return subst(e._VECTptr->front(),v,w,false,contextptr);	
      }
    }
    vector<const unary_function_ptr *> v;
    vector< gen_op_context > w;
    v.push_back(at_prod);
    v.push_back(at_pow);
    v.push_back(at_neg);
    w.push_back(&prod_expand);
    w.push_back(&expand_pow_expand);
    w.push_back(&expand_neg_expand);
    return _simplifier(subst(e,v,w,false,contextptr),contextptr);
  }
  static const char _expand_s []="expand";
  symbolic symb_expand(const gen & args){
    return symbolic(at_expand,args);
  }
  static define_unary_function_eval (__expand,&expand,_expand_s);
  define_unary_function_ptr( at_expand ,alias_at_expand ,&__expand);

  gen expexpand(const gen & e,GIAC_CONTEXT){
    if (is_equal(e))
      return apply_to_equal(e,expexpand,contextptr);
    gen var,res;
    if (is_algebraic_program(e,var,res))
      return symbolic(at_program,makesequence(var,0,expexpand(res,contextptr)));
    vector<const unary_function_ptr *> v(1,at_exp);
    vector< gen_op_context > w(1,&exp_expand);
    return subst(e,v,w,false,contextptr);
  }
  static const char _expexpand_s []="expexpand";
  static define_unary_function_eval (__expexpand,&expexpand,_expexpand_s);
  define_unary_function_ptr5( at_expexpand ,alias_at_expexpand,&__expexpand,0,true);

  gen lnexpand(const gen & e,GIAC_CONTEXT){
    if (is_equal(e))
      return apply_to_equal(e,lnexpand,contextptr);
    gen var,res;
    if (is_algebraic_program(e,var,res))
      return symbolic(at_program,makesequence(var,0,lnexpand(res,contextptr)));
    vector<const unary_function_ptr *> v(1,at_ln);
    vector< gen_op_context > w(1,&ln_expand);
    return subst(e,v,w,false,contextptr);
  }
  static const char _lnexpand_s []="lnexpand";
  static define_unary_function_eval (__lnexpand,&lnexpand,_lnexpand_s);
  define_unary_function_ptr5( at_lnexpand ,alias_at_lnexpand,&__lnexpand,0,true);

  gen trigexpand(const gen & e,GIAC_CONTEXT){
    if (is_equal(e))
      return apply_to_equal(e,trigexpand,contextptr);
    gen var,res;
    if (is_algebraic_program(e,var,res))
      return symbolic(at_program,makesequence(var,0,trigexpand(res,contextptr)));
    vector<const unary_function_ptr *> v;
    vector< gen_op_context > w;
    v.push_back(at_sin);
    w.push_back(&sin_expand);
    v.push_back(at_cos);
    w.push_back(&cos_expand);
    v.push_back(at_tan);
    w.push_back(&tan_expand);
    v.push_back(at_prod);
    w.push_back(&prod_expand);    
    return subst(e,v,w,false,contextptr);
  }
  static const char _trigexpand_s []="trigexpand";
  static define_unary_function_eval (__trigexpand,&trigexpand,_trigexpand_s);
  define_unary_function_ptr5( at_trigexpand ,alias_at_trigexpand,&__trigexpand,0,true);

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
