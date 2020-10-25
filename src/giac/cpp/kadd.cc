#include "config.h"
#include "giacPCH.h"
#ifdef KHICAS
#include "kdisplay.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC
  xcas::tableur * new_tableur(GIAC_CONTEXT){
    xcas::tableur * sheetptr=new xcas::tableur;
#ifdef NUMWORKS
    sheetptr->nrows=14; sheetptr->ncols=4;
#else
    sheetptr->nrows=20; sheetptr->ncols=5;
#endif
    gen g=vecteur(sheetptr->ncols);
    sheetptr->m=makefreematrice(vecteur(sheetptr->nrows,g));
    makespreadsheetmatrice(sheetptr->m,contextptr);
    sheetptr->cur_row=sheetptr->cur_col=sheetptr->disp_row_begin=sheetptr->disp_col_begin=0;
    sheetptr->sel_row_begin=sheetptr->sel_col_begin=-1;
    sheetptr->cmd_pos=sheetptr->cmd_row=sheetptr->cmd_col=-1;
    sheetptr->changed=false;
    sheetptr->recompute=true;
    sheetptr->matrix_fill_cells=true;
    sheetptr->movedown=true;
    sheetptr->filename="session";
    return sheetptr;
  }
  gen current_sheet(const gen & g,GIAC_CONTEXT){
    if (!xcas::sheetptr)
      xcas::sheetptr=new_tableur(contextptr);
    xcas::tableur & t=*xcas::sheetptr;
    if (ckmatrix(g,true)){
      t.m=*g._VECTptr;
      makespreadsheetmatrice(t.m,contextptr);
      t.cur_row=t.cur_col=0;
      t.nrows=t.m.size();
      t.ncols=t.m.front()._VECTptr->size();
      t.sel_row_begin=-1;
      t.cmd_row=t.cmd_pos=-1;
      return 1;
    }
    int r,c;
    if (iscell(g,c,r,contextptr)){
      if (r>=t.nrows||c>=t.ncols)
	return undef;
      gen tmp=t.m[r];
      tmp=tmp[c];
      return tmp[1];
    }
    if (g.type==_VECT && g.subtype==0 && g._VECTptr->empty())
      return gen(extractmatricefromsheet(t.m,false),_SPREAD__VECT);
    gen m(extractmatricefromsheet(t.m),_MATRIX__VECT);
    if (g.type==_VECT && g._VECTptr->empty())
      return m;
    return m[g];
  }
  static const char _current_sheet_s []="current_sheet";
  static define_unary_function_eval(__current_sheet,&current_sheet,_current_sheet_s);
  define_unary_function_ptr5( at_current_sheet ,alias_at_current_sheet,&__current_sheet,_QUOTE_ARGUMENTS,true);
  
#ifndef NO_NAMESPACE_GIAC
}
#endif // ndef NO_NAMESPACE_GIAC


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
    int k=getkey(1);
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
    int key=getkey(1);
    if (key==KEY_SHUTDOWN)
      return key;
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
  double d=10;
  if (inputdouble(lang?"Number of iterations? (default 10)":"Nombre d'iterations? (defaut 10)",d,contextptr) && d>=1 && d<=20)
    Nmax=d;
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
    sync_screen();
  }
  statuslinemsg("Ecran fige. Taper EXIT");
  getkey(1);
  return 0;
}


