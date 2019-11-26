#include "config.h"
#include "giacPCH.h"
#ifdef KHICAS
#include "kdisplay.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

using namespace std;
using namespace giac;
using namespace xcas;

#if 0
int ext_main(){
  while (1){
    statuslinemsg("Numworks loader");
    drawRectangle(0,0,LCD_WITH_PX,LCD_HEIGHT_PX,_BLACK);
    os_draw_string(0,20,_WHITE,_BLACK,"1. Khicas shell");
    os_draw_string(0,40,_WHITE,_BLACK,"2. Epsilon (Numworks HOME)");
    int k=getkey(true);
    if (k=='1' ) run_epsilon();
    if (k=='2') caseval("*");
  }
}
#else
int ext_main(){
  caseval("*");
  return 0;
}
#endif

unsigned short mmind_col[]={COLOR_BLUE,COLOR_RED,COLOR_MAGENTA,COLOR_GREEN,COLOR_CYAN,COLOR_YELLOW};

void mastermind_disp(const vector<int> & solution,const vector< vector<int> > & essais,const vector<int> & essai,bool fulldisp,GIAC_CONTEXT){
  int x0=30,y0=30;
  if (fulldisp)
    drawRectangle(0,0,LCD_WIDTH_PX,LCD_HEIGHT_PX,_WHITE);
  else
    drawRectangle(0,y0+6*20,LCD_WIDTH_PX,LCD_HEIGHT_PX-(y0+4*20),_WHITE);
  if (fulldisp){
    // grille
    for (int i=y0;i<=y0+4*20;i+=20)
      draw_line(x0,i,x0+12*20,i,_BLACK);
    for (int j=x0;j<=x0+12*20;j+=20)
      draw_line(j,y0,j,y0+4*20,_BLACK);
    // affichage des coups precedents et resultats
    for (int c=0;c<essais.size();++c){
      const vector<int> & essai=essais[c];
      for (int i=0;i<4;++i){
	draw_filled_circle(x0+20*c+10,y0+20*i+10,10,mmind_col[essai[i]],true,true,contextptr);
      }
      // resultats
      vector<int> S(solution),E(essai);
      // bien places
      int bien=0;
      for (int i=0;i<S.size();++i){
	if (S[i]==E[i]){
	  ++bien;
	  S.erase(S.begin()+i);
	  E.erase(E.begin()+i);
	  --i;
	}
      }
      // mal places
      int mal=0;
      sort(S.begin(),S.end());
      sort(E.begin(),E.end());
      int s=0,e=0;
      for (;;){
	if (s>=S.size() || e>=E.size())
	  break;
	if (S[s]==E[e]){
	  ++mal;
	  ++s; ++e;
	  continue;
	}
	if (S[s]<E[e])
	  ++s;
	else
	  ++e;
      }
      char buf[2]={0,0};
      buf[0]='0'+bien;
      os_draw_string(x0+20*c+3,y0+20*4+2,COLOR_GREEN,_WHITE,buf);
      buf[0]='0'+mal;
      os_draw_string(x0+20*c+3,y0+20*5+2,COLOR_MAGENTA,_WHITE,buf);
      //CERR << solution << " " << essai << " " << bien << " " << mal << endl;
    }
  }
  int y=170;
  int x=os_draw_string_small_(x0,y,"0");
  draw_filled_circle(x+10,y+10,10,COLOR_BLUE);
  x=os_draw_string_small_(x+30,y,"1");
  draw_filled_circle(x+10,y+10,10,COLOR_RED);
  x=os_draw_string_small_(x+30,y,"2");
  draw_filled_circle(x+10,y+10,10,COLOR_MAGENTA);
  x=os_draw_string_small_(x+30,y,"3");
  draw_filled_circle(x+10,y+10,10,COLOR_GREEN);
  x=os_draw_string_small_(x+30,y,"4");
  draw_filled_circle(x+10,y+10,10,COLOR_CYAN);
  x=os_draw_string_small_(x+30,y,"5");
  draw_filled_circle(x+10,y+10,10,COLOR_YELLOW);
  y += 20;
  // affichage du coup actuel
  for (int i=0;i<essai.size();++i)
    draw_filled_circle(x0+20*i+10,y+10,10,mmind_col[essai[i]],true,true,contextptr);
}    
  
int mastermind(GIAC_CONTEXT){
  // Mastermind
  vector<int> solution(4),essai;
  vector< vector<int> > essais;
  const int nbcouleurs=6;
  const int nbessais=12;
  for (int i=0;i<4;++i)
    solution[i]=giac_rand(contextptr) % nbcouleurs;
  int i=0,j=0;
  bool fulldisp=true;
  for (;;){
    mastermind_disp(solution,essais,essai,fulldisp,contextptr);
    // saisie du prochain coup
    int key=getkey(true);
    fulldisp=false;
    if (key==KEY_CTRL_MENU)
      return key;
    if (key==KEY_PRGM_ACON){
      fulldisp=true;
      continue;
    }
    if (key>='0' && key<='5'){
      if (essai.size()==4)
	continue;
      essai.push_back(key-'0');
    }
    if (key==KEY_CTRL_EXE || key==KEY_CTRL_OK){
      if (essai.size()==4){
	if (essai==solution){
	  char buf[16]; sprint_int(buf,essais.size());
	  confirm("Vous avez trouve. Essais:",buf);
	  return i;
	}
	fulldisp=true;
	essais.push_back(essai);
	essai.clear();
	if (essais.size()==nbessais){
	  mastermind_disp(solution,essais,essai,true,contextptr);
	  for (int i=0;i<solution.size();++i)
	    draw_filled_circle(30+20*i+10,190+20,10,mmind_col[solution[i]],true,true,contextptr);
	  confirm("Vous avez perdu.","La solution etait",false,140);
	  return -1;
	}
      }
    }
    if (key==KEY_CTRL_DEL){
      if (!essai.empty())
	essai.pop_back();
      continue;
    }
  }
  return 0;
}

