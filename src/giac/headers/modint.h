// Handle 4 modular int at the same time,
// modint is a signed int (should be 31 bits)
// modint2 should be twice as long (63 bits signed)
// mod4int is 4 modint, mod4int2 is 4 modint2
// to be used inside template code
// and SIMD acceleration (https://github.com/vectorclass)
/*  Copyright (C) 2024 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#ifndef MODINT_H
#define MODINT_H
namespace giac {
  int smod(int r,int m);
}
typedef int modint;
typedef long long modint2;
inline long long extend(int n){ return n;}
inline int shrink(long long n){ return n;}
inline int shrink(int n){ return n;}

struct mod4int {
  int tab[4];
  inline mod4int operator = (int b){
    tab[0]=b;
    tab[1]=b;
    tab[2]=b;
    tab[3]=b;
    return *this;  
  }
};
inline bool operator <(const mod4int & a,int b){
  return a.tab[0]<b && a.tab[1]<b && a.tab[2]<b && a.tab[3]<b;
}
inline bool operator >(const mod4int & a,int b){
  return a.tab[0]>b && a.tab[1]>b && a.tab[2]>b && a.tab[3]>b;
}
inline bool operator ==(const mod4int & a,const mod4int & b){
  return a.tab[0]==b.tab[0] && a.tab[1]==b.tab[1] && a.tab[2]==b.tab[2] && a.tab[3]==b.tab[3];
}
inline bool operator !=(const mod4int & a,const mod4int & b){
  return !(a==b);
}

inline bool operator ==(const mod4int & a,int  b){
  return a.tab[0]==b && a.tab[1]==b && a.tab[2]==b && a.tab[3]==b;
}
inline bool operator !=(const mod4int & a,int  b){
  return !(a==b);
}

template<class modint_t> modint_t create(int n);
inline mod4int mkmod4int(int n){ mod4int res={n,n,n,n}; return res; }
template<> inline mod4int create(int n){ return mkmod4int(n);}
template<> inline modint create(int n){ return n;}
template<> inline modint2 create(int n){ return n;}

inline mod4int operator + (const mod4int & a,const mod4int & b){
  mod4int res={a.tab[0]+b.tab[0],a.tab[1]+b.tab[1],a.tab[2]+b.tab[2],a.tab[3]+b.tab[3]};
  return res;  
}
inline mod4int operator += (mod4int & a,const mod4int & b){
  a.tab[0]+=b.tab[0];
  a.tab[1]+=b.tab[1];
  a.tab[2]+=b.tab[2];
  a.tab[3]+=b.tab[3];
  return a;  
}
inline mod4int operator - (const mod4int & a,const mod4int & b){
  mod4int res={a.tab[0]-b.tab[0],a.tab[1]-b.tab[1],a.tab[2]-b.tab[2],a.tab[3]-b.tab[3]};
  return res;  
}
inline mod4int operator -= (mod4int & a,const mod4int & b){
  a.tab[0]-=b.tab[0];
  a.tab[1]-=b.tab[1];
  a.tab[2]-=b.tab[2];
  a.tab[3]-=b.tab[3];
  return a;  
}
inline mod4int operator - (const mod4int & b){
  mod4int res={-b.tab[0],-b.tab[1],-b.tab[2],-b.tab[3]};
  return res;  
}
inline mod4int operator * (const mod4int & a,const mod4int & b){
  mod4int res={a.tab[0]*b.tab[0],a.tab[1]*b.tab[1],a.tab[2]*b.tab[2],a.tab[3]*b.tab[3]};
  return res;  
}
inline mod4int operator *= (mod4int & a,const mod4int & b){
  a.tab[0]*=b.tab[0];
  a.tab[1]*=b.tab[1];
  a.tab[2]*=b.tab[2];
  a.tab[3]*=b.tab[3];
  return a;  
}
inline mod4int operator * (const mod4int & a,const modint & b){
  mod4int res={a.tab[0]*b,a.tab[1]*b,a.tab[2]*b,a.tab[3]*b};
  return res;  
}
inline mod4int operator * (const modint & b,const mod4int & a){
  mod4int res={a.tab[0]*b,a.tab[1]*b,a.tab[2]*b,a.tab[3]*b};
  return res;  
}
inline mod4int operator >> (const mod4int & a,unsigned b){
  mod4int res={a.tab[0]>>b,a.tab[1]>>b,a.tab[2]>>b,a.tab[3]>>b};
  return res;  
}
inline mod4int operator << (const mod4int & a,unsigned b){
  mod4int res={a.tab[0]<<b,a.tab[1]<<b,a.tab[2]<<b,a.tab[3]<<b};
  return res;  
}
inline mod4int operator & (const mod4int & a,int b){
  mod4int res={a.tab[0]&b,a.tab[1]&b,a.tab[2]&b,a.tab[3]&b};
  return res;  
}
inline mod4int operator & (const mod4int & a,const mod4int & b){
  mod4int res={a.tab[0]&b.tab[0],a.tab[1]&b.tab[1],a.tab[2]&b.tab[2],a.tab[3]&b.tab[3]};
  return res;  
}
inline mod4int operator / (const mod4int & a,const mod4int & b){
  mod4int res={a.tab[0]/b.tab[0],a.tab[1]/b.tab[1],a.tab[2]/b.tab[2],a.tab[3]/b.tab[3]};
  return res;  
}
inline mod4int operator % (const mod4int & a,const mod4int & b){
  mod4int res={a.tab[0]%b.tab[0],a.tab[1]%b.tab[1],a.tab[2]%b.tab[2],a.tab[3]%b.tab[3]};
  return res;  
}
inline mod4int operator / (const mod4int & a,const modint & b){
  mod4int res={a.tab[0]/b,a.tab[1]/b,a.tab[2]/b,a.tab[3]/b};
  return res;  
}
inline mod4int operator % (const mod4int & a,const modint & b){
  mod4int res={a.tab[0]%b,a.tab[1]%b,a.tab[2]%b,a.tab[3]%b};
  return res;  
}
inline mod4int smod(const mod4int & a,const mod4int & b){
  mod4int res={giac::smod(a.tab[0],b.tab[0]),giac::smod(a.tab[1],b.tab[1]),giac::smod(a.tab[2],b.tab[2]),giac::smod(a.tab[3],b.tab[3])};
  return res;  
}
inline mod4int shrink(const mod4int & a){ return a; }

inline bool is_zero(mod4int u){
  return u.tab[0]==0 && u.tab[1]==0 && u.tab[2]==0 && u.tab[3]==0;
}

inline bool is_one(mod4int u){
  return u.tab[0]==1 && u.tab[1]==1 && u.tab[2]==1 && u.tab[3]==1;
}

struct mod4int2 {
  long long tab[4];
};
inline mod4int2 mkmod4int2(int n){ mod4int2 res={n,n,n,n}; return res; }
template<> inline mod4int2 create(int n){ return mkmod4int2(n);}
inline mod4int2 extend(const mod4int & a){
  mod4int2 res={a.tab[0],a.tab[1],a.tab[2],a.tab[3]};
  return res;
}
inline mod4int shrink(const mod4int2 & a){
  mod4int res={shrink(a.tab[0]),shrink(a.tab[1]),shrink(a.tab[2]),shrink(a.tab[3])};
  return res;
}
inline bool is_zero(mod4int2 u){
  return u.tab[0]==0 && u.tab[1]==0 && u.tab[2]==0 && u.tab[3]==0;
}

inline mod4int2 operator + (const mod4int2 & a,const mod4int2 & b){
  mod4int2 res={a.tab[0]+b.tab[0],a.tab[1]+b.tab[1],a.tab[2]+b.tab[2],a.tab[3]+b.tab[3]};
  return res;  
}
inline mod4int2 operator + (const mod4int2 & a,const mod4int & b){
  return a+extend(b);
}
inline mod4int2 operator + (const mod4int & a,const mod4int2 & b){
  return extend(a)+b;
}
inline mod4int2 operator += (mod4int2 & a,const mod4int & b){
  a.tab[0]+=b.tab[0];
  a.tab[1]+=b.tab[1];
  a.tab[2]+=b.tab[2];
  a.tab[3]+=b.tab[3];
  return a;  
}
inline mod4int2 operator += (mod4int2 & a,const mod4int2 & b){
  a.tab[0]+=b.tab[0];
  a.tab[1]+=b.tab[1];
  a.tab[2]+=b.tab[2];
  a.tab[3]+=b.tab[3];
  return a;  
}
inline mod4int2 operator - (const mod4int2 & a,const mod4int2 & b){
  mod4int2 res={a.tab[0]-b.tab[0],a.tab[1]-b.tab[1],a.tab[2]-b.tab[2],a.tab[3]-b.tab[3]};
  return res;  
}
inline mod4int2 operator - (const mod4int2 & a,const mod4int & b){
  return a-extend(b);
}
inline mod4int2 operator - (const mod4int & a,const mod4int2 & b){
  return extend(a)-b;
}
inline mod4int2 operator - (const mod4int2 & b){
  mod4int2 res={-b.tab[0],-b.tab[1],-b.tab[2],-b.tab[3]};
  return res;  
}
inline mod4int2 operator -= (mod4int2 & a,const mod4int & b){
  a.tab[0]-=b.tab[0];
  a.tab[1]-=b.tab[1];
  a.tab[2]-=b.tab[2];
  a.tab[3]-=b.tab[3];
  return a;  
}
inline mod4int2 operator -= (mod4int2 & a,const mod4int2 & b){
  a.tab[0]-=b.tab[0];
  a.tab[1]-=b.tab[1];
  a.tab[2]-=b.tab[2];
  a.tab[3]-=b.tab[3];
  return a;  
}
inline mod4int2 operator * (const mod4int2 & a,const mod4int2 & b){
  mod4int2 res={a.tab[0]*b.tab[0],a.tab[1]*b.tab[1],a.tab[2]*b.tab[2],a.tab[3]*b.tab[3]};
  return res;  
}
inline mod4int2 operator * (const mod4int2 & a,const mod4int & b){
  return a*extend(b);
}
inline mod4int2 operator * (const mod4int & a,const mod4int2 & b){
  return extend(a)*b;
}
inline mod4int2 operator * (const modint2 & b,const mod4int2 & a){
  mod4int2 res={a.tab[0]*b,a.tab[1]*b,a.tab[2]*b,a.tab[3]*b};
  return res;  
}
inline mod4int2 operator * (const mod4int2 & a,const modint2 & b){
  mod4int2 res={a.tab[0]*b,a.tab[1]*b,a.tab[2]*b,a.tab[3]*b};
  return res;  
}
inline mod4int2 operator >> (const mod4int2 & a,unsigned b){
  mod4int2 res={a.tab[0]>>b,a.tab[1]>>b,a.tab[2]>>b,a.tab[3]>>b};
  return res;  
}
inline mod4int2 operator << (const mod4int2 & a,unsigned b){
  mod4int2 res={a.tab[0]<<b,a.tab[1]<<b,a.tab[2]<<b,a.tab[3]<<b};
  return res;  
}
inline mod4int2 operator & (const mod4int2 & a,modint2 b){
  mod4int2 res={a.tab[0]&b,a.tab[1]&b,a.tab[2]&b,a.tab[3]&b};
  return res;  
}
inline mod4int2 operator & (const mod4int2 & a,const mod4int2 & b){
  mod4int2 res={a.tab[0]&b.tab[0],a.tab[1]&b.tab[1],a.tab[2]&b.tab[2],a.tab[3]&b.tab[3]};
  return res;  
}
inline mod4int2 operator / (const mod4int2 & a,const mod4int2 & b){
  mod4int2 res={a.tab[0]/b.tab[0],a.tab[1]/b.tab[1],a.tab[2]/b.tab[2],a.tab[3]/b.tab[3]};
  return res;  
}
inline mod4int2 operator % (const mod4int2 & a,const mod4int2 & b){
  mod4int2 res={a.tab[0]%b.tab[0],a.tab[1]%b.tab[1],a.tab[2]%b.tab[2],a.tab[3]%b.tab[3]};
  return res;  
}
inline mod4int operator % (const mod4int2 & a,const mod4int & b){
  mod4int res={(modint)(a.tab[0]%b.tab[0]),(modint)(a.tab[1]%b.tab[1]),(modint)(a.tab[2]%b.tab[2]),(modint)(a.tab[3]%b.tab[3])};
  return res;  
}
inline mod4int operator % (long long a,const mod4int & b){
  mod4int res={(modint)(a%b.tab[0]),(modint)(a%b.tab[1]),(modint)(a%b.tab[2]),(modint)(a%b.tab[3])};
  return res;  
}
inline mod4int2 operator %= (mod4int2 & a,const mod4int & b){
  a.tab[0]%=b.tab[0];
  a.tab[1]%=b.tab[1];
  a.tab[2]%=b.tab[2];
  a.tab[3]%=b.tab[3];
  return a;  
}
inline mod4int2 operator / (const mod4int2 & a,const modint2 & b){
  mod4int2 res={a.tab[0]/b,a.tab[1]/b,a.tab[2]/b,a.tab[3]/b};
  return res;  
}
inline mod4int2 operator % (const mod4int2 & a,const modint2 & b){
  mod4int2 res={a.tab[0]%b,a.tab[1]%b,a.tab[2]%b,a.tab[3]%b};
  return res;  
}
inline mod4int2 operator / (const mod4int2 & a,const modint & b){
  mod4int2 res={a.tab[0]/b,a.tab[1]/b,a.tab[2]/b,a.tab[3]/b};
  return res;  
}
inline mod4int operator % (const mod4int2 & a,const modint & b){
  mod4int res={(modint)(a.tab[0]%b),(modint)(a.tab[1]%b),(modint)(a.tab[2]%b),(modint)(a.tab[3]%b)};
  return res;  
}

inline mod4int smod(const mod4int2 & a,const mod4int & b){
  mod4int res={giac::smod(a.tab[0],b.tab[0]),giac::smod(a.tab[1],b.tab[1]),giac::smod(a.tab[2],b.tab[2]),giac::smod(a.tab[3],b.tab[3])};
  return res;  
}


#endif // MODINT_H