int khicas_addins_menu(GIAC_CONTEXT){
  Menu smallmenu;
  smallmenu.numitems=7; // INCREMENT IF YOU ADD AN APPLICATION
  // and uncomment first smallmenuitems[app_number].text="Reserved"
  // replace by your application name
  // and add if (smallmenu.selection==app_number-1){ call your code }
  MenuItem smallmenuitems[smallmenu.numitems];      
  smallmenu.items=smallmenuitems;
  smallmenu.height=12;
  smallmenu.scrollbar=1;
  smallmenu.scrollout=1;
  smallmenuitems[0].text = (char*)(lang?"Tableur":"Spreadsheet");
  smallmenuitems[1].text = (char*)(lang?"Table periodique":"Periodic table");
  smallmenuitems[2].text = (char*)(lang?"Exemple simple: Syracuse":"Simple example; Syracuse");
  smallmenuitems[3].text = (char*)(lang?"Exemple de jeu: Mastermind":"Game example: Mastermind");
  smallmenuitems[4].text = (char*)(lang?"Fractale de Mandelbrot":"Mandelbrot fractal");
  // smallmenuitems[4].text = (char*)"Reserverd";
  // smallmenuitems[5].text = (char*)"Reserverd";
  // smallmenuitems[6].text = (char*)"Reserverd";
  // smallmenuitems[7].text = (char*)"Reserverd";
  // smallmenuitems[8].text = (char*)"Reserverd";
  // smallmenuitems[9].text = (char*)"Reserverd";
  // smallmenuitems[10].text = (char*)"Reserverd";
  smallmenuitems[smallmenu.numitems-2].text = (char*)(lang?"Quitter le menu":"Leave menu");
  smallmenuitems[smallmenu.numitems-1].text = (char*)(lang?"Quitter KhiCAS":"Leave KhiCAS");
  while(1) {
    int sres = doMenu(&smallmenu);
    if(sres == MENU_RETURN_SELECTION || sres==KEY_CTRL_EXE) {
      if (smallmenu.selection==smallmenu.numitems){
	return KEY_CTRL_MENU;
      }
      if (smallmenu.selection==1)
	sheet(contextptr);
      if (smallmenu.selection==4)
	mastermind(contextptr);
      if (smallmenu.selection==5)
	fractale(contextptr);
      if (smallmenu.selection==3){
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
      if (smallmenu.selection==2){
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
    Console_Disp(1,contextptr);
    break;
  } // end endless while
  return CONSOLE_SUCCEEDED;
}  

/* **************************
   * SPREADSHEET CODE       *
   ************************** */
const int row_height=20;
const int col_width=60;
string printcell(int i,int j){
  string s="";
  s+=('A'+j);
  s+=print_INT_(i);
  return s;
}
string printsel(int r,int c,int R,int C){
  return printcell(r,c)+":"+printcell(R,C);
}

void change_undo(tableur & t){
  t.undo=t.m;
  t.changed=true;
}

void save_sheet(tableur & t,GIAC_CONTEXT){
  string s=gen(extractmatricefromsheet(t.m,false),_SPREAD__VECT).print(contextptr);
  string filename(remove_path(remove_extension(t.filename)));
  filename+=".tab";
#ifdef NSPIRE_NEWLIB
  filename+=".tns";
#endif
  write_file(filename.c_str(),s.c_str(),s.size());
}
void sheet_status(tableur & t,GIAC_CONTEXT){
  string st;
  if (python_compat(contextptr))
    st="tabl Py ";
  else
    st="tabl Xcas ";
  if (t.var.type==_IDNT)
    st += t.var.print(contextptr);
  else
    st += "<>";
  st += ' ';
  st += t.filename ;
  st += " R";
  st += print_INT_(t.nrows);
  st += " C";
  st += print_INT_(t.ncols);
  if (t.changed)
    st += " *";
  else
    st += " -";
  if (t.sel_row_begin>=0)
    st += (lang==1)?" esc: annule selection":" esc: cancel selection";
  else {
    if (t.cmd_row>=0)
      st += (lang==1)?" esc: annule ligne cmd":" esc: cancel cmdline";
  }
  statuslinemsg(st.c_str());
}
bool sheet_display(tableur &t,GIAC_CONTEXT){
  int disp_rows=LCD_HEIGHT_PX/row_height-3;
  int disp_cols=LCD_WIDTH_PX/(col_width+4)-1;
  if (t.disp_row_begin>t.cur_row)
    t.disp_row_begin=t.cur_row;
  if (t.disp_row_begin<t.cur_row-disp_rows+1)
    t.disp_row_begin=t.cur_row-disp_rows+1;
  if (t.disp_col_begin>t.cur_col)
    t.disp_col_begin=t.cur_col;
  if (t.disp_col_begin<t.cur_col-disp_cols+1)
    t.disp_col_begin=t.cur_col-disp_cols+1;
  int I=giacmin(giacmin(t.nrows,t.m.size()),t.disp_row_begin+disp_rows);
  bool has_sel=t.sel_row_begin>=0 && t.sel_row_begin<t.nrows;
  int sel_r=t.sel_row_begin,sel_R=t.cur_row,sel_c=t.sel_col_begin,sel_C=t.cur_col;
  if (sel_r>sel_R)
    swapint(sel_r,sel_R);
  if (sel_c>sel_C)
    swapint(sel_c,sel_C);
  bool has_cmd=t.cmd_row>=0 && t.cmd_row<t.nrows;
  waitforvblank();
  drawRectangle(0,0,LCD_WIDTH_PX,row_height,_WHITE); // clear column indices row
  string s;
  if (has_sel)
    s=printsel(sel_r,sel_c,sel_R,sel_C);
  else
    s=printcell(t.cur_row,t.cur_col);
  os_draw_string(2,1,_BLACK,_WHITE,s.c_str(),false);
  int y=row_height;
  int x=col_width;
  int J=giacmin(t.ncols,t.disp_col_begin+disp_cols);
  for (int j=t.disp_col_begin;j<J;++j){
    draw_line(x,0,x,row_height,_BLACK);
    char colname[3]="A"; 
    if (j>=26){ // if we accept more than 26 cols
      colname[0] += j/26;
      colname[1] = 'A'+(j%26);
      colname[2]=0;
    }
    else
      colname[0] += (j % 26);
    os_draw_string(x+col_width/2-4,2,_BLACK,_WHITE,colname);
    x+=col_width+4;
  }
  int waitn=2;
  for (int i=t.disp_row_begin;i<I;++i){
    if ( (i-t.disp_row_begin) % waitn==waitn-1)
      waitforvblank();
    drawRectangle(0,y,LCD_WIDTH_PX,row_height,_WHITE); // clear current row
    // draw_line(0,y,LCD_WIDTH_PX,y,_BLACK);
    os_draw_string(4,y,_BLACK,_WHITE,print_INT_(i).c_str()); // row number
    gen g=t.m[i];
    if (g.type!=_VECT)
      return false;
    vecteur & v=*g._VECTptr;
    int J=giacmin(t.ncols,v.size());
    J=giacmin(J,t.disp_col_begin+disp_cols);
    x=col_width;
    for (int j=t.disp_col_begin;j<J;++j){
      draw_line(x,y,x,y+row_height,_BLACK);
      gen vj=v[j];
      if (vj.type<_IDNT)
	vj=makevecteur(vj,vj,0);
      if (vj.type==_VECT && vj._VECTptr->size()==3){
	bool iscur=i==t.cur_row && j==t.cur_col;
	string s;
	if (iscur){
	  if (!has_cmd)
	    t.cmdline=(*vj._VECTptr)[0].print(contextptr);
	}
	bool rev=has_sel?(sel_r<=i && i<=sel_R && sel_c<=j && j<=sel_C):iscur;
	if (rev)
	  drawRectangle(x,y,col_width+4,row_height,_BLACK);	  
	s=(*vj._VECTptr)[1].print(contextptr);
	int dx=os_draw_string(0,0,0,0,s.c_str(),true); // find width
	if (dx<col_width)
	  os_draw_string(x+2,y,rev?_WHITE:_BLACK,rev?_BLACK:_WHITE,s.c_str(),false); // draw
	else {
	  if (iscur && !has_sel && t.cmd_row<0)
	    statuslinemsg(s.c_str());
	  s=s.substr(0,8)+"...";
	  os_draw_string_small(x+2,y,rev?_WHITE:_BLACK,rev?_BLACK:_WHITE,s.c_str(),false); // draw
	}
      }
      x+=col_width+4;
    }
    draw_line(0,y,LCD_WIDTH_PX,y,_BLACK);
    y+=row_height;
  }
  waitforvblank();
  drawRectangle(0,y,LCD_WIDTH_PX,LCD_HEIGHT_PX-y,_WHITE); // clear cmdline
  draw_line(0,y,LCD_WIDTH_PX,y,_BLACK);
  // commandline
  int p=python_compat(contextptr); python_compat(0,contextptr);
  int xpe=xcas_python_eval; xcas_python_eval=0;
  s=t.cmdline;
  int dx=os_draw_string(0,0,0,0,s.c_str(),true),xend=2; // find width
  bool small=t.keytooltip || dx>=LCD_WIDTH_PX-50;
  int sheety=LCD_HEIGHT_PX-2*row_height,xtooltip=0;
  if (t.cmd_row>=0 && t.cmd_pos>=0 && t.cmd_pos<=s.size()){
    xend=os_draw_string(xend,sheety,_BLUE,_WHITE,printcell(t.cmd_row,t.cmd_col).c_str())+5;
    string s1=s.substr(0,t.cmd_pos);
#if 1
    xtooltip=xend=print_color(xend,sheety,s1.c_str(),_BLACK,false,small,contextptr);
#else
    if (small)
      xend=os_draw_string_small(xend,sheety,_BLACK,_WHITE,s1.c_str(),false);
    else
      xend=os_draw_string(xend,sheety,_BLACK,_WHITE,s1.c_str(),false);
#endif
    drawRectangle(xend+1,sheety+2,2,small?10:13,_BLACK);
    xend+=4;
    s=s.substr(t.cmd_pos,s.size()-t.cmd_pos);
    if (has_sel){
      s1=printsel(sel_r,sel_c,sel_R,sel_C);
      xend=os_draw_string_small(xend,sheety,_WHITE,_BLACK,s1.c_str(),false);
    }
    else {
      if (t.cmd_row!=t.cur_row || t.cmd_col!=t.cur_col)
	xend=os_draw_string_small(xend,sheety,_WHITE,_BLACK,printcell(t.cur_row,t.cur_col).c_str(),false);
    }
  } // end cmdline active
  else
    xend=os_draw_string(xend,sheety,_BLACK,_WHITE,printcell(t.cur_row,t.cur_col).c_str())+5;    
  int bg=t.cmd_row>=0?_WHITE:57051;
#if 1
    xend=print_color(xend,sheety,s.c_str(),_BLACK,false,small,contextptr);
#else
  if (small)
    xend=os_draw_string_small(xend,sheety,_BLACK,bg,s.c_str(),false);
  else
    xend=os_draw_string(xend,sheety,_BLACK,bg,s.c_str(),false);
#endif
  if (t.keytooltip)
    t.keytooltip=tooltip(xtooltip,sheety,t.cmd_pos,t.cmdline.c_str(),contextptr);
  python_compat(p,contextptr); xcas_python_eval=xpe;
  // fast menus
  string menu("shift-1 stat1d|2 2d|3 seq|4 edit|5 view|6 graph|7 R|8 list| ");
  bg=65039;// bg=52832;
  drawRectangle(0,205,LCD_WIDTH_PX,17,bg);
  os_draw_string_small(0,205,_BLACK,bg,menu.c_str());
  return true;
}

void activate_cmdline(tableur & t){
  if (t.cmd_row==-1){
    t.cmd_row=t.cur_row;
    t.cmd_col=t.cur_col;
    t.cmd_pos=t.cmdline.size();
  }
}

bool sheet_eval(tableur & t,GIAC_CONTEXT,bool ckrecompute=true){
  t.changed=true;
  if (!ckrecompute || t.recompute)
    spread_eval(t.m,contextptr);
  return true;
}

void copy_right(tableur & t,GIAC_CONTEXT){
  int R=t.cur_row,C=t.cur_col,c=t.ncols;
  vecteur v=*t.m[R]._VECTptr;
  gen g=v[C];
  for (int i=C+1;i<c;++i){
    v[i]=freecopy(g);
  }
  t.m[R]=v;
  sheet_eval(t,contextptr,true);
}

void copy_down(tableur & t,GIAC_CONTEXT){
  int R=t.cur_row,C=t.cur_col,r=giacmin(t.nrows,t.m.size());
  gen g=t.m[R][C];
  for (int i=R+1;i<r;++i){
    vecteur v=*t.m[i]._VECTptr;
    v[C]=freecopy(g);
    t.m[i]=v;
  }
  sheet_eval(t,contextptr,true);
}

void paste(tableur & t,const matrice & m,GIAC_CONTEXT){
  int r=t.cur_row,c=t.cur_col,R=t.nrows,C=t.ncols;
  int dr=t.clip.size(),dc=0;
  if (r+dr>R)
    dr=R-r;
  if (dr && ckmatrix(m,true)){
    dc=m.front()._VECTptr->size();
    if (c+dc>C)
      dc=C-c;
    if (dc){
      for (int i=0;i<dr;++i){
	const vecteur & w=*m[i]._VECTptr;
	vecteur v=*t.m[r+i]._VECTptr;
	for (int j=0;j<dc;++j)
	  v[c+j]=w[j];
	t.m[r+i]=v;
      }
    }
  }
  sheet_eval(t,contextptr,true);
}

void paste(tableur & t,GIAC_CONTEXT){
  paste(t,t.clip,contextptr);
}

void sheet_pntv(const vecteur & v,vecteur & res);
void sheet_pnt(const gen & g,vecteur & res){
  if (g.type==_VECT)
    sheet_pntv(*g._VECTptr,res);
  if (g.is_symb_of_sommet(at_pnt))
    res.push_back(g);
}

void sheet_pntv(const vecteur & v,vecteur & res){
  for (int i=0;i<v.size();++i){
    sheet_pnt(v[i],res);
  }
}

void resizesheet(tableur &t){
  int cur_r=t.m.size(),cur_c=t.m.front()._VECTptr->size(),nr=t.nrows,nc=t.ncols;
  if (nr!=cur_r || nc!=cur_c){
    if (do_confirm(((lang==1?"Redimensionner ":"Resize ")+print_INT_(cur_r)+"x"+print_INT_(cur_c)+"->"+print_INT_(nr)+"x"+print_INT_(nc)).c_str())){
      vecteur fill(3,0);
      if (nr<cur_r) // erase rows
	t.m.resize(nr);
      else {
	for (;cur_r<nr;++cur_r){
	  vecteur tmp;
	  for (int j=0;j<nc;++j)
	    tmp.push_back(freecopy(fill));
	  t.m.push_back(tmp);
	}
      }
      for (int i=0;i<nr;++i){
	vecteur & v=*t.m[i]._VECTptr;
	int cur_c=v.size();
	if (nc<cur_c){
	  t.m[i]=vecteur(v.begin(),v.begin()+nc);
	}
	else {
	  for (;cur_c<nc;++cur_c)
	    v.push_back(freecopy(fill));
	}
      }
      t.cur_row=giacmin(t.cur_row,t.nrows);
      t.cur_col=giacmin(t.cur_col,t.ncols);
      t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
    } // end confirmed table resize
    else {
      t.nrows=cur_r;
      t.ncols=cur_c;
    }
  }  
}

void sheet_menu_setup(tableur & t,GIAC_CONTEXT){
  Menu smallmenu;
  smallmenu.numitems=7;
  MenuItem smallmenuitems[smallmenu.numitems];
  smallmenu.items=smallmenuitems;
  smallmenu.height=12;
  smallmenu.scrollbar=1;
  smallmenu.scrollout=1;
  smallmenu.title = (char*)(lang==1?"Configuration tableur":"Sheet config");
  smallmenuitems[3].type = MENUITEM_CHECKBOX;
  smallmenuitems[3].text = (char*)"Reeval";
  smallmenuitems[4].type = MENUITEM_CHECKBOX;
  smallmenuitems[4].text = (char*)(lang==1?"Matrice: remplir cellules":"Matrix: fill cells");
  smallmenuitems[5].type = MENUITEM_CHECKBOX;
  smallmenuitems[5].text = (char*)(lang==1?"Deplacement vers le bas":"Move down");
  smallmenuitems[smallmenu.numitems-1].text = (char*) "Quit";
  while(1) {
    string dig("Digits (in Xcas): ");
    dig += print_INT_(decimal_digits(contextptr));
    smallmenuitems[0].text = (char*)dig.c_str();
    string nrows((lang==1?"Lignes ":"Rows ")+print_INT_(t.nrows));
    smallmenuitems[1].text = (char*)nrows.c_str();
    string ncols((lang==1?"Colonnes ":"Cols ")+print_INT_(t.ncols));
    smallmenuitems[2].text = (char*)ncols.c_str();
    smallmenuitems[3].value = t.recompute;
    smallmenuitems[4].value = t.matrix_fill_cells;
    smallmenuitems[5].value = t.movedown;
    int sres = doMenu(&smallmenu);
    if (sres==MENU_RETURN_EXIT){
      resizesheet(t);
      break;
    }
    if (sres == MENU_RETURN_SELECTION  || sres==KEY_CTRL_EXE) {
      if (smallmenu.selection == 1){
	double d=decimal_digits(contextptr);
	if (inputdouble("Nombre de digits?",d,contextptr) && d==int(d) && d>0){
	  decimal_digits(d,contextptr);
	}
	continue;
      }
      if (smallmenu.selection == 2){
	double d=t.nrows;
	if (inputdouble((lang==1?"Nombre de lignes?":"Rows?"),d,contextptr) && d==int(d) && d>0){
	  t.nrows=d;
	}
	continue;
      }
      if (smallmenu.selection == 3){
	double d=t.ncols;
	if (inputdouble((lang==1?"Nombre de colonnes?":"Colonnes?"),d,contextptr) && d==int(d) && d>0){
	  t.ncols=d;
	}
	continue;
      }
      if (smallmenu.selection == 4){
	t.recompute=!t.recompute;
	continue;
      }
      if (smallmenu.selection==5){
	t.matrix_fill_cells=!t.matrix_fill_cells;
	continue;
      }
      if (smallmenu.selection == 6){
	t.movedown=!t.movedown;
	continue;
      }
      if (smallmenu.selection == smallmenu.numitems){
	change_undo(t);
	resizesheet(t);
	break;
      }	
    }      
  } // end endless while
}

void sheet_graph(tableur &t,GIAC_CONTEXT){
  vecteur v;
  sheet_pnt(t.m,v);
  gen g(v);
  check_do_graph(g,2,contextptr);
}

int sheet_menu_menu(tableur & t,GIAC_CONTEXT){
  t.cmd_row=-1; t.cmd_pos=-1; t.sel_row_begin=-1;
  Menu smallmenu;
  smallmenu.numitems=14;
  MenuItem smallmenuitems[smallmenu.numitems];
  smallmenu.items=smallmenuitems;
  smallmenu.height=12;
  //smallmenu.width=24;
  smallmenu.scrollbar=1;
  smallmenu.scrollout=1;
  smallmenu.title = (char*)(lang==1?"Esc: annule menu tableur":"Esc: cancel sheet menu");
  smallmenuitems[0].text = (char *)(lang==1?"Sauvegarder tableur":"Save sheet");
  smallmenuitems[1].text = (char *)(lang==1?"Sauvegarder tableur comme":"Save sheet as");
  smallmenuitems[2].text = (char*)(lang==1?"Charger":"Load");
  string cell=(lang==1?"Editer cellule ":"Edit cell ")+printcell(t.cur_row,t.cur_col);
  smallmenuitems[3].text = (char*)cell.c_str();
  smallmenuitems[4].text = (char*)(lang==1?"Voir graphique (shift 6)":"View graph (shift 4)");
  smallmenuitems[5].text = (char*)(lang==1?"Copier vers le bas (ctrl D)":"Copy down (ctrl D)");
  smallmenuitems[6].text = (char*)(lang==1?"Copier vers la droite (ctrl R)":"Copy right (ctrl R)");
  smallmenuitems[7].text = (char*)(lang==1?"Inserer une ligne":"Insert row");
  smallmenuitems[8].text = (char*)(lang==1?"Inserer une colonne":"Insert column");
  smallmenuitems[9].text = (char*)(lang==1?"Effacer ligne courante":"Remove current row");
  smallmenuitems[10].text = (char*)(lang==1?"Effacer colonne courante":"Remove current column");
  smallmenuitems[11].text = (char*)(lang==1?"Remplir le tableau de 0":"Fill sheet with 0");
  smallmenuitems[smallmenu.numitems-2].text = (char*) "Config";
  smallmenuitems[smallmenu.numitems-1].text = (char*) (lang==1?"Quitter tableur":"Leave sheet");
  while(1) {
    int sres = doMenu(&smallmenu);
    if (sres==MENU_RETURN_EXIT)
      return -1;
    if (sres == MENU_RETURN_SELECTION  || sres==KEY_CTRL_EXE) {
      if (smallmenu.selection == 1){
	// save
	save_sheet(t,contextptr);
	return -1;
      }
      if (smallmenu.selection == 2 ){
	// save
	char buf[270];
	if (get_filename(buf,".tab")){
	  t.filename=remove_path(remove_extension(buf));
	  save_sheet(t,contextptr);
	  return -1;
	}
      }
      if (smallmenu.selection== 3 && !exam_mode) {
	char filename[128];
	if (giac_filebrowser(filename,"tab",(lang==1?"Fichiers tableurs":"Sheet files"))){
	  if (t.changed && do_confirm(lang==1?"Sauvegarder le tableur actuel?":"Save current sheet?"))
	    save_sheet(t,contextptr);
	  const char * s=read_file(filename);
	  if (s){
	    gen g(s,contextptr);
	    g=eval(g,1,contextptr);
	    if (ckmatrix(g,true)){
	      t.filename=filename;
	      t.m=*g._VECTptr;
	      t.nrows=t.m.size();
	      t.ncols=t.m.front()._VECTptr->size();
	      t.cur_col=t.cur_row=0;
	      t.sel_row_begin=t.cmd_row=-1;
	    }
	    else
	      s=0;
	  }
	  if (!s)
	    do_confirm(lang==1?"Erreur de lecture du fichier":"Error reading file");
	}
	return -1;
      } // end load
      if (smallmenu.selection==4){
	activate_cmdline(t);
	t.cmd_pos=t.cmdline.size();
	return -1;
      }
      if (smallmenu.selection==5){
	sheet_graph(t,contextptr);
	return -1;
      }
      if (smallmenu.selection==6){
	t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	copy_down(t,contextptr);
	return -1;
      }
      if (smallmenu.selection==7){
	t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	copy_right(t,contextptr);
	return -1;
      }
      if (smallmenu.selection==8){
	t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	change_undo(t);
	t.m=matrice_insert(t.m,t.cur_row,t.cur_col,1,0,makevecteur(0,0,2),contextptr);
	t.nrows++;
	return -1;
      }
      if (smallmenu.selection==9){
	t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	change_undo(t);
	t.m=matrice_insert(t.m,t.cur_row,t.cur_col,0,1,makevecteur(0,0,2),contextptr);
	t.ncols++;
	return -1;
      }
      if (smallmenu.selection==10 && t.nrows>=2){
	t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	change_undo(t);
	t.m=matrice_erase(t.m,t.cur_row,t.cur_col,1,0,contextptr);
	--t.nrows;
	return -1;
      }
      if (smallmenu.selection==11 && t.ncols>=2){
	t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	change_undo(t);
	t.m=matrice_erase(t.m,t.cur_row,t.cur_col,0,1,contextptr);
	--t.ncols;
	return -1;
      }
      if (smallmenu.selection==12){
	t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	change_undo(t);
	gen g=vecteur(t.ncols);
	t.m=makefreematrice(vecteur(t.nrows,g));
	makespreadsheetmatrice(t.m,contextptr);
	return -1;
      }
      if (smallmenu.selection == smallmenu.numitems-1){
	sheet_menu_setup(t,contextptr);
	continue;
      }
      if (smallmenu.selection == smallmenu.numitems){
	return 0;
      }
    }
  } // end endless while
  return 1;
}

void sheet_cmd(tableur & t,const char * ans){
  string s=ans; 
  if (t.sel_row_begin>=0){
    t.cmdline="";
    s="="+s+"matrix("+print_INT_(absint(t.sel_row_begin-t.cur_row)+1)+","+print_INT_(absint(t.sel_col_begin-t.cur_col)+1)+","+printsel(t.sel_row_begin,t.sel_col_begin,t.cur_row,t.cur_col)+")";
    if (t.cur_row<t.sel_row_begin)
      t.cur_row=t.sel_row_begin;
    t.sel_row_begin=-1;
    if (t.cur_col<t.sel_col_begin)
      t.cur_col=t.sel_col_begin;
    int i,j=t.cur_col;
    // find empty cell in next rows
    for (i=t.cur_row;i<t.nrows;++i){
      if (is_zero(t.m[i][t.cur_col][0]))
	break;
    }
    if (i==t.nrows){
      // find an empty cell in next columns
      for (j=t.cur_col+1;j<t.ncols;++j){
	for (i=0;i<t.nrows;++i){
	  if (is_zero(t.m[i][j][0]))
	    break;
	}
	if (i<t.nrows)
	  break;
      }
    }
    if (i<t.nrows && j<t.ncols){
      t.cur_row=i;
      t.cur_col=j;
    }
    else {
      do_confirm((lang==1?"Impossible de trouver une cellule libre":"Could not find an empty cell"));
      return;
    }
  }
  activate_cmdline(t);
  insert(t.cmdline,t.cmd_pos,s.c_str());
  t.cmd_pos += s.size();
  t.keytooltip=true;
}

void sheet_cmdline(tableur &t,GIAC_CONTEXT){
  gen g(t.cmdline,contextptr);
  change_undo(t);
  bool doit=true;
  bool tableseq=g.is_symb_of_sommet(at_tableseq);
  bool tablefunc=g.is_symb_of_sommet(at_tablefunc);
  if (tableseq || t.matrix_fill_cells){
    set_abort();
    gen g1=protecteval(g,1,contextptr);
    clear_abort();
    if (g1.type==_VECT){
      doit=false;
      matrice & m=*g1._VECTptr;
      if (!ckmatrix(m) && t.movedown)
	m=mtran(vecteur(1,m));
      matrice clip=t.clip;
      makespreadsheetmatrice(m,contextptr);
      t.clip=m;
      paste(t,contextptr);
      t.clip=clip;
      if (tableseq && t.cur_row+4<t.nrows){
	t.cur_row += 4;
	copy_down(t,contextptr);
      }
      if (tablefunc && t.cur_row+3<t.nrows && t.cur_col+1<t.ncols){
	t.cur_row += 3;
	copy_down(t,contextptr);
	t.cur_col++;
	copy_down(t,contextptr);
      }
    }
  }
  if (doit) {
    if (t.cmd_row<t.m.size()){
      gen v=t.m[t.cmd_row];
      if (v.type==_VECT && t.cmd_col>=0 && t.cmd_col<v._VECTptr->size()){
	vecteur w=*v._VECTptr;
	g=spread_convert(g,t.cur_row,t.cur_col,contextptr);
	w[t.cmd_col]=makevecteur(g,g,0);
	t.m[t.cmd_row]=w;
	sheet_eval(t,contextptr,true);
      }
    }
  }
  t.cur_row=t.cmd_row;
  t.cur_col=t.cmd_col;
  t.cmd_row=-1;
  t.cmd_pos=-1;
  if (t.movedown){
    ++t.cur_row;
    if (t.cur_row>=t.nrows){
      t.cur_row=0;
      ++t.cur_col;
      if (t.cur_col>=t.ncols)
	t.cur_col=0;
    }
  }
  else {
    ++t.cur_col;
    if (t.cur_col>=t.ncols){
      t.cur_col=0;
      ++t.cur_row;
      if (t.cur_row>=t.nrows){
	t.cur_row=0;
      }
    }
  }
}

void sheet_help_insert(tableur & t,int exec,GIAC_CONTEXT){
  int back;
  string adds=help_insert(t.cmdline.substr(0,t.cmd_pos).c_str(),back,exec,contextptr);
  if (back>=t.cmd_pos){
    t.cmdline=t.cmdline.substr(0,t.cmd_pos-back)+t.cmdline.substr(t.cmd_pos,t.cmdline.size()-t.cmd_pos);
    t.cmd_pos-=back;
  }
  if (!adds.empty())
    sheet_cmd(t,adds.c_str());
}

giac::gen sheet(GIAC_CONTEXT){
  if (!sheetptr)
    sheetptr=new_tableur(contextptr);
  tableur & t=*sheetptr;
  bool status_freeze=false;
  t.keytooltip=false;
  for (;;){
    int R=t.cur_row,C=t.cur_col;
    if (t.cmd_row>=0){
      R=t.cmd_row;
      C=t.cmd_col;
    }
    printcell_current_row(contextptr)=R;
    printcell_current_col(contextptr)=C;
    if (!status_freeze)
      sheet_status(t,contextptr);
    sheet_display(t,contextptr);
    int key=getkey(1);
    if (key==KEY_SHUTDOWN)
      return key;
    if (t.keytooltip){
      t.keytooltip=false;
      if (key==KEY_CTRL_EXIT)
	continue;
      if (key==KEY_CTRL_RIGHT && t.cmd_pos==t.cmdline.size())
	key=KEY_CTRL_OK;
      if (key==KEY_CTRL_DOWN || key==KEY_CTRL_VARS)
	key=KEY_BOOK;
      if (key==KEY_CTRL_OK || key==KEY_CHAR_ANS){
	sheet_help_insert(t,key,contextptr);
	continue;
      }
    }
    status_freeze=false;
    if (key==KEY_CTRL_SETUP){
      sheet_menu_setup(t,contextptr);
      continue;
    }
    if (key==KEY_CTRL_MENU){
      if (sheet_menu_menu(t,contextptr)==0)
	return 0;
    }
    if (key==KEY_CTRL_EXIT){
      if (t.sel_row_begin>=0){
	t.sel_row_begin=-1;
	continue;
      }
      if (t.cmd_row>=0){
	bool b= t.cmd_row==t.cur_row && t.cmd_col==t.cur_col;
	t.cur_row=t.cmd_row;
	t.cur_col=t.cmd_col;
	if (b)
	  t.cmd_row=-1;
	continue;
      }
      if (!t.changed || do_confirm("Quit?"))
	return 0;
    }
    switch (key){
    case KEY_CTRL_UNDO:
      std::swap(t.m,t.undo);
      sheet_eval(t,contextptr);
      continue;
    case KEY_CTRL_CLIP:
      if (t.sel_row_begin<0){
	t.sel_row_begin=t.cur_row;
	t.sel_col_begin=t.cur_col;
      }
      else {
	int r=t.cur_row,R=t.sel_row_begin,c=t.cur_col,C=t.sel_col_begin;
	if (r>R)
	  swapint(r,R);
	if (c>C)
	  swapint(c,C);
	t.clip=matrice_extract(t.m,r,c,R-r+1,C-c+1);
	copy_clipboard(gen(extractmatricefromsheet(t.clip)).print(contextptr).c_str(),true);
	t.sel_row_begin=-1;
      }
      continue;
    case KEY_CTRL_PASTE:
      paste(t,contextptr);
      status_freeze=true;
      continue;
    case KEY_SELECT_RIGHT:
      if (t.sel_row_begin<0){
	t.sel_row_begin=t.cur_row;
	t.sel_col_begin=t.cur_col;
      }
    case KEY_CTRL_RIGHT:
      if (t.cmd_pos>=0 && t.cmd_row==t.cur_row && t.cmd_col==t.cur_col && t.sel_row_begin==-1){
	++t.cmd_pos;
	if (t.cmd_pos>t.cmdline.size())
	  t.cmd_pos=t.cmdline.size();
      }
      else {
	++t.cur_col;
	if (t.cur_col>=t.ncols)
	  t.cur_col=0;
      }
      continue;
    case KEY_SHIFT_RIGHT:
      if (t.cmd_pos>=0 && t.cmd_row==t.cur_row && t.cmd_col==t.cur_col && t.sel_row_begin==-1){
	t.cmd_pos=t.cmdline.size();
      }
      else 
	t.cur_col=t.ncols-1;
      break;
    case KEY_SELECT_LEFT:
      if (t.sel_row_begin<0){
	t.sel_row_begin=t.cur_row;
	t.sel_col_begin=t.cur_col;
      }
    case KEY_CTRL_LEFT:
      if (t.cmd_pos>=0 && t.cmd_row==t.cur_row && t.cmd_col==t.cur_col && t.sel_row_begin==-1){
	if (t.cmd_pos>0)
	  --t.cmd_pos;
      }
      else {
	--t.cur_col;
	if (t.cur_col<0)
	  t.cur_col=t.ncols-1;
      }
      continue;
    case KEY_SHIFT_LEFT:
      if (t.cmd_pos>=0 && t.cmd_row==t.cur_row && t.cmd_col==t.cur_col && t.sel_row_begin==-1){
	t.cmd_pos=0;
      }
      else {
	t.cur_col=0;
      }
      break;
    case KEY_SELECT_UP:
      if (t.sel_row_begin<0){
	t.sel_row_begin=t.cur_row;
	t.sel_col_begin=t.cur_col;
      }
    case KEY_CTRL_UP:
      --t.cur_row;
      if (t.cur_row<0)
	t.cur_row=t.nrows-1;
      continue;
    case KEY_SELECT_DOWN:
      if (t.sel_row_begin<0){
	t.sel_row_begin=t.cur_row;
	t.sel_col_begin=t.cur_col;
      }
    case KEY_CTRL_DOWN:
      ++t.cur_row;
      if (t.cur_row>=t.nrows)
	t.cur_row=0;
      continue;
    case KEY_CTRL_DEL:
      if (t.cmd_row>=0){
	if (t.cmd_pos>0){
	  t.cmdline.erase(t.cmdline.begin()+t.cmd_pos-1);
	  --t.cmd_pos;
	  t.keytooltip=true;
	}
      }
      else {
	t.cmdline="";
	t.cmd_row=t.cur_row;
	t.cmd_col=t.cur_col;
	t.cmd_pos=0;
      }
      continue;
    case KEY_CTRL_EXE:
#if 1
      if (t.cmd_row<0){
	sheet_eval(t,contextptr);
	continue;
      }
#else
      if (t.cmd_row<0){
	int r=t.sel_row_begin;
	if (r<0)
	  return extractmatricefromsheet(t.m);
	int R=t.cur_row,c=t.sel_col_begin,C=t.cur_col;
	if (r>R)
	  swapint(r,R);
	if (c>C)
	  swapint(c,C);
	return extractmatricefromsheet(matrice_extract(t.m,r,c,R-r+1,C-c+1));
      }
#endif
    case KEY_CTRL_OK:
      if (t.cmd_row>=0){
	string s;
	if (t.sel_row_begin>=0){
	  s=printsel(t.sel_row_begin,t.sel_col_begin,t.cur_row,t.cur_col);
	  t.cur_row=t.cmd_row;
	  t.cur_col=t.cmd_col;
	  t.sel_row_begin=-1;
	}
	if (t.cmd_row!=t.cur_row || t.cmd_col!=t.cur_col){
	  s=printcell(t.cur_row,t.cur_col);
	  t.cur_row=t.cmd_row;
	  t.cur_col=t.cmd_col;
	}
	if (s.empty())
	  sheet_cmdline(t,contextptr);
	else {
	  insert(t.cmdline,t.cmd_pos,s.c_str());
	  t.cmd_pos+=s.size();
	}
      } // if t.cmd_row>=0
      else {
	t.cmd_row=t.cur_row;
	t.cmd_col=t.cur_col;
	t.cmd_pos=t.cmdline.size();
      }
      continue;
    case KEY_CTRL_F5: // view
      {
	string value((*t.m[t.cur_row]._VECTptr)[t.cur_col][1].print(contextptr));
	char buf[1024];
	strcpy(buf,value.substr(0,1024-1).c_str());
	textedit(buf,1024-1,contextptr );
      }
      continue;
    case KEY_CTRL_F6: // view graph
      sheet_graph(t,contextptr);
      continue;
    case KEY_CTRL_D: // copy down
      copy_down(t,contextptr);
      continue;
#ifndef NUMWORKS
    case KEY_CTRL_R:
      copy_right(t,contextptr);
      continue;
#endif
    case KEY_CTRL_CATALOG: case KEY_BOOK: case '\t':
      {
	if (t.cmd_pos>=0)
	  sheet_help_insert(t,0,contextptr);
      }
      continue;
    } // end switch
    if ( (key >= KEY_CTRL_F1 && key <= KEY_CTRL_F6) ||
	  (key >= KEY_CTRL_F7 && key <= KEY_CTRL_F14) 
	 ){
      const char tmenu[]= "F1 stat1d\nsum(\nmean(\nstddev(\nmedian(\nhistogram(\nbarplot(\nboxwhisker(\nF2 stat2d\nlinear_regression_plot(\nlogarithmic_regression_plot(\nexponential_regression_plot(\npower_regression_plot(\npolynomial_regression_plot(\nsin_regression_plot(\nscatterplot(\npolygonscatterplot(\nF3 seq\nrange(\nseq(\ntableseq(\nplotseq(\ntablefunc(\nrandvector(\nrandmatrix(\nF4 edt\nedit_cell\nundo\ncopy_down\ncopy_right\ninsert_row\ninsert_col\nerase_row\nerase_col\nF6 graph\nreserved\nF= poly\nproot(\npcoeff(\nquo(\nrem(\ngcd(\negcd(\nresultant(\nGF(\nF: arit\n mod \nirem(\nifactor(\ngcd(\nisprime(\nnextprime(\npowmod(\niegcd(\nF8 list\nmakelist(\nrange(\nseq(\nlen(\nappend(\nranv(\nsort(\napply(\nF; plot\nplot(\nplotseq(\nplotlist(\nplotparam(\nplotpolar(\nplotfield(\nhistogram(\nbarplot(\nF7 real\nexact(\napprox(\nfloor(\nceil(\nround(\nsign(\nmax(\nmin(\nF< prog\n:\n&\n#\nhexprint(\nbinprint(\nf(x):=\ndebug(\npython(\nF> cplx\nabs(\narg(\nre(\nim(\nconj(\ncsolve(\ncfactor(\ncpartfrac(\nF= misc\n!\nrand(\nbinomial(\nnormald(\nexponentiald(\n\\\n % \n\n";
      const char * s=console_menu(key,(char *)tmenu,0);
      if (s && strlen(s)){
	if (strcmp(s,"undo")==0){
	  t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	  std::swap(t.m,t.undo);
	  sheet_eval(t,contextptr);
	  continue;
	}
	if (strcmp(s,"copy_down")==0){
	  t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	  copy_down(t,contextptr);
	  continue;
	}
	if (strcmp(s,"copy_right")==0){
	  t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	  copy_right(t,contextptr);
	  continue;
	}
	if (strcmp(s,"insert_row")==0){
	  t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	  change_undo(t);
	  t.m=matrice_insert(t.m,t.cur_row,t.cur_col,1,0,makevecteur(0,0,2),contextptr);
	  t.nrows++;
	  continue;
	}
	if (strcmp(s,"insert_col")==0){
	  t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	  change_undo(t);
	  t.m=matrice_insert(t.m,t.cur_row,t.cur_col,0,1,makevecteur(0,0,2),contextptr);
	  t.ncols++;
	  continue;
	}
	if (strcmp(s,"erase_row")==0 && t.nrows>=2){
	  t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	  change_undo(t);
	  t.m=matrice_erase(t.m,t.cur_row,t.cur_col,1,0,contextptr);
	  --t.nrows;
	  continue;
	}
	if (strcmp(s,"erase_col")==0 && t.ncols>=2){
	  t.cmd_pos=t.cmd_row=t.sel_row_begin=-1;
	  change_undo(t);
	  t.m=matrice_erase(t.m,t.cur_row,t.cur_col,0,1,contextptr);
	  --t.ncols;
	  continue;
	}
	if (strcmp(s,"edit_cell")==0){
	  if (t.cmd_row<0 && t.sel_row_begin<0){
	    char buf[1024];
	    strcpy(buf,t.cmdline.substr(0,1024-1).c_str());
	    if (textedit(buf,1024-1,contextptr )){
	      t.cmdline=buf;
	      t.cmd_row=t.cur_row; t.cmd_col=t.cur_col;
	      sheet_cmdline(t,contextptr);
	    }
	  }
	  continue;
	}
	if (t.cmd_row<0)
	  t.cmdline="";
	sheet_cmd(t,s);
      }
      continue;
    }
    if (key==KEY_CHAR_CROCHETS || key==KEY_CHAR_ACCOLADES){
      if (t.cmd_row<0)
	t.cmdline="";
      activate_cmdline(t);
      t.cmdline.insert(t.cmdline.begin()+t.cmd_pos,key==KEY_CHAR_CROCHETS?'[':'{');
      ++t.cmd_pos;
      t.cmdline.insert(t.cmdline.begin()+t.cmd_pos,key==KEY_CHAR_CROCHETS?']':'}');
      continue;
    }
    if (key>=32 && key<128){
      if (t.cmd_row<0)
	t.cmdline="";
      activate_cmdline(t);
      t.cmdline.insert(t.cmdline.begin()+t.cmd_pos,char(key));
      ++t.cmd_pos;
      t.keytooltip=true;
      continue;
    }
    if (const char * ans=keytostring(key,0,false,contextptr)){
      if (ans && strlen(ans)){
	if (t.cmd_row<0)
	  t.cmdline="";
	sheet_cmd(t,ans);
      }
      continue;
    }
    if (key==KEY_CTRL_AC && t.cmd_row>=0){
      if (t.cmdline=="")
	t.cmd_row=-1;
      t.cmdline="";
      t.cmd_pos=0;
      continue;
    }
    
  }
}


#endif