int fractale(GIAC_CONTEXT){
  freeze=true;
  int X=320,Y=222,Nmax=10;
  double w=2.7/X;
  double h=-1.87/Y;
  for (int y=0;y<=Y/2;++y){
    complex<double> c(-2.1,h*y+0.935);
    for (int x=0;x<X;++x){
      complex<double> z(0);
      int j;
      for (j=0;j<Nmax;++j){
	z=z*z+c;
	if (abs(z)>2)
	  break;
      }
      int color=126*j+2079;
      os_set_pixel(x,y,color);
      os_set_pixel(x,(Y-y),color);
      c = c+w;
    }
  }
  statuslinemsg("Ecran fige. Taper EXIT");
  getkey(true);
  return 0;
}


int khicas_addins_menu(GIAC_CONTEXT){
  Menu smallmenu;
  smallmenu.numitems=6; // INCREMENT IF YOU ADD AN APPLICATION
  // and uncomment first smallmenuitems[app_number].text="Reserved"
  // replace by your application name
  // and add if (smallmenu.selection==app_number-1){ call your code }
  MenuItem smallmenuitems[smallmenu.numitems];      
  smallmenu.items=smallmenuitems;
  smallmenu.height=12;
  smallmenu.scrollbar=1;
  smallmenu.scrollout=1;
  smallmenuitems[0].text = (char*)"Table periodique";
  smallmenuitems[1].text = (char*)"Exemple simple: Syracuse";
  smallmenuitems[2].text = (char*)"Exemple de jeu: Mastermind";
  smallmenuitems[3].text = (char*)"Fractale de Mandelbrot";
  // smallmenuitems[4].text = (char*)"Reserverd";
  // smallmenuitems[5].text = (char*)"Reserverd";
  // smallmenuitems[6].text = (char*)"Reserverd";
  // smallmenuitems[7].text = (char*)"Reserverd";
  // smallmenuitems[8].text = (char*)"Reserverd";
  // smallmenuitems[9].text = (char*)"Reserverd";
  // smallmenuitems[10].text = (char*)"Reserverd";
  smallmenuitems[smallmenu.numitems-2].text = (char*)"Quitter le menu";
  smallmenuitems[smallmenu.numitems-1].text = (char*)"Quitter Khicas";
  while(1) {
    int sres = doMenu(&smallmenu);
    if(sres == MENU_RETURN_SELECTION || sres==KEY_CTRL_EXE) {
      if (smallmenu.selection==smallmenu.numitems){
	return KEY_CTRL_MENU;
      }
      if (smallmenu.selection==3)
	mastermind(contextptr);
      if (smallmenu.selection==4)
	fractale(contextptr);
      if (smallmenu.selection==2){
	// Exemple simple d'application tierce: la suite de Syracuse
	// on entre la valeur de u0
	double d; int i;
	for (;;){
	  inputdouble(gettext("Suite de Syracuse. u0?"),d,contextptr);
	  i=(d);
	  if (i==d)
	    break;
	  confirm(gettext("u0 doit etre entier!"),gettext("Recommencez"));
	}
	i=max(i,1);
	vecteur v(1,i); // initialise une liste avec u0
	while (i!=1){
	  if (i%2)
	    i=3*i+1;
	  else
	    i=i/2;
	  v.push_back(i);
	}
	// representation graphique de la liste
	displaygraph(_listplot(v,contextptr),contextptr);
	// on entre la liste en ligne de commande
	Console_Input(gen(v).print(contextptr).c_str());
      }
      if (smallmenu.selection==1){
	const char * name,*symbol;
	char protons[32],nucleons[32],mass[32],electroneg[32];
	int res=periodic_table(name,symbol,protons,nucleons,mass,electroneg);
	if (!res)
	  continue;
	char console_buf[64]={0};
	char * ptr=console_buf;
	if (res & 1)
	  ptr=strcpy(ptr,name)+strlen(ptr);
	if (res & 2){
	  if (res & 1)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,symbol)+strlen(ptr);
	}
	if (res & 4){
	  if (res&3)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,protons)+strlen(ptr);
	}
	if (res & 8){
	  if (res&7)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,nucleons)+strlen(ptr);
	}
	if (res & 16){
	  if (res&15)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,mass+2)+strlen(ptr);
	  ptr=strcpy(ptr,"_(g/mol)")+8;
	}
	if (res & 32){
	  if (res&31)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,electroneg+4)+strlen(ptr);
	}
	return Console_Input(console_buf);
      }
    } // end sres==menu_selection
    Console_Disp();
    break;
  } // end endless while
  return CONSOLE_SUCCEEDED;
}  

#endif
