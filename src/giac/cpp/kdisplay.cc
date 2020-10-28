// -*- mode:C++ ; compile-command: "g++-3.4 -I. -I.. -g -c Equation.cc -DHAVE_CONFIG_H -DIN_GIAC -Wall" -*-
/*
 *  Copyright (C) 2005,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#include "config.h"
#include "giacPCH.h"
#if defined HAVE_UNISTD_H && !defined NUMWORKS
#include <dirent.h>
#endif
#ifdef NSPIRE_NEWLIB
#include <fstream>
#include <libndls.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <os.h>
#include <syscall.h>
#include "sha256.h"
#endif
#ifndef is_cx2
#define is_cx2 false
#endif
#ifdef KHICAS
#ifndef NSPIRE_NEWLIB
extern "C" {
  short int nspire_exam_mode=0;
}
#endif
#define XWASPY 1 // save .xw file as _xw.py (to be recognized by Numworks workshop)
const int xwaspy_shift=33; // must be between 32 and 63, reflect in xcas.js and History.cc
#include "kdisplay.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "input_lexer.h"
#include "input_parser.h"
//giac::context * contextptr=0;
int clip_ymin=0;
int lang=1;
short int nspirelua=0;
bool warn_nr=true; 
bool xthetat=false;
//bool freezeturtle=false;
bool global_show_axes=true;
int esc_flag=0;
int xcas_python_eval=0;
char * python_heap=0;
#ifdef DEVICE
int python_stack_size=30*1024,python_heap_size=40*1024;
#else
int python_stack_size=64*1024,python_heap_size=256*1024;
#endif
#ifdef MICROPY_LIB
extern "C" int mp_token(const char * line);

void python_free(){
  if (!python_heap) return;
  mp_deinit(); free(python_heap); python_heap=0;
}

int python_init(int stack_size,int heap_size){
#if 1 // defined NUMWORKS
  python_free();
  python_heap=micropy_init(stack_size,heap_size);
  if (!python_heap)
    return 0;
#endif
  return 1;
}

int micropy_ck_eval(const char *line){
#if 1 // def NUMWORKS
  giac::ctrl_c=giac::interrupted=false;
  if (python_heap && line[0]==0)
    return 1;
  if (!python_heap){
    python_init(python_stack_size,python_heap_size);
  }
  if (!python_heap){
    console_output("Memory full",11);
    return RAND_MAX;
  }
#endif
  return micropy_eval(line);
  // if MP_PARSE_SINGLE_INPUT is used, split input if newline not followed by a space, return shift
  int shift=0,nl=0;
  const char * ptr=line;
  for (;;++ptr){
    if (*ptr=='\n')
      ++nl;
    if (*ptr==0 || (*ptr=='\n' && *(ptr+1)!=' ')){
      int n=ptr-line;
      char buf[n+1];
      strncpy(buf,line,n);
      buf[n]=0;
      micropy_eval(buf);
      if (parser_errorline)
	return shift;
      if (*ptr==0)
	return 0;
      line=ptr+1;
      shift=nl;
    }
  }
  return 0;
}
#endif

using namespace std;
using namespace giac;
const int LCD_WIDTH_PX=320;
const int LCD_HEIGHT_PX=222;
char* fmenu_cfg=0;
int khicas_addins_menu(GIAC_CONTEXT); // in kadd.cc
#ifdef MICROPY_LIB
extern "C" const char * const * mp_vars();
#endif

// Numworks Logo commands
#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC
  void Bdisp_PutDisp_DD(){
    sync_screen();
  }
  void Bdisp_AllClr_VRAM(){
    waitforvblank();
    drawRectangle(0,0,LCD_WIDTH_PX,LCD_HEIGHT_PX,_WHITE);
  }
  void drawLine(int x1,int y1,int x2,int y2,int c){
    draw_line(x1,y1,x2,y2,c,context0);
  }
  void stroke_rectangle(int x,int y,int w,int h,int c){
    drawLine(x,y,x+w,y,c);
    drawLine(x,y+h,x+w,y+h,c);
    drawLine(x,y,x,y+h,c);
    drawLine(x+w,y,x+w,y+h,c);
  }
  void DefineStatusMessage(const char * s,int a,int b,int c){
    statuslinemsg(s);
  }

  void DisplayStatusArea(){
    sync_screen();
  }

  void set_xcas_status(){
    statusline(1+2*xcas_python_eval);
  }
  int GetSetupSetting(int mode){
    return 0;
  }

  void SetSetupSetting(int mode,int){
  }

  void handle_f5(){
    lock_alpha();
  }

  void delete_clipboard(){}

  bool clip_pasted=true;
  
  string * clipboard(){
    static string * ptr=0;
    if (!ptr)
      ptr=new string;
    return ptr;
  }
  
  void copy_clipboard(const string & s,bool status){
    if (1 || clip_pasted) // adding to clipboard is sometimes annoying
      *clipboard()=s;
    else
      *clipboard()+=s;
    clip_pasted=false;
    if (status){
      DefineStatusMessage((char*)((lang==1)?"Selection copiee vers presse-papiers.":"Selection copied to clipboard"), 1, 0, 0);
      DisplayStatusArea();
    }
  }
  
  const char * paste_clipboard(){
    clip_pasted=true;
    return clipboard()->c_str();
  }
  
  int print_msg12(const char * msg1,const char * msg2,int textY=40){
    drawRectangle(0, textY+10, LCD_WIDTH_PX, 44, COLOR_WHITE);
    drawRectangle(3,textY+10,316,3, COLOR_BLACK);
    drawRectangle(3,textY+10,3,44, COLOR_BLACK);
    drawRectangle(316,textY+10,3,44, COLOR_BLACK);
    drawRectangle(3,textY+54,316,3, COLOR_BLACK);
    int textX=30;
    if (msg1){
      if (strlen(msg1)>=30)
	os_draw_string_small_(textX,textY+15,msg1);
      else
	os_draw_string_(textX,textY+15,msg1);
    }
    textX=10;
    textY+=33;
    if (msg2){
      if (strlen(msg2)>=30)
	os_draw_string_small_(textX,textY,msg2);
      else      
	textX=os_draw_string_(textX,textY,msg2);
    }
    return textX;
  }
  
  void insert(string & s,int pos,const char * add){
    if (pos>s.size())
      pos=s.size();
    if (pos<0)
      pos=0;
    s=s.substr(0,pos)+add+s.substr(pos,s.size()-pos);
  }
  
  bool do_confirm(const char * s){
#ifdef NSPIRE_NEWLIB
    return confirm(s,((lang==1)?"enter: oui,  esc:annuler":"enter: yes,   esc: cancel"))==KEY_CTRL_F1;
#else
    return confirm(s,((lang==1)?"OK: oui,  Back:annuler":"OK: yes,   Back: cancel"))==KEY_CTRL_F1;
#endif
  }
  
  int confirm(const char * msg1,const char * msg2,bool acexit,int y){
    int key=0;
    print_msg12(msg1,msg2,y);
    while (key!=KEY_CTRL_F1 && key!=KEY_CTRL_F6){
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;
      if (key==KEY_CTRL_EXE || key==KEY_CTRL_OK)
	key=KEY_CTRL_F1;
      if (key==KEY_CTRL_AC || key==KEY_CTRL_EXIT || key==KEY_CTRL_MENU){
	if (acexit) return -1;
	key=KEY_CTRL_F6;
      }
      set_xcas_status();
    }
    return key;
  }  
  
  bool confirm_overwrite(){
#ifdef NSPIRE_NEWLIB
    return do_confirm((lang==1)?"enter: oui,  esc:annuler":"enter: yes,   esc: cancel")==KEY_CTRL_F1;
#else
    return do_confirm((lang==1)?"OK: oui,  Back:annuler":"OK: yes,   Back: cancel")==KEY_CTRL_F1;
#endif
  }
  
  void invalid_varname(){
    confirm((lang==1)?"Nom de variable incorrect":"Invalid variable name",
#ifdef NSPIRE_NEWLIB
	    (lang==1)?"enter: ok":"enter: ok"
#else
	    (lang==1)?"OK: ok":"OK: ok"
#endif
	    );
  }


#ifdef SCROLLBAR
  typedef scrollbar TScrollbar;
#endif

#define C24 18 // 24 on 90
#define C18 18 // 18
#define C10 10 // 18
#define C6 6 // 6

  int MB_ElementCount(const char * s){
    return strlen(s); // FIXME for UTF8
  }

  void PrintXY(int x,int y,const char * s,int mode,int c=giac::_BLACK,int bg=giac::_WHITE){
    if (mode==TEXT_MODE_NORMAL)
      os_draw_string(x,y,c,bg,s);
    else
      os_draw_string(x,y,bg,c,s);
  }

  int PrintMiniMini(int x,int y,const char * s,int mode,int c=giac::_BLACK,int bg=giac::_WHITE,bool fake=false){
    if (mode==TEXT_MODE_NORMAL)
      return os_draw_string_small(x,y,c,bg,s,fake);
    else
      return os_draw_string_small(x,y,bg,c,s,fake);
  }
  
  int PrintMini(int x,int y,const char * s,int mode,int c=giac::_BLACK,int bg=giac::_WHITE,bool fake=false){
    if (mode==TEXT_MODE_NORMAL)
      return os_draw_string(x,y,c,bg,s,fake);
    else
      return os_draw_string(x,y,bg,c,s,fake);
  }
  
  void printCentered(const char* text, int y) {
    int len = strlen(text);
    int x = LCD_WIDTH_PX/2-(len*6)/2;
    PrintXY(x,y,text,0);
  }

  int doMenu(Menu* menu, MenuItemIcon* icontable) { // returns code telling what user did. selection is on menu->selection. menu->selection starts at 1!
    int itemsStartY=menu->startY; // char Y where to start drawing the menu items. Having a title increases this by one
    int itemsHeight=menu->height;
    int showtitle = menu->title != NULL;
    if (showtitle) {
      itemsStartY++;
      itemsHeight--;
    }
    char keyword[5];
    keyword[0]=0;
    if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
      menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
    if(menu->selection-1 < menu->scroll)
      menu->scroll = menu->selection -1;
  
    while(1) {
      // Cursor_SetFlashOff();
      if (menu->selection <=1)
	menu->selection=1;
      if (menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
	menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
      if (menu->selection-1 < menu->scroll)
	menu->scroll = menu->selection -1;
      if(menu->statusText != NULL) DefineStatusMessage(menu->statusText, 1, 0, 0);
      // Clear the area of the screen we are going to draw on
      if(0 == menu->pBaRtR) {
	int x=C10*menu->startX-1,
	  y=C24*(menu->miniMiniTitle ? itemsStartY:menu->startY)-1,
	  w=2+C10*menu->width /* + ((menu->scrollbar && menu->scrollout)?C10:0) */,
	  h=2+C24*menu->height-(menu->miniMiniTitle ? C24:0);
	// drawRectangle(x, y, w, h, COLOR_WHITE);
	draw_line(x,y,x+w,y,COLOR_BLACK,context0);
	draw_line(x,y+h,x+w,y+h,COLOR_BLACK,context0);
	draw_line(x,y,x,y+h,COLOR_BLACK,context0);
	draw_line(x+w,y,x+w,y+h,COLOR_BLACK,context0);
      }
      if (menu->numitems>0) {
	for(int curitem=0; curitem < menu->numitems; curitem++) {
	  // print the menu item only when appropriate
	  if(menu->scroll <= curitem && menu->scroll > curitem-itemsHeight) {
	    if ((curitem-menu->scroll) % 6==0)
	      waitforvblank();
	    char menuitem[256] = "";
	    if(menu->numitems>=100 || menu->type == MENUTYPE_MULTISELECT){
	      strcpy(menuitem, "  "); //allow for the folder and selection icons on MULTISELECT menus (e.g. file browser)
	      strcpy(menuitem+2,menu->items[curitem].text);
	    }
	    else {
	      int cur=curitem+1;
	      if (menu->numitems<10){
		menuitem[0]='0'+cur;
		menuitem[1]=' ';
		menuitem[2]=0;
		strcpy(menuitem+2,menu->items[curitem].text);
	      }
	      else {
		menuitem[0]=cur>=10?('0'+(cur/10)):' ';
		menuitem[1]='0'+(cur%10);
		menuitem[2]=' ';
		menuitem[3]=0;
		strcpy(menuitem+3,menu->items[curitem].text);
	      }
	    }
	    //strncat(menuitem, menu->items[curitem].text, 68);
	    if(menu->items[curitem].type != MENUITEM_SEPARATOR) {
	      //make sure we have a string big enough to have background when item is selected:          
	      // MB_ElementCount is used instead of strlen because multibyte chars count as two with strlen, while graphically they are just one char, making fillerRequired become wrong
	      int fillerRequired = menu->width - MB_ElementCount(menu->items[curitem].text) - (menu->type == MENUTYPE_MULTISELECT ? 2 : 3);
	      for(int i = 0; i < fillerRequired; i++)
		strcat(menuitem, " ");
	      drawRectangle(C10*menu->startX,C18*(curitem+itemsStartY-menu->scroll),C10*menu->width,C24,(menu->selection == curitem+1 ? _BLACK : _WHITE));
	      PrintXY(C10*menu->startX,C18*(curitem+itemsStartY-menu->scroll),menuitem, (menu->selection == curitem+1 ? TEXT_MODE_INVERT : TEXT_MODE_NORMAL));
	    } else {
	      /*int textX = (menu->startX-1) * C18;
		int textY = curitem*C24+itemsStartY*C24-menu->scroll*C24-C24+C10;
		clearLine(menu->startX, curitem+itemsStartY-menu->scroll, (menu->selection == curitem+1 ? textColorToFullColor(menu->items[curitem].color) : COLOR_WHITE));
		drawLine(textX, textY+C24-4, LCD_WIDTH_PX-2, textY+C24-4, COLOR_GRAY);
		PrintMini(&textX, &textY, (unsigned char*)menuitem, 0, 0xFFFFFFFF, 0, 0, (menu->selection == curitem+1 ? COLOR_WHITE : textColorToFullColor(menu->items[curitem].color)), (menu->selection == curitem+1 ? textColorToFullColor(menu->items[curitem].color) : COLOR_WHITE), 1, 0);*/
	    }
	    // deal with menu items of type MENUITEM_CHECKBOX
	    if(menu->items[curitem].type == MENUITEM_CHECKBOX) {
	      PrintXY(C10*(menu->startX+menu->width-4),C18*(curitem+itemsStartY-menu->scroll),
		      (menu->items[curitem].value == MENUITEM_VALUE_CHECKED ? " [+]" : " [-]"),
		      (menu->selection == curitem+1 ? TEXT_MODE_INVERT : (menu->pBaRtR == 1? TEXT_MODE_NORMAL : TEXT_MODE_NORMAL)));
	    }
	    // deal with multiselect menus
	    if(menu->type == MENUTYPE_MULTISELECT) {
	      if((curitem+itemsStartY-menu->scroll)>=itemsStartY &&
		 (curitem+itemsStartY-menu->scroll)<=(itemsStartY+itemsHeight) &&
		 icontable != NULL
		 ) {
#if 0
		if (menu->items[curitem].isfolder == 1) {
		  // assumes first icon in icontable is the folder icon
		  CopySpriteMasked(icontable[0].data, (menu->startX)*C18, (curitem+itemsStartY-menu->scroll)*C24, 0x12, 0x18, 0xf81f  );
		} else {
		  if(menu->items[curitem].icon >= 0) CopySpriteMasked(icontable[menu->items[curitem].icon].data, (menu->startX)*C18, (curitem+itemsStartY-menu->scroll)*C24, 0x12, 0x18, 0xf81f  );
		}
#endif
	      }
	      if (menu->items[curitem].isselected) {
		if (menu->selection == curitem+1) {
		  PrintXY(C10*menu->startX,C18*(curitem+itemsStartY-menu->scroll),"\xe6\x9b", TEXT_MODE_NORMAL);
		} else {
		  PrintXY(C10*menu->startX,C18*(curitem+itemsStartY-menu->scroll),"\xe6\x9b", TEXT_MODE_NORMAL);
		}
	      }
	    }
	  }
	} // end for curitem<menu->numitem
	int dh=menu->height-menu->numitems-(showtitle?1:0);
	if (dh>0)
	  drawRectangle(C10*menu->startX,C24*(menu->numitems+(showtitle?1:0)),C10*menu->width,C24*dh,_WHITE);
	if (menu->scrollbar) {
#ifdef SCROLLBAR
	  TScrollbar sb;
	  sb.I1 = 0;
	  sb.I5 = 0;
	  sb.indicatormaximum = menu->numitems;
	  sb.indicatorheight = itemsHeight;
	  sb.indicatorpos = menu->scroll;
	  sb.barheight = itemsHeight*C24;
	  sb.bartop = (itemsStartY-1)*C24;
	  sb.barleft = menu->startX*C18+menu->width*C18 - C18 - (menu->scrollout ? 0 : 5);
	  sb.barwidth = C10;
	  Scrollbar(&sb);
#endif
	}
	//if(menu->type==MENUTYPE_MULTISELECT && menu->fkeypage == 0) drawFkeyLabels(0x0037); // SELECT (white)
      } else {
	giac::printCentered(menu->nodatamsg, (itemsStartY*C24)+(itemsHeight*C24)/2-12);
      }
      if(showtitle) {
	int textX = C10*menu->startX, textY=menu->startY*C24;
	drawRectangle(textX,textY,C10*menu->width,C24,_WHITE);
	if (menu->miniMiniTitle) 
	  PrintMini( textX, textY, menu->title, 0 );
	else
	  PrintXY(textX, textY, menu->title, TEXT_MODE_NORMAL);
	if(menu->subtitle != NULL) {
	  int textX=(MB_ElementCount(menu->title)+menu->startX-1)*C18+C10, textY=C10;
	  PrintMini(textX, textY, menu->subtitle, 0);
	}
	PrintXY(textX+C10*(menu->width-5), 1, "____", 0);
	PrintXY(textX+C10*(menu->width-5), 1, keyword, 0);
      }
      /*if(menu->darken) {
	DrawFrame(COLOR_BLACK);
	VRAMInvertArea(menu->startX*C18-C18, menu->startY*C24, menu->width*C18-(menu->scrollout || !menu->scrollbar ? 0 : 5), menu->height*C24);
	}*/
      if(menu->type == MENUTYPE_NO_KEY_HANDLING) return MENU_RETURN_INSTANT; // we don't want to handle keys
      int key;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;
      if (key==KEY_CTRL_MENU){
	menu->selection=menu->numitems;
	return MENU_RETURN_SELECTION;
      }
      if (key<256 && isalpha(key)){
	key=tolower(key);
	int pos=strlen(keyword);
	if (pos>=4)
	  pos=0;
	keyword[pos]=key;
	keyword[pos+1]=0;
	int cur=0;
	for (;cur<menu->numitems;++cur){
#if 1
	  if (strcmp(menu->items[cur].text,keyword)>=0)
	    break;
#else
	  char c=menu->items[cur].text[0];
	  if (key<=c)
	    break;
#endif
	}
	if (cur<menu->numitems){
	  menu->selection=cur+1;
	  if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
	    menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
	  if(menu->selection-1 < menu->scroll)
	    menu->scroll = menu->selection -1;
	}
	continue;
      }
      switch(key) {
      case KEY_CTRL_PAGEDOWN:
	menu->selection+=6;
	if (menu->selection >= menu->numitems)
	  menu->selection=menu->numitems;
	if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
	  menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
	break;
      case KEY_CTRL_DOWN:
	if(menu->selection == menu->numitems)
	  {
	    if(menu->returnOnInfiniteScrolling) {
	      return MENU_RETURN_SCROLLING;
	    } else {
	      menu->selection = 1;
	      menu->scroll = 0;
	    }
	  }
	else
	  {
	    menu->selection++;
	    if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
	      menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
	  }
	if(menu->pBaRtR==1) return MENU_RETURN_INSTANT;
	break;
      case KEY_CTRL_PAGEUP:
	menu->selection-=6;
	if (menu->selection <=1)
	  menu->selection=1;
	if(menu->selection-1 < menu->scroll)
	  menu->scroll = menu->selection -1;
	break;
      case KEY_CTRL_UP:
	if(menu->selection == 1)
	  {
	    if(menu->returnOnInfiniteScrolling) {
	      return MENU_RETURN_SCROLLING;
	    } else {
	      menu->selection = menu->numitems;
	      menu->scroll = menu->selection-(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
	    }
	  }
	else
	  {
	    menu->selection--;
	    if(menu->selection-1 < menu->scroll)
	      menu->scroll = menu->selection -1;
	  }
	if(menu->pBaRtR==1) return MENU_RETURN_INSTANT;
	break;
      case KEY_CTRL_F1:
	if(menu->type==MENUTYPE_MULTISELECT && menu->fkeypage == 0 && menu->numitems > 0) {
          /*if(menu->items[menu->selection-1].isselected) {
            menu->items[menu->selection-1].isselected=0;
            menu->numselitems = menu->numselitems-1;
	    } else {
            menu->items[menu->selection-1].isselected=1;
            menu->numselitems = menu->numselitems+1;
	    }
	    return key; //return on F1 too so that parent subroutines have a chance to e.g. redraw fkeys*/
	} else if (menu->type == MENUTYPE_FKEYS) {
	  return key;
	}
	break;
      case KEY_CTRL_F2:
      case KEY_CTRL_F3:
      case KEY_CTRL_F4:
      case KEY_CTRL_F5:
      case KEY_CTRL_F6: case KEY_CTRL_CATALOG: case KEY_BOOK: case '\t':
      case KEY_CHAR_ANS: 
	if (menu->type == MENUTYPE_FKEYS || menu->type==MENUTYPE_MULTISELECT) return key; // MULTISELECT also returns on Fkeys
	break;
      case KEY_CTRL_PASTE:
	if (menu->type==MENUTYPE_MULTISELECT) return key; // MULTISELECT also returns on paste
      case KEY_CTRL_OPTN:
	if (menu->type==MENUTYPE_FKEYS || menu->type==MENUTYPE_MULTISELECT) return key;
	break;
      case KEY_CTRL_FORMAT:
	if (menu->type==MENUTYPE_FKEYS) return key; // return on the Format key so that event lists can prompt to change event category
	break;
      case KEY_CTRL_RIGHT:
	if(menu->type != MENUTYPE_MULTISELECT) return KEY_BOOK; // break;
	// else fallthrough
      case KEY_CTRL_EXE: case KEY_CTRL_OK:
	if(menu->numitems>0) return key==KEY_CTRL_OK?MENU_RETURN_SELECTION:key;
	break;
      case KEY_CTRL_LEFT:
	if(menu->type != MENUTYPE_MULTISELECT) break;
	// else fallthrough
      case KEY_CTRL_DEL:
	if (strlen(keyword))
	  keyword[strlen(keyword)-1]=0;
	else {
	  if (strcmp(menu->title,"Variables")==0)
	    return key;
	}
	break;
      case KEY_CTRL_AC:
	if (strlen(keyword)){
	  keyword[0]=0;
	  lock_alpha();//SetSetupSetting( (unsigned int)0x14, 0x88);	
	  //DisplayStatusArea();
	  break;
	}
      case KEY_CTRL_EXIT: 
	return MENU_RETURN_EXIT;
	break;
      case KEY_CHAR_1:
      case KEY_CHAR_2:
      case KEY_CHAR_3:
      case KEY_CHAR_4:
      case KEY_CHAR_5:
      case KEY_CHAR_6:
      case KEY_CHAR_7:
      case KEY_CHAR_8:
      case KEY_CHAR_9:
	if(menu->numitems>=(key-0x30)) {
	  menu->selection = (key-0x30);
	  if (menu->type != MENUTYPE_FKEYS) return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_0:
	if(menu->numitems>=10) {
	  menu->selection = 10;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_EXPN:
	if(menu->numitems>=11) {
	  menu->selection = 11;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_LN:
	if(menu->numitems>=12) {
	  menu->selection = 12;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_LOG:
	if(menu->numitems>=13) {
	  menu->selection = 13;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_IMGNRY:
	if(menu->numitems>=14) {
	  menu->selection = 14;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_COMMA:
	if(menu->numitems>=15) {
	  menu->selection = 15;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_POW:
	if(menu->numitems>=16) {
	  menu->selection = 16;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_SIN:
      case KEY_CHAR_COS:
      case KEY_CHAR_TAN:
	if(menu->numitems>=(key-112)) {
	  menu->selection = (key-112);
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_PI:
	if(menu->numitems>=20) {
	  menu->selection = 20;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_ROOT:
	if(menu->numitems>=21) {
	  menu->selection = 21;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_SQUARE:
	if(menu->numitems>=22) {
	  menu->selection = 22;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_LPAR:
      case KEY_CHAR_RPAR:
	if(menu->numitems>=(key-21)) {
	  menu->selection = (key-21);
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      }
    }
    return MENU_RETURN_EXIT;
  }

#define CAT_CATEGORY_ALL 0
#define CAT_CATEGORY_ALGEBRA 1
#define CAT_CATEGORY_LINALG 2
#define CAT_CATEGORY_CALCULUS 3
#define CAT_CATEGORY_ARIT 4
#define CAT_CATEGORY_COMPLEXNUM 5
#define CAT_CATEGORY_PLOT 6
#define CAT_CATEGORY_POLYNOMIAL 7
#define CAT_CATEGORY_PROBA 8
#define CAT_CATEGORY_PROGCMD 9
#define CAT_CATEGORY_REAL 10
#define CAT_CATEGORY_SOLVE 11
#define CAT_CATEGORY_STATS 12
#define CAT_CATEGORY_TRIG 13
#define CAT_CATEGORY_OPTIONS 14
#define CAT_CATEGORY_LIST 15
#define CAT_CATEGORY_MATRIX 16
#define CAT_CATEGORY_PROG 17
#define CAT_CATEGORY_SOFUS 18
#define CAT_CATEGORY_PHYS 19
#define CAT_CATEGORY_UNIT 20
#define CAT_CATEGORY_LOGO 21 // should be the last one
#define XCAS_ONLY 0x80000000
  void init_locale(){
    lang=1;
  }

  const catalogFunc completeCatfr[] = { // list of all functions (including some not in any category)
    // {"cosh(x)", 0, "Hyperbolic cosine of x.", 0, 0, CAT_CATEGORY_TRIG},
    // {"exp(x)", 0, "Renvoie e^x.", "1.2", 0, CAT_CATEGORY_REAL},
    // {"log(x)", 0, "Logarithme naturel de x.", 0, 0, CAT_CATEGORY_REAL},
    // {"sinh(x)", 0, "Hyperbolic sine of x.", 0, 0, CAT_CATEGORY_TRIG},
    // {"tanh(x)", 0, "Hyperbolic tangent of x.", 0, 0, CAT_CATEGORY_TRIG},
    {" boucle for (pour)", "for ", "Boucle definie pour un indice variant entre 2 valeurs fixees", "#\nfor ", 0, CAT_CATEGORY_PROG},
    {" boucle liste", "for in", "Boucle sur tous les elements d'une liste.", "#\nfor in", 0, CAT_CATEGORY_PROG},
    {" boucle while (tantque)", "while ", "Boucle indefinie tantque.", "#\nwhile ", 0, CAT_CATEGORY_PROG},
    {" test si alors", "if ", "Test", "#\nif ", 0, CAT_CATEGORY_PROG},
    {" test sinon", "else ", "Clause fausse du test", 0, 0, CAT_CATEGORY_PROG},
    {" fonction def.", "f(x):=", "Definition de fonction.", "#\nf(x):=", 0, CAT_CATEGORY_PROG},
    {" local j,k;", "local ", "Declaration de variables locales Xcas", 0, 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {" range(a,b)", "in range(", "Dans l'intervalle [a,b[ (a inclus, b exclus)", "# in range(1,10)", 0, CAT_CATEGORY_PROG},
    {" return res;", "return ", "return ou retourne quitte la fonction et renvoie le resultat res", 0, 0, CAT_CATEGORY_PROG},
    //{" edit list ", "list(", "Assistant creation de liste.", 0, 0, CAT_CATEGORY_LIST},
    //{" edit matrix ", "matrix(", "Assistant creation de matrice.", 0, 0, CAT_CATEGORY_MATRIX },
    {" mksa(x)", 0, "Conversion en unites MKSA", 0, 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8)  | XCAS_ONLY},
    {" ufactor(a,b)", 0, "Factorise l'unite b dans a", "100_J,1_kW", 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8) | XCAS_ONLY},
    {" usimplify(a)", 0, "Simplifie l'unite dans a", "100_l/10_cm^2", 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8) | XCAS_ONLY},
    //{"fonction def Xcas", "fonction f(x) local y;   ffonction:;", "Definition de fonction.", "#fonction f(x) local y; y:=x^2; return y; ffonction:;", 0, CAT_CATEGORY_PROG},
    {"!", "!", "Non logique (prefixe) ou factorielle de n (suffixe).", "#7!", "#!b", CAT_CATEGORY_PROGCMD},
    {"#", "#", "Commentaire Python, en Xcas taper //.", 0, 0, CAT_CATEGORY_PROG},
    {"%", "%", "a % b signifie a modulo b", 0, 0, CAT_CATEGORY_ARIT | (CAT_CATEGORY_PROGCMD << 8)},
    {"&", "&", "Et logique ou +", "#1&2", 0, CAT_CATEGORY_PROGCMD},
    {":=", ":=", "Affectation vers la gauche (inverse de =>).", "#a:=3", 0, CAT_CATEGORY_PROGCMD|(CAT_CATEGORY_SOFUS<<8)|XCAS_ONLY},
    {"<", "<", "Inferieur strict. Raccourci SHIFT F2", 0, 0, CAT_CATEGORY_PROGCMD},
    {"=>", "=>", "Affectation vers la droite ou conversion en (touche ->). Par exemple 5=>a ou x^4-1=>* ou (x+1)^2=>+ ou sin(x)^2=>cos.", "#5=>a", "#15_m=>_cm", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_PHYS <<8) | (CAT_CATEGORY_UNIT << 16) | XCAS_ONLY},
    {">", ">", "Superieur strict. Raccourci F2.", 0, 0, CAT_CATEGORY_PROGCMD},
    {"\\", "\\", "Caractere \\", 0, 0, CAT_CATEGORY_PROGCMD},
    {"_", "_", "Caractere _. Prefixe d'unites.", 0, 0, CAT_CATEGORY_PROGCMD},
    {"_(km/h)", "_(km/h)", "Vitesse en kilometre/heure", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m/s)", "_(m/s)", "Vitesse en metre/seconde", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m/s^2)", "_(m/s^2)", "Acceleration en metre par seconde au carre", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m^2/s)", "_(m^2/s)", "Viscosite", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_A", 0, "Intensite electrique en Ampere", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Bq", 0, "Radioactivite: Becquerel", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_C", 0, "Charge electrique en Coulomb", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Ci", 0, "Radioactivite: Curie", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_F", 0, "Farad", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_F_", 0, "constante de Faraday (charge globale d'une mole de charges élémentaires).", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_G_", 0, "constante de gravitation universelle. Force=_G_*m1*m2/r^2", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_H", 0, "Henry", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Hz", 0, "Hertz", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_J", 0, "Energie en Joule=kg*m^2/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_K", 0, "Temperature en Kelvin", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Kcal", 0, "Energie en kilo-calorier", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_MeV", 0, "Energie en mega-electron-Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_N", 0, "Force en Newton=kg*m/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_NA_", 0, "Avogadro", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Ohm", 0, "Resistance electrique en Ohm", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_PSun_", 0, "puissance du Soleil", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Pa", 0, "Pression en Pascal=kg/m/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_REarth_", 0, "Rayon de la Terre", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_RSun_", 0, "rayon du Soleil", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_R_", 0, "constante des gaz (de Boltzmann par mole)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_S", 0, "", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_StdP_", 0, "Pression standard (au niveau de la mer)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_StdT_", 0, "temperature standard (0 degre Celsius exprimes en Kelvins)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Sv", 0, "Radioactivite: Sievert", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_T", 0, "Tesla", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_V", 0, "Tension electrique en Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Vm_", 0, "Volume molaire", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_W", 0, "Puissance en Watt=kg*m^2/s^3", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Wb", 0, "Weber", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_alpha_", 0, "constante de structure fine", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_c_", 0, "vitesse de la lumiere", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_cd", 0, "Luminosite en candela", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_cdf", "_cdf", "Suffixe pour obtenir une distribution cumulee. Taper F2 pour la distribution cumulee inverse.", "#_icdf", 0, CAT_CATEGORY_PROBA|XCAS_ONLY},
    {"_d", 0, "Temps: jour", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_deg", 0, "Angle en degres", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_eV", 0, "Energie en electron-Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_epsilon0_", 0, "permittivite du vide", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_ft", 0, "Longueur en pieds", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_g_", 0, "gravite au sol", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_grad", 0, "Angle en grades", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_h", 0, "Heure", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_h_", 0, "constante de Planck", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_ha", 0, "Aire en hectare", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_hbar_", 0, "constante de Planck/(2*pi)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_inch", 0, "Longueur en pouces", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_kWh", 0, "Energie en kWh", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_k_", 0, "constante de Boltzmann", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_kg", 0, "Masse en kilogramme", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_l", 0, "Volume en litre", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_m", 0, "Longueur en metre", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mEarth_", 0, "masse de la Terre", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_m^2", 0, "Aire en m^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_m^3", 0, "Volume en m^3", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_me_", 0, "masse electron", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_miUS", 0, "Longueur en miles US", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mn", 0, "Temps: minute", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mp_", 0, "masse proton", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_mpme_", 0, "ratio de masse proton/electron", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_mu0_", 0, "permeabilite du vide", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_phi_", 0, "quantum flux magnetique", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_plot", "_plot", "Suffixe pour obtenir le graphe d'une regression.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];polynomial_regression_plot(X,Y,2);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS| XCAS_ONLY},
    {"_qe_", 0, "charge de l'electron", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_qme_", 0, "_q_/_me_", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_rad", 0, "Angle en radians", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_rem", 0, "Radioactivite: rem", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_s", 0, "Temps: seconde", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_sd_", 0, "Jour sideral", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_syr_", 0, "Annee siderale", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_tr", 0, "Angle en tours", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_yd", 0, "Longueur en yards", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"a and b", " and ", "Et logique", 0, 0, CAT_CATEGORY_PROGCMD},
    {"a or b", " or ", "Ou logique", 0, 0, CAT_CATEGORY_PROGCMD},
    {"abcuv(a,b,c)", 0, "Cherche 2 polynomes u,v tels que a*u+b*v=c","x+1,x^2-2,x", 0, CAT_CATEGORY_POLYNOMIAL| XCAS_ONLY},
    {"abs(x)", 0, "Valeur absolue, module ou norme de x", "-3", "[1,2,3]", CAT_CATEGORY_COMPLEXNUM | (CAT_CATEGORY_REAL<<8)},
    {"add(u,v)", 0, "En Python, additionne des listes ou listes de listes u et v comme des vecteurs ou matrices.","[1,2,3],[0,1,3]", "[[1,2]],[[3,4]]", CAT_CATEGORY_LINALG},
    {"append", 0, "Ajoute un element en fin de liste l","#l.append(x)", 0, CAT_CATEGORY_LIST},
    {"approx(x)", 0, "Valeur approchee de x. Raccourci S-D", "pi", 0, CAT_CATEGORY_REAL| XCAS_ONLY},
    {"arg(z)", 0, "Argument du complexe z.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM | XCAS_ONLY},
    {"asc(string)", 0, "Liste des codes ASCII d'une chaine", "\"Bonjour\"", 0, CAT_CATEGORY_ARIT},
    {"assume(hyp)", 0, "Hypothese sur une variable.", "x>1", "x>-1 and x<1", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_SOFUS<<8) | XCAS_ONLY},
    {"avance n", "avance ", "La tortue avance de n pas, par defaut n=10", "#avance 40", 0, CAT_CATEGORY_LOGO},
    {"axes", "axes", "Axes visibles ou non axes=1 ou 0", "#axes=0", "#axes=1", CAT_CATEGORY_PROGCMD << 8|XCAS_ONLY},
    {"baisse_crayon ", "baisse_crayon ", "La tortue se deplace en marquant son passage.", 0, 0, CAT_CATEGORY_LOGO},
    {"barplot(list)", 0, "Diagramme en batons d'une serie statistique 1d.", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | (CAT_CATEGORY_PLOT<<8)},
    {"binomial(n,p,k)", 0, "binomial(n,p,k) probabilite de k succes avec n essais ou p est la proba de succes d'un essai. binomial_cdf(n,p,k) est la probabilite d'obtenir au plus k succes avec n essais. binomial_icdf(n,p,t) renvoie le plus petit k tel que binomial_cdf(n,p,k)>=t", "10,.5,4", 0, CAT_CATEGORY_PROBA | XCAS_ONLY},
    {"bitxor", "bitxor", "Ou exclusif", "#bitxor(1,2)", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"black", "black", "Option d'affichage", "#display=black", 0, CAT_CATEGORY_PROGCMD},
    {"blue", "blue", "Option d'affichage", "#display=blue", 0, CAT_CATEGORY_PROGCMD},
    {"caseval", "caseval", "Evalue une chaine de caractere en appelant le CAS.", "caseval(\"limit(sin(x)/x,x=0)\")", "caseval(\"factor(x^10-1)\")", CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_CALCULUS <<8)},
    {"cache_tortue ", "cache_tortue ", "Cache la tortue apres avoir trace le dessin.", 0, 0, CAT_CATEGORY_LOGO},
    {"camembert(list)", 0, "Diagramme en camembert d'une serie statistique 1d.", "[[\"France\",6],[\"Allemagne\",12],[\"Suisse\",5]]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"ceil(x)", 0, "Partie entiere superieure", "1.2", 0, CAT_CATEGORY_REAL},
    {"cercle(centre,rayon)", 0, "Cercle donne par centre et rayon ou par un diametre", "2+i,3", "1-i,1+i", CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"cfactor(p)", 0, "Factorisation sur C.", "x^4-1", 0, CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_COMPLEXNUM << 8) | XCAS_ONLY},
    {"char(liste)", 0, "Chaine donnee par une liste de code ASCII", "[97,98,99]", 0, CAT_CATEGORY_ARIT},
    {"charpoly(M,x)", 0, "Polynome caracteristique de la matrice M en la variable x.", "[[1,2],[3,4]],x", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"clearscreen()", "clearscreen()", "Efface l'ecran.", 0, 0, CAT_CATEGORY_PROGCMD|XCAS_ONLY},
    {"coeff(p,x,n)", 0, "Coefficient de x^n dans le polynome p.", "(1+x)^6,x,3", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"comb(n,k)", 0, "Renvoie k parmi n.", "10,4", 0, CAT_CATEGORY_PROBA | XCAS_ONLY},
    {"cond(A,[1,2,inf])", 0, "Nombre de condition d'une matrice par rapport a la norme specifiee (par defaut 1)", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"conj(z)", 0, "Conjugue complexe de z.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
    {"correlation(l1,l2)", 0, "Correlation listes l1 et l2", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"covariance(l1,l2)", 0, "Covariance listes l1 et l2", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"cpartfrac(p,x)", 0, "Decomposition en elements simples sur C.", "1/(x^4-1)", 0, CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_COMPLEXNUM << 8) | XCAS_ONLY},
    {"crayon ", "crayon ", "Couleur de trace de la tortue", "#crayon rouge", 0, CAT_CATEGORY_LOGO},
    {"cross(u,v)", 0, "Produit vectoriel de u et v.","[1,2,3],[0,1,3]", 0, CAT_CATEGORY_LINALG},
    {"csolve(equation,x)", 0, "Resolution exacte dans C d'une equation en x (ou d'un systeme polynomial).","x^2+x+1=0", 0, CAT_CATEGORY_SOLVE | (CAT_CATEGORY_COMPLEXNUM << 8) | XCAS_ONLY},
    {"curl(u,vars)", 0, "Rotationnel du vecteur u.", "[2*x*y,x*z,y*z],[x,y,z]", 0, CAT_CATEGORY_LINALG | XCAS_ONLY},
    {"cyan", "cyan", "Option d'affichage", "#display=cyan", 0, CAT_CATEGORY_PROGCMD},
    {"debug(f(args))", 0, "Execute la fonction f en mode pas a pas.", 0, 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {"degree(p,x)", 0, "Degre du polynome p en x.", "x^4-1", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"denom(x)", 0, "Denominateur de l'expression x.", "3/4", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"desolve(equation,t,y)", 0, "Resolution exacte d'equation differentielle ou de systeme differentiel lineaire a coefficients constants.", "[y'+y=exp(x),y(0)=1]", "[y'=[[1,2],[2,1]]*y+[x,x+1],y(0)=[1,2]]", CAT_CATEGORY_SOLVE | (CAT_CATEGORY_CALCULUS << 8) | XCAS_ONLY},
    {"det(A)", 0, "Determinant de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"diff(f,var,[n])", 0, "Derivee de l'expression f par rapport a var (a l'ordre n, n=1 par defaut), par exemple diff(sin(x),x) ou diff(x^3,x,2). Pour deriver f par rapport a x, utiliser f' (raccourci F3). Pour le gradient de f, var est la liste des variables.", "sin(x),x", "sin(x^2),x,3", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"display", "display", "Option d'affichage", "#display=red", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"disque n", "disque ", "Cercle rempli tangent a la tortue, de rayon n. Utiliser disque n,theta pour remplir un morceau de camembert ou disque n,theta,segment pour remplir un segment de disque", "#disque 30", "#disque(30,90)", CAT_CATEGORY_LOGO},
    {"dot(a,b)", 0, "Produit scalaire de 2 vecteurs. Raccourci: *", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_LINALG},
    {"draw_arc(x1,y1,rx,ry,theta1,theta2,c)", 0, "Arc d'ellipse pixelise.", "100,100,60,80,0,pi,magenta", 0, CAT_CATEGORY_PROGCMD},
    {"draw_circle(x1,y1,r,c)", 0, "Cercle pixelise. Option filled pour le remplir.", "100,100,60,cyan+filled", 0, CAT_CATEGORY_PROGCMD},
    {"draw_line(x1,y1,x2,y2,c)", 0, "Droite pixelisee.", "100,50,300,200,blue", 0, CAT_CATEGORY_PROGCMD},
    {"draw_pixel(x,y,color)", 0, "Colorie le pixel x,y. Faire draw_pixel() pour synchroniser l'ecran.", 0, 0, CAT_CATEGORY_PROGCMD},
    {"draw_polygon([[x1,y1],...],c)", 0, "Polygone pixelise.", "[[100,50],[30,20],[60,70]],red+filled", 0, CAT_CATEGORY_PROGCMD},
    {"draw_rectangle(x,y,w,h,c)", 0, "Rectangle pixelise.", "100,50,30,20,red+filled", 0, CAT_CATEGORY_PROGCMD},
    {"draw_string(s,x,y,c)", 0, "Affiche la chaine s en x,y", "\"Bonjour\",80,60", 0, CAT_CATEGORY_PROGCMD},
    {"droite(equation)", 0, "Droite donnee par une equation ou 2 points", "y=2x+1", "1+i,2-i", CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"ecris ", "ecris ", "Ecrire a la position de la tortue", "#ecris \"coucou\"", 0, CAT_CATEGORY_LOGO},
    {"efface", "efface", "Remise a zero de la tortue", 0, 0, CAT_CATEGORY_LOGO | XCAS_ONLY},
    {"egcd(A,B)", 0, "Cherche des polynomes U,V,D tels que A*U+B*V=D=gcd(A,B)","x^2+3x+1,x^2-5x-1", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"eigenvals(A)", 0, "Valeurs propres de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"eigenvects(A)", 0, "Vecteurs propres de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
    {"elif (test)", "elif", "Tests en cascade", 0, 0, CAT_CATEGORY_PROG | XCAS_ONLY},
				     //{"end", "end", "Fin de bloc", 0, 0, CAT_CATEGORY_PROG},
    {"erf(x)", 0, "Fonction erreur en x.", "1.2", 0, CAT_CATEGORY_PROBA},
    {"erfc(x)", 0, "Fonction erreur complementaire en x.", "1.2", 0, CAT_CATEGORY_PROBA},
    {"euler(n)",0,"Indicatrice d'Euler: nombre d'entiers < n premiers avec n","25",0,CAT_CATEGORY_ARIT},
    {"eval(f)", 0, "Evalue f.", 0, 0, CAT_CATEGORY_PROGCMD},
    {"evalc(z)", 0, "Ecrit z=x+i*y.", "1/(1+i*sqrt(3))", 0, CAT_CATEGORY_COMPLEXNUM | XCAS_ONLY},
    {"exact(x)", 0, "Convertit x en rationnel. Raccourci shift S-D", "1.2", 0, CAT_CATEGORY_REAL | XCAS_ONLY},
    {"exp2trig(expr)", 0, "Conversion d'exponentielles complexes en sin/cos", "exp(i*x)", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"exponential_regression(Xlist,Ylist)", 0, "Regression exponentielle.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"exponential_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression exponentielle.", "#X,Y:=[1,2,3,4,5],[1,3,4,6,8];exponential_regression_plot(X,Y);", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"exponentiald(lambda,x)", 0, "Loi exponentielle de parametre lambda. exponentiald_cdf(lambda,x) probabilite que \"loi exponentielle <=x\" par ex. exponentiald_cdf(2,3). exponentiald_icdf(lambda,t) renvoie x tel que \"loi exponentielle <=x\" vaut t, par ex. exponentiald_icdf(2,0.95) ", "5.1,3.4", 0, CAT_CATEGORY_PROBA | XCAS_ONLY},
    {"extend", 0, "Concatene 2 listes. Attention en Xcas, ne pas utiliser + qui effectue l'addition de 2 vecteurs.","#l1.extend(l2)", 0, CAT_CATEGORY_LIST},
    {"factor(p,[x])", 0, "Factorisation du polynome p (utiliser ifactor pour un entier). Raccourci: p=>*", "x^4-1", "x^6+1,sqrt(3)", CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_POLYNOMIAL << 8) | XCAS_ONLY},
    {"filled", "filled", "Option d'affichage", 0, 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"float(x)", 0, "Convertit x en nombre approche (flottant).", "pi", 0, CAT_CATEGORY_REAL},
    {"floor(x)", 0, "Partie entiere de x", "pi", 0, CAT_CATEGORY_REAL},
    {"fonction f(x)", "fonction", "Definition de fonction (Xcas). Par exemple\nfonction f(x)\n local y;\ny:=x*x;\nreturn y;\nffonction", 0, 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {"from arit import *", "from arit import *", "Instruction pour utiliser les fonctions d'arithmetique entiere en Python", "#from arit import *", "#import arit", CAT_CATEGORY_ARIT},
    {"from cas import *", "from cas import *", "Permet d'utiliser le calcul formel depuis Python", "#from cas import *", "#import cas", CAT_CATEGORY_ALGEBRA|(CAT_CATEGORY_CALCULUS<<8)},
    {"from cmath import *", "from cmath import *", "Instruction pour utiliser les fonctions de maths sur les complexes (trigo, exponentielle, log, ...) en Python", "#from cmath import *;i=1j", "#import cmath", CAT_CATEGORY_COMPLEXNUM},
    {"from linalg import *", "from linalg import *", "Instruction pour utiliser les fonctions d'algebre lineaire en Python", "#from linalg import *;i=1j", "#import linalg", CAT_CATEGORY_LINALG | (CAT_CATEGORY_MATRIX<<8) | (CAT_CATEGORY_POLYNOMIAL<<16)},
    {"from numpy import *", "from numpy import *", "Instruction pour utiliser les fonctions sur les matrice en Python", "#from numpy import *;i=1j", "#import numpy", CAT_CATEGORY_LINALG | (CAT_CATEGORY_MATRIX <<8) | (CAT_CATEGORY_COMPLEXNUM << 16)},
    {"from math import *", "from math import *", "Instruction pour utiliser les fonctions de maths (trigo, exponentielle, log, ...) en Python", "#from math import *", "#import math", CAT_CATEGORY_REAL},
    {"from matplotl import *", "from matplotl import *", "Instruction pour utiliser les fonctions de trace en Python", "#from matplotl import *", "#import matplotl", CAT_CATEGORY_PROBA|(CAT_CATEGORY_PLOT <<8)|(CAT_CATEGORY_STATS<<16)},
    {"from random import *", "from random import *", "Instruction pour utiliser les fonctions aleatoires en Python", "#from random import *", "#import random", CAT_CATEGORY_PROBA},
    {"from turtle import *", "from turtle import *", "Instruction pour utiliser la tortue en Python", "#from turtle import *", "#import turtle", CAT_CATEGORY_LOGO},
    {"fsolve(equation,x=a[..b])", 0, "Resolution approchee de equation pour x dans l'intervalle a..b ou en partant de x=a.","cos(x)=x,x=0..1", "cos(x)-x,x=0.0", CAT_CATEGORY_SOLVE | XCAS_ONLY},
    {"gauss(q)", 0, "Reduction de Gauss d'une forme quadratique q", "x^2+x*y+x*z,[x,y,z]", "x^2+4*x*y,[]", CAT_CATEGORY_LINALG | XCAS_ONLY },
    {"gcd(a,b,...)", 0, "Plus grand commun diviseur. En Python ne fonctionne qu'avec des entiers. Voir iegcd ou egcd pour Bezout.", "23,13", "x^2-1,x^3-1", CAT_CATEGORY_ARIT | (CAT_CATEGORY_POLYNOMIAL << 8)},
    {"gl_x", "gl_x", "Reglage graphique X gl_x=xmin..xmax", "#gl_x=0..2", 0, CAT_CATEGORY_PROGCMD << 8 | XCAS_ONLY},
    {"gl_y", "gl_y", "Reglage graphique Y gl_y=ymin..ymax", "#gl_y=-1..1", 0, CAT_CATEGORY_PROGCMD << 8 | XCAS_ONLY},
    {"green", "green", "Option d'affichage", "#display=green", 0, CAT_CATEGORY_PROGCMD},
    {"halftan(expr)", 0, "Exprime cos, sin, tan avec tan(angle/2).","cos(x)", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"hermite(n)", 0, "n-ieme polynome de Hermite", "10", "10,t", CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"hilbert(n)", 0, "Matrice de Hilbert de taille n.", "4", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"histogram(list,min,size)", 0, "Histogramme d'une liste de donneees, classes commencant a min de taille size.","ranv(100,uniformd,0,1),0,0.1", 0, CAT_CATEGORY_STATS | (CAT_CATEGORY_PLOT<<8)},
    {"iabcuv(a,b,c)", 0, "Cherche 2 entiers u,v tels que a*u+b*v=c","23,13,15", 0, CAT_CATEGORY_ARIT | XCAS_ONLY},
    {"ichinrem([a,m],[b,n])", 0,"Restes chinois entiers de a mod m et b mod n.", "[3,13],[2,7]", 0, CAT_CATEGORY_ARIT | XCAS_ONLY},
    {"idivis(n)", 0, "Liste des diviseurs d'un entier n.", "10", 0, CAT_CATEGORY_ARIT},
    {"idn(n)", 0, "matrice identite n * n", "4", 0, CAT_CATEGORY_MATRIX},
    {"iegcd(a,b)", 0, "Determine les entiers u,v,d tels que a*u+b*v=d=gcd(a,b)","23,13", 0, CAT_CATEGORY_ARIT},
    {"ifactor(n)", 0, "Factorisation d'un entier (pas trop grand!). Raccourci n=>*", "1234", 0, CAT_CATEGORY_ARIT},
    {"ilaplace(f,s,x)", 0, "Transformee inverse de Laplace de f", "s/(s^2+1),s,x", 0, CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"im(z)", 0, "Partie imaginaire (z.im en Python)", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
    {"inf", "inf", "Plus l'infini. Utiliser -inf pour moins l'infini ou infinity pour l'infini complexe. Raccourci shift INS.", "-inf", "infinity", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"input()", "input()", "Lire une chaine au clavier", "\"Valeur ?\"", 0, CAT_CATEGORY_PROG},
    {"integrate(f,x,[,a,b])", 0, "Primitive de f par rapport a la variable x, par ex. integrate(x*sin(x),x). Pour calculer une integrale definie, entrer les arguments optionnels a et b, par ex. integrate(x*sin(x),x,0,pi). Raccourci SHIFT F3.", "x*sin(x),x", "cos(x)/(1+x^4),x,0,inf", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"interp(X,Y[,interp])", 0, "Interpolation de Lagrange aux points (xi,yi) avec X la liste des xi et Y des yi. Renvoie la liste des differences divisees si interp est passe en parametre.", "[1,2,3,4,5],[0,1,3,4,4]", "[1,2,3,4,5],[0,1,3,4,4],interp", CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"inv(A)", 0, "Inverse de A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX|(CAT_CATEGORY_LINALG<<8)},
    {"inverser(v)", "inverser ", "La variable v est remplacee par son inverse", "#v:=3; inverser v", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
    {"iquo(a,b)", 0, "Quotient euclidien de deux entiers.", "23,13", 0, CAT_CATEGORY_ARIT | XCAS_ONLY},
    {"irem(a,b)", 0,"Reste euclidien de deux entiers", "23,13", 0, CAT_CATEGORY_ARIT | XCAS_ONLY},
    {"isprime(n)", 0, "Renvoie 1 si n est premier, 0 sinon.", "11", "10", CAT_CATEGORY_ARIT},
    {"jordan(A)", 0, "Forme normale de Jordan de la matrice A, renvoie P et D tels que P^-1*A*P=D", "[[1,2],[3,4]]", "[[1,1,-1,2,-1],[2,0,1,-4,-1],[0,1,1,1,1],[0,1,2,0,1],[0,0,-3,3,-1]]", CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"laguerre(n,a,x)", 0, "n-ieme polynome de Laguerre (a=0 par defaut).", "10", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"laplace(f,x,s)", 0, "Transformee de Laplace de f","sin(x),x,s", 0, CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"lcm(a,b,...)", 0, "Plus petit commun multiple.", "23,13", "x^2-1,x^3-1", CAT_CATEGORY_ARIT | (CAT_CATEGORY_POLYNOMIAL << 8) | XCAS_ONLY},
    {"lcoeff(p,x)", 0, "Coefficient dominant du polynome p.", "x^4-1", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"legendre(n)", 0, "n-ieme polynome de Legendre.", "10", "10,t", CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
#ifdef RELEASE
    {"len(l)", 0, "Taille d'une liste.", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_LIST},
#endif
    {"leve_crayon ", "leve_crayon ", "La tortue se deplace sans marquer son passage", 0, 0, CAT_CATEGORY_LOGO},
    {"limit(f,x=a)", 0, "Limite de f en x = a. Ajouter 1 ou -1 pour une limite a droite ou a gauche, limit(sin(x)/x,x=0) ou limit(abs(x)/x,x=0,1). Raccourci: SHIFT MIXEDFRAC", "sin(x)/x,x=0", "exp(-1/x),x=0,1", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"line_width_", "line_width_", "Prefixe d'epaisseur (2 a 8)", 0, 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"linear_regression(Xlist,Ylist)", 0, "Regression lineaire.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"linear_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression lineaire.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];linear_regression_plot(X,Y);", 0, CAT_CATEGORY_STATS | (CAT_CATEGORY_PLOT<<8)},
    {"linetan(expr,x,x0)", 0, "Tangente au graphe en x=x0.", "sin(x),x,pi/2", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"linsolve([eq1,eq2,..],[x,y,..])", 0, "Resolution de systeme lineaire. Peut utiliser le resultat de lu pour resolution en O(n^2).","[x+y=1,x-y=2],[x,y]", "#p,l,u:=lu([[1,2],[3,4]]); linsolve(p,l,u,[5,6])", CAT_CATEGORY_SOLVE | (CAT_CATEGORY_LINALG <<8) | (CAT_CATEGORY_MATRIX << 16) | XCAS_ONLY},
    {"logarithmic_regression(Xlist,Ylist)", 0, "Regression logarithmique.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"logarithmic_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression logarithmique.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];logarithmic_regression_plot(X,Y);", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"lu(A)", 0, "decomposition LU de la matrice A, P*A=L*U, renvoie P permutation, L et U triangulaires inferieure et superieure", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"magenta", "magenta", "Option d'affichage", "#display=magenta", 0, CAT_CATEGORY_PROGCMD},
    {"map(f,l)", 0, "Applique f aux elements de la liste l.","lambda x:x*x,[1,2,3]", 0, CAT_CATEGORY_LIST},
    {"matpow(A,n)", 0, "Renvoie A^n, la matrice A la puissance n", "[[1,2],[3,4]],n","#assume(n>=1);matpow([[0,2],[0,4]],n)", CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"matrix(l,c,func)", 0, "Matrice de terme general donne.", "2,3,(j,k)->j^k", 0, CAT_CATEGORY_MATRIX},
    {"mean(l)", 0, "Moyenne arithmetique liste l", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"median(l)", 0, "Mediane", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"montre_tortue ", "montre_tortue ", "Affiche la tortue", 0, 0, CAT_CATEGORY_LOGO},
    {"mul(A,B)", 0, "En Python, multiplie des listes de listes u et v comme des matrices.","[[1,2],[3,4]],[5,6]", "[[1,2],[3,4]].[[5,6],[7,8]]", CAT_CATEGORY_LINALG},
    {"mult_c_conjugate", 0, "Multiplier par le conjugue complexe.", "1+2*i", 0,  (CAT_CATEGORY_COMPLEXNUM << 8) | XCAS_ONLY},
    {"mult_conjugate", 0, "Multiplier par le conjugue (sqrt).", "sqrt(2)-sqrt(3)", 0, CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"normald([mu,sigma],x)", 0, "Loi normale, par defaut mu=0 et sigma=1. normald_cdf([mu,sigma],x) probabilite que \"loi normale <=x\" par ex. normald_cdf(1.96). normald_icdf([mu,sigma],t) renvoie x tel que \"loi normale <=x\" vaut t, par ex. normald_icdf(0.975) ", "1.2", 0, CAT_CATEGORY_PROBA | XCAS_ONLY},
    {"not(x)", 0, "Non logique.", 0, 0, CAT_CATEGORY_PROGCMD},
    {"numer(x)", 0, "Numerateur de x.", "3/4", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"odesolve(f(t,y),[t,y],[t0,y0],t1)", 0, "Solution approchee d'equation differentielle y'=f(t,y) et y(t0)=y0, valeur en t1 (ajouter curve pour les valeurs intermediaires de y)", "sin(t*y),[t,y],[0,1],2", "0..pi,(t,v)->{[-v[1],v[0]]},[0,1]", CAT_CATEGORY_SOLVE | XCAS_ONLY},
    {"partfrac(p,x)", 0, "Decomposition en elements simples. Raccourci p=>+", "1/(x^4-1)", 0, CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"pas_de_cote n", "pas_de_cote ", "Saut lateral de la tortue, par defaut n=10", "#pas_de_cote 30", 0, CAT_CATEGORY_LOGO},
    {"plot(expr,x)", 0, "Xcas: graphe de fonction, par exemple plot(sin(x)), plot(ln(x),x.0,5). Python et Xcas: plot(Xlist,Ylist) ligne polygonale", "[1,2,3,4,5,6],[2,3,5,2,1,4]","ln(x),x=0..5,xstep=0.1", CAT_CATEGORY_PLOT},
    {"plotarea(expr,x=a..b,[n,meth])", 0, "Aire sous la courbe selon une methode d'integration.", "1/x,x=1..5,4,rectangle_gauche", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotcontour(expr,[x=xm..xM,y=ym..yM],niveaux)", 0, "Lignes de niveau de expr.", "x^2+2y^2, [x=-2..2,y=-2..2],[1,2]", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotdensity(expr,[x=xm..xM,y=ym..yM])", 0, "Representation en niveaux de couleurs d'une expression de 2 variables.", "x^2-y^2,[x=-3..3,y=-2..2]", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotfield(f(t,y), [t=tmin..tmax,y=ymin..ymax])", 0, "Champ des tangentes de y'=f(t,y), optionnellement graphe avec plotode=[t0,y0]", "sin(t*y), [t=-3..3,y=-3..3],plotode=[0,1]", "5*[-y,x], [x=-1..1,y=-1..1]", CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotlist(list)", 0, "Graphe d'une liste", "[3/2,2,1,1/2,3,2,3/2]", "[1,13],[2,10],[3,15],[4,16]", CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotode(f(t,y), [t=tmin..tmax,y],[t0,y0])", 0, "Graphe de solution d'equation differentielle y'=f(t,y), y(t0)=y0.", "sin(t*y),[t=-3..3,y],[0,1]", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotparam([x,y],t)", 0, "Graphe en parametriques. Par exemple plotparam([sin(3t),cos(2t)],t,0,pi) ou plotparam(exp(i*t),t,0,pi)", "[sin(3t),cos(2t)],t,0,pi", "[t^2,t^3],t=-1..1,tstep=0.1", CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotpolar(r,theta)", 0, "Graphe en polaire.","cos(3*x),x,0,pi", "1/(1+cos(x)),x=0..pi,xstep=0.05", CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotseq(f(x),x=[u0,m,M],n)", 0, "Trace f(x) sur [m,M] et n termes de la suite recurrente u_{n+1}=f(u_n) de 1er terme u0.","sqrt(2+x),x=[6,0,7],5", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plus_point", "plus_point", "Option d'affichage", "#display=blue+plus_point", 0, CAT_CATEGORY_PROGCMD  | XCAS_ONLY},
    {"point(x,y)", 0, "Point", "1,2", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"polygon(list)", 0, "Polygone ferme.", "1-i,2+i,3", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"polygonscatterplot(Xlist,Ylist)", 0, "Nuage de points relies.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"polynomial_regression(Xlist,Ylist,n)", 0, "Regression polynomiale de degre <= n.", "[1,2,3,4,5],[0,1,3,4,4],2", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"polynomial_regression_plot(Xlist,Ylist,n)", 0, "Graphe d'une regression polynomiale de degre <= n.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];polynomial_regression_plot(X,Y,2);scatterplot(X,Y);", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"pour (boucle Xcas)", "pour  de  to  faire  fpour;", "Boucle definie.","#pour j de 1 to 10 faire print(j,j^2); fpour;", 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {"power_regression(Xlist,Ylist,n)", 0, "Regression puissance.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"power_regression_plot(Xlist,Ylist,n)", 0, "Graphe d'une regression puissance.", "#X,Y:=[1,2,3,4,5],[1,1,3,4,4];power_regression_plot(X,Y);", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"pow(a,n,p)", 0, "Renvoie a^n mod p","123,456,789", 0, CAT_CATEGORY_ARIT},
    {"powmod(a,n,p[,P,x])", 0, "Renvoie a^n mod p, ou a^n mod un entier p et un polynome P en x.","123,456,789", "x+1,452,19,x^4+x+1,x", CAT_CATEGORY_ARIT | XCAS_ONLY},
    {"print(expr)", 0, "Afficher dans la console", 0, 0, CAT_CATEGORY_PROG},
    {"pcoeff(p)", 0, "Polynome unitaire dont on donne la liste des racines (fonction reciproque de proot)", "[1,2,3]", 0, CAT_CATEGORY_POLYNOMIAL},
    {"peval(p,x)", 0, "Valeur d'un polynome en un point", "[1,2,3],4", 0, CAT_CATEGORY_POLYNOMIAL},
    {"proot(p)", 0, "Racines reelles et complexes approchees d'un polynome. Exemple proot([1,2.1,3,4.2]) ou proot(x^3+2.1*x^2+3x+4.2)", "[1.,2.1,3,4.2]","x^3+2.1*x^2+3x+4.2", CAT_CATEGORY_POLYNOMIAL|(CAT_CATEGORY_SOLVE<<8)},
    {"purge(x)", 0, "Purge le contenu de la variable x. Raccourci SHIFT-FORMAT", 0, 0, CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_SOFUS<<8) | XCAS_ONLY},
    {"python(f)", 0, "Affiche la fonction f en syntaxe Python.", 0, 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"python_compat(0|1|2|4)", 0, "python_compat(0) syntaxe Xcas, python_compat(1) syntaxe Python avec ^ interprete comme puissance, python_compat(2) ^ interprete comme ou exclusif bit a bit", "0", "1", CAT_CATEGORY_PROG | XCAS_ONLY},
    {"qr(A)", 0, "Factorisation A=Q*R avec Q orthogonale et R triangulaire superieure", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"quartile1(l)", 0, "1er quartile", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"quartile3(l)", 0, "3eme quartile", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"quo(p,q,x)", 0, "Quotient de division euclidienne polynomiale en x.", 0, 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"quote(x)", 0, "Renvoie l'expression x non evaluee.", 0, 0, CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"rand()", "rand()", "Reel aleatoire entre 0 et 1", 0, 0, CAT_CATEGORY_PROBA},
    {"randint(a,b)", 0, "Entier aleatoire entre a et b. En Xcas, avec un seul argument n, entier entre 1 et n.", "5,20", "6", CAT_CATEGORY_PROBA},
    {"ranm(n,m,[loi,parametres])", 0, "Matrice aleatoire a coefficients entiers ou selon une loi de probabilites (ranv pour un vecteur). Exemples ranm(2,3), ranm(3,2,binomial,20,.3), ranm(4,2,normald,0,1)", "3,3","4,2,normald,0,1",  CAT_CATEGORY_MATRIX},
    {"ranv(n,[loi,parametres])", 0, "Vecteur aleatoire", "4,normald,0,1", "10,30", CAT_CATEGORY_LINALG},
    {"ratnormal(x)", 0, "Ecrit sous forme d'une fraction irreductible.", "(x+1)/(x^2-1)^2", 0, CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"re(z)", 0, "Partie reelle (z.re en Python)", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
    {"read(\"filename\")", "read(\"", "Lire un fichier. Voir aussi write", 0, 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"rectangle_plein a,b", "rectangle_plein ", "Rectangle direct rempli depuis la tortue de cotes a et b (si b est omis, la tortue remplit un carre)", "#rectangle_plein 30", "#rectangle_plein(20,40)", CAT_CATEGORY_LOGO | XCAS_ONLY},
    {"recule n", "recule ", "La tortue recule de n pas, par defaut n=10", "#recule 30", 0, CAT_CATEGORY_LOGO},
    {"red", "red", "Option d'affichage", "#display=red", 0, CAT_CATEGORY_PROGCMD},
    {"rem(p,q,x)", 0, "Reste de division euclidienne polynomiale en x", 0, 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"repete(n,...)", "repete( ", "Repete plusieurs fois les instructions", "#repete(4,avance,tourne_gauche)", 0, CAT_CATEGORY_LOGO | XCAS_ONLY},
#ifdef RELEASE
    {"residue(f(z),z,z0)", 0, "Residu de l'expression en z0.", "1/(x^2+1),x,i", 0, CAT_CATEGORY_COMPLEXNUM | XCAS_ONLY},
#endif
    {"resultant(p,q,x)", 0, "Resultant en x des polynomes p et q.", "#P:=x^3+p*x+q;resultant(P,P',x);", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"revert(p[,x])", 0, "Developpement de Taylor reciproque, p doit etre nul en 0","x+x^2+x^4", 0, CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"rgb(r,g,b)", 0, "couleur definie par niveau de rouge, vert, bleu entre 0 et 255", "255,0,255", 0, CAT_CATEGORY_PROGCMD},
    {"rhombus_point", "rhombus_point", "Option d'affichage", "#display=magenta+rhombus_point", 0, CAT_CATEGORY_PROGCMD  | XCAS_ONLY},
    {"rond n", "rond ", "Cercle tangent a la tortue de rayon n. Utiliser rond n,theta pour un arc de cercle.", "#rond 30", "#rond(30,90)", CAT_CATEGORY_LOGO},
    {"rref(M)","rref","Reduction d'une matrice par le pivot de Gauss.","[[1,2,3],[4,5,6]]",0,CAT_CATEGORY_MATRIX|(CAT_CATEGORY_LINALG<<8)},
    {"rsolve(equation,u(n),[init])", 0, "Expression d'une suite donnee par une recurrence.","u(n+1)=2*u(n)+3,u(n),u(0)=1", "([u(n+1)=3*v(n)+u(n),v(n+1)=v(n)+u(n)],[u(n),v(n)],[u(0)=1,v(0)=2]", CAT_CATEGORY_SOLVE | XCAS_ONLY},
    {"saute n", "saute ", "La tortue fait un saut de n pas, par defaut n=10", "#saute 30", 0, CAT_CATEGORY_LOGO},
    {"scatterplot(Xlist,Ylist)", 0, "Nuage de points (scatter en Python)", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS| (CAT_CATEGORY_PLOT<<8)},
    {"segment(A,B)", 0, "Segment", "1,2+i", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"seq(expr,var,a,b[,pas])", 0, "Liste de terme general donne.","j^2,j,1,10", "j^2,j,1,10,2", CAT_CATEGORY_LIST | XCAS_ONLY},
    {"si (test Xcas)", "si  alors  sinon  fsi;", "Test.", "#f(x):=si x>0 alors x; sinon -x; fsi;", 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {"sign(x)", 0, "Renvoie -1 si x est negatif, 0 si x est nul et 1 si x est positif.", 0, 0, CAT_CATEGORY_REAL | XCAS_ONLY},
    {"simplify(expr)", 0, "Renvoie en general expr sous forme simplifiee. Raccourci expr=>/", "sin(3x)/sin(x)", "ln(4)-ln(2)", CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"sin_regression(Xlist,Ylist)", 0, "Regression trigonometrique.", "[1,2,3,4,5,6,7,8,9,10,11,12,13,14],[0.1,0.5,0.8,1,0.7,0.5,0.05,-.5,-.75,-1,-.7,-.4,0.1,.5]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"sin_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression trigonometrique.", "#X,Y:=[1,2,3,4,5,6,7,8,9,10,11,12,13,14],[0.1,0.5,0.8,1,0.7,0.5,0.05,-.5,-.75,-1,-.7,-.4,0.1,.5];sin_regression_plot(X,Y);", 0, CAT_CATEGORY_STATS  | XCAS_ONLY},
    {"solve()", 0, "Xcas: solve(equation,x) resolution exacte d'une equation en x (ou d'un systeme polynomial). Utiliser csolve pour les solutions complexes, linsolve pour un systeme lineaire. Python et Xcas: solve(A,b) resolution d'un systeme de Cramer A*x=b", "x^2-x-1=0,x", "[x^2-y^2=0,x^2-z^2=0],[x,y,z]", CAT_CATEGORY_SOLVE},
    {"sorted(l)", 0, "Trie une liste.","[3/2,2,1,1/2,3,2,3/2]", "[[1,2],[2,3],[4,3]],(x,y)->when(x[1]==y[1],x[0]>y[0],x[1]>y[1]", CAT_CATEGORY_LIST},
    {"square_point", "square_point", "Option d'affichage", "#display=cyan+square_point", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY },
    {"star_point", "star_point", "Option d'affichage", "#display=magenta+star_point", 0, CAT_CATEGORY_PROGCMD  | XCAS_ONLY},
    {"stddev(l)", 0, "Ecart-type d'une liste l", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"sub(u,v)", 0, "En Python, soustrait des listes ou listes de listes u et v comme des vecteurs ou matrices.","[1,2,3],[0,1,3]", "[[1,2]],[[3,4]]", CAT_CATEGORY_LINALG},
    {"subst(a,b=c)", 0, "Remplace b par c dans a. Raccourci a(b=c). Pour faire plusieurs remplacements, saisir subst(expr,[b1,b2...],[c1,c2...])", "x^2,x=3", "x+y^2,[x,y],[1,2]", CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"sum(f,k,m,M)", 0, "Somme de l'expression f dependant de k pour k variant de m a M. Exemple sum(k^2,k,1,n)=>*. Raccourci ALPHA F3", "k,k,1,n", "k^2,k", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"svd(A)", 0, "Singular Value Decomposition, renvoie U orthogonale, S vecteur des valeurs singulières, Q orthogonale tels que A=U*diag(S)*tran(Q).", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"tabvar(f,[x=a..b])", 0, "Tableau de variations de l'expression f, avec arguments optionnels la variable x dans l'intervalle a..b.", "sqrt(x^2+x+1)", "[cos(2t),sin(3t)],t", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"tantque (boucle Xcas)", "tantque  faire  ftantque;", "Boucle indefinie.", "#j:=13; tantque j!=1 faire j:=ifte(even(j),j/2,3j+1); print(j); ftantque;", 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {"taylor(f,x=a,n,[polynom])", 0, "Developpement de Taylor de l'expression f en x=a a l'ordre n, ajouter le parametre polynom pour enlever le terme de reste.","sin(x),x=0,5", "sin(x),x=0,5,polynom", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"tchebyshev1(n)", 0, "Polynome de Tchebyshev de 1ere espece: cos(n*x)=T_n(cos(x))", "10", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"tchebyshev2(n)", 0, "Polynome de Tchebyshev de 2eme espece: sin((n+1)*x)=sin(x)*U_n(cos(x))", "10", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"tcollect(expr)", 0, "Linearisation trigonometrique et regroupement.","sin(x)+cos(x)", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"texpand(expr)", 0, "Developpe les fonctions trigonometriques, exp et ln.","sin(3x)", "ln(x*y)", CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"time(cmd)", 0, "Temps pour effectuer une commande ou mise a l'heure de horloge","int(1/(x^4+1),x)","8,0", CAT_CATEGORY_PROG},
    {"tlin(expr)", 0, "Linearisation trigonometrique de l'expression.","sin(x)^3", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"tourne_droite n", "tourne_droite ", "La tortue tourne de n degres, par defaut n=90", "#tourne_droite 45", 0, CAT_CATEGORY_LOGO},
    {"tourne_gauche n", "tourne_gauche ", "La tortue tourne de n degres, par defaut n=90", "#tourne_gauche 45", 0, CAT_CATEGORY_LOGO},
    {"trace(A)", 0, "Trace de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
    {"transpose(A)", 0, "Transposee de la matrice A. Pour la transconjuguee utiliser trn(A) ou A^*.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
    {"triangle_point", "triangle_point", "Option d'affichage", "#display=yellow+triangle_point", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"trig2exp(expr)", 0, "Convertit les fonctions trigonometriques en exponentielles.","cos(x)^3", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"trigcos(expr)", 0, "Exprime sin^2 et tan^2 avec cos^2.","sin(x)^4", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"trigsin(expr)", 0, "Exprime cos^2 et tan^2 avec sin^2.","cos(x)^4", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"trigtan(expr)", 0, "Exprime cos^2 et sin^2 avec tan^2.","cos(x)^4", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"uniformd(a,b,x)", 0, "loi uniforme sur [a,b] de densite 1/(b-a)", 0, 0, CAT_CATEGORY_PROBA | XCAS_ONLY},
    {"v augmente_de n", " augmente_de ", "La variable v augmente de n, ou de n %", "#v:=3; v augmente_de 1", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
    {"v diminue_de n", " diminue_de ", "La variable v diminue de n, ou de n %", "#v:=3; v diminue_de 1", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
    {"v est_divise_par n", " est_divise_par ", "La variable v est divisee par n", "#v:=3; v est_divise_par 2", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
    {"v est_eleve_puissance n", " est_eleve_puissance ", "La variable v est eleveee a la puissance n", "#v:=3; v est_eleve_puissance 2", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
    {"v est_multiplie_par n", " est_multiplie_par ", "La variable v est multipliee par n", "#v:=3; v est_multiplie_par 2", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
				     //{"version", "version()", "Khicas 1.5.0, (c) B. Parisse et al. www-fourier.ujf-grenoble.fr/~parisse. License GPL version 2. Interface adaptee d'Eigenmath pour Casio, G. Maia, http://gbl08ma.com", 0, 0, CAT_CATEGORY_PROGCMD},
    {"write(\"filename\",var)", "write(\"", "Sauvegarde une ou plusieurs variables dans un fichier. Par exemple f(x):=x^2; write(\"func_f\",f).",  0, 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"yellow", "yellow", "Option d'affichage", "#display=yellow", 0, CAT_CATEGORY_PROGCMD},
    {"|", "|", "Ou logique", "#1|2", 0, CAT_CATEGORY_PROGCMD},
    {"~", "~", "Complement", "#~7", 0, CAT_CATEGORY_PROGCMD},
  };

const catalogFunc completeCaten[] = { // list of all functions (including some not in any category)
  {" loop for", "for ", "Defined loop.", "#\nfor ", 0, CAT_CATEGORY_PROG},
  {" loop in list", "for in", "Loop on all elements of a list.", "#\nfor in", 0, CAT_CATEGORY_PROG},
  {" loop while", "while ", "Undefined loop.", "#\nwhile ", 0, CAT_CATEGORY_PROG},
  {" test if", "if ", "Test", "#\nif ", 0, CAT_CATEGORY_PROG},
  {" test else", "else ", "Test false case", 0, 0, CAT_CATEGORY_PROG},
  {" function def", "f(x):=", "Definition of function.", "#\nf(x):=", 0, CAT_CATEGORY_PROG},
  {" local j,k;", "local ", "Local variables declaration (Xcas)", 0, 0, CAT_CATEGORY_PROG},
  {" range(a,b)", 0, "In range [a,b) (a included, b excluded)", "# in range(1,10)", 0, CAT_CATEGORY_PROG},
  {" return res", "return ", "Leaves current function and returns res.", 0, 0, CAT_CATEGORY_PROG},
  {" edit list ", "list ", "List creation wizzard.", 0, 0, CAT_CATEGORY_LIST},
  {" edit matrix ", "matrix ", "Matrix creation wizzard.", 0, 0, CAT_CATEGORY_MATRIX},
    {" mksa(x)", 0, "Conversion to MKSA units", 0, 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8) | XCAS_ONLY},
    {" ufactor(a,b)", 0, "Factorize unit b in a", "100_J,1_kW", 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8) | XCAS_ONLY},
    {" usimplify(a)", 0, "Simplify unit", "100_l/10_cm^2", 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8) | XCAS_ONLY},
  {"!", "!", "Logical not (prefix) or factorial of n (suffix).", "#7!", "~!b", CAT_CATEGORY_PROGCMD},
  {"#", "#", "Python comment, for Xcas comment type //. Shortcut ALPHA F2", 0, 0, CAT_CATEGORY_PROG},
  {"%", "%", "a % b means a modulo b", 0, 0, CAT_CATEGORY_ARIT | (CAT_CATEGORY_PROGCMD << 8)},
  {"&", "&", "Logical and or +", "#1&2", 0, CAT_CATEGORY_PROGCMD},
  {":=", ":=", "Set variable value. Shortcut SHIFT F1", "#a:=3", 0, CAT_CATEGORY_PROGCMD|(CAT_CATEGORY_SOFUS<<8)|XCAS_ONLY},
  {"<", "<", "Shortcut SHIFT F2", 0, 0, CAT_CATEGORY_PROGCMD},
  {"=>", "=>", "Store value in variable or conversion (touche ->). For example 5=>a or x^4-1=>* or (x+1)^2=>+ or sin(x)^2=>cos.", "#5=>a", "#15_ft=>_cm", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_PHYS <<8) | (CAT_CATEGORY_UNIT << 16) | XCAS_ONLY},
  {">", ">", "Shortcut F2.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"\\", "\\", "\\ char", 0, 0, CAT_CATEGORY_PROGCMD},
  {"_", "_", "_ char, shortcut (-).", 0, 0, CAT_CATEGORY_PROGCMD},
    {"_(km/h)", "_(km/h)", "Speed kilometer per hour", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m/s)", "_(m/s)", "Speed meter/second", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m/s^2)", "_(m/s^2)", "Acceleration", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m^2/s)", "_(m^2/s)", "Viscosity", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_A", 0, "Ampere", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Bq", 0, "Becquerel", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_C", 0, "Coulomb", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Ci", 0, "Curie", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_F", 0, "Farad", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_F_", 0, "Faraday constant", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_G_", 0, "Gravitation force=_G_*m1*m2/r^2", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_H", 0, "Henry", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Hz", 0, "Hertz", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_J", 0, "Joule=kg*m^2/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_K", 0, "Temperature in Kelvin", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Kcal", 0, "Energy kilo-calorie", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_MeV", 0, "Energy mega-electron-Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_N", 0, "Force Newton=kg*m/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_NA_", 0, "Avogadro constant", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Ohm", 0, "Ohm", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_PSun_", 0, "Sun power", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Pa", 0, "Pressure in Pascal=kg/m/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_REarth_", 0, "Earth radius", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_RSun_", 0, "Sun radius", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_R_", 0, "Boltzmann constant (per mol)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_S", 0, "", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_StdP_", 0, "Standard pressure", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_StdT_", 0, "Standard temperature (0 degre Celsius in Kelvins)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Sv", 0, "Sievert", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_T", 0, "Tesla", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_V", 0, "Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Vm_", 0, "Volume molaire", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_W", 0, "Watt=kg*m^2/s^3", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Wb", 0, "Weber", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_alpha_", 0, "fine structure constant", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_c_", 0, "speed of light", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_cd", 0, "candela", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
  {"_cdf", "_cdf", "Suffix to get a cumulative distribution function. Type F2 for inverse cumulative distribution function _icdf suffix.", "#_icdf", 0, CAT_CATEGORY_PROBA|XCAS_ONLY},
    {"_d", 0, "day", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_deg", 0, "degree", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_eV", 0, "electron-Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_epsilon0_", 0, "vacuum permittivity", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_ft", 0, "feet", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_g_", 0, "Earth gravity (ground)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_grad", 0, "grades (angle unit(", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_h", 0, "Hour", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_h_", 0, "Planck constant", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_ha", 0, "hectare", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_hbar_", 0, "Planck constant/(2*pi)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_inch", 0, "inches", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_kWh", 0, "kWh", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_k_", 0, "Boltzmann constant", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_kg", 0, "kilogram", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_l", 0, "liter", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_m", 0, "meter", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mEarth_", 0, "Earth mass", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_m^2", 0, "Area in m^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_m^3", 0, "Volume in m^3", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_me_", 0, "electron mass", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_miUS", 0, "US miles", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mn", 0, "minute", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mp_", 0, "proton mass", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_mpme_", 0, "proton/electron mass-ratio", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_mu0_", 0, "", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_phi_", 0, "magnetic flux quantum", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_plot", "_plot", "Suffix for a regression graph.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];polynomial_regression_plot(X,Y,2);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
    {"_qe_", 0, "electron charge", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_qme_", 0, "_q_/_me_", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_rad", 0, "radians", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_rem", 0, "rem", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_s", 0, "second", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_sd_", 0, "Sideral day", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_syr_", 0, "Siderale year", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_tr", 0, "tour (angle unit)", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_yd", 0, "yards", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
  {"a and b", " and ", "Logical and", 0, 0, CAT_CATEGORY_PROGCMD},
  {"a or b", " or ", "Logical or", 0, 0, CAT_CATEGORY_PROGCMD},
  {"abcuv(a,b,c)", 0, "Find 2 polynomial u,v such that a*u+b*v=c","x+1,x^2-2,x", 0, CAT_CATEGORY_POLYNOMIAL},
  {"abs(x)", 0, "Absolute value or norm of x x", "-3", "[1,2,3]", CAT_CATEGORY_COMPLEXNUM | (CAT_CATEGORY_REAL<<8)},
  {"append", 0, "Adds an element at the end of a list","#l.append(x)", 0, CAT_CATEGORY_LIST},
  {"approx(x)", 0, "Approx. value x. Shortcut S-D", "pi", 0, CAT_CATEGORY_REAL},
  {"arg(z)", 0, "Angle of complex z.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"asc(string)", 0, "List of ASCII codes os a string", "\"Hello\"", 0, CAT_CATEGORY_ARIT},
  {"assume(hyp)", 0, "Assumption on variable.", "x>1", "x>-1 and x<1", CAT_CATEGORY_PROGCMD|(CAT_CATEGORY_SOFUS<<8)},
  {"avance n", "avance ", "Turtle forward n steps, default n=10", "#avance 30", 0, CAT_CATEGORY_LOGO},
  {"axes", "axes", "Axes visible or not axes=1 or 0", "#axes=0", 0, CAT_CATEGORY_PROGCMD << 8|XCAS_ONLY},
  {"baisse_crayon ", "baisse_crayon ", "Turtle moves with the pen writing.", 0, 0, CAT_CATEGORY_LOGO},
  {"barplot(list)", 0, "Bar plot of 1-d statistic serie data in list.", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"binomial(n,p,k)", 0, "binomial(n,p,k) probability to get k success with n trials where p is the probability of success of 1 trial. binomial_cdf(n,p,k) is the probability to get at most k successes. binomial_icdf(n,p,t) returns the smallest k such that binomial_cdf(n,p,k)>=t", "10,.5,4", 0, CAT_CATEGORY_PROBA},
  {"bitxor", "bitxor", "Exclusive or", "#bitxor(1,2)", 0, CAT_CATEGORY_PROGCMD},
  {"black", "black", "Display option", "#display=black", 0, CAT_CATEGORY_PROGCMD},
  {"blue", "blue", "Display option", "#display=blue", 0, CAT_CATEGORY_PROGCMD},
  {"camembert(list)", 0, "Camembert pie-chart of a 1-d statistical serie.", "[[\"France\",6],[\"Germany\",12],[\"Switzerland\",5]]", 0, CAT_CATEGORY_STATS},
  {"cache_tortue ", "cache_tortue ", "Hide turtle (once the picture has been drawn).", 0, 0, CAT_CATEGORY_LOGO},
  {"ceil(x)", 0, "Smallest integer not less than x", "1.2", 0, CAT_CATEGORY_REAL},
  {"cfactor(p)", 0, "Factorization over C.", "x^4-1", 0, CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"char(liste)", 0, "Converts a list of ASCII codes to a string.", "[97,98,99]", 0, CAT_CATEGORY_ARIT},
  {"charpoly(M,x)", 0, "Characteristic polynomial of matrix M in variable x.", "[[1,2],[3,4]],x", 0, CAT_CATEGORY_MATRIX},
  {"circle(center,radius)", 0, "Circle", "2+i,3", "1-i,1+i", CAT_CATEGORY_PROGCMD},
  {"clearscreen()", "clearscreen()", "Clear screen.", 0, 0, CAT_CATEGORY_PROGCMD|XCAS_ONLY},
  {"coeff(p,x,n)", 0, "Coefficient of x^n in polynomial p.", 0, 0, CAT_CATEGORY_POLYNOMIAL},
  {"comb(n,k)", 0, "Returns nCk", "10,4", 0, CAT_CATEGORY_PROBA},
  {"cond(A,[1,2,inf])", 0, "Nombre de condition d'une matrice par rapport a la norme specifiee (par defaut 1)", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"conj(z)", 0, "Complex conjugate of z.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"correlation(l1,l2)", 0, "Correlation of lists l1 and l2", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"covariance(l1,l2)", 0, "Covariance of lists l1 and l2", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"cpartfrac(p,x)", 0, "Partial fraction decomposition over C.", "1/(x^4-1)", 0, CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"crayon ", "crayon ", "Turtle drawing color", "#crayon red", 0, CAT_CATEGORY_LOGO},
  {"cross(u,v)", 0, "Cross product of vectors u and v.","[1,2,3],[0,1,3]", 0, CAT_CATEGORY_LINALG},
  {"csolve(equation,x)", 0, "Solve equation (or polynomial system) in exact mode over the complex numbers.","x^2+x+1=0", 0, CAT_CATEGORY_SOLVE| (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"curl(u,vars)", 0, "Curl of vector u.", "[2*x*y,x*z,y*z],[x,y,z]", 0, CAT_CATEGORY_LINALG},
  {"cyan", "cyan", "Display option", "#display=cyan", 0, CAT_CATEGORY_PROGCMD},
  {"debug(f(args))", 0, "Runs user function f in step by step mode.", 0, 0, CAT_CATEGORY_PROG},
  {"degree(p,x)", 0, "Degre of polynomial p in x.", "x^4-1", 0, CAT_CATEGORY_POLYNOMIAL},
  {"denom(x)", 0, "Denominator of expression x.", "3/4", 0, CAT_CATEGORY_POLYNOMIAL},
  {"desolve(equation,t,y)", 0, "Exact differential equation solving.", "desolve([y'+y=exp(x),y(0)=1])", "[y'=[[1,2],[2,1]]*y+[x,x+1],y(0)=[1,2]]", CAT_CATEGORY_SOLVE | (CAT_CATEGORY_CALCULUS << 8)},
  {"det(A)", 0, "Determinant of matrix A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"diff(f,var,[n])", 0, "Derivative of expression f with respect to var (order n, n=1 by default), for example diff(sin(x),x) or diff(x^3,x,2). For derivation with respect to x, run f' (shortcut F3). For the gradient of f, var is the list of variables.", "sin(x),x", "sin(x^2),x,3", CAT_CATEGORY_CALCULUS},
  {"display", "display", "Display option", "#display=red", 0, CAT_CATEGORY_PROGCMD},
  {"disque n", "disque ", "Filled circle tangent to the turtle, radius n. Run disque n,theta for a filled arc of circle, theta in degrees, or disque n,theta,segment for a segment of circle.", "#disque 30", "#disque(30,90)", CAT_CATEGORY_LOGO},
  {"dot(a,b)", 0, "Dot product of 2 vectors. Shortcut: *", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_LINALG},
  {"draw_arc(x1,y1,rx,ry,theta1,theta2,c)", 0, "Pixelised arc of ellipse.", "100,100,60,80,0,pi,magenta", 0, CAT_CATEGORY_PROGCMD},
  {"draw_circle(x1,y1,r,c)", 0, "Pixelised circle. Option: filled", "100,100,60,cyan+filled", 0, CAT_CATEGORY_PROGCMD},
  {"draw_line(x1,y1,x2,y2,c)", 0, "Pixelised line.", "100,50,300,200,blue", 0, CAT_CATEGORY_PROGCMD},
  {"draw_pixel(x,y,color)", 0, "Colors pixel x,y. Run draw_pixel() to synchronise screen.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"draw_polygon([[x1,y1],...],c)", 0, "Pixelised polygon.", "[[100,50],[30,20],[60,70]],red+filled", 0, CAT_CATEGORY_PROGCMD},
  {"draw_rectangle(x,y,w,h,c)", 0, "Rectangle.", "100,50,30,20,red+filled", 0, CAT_CATEGORY_PROGCMD},
  {"draw_string(s,x,y,c)", 0, "Draw string s at pixel x,y", "\"Bonjour\",80,60", 0, CAT_CATEGORY_PROGCMD},
#ifndef TURTLETAB
  {"ecris ", "ecris ", "Write at turtle position", "#ecris \"hello\"", 0, CAT_CATEGORY_LOGO},
#endif
  {"efface", "efface", "Reset turtle", 0, 0, CAT_CATEGORY_LOGO},
  {"egcd(A,B)", 0, "Find polynomials U,V,D such that A*U+B*V=D=gcd(A,B)","x^2+3x+1,x^2-5x-1", 0, CAT_CATEGORY_POLYNOMIAL},
  {"elif test", "elif ", "Test cascade", 0, 0, CAT_CATEGORY_PROG},
  {"eigenvals(A)", 0, "Eigenvalues of matrix  A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX |XCAS_ONLY},
  {"eigenvects(A)", 0, "Eigenvectors of matrix A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"erf(x)", 0, "Error function of x.", "1.2", 0, CAT_CATEGORY_PROBA},
  {"erfc(x)", 0, "Complementary error function of x.", "1.2", 0, CAT_CATEGORY_PROBA},
  {"euler(n)",0,"Euler indicatrix: number of integers < n coprime with n","25",0,CAT_CATEGORY_ARIT},
  {"eval(f)", 0, "Evals f.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"evalc(z)", 0, "Write z=x+i*y.", "1/(1+i*sqrt(3))", 0, CAT_CATEGORY_COMPLEXNUM},
  {"exact(x)", 0, "Converts x to a rational. Shortcut shift S-D", "1.2", 0, CAT_CATEGORY_REAL},
  {"exp2trig(expr)", 0, "Convert complex exponentials to sin/cos", "exp(i*x)", 0, CAT_CATEGORY_TRIG},
  {"exponential_regression(Xlist,Ylist)", 0, "Exponential regression.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"exponential_regression_plot(Xlist,Ylist)", 0, "Exponential regression plot.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];exponential_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  {"exponentiald(lambda,x)", 0, "Exponential distribution law of  parameter lambda. exponentiald_cdf(lambda,x) probability that \"exponential distribution <=x\" e.g. exponentiald_cdf(2,3). exponentiald_icdf(lambda,t) returns x such that \"exponential distribution <=x\" has probability t, e.g, exponentiald_icdf(2,0.95) ", "5.1,3.4", 0, CAT_CATEGORY_PROBA},
  {"extend", 0, "Merge 2 lists. Note that + does not merge lists, it adds vectors","#l1.extend(l2)", 0, CAT_CATEGORY_LIST},
  {"factor(p,[x])", 0, "Factors polynomial p (run ifactor for an integer). Shortcut: p=>*", "x^4-1", "x^6+1,sqrt(3)", CAT_CATEGORY_ALGEBRA| (CAT_CATEGORY_POLYNOMIAL << 8)},
  {"filled", "filled", "Display option", 0, 0, CAT_CATEGORY_PROGCMD},
  {"float(x)", 0, "Converts x to a floating point value.", "pi", 0, CAT_CATEGORY_REAL},
  {"floor(x)", 0, "Largest integer not greater than x", "pi", 0, CAT_CATEGORY_REAL},
  {"fourier_an(f,x,T,n,a)", 0, "Cosine Fourier coefficients of f", "x^2,x,2*pi,n,-pi", 0, CAT_CATEGORY_CALCULUS},
  {"fourier_bn(f,x,T,n,a)", 0, "Sine Fourier coefficients of f", "x^2,x,2*pi,n,-pi", 0, CAT_CATEGORY_CALCULUS},
  {"fourier_cn(f,x,T,n,a)", 0, "Exponential Fourier coefficients of f", "x^2,x,2*pi,n,-pi", 0, CAT_CATEGORY_CALCULUS},
  {"from math/... import *", "from math import *", "Access to math or to random functions ([random]) or turtle with English commandnames [turtle]. Math import is not required in KhiCAS", "#from random import *", "#from turtle import *", CAT_CATEGORY_PROG},
  {"fsolve(equation,x=a..b)", 0, "Approx equation solving in interval a..b.","cos(x)=x,x=0..1", "cos(x)-x,x=0.0", CAT_CATEGORY_SOLVE},
  // {"function f(x):...", "function f(x) local y;   ffunction:;", "Function definition.", "#function f(x) local y; y:=x^2; return y; ffunction:;", 0, CAT_CATEGORY_PROG},
  {"gauss(q)", 0, "Quadratic form reduction", "x^2+x*y+x*z+y^2+z^2,[x,y,z]", 0, CAT_CATEGORY_LINALG},
  {"gcd(a,b,...)", 0, "Greatest common divisor. See also iegcd and egcd for extended GCD.", "23,13", "x^2-1,x^3-1", CAT_CATEGORY_ARIT | (CAT_CATEGORY_POLYNOMIAL << 8)},
  {"gl_x", "gl_x", "Display settings X gl_x=xmin..xmax", "#gl_x=0..2", 0, CAT_CATEGORY_PROGCMD},
  {"gl_y", "gl_y", "Display settings Y gl_y=ymin..ymax", "#gl_y=-1..1", 0, CAT_CATEGORY_PROGCMD},
  {"gramschmidt(M)", 0, "Gram-Schmidt orthonormalization (line vectors or linearly independant set of vectors)", "[[1,2,3],[4,5,6]]", "[1,1+x],(p,q)->integrate(p*q,x,-1,1)", CAT_CATEGORY_LINALG},
  {"green", "green", "Display option", "#display=green", 0, CAT_CATEGORY_PROGCMD},
  {"halftan(expr)", 0, "Convert cos, sin, tan with tan(angle/2).","cos(x)", 0, CAT_CATEGORY_TRIG},
  {"hermite(n)", 0, "n-th Hermite polynomial", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"hilbert(n)", 0, "Hilbert matrix of order n.", "4", 0, CAT_CATEGORY_MATRIX},
  {"histogram(list,min,size)", 0, "Histogram of data in list, classes begin at min of size size.","ranv(100,uniformd,0,1),0,0.1", 0, CAT_CATEGORY_STATS},
  {"iabcuv(a,b,c)", 0, "Find 2 integers u,v such that a*u+b*v=c","23,13,15", 0, CAT_CATEGORY_ARIT},
  {"ichinrem([a,m],[b,n])", 0,"Integer chinese remainder of a mod m and b mod n.", "[3,13],[2,7]", 0, CAT_CATEGORY_ARIT},
  {"idivis(n)", 0, "Returns the list of divisors of an integer n.", "10", 0, CAT_CATEGORY_ARIT},
  {"idn(n)", 0, "Identity matrix of order n", "4", 0, CAT_CATEGORY_MATRIX},
  {"iegcd(a,b)", 0, "Find integers u,v,d such that a*u+b*v=d=gcd(a,b)","23,13", 0, CAT_CATEGORY_ARIT},
  {"ifactor(n)", 0, "Factorization of an integer (not too large!). Shortcut n=>*", 0, 0, CAT_CATEGORY_ARIT},
  {"ilaplace(f,s,x)", 0, "Inverse Laplace transform of f", "s/(s^2+1),s,x", 0, CAT_CATEGORY_CALCULUS},
  {"im(z)", 0, "Imaginary part.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"inf", "inf", "Plus infinity. -inf for minus infinity and infinity for unsigned/complex infinity. Shortcut shift INS.", "oo", 0, CAT_CATEGORY_CALCULUS},
  {"input()", "input()", "Read a string from keyboard", 0, 0, CAT_CATEGORY_PROG},
  {"integrate(f,x,[a,b])", 0, "Antiderivative of f with respect to x, like integrate(x*sin(x),x). For definite integral enter optional arguments a and b, like integrate(x*sin(x),x,0,pi). Shortcut SHIFT F3.", "x*sin(x),x", "cos(x)/(1+x^4),x,0,inf", CAT_CATEGORY_CALCULUS},
  {"interp(X,Y)", 0, "Lagrange interpolation at points (xi,yi) where X is the list of xi and Y of yi. If interp is passed as 3rd argument, returns the divided differences list.", "[1,2,3,4,5],[0,1,3,4,4]", "[1,2,3,4,5],[0,1,3,4,4],interp", CAT_CATEGORY_POLYNOMIAL},
  {"inv(A)", 0, "Inverse of A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"iquo(a,b)", 0, "Integer quotient of a and b.", "23,13", 0, CAT_CATEGORY_ARIT},
  {"irem(a,b)", 0,"Integer remainder of a and b.", "23,13", 0, CAT_CATEGORY_ARIT},
  {"isprime(n)", 0, "Returns 1 if n is prime, 0 otherwise.", "11", "10", CAT_CATEGORY_ARIT},
  {"jordan(A)", 0, "Jordan normal form of matrix A, returns P and D such that P^-1*A*P=D", "[[1,2],[3,4]]", "[[1,1,-1,2,-1],[2,0,1,-4,-1],[0,1,1,1,1],[0,1,2,0,1],[0,0,-3,3,-1]]", CAT_CATEGORY_MATRIX},
  {"laguerre(n,a,x)", 0, "n-ieme Laguerre polynomial (default a=0).", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"laplace(f,x,s)", 0, "Laplace transform of f","sin(x),x,s", 0, CAT_CATEGORY_CALCULUS},
  {"lcm(a,b,...)", 0, "Least common multiple.", "23,13", "x^2-1,x^3-1", CAT_CATEGORY_ARIT | (CAT_CATEGORY_POLYNOMIAL << 8)},
  {"lcoeff(p,x)", 0, "Leading coefficient of polynomial p in x.", "x^4-1", 0, CAT_CATEGORY_POLYNOMIAL},
  {"legendre(n)", 0, "n-the Legendre polynomial.", "10", "10,t", CAT_CATEGORY_POLYNOMIAL},
#ifdef RELEASE
  {"len(l)", 0, "Size of a list.", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_LIST},
#endif
  {"leve_crayon ", "leve_crayon ", "Turtle moves without trace.", 0, 0, CAT_CATEGORY_LOGO},
  {"limit(f,x=a)", 0, "Limit of f at x = a. Add 1 or -1 for unidirectional limits, e.g. limit(sin(x)/x,x=0) or limit(abs(x)/x,x=0,1). Shortcut: SHIFT MIXEDFRAC", "sin(x)/x,x=0", "exp(-1/x),x=0,1", CAT_CATEGORY_CALCULUS},
  {"line(equation)", 0, "Line of equation", "y=2x+1", 0, CAT_CATEGORY_PROGCMD},
  {"line_width_", "line_width_", "Width prefix (2 to 8)", 0, 0, CAT_CATEGORY_PROGCMD},
  {"linear_regression(Xlist,Ylist)", 0, "Linear regression.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"linear_regression_plot(Xlist,Ylist)", 0, "Linear regression plot.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];linear_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  {"linetan(expr,x,x0)", 0, "Tangent to the graph at x=x0.", "sin(x),x,pi/2", 0, CAT_CATEGORY_PLOT},
  {"linsolve([eq1,eq2,..],[x,y,..])", 0, "Linear system solving. May use the output of lu for O(n^2) solving (see example 2).","[x+y=1,x-y=2],[x,y]", "#p,l,u:=lu([[1,2],[3,4]]); linsolve(p,l,u,[5,6])", CAT_CATEGORY_SOLVE | (CAT_CATEGORY_LINALG <<8) | (CAT_CATEGORY_MATRIX << 16)},
  {"logarithmic_regression(Xlist,Ylist)", 0, "Logarithmic egression.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"logarithmic_regression_plot(Xlist,Ylist)", 0, "Logarithmic regression plot.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];logarithmic_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  {"lu(A)", 0, "LU decomposition LU of matrix A, P*A=L*U", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"magenta", "magenta", "Display option", "#display=magenta", 0, CAT_CATEGORY_PROGCMD},
  {"map(f,l)", 0, "Maps f on element of list l.","lambda x:x*x,[1,2,3]", 0, CAT_CATEGORY_LIST},
  {"matpow(A,n)", 0, "Returns matrix A^n", "[[1,2],[3,4]],n","#assume(n>=1);matpow([[0,2],[0,4]],n)",  CAT_CATEGORY_MATRIX},
  {"matrix(r,c,func)", 0, "Matrix from a defining function.", "2,3,(j,k)->j^k", 0, CAT_CATEGORY_MATRIX},
  {"mean(l)", 0, "Arithmetic mean of list l", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"median(l)", 0, "Median", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"montre_tortue ", "montre_tortue ", "Displays the turtle", 0, 0, CAT_CATEGORY_LOGO},
  {"mult_c_conjugate", 0, "Multiplier par le conjugue complexe.", "1+2*i", 0,  (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"mult_conjugate", 0, "Multiplier par le conjugue (sqrt).", "sqrt(2)-sqrt(3)", 0, CAT_CATEGORY_ALGEBRA},
  {"normald([mu,sigma],x)", 0, "Normal distribution probability density, by default mu=0 and sigma=1. normald_cdf([mu,sigma],x) probability that \"normal distribution <=x\" e.g. normald_cdf(1.96). normald_icdf([mu,sigma],t) returns x such that \"normal distribution <=x\" has probability t, e.g. normald_icdf(0.975) ", "1.2", 0, CAT_CATEGORY_PROBA},
  {"not(x)", 0, "Logical not.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"numer(x)", 0, "Numerator of x.", "3/4", 0, CAT_CATEGORY_POLYNOMIAL},
  {"odesolve(f(t,y),[t,y],[t0,y0],t1)", 0, "Approx. solution of differential equation y'=f(t,y) and y(t0)=y0, value for t=t1 (add curve to get intermediate values of y)", "sin(t*y),[t,y],[0,1],2", "0..pi,(t,v)->{[-v[1],v[0]]},[0,1]", CAT_CATEGORY_SOLVE},
  {"partfrac(p,x)", 0, "Partial fraction expansion. Shortcut p=>+", "1/(x^4-1)", 0, CAT_CATEGORY_ALGEBRA},
  {"pas_de_cote n", "pas_de_cote ", "Turtle side jump from n steps, by default n=10", "#pas_de_cote 30", 0, CAT_CATEGORY_LOGO},
  {"plot(expr,x)", 0, "Plot an expression. For example plot(sin(x)), plot(ln(x),x.0,5)", "ln(x),x,0,5", "1/x,x=1..5,xstep=1", CAT_CATEGORY_PLOT},
#ifdef RELEASE
  {"plotarea(expr,x=a..b,[n,meth])", 0, "Area under curve with specified quadrature.", "1/x,x=1..3,2,trapezoid", 0, CAT_CATEGORY_PLOT},
#endif
  {"plotcontour(expr,[x=xm..xM,y=ym..yM],levels)", 0, "Levels of expr.", "x^2+2y^2,[x=-2..2,y=-2..2],[1,2]", 0, CAT_CATEGORY_PLOT},
  {"plotfield(f(t,y),[t=tmin..tmax,y=ymin..ymax])", 0, "Plot field of differential equation y'=f(t,y), an optionnally one solution by adding plotode=[t0,y0]", "sin(t*y),[t=-3..3,y=-3..3],plotode=[0,1]", 0, CAT_CATEGORY_PLOT},
  {"plotlist(list)", 0, "Plot a list", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_PLOT},
  {"plotode(f(t,y),[t=tmin..tmax,y],[t0,y0])", 0, "Plot solution of differential equation y'=f(t,y), y(t0)=y0.", "sin(t*y),[t=-3..3,y],[0,1]", 0, CAT_CATEGORY_PLOT},
  {"plotparam([x,y],t)", 0, "Parametric plot. For example plotparam([sin(3t),cos(2t)],t,0,pi) or plotparam(exp(i*t),t,0,pi)", "[sin(3t),cos(2t)],t,0,pi", "[t^2,t^3],t=-1..1,tstep=0.1", CAT_CATEGORY_PLOT},
  {"plotpolar(r,theta)", 0, "Polar plot.","cos(3*x),x,0,pi", "1/(1+cos(x)),x=0..pi,xstep=0.05", CAT_CATEGORY_PLOT},
  {"plotseq(f(x),x=[u0,m,M],n)", 0, "Plot f(x) on [m,M] and n terms of the sequence defined by u_{n+1}=f(u_n) and u0.","sqrt(2+x),x=[6,0,7],5", 0, CAT_CATEGORY_PLOT},
  {"plus_point", "plus_point", "Display option", "#display=blue+plus_point", 0, CAT_CATEGORY_PROGCMD},
  {"point(x,y)", 0, "Point", "1,2", 0, CAT_CATEGORY_PLOT},
  {"polygon(list)", 0, "Closed polygon.", "1-i,2+i,3", 0, CAT_CATEGORY_PROGCMD},
  {"polygonscatterplot(Xlist,Ylist)", 0, "Plot points and polygonal line.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"polynomial_regression(Xlist,Ylist,n)", 0, "Polynomial regression, degree <= n.", "[1,2,3,4,5],[0,1,3,4,4],2", 0, CAT_CATEGORY_STATS},
  {"polynomial_regression_plot(Xlist,Ylist,n)", 0, "Polynomial regression plot, degree <= n.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];polynomial_regression_plot(X,Y,2);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  //{"pour", "pour j de 1 jusque  faire  fpour;", "For loop.","#pour j de 1 jusque 10 faire print(j,j^2); fpour;", 0, CAT_CATEGORY_PROG},
  {"power_regression(Xlist,Ylist,n)", 0, "Power regression.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"power_regression_plot(Xlist,Ylist,n)", 0, "Power regression graph", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];power_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  {"powmod(a,n,p)", 0, "Returns a^n mod p.","123,456,789", 0, CAT_CATEGORY_ARIT},
  {"print(expr)", 0, "Print expr in console", 0, 0, CAT_CATEGORY_PROG},
  {"proot(p)", 0, "Returns real and complex roots, of polynomial p. Exemple proot([1,2.1,3,4.2]) or proot(x^3+2.1*x^2+3x+4.2)", "x^3+2.1*x^2+3x+4.2", 0, CAT_CATEGORY_POLYNOMIAL},
  {"purge(x)", 0, "Clear assigned variable x. Shortcut SHIFT-FORMAT", 0, 0, CAT_CATEGORY_PROGCMD|(CAT_CATEGORY_SOFUS<<8)},
  {"python(f)", 0, "Displays f in Python syntax.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"python_compat(0|1|2)", 0, "python_compat(0) Xcas syntax, python_compat(1) Python syntax with ^ interpreted as power, python_compat(2) ^ as bit xor", "0", "1", CAT_CATEGORY_PROG},
  {"qr(A)", 0, "A=Q*R factorization with Q orthogonal and R upper triangular", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"quartile1(l)", 0, "1st quartile", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"quartile3(l)", 0, "3rd quartile", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"quo(p,q,x)", 0, "Quotient of synthetic division of polynomials p and q (variable x).", 0, 0, CAT_CATEGORY_POLYNOMIAL},
  {"quote(x)", 0, "Returns expression x unevaluated.", 0, 0, CAT_CATEGORY_ALGEBRA},
  {"rand()", "rand()", "Random real between 0 and 1", 0, 0, CAT_CATEGORY_PROBA},
  {"randint(a,b)", 0, "Random integer between a and b. With 1 argument in Xcas, random integer between 1 and n.", "5,25", "6", CAT_CATEGORY_PROBA},
  {"ranm(n,m,[loi,parametres])", 0, "Random matrix with integer coefficients or according to a probability law (ranv for a vector). Examples ranm(2,3), ranm(3,2,binomial,20,.3), ranm(4,2,normald,0,1)", "3,3","4,2,normald,0,1",  CAT_CATEGORY_MATRIX},
  {"ranv(n,[loi,parametres])", 0, "Random vector.", "10","4,normald,0,1", CAT_CATEGORY_LINALG},
  {"ratnormal(x)", 0, "Puts everything over a common denominator.", 0, 0, CAT_CATEGORY_ALGEBRA},
  {"re(z)", 0, "Real part.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"read(\"filename\")", "read(\"", "Read a file.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"rectangle_plein a,b", "rectangle_plein ", "Direct filled rectangle from turtle position, if b is omitted b==a", "#rectangle_plein 30","#rectangle_plein 20,40", CAT_CATEGORY_LOGO},
  {"recule n", "recule ", "Turtle backward n steps, n=10 by default", "#recule 30", 0, CAT_CATEGORY_LOGO},
  {"red", "red", "Display option", "#display=red", 0, CAT_CATEGORY_PROGCMD},
  {"rem(p,q,x)", 0, "Remainder of synthetic division of polynomials p and q (variable x)", 0, 0, CAT_CATEGORY_POLYNOMIAL},
#ifdef RELEASE
  {"residue(f(z),z,z0)", 0, "Residue of an expression at z0.", "1/(x^2+1),x,i", 0, CAT_CATEGORY_COMPLEXNUM},
#endif
  {"resultant(p,q,x)", 0, "Resultant in x of polynomials p and q.", "#P:=x^3+p*x+q;resultant(P,P',x);", 0, CAT_CATEGORY_POLYNOMIAL},
  {"revert(p[,x])", 0, "Revert Taylor series","x+x^2+x^4", 0, CAT_CATEGORY_CALCULUS},
  {"rgb(r,g,b)", 0, "color defined from red, green, blue from 0 to 255", "255,0,255", 0, CAT_CATEGORY_PROGCMD},
  {"rhombus_point", "rhombus_point", "Display option", "#display=magenta+rhombus_point", 0, CAT_CATEGORY_PROGCMD},
  {"rond n", "rond ", "Circle tangent to the turtle, radius n. Run rond n,theta for an arc of circle of theta degrees", 0, 0, CAT_CATEGORY_LOGO},
  {"rsolve(equation,u(n),[init])", 0, "Solve a recurrence relation.","u(n+1)=2*u(n)+3,u(n),u(0)=1", "([u(n+1)=3*v(n)+u(n),v(n+1)=v(n)+u(n)],[u(n),v(n)],[u(0)=1,v(0)=2]", CAT_CATEGORY_SOLVE},
  {"saute n", "saute ", "Turtle jumps n steps, by default n=10", "#saute 30", 0, CAT_CATEGORY_LOGO},
  {"scatterplot(Xlist,Ylist)", 0, "Draws points", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"segment(A,B)", 0, "Segment", "1,2+i", 0, CAT_CATEGORY_PROGCMD},
  {"seq(expr,var,a,b)", 0, "Generates a list from an expression.","j^2,j,1,10", 0, CAT_CATEGORY_PROGCMD},
  //{"si", "si  alors  sinon  fsi;", "Test.", "#f(x):=si x>0 alors x; sinon -x; fsi;// valeur absolue", 0, CAT_CATEGORY_PROG},
  {"sign(x)", 0, "Returns -1 if x is negative, 0 if x is zero and 1 if x is positive.", 0, 0, CAT_CATEGORY_REAL|XCAS_ONLY},
  {"simplify(expr)", 0, "Returns x in a simpler form. Shortcut expr=>/", "sin(3x)/sin(x)", 0, CAT_CATEGORY_ALGEBRA},
  {"solve(equation,x)", 0, "Exact solving of equation w.r.t. x (or of a polynomial system). Run csolve for complex solutions, linsolve for a linear system. Shortcut SHIFT XthetaT", "x^2-x-1=0,x", "[x^2-y^2=0,x^2-z^2=0],[x,y,z]", CAT_CATEGORY_SOLVE},
  {"sorted(l)", 0, "Sorts a list.","[3/2,2,1,1/2,3,2,3/2]", "[[1,2],[2,3],[4,3]],(x,y)->when(x[1]==y[1],x[0]>y[0],x[1]>y[1]", CAT_CATEGORY_LIST},
  {"square_point", "square_point", "Display option", "#display=cyan+square_point", 0, CAT_CATEGORY_PROGCMD},
  {"star_point", "star_point", "Display option", "#display=magenta+star_point", 0, CAT_CATEGORY_PROGCMD},
  {"stddev(l)", 0, "Standard deviation of list l", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"subst(a,b=c)", 0, "Substitutes b for c in a. Shortcut a(b=c).", "x^2,x=3", 0, CAT_CATEGORY_ALGEBRA},
  {"sum(f,k,m,M)", 0, "Summation of expression f for k from m to M. Exemple sum(k^2,k,1,n)=>*. Shortcut ALPHA F3", "k,k,1,n", 0, CAT_CATEGORY_CALCULUS},
  {"svd(A)", 0, "Singular Value Decomposition, returns U orthogonal, S vector of singular values, Q orthogonal such that A=U*diag(S)*tran(Q).", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"tabvar(f,[x=a..b])", 0, "Table of variations of expression f, optional arguments variable x in interval a..b", "sqrt(x^2+x+1)",  "[cos(t),sin(3t)],t", CAT_CATEGORY_CALCULUS},
  //{"tantque", "tantque  faire   ftantque;", "While loop.", "#j:=13; tantque j!=1 faire j:=when(even(j),j/2,3j+1); print(j); ftantque;", 0, CAT_CATEGORY_PROG},
  {"taylor(f,x=a,n,[polynom])", 0, "Taylor expansion of f of x at a order n, add parameter polynom to remove remainder term.","sin(x),x=0,5", "sin(x),x=0,5,polynom", CAT_CATEGORY_CALCULUS},
  {"tchebyshev1(n)", 0, "Tchebyshev polynomial 1st kind: cos(n*x)=T_n(cos(x))", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"tchebyshev2(n)", 0, "Tchebyshev polynomial 2nd kind: sin((n+1)*x)=sin(x)*U_n(cos(x))", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"tcollect(expr)", 0, "Linearize and collect trig functions.","sin(x)+cos(x)", 0, CAT_CATEGORY_TRIG},
  {"texpand(expr)", 0, "Expand trigonometric, exp and ln functions.","sin(3x)", 0, CAT_CATEGORY_TRIG},
  {"time(cmd)", 0, "Time to run a command or set the clock","int(1/(x^4+1),x)","8,0", CAT_CATEGORY_PROG},
  {"tlin(expr)", 0, "Trigonometric linearization of expr.","sin(x)^3", 0, CAT_CATEGORY_TRIG},
  {"tourne_droite n", "tourne_droite ", "Turtle turns right n degrees, n=90 by default", 0, 0, CAT_CATEGORY_LOGO},
  {"tourne_gauche n", "tourne_gauche ", "Turtle turns left n degrees, n=90 by default", 0, 0, CAT_CATEGORY_LOGO},
  {"trace(A)", 0, "Trace of the matrix A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"transpose(A)", 0, "Transposes matrix A. Transconjugate command is trn(A) or A^*.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"triangle_point", "triangle_point", "Display option", "#display=yellow+triangle_point", 0, CAT_CATEGORY_PROGCMD},
  {"trig2exp(expr)", 0, "Convert complex exponentials to trigonometric functions","cos(x)^3", 0, CAT_CATEGORY_TRIG},
  {"trigcos(expr)", 0, "Convert sin^2 and tan^2 to cos^2.","sin(x)^4", 0, CAT_CATEGORY_TRIG},
  {"trigsin(expr)", 0, "Convert cos^2 and tan^2 to sin^2.","cos(x)^4", 0, CAT_CATEGORY_TRIG},
  {"trigtan(expr)", 0, "Convert cos^2 and sin^2 to tan^2.","cos(x)^4", 0, CAT_CATEGORY_TRIG},
  {"uniformd(a,b,x)", "uniformd", "uniform law on [a,b] of density 1/(b-a)", 0, 0, CAT_CATEGORY_PROBA},
  //{"version", "version()", "Khicas 1.5.0, (c) B. Parisse et al. www-fourier.ujf-grenoble.fr/~parisse\nLicense GPL version 2. Interface adapted from Eigenmath for Casio, G. Maia, http://gbl08ma.com. Do not use if CAS calculators are forbidden.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"write(\"filename\",var)", "write(\"", "Save 1 or more variables in a file. For example f(x):=x^2; write(\"func_f\",f).",  0, 0, CAT_CATEGORY_PROGCMD},
  {"yellow", "yellow", "Display option", "#display=yellow", 0, CAT_CATEGORY_PROGCMD},
  {"|", "|", "Logical or", "#1|2", 0, CAT_CATEGORY_PROGCMD},
  {"~", "~", "Complement", "#~7", 0, CAT_CATEGORY_PROGCMD},
};

  const char aide_khicas_string[]="Aide Khicas";
#ifdef NUMWORKS
  const char shortcuts_fr_string[]="Raccourcis clavier (shell et editeur)\nshift-/: %\nalpha shift \": '\nshift--: \\\nshift-ans: completion\nshift-*: factor\nshift-+: normal\nshift-1 a 6: selon bandeau en bas\nshift-7: matrices\nshift-8: complexes\nshift-9:arithmetique entiere\nshift-0: probas\nshift-.: reels\nshift-10^: polynomes\nvar: liste des variables\nans: figure tortue (editeur)\n\nshift-x^y (sto) renvoie =>\n=>+: partfrac\n=>*: factor\n=>sin/cos/tan\n=>=>: solve\n\nShell:\nshift-5: Editeur 2d ou graphique ou texte selon objet\nshift-6: editeur texte\n+ ou - modifie un parametre en surbrillance\n\nEditeur d'expressions\nshift-cut: defaire/refaire (1 fois)\npave directionnel: deplace la selection dans l'arborescence de l'expression\nshift-droit/gauche echange selection avec argument a droite ou a gauche\nalpha-droit/gauche dans une somme ou un produit: augmente la selection avec argument droit ou gauche\nshift-4: Editer selection, shift-5: taille police + ou - grande\nEXE: evaluer la selection\nshift-6: valeur approchee\nBackspace: supprime l'operateur racine de la selection\n\nEditeur de scripts\nEXE: passage a la ligne\nshift-CUT: documentation\nshift COPY (ou shift et deplacement curseur simultanement): marque le debut de la selection, deplacer le curseur vers la fin puis Backspace pour effacer ou shift-COPY pour copier sans effacer. shift-PASTE pour coller.\nHome-6 recherche seule: entrer un mot puis EXE puis EXE. Taper EXE pour l'occurence suivante, Back pour annuler.\nHome-6 remplacer: entrer un mot puis EXE puis le remplacement et EXE. Taper EXE ou Back pour remplacer ou non et passer a l'occurence suivante, AC pour annuler\nOK: tester syntaxe\n\nRaccourcis Graphes:\n+ - zoom\n(-): zoomout selon y\n*: autoscale\n/: orthonormalisation\nOPTN: axes on/off";
  const char shortcuts_en_string[]="Keyboard shortcuts (shell and editor)\nshift-/: %\nalpha shift \": '\nshift--: \\\nshift ans: completion\nshift-*: factor\nshift-+: normal\nshift-1 to 6: cf. screen bottom\nshift-7: matrices\nshift-8: complexes\nshift-9:arithmetic\nshift-0: proba\nshift-.: reals\nshift-10^: polynomials\nvar: variables list\nans: turtle screen (editor)\n\nshift-x^y (sto) returns =>\n=>+: partfrac\n=>*: factor\n=>sin/cos/tan\n=>=>: solve\n\nShell:\nshift-5: 2d editor or graph or text\nshift-6: text edit\n+ ou - modifies selected slider\n\nExpressions editor\nshift-cut: undo/redo (1 fois)\nkeypad: move selection inside expression tree\nshift-right/left exchange selection with right or left argument\nalpha-right/left: inside a sum or product: increase selection with right or left argument\nshift-4: Edit selection, shift-5: change fontsize\nEXE: eval selection\nshift-6: approx value\nBackspace: suppress selection's rootnode operator\n\nScript Editor\nEXE: newline\nshift-CUT: documentation\nshift-COPY: marks selection begin, move the cursor to the end, then hit Backspace to erase or shift-COPY to copy (no erase). shift-PASTE to paste.\nHome-6 search: enter a word then EXE then again EXE. Type EXE for next occurence, Back to cancel.\nHome-6 replace: enter a word then EXE then replacement word then EXE. Type EXE or Back to replace or ignore and go to next occurence, AC to cancel\nOK: test syntax\n\nGraph shortcuts:\n+ - zoom\n(-): zoomout along y\n*: autoscale\n/: orthonormalization\nOPTN: axes on/off";
#else
  const char shortcuts_fr_string[]="Raccourcis clavier (shell et editeur)\nlivre: aide/complete\ntab: complete (shell)/indente (editeur)\nshift-/: %\nshift *: '\nctrl-/: \\\nshift-1 a 6: selon bandeau en bas\nshift-7: matrices\nshift-8: complexes\nshift-9:arithmetique\nshift-0: probas\nshift-.: reels\nctrl P: programme\nvar: liste des variables\nans (shift (-)): figure tortue (editeur)\n\nctrl-var (sto) renvoie =>\n=>+: partfrac\n=>*: factor\n=>sin/cos/tan\n=>=>: solve\n\nShell:\nshift-5: Editeur 2d ou graphique ou texte selon objet\nshift-4: editeur texte\n+ ou - modifie un parametre en surbrillance\n\nEditeur d'expressions\nctrl z: defaire/refaire (1 fois)\npave directionnel: deplace la selection dans l'arborescence de l'expression\nshift-droit/gauche echange selection avec argument a droite ou a gauche\nctrl droit/gauche dans une somme ou un produit: augmente la selection avec argument droit ou gauche\nshift-4: Editer selection, shift-5: taille police + ou - grande\nenter: evaluer la selection\nshift-6: valeur approchee\nDel: supprime l'operateur racine de la selection\n\nEditeur de scripts\nenter: passage a la ligne\nctrl z: defaire/refaire (1 fois)\nctrl c ou shift et touche curseur simultanement: marque le debut de la selection, deplacer le curseur vers la fin puis Del pour effacer ou ctrl c pour copier sans effacer. ctrl v pour coller.\nMenu-6 recherche seule: entrer un mot puis enter puis enter. Taper enter pour l'occurence suivante, esc pour annuler.\nMenu-6 remplacer: entrer un mot puis enter puis le remplacement et enter. Taper enter ou esc pour remplacer ou non et passer a l'occurence suivante, ctrl del pour annuler\nvalidation (a droite de U): tester syntaxe\n\nRaccourcis Graphes:\n+ - zoom\n(-): zoomout selon y\n*: autoscale\n/: orthonormalisation\nOPTN: axes on/off";
  const char shortcuts_en_string[]="Keyboard shortcuts (shell and editor)\nbook: help or completion\ntab: completion (shell), indent (editor)\nshift-/: %\nalpha shift *: '\nctrl-/: \\\nshift-1 a 6: see at bottom\nshift-7: matrices\nshift-8: complexes\nshift-9:arithmetic\nshift-0: probas\nshift-.: reals\nctrl P: program\nvar: variables list\n ans (shift (-)): turtle screen (editor)\n\nctrl var (sto) returns =>\n=>+: partfrac\n=>*: factor\n=>sin/cos/tan\n=>=>: solve\n\nShell:\nshift-5: 2d editor or graph or text\nshift-4: text edit\n+ ou - modifies selected slider\n\nExpressions editor\nctrl z: undo/redo (1 fois)\nkeypad: move selection inside expression tree\nshift-right/left exchange selection with right or left argument\nalpha-right/left: inside a sum or product: increase selection with right or left argument\nshift-4: Edit selection, shift-5: change fontsize\nenter: eval selection\nshift-6: approx value\nDel: suppress selection's rootnode operator\n\nScript Editor\nenter: newline\nctrl z: undo/redo (1 time)\nctrl c or shift + cursor key simultaneously: marks selection begin, move the cursor to the end, then hit Del to erase or ctrl c to copy (no erase). ctrl v to paste.\nMenu-6 search: enter a word then enter then again enter. Type enter for next occurence, esc to cancel.\nMenu-6 replace: enter a word then enter then replacement word then enter. Type enter or esc to replace or ignore and go to next occurence, AC to cancel\nOK: test syntax\n\nGraph shortcuts:\n+ - zoom\n(-): zoomout along y\n*: autoscale\n/: orthonormalization\nOPTN: axes on/off";
#endif
  
  const char apropos_fr_string[]="Giac/Xcas 1.6.0, (c) 2020 B. Parisse et R. De Graeve, www-fourier.univ-grenoble-alpes.fr/~parisse.\nKhicas, interface pour calculatrices par B. Parisse, license GPL version 2, adaptee de l'interface d'Eigenmath pour Casio, G. Maia (http://gbl08ma.com), Mike Smith, Nemhardy, LePhenixNoir, ...\nPortage sur Numworks par Damien Nicolet. Remerciements a Jean-Baptiste Boric et Maxime Friess\nPortage sur Nspire grace a Fabian Vogt (firebird-emu, ndless...).\nTable periodique d'apres Maxime Friess\nRemerciements au site tiplanet, en particulier Xavier Andreani, Adrien Bertrand, Lionel Debroux";

  const char apropos_en_string[]="Giac/Xcas 1.6.0, (c) 2020 B. Parisse et R. De Graeve, www-fourier.univ-grenoble-alpes.fr/~parisse.\nKhicas, calculators interface by B. Parisse, GPL license version 2, adapted from Eigenmath for Casio, G. Maia (http://gbl08ma.com), Mike Smith, Nemhardy, LePhenixNoir, ...\nPorted on Numworks by Damien Nicolet. Thanks to Jean-Baptiste Boric and Maxime Friess\nPorted on Nspire thanks to Fabian Vogt (firebird-emu, ndless...)\nPeriodic table by Maxime Friess\nThanks to tiplanet, especially Xavier Andreani, Adrien Bertrand, Lionel Debroux";

  const int CAT_COMPLETE_COUNT_FR=sizeof(completeCatfr)/sizeof(catalogFunc);
  const int CAT_COMPLETE_COUNT_EN=sizeof(completeCaten)/sizeof(catalogFunc);

  std::string insert_string(int index){
    std::string s;
    const catalogFunc * completeCat=(lang==1)?completeCatfr:completeCaten;
    if (completeCat[index].insert)
      s=completeCat[index].insert;
    else {
      s=completeCat[index].name;
      int pos=s.find('(');
      if (pos>=0 && pos<s.size())
	s=s.substr(0,pos+1);
    }
    return s;//s+' ';
  }

  // not tested
  void aide2catalogFunc(const giac::aide & a,catalogFunc & c){
    static aide as=a;
    static string desc;
    string descrip;
    c.name=as.cmd_name.c_str();
    c.insert=c.name;
    desc=as.syntax+'\n';
    for (int i=0;i<as.blabla.size();++i){
      localized_string & ls=as.blabla[i];
      if (ls.language==lang){ // exact match
	descrip=as.blabla[i].chaine.c_str();
	break;
      }
      if (ls.language==0) // default
	descrip=as.blabla[i].chaine.c_str();
    }
    desc += descrip;
    c.desc=desc.c_str();
    c.example=as.examples.size()?as.examples[0].c_str():0;
    c.example2=as.examples.size()>=2?as.examples[1].c_str():0;
    c.category=-1;
  }
  int showCatalog(char* insertText,int preselect,int menupos,GIAC_CONTEXT) {
    // returns 0 on failure (user exit) and 1 on success (user chose a option)
    MenuItem menuitems[CAT_CATEGORY_LOGO+1];
    menuitems[CAT_CATEGORY_ALL].text = (char*)((lang==1)?"Tout":"All");
    menuitems[CAT_CATEGORY_ALGEBRA].text = (char*)((lang==1)?"Algebre":"Algebra");
    menuitems[CAT_CATEGORY_LINALG].text = (char*)((lang==1)?"Algebre lineaire":"Linear algebra");
    menuitems[CAT_CATEGORY_CALCULUS].text = (char*)((lang==1)?"Analyse":"Calculus");
    menuitems[CAT_CATEGORY_ARIT].text = (char*)"Arithmetic, crypto";
    menuitems[CAT_CATEGORY_COMPLEXNUM].text = (char*)"Complexes";
    menuitems[CAT_CATEGORY_PLOT].text = (char*)((lang==1)?"Courbes":"Curves");
    menuitems[CAT_CATEGORY_POLYNOMIAL].text = (char*)((lang==1)?"Polynomes":"Polynomials");
    menuitems[CAT_CATEGORY_PROBA].text = (char*)((lang==1)?"Probabilites":"Probabilities");
    menuitems[CAT_CATEGORY_PROGCMD].text = (char*)((lang==1)?"Programmes cmds (0)":"Program cmds (0)");
    menuitems[CAT_CATEGORY_REAL].text = (char*)((lang==1)?"Reels (e^)":"Reals");
    menuitems[CAT_CATEGORY_SOLVE].text = (char*)((lang==1)?"Resoudre (ln)":"Solve (ln)");
    menuitems[CAT_CATEGORY_STATS].text = (char*)((lang==1)?"Statistiques (log)":"Statistics (log)");
    menuitems[CAT_CATEGORY_TRIG].text = (char*)((lang==1)?"Trigonometrie (i)":"Trigonometry (i)");
    menuitems[CAT_CATEGORY_OPTIONS].text = (char*)"Options (,)";
    menuitems[CAT_CATEGORY_LIST].text = (char*)((lang==1)?"Listes (x^y)":"Lists (x^y)");
    menuitems[CAT_CATEGORY_MATRIX].text = (char*)"Matrices (sin)";
    menuitems[CAT_CATEGORY_PROG].text = (char*)((lang==1)?"Programmes (cos)":"Programs");
    menuitems[CAT_CATEGORY_SOFUS].text = (char*)((lang==1)?"Modifier variables (tan)":"Change variables (tan)");
    menuitems[CAT_CATEGORY_PHYS].text = (char*)((lang==1)?"Constantes physique (pi)":"Physics constants (pi)");
    menuitems[CAT_CATEGORY_UNIT].text = (char*)((lang==1)?"Unites physiques (sqrt)":"Units (sqrt)");
    menuitems[CAT_CATEGORY_LOGO].text = (char*)((lang==1)?"Tortue (x^2)":"Turtle (x^2)");
  
    Menu menu;
    menu.items=menuitems;
    menu.numitems=sizeof(menuitems)/sizeof(MenuItem);
    menu.scrollout=1;
    menu.title = (char*)((lang==1)?"Liste de commandes":"Commands list");
    //puts("catalog 1");
    while(1) {
      if (preselect)
	menu.selection=preselect;
      else {
	if (menupos>0)
	  menu.selection=menupos;
	int sres = doMenu(&menu);
	if (sres != MENU_RETURN_SELECTION && sres!=KEY_CTRL_EXE)
	  return 0;
      }
      // puts("catalog 3");
      if(doCatalogMenu(insertText, menuitems[menu.selection-1].text, menu.selection-1,contextptr)) 
	return 1;
      if (preselect)
	return 0;
    }
    return 0;
  }

  int showCatalog(char * text,int nmenu,GIAC_CONTEXT){
    return showCatalog(text,0,nmenu,contextptr);
  }

  bool isalphanum(char c){
    return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9');
  }

  string remove_accents(const string & s){
    string r;
    for (int i=0;i<s.size();++i){
      unsigned char ch=s[i];
      if (ch==195 && i<s.size()-1){
	++i;
	switch ((unsigned char)s[i]){
	case 160: case 161: case 162:
	  r+='a';
	  continue;
	case 168: case 169: case 170:
	  r+='e';
	  continue;
	case 172: case 173: case 174:
	  r+='i';
	  continue;
	case 178: case 179: case 180:
	  r += 'o';
	  continue;
	case 185: case 186: case 187:
	  r+='u';
	  continue;
	}
	r += '?';
	continue;
      }
      r+=ch;
    }
    return r;
  }

  // back is the number of char that should be deleted before inserting
  string help_insert(const char * cmdline,int & back,int exec,GIAC_CONTEXT){
    if (exec==KEY_CTRL_OK)
      exec=MENU_RETURN_SELECTION;
    back=0;
    int l=strlen(cmdline);
    char buf[l+1];
    strcpy(buf,cmdline);
    bool openpar=l && buf[l-1]=='(';
    if (openpar){
      buf[l-1]=0;
      --l;
      ++back;
    }
    for (;l>0;--l){
      if (!isalphanum(buf[l-1]) && buf[l-1]!='_')
	break;
    }
    // cmdname in buf+l
    const char * cmdname=buf+l,*cmdnameorig=cmdname;
    l=strlen(cmdname);
    // search in catalog: dichotomy would be more efficient
    // but leading spaces cmdnames would be missed
    int nfunc=(lang==1)?CAT_COMPLETE_COUNT_FR:CAT_COMPLETE_COUNT_EN;//sizeof(completeCat)/sizeof(catalogFunc);
#if defined NSPIRE_NEWLIB || defined NUMWORKS // should match static_help[] in help.cc
    int iii=nfunc; // no search in completeCat, directly in static_help.h
    //if (xcas_python_eval) iii=0;
#else
    int iii=0;
#endif
    const catalogFunc * completeCat=(lang==1)?completeCatfr:completeCaten;
    for (;iii<nfunc;++iii){
      if (xcas_python_eval && (completeCat[iii].category & XCAS_ONLY) )
	continue;
      const char * name=completeCat[iii].name;
      while (*name==' ')
	++name;
      int j=0;
      for (;j<l;++j){
	if (name[j]!=cmdname[j])
	  break;
      }
      if (j==l)
	break;
    }
    const catalogFunc * catf=iii==nfunc?0:completeCat+iii;
    const char * fhowto=0,* fsyntax=0,* frelated=0,* fexamples=0;
    string cf="";
    char fbuf[1024];
    if (iii==nfunc){
      if (!has_static_help(cmdname,exec?(lang==0?-2:-lang):lang,fhowto,fsyntax,fexamples,frelated)){
	confirm("Pas d'aide disponible pour",cmdname,true);
	return "";
      }
      cf=frelated;
      if (!fexamples || fexamples[0]==0){
	fexamples=frelated;
	frelated=0;
      }
      // cut example at ; if there is one
      for (int i=0;i<sizeof(fbuf);++i){
	if (fexamples[i]==0)
	  break;
	if (i>0 && fexamples[i]==';' && fexamples[i-1]!=' '){
	  strcpy(fbuf,fexamples);
	  fbuf[i]=0;
	  fexamples=fbuf;
	  frelated=fbuf+i+1;
	  while (*frelated==' ')
	    ++frelated;
	  for (++i;i<sizeof(fbuf);++i){
	    if (fbuf[i]==0)
	      break;
	    if (fbuf[i]==';'){
	      fbuf[i]=0;
	      break;
	    }
	  }
	  break;
	}
      }
    }
    const char * example=catf?catf->example:fexamples;
    const char * example2=catf?catf->example2:frelated;
    if (exec){
      if (!fsyntax){
	cmdname=example;
	example=example2;
      }
    }
    else {
      xcas::textArea text;
      text.editable=false;
      text.clipline=-1;
      text.title = (char*)((lang==1)?"Aide sur la commande":"Help on command");
      text.allowF1=true;
      text.python=false;
      std::vector<xcas::textElement> & elem=text.elements;
      elem = std::vector<xcas::textElement> (example2?5:4);
      elem[0].s = catf?catf->name:cmdname;
      elem[0].newLine = 0;
      elem[1].lineSpacing = 0;
      if (fsyntax){
	elem[1].newLine = 1;
	elem[1].s=(lang==1?"Syntaxe: ":"Syntax: ")+elem[0].s+"("+(strlen(fsyntax)?fsyntax:"arg")+")";
      }
      else {
	elem[1].newLine = 0;
	elem[1].s=elem[0].s;
      }
      if (cf.size())
	elem[0].s += " (cf. "+cf+")";
      if (elem[0].s.size()<16)
	elem[0].s=string(16-elem[0].s.size()/2,' ')+elem[0].s;
      //elem[0].color = COLOR_BLUE;
      elem[2].newLine = 1;
      elem[2].lineSpacing = 1;
      elem[2].minimini=1;
      std::string autoexample;
      if (catf && catf->desc==0){
	// if (token==T_UNARY_OP || token==T_UNARY_OP_38)
	elem[2].s=elem[0].s+"(args)";
      }
      else {
#ifdef NUMWORKS
	elem[2].s = remove_accents(catf?catf->desc:fhowto);
#else
	elem[2].s = catf?catf->desc:fhowto;
#endif
      }
#ifdef NSPIRE_NEWLIB
      std::string ex("tab: ");
#else
      std::string ex("Ans: ");
#endif
      elem[3].newLine = 1;
      elem[3].lineSpacing = 0;
      //elem[2].minimini=1;
      if (example){
	if (example[0]=='#')
	  ex += example+1;
	else {
	  if (iii==nfunc)
	    ex += fexamples;
	  else {
	    ex += insert_string(iii);
	    ex += example;
	    ex += ")";
	  }
	}
	elem[3].s = ex;
	if (example2){
#ifdef NSPIRE_NEWLIB
	  string ex2="ret: ";
#else
	  string ex2="EXE: ";
#endif
	  if (example2[0]=='#')
	    ex2 += example2+1;
	  else {
	    if (iii==nfunc)
	      ex2 += example2;
	    else {
	      ex2 += insert_string(iii);
	      ex2 += example2;
	      ex2 += ")";
	    }
	  }
	  elem[4].newLine = 1;
	  // elem[3].lineSpacing = 0;
	  //elem[3].minimini=1;
	  elem[4].s=ex2;
	}
      }
      else {
	if (autoexample.size())
	  elem[3].s=ex+autoexample;
	else
	  elem.pop_back();
      }
      exec=doTextArea(&text,contextptr);
    }
    if (exec==KEY_SHUTDOWN)
      return "";
    if (exec==MENU_RETURN_SELECTION){
      while (*cmdname && *cmdname==*cmdnameorig){
	++cmdname; ++cmdnameorig;
      }
      return cmdname;
    }
    if (exec == KEY_CHAR_ANS || exec==KEY_BOOK || exec=='\t' || exec==KEY_CTRL_EXE) {
      reset_kbd();
      std::string s;
      const char * example=0;
      if (exec==KEY_CHAR_ANS || exec==KEY_BOOK || exec=='\t')
	example=catf?catf->example:fexamples;
      else
	example=catf?catf->example2:frelated;
      if (example){
	while (*example && *example==*cmdnameorig){
	  ++example; ++cmdnameorig;
	}
	while (*cmdnameorig){
	  ++back;
	  ++cmdnameorig;
	}
	if (example[0]=='#')
	  s=example+1;
	else {
	  s += example;
	  //if (catf && s[s.size()-1]!=')') s += ")";
	}
      }
      if (python_compat(contextptr) & 4){
	// replace := by =
	for (int i=1;i<s.size();++i){
	  if (s[i]=='=' && s[i-1]==':')
	    s.erase(s.begin()+i-1);
	}
      }
      return s;
    }
    return "";
  }

  // 0 on exit, 1 on success
  int doCatalogMenu(char* insertText, const char* title, int category,GIAC_CONTEXT) {
    const catalogFunc * completeCat=(lang==1)?completeCatfr:completeCaten;
    for (;;){
      int allcmds=builtin_lexer_functions_end()-builtin_lexer_functions_begin();
      int allopts=lexer_tab_int_values_end-lexer_tab_int_values_begin;
      bool isall=category==CAT_CATEGORY_ALL;
      bool isopt=category==CAT_CATEGORY_OPTIONS;
      const int CAT_COMPLETE_COUNT=((lang==1)?CAT_COMPLETE_COUNT_FR:CAT_COMPLETE_COUNT_EN);
      int nitems = isall? allcmds:(isopt?allopts:CAT_COMPLETE_COUNT);
#ifdef MENUITEM_MALLOC
      MenuItem *menuitems=(MenuItem *) malloc(sizeof(MenuItem)*nitems);
      if (!menuitems)
	return 0;
#else
      MenuItem menuitems[nitems];
#endif
      int cur = 0,curmi = 0,i=0;
#ifdef MICROPY_LIB
      if (xcas_python_eval)
	micropy_ck_eval("1");
#endif
      gen g;
      while(cur<nitems) {
	menuitems[curmi].type = MENUITEM_NORMAL;
	menuitems[curmi].color = _BLACK;    
	if (isall || isopt) {
	  const char * text=isall?(builtin_lexer_functions_begin()+cur)->first:(lexer_tab_int_values_begin+curmi)->keyword;
#ifdef MICROPY_LIB
	  if (xcas_python_eval && xcas::find_color(text,contextptr)!=3){
	    ++cur;
	    continue;
	  }
#endif
	  menuitems[curmi].text = (char*) text;
	  menuitems[curmi].isfolder = allcmds; // assumes allcmds>allopts
	  menuitems[curmi].token=isall?((builtin_lexer_functions_begin()+curmi)->second.subtype+256):((lexer_tab_int_values_begin+curmi)->subtype+(lexer_tab_int_values_begin+curmi)->return_value*256);
	  // menuitems[curmi].token=isall?find_or_make_symbol(text,g,0,false,contextptr):((lexer_tab_int_values_begin+curmi)->subtype+(lexer_tab_int_values_begin+curmi)->return_value*256);
	  for (;i<CAT_COMPLETE_COUNT;++i){
	    const char * catname=completeCat[i].name;
	    int tmp=strcmp(catname,text);
	    if (tmp>=0){
	      size_t st=strlen(text),j=tmp?0:st;
	      for (;j<st;++j){
		if (catname[j]!=text[j])
		  break;
	      }
	      if (j==st && (!isalphanum(catname[j]))){
		menuitems[curmi].isfolder = i;
		++i;
	      }
	      break;
	    }
	  }
	  // compare text with completeCat
	  ++curmi;
	}
	else {
	  int cat=completeCat[cur].category;
	  if (
	      (!xcas_python_eval || !(cat & XCAS_ONLY) ) &&
	      ((cat & 0xff) == category ||
	       (cat & 0xff00) == (category<<8) ||
	       (cat & 0xff0000) == (category <<16) )
	      ){
	    menuitems[curmi].isfolder = cur; // little hack: store index of the command in the full list in the isfolder property (unused by the menu system in this case)
	    menuitems[curmi].text = (char *) completeCat[cur].name;
	    curmi++;
	  }
	}
	cur++;
      }
      
      Menu menu;
      menu.items=menuitems;
      menu.numitems=curmi;
      if (isopt){ menu.selection=5; menu.scroll=4; }
      if (curmi>=100)
	lock_alpha(); //SetSetupSetting( (unsigned int)0x14, 0x88);	
      // DisplayStatusArea();
      menu.scrollout=1;
      menu.title = (char *) title;
      menu.type = MENUTYPE_FKEYS;
      menu.height = 11;
      while(1) {
	drawRectangle(0,200,LCD_WIDTH_PX,22,giac::_WHITE);
#ifdef NSPIRE_NEWLIB
	PrintMini(0,200,(category==CAT_CATEGORY_ALL?"doc: help | tab: ex1 | enter: ex2":"doc: help | enter ex1 | tab ex2"),4,33333,giac::_WHITE);
#else
	PrintMini(0,200,(category==CAT_CATEGORY_ALL?"Toolbox help | Ans ex1 | EXE  ex2":"Toolbox help | EXE ex1 | Ans ex2"),4,33333,giac::_WHITE);
#endif
	int sres = 0;
	if (curmi==0){
	  do_confirm(lang==1?"Commandes seulement en mode Xcas":"Commands only in Xcas mode");
	  sres=MENU_RETURN_EXIT;
	}
	else
	  sres=doMenu(&menu);
	if (sres==KEY_CTRL_F4 && category!=CAT_CATEGORY_ALL){
	  break;
	}
	if(sres == MENU_RETURN_EXIT){
	  reset_kbd();
#ifdef MENUITEM_MALLOC
	  free(menuitems);
#endif
	  return sres;
	}
	int index=menuitems[menu.selection-1].isfolder;
	if(sres == KEY_CTRL_CATALOG || sres==KEY_BOOK) {
	  const char * example=index<allcmds?completeCat[index].example:0;
	  const char * example2=index<allcmds?completeCat[index].example2:0;
	  xcas::textArea text;
	  text.editable=false;
	  text.clipline=-1;
	  text.title = (char*)((lang==1)?"Aide sur la commande":"Help on command");
	  text.allowF1=true;
	  text.python=python_compat(contextptr);
	  std::vector<xcas::textElement> & elem=text.elements;
	  elem = std::vector<xcas::textElement> (example2?4:3);
	  elem[0].s = index<allcmds?completeCat[index].name:menuitems[menu.selection-1].text;
	  if (index<allcmds && (completeCat[index].category & XCAS_ONLY) )
	    elem[0].s += lang==1?" (Xcas seulement)":" (Xcas only)";
	  elem[0].newLine = 0;
	  //elem[0].color = COLOR_BLUE;
	  elem[1].newLine = 1;
	  elem[1].lineSpacing = 1;
	  elem[1].minimini=1;
	  std::string autoexample;
	  if (index<allcmds)
	    elem[1].s = completeCat[index].desc;
	  else {
	    int token=menuitems[menu.selection-1].token;
	    elem[1].s="Desole, pas d'aide disponible...";
	    const char *fcmdname=menuitems[menu.selection-1].text,* fhowto=0,*fsyntax=0,*fexamples=0,*frelated=0;
	    if (has_static_help(fcmdname,lang,fhowto,fsyntax,fexamples,frelated)){
	      elem[1].s=fhowto;
	      example=fexamples;
	    }
	    else {
	      // *logptr(contextptr) << token << endl;
	      if (isopt){
		if (token==_INT_PLOT+T_NUMBER*256){
		  autoexample="display="+elem[0].s;
		  elem[1].s ="Option d'affichage: "+ autoexample;
		}
		if (token==_INT_COLOR+T_NUMBER*256){
		  autoexample="display="+elem[0].s;
		  elem[1].s="Option de couleur: "+ autoexample;
		}
		if (token==_INT_SOLVER+T_NUMBER*256){
		  autoexample=elem[0].s;
		  elem[1].s="Option de fsolve: " + autoexample;
		}
		if (token==_INT_TYPE+T_TYPE_ID*256){
		  autoexample=elem[0].s;
		  elem[1].s="Type d'objet: " + autoexample;
		}
	      }
	      if (isall){
		if (token==T_UNARY_OP || token==T_UNARY_OP_38)
		  elem[1].s=elem[0].s+"(args)";
	      }
	    }
	  }
#ifdef NSPIRE_NEWLIB
	  std::string ex("enter: ");
#else
	  std::string ex("EXE: ");
#endif
	  elem[2].newLine = 1;
	  elem[2].lineSpacing = 0;
	  //elem[2].minimini=1;
	  if (example){
	    if (example[0]=='#')
	      ex += example+1;
	    else {
	      if (index<allcmds){
		ex += insert_string(index);
		ex += example;
		ex += ")";
	      }
	      else ex+=example;
	    }
	    elem[2].s = ex;
	    if (example2){
#ifdef NSPIRE_NEWLIB
	      string ex2="tab: ";
#else
	      string ex2="Ans: ";
#endif
	      if (example2[0]=='#')
		ex2 += example2+1;
	      else {
		if (index<allcmds){
		  ex2 += insert_string(index);
		  ex2 += example2;
		  ex2 += ")";
		}
		else
		  ex2 += example2;
	      }
	      elem[3].newLine = 1;
	      // elem[3].lineSpacing = 0;
	      //elem[3].minimini=1;
	      elem[3].s=ex2;
	    }
	  }
	  else {
	    if (autoexample.size())
	      elem[2].s=ex+autoexample;
	    else
	      elem.pop_back();
	  }
	  sres=doTextArea(&text,contextptr);
	}
	if (sres == KEY_CHAR_ANS || sres=='\t' ||sres==KEY_BOOK || sres==KEY_CTRL_EXE) {
	  reset_kbd();
	  const char * example=0;
	  std::string s;
	  if (index<allcmds ){
	    s=insert_string(index);
	    if (sres==KEY_CHAR_ANS || sres=='\t' || sres==KEY_BOOK)
	      example=completeCat[index].example2;
	    else
	      example=completeCat[index].example;
	  }
	  else {
	    const char *fcmdname=menuitems[menu.selection-1].text,* fhowto=0,*fsyntax=0,*fexamples=0,*frelated=0;
	    if (has_static_help(fcmdname,lang,fhowto,fsyntax,fexamples,frelated)){
	      example=fexamples;
	    }
	  }
	  if (example){
	    if (example[0]=='#')
	      s=example+1;
	    else {
	      s += example;
	      if (s[s.size()-1]!=')')
		s += ")";
	    }
	    strcpy(insertText, s.c_str());
#ifdef MENUITEM_MALLOC
	    free(menuitems);
#endif
	    return 1;
	  }
	  else {
	    if (isopt){
	      int token=menuitems[menu.selection-1].token;
	      if (token==_INT_PLOT+T_NUMBER*256 || token==_INT_COLOR+T_NUMBER*256)
		strcpy(insertText,"display=");
	      else
		*insertText=0;
	      strcat(insertText,menuitems[menu.selection-1].text);
#ifdef MENUITEM_MALLOC
	      free(menuitems);
#endif
	      return 1;
	    }
	  }
	  sres=KEY_CTRL_OK;
	}
	if(sres == MENU_RETURN_SELECTION || sres == KEY_CTRL_OK) {
	  reset_kbd();
	  strcpy(insertText,index<allcmds?insert_string(index).c_str():menuitems[menu.selection-1].text);
#ifdef MENUITEM_MALLOC
	  free(menuitems);
#endif
	  return 1;
	}
      }
      title="CATALOG";
      category=0;
    } // end endless for
    return 0;
  }

  int trialpha(const void *p1,const void * p2){
    int i=strcmp(* (char * const *) p1, * (char * const *) p2);
    return i;
  }

  gen select_var(GIAC_CONTEXT){
    kbd_interrupted=giac::ctrl_c=giac::interrupted=false;
#ifdef MICROPY_LIB
    if (xcas_python_eval==1){
      micropy_ck_eval("");
      const char ** tab=(const char **)mp_vars();
      const char **ptr=tab;
      for (;*ptr;)
	++ptr;
      // del at end should not be sorted
      if (ptr-tab>=1 && strcmp(*(ptr-1),"del ")==0)
	--ptr;
      qsort(tab,ptr-tab,sizeof(char *),trialpha);
      if (tab){
	int i=select_item(tab,"VARS",true);
	gen g=undef;
	if (i>=0 && tab[i])
	  g=string2gen(tab[i],false);
	free(tab);
	return g;
      }
    }
#endif
    gen g(_VARS(0,contextptr));
    if (g.type!=_VECT || g._VECTptr->empty()){
      confirm((lang==1)?"Pas de variables. Exemples pour en creer":"No variables. Examples to create",(lang==1)?"a=1 ou f(x):=sin(x^2)":"a=1 or f(x):=sin(x^2)",true);
      return undef;
    }
    vecteur & v=*g._VECTptr;
    MenuItem smallmenuitems[v.size()+3];
    vector<std::string> vs(v.size()+1);
    int i,total=0;
    const char typ[]="idzDcpiveSfEsFRmuMwgPF";
    for (i=0;i<v.size();++i){
      vs[i]=v[i].print(contextptr);
      if (v[i].type==giac::_IDNT){
	giac::gen w;
	v[i]._IDNTptr->in_eval(0,v[i],w,contextptr,true);
#if 1
	vector<int> vi(9);
	tailles(w,vi);
	total += vi[8];
	if (vi[8]<400)
	  vs[i]+=":="+w.print(contextptr);
	else {
	  vs[i] += " ~";
	  vs[i] += giac::print_INT_(vi[8]);
	  vs[i] += ',';
	  vs[i] += typ[w.type];
	}
#else
	if (taille(w,50)<50)
	  vs[i]+=": "+w.print(contextptr);
#endif
      }
      smallmenuitems[i].text=(char *) vs[i].c_str();
    }
    total +=
      // giac::syms().capacity()*(sizeof(string)+sizeof(giac::gen)+8)+sizeof(giac::sym_string_tab) +
      giac::turtle_stack().capacity()*sizeof(giac::logo_turtle) +
      // sizeof(giac::context)+contextptr->tabptr->capacity()*(sizeof(const char *)+sizeof(giac::gen)+8)+
      bytesize(giac::history_in(contextptr))+bytesize(giac::history_out(contextptr));
    vs[i]="purge(~"+giac::print_INT_(total)+')';
    smallmenuitems[i].text=(char *)vs[i].c_str();
    smallmenuitems[i+1].text=(char *)"assume(";
    smallmenuitems[i+2].text=(char *)"restart";
    Menu smallmenu;
    smallmenu.numitems=v.size()+3; 
    smallmenu.items=smallmenuitems;
    smallmenu.height=12;
    smallmenu.scrollbar=1;
    smallmenu.scrollout=1;
    smallmenu.title = (char*)"Variables";
    //MsgBoxPush(5);
    int sres = doMenu(&smallmenu);
    //MsgBoxPop();
    if (sres==KEY_CTRL_DEL && smallmenu.selection<=v.size())
      return symbolic(at_purge,v[smallmenu.selection-1]);
    if (sres!=MENU_RETURN_SELECTION && sres!=KEY_CTRL_EXE)
      return undef;
    if (smallmenu.selection==1+v.size())
      return string2gen("purge(",false);
    if (smallmenu.selection==2+v.size())
      return string2gen("assume(",false);
    if (smallmenu.selection==3+v.size())
      return string2gen("restart",false);
    return v[smallmenu.selection-1];
  }
  
  const char * keytostring(int key,int keyflag,bool py,const giac::context * contextptr){
    const int textsize=512;
    static char text[textsize];
    if (key>=0x20 && key<=0x7e){
      text[0]=key;
      text[1]=0;
      return text;
    }
    switch (key){
    case KEY_CHAR_PLUS:
      return "+";
    case KEY_CHAR_MINUS:
      return "-";
    case KEY_CHAR_PMINUS:
      return "_";
    case KEY_CHAR_MULT:
      return "*";
    case KEY_CHAR_FRAC:
      return py?"\\":"solve(";
    case KEY_CHAR_DIV: 
      return "/";
    case KEY_CHAR_POW:
      return "^";
    case KEY_CHAR_ROOT:
      return "sqrt(";
    case KEY_CHAR_SQUARE:
      return py?"**2":"^2";
    case KEY_CHAR_CUBEROOT:
      return py?"**(1/3)":"^(1/3)";
    case KEY_CHAR_POWROOT:
      return py?"**(1/":"^(1/";
    case KEY_CHAR_RECIP:
      return py?"**-1":"^-1";
    case KEY_CHAR_THETA:
      return "arg(";
    case KEY_CHAR_VALR:
      return "abs(";
    case KEY_CHAR_ANGLE:
      return "polar_complex(";
    case KEY_CTRL_XTT:
      return xthetat?"t":"x";
    case KEY_CHAR_LN:
      return "ln(";
    case KEY_CHAR_LOG:
      return "log10(";
    case KEY_CHAR_EXPN10:
      return "10^";
    case KEY_CHAR_EXPN:
      return "exp(";
    case KEY_CHAR_SIN:
      return "sin(";
    case KEY_CHAR_COS:
      return "cos(";
    case KEY_CHAR_TAN:
      return "tan(";
    case KEY_CHAR_ASIN:
      return "asin(";
    case KEY_CHAR_ACOS:
      return "acos(";
    case KEY_CHAR_ATAN:
      return "atan(";
    case KEY_CTRL_MIXEDFRAC:
      return "limit(";
    case KEY_CTRL_FRACCNVRT:
      return "exact(";
      // case KEY_CTRL_FORMAT: return "purge(";
    case KEY_CTRL_FD:
      return "approx(";
    case KEY_CHAR_STORE:
      // if (keyflag==1) return "inf";
      return "=>";
    case KEY_CHAR_IMGNRY:
      return "i";
    case KEY_CHAR_PI:
      return "pi";
    case KEY_CTRL_VARS: {
      giac::gen var=select_var(contextptr);
      if (!giac::is_undef(var)){
	strcpy(text,(var.type==giac::_STRNG?*var._STRNGptr:var.print(contextptr)).c_str());
	return text;
      }
      return "";//"VARS()";
    }
    case KEY_CHAR_EXP:
      return "e";
    case KEY_CHAR_ANS:
      return "ans()";
    case KEY_CHAR_CROCHETS:
      return "[]";
    case KEY_CHAR_ACCOLADES:
      return "{}";
    case KEY_CTRL_INS:
      return ":=";
    case KEY_CHAR_MAT:{
      const char * ptr=xcas::input_matrix(false,contextptr); if (ptr) return ptr;
      if (showCatalog(text,17,contextptr)) return text;
      return "";
    }
    case KEY_CHAR_LIST: {
      const char * ptr=xcas::input_matrix(true,contextptr); if (ptr) return ptr;
      if (showCatalog(text,16,contextptr)) return text;
      return "";
    }
    case KEY_CTRL_PRGM:
      // open functions catalog, prgm submenu
      if(showCatalog(text,18,contextptr))
	return text;
      return "";
    case KEY_CTRL_CATALOG: case KEY_BOOK:
      if(showCatalog(text,0,contextptr)) 
	return text;
      return "";
    case KEY_CTRL_F4:
      if(showCatalog(text,0,contextptr)) 
	return text;
      return "";
    case KEY_CTRL_OPTN:
      if(showCatalog(text,15,contextptr))
	return text;
      return "";
    case KEY_CTRL_QUIT: 
      if(showCatalog(text,20,contextptr))
	return text;
      return "";
    case KEY_CTRL_PASTE:
      return paste_clipboard();
    case KEY_CHAR_DQUATE:
      return "\"";
    case KEY_CHAR_FACTOR:
      return "factor(";
    case KEY_CHAR_NORMAL:
      return "normal(";
    }
    return 0;
  }
  
  const char * keytostring(int key,int keyflag,GIAC_CONTEXT){
    return keytostring(key,keyflag,python_compat(contextptr),contextptr);
  }
  
  bool stringtodouble(const string & s1,double & d){
    gen g(s1,context0);
    g=evalf(g,1,context0);
    if (g.type!=_DOUBLE_){
      confirm("Invalid value",s1.c_str());
      return false;
    }
    d=g._DOUBLE_val;
    return true;
  }

  bool inputdouble(const char * msg1,double & d,GIAC_CONTEXT){
    int di=d;
    string s1;
    if (di==d)
      s1=print_INT_(di);
    else
      s1=print_DOUBLE_(d,3);
    inputline(msg1,((lang==1)?"Nouvelle valeur? ":"New value? "),s1,false,65,contextptr);
    return stringtodouble(s1,d);
  }
  
  int inputline(const char * msg1,const char * msg2,string & s,bool numeric,int ypos,GIAC_CONTEXT){
    //s=msg2;
    int pos=s.size(),beg=0;
    for (;;){
      int X1=print_msg12(msg1,msg2,ypos-30);
      int textX=X1,textY=ypos;
      drawRectangle(textX,textY,LCD_WIDTH_PX-textX-4,18,COLOR_WHITE);
      if (pos-beg>36)
	beg=pos-12;
      if (int(s.size())-beg<36)
	beg=giac::giacmax(0,int(s.size())-36);
      textX=X1;
#if 0
      os_draw_string_(textX,textY,(s.substr(beg,pos-beg)+"|"+s.substr(pos,s.size()-pos)).c_str());
#else
      textX=os_draw_string_(textX,textY+2,s.substr(beg,pos-beg).c_str());
      os_draw_string_(textX,textY+2,s.substr(pos,s.size()-pos).c_str());
      drawRectangle(textX,textY+4,2,13,COLOR_BLACK); // cursor
      // PrintMini(0,58,"         |        |        |        |  A<>a  |       ",4);
#endif
      int key;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;      
      // if (!giac::freeze) set_xcas_status();    
      if (key==KEY_CTRL_EXE || key==KEY_CTRL_OK)
	return KEY_CTRL_EXE;
      if (key>=32 && key<128){
	if (!numeric || key=='-' || (key>='0' && key<='9')){
	  s.insert(s.begin()+pos,char(key));
	  ++pos;
	}
	continue;
      }
      if (key==KEY_CHAR_ACCOLADES || key==KEY_CHAR_CROCHETS){
	s.insert(s.begin()+pos,key==KEY_CHAR_ACCOLADES?'}':']');
	s.insert(s.begin()+pos,key==KEY_CHAR_ACCOLADES?'{':'[');
	++pos;
	continue;	
      }
      if (key==KEY_CTRL_DEL){
	if (pos){
	  s.erase(s.begin()+pos-1);
	  --pos;
	}
	continue;
      }
      if (key==KEY_CTRL_AC){
	if (s=="")
	  return KEY_CTRL_EXIT;
	s="";
	pos=0;
	continue;
      }
      if (key==KEY_CTRL_EXIT)
	return key;
      if (key==KEY_CTRL_RIGHT){
	if (pos<s.size())
	  ++pos;
	continue;
      }
      if (key==KEY_SHIFT_RIGHT){
	pos=s.size();
	continue;
      }
      if (key==KEY_CTRL_LEFT){
	if (pos)
	  --pos;
	continue;
      }
      if (key==KEY_SHIFT_LEFT){
	pos=0;
	continue;
      }
      if (const char * ans=keytostring(key,0,false,contextptr)){
	insert(s,pos,ans);
	pos+=strlen(ans);
	continue;
      }
    }
  }

  logo_turtle * turtleptr=0;
  
  logo_turtle & turtle(){
    if (!turtleptr)
      turtleptr=new logo_turtle;
    return * turtleptr;
  }

#ifdef NSPIRE_NEWLIB
  const int MAX_LOGO=2048; 
#else
  const int MAX_LOGO=368; // 512;
#endif

  std::vector<logo_turtle> & turtle_stack(){
    static std::vector<logo_turtle> * ans = 0;
    if (!ans){
      // initialize from python app storage
      ans=new std::vector<logo_turtle>(1,(*turtleptr));
     
    }
    return *ans;
  }

  logo_turtle vecteur2turtle(const vecteur & v){
    int s=int(v.size());
    if (s>=5 && v[0].type==_DOUBLE_ && v[1].type==_DOUBLE_ && v[2].type==_DOUBLE_ && v[3].type==_INT_ && v[4].type==_INT_ ){
      logo_turtle t;
      t.x=v[0]._DOUBLE_val;
      t.y=v[1]._DOUBLE_val;
      t.theta=v[2]._DOUBLE_val;
      int i=v[3].val;
      t.mark=(i%2)!=0;
      i=i >> 1;
      t.visible=(i%2)!=0;
      i=i >> 1;
      t.direct = (i%2)!=0;
      i=i >> 1;
      t.turtle_length = i & 0xff;
      i=i >> 8;
      t.color = i;
      t.radius = v[4].val;
      if (s>5 && v[5].type==_INT_)
	t.s=v[5].val;
      else
	t.s=-1;
      return t;
    }
#ifndef NO_STDEXCEPT
    setsizeerr(gettext("vecteur2turtle")); // FIXME
#endif
    return logo_turtle();
  }

  static int turtle_status(const logo_turtle & turtle){
    int status= (turtle.color << 11) | ( (turtle.turtle_length & 0xff) << 3) ;
    if (turtle.direct)
      status += 4;
    if (turtle.visible)
      status += 2;
    if (turtle.mark)
      status += 1;
    return status;
  }

  bool set_turtle_state(const vecteur & v,GIAC_CONTEXT){
    if (v.size()>=2 && v[0].type==_DOUBLE_ && v[1].type==_DOUBLE_){
      vecteur w(v);
      int s=int(w.size());
      if (s==2)
	w.push_back(double((*turtleptr).theta));
      if (s<4)
	w.push_back(turtle_status((*turtleptr)));
      if (s<5)
	w.push_back(0);
      if (w[2].type==_DOUBLE_ && w[3].type==_INT_ && w[4].type==_INT_){
	(*turtleptr)=vecteur2turtle(w);
#ifdef TURTLETAB
	turtle_stack_push_back(*turtleptr);
#else
	turtle_stack().push_back((*turtleptr));
#endif
	return true;
      }
    }
    return false;
  }

  gen turtle2gen(const logo_turtle & turtle){
    return gen(makevecteur(turtle.x,turtle.y,double(turtle.theta),turtle_status(turtle),turtle.radius,turtle.s),_LOGO__VECT);
  }

  gen turtle_state(GIAC_CONTEXT){
    return turtle2gen((*turtleptr));
  }

  static gen update_turtle_state(bool clrstring,GIAC_CONTEXT){
#ifdef TURTLETAB
    if (turtle_stack_size>=MAX_LOGO)
      return gensizeerr("Not enough memory");
#else
    if (turtle_stack().size()>=MAX_LOGO){
      ctrl_c=true; interrupted=true;
      return gensizeerr("Not enough memory");
    }
#endif
    if (clrstring)
      (*turtleptr).s=-1;
    (*turtleptr).theta = (*turtleptr).theta - floor((*turtleptr).theta/360)*360;
#ifdef TURTLETAB
    turtle_stack_push_back((*turtleptr));
#else
    turtle_stack().push_back((*turtleptr));
#endif
    gen res=turtle_state(contextptr);
#ifdef EMCC // should directly interact with canvas
    return gen(turtlevect2vecteur(turtle_stack()),_LOGO__VECT);
#endif
    return res;
  }

  int turtle_speed=0;
  gen _speed(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_VECT && g._VECTptr->empty())
      return turtle_speed;
    if (g.type!=_INT_)
      return gensizeerr(contextptr);
    int i=g.val;
    if (i<0) i=0;
    if (i>1000) i=1000;
    turtle_speed=i;
    return i;
  }  
  static const char _speed_s []="speed";
  static define_unary_function_eval2 (__speed,&_speed,_speed_s,&printastifunction);
  define_unary_function_ptr5( at_speed ,alias_at_speed,&__speed,0,T_LOGO);

  gen _avance(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    double i;
    if (g.type!=_INT_){
      if (g.type==_VECT)
	i=(*turtleptr).turtle_length;
      else {
	gen g1=evalf_double(g,1,contextptr);
	if (g1.type==_DOUBLE_)
	  i=g1._DOUBLE_val;
	else
	  return gensizeerr(contextptr);
      }
    }
    else
      i=g.val;
    (*turtleptr).x += i * std::cos((*turtleptr).theta*deg2rad_d);
    (*turtleptr).y += i * std::sin((*turtleptr).theta*deg2rad_d) ;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _avance_s []="avance";
  static define_unary_function_eval2 (__avance,&_avance,_avance_s,&printastifunction);
  define_unary_function_ptr5( at_avance ,alias_at_avance,&__avance,0,T_LOGO);

  static const char _forward_s []="forward";
  static define_unary_function_eval (__forward,&_avance,_forward_s);
  define_unary_function_ptr5( at_forward ,alias_at_forward,&__forward,0,true);

  gen _recule(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    if (g.type==_VECT)
      return _avance(-(*turtleptr).turtle_length,contextptr);
    return _avance(-g,contextptr);
  }
  static const char _recule_s []="recule";
  static define_unary_function_eval2 (__recule,&_recule,_recule_s,&printastifunction);
  define_unary_function_ptr5( at_recule ,alias_at_recule,&__recule,0,T_LOGO);

  static const char _backward_s []="backward";
  static define_unary_function_eval (__backward,&_recule,_backward_s);
  define_unary_function_ptr5( at_backward ,alias_at_backward,&__backward,0,true);

  gen _position(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    if (g.type!=_VECT)
      return makevecteur((*turtleptr).x,(*turtleptr).y);
    // return turtle_state();
    vecteur v = *g._VECTptr;
    int s=int(v.size());
    if (!s)
      return makevecteur((*turtleptr).x,(*turtleptr).y);
    v[0]=evalf_double(v[0],1,contextptr);
    if (s>1)
      v[1]=evalf_double(v[1],1,contextptr);
    if (s>2)
      v[2]=evalf_double(v[2],1,contextptr); 
    if (set_turtle_state(v,contextptr))
      return update_turtle_state(true,contextptr);
    return zero;
  }
  static const char _position_s []="position";
  static define_unary_function_eval2 (__position,&_position,_position_s,&printastifunction);
  define_unary_function_ptr5( at_position ,alias_at_position,&__position,0,T_LOGO);

  gen _cap(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    gen gg=evalf_double(g,1,contextptr);
    if (gg.type!=_DOUBLE_)
      return double((*turtleptr).theta);
    (*turtleptr).theta=gg._DOUBLE_val;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _cap_s []="cap";
  static define_unary_function_eval2 (__cap,&_cap,_cap_s,&printastifunction);
  define_unary_function_ptr5( at_cap ,alias_at_cap,&__cap,0,T_LOGO);

  static const char _heading_s []="heading";
  static define_unary_function_eval (__heading,&_cap,_heading_s);
  define_unary_function_ptr5( at_heading ,alias_at_heading,&__heading,0,true);


  gen _tourne_droite(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    if (g.type!=_INT_){
      if (g.type==_VECT)
	(*turtleptr).theta -= 90;
      else {
	gen g1=evalf_double(g,1,contextptr);
	if (g1.type==_DOUBLE_)
	  (*turtleptr).theta -= g1._DOUBLE_val;
	else
	  return gensizeerr(contextptr);
      }
    }
    else
      (*turtleptr).theta -= g.val;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _tourne_droite_s []="tourne_droite";
  static define_unary_function_eval2 (__tourne_droite,&_tourne_droite,_tourne_droite_s,&printastifunction);
  define_unary_function_ptr5( at_tourne_droite ,alias_at_tourne_droite,&__tourne_droite,0,T_LOGO);

  gen _tourne_gauche(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    if (g.type==_VECT){
      (*turtleptr).theta += 90;
      (*turtleptr).radius = 0;
      return update_turtle_state(true,contextptr);
    }
    return _tourne_droite(-g,contextptr);
  }
  static const char _tourne_gauche_s []="tourne_gauche";
  static define_unary_function_eval2 (__tourne_gauche,&_tourne_gauche,_tourne_gauche_s,&printastifunction);
  define_unary_function_ptr5( at_tourne_gauche ,alias_at_tourne_gauche,&__tourne_gauche,0,T_LOGO);

  gen _leve_crayon(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    (*turtleptr).mark = false;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _leve_crayon_s []="leve_crayon";
  static define_unary_function_eval2 (__leve_crayon,&_leve_crayon,_leve_crayon_s,&printastifunction);
  define_unary_function_ptr5( at_leve_crayon ,alias_at_leve_crayon,&__leve_crayon,0,T_LOGO);

  static const char _penup_s []="penup";
  static define_unary_function_eval (__penup,&_leve_crayon,_penup_s);
  define_unary_function_ptr5( at_penup ,alias_at_penup,&__penup,0,T_LOGO);

  gen _baisse_crayon(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    (*turtleptr).mark = true;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _baisse_crayon_s []="baisse_crayon";
  static define_unary_function_eval2 (__baisse_crayon,&_baisse_crayon,_baisse_crayon_s,&printastifunction);
  define_unary_function_ptr5( at_baisse_crayon ,alias_at_baisse_crayon,&__baisse_crayon,0,T_LOGO);

  static const char _pendown_s []="pendown";
  static define_unary_function_eval (__pendown,&_baisse_crayon,_pendown_s);
  define_unary_function_ptr5( at_pendown ,alias_at_pendown,&__pendown,0,T_LOGO);

  vector<string> * ecrisptr=0;
  vector<string> & ecristab(){
    if (!ecrisptr)
      ecrisptr=new vector<string>;
    return * ecrisptr;
  }
  gen _ecris(const gen & g,GIAC_CONTEXT){    
    if ( g.type==_STRNG && g.subtype==-1) return  g;
#if 0 //def TURTLETAB
    return gensizeerr("String support does not work with static turtle table");
#endif
    // logo instruction
    (*turtleptr).radius=14;
    if (g.type==_VECT){ 
      vecteur & v =*g._VECTptr;
      int s=int(v.size());
      if (s==2 && v[1].type==_INT_){
	(*turtleptr).radius=absint(v[1].val);
	(*turtleptr).s=ecristab().size();
	ecristab().push_back(gen2string(v.front()));
	return update_turtle_state(false,contextptr);
      }
      if (s==4 && v[1].type==_INT_ && v[2].type==_INT_ && v[3].type==_INT_){
	logo_turtle t=(*turtleptr);
	_leve_crayon(0,contextptr);
	_position(makevecteur(v[2],v[3]),contextptr);
	(*turtleptr).radius=absint(v[1].val);
	(*turtleptr).s=ecristab().size();
	ecristab().push_back(gen2string(v.front()));
	update_turtle_state(false,contextptr);
	(*turtleptr)=t;
	return update_turtle_state(true,contextptr);
      }
    }
    (*turtleptr).s=ecristab().size();
    ecristab().push_back(gen2string(g));
    return update_turtle_state(false,contextptr);
  }
  static const char _ecris_s []="ecris";
  static define_unary_function_eval2 (__ecris,&_ecris,_ecris_s,&printastifunction);
  define_unary_function_ptr5( at_ecris ,alias_at_ecris,&__ecris,0,T_LOGO);

  gen _signe(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    return _ecris(makevecteur(g,20,10,10),contextptr);
  }
  static const char _signe_s []="signe";
  static define_unary_function_eval2 (__signe,&_signe,_signe_s,&printastifunction);
  define_unary_function_ptr5( at_signe ,alias_at_signe,&__signe,0,T_LOGO);

  gen _saute(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    _leve_crayon(0,contextptr);
    _avance(g,contextptr);
    return _baisse_crayon(0,contextptr);
  }
  static const char _saute_s []="saute";
  static define_unary_function_eval2 (__saute,&_saute,_saute_s,&printastifunction);
  define_unary_function_ptr5( at_saute ,alias_at_saute,&__saute,0,T_LOGO);

  static const char _jump_s []="jump";
  static define_unary_function_eval2 (__jump,&_saute,_jump_s,&printastifunction);
  define_unary_function_ptr5( at_jump ,alias_at_jump,&__jump,0,T_LOGO);

  gen _pas_de_cote(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    _leve_crayon(0,contextptr);
    _tourne_droite(-90,contextptr);
    _avance(g,contextptr);
    _tourne_droite(90,contextptr);
    return _baisse_crayon(0,contextptr);
  }
  static const char _pas_de_cote_s []="pas_de_cote";
  static define_unary_function_eval2 (__pas_de_cote,&_pas_de_cote,_pas_de_cote_s,&printastifunction);
  define_unary_function_ptr5( at_pas_de_cote ,alias_at_pas_de_cote,&__pas_de_cote,0,T_LOGO);

  static const char _skip_s []="skip";
  static define_unary_function_eval2 (__skip,&_pas_de_cote,_skip_s,&printastifunction);
  define_unary_function_ptr5( at_skip ,alias_at_skip,&__skip,0,T_LOGO);

  gen _cache_tortue(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    (*turtleptr).visible=false;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _cache_tortue_s []="cache_tortue";
  static define_unary_function_eval2 (__cache_tortue,&_cache_tortue,_cache_tortue_s,&printastifunction);
  define_unary_function_ptr5( at_cache_tortue ,alias_at_cache_tortue,&__cache_tortue,0,T_LOGO);

  static const char _hideturtle_s []="hideturtle";
  static define_unary_function_eval (__hideturtle,&_cache_tortue,_hideturtle_s);
  define_unary_function_ptr5( at_hideturtle ,alias_at_hideturtle,&__hideturtle,0,true);

  gen _montre_tortue(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    (*turtleptr).visible=true;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _montre_tortue_s []="montre_tortue";
  static define_unary_function_eval2 (__montre_tortue,&_montre_tortue,_montre_tortue_s,&printastifunction);
  define_unary_function_ptr5( at_montre_tortue ,alias_at_montre_tortue,&__montre_tortue,0,T_LOGO);

  static const char _showturtle_s []="showturtle";
  static define_unary_function_eval (__showturtle,&_montre_tortue,_showturtle_s);
  define_unary_function_ptr5( at_showturtle ,alias_at_showturtle,&__showturtle,0,true);


  gen _repete(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type!=_VECT || g._VECTptr->size()<2)
      return gensizeerr(contextptr);
    // logo instruction
    vecteur v = *g._VECTptr;
    v[0]=eval(v[0],contextptr);
    if (v.front().type!=_INT_)
      return gentypeerr(contextptr);
    gen prog=vecteur(v.begin()+1,v.end());
    int i=absint(v.front().val);
    gen res;
    for (int j=0;j<i;++j){
      res=eval(prog,contextptr);
    }
    return res;
  }
  static const char _repete_s []="repete";
  static define_unary_function_eval_quoted (__repete,&_repete,_repete_s);
  define_unary_function_ptr5( at_repete ,alias_at_repete,&__repete,_QUOTE_ARGUMENTS,T_RETURN);

  gen _crayon(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_STRNG) return _crayon(gen(*g._STRNGptr,contextptr),contextptr);
    // logo instruction
    if (g.type==_VECT && g._VECTptr->size()==3)
      return _crayon(_rgb(g,contextptr),contextptr);
    if (g.type!=_INT_){
      gen res=(*turtleptr).color;
      res.subtype=_INT_COLOR;
      return res;
    }
    (*turtleptr).color=g.val;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _crayon_s []="crayon";
  static define_unary_function_eval2 (__crayon,&_crayon,_crayon_s,&printastifunction);
  define_unary_function_ptr5( at_crayon ,alias_at_crayon,&__crayon,0,T_LOGO);

  static const char _pencolor_s []="pencolor";
  static define_unary_function_eval (__pencolor,&_crayon,_pencolor_s);
  define_unary_function_ptr5( at_pencolor ,alias_at_pencolor,&__pencolor,0,T_LOGO);

  gen _efface_logo(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_INT_){
      _crayon(int(FL_WHITE),contextptr);
      _recule(g,contextptr);
      return _crayon(0,contextptr);
    }
    // logo instruction
    (*turtleptr) = logo_turtle();
#ifdef TURTLETAB
    turtle_stack_size=0;
#else
    turtle_stack().clear();
#endif
    ecristab().clear();
    if (g.type==_VECT && g._VECTptr->size()==2){
      vecteur v = *g._VECTptr;
      int s=int(v.size());
      v[0]=evalf_double(v[0],1,contextptr);
      if (s>1)
	v[1]=evalf_double(v[1],1,contextptr);
      (*turtleptr).mark = false; // leve_crayon
      (*turtleptr).radius = 0;
      update_turtle_state(true,contextptr);
      set_turtle_state(v,contextptr); // baisse_crayon
      update_turtle_state(true,contextptr);
      (*turtleptr).mark = true;
      (*turtleptr).radius = 0;
    }
    return update_turtle_state(true,contextptr);
  }
  static const char _efface_logo_s []="efface";
  static define_unary_function_eval2 (__efface_logo,&_efface_logo,_efface_logo_s,&printastifunction);
  define_unary_function_ptr5( at_efface_logo ,alias_at_efface_logo,&__efface_logo,0,T_LOGO);

  static const char _efface_s []="efface";
  static define_unary_function_eval2 (__efface,&_efface_logo,_efface_s,&printastifunction);
  define_unary_function_ptr5( at_efface ,alias_at_efface,&__efface,0,T_LOGO);

  static const char _reset_s []="reset";
  static define_unary_function_eval2 (__reset,&_efface_logo,_reset_s,&printastifunction);
  define_unary_function_ptr5( at_reset ,alias_at_reset,&__reset,0,T_LOGO);

  static const char _clearscreen_s []="clearscreen";
  static define_unary_function_eval2 (__clearscreen,&_efface_logo,_clearscreen_s,&printastifunction);
  define_unary_function_ptr5( at_clearscreen ,alias_at_clearscreen,&__clearscreen,0,T_LOGO);

  gen _debut_enregistrement(const gen &g,GIAC_CONTEXT){
    return undef;
  }
  static const char _debut_enregistrement_s []="debut_enregistrement";
  static define_unary_function_eval2 (__debut_enregistrement,&_debut_enregistrement,_debut_enregistrement_s,&printastifunction);
  define_unary_function_ptr5( at_debut_enregistrement ,alias_at_debut_enregistrement,&__debut_enregistrement,0,T_LOGO);

  static const char _fin_enregistrement_s []="fin_enregistrement";
  static define_unary_function_eval2 (__fin_enregistrement,&_debut_enregistrement,_fin_enregistrement_s,&printastifunction);
  define_unary_function_ptr5( at_fin_enregistrement ,alias_at_fin_enregistrement,&__fin_enregistrement,0,T_LOGO);

  static const char _turtle_stack_s []="turtle_stack";
  static define_unary_function_eval2 (__turtle_stack,&_debut_enregistrement,_turtle_stack_s,&printastifunction);
  define_unary_function_ptr5( at_turtle_stack ,alias_at_turtle_stack,&__turtle_stack,0,T_LOGO);

  gen _vers(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    if (g.type!=_VECT || g._VECTptr->size()!=2)
      return gensizeerr(contextptr);
    gen x=evalf_double(g._VECTptr->front(),1,contextptr),
      y=evalf_double(g._VECTptr->back(),1,contextptr);
    if (x.type!=_DOUBLE_ || y.type!=_DOUBLE_)
      return gensizeerr(contextptr);
    double xv=x._DOUBLE_val,yv=y._DOUBLE_val,xt=(*turtleptr).x,yt=(*turtleptr).y;
    double theta=atan2(yv-yt,xv-xt);
    return _cap(theta*180/M_PI,contextptr);
  }
  static const char _vers_s []="vers";
  static define_unary_function_eval2 (__vers,&_vers,_vers_s,&printastifunction);
  define_unary_function_ptr5( at_vers ,alias_at_vers,&__vers,0,T_LOGO);

  static int find_radius(const gen & g,int & r,int & theta2,bool & direct){
    int radius;
    direct=true;
    theta2 = 360 ;
    // logo instruction
    if (g.type==_VECT && !g._VECTptr->empty()){
      vecteur v = *g._VECTptr;
      bool seg=false;
      if (v.back()==at_segment){
	v.pop_back();
	seg=true;
      }
      if (v.size()<2)
	return RAND_MAX; // setdimerr(contextptr);
      if (v[0].type==_INT_)
	r=v[0].val;
      else {
	gen v0=evalf_double(v[0],1,context0);
	if (v0.type==_DOUBLE_)
	  r=int(v0._DOUBLE_val+0.5);
	else 
	  return RAND_MAX; // setsizeerr(contextptr);
      }
      if (r<0){
	r=-r;
	direct=false;
      }
      int theta1;
      if (v[1].type==_DOUBLE_)
	theta1=int(v[1]._DOUBLE_val+0.5);
      else { 
	if (v[1].type==_INT_)
	  theta1=v[1].val;
	else return RAND_MAX; // setsizeerr(contextptr);
      }
      while (theta1<0)
	theta1 += 360;
      if (v.size()>=3){
	if (v[2].type==_DOUBLE_)
	  theta2 = int(v[2]._DOUBLE_val+0.5);
	else {
	  if (v[2].type==_INT_)
	    theta2 = v[2].val;
	  else return RAND_MAX; // setsizeerr(contextptr);
	}
	while (theta2<0)
	  theta2 += 360;
	radius = giacmin(r,512) | (giacmin(theta1,360) << 9) | (giacmin(theta2,360) << 18 ) | (seg?(1<<28):0);
      }
      else {// angle 1=0
	theta2 = theta1;
	if (theta2<0)
	  theta2 += 360;
	radius = giacmin(r,512) | (giacmin(theta2,360) << 18 ) | (seg?(1<<28):0);
      }
      return radius;
    }
    radius = 10;
    if (g.type==_INT_)
      radius= (r=g.val);
    if (g.type==_DOUBLE_)
      radius= (r=int(g._DOUBLE_val));
    if (radius<=0){
      radius = -radius;
      direct=false;
    }
    radius = giacmin(radius,512 )+(360 << 18) ; // 2nd angle = 360 degrees
    return radius;
  }

  static void turtle_move(int r,int theta2,GIAC_CONTEXT){
    double theta0;
    if ((*turtleptr).direct)
      theta0=(*turtleptr).theta-90;
    else {
      theta0=(*turtleptr).theta+90;
      theta2=-theta2;
    }
    (*turtleptr).x += r*(std::cos(M_PI/180*(theta2+theta0))-std::cos(M_PI/180*theta0));
    (*turtleptr).y += r*(std::sin(M_PI/180*(theta2+theta0))-std::sin(M_PI/180*theta0));
    (*turtleptr).theta = (*turtleptr).theta+theta2 ;
    if ((*turtleptr).theta<0)
      (*turtleptr).theta += 360;
    if ((*turtleptr).theta>360)
      (*turtleptr).theta -= 360;
  }

  gen _rond(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    int r,theta2,tmpr;
    tmpr=find_radius(g,r,theta2,(*turtleptr).direct);
    if (tmpr==RAND_MAX)
      return gensizeerr(contextptr);
    (*turtleptr).radius=tmpr;
    turtle_move(r,theta2,contextptr);
    return update_turtle_state(true,contextptr);
  }
  static const char _rond_s []="rond";
  static define_unary_function_eval2 (__rond,&_rond,_rond_s,&printastifunction);
  define_unary_function_ptr5( at_rond ,alias_at_rond,&__rond,0,T_LOGO);

  gen _disque(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    int r,theta2,tmpr=find_radius(g,r,theta2,(*turtleptr).direct);
    if (tmpr==RAND_MAX)
      return gensizeerr(contextptr);
    (*turtleptr).radius=tmpr;
    turtle_move(r,theta2,contextptr);
    (*turtleptr).radius += 1 << 27;
    return update_turtle_state(true,contextptr);
  }
  static const char _disque_s []="disque";
  static define_unary_function_eval2 (__disque,&_disque,_disque_s,&printastifunction);
  define_unary_function_ptr5( at_disque ,alias_at_disque,&__disque,0,T_LOGO);

  gen _disque_centre(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    int r,theta2;
    bool direct;
    int radius=find_radius(g,r,theta2,direct);
    if (radius==RAND_MAX)
      return gensizeerr(contextptr);
    r=absint(r);
    _saute(r,contextptr);
    _tourne_gauche(direct?90:-90,contextptr);
    (*turtleptr).radius = radius;
    (*turtleptr).direct=direct;
    turtle_move(r,theta2,contextptr);
    (*turtleptr).radius += 1 << 27;
    update_turtle_state(true,contextptr);
    _tourne_droite(direct?90:-90,contextptr);
    return _saute(-r,contextptr);
  }
  static const char _disque_centre_s []="disque_centre";
  static define_unary_function_eval2 (__disque_centre,&_disque_centre,_disque_centre_s,&printastifunction);
  define_unary_function_ptr5( at_disque_centre ,alias_at_disque_centre,&__disque_centre,0,T_LOGO);

  gen _polygone_rempli(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_INT_){
      (*turtleptr).radius=-absint(g.val);
      if ((*turtleptr).radius<-1)
	return update_turtle_state(true,contextptr);
    }
    return gensizeerr(gettext("Integer argument >= 2"));
  }
  static const char _polygone_rempli_s []="polygone_rempli";
  static define_unary_function_eval2 (__polygone_rempli,&_polygone_rempli,_polygone_rempli_s,&printastifunction);
  define_unary_function_ptr5( at_polygone_rempli ,alias_at_polygone_rempli,&__polygone_rempli,0,T_LOGO);

  gen _rectangle_plein(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    gen gx=g,gy=g;
    if (g.type==_VECT && g._VECTptr->size()==2){
      gx=g._VECTptr->front();
      gy=g._VECTptr->back();
    }
    for (int i=0;i<2;++i){
      _avance(gx,contextptr);
      _tourne_droite(-90,contextptr);
      _avance(gy,contextptr);
      _tourne_droite(-90,contextptr);
    }
    //for (int i=0;i<turtle_stack().size();++i){ *logptr(contextptr) << turtle2gen(turtle_stack()[i]) <<endl;}
    return _polygone_rempli(-8,contextptr);
  }
  static const char _rectangle_plein_s []="rectangle_plein";
  static define_unary_function_eval2 (__rectangle_plein,&_rectangle_plein,_rectangle_plein_s,&printastifunction);
  define_unary_function_ptr5( at_rectangle_plein ,alias_at_rectangle_plein,&__rectangle_plein,0,T_LOGO);

  gen _triangle_plein(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    gen gx=g,gy=g,gtheta=60;
    if (g.type==_VECT && g._VECTptr->size()>=2){
      vecteur & v=*g._VECTptr;
      gx=v.front();
      gy=v[1];
      gtheta=90;
      if (v.size()>2)
	gtheta=v[2];
    }
    logo_turtle t=(*turtleptr);
    _avance(gx,contextptr);
    double save_x=(*turtleptr).x,save_y=(*turtleptr).y;
    _recule(gx,contextptr);
    _tourne_gauche(gtheta,contextptr);
    _avance(gy,contextptr);
    (*turtleptr).x=save_x;
    (*turtleptr).y=save_y;
    update_turtle_state(true,contextptr);
    (*turtleptr)=t;
    (*turtleptr).radius=0;
    update_turtle_state(true,contextptr);
    return _polygone_rempli(-3,contextptr);
  }
  static const char _triangle_plein_s []="triangle_plein";
  static define_unary_function_eval2 (__triangle_plein,&_triangle_plein,_triangle_plein_s,&printastifunction);
  define_unary_function_ptr5( at_triangle_plein ,alias_at_triangle_plein,&__triangle_plein,0,T_LOGO);

  gen _dessine_tortue(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    /*
      _triangle_plein(makevecteur(17,5));
      _tourne_droite(90);
      _triangle_plein(makevecteur(5,17));
      return _tourne_droite(-90);
    */
    double save_x=(*turtleptr).x,save_y=(*turtleptr).y;
    _tourne_droite(90,contextptr);
    _avance(5,contextptr);
    _tourne_gauche(106,contextptr);
    _avance(18,contextptr);
    _tourne_gauche(148,contextptr);
    _avance(18,contextptr);
    _tourne_gauche(106,contextptr);
    _avance(5,contextptr);
    (*turtleptr).x=save_x; (*turtleptr).y=save_y;
    gen res(_tourne_gauche(90,contextptr));
    if (is_one(g))
      return res;
    return _polygone_rempli(-9,contextptr);
  }
  static const char _dessine_tortue_s []="dessine_tortue";
  static define_unary_function_eval2 (__dessine_tortue,&_dessine_tortue,_dessine_tortue_s,&printastifunction);
  define_unary_function_ptr5( at_dessine_tortue ,alias_at_dessine_tortue,&__dessine_tortue,0,T_LOGO);
  
#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC


#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS
  void drawRectangle(int x,int y,int w,int h,int c){
    draw_rectangle(x,y,w,h,c,context0);
  }
  void draw_rectangle(int x,int y,int w,int h,int c){
    draw_rectangle(x,y,w,h,c,context0);
  }
  void draw_line(int x0,int y0,int x1,int y1,int c){
    if (x0==x1){
      if (y0<=y1)
	draw_rectangle(x0,y0,1,y1-y0+1,c);
      else
	draw_rectangle(x0,y1,1,y0-y1+1,c);
    }
    else {
      if (y0==y1){
	if (x0<=x1)
	  draw_rectangle(x0,y0,x1-x0+1,1,c);
	else
	  draw_rectangle(x1,y0,x0-x1+1,1,c);
      }
      else
	draw_line(x0,y0,x1,y1,c,context0);
    }
  }
  void draw_circle(int xc,int yc,int r,int color,bool q1,bool q2,bool q3,bool q4){
    draw_circle(xc,yc,r,color,q1,q2,q3,q4,context0);
  }
  void draw_filled_circle(int xc,int yc,int r,int color,bool left,bool right){
    draw_filled_circle(xc,yc,r,color,left,right,context0);
  }
  void draw_polygon(std::vector< std::vector<int> > & v1,int color){
    draw_polygon(v1,color,context0);
  }
  void draw_filled_polygon(std::vector< vector<int> > &L,int xmin,int xmax,int ymin,int ymax,int color){
    draw_filled_polygon(L,xmin,xmax,ymin,ymax,color,context0);
  }
  void draw_arc(int xc,int yc,int rx,int ry,int color,double theta1, double theta2){
    draw_arc(xc,yc,rx,ry,color,theta1,theta2,giac::context0);
  }
  void draw_filled_arc(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int color,int xmin,int xmax,int ymin,int ymax,bool segment){
    draw_filled_arc(x,y,rx,ry,theta1_deg,theta2_deg,color,xmin,xmax,ymin,ymax,segment,context0);
  }


  unsigned max_prettyprint_equation=256;

  // make a free copy of g
  gen Equation_copy(const gen & g){
    if (g.type==_EQW)
      return *g._EQWptr;
    if (g.type!=_VECT)
      return g;
    vecteur & v = *g._VECTptr;
    const_iterateur it=v.begin(),itend=v.end();
    vecteur res;
    res.reserve(itend-it);
    for (;it!=itend;++it)
      res.push_back(Equation_copy(*it));
    return gen(res,g.subtype);
  }

  // matrix/list select
  bool do_select(gen & eql,bool select,gen & value){
    if (eql.type==_VECT && !eql._VECTptr->empty()){
      vecteur & v=*eql._VECTptr;
      size_t s=v.size();
      if (v[s-1].type!=_EQW)
	return false;
      v[s-1]._EQWptr->selected=select;
      gen sommet=v[s-1]._EQWptr->g;
      --s;
      vecteur args(s);
      for (size_t i=0;i<s;++i){
	if (!do_select(v[i],select,args[i]))
	  return false;
	if (args[i].type==_EQW)
	  args[i]=args[i]._EQWptr->g;
      }
      gen va=s==1?args[0]:gen(args,_SEQ__VECT);
      if (sommet.type==_FUNC)
	va=symbolic(*sommet._FUNCptr,va);
      else
	va=sommet(va,context0);
      //cout << "va " << va << endl;
      value=*v[s]._EQWptr;
      value._EQWptr->g=va;
      //cout << "value " << value << endl;
      return true;
    }
    if (eql.type!=_EQW)
      return false;
    eql._EQWptr->selected=select;
    value=eql;
    return true;
  }
  
  bool Equation_box_sizes(const gen & g,int & l,int & h,int & x,int & y,attributs & attr,bool & selected){
    if (g.type==_EQW){
      eqwdata & w=*g._EQWptr;
      x=w.x;
      y=w.y;
      l=w.dx;
      h=w.dy;
      selected=w.selected;
      attr=w.eqw_attributs;
      //cout << g << endl;
      return true;
    }
    else {
      if (g.type!=_VECT || g._VECTptr->empty() ){
	l=0;
	h=0;
	x=0;
	y=0;
	attr=attributs(0,0,0);
	selected=false;
	return true;
      }
      gen & g1=g._VECTptr->back();
      Equation_box_sizes(g1,l,h,x,y,attr,selected);
      return false;
    }
  }

  // return true if g has some selection inside, gsel points to the selection
  bool Equation_adjust_xy(gen & g,int & xleft,int & ytop,int & xright,int & ybottom,gen * & gsel,gen * & gselparent,int &gselpos,std::vector<int> * goto_ptr){
    gsel=0;
    gselparent=0;
    gselpos=0;
    int x,y,w,h;
    attributs f(0,0,0);
    bool selected;
    Equation_box_sizes(g,w,h,x,y,f,selected);
    if ( (g.type==_EQW__VECT) || selected ){ // terminal or selected
      xleft=x;
      ybottom=y;
      if (selected){ // g is selected
	ytop=y+h;
	xright=x+w;
	gsel =  &g;
	//cout << "adjust " << *gsel << endl;
	return true;
      }
      else { // no selection
	xright=x;
	ytop=y;
	return false;
      }
    }
    if (g.type!=_VECT)
      return false;
    // last not selected, recurse
    iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end()-1;
    for (;it!=itend;++it){
      if (Equation_adjust_xy(*it,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,goto_ptr)){
	if (goto_ptr){
	  goto_ptr->push_back(it-g._VECTptr->begin());
	  //cout << g << ":" << *goto_ptr << endl;
	}
	if (gsel==&*it){
	  // check next siblings
	  
	  gselparent= &g;
	  gselpos=it-g._VECTptr->begin();
	  //cout << "gselparent " << g << endl;
	}
	return true;
      }
    }
    return false;
  }
 
  // select or deselect part of the current eqution
  // This is done *in place*
  void Equation_select(gen & g,bool select){
    if (g.type==_EQW){
      eqwdata & e=*g._EQWptr;
      e.selected=select;
    }
    if (g.type!=_VECT)
      return;
    vecteur & v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      Equation_select(*it,select);
  }

  // decrease selection (like HP49 eqw Down key)
  int eqw_select_down(gen & g){
    int xleft,ytop,xright,ybottom,gselpos;
    int newxleft,newytop,newxright,newybottom;
    gen * gsel,*gselparent;
    if (Equation_adjust_xy(g,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos)){
      //cout << "select down before " << *gsel << endl;
      if (gsel->type==_VECT && !gsel->_VECTptr->empty()){
	Equation_select(*gsel,false);
	Equation_select(gsel->_VECTptr->front(),true);
	//cout << "select down after " << *gsel << endl;
	Equation_adjust_xy(g,newxleft,newytop,newxright,newybottom,gsel,gselparent,gselpos);
	return newytop-ytop;
      }
    }
    return 0;
  }

  int eqw_select_up(gen & g){
    int xleft,ytop,xright,ybottom,gselpos;
    int newxleft,newytop,newxright,newybottom;
    gen * gsel,*gselparent;
    if (Equation_adjust_xy(g,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos) && gselparent){
      Equation_select(*gselparent,true);
      //cout << "gselparent " << *gselparent << endl;
      Equation_adjust_xy(g,newxleft,newytop,newxright,newybottom,gsel,gselparent,gselpos);
      return newytop-ytop;
    }
    return false;
  }

  // exchange==0 move selection to left or right sibling, ==2 add left or right
  // sibling, ==1 exchange selection with left or right sibling
  int eqw_select_leftright(Equation & eq,bool left,int exchange,GIAC_CONTEXT){
    gen & g=eq.data;
    int xleft,ytop,xright,ybottom,gselpos;
    int newxleft,newytop,newxright,newybottom;
    gen * gsel,*gselparent;
    vector<int> goto_sel;
    if (Equation_adjust_xy(g,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gselparent && gselparent->type==_VECT){
      vecteur & gselv=*gselparent->_VECTptr;
      int n=gselv.size()-1,gselpos_orig=gselpos;
      if (n<1) return 0;
      if (left) {
	if (gselpos==0)
	  gselpos=n-1;
	else
	  gselpos--;
      }
      else {
	if (gselpos==n-1)
	  gselpos=0;
	else
	  gselpos++;
      }
      if (exchange==1){ // exchange gselpos_orig and gselpos
	swapgen(gselv[gselpos],gselv[gselpos_orig]);
	gsel=&gselv[gselpos_orig];
	gen value;
	if (xcas::do_select(*gsel,true,value) && value.type==_EQW)
	  replace_selection(eq,value._EQWptr->g,gsel,&goto_sel,contextptr);
      }
      else {
	// increase selection to next sibling possible for + and * only
	if (n>2 && exchange==2 && gselv[n].type==_EQW && (gselv[n]._EQWptr->g==at_plus || gselv[n]._EQWptr->g==at_prod)){
	  gen value1, value2,tmp;
	  if (gselpos_orig<gselpos)
	    swapint(gselpos_orig,gselpos);
	  // now gselpos<gselpos_orig
	  xcas::do_select(gselv[gselpos_orig],true,value1);
	  xcas::do_select(gselv[gselpos],true,value2);
	  if (value1.type==_EQW && value2.type==_EQW){
	    tmp=gselv[n]._EQWptr->g==at_plus?value1._EQWptr->g+value2._EQWptr->g:value1._EQWptr->g*value2._EQWptr->g;
	    gselv.erase(gselv.begin()+gselpos_orig);
	    replace_selection(eq,tmp,&gselv[gselpos],&goto_sel,contextptr);
	  }
	}
	else {
	  Equation_select(*gselparent,false);
	  gen & tmp=(*gselparent->_VECTptr)[gselpos];
	  Equation_select(tmp,true);
	}
      }
      Equation_adjust_xy(g,newxleft,newytop,newxright,newybottom,gsel,gselparent,gselpos);
      return newxleft-xleft;
    }
    return 0;
  }

  bool eqw_select(const gen & eq,int l,int c,bool select,gen & value){
    value=undef;
    if (l<0 || eq.type!=_VECT || eq._VECTptr->size()<=l)
      return false;
    gen & eql=(*eq._VECTptr)[l];
    if (c<0)
      return do_select(eql,select,value);
    if (eql.type!=_VECT || eql._VECTptr->size()<=c)
      return false;
    gen & eqlc=(*eql._VECTptr)[c];
    return do_select(eqlc,select,value);
  }

  gen Equation_compute_size(const gen & g,const attributs & a,int windowhsize,GIAC_CONTEXT);
  
  // void Bdisp_MMPrint(int x, int y, const char* string, int mode_flags, int xlimit, int P6, int P7, int color, int back_color, int writeflag, int P11); 
  // void PrintCXY(int x, int y, const char *cptr, int mode_flags, int P5, int color, int back_color, int P8, int P9)
  // void PrintMini( int* x, int* y, const char* string, int mode_flags, unsigned int xlimit, int P6, int P7, int color, int back_color, int writeflag, int P11) 
  void text_print(int fontsize,const char * s,int x,int y,int c=COLOR_BLACK,int bg=COLOR_WHITE,int mode=0){
    // *logptr(contextptr) << x << " " << y << " " << fontsize << " " << s << endl; return;
    c=(unsigned short) c;
    if (x>LCD_WIDTH_PX) return;
    int ss=strlen(s);
    if (ss==1 && s[0]==0x1e){ // arrow for limit
      if (mode==4)
	c=bg;
      draw_line(x,y-4,x+fontsize/2,y-4,c);
      draw_line(x,y-3,x+fontsize/2,y-3,c);
      draw_line(x+fontsize/2-4,y,x+fontsize/2,y-4,c);
      draw_line(x+fontsize/2-3,y,x+fontsize/2+1,y-4,c);
      draw_line(x+fontsize/2-4,y-7,x+fontsize/2,y-3,c);   
      draw_line(x+fontsize/2-3,y-7,x+fontsize/2+1,y-3,c);   
      return;
    }
    if (ss==2 && strcmp(s,"pi")==0){
      if (mode==4){
	drawRectangle(x,y+2-fontsize,fontsize,fontsize,c);
	c=bg;
      }
      draw_line(x+fontsize/3-1,y+1,x+fontsize/3,y+6-fontsize,c);
      draw_line(x+fontsize/3-2,y+1,x+fontsize/3-1,y+6-fontsize,c);
      draw_line(x+2*fontsize/3,y+1,x+2*fontsize/3,y+6-fontsize,c);
      draw_line(x+2*fontsize/3+1,y+1,x+2*fontsize/3+1,y+6-fontsize,c);
      draw_line(x+2,y+6-fontsize,x+fontsize,y+6-fontsize,c);
      draw_line(x+2,y+5-fontsize,x+fontsize,y+5-fontsize,c);
      return;
    }
    if (fontsize>=16 && ss==2 && s[0]==char(0xe5) && (s[1]==char(0xea) || s[1]==char(0xeb))) // special handling for increasing and decreasing in tabvar output
      fontsize=18;
    if (fontsize>=18){
      y -= 16;// status area shift
      os_draw_string(x,y,mode==4?bg:c,mode==4?c:bg,s);
      // PrintMini(&x,&y,(unsigned char *)s,mode,0xffffffff,0,0,c,bg,1,0);
      return;
    }
    y -= 12;
    x=os_draw_string_small(x,y,mode==4?bg:c,mode==4?c:bg,s);// PrintMiniMini( &x, &y, (unsigned char *)s, mode,c, 0 );
    return;
  }
  
  int text_width(int fontsize,const char * s){
#ifdef NSPIRE_NEWLIB
    int x=0;
    if (fontsize>=18)
      x=os_draw_string(0,0,0,1,s,true);
    else
      x=os_draw_string_small(0,0,0,1,s,true);
    return x;
#else
    if (fontsize>=18)
      return strlen(s)*11;
    return strlen(s)*7;
#endif
  }

  void fl_arc(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int c=COLOR_BLACK){
    rx/=2;
    ry/=2;
    // *logptr(contextptr) << "theta " << theta1_deg << " " << theta2_deg << endl;
    if (ry==rx){
      if (theta2_deg-theta1_deg==360){
	draw_circle(x+rx,y+rx,rx,c);
	return;
      }
      if (theta1_deg==0 && theta2_deg==180){
	draw_circle(x+rx,y+rx,rx,c,true,true,false,false);
	return;
      }
      if (theta1_deg==180 && theta2_deg==360){
	draw_circle(x+rx,y+rx,rx,c,false,false,true,true);
	return;
      }
    }
    // *logptr(contextptr) << "draw_arc" << theta1_deg*M_PI/180. << " " << theta2_deg*M_PI/180. << endl;
    draw_arc(x+rx,y+ry,rx,ry,c,theta1_deg*M_PI/180.,theta2_deg*M_PI/180.,context0);
  }

  void fl_pie(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int c=COLOR_BLACK,bool segment=false){
    //cout << "fl_pie " << theta1_deg << " " << theta2_deg << " " << c << endl;
    if (!segment && ry==rx){
      if (theta2_deg-theta1_deg>=360){
	rx/=2;
	draw_filled_circle(x+rx,y+rx,rx,c);
	return;
      }
      if (theta1_deg==-90 && theta2_deg==90){
	rx/=2;
	draw_filled_circle(x+rx,y+rx,rx,c,false,true);
	return;
      }
      if (theta1_deg==90 && theta2_deg==270){
	rx/=2;
	draw_filled_circle(x+rx,y+rx,rx,c,true,false);
	return;
      }
    }
    // approximation by a filled polygon
    // points: (x,y), (x+rx*cos(theta)/2,y+ry*sin(theta)/2) theta=theta1..theta2
    while (theta2_deg<theta1_deg)
      theta2_deg+=360;
    if (theta2_deg-theta1_deg>=360){
      theta1_deg=0;
      theta2_deg=360;
    }
    int N0=theta2_deg-theta1_deg+1;
    // reduce N if rx or ry is small
    double red=double(rx)/LCD_WIDTH_PX*double(ry)/LCD_HEIGHT_PX;
    if (red>1) red=1;
    if (red<0.1) red=0.1;
    int N=red*N0;
    if (N<5)
      N=N0>5?5:N0;
    if (N<2)
      N=2;
    vector< vector<int> > v(segment?N+1:N+2,vector<int>(2));
    x += rx/2;
    y += ry/2;
    int i=0;
    if (!segment){
      v[0][0]=x;
      v[0][1]=y;
      ++i;
    }
    double theta=theta1_deg*M_PI/180;
    double thetastep=(theta2_deg-theta1_deg)*M_PI/(180*(N-1));
    for (;i<v.size()-1;++i){
      v[i][0]=int(x+rx*std::cos(theta)/2+.5);
      v[i][1]=int(y-ry*std::sin(theta)/2+.5); // y is inverted
      theta += thetastep;
    }
    v.back()=v.front();
    draw_filled_polygon(v,0,LCD_WIDTH_PX,24,LCD_HEIGHT_PX,c);
  }

  bool binary_op(const unary_function_ptr & u){
    const unary_function_ptr binary_op_tab_ptr []={*at_plus,*at_prod,*at_pow,*at_and,*at_ou,*at_xor,*at_different,*at_same,*at_equal,*at_unit,*at_compose,*at_composepow,*at_deuxpoints,*at_tilocal,*at_pointprod,*at_pointdivision,*at_pointpow,*at_division,*at_normalmod,*at_minus,*at_intersect,*at_union,*at_interval,*at_inferieur_egal,*at_inferieur_strict,*at_superieur_egal,*at_superieur_strict,*at_equal2,0};
    return equalposcomp(binary_op_tab_ptr,u);
  }
  
  eqwdata Equation_total_size(const gen & g){
    if (g.type==_EQW)
      return *g._EQWptr;
    if (g.type!=_VECT || g._VECTptr->empty())
      return eqwdata(0,0,0,0,attributs(0,0,0),undef);
    return Equation_total_size(g._VECTptr->back());
  }

  // find smallest value of y and height
  void Equation_y_dy(const gen & g,int & y,int & dy){
    y=0; dy=0;
    if (g.type==_EQW){
      y=g._EQWptr->y;
      dy=g._EQWptr->dy;
    }
    if (g.type==_VECT){
      iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      for (;it!=itend;++it){
	int Y,dY;
	Equation_y_dy(*it,Y,dY);
	// Y, Y+dY and y,y+dy
	int ymax=giacmax(y+dy,Y+dY);
	if (Y<y)
	  y=Y;
	dy=ymax-y;
      }
    }
  }

  void Equation_translate(gen & g,int deltax,int deltay){
    if (g.type==_EQW){
      g._EQWptr->x += deltax;
      g._EQWptr->y += deltay;
      g._EQWptr->baseline += deltay;
      return ;
    }
    if (g.type!=_VECT)
      setsizeerr();
    vecteur & v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      Equation_translate(*it,deltax,deltay);
  }

  gen Equation_change_attributs(const gen & g,const attributs & newa){
    if (g.type==_EQW){
      gen res(*g._EQWptr);
      res._EQWptr->eqw_attributs = newa;
      return res;
    }
    if (g.type!=_VECT)
      return gensizeerr();
    vecteur v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      *it=Equation_change_attributs(*it,newa);
    return gen(v,g.subtype);
  }

  vecteur Equation_subsizes(const gen & arg,const attributs & a,int windowhsize,GIAC_CONTEXT){
    vecteur v;
    if ( (arg.type==_VECT) && ( (arg.subtype==_SEQ__VECT) 
				// || (!ckmatrix(arg)) 
				) ){
      const_iterateur it=arg._VECTptr->begin(),itend=arg._VECTptr->end();
      for (;it!=itend;++it)
	v.push_back(Equation_compute_size(*it,a,windowhsize,contextptr));
    }
    else {
      v.push_back(Equation_compute_size(arg,a,windowhsize,contextptr));
    }
    return v;
  }

  // vertical merge with same baseline
  // for vertical merge of hp,yp at top (like ^) add fontsize to yp
  // at bottom (like lower bound of int) substract fontsize from yp
  void Equation_vertical_adjust(int hp,int yp,int & h,int & y){
    int yf=min(y,yp);
    h=max(y+h,yp+hp)-yf;
    y=yf;
  }

  gen Equation_compute_symb_size(const gen & g,const attributs & a,int windowhsize,GIAC_CONTEXT){
    if (g.type!=_SYMB)
      return Equation_compute_size(g,a,windowhsize,contextptr);
    unary_function_ptr & u=g._SYMBptr->sommet;
    gen arg=g._SYMBptr->feuille,rootof_value;
    if (u==at_makevector){
      vecteur v(1,arg);
      if (arg.type==_VECT)
	v=*arg._VECTptr;
      iterateur it=v.begin(),itend=v.end();
      for (;it!=itend;++it){
	if ( (it->type==_SYMB) && (it->_SYMBptr->sommet==at_makevector) )
	  *it=_makevector(it->_SYMBptr->feuille,contextptr);
      }
      return Equation_compute_size(v,a,windowhsize,contextptr);
    }
    if (u==at_makesuite){
      if (arg.type==_VECT)
	return Equation_compute_size(gen(*arg._VECTptr,_SEQ__VECT),a,windowhsize,contextptr);
      else
	return Equation_compute_size(arg,a,windowhsize,contextptr);
    }
    if (u==at_sqrt)
      return Equation_compute_size(symb_pow(arg,plus_one_half),a,windowhsize,contextptr);
    if (u==at_division){
      if (arg.type!=_VECT || arg._VECTptr->size()!=2)
	return Equation_compute_size(arg,a,windowhsize,contextptr);
      gen tmp=Tfraction<gen>(arg._VECTptr->front(),arg._VECTptr->back());
      return Equation_compute_size(tmp,a,windowhsize,contextptr);
    }
    if (u==at_prod){
      gen n,d;
      if (rewrite_prod_inv(arg,n,d)){
	if (n.is_symb_of_sommet(at_neg))
	  return Equation_compute_size(symbolic(at_neg,Tfraction<gen>(-n,d)),a,windowhsize,contextptr);
	return Equation_compute_size(Tfraction<gen>(n,d),a,windowhsize,contextptr);
      }
    }
    if (u==at_inv){
      if ( (is_integer(arg) && is_positive(-arg,contextptr))
	   || (arg.is_symb_of_sommet(at_neg)))
	return Equation_compute_size(symbolic(at_neg,Tfraction<gen>(plus_one,-arg)),a,windowhsize,contextptr);
      return Equation_compute_size(Tfraction<gen>(plus_one,arg),a,windowhsize,contextptr);
    }
    if (u==at_expr && arg.type==_VECT && arg.subtype==_SEQ__VECT && arg._VECTptr->size()==2 && arg._VECTptr->back().type==_INT_){
      gen varg1=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv(Equation_total_size(varg1));
      gen varg2=eqwdata(0,0,0,0,a,arg._VECTptr->back());
      vecteur v12(makevecteur(varg1,varg2));
      v12.push_back(eqwdata(vv.dx,vv.dy,0,vv.y,a,at_expr,0));
      return gen(v12,_SEQ__VECT);
    }
    int llp=int(text_width(a.fontsize,("(")))-1;
    int lrp=llp;
    int lc=int(text_width(a.fontsize,(",")));
    string us=u.ptr()->s;
    int ls=int(text_width(a.fontsize,(us.c_str())));
    // if (isalpha(u.ptr()->s[0])) ls += 1;
    if (u==at_abs)
      ls = 2;
    // special cases first int, sigma, /, ^
    // and if printed as printsommetasoperator
    // otherwise print with usual functional notation
    int x=0;
    int h=a.fontsize;
    int y=0;
#if 1
    if ((u==at_integrate) || (u==at_sum) ){ // Int
      int s=1;
      if (arg.type==_VECT)
	s=arg._VECTptr->size();
      else
	arg=vecteur(1,arg);
      // s==1 -> general case
      if ( (s==1) || (s==2) ){ // int f(x) dx and sum f(n) n
	vecteur v(Equation_subsizes(gen(*arg._VECTptr,_SEQ__VECT),a,windowhsize,contextptr));
	eqwdata vv(Equation_total_size(v[0]));
	if (s==1){
	  x=a.fontsize;
	  Equation_translate(v[0],x,0);
	  x += int(text_width(a.fontsize,(" dx")));
	}
	if (s==2){
	  if (u==at_integrate){
	    x=a.fontsize;
	    Equation_translate(v[0],x,0);
	    x += vv.dx+int(text_width(a.fontsize,(" d")));
	    Equation_vertical_adjust(vv.dy,vv.y,h,y);
	    vv=Equation_total_size(v[1]);
	    Equation_translate(v[1],x,0);
	    Equation_vertical_adjust(vv.dy,vv.y,h,y);
	  }
	  else {
	    Equation_vertical_adjust(vv.dy,vv.y,h,y);
	    eqwdata v1=Equation_total_size(v[1]);
	    x=max((int)a.fontsize,(int)v1.dx)+2*a.fontsize/3; // var name size
	    Equation_translate(v[1],0,-v1.dy-v1.y);
	    Equation_vertical_adjust(v1.dy,-v1.dy,h,y);
	    Equation_translate(v[0],x,0);
	    x += vv.dx; // add function size
	  }
	}
	if (u==at_integrate){
	  x += vv.dx;
	  if (h==a.fontsize)
	    h+=2*a.fontsize/3;
	  if (y==0){
	    y=-2*a.fontsize/3;
	    h+=2*a.fontsize/3;
	  }
	}
	v.push_back(eqwdata(x,h,0,y,a,u,0));
	return gen(v,_SEQ__VECT);
      }
      if (s>=3){ // int _a^b f(x) dx
	vecteur & intarg=*arg._VECTptr;
	gen tmp_l,tmp_u,tmp_f,tmp_x;
	attributs aa(a);
	if (a.fontsize>=10)
	  aa.fontsize -= 2;
	tmp_f=Equation_compute_size(intarg[0],a,windowhsize,contextptr);
	tmp_x=Equation_compute_size(intarg[1],a,windowhsize,contextptr);
	tmp_l=Equation_compute_size(intarg[2],aa,windowhsize,contextptr);
	if (s==4)
	  tmp_u=Equation_compute_size(intarg[3],aa,windowhsize,contextptr);
	x=a.fontsize+(u==at_integrate?-2:+4);
	eqwdata vv(Equation_total_size(tmp_l));
	Equation_translate(tmp_l,x,-vv.y-vv.dy);
	vv=Equation_total_size(tmp_l);
	Equation_vertical_adjust(vv.dy,vv.y,h,y);
	int lx = vv.dx;
	if (s==4){
	  vv=Equation_total_size(tmp_u);
	  Equation_translate(tmp_u,x,a.fontsize-3-vv.y);
	  vv=Equation_total_size(tmp_u);
	  Equation_vertical_adjust(vv.dy,vv.y,h,y);
	}
	x += max(lx,(int)vv.dx);
	Equation_translate(tmp_f,x,0);
	vv=Equation_total_size(tmp_f);
	Equation_vertical_adjust(vv.dy,vv.y,h,y);
	if (u==at_integrate){
	  x += vv.dx+int(text_width(a.fontsize,(" d")));
	  Equation_translate(tmp_x,x,0);
	  vv=Equation_total_size(tmp_x);
	  Equation_vertical_adjust(vv.dy,vv.y,h,y);
	  x += vv.dx;
	}
	else {
	  x += vv.dx;
	  Equation_vertical_adjust(vv.dy,vv.y,h,y);
	  vv=Equation_total_size(tmp_x);
	  x=max(x,(int)vv.dx)+a.fontsize/3;
	  Equation_translate(tmp_x,0,-vv.dy-vv.y);
	  //Equation_translate(tmp_l,0,-1);	  
	  if (s==4) Equation_translate(tmp_u,-2,0);	  
	  Equation_vertical_adjust(vv.dy,-vv.dy,h,y);
	}
	vecteur res(makevecteur(tmp_f,tmp_x,tmp_l));
	if (s==4)
	  res.push_back(tmp_u);
	res.push_back(eqwdata(x,h,0,y,a,u,0));
	return gen(res,_SEQ__VECT);
      }
    }
    if (u==at_limit && arg.type==_VECT){ // limit
      vecteur limarg=*arg._VECTptr;
      int s=limarg.size();
      if (s==2 && limarg[1].is_symb_of_sommet(at_equal)){
	limarg.push_back(limarg[1]._SYMBptr->feuille[1]);
	limarg[1]=limarg[1]._SYMBptr->feuille[0];
	++s;
      }
      if (s>=3){
	gen tmp_l,tmp_f,tmp_x,tmp_dir;
	attributs aa(a);
	if (a.fontsize>=10)
	  aa.fontsize -= 2;
	tmp_f=Equation_compute_size(limarg[0],a,windowhsize,contextptr);
	tmp_x=Equation_compute_size(limarg[1],aa,windowhsize,contextptr);
	tmp_l=Equation_compute_size(limarg[2],aa,windowhsize,contextptr);
	if (s==4)
	  tmp_dir=Equation_compute_size(limarg[3],aa,windowhsize,contextptr);
	eqwdata vf(Equation_total_size(tmp_f));
	eqwdata vx(Equation_total_size(tmp_x));
	eqwdata vl(Equation_total_size(tmp_l));
	eqwdata vdir(Equation_total_size(tmp_dir));
	int sous=max(vx.dy,vl.dy);
	if (s==4)
	  Equation_translate(tmp_f,vx.dx+vl.dx+vdir.dx+a.fontsize+4,0);
	else
	  Equation_translate(tmp_f,vx.dx+vl.dx+a.fontsize+2,0);
	Equation_translate(tmp_x,0,-sous-vl.y);
	Equation_translate(tmp_l,vx.dx+a.fontsize+2,-sous-vl.y);
	if (s==4)
	  Equation_translate(tmp_dir,vx.dx+vl.dx+a.fontsize+4,-sous-vl.y);
	h=vf.dy;
	y=vf.y;
	vl=Equation_total_size(tmp_l);
	Equation_vertical_adjust(vl.dy,vl.y,h,y);
	vecteur res(makevecteur(tmp_f,tmp_x,tmp_l));
	if (s==4){
	  res.push_back(tmp_dir);
	  res.push_back(eqwdata(vf.dx+vx.dx+a.fontsize+4+vl.dx+vdir.dx,h,0,y,a,u,0));
	}
	else
	  res.push_back(eqwdata(vf.dx+vx.dx+a.fontsize+2+vl.dx,h,0,y,a,u,0));
	return gen(res,_SEQ__VECT);
      }
    }
#endif
    if ( (u==at_of || u==at_at) && arg.type==_VECT && arg._VECTptr->size()==2 ){
      // user function, function in 1st arg, arguments in 2nd arg
      gen varg1=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg1);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      gen arg2=arg._VECTptr->back();
      if (u==at_at && xcas_mode(contextptr)!=0){
	if (arg2.type==_VECT)
	  arg2=gen(addvecteur(*arg2._VECTptr,vecteur(arg2._VECTptr->size(),plus_one)),_SEQ__VECT);
	else
	  arg2=arg2+plus_one; 
      }
      gen varg2=Equation_compute_size(arg2,a,windowhsize,contextptr);
      Equation_translate(varg2,vv.dx+llp,0);
      vv=Equation_total_size(varg2);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      vecteur res(makevecteur(varg1,varg2));
      res.push_back(eqwdata(vv.dx+vv.x+lrp,h,0,y,a,u,0));
      return gen(res,_SEQ__VECT);
    }
    if (u==at_pow){ 
      // first arg not translated
      gen varg=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      // 1/2 ->sqrt, otherwise as exponent
      if (arg._VECTptr->back()==plus_one_half){
	Equation_translate(varg,a.fontsize,0);
	vecteur res(1,varg);
	res.push_back(eqwdata(vv.dx+a.fontsize,vv.dy+4,vv.x,vv.y,a,at_sqrt,0));
	return gen(res,_SEQ__VECT);
      }
      bool needpar=vv.g.type==_FUNC || vv.g.is_symb_of_sommet(at_pow) || need_parenthesis(vv.g);
      if (needpar)
	x=llp;
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      vecteur res(1,varg);
      // 2nd arg translated 
      if (needpar)
	x+=vv.dx+lrp;
      else
	x+=vv.dx+1;
      int arg1dy=vv.dy,arg1y=vv.y;
      if (a.fontsize>=16){
	attributs aa(a);
	aa.fontsize -= 2;
	varg=Equation_compute_size(arg._VECTptr->back(),aa,windowhsize,contextptr);
      }
      else
	varg=Equation_compute_size(arg._VECTptr->back(),a,windowhsize,contextptr);
      vv=Equation_total_size(varg);
      Equation_translate(varg,x,arg1y+(3*arg1dy)/4-vv.y);
      res.push_back(varg);
      vv=Equation_total_size(varg);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      x += vv.dx;
      res.push_back(eqwdata(x,h,0,y,a,u,0));
      return gen(res,_SEQ__VECT);
    }
    if (u==at_factorial){
      vecteur v;
      gen varg=Equation_compute_size(arg,a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      bool paren=need_parenthesis(vv.g) || vv.g==at_prod || vv.g==at_division || vv.g==at_pow;
      if (paren)
	x+=llp;
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      v.push_back(varg);
      x += vv.dx;
      if (paren)
	x+=lrp;
      varg=eqwdata(x+4,h,0,y,a,u,0);
      v.push_back(varg);
      return gen(v,_SEQ__VECT);
    }
    if (u==at_sto){ // A:=B, *it -> B
      gen varg=Equation_compute_size(arg._VECTptr->back(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      Equation_translate(varg,x,0);
      vecteur v(2);
      v[1]=varg;
      x+=vv.dx;
      x+=ls+3;
      // first arg not translated
      varg=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      vv=Equation_total_size(varg);
      if (need_parenthesis(vv.g))
	x+=llp;
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      v[0]=varg;
      x += vv.dx;
      if (need_parenthesis(vv.g))
	x+=lrp;
      v.push_back(eqwdata(x,h,0,y,a,u,0));
      return gen(v,_SEQ__VECT);
    }
    if (u==at_program && arg._VECTptr->back().type!=_VECT && !arg._VECTptr->back().is_symb_of_sommet(at_local) ){
      gen varg=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      Equation_translate(varg,x,0);
      vecteur v(2);
      v[0]=varg;
      x+=vv.dx;
      x+=int(text_width(a.fontsize,("->")))+3;
      varg=Equation_compute_size(arg._VECTptr->back(),a,windowhsize,contextptr);
      vv=Equation_total_size(varg);
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      v[1]=varg;
      x += vv.dx;
      v.push_back(eqwdata(x,h,0,y,a,u,0));
      return gen(v,_SEQ__VECT);      
    }
    bool binaryop= (u.ptr()->printsommet==&printsommetasoperator) || binary_op(u);
    if ( u!=at_sto && u.ptr()->printsommet!=NULL && !binaryop ){
      gen tmp=string2gen(g.print(contextptr),false);
      return Equation_compute_size(symbolic(at_expr,makesequence(tmp,xcas_mode(contextptr))),a,windowhsize,contextptr);
    }
    vecteur v;
    if (!binaryop || arg.type!=_VECT)
      v=Equation_subsizes(arg,a,windowhsize,contextptr);
    else
      v=Equation_subsizes(gen(*arg._VECTptr,_SEQ__VECT),a,windowhsize,contextptr);
    iterateur it=v.begin(),itend=v.end();
    if ( it==itend || (itend-it==1) ){ 
      gen gtmp;
      if (it==itend)
	gtmp=Equation_compute_size(gen(vecteur(0),_SEQ__VECT),a,windowhsize,contextptr);
      else
	gtmp=*it;
      // unary op, shift arg position horizontally
      eqwdata vv=Equation_total_size(gtmp);
      bool paren = u!=at_neg || (vv.g!=at_prod && need_parenthesis(vv.g)) ;
      x=ls+(paren?llp:0);
      gen tmp=gtmp; Equation_translate(tmp,x,0);
      x=x+vv.dx+(paren?lrp:0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      return gen(makevecteur(tmp,eqwdata(x,h,0,y,a,u,0)),_EQW__VECT);
    }
    if (binaryop){ // op (default with par)
      int currenth=h,largeur=0;
      iterateur itprec=v.begin();
      h=0;
      if (u==at_plus){ // op without parenthesis
	if (it->type==_VECT && !it->_VECTptr->empty() && it->_VECTptr->back().type==_EQW && it->_VECTptr->back()._EQWptr->g==at_equal)
	  ;
	else {
	  llp=0;
	  lrp=0;
	}
      }
      for (;;){
	eqwdata vv=Equation_total_size(*it);
	if (need_parenthesis(vv.g))
	  x+=llp;
	if (u==at_plus && it!=v.begin() &&
	    ( 
	     (it->type==_VECT && it->_VECTptr->back().type==_EQW && it->_VECTptr->back()._EQWptr->g==at_neg) 
	     || 
	     ( it->type==_EQW && (is_integer(it->_EQWptr->g) || it->_EQWptr->g.type==_DOUBLE_) && is_strictly_positive(-it->_EQWptr->g,contextptr) ) 
	      ) 
	    )
	  x -= ls;
#if 0 //
	if (x>windowhsize-vv.dx && x>windowhsize/2 && (itend-it)*vv.dx>windowhsize/2){
	  largeur=max(x,largeur);
	  x=0;
	  if (need_parenthesis(vv.g))
	    x+=llp;
	  h+=currenth;
	  Equation_translate(*it,x,0);
	  for (iterateur kt=v.begin();kt!=itprec;++kt)
	    Equation_translate(*kt,0,currenth);
	  if (y){
	    for (iterateur kt=itprec;kt!=it;++kt)
	      Equation_translate(*kt,0,-y);
	  }
	  itprec=it;
	  currenth=vv.dy;
	  y=vv.y;
	}
	else
#endif
	  {
	    Equation_translate(*it,x,0);
	    vv=Equation_total_size(*it);
	    Equation_vertical_adjust(vv.dy,vv.y,currenth,y);
	  }
	x+=vv.dx;
	if (need_parenthesis(vv.g))
	  x+=lrp;
	++it;
	if (it==itend){
	  for (iterateur kt=v.begin();kt!=itprec;++kt)
	    Equation_translate(*kt,0,currenth+y);
	  h+=currenth;
	  v.push_back(eqwdata(max(x,largeur),h,0,y,a,u,0));
	  //cout << v << endl;
	  return gen(v,_SEQ__VECT);
	}
	x += ls+3;
      } 
    }
    // normal printing
    x=ls+llp;
    for (;;){
      eqwdata vv=Equation_total_size(*it);
      Equation_translate(*it,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      x+=vv.dx;
      ++it;
      if (it==itend){
	x+=lrp;
	v.push_back(eqwdata(x,h,0,y,a,u,0));
	return gen(v,_SEQ__VECT);
      }
      x+=lc;
    }
  }

  // windowhsize is used for g of type HIST__VECT (history) right justify answers
  // Returns either a eqwdata type object (terminal) or a vector 
  // (of subtype _EQW__VECT or _HIST__VECT)
  gen Equation_compute_size(const gen & g,const attributs & a,int windowhsize,GIAC_CONTEXT){
    /*****************
     *   FRACTIONS   *
     *****************/
    if (g.type==_FRAC){
      if (is_integer(g._FRACptr->num) && is_positive(-g._FRACptr->num,contextptr))
	return Equation_compute_size(symbolic(at_neg,fraction(-g._FRACptr->num,g._FRACptr->den)),a,windowhsize,contextptr);
      gen v1=Equation_compute_size(g._FRACptr->num,a,windowhsize,contextptr);
      eqwdata vv1=Equation_total_size(v1);
      gen v2=Equation_compute_size(g._FRACptr->den,a,windowhsize,contextptr);
      eqwdata vv2=Equation_total_size(v2);
      // Center the fraction
      int w1=vv1.dx,w2=vv2.dx;
      int w=max(w1,w2)+6;
      vecteur v(3);
      v[0]=v1; Equation_translate(v[0],(w-w1)/2,11-vv1.y);
      v[1]=v2; Equation_translate(v[1],(w-w2)/2,5-vv2.dy-vv2.y);
      v[2]=eqwdata(w,a.fontsize/2+vv1.dy+vv2.dy+1,0,(a.fontsize<=14?4:3)-vv2.dy,a,at_division,0);
      return gen(v,_SEQ__VECT);
    }
    /***************
     *   VECTORS   *
     ***************/
    if ( (g.type==_VECT) && !g._VECTptr->empty() ){
      vecteur v;
      const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      int x=0,y=0,h=a.fontsize; 
      /***************
       *   MATRICE   *
       ***************/
      bool gmat=ckmatrix(g);
      vector<int> V; int p=0;
      if (!gmat && is_mod_vecteur(*g._VECTptr,V,p) && p!=0){
	gen gm=makemodquoted(unmod(g),p);
	return Equation_compute_size(gm,a,windowhsize,contextptr);
      }
      vector< vector<int> > M; 
      if (gmat && is_mod_matrice(*g._VECTptr,M,p) && p!=0){
	gen gm=makemodquoted(unmod(g),p);
	return Equation_compute_size(gm,a,windowhsize,contextptr);
      }
      if (gmat && g.subtype!=_SEQ__VECT && g.subtype!=_SET__VECT && g.subtype!=_POLY1__VECT && g._VECTptr->front().subtype!=_SEQ__VECT){
	gen mkvect(at_makevector);
	mkvect.subtype=_SEQ__VECT;
	gen mkmat(at_makevector);
	mkmat.subtype=_MATRIX__VECT;
	int nrows,ncols;
	mdims(*g._VECTptr,nrows,ncols);
	if (ncols){
	  vecteur all_sizes;
	  all_sizes.reserve(nrows);
	  vector<int> row_heights(nrows),row_bases(nrows),col_widths(ncols);
	  // vertical gluing
	  for (int i=0;it!=itend;++it,++i){
	    gen tmpg=*it;
	    tmpg.subtype=_SEQ__VECT;
	    vecteur tmp(Equation_subsizes(tmpg,a,max(windowhsize/ncols-a.fontsize,230),contextptr));
	    int h=a.fontsize,y=0;
	    const_iterateur jt=tmp.begin(),jtend=tmp.end();
	    for (int j=0;jt!=jtend;++jt,++j){
	      eqwdata w(Equation_total_size(*jt));
	      Equation_vertical_adjust(w.dy,w.y,h,y);
	      col_widths[j]=max(col_widths[j],(int)w.dx);
	    }
	    if (i)
	      row_heights[i]=row_heights[i-1]+h+a.fontsize/2;
	    else
	      row_heights[i]=h;
	    row_bases[i]=y;
	    all_sizes.push_back(tmp);
	  }
	  // accumulate col widths
	  col_widths.front() +=(3*a.fontsize)/2;
	  vector<int>::iterator iit=col_widths.begin()+1,iitend=col_widths.end();
	  for (;iit!=iitend;++iit)
	    *iit += *(iit-1)+a.fontsize;
	  // translate each cell
	  it=all_sizes.begin();
	  itend=all_sizes.end();
	  int h,y,prev_h=0;
	  for (int i=0;it!=itend;++it,++i){
	    h=row_heights[i];
	    y=row_bases[i];
	    iterateur jt=it->_VECTptr->begin(),jtend=it->_VECTptr->end();
	    for (int j=0;jt!=jtend;++jt,++j){
	      eqwdata w(Equation_total_size(*jt));
	      if (j)
		Equation_translate(*jt,col_widths[j-1]-w.x,-h-y);
	      else
		Equation_translate(*jt,-w.x+a.fontsize/2,-h-y);
	    }
	    it->_VECTptr->push_back(eqwdata(col_widths.back(),h-prev_h,0,-h,a,mkvect,0));
	    prev_h=h;
	  }
	  all_sizes.push_back(eqwdata(col_widths.back(),row_heights.back(),0,-row_heights.back(),a,mkmat,-row_heights.back()/2));
	  gen all_sizesg=all_sizes; Equation_translate(all_sizesg,0,row_heights.back()/2); return all_sizesg;
	}
      } // end matrices
      /*************************
       *   SEQUENCES/VECTORS   *
       *************************/
      // horizontal gluing
      if (g.subtype!=_PRINT__VECT) x += a.fontsize/2;
      int ncols=itend-it;
      //ncols=min(ncols,5);
      for (;it!=itend;++it){
	gen cur_size=Equation_compute_size(*it,a,
					   max(windowhsize/ncols-a.fontsize,
#ifdef IPAQ
					       200
#else
					       480
#endif
					       ),contextptr);
	eqwdata tmp=Equation_total_size(cur_size);
	Equation_translate(cur_size,x-tmp.x,0); v.push_back(cur_size);
	x=x+tmp.dx+((g.subtype==_PRINT__VECT)?2:a.fontsize);
	Equation_vertical_adjust(tmp.dy,tmp.y,h,y);
      }
      gen mkvect(at_makevector);
      if (g.subtype==_SEQ__VECT)
	mkvect=at_makesuite;
      else
	mkvect.subtype=g.subtype;
      v.push_back(eqwdata(x,h,0,y,a,mkvect,0));
      return gen(v,_EQW__VECT);
    } // end sequences
    if (g.type==_MOD){ 
      int x=0;
      int h=a.fontsize;
      int y=0;
      bool py=python_compat(contextptr);
      int modsize=int(text_width(a.fontsize,(py?" mod":"%")))+4;
      bool paren=is_positive(-*g._MODptr,contextptr);
      int llp=int(text_width(a.fontsize,("(")));
      int lrp=int(text_width(a.fontsize,(")")));
      gen varg1=Equation_compute_size(*g._MODptr,a,windowhsize,contextptr);
      if (paren) Equation_translate(varg1,llp,0);
      eqwdata vv=Equation_total_size(varg1);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      gen arg2=*(g._MODptr+1);
      gen varg2=Equation_compute_size(arg2,a,windowhsize,contextptr);
      if (paren)
	Equation_translate(varg2,vv.dx+modsize+lrp,0);
      else
	Equation_translate(varg2,vv.dx+modsize,0);
      vv=Equation_total_size(varg2);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      vecteur res(makevecteur(varg1,varg2));
      res.push_back(eqwdata(vv.dx+vv.x,h,0,y,a,at_normalmod,0));
      return gen(res,_SEQ__VECT);
    }
    if (g.type!=_SYMB){
      string s=g.type==_STRNG?*g._STRNGptr:g.print(contextptr);
      //if (g==cst_pi) s=char(129);
      if (s.size()>2000)
	s=s.substr(0,2000)+"...";
      int i=int(text_width(a.fontsize,(s.c_str())));
      gen tmp=eqwdata(i,a.fontsize,0,0,a,g);
      return tmp;
    }
    /**********************
     *  SYMBOLIC HANDLING *
     **********************/
    return Equation_compute_symb_size(g,a,windowhsize,contextptr);
    // return Equation_compute_symb_size(aplatir_fois_plus(g),a,windowhsize,contextptr);
    // aplatir_fois_plus is a problem for Equation_replace_selection
    // because it will modify the structure of the data
  }

  void Equation_draw(const eqwdata & e,int x,int y,int rightx,int lowery,Equation * eq,GIAC_CONTEXT){
    if ( (e.dx+e.x<x) || (e.x>rightx) || (e.y>y) || e.y+e.dy<lowery)
      ; // return; // nothing to draw, out of window
    gen gg=e.g;
    int fontsize=e.eqw_attributs.fontsize;
    int text_color=COLOR_BLACK;
    int background=COLOR_WHITE;
    string s=gg.type==_STRNG?*gg._STRNGptr:gg.print(contextptr);
    if (gg.type==_IDNT && !s.empty() && s[0]=='_')
      s=s.substr(1,s.size()-1);
    // if (gg==cst_pi){      s="p";      s[0]=(unsigned char)129;    }
    if (s.size()>2000)
      s=s.substr(0,2000)+"...";
    // cerr << s.size() << endl;
    text_print(fontsize,s.c_str(),eq->x()+e.x-x,eq->y()+y-e.y,text_color,background,e.selected?4:0);
    return;
  }

  inline void check_fl_rectf(int x,int y,int w,int h,int imin,int jmin,int di,int dj,int delta_i,int delta_j,int c){
    drawRectangle(x+delta_i,y+delta_j,w,h,c);
    //fl_rectf(x+delta_i,y+delta_j,w,h,c);
  }

  void Equation_draw(const gen & g,int x,int y,int rightx,int lowery,Equation * equat,GIAC_CONTEXT){
    int eqx=equat->x(),eqy=equat->y();
    if (g.type==_EQW){ // terminal
      eqwdata & e=*g._EQWptr;
      Equation_draw(e,x,y,rightx,lowery,equat,contextptr);
    }
    if (g.type!=_VECT)
      return;
    vecteur & v=*g._VECTptr;
    if (v.empty())
      return;
    gen tmp=v.back();
    if (tmp.type!=_EQW){
      cout << "EQW error:" << v << endl;
      return;
    }
    eqwdata & w=*tmp._EQWptr;
    if ( (w.dx+w.x-x<0) || (w.x>rightx) || (w.y>y) || (w.y+w.dy<lowery) )
      ; // return; // nothing to draw, out of window
    /*******************
     * draw the vector *
     *******************/
    // v is the vector, w the master operator eqwdata
    gen oper=w.g; 
    bool selected=w.selected ;
    int fontsize=w.eqw_attributs.fontsize;
    int background=w.eqw_attributs.background;
    int text_color=w.eqw_attributs.text_color;
    int mode=selected?4:0;
    int draw_line_color=selected?background:text_color;
    int x0=w.x;
    int y0=w.y; // lower coordinate of the master vector
    int y1=y0+w.dy; // upper coordinate of the master vector
    if (selected)
      drawRectangle(eqx+w.x-x,eqy+y-w.y-w.dy+1,w.dx,w.dy+1,text_color);
    // draw arguments of v
    const_iterateur it=v.begin(),itend=v.end()-1;
    if (oper==at_expr && v.size()==3){
      Equation_draw(*it,x,y,rightx,lowery,equat,contextptr);
      return;
    }
    for (;it!=itend;++it)
      Equation_draw(*it,x,y,rightx,lowery,equat,contextptr);
    if (oper==at_multistring)
      return;
    string s;
    if (oper.type==_FUNC){
      // catch here special cases user function, vect/matr, ^, int, sqrt, etc.
      unary_function_ptr & u=*oper._FUNCptr;
      if (u==at_at){ // draw brackets around 2nd arg
	gen arg2=v[1]; // 2nd arg of at_of, i.e. what's inside the parenth.
	eqwdata varg2=Equation_total_size(arg2);
	x0=varg2.x;
	y0=varg2.y;
	y1=y0+varg2.dy;
	fontsize=varg2.eqw_attributs.fontsize;
	if (x0<rightx)
	  text_print(fontsize,"[",eqx+x0-x-int(text_width(fontsize,("["))),eqy+y-varg2.baseline,text_color,background,mode);
	x0 += varg2.dx ;
	if (x0<rightx)
	  text_print(fontsize,"]",eqx+x0-x,eqy+y-varg2.baseline,text_color,background,mode);
	return;
      }
      if (u==at_of){ // do we need to draw some parenthesis?
	gen arg2=v[1]; // 2nd arg of at_of, i.e. what's inside the parenth.
	if (arg2.type!=_VECT || arg2._VECTptr->back().type !=_EQW || arg2._VECTptr->back()._EQWptr->g!=at_makesuite){ // Yes (if not _EQW it's a sequence with parent)
	  eqwdata varg2=Equation_total_size(arg2);
	  x0=varg2.x;
	  y0=varg2.y;
	  y1=y0+varg2.dy;
	  fontsize=varg2.eqw_attributs.fontsize;
	  int pfontsize=max(fontsize,(fontsize+(varg2.baseline-varg2.y))/2);
	  if (x0<rightx)
	    text_print(pfontsize,"(",eqx+x0-x-int(text_width(fontsize,("("))),eqy+y-varg2.baseline,text_color,background,mode);
	  x0 += varg2.dx ;
	  if (x0<rightx)
	    text_print(pfontsize,")",eqx+x0-x,eqy+y-varg2.baseline,text_color,background,mode);
	}
	return;
      }
      if (u==at_makesuite){
	bool paren=v.size()!=2; // Sequences with 1 arg don't show parenthesis
	int pfontsize=max(fontsize,(fontsize+(w.baseline-w.y))/2);
	if (paren && x0<rightx)
	  text_print(pfontsize,"(",eqx+x0-x-int(text_width(fontsize,("(")))/2,eqy+y-w.baseline,text_color,background,mode);
	x0 += w.dx;
	if (paren && x0<rightx)
	  text_print(pfontsize,")",eqx+x0-x-int(text_width(fontsize,("(")))/2,eqy+y-w.baseline,text_color,background,mode);
	// print commas between args
	it=v.begin(),itend=v.end()-2;
	for (;it!=itend;++it){
	  eqwdata varg2=Equation_total_size(*it);
	  fontsize=varg2.eqw_attributs.fontsize;
	  if (varg2.x+varg2.dx<rightx)
	    text_print(fontsize,",",eqx+varg2.x+varg2.dx-x+1,eqy+y-varg2.baseline,text_color,background,mode);
	}
	return;
      }
      if (u==at_makevector){ // draw [] delimiters for vector/matrices
	if (oper.subtype!=_SEQ__VECT && oper.subtype!=_PRINT__VECT){
	  int decal=1;
	  switch (oper.subtype){
	  case _MATRIX__VECT: decal=2; break;
	  case _SET__VECT: decal=4; break;
	  case _POLY1__VECT: decal=6; break;
	  }
	  if (eqx+x0-x+1>=0){
	    draw_line(eqx+x0-x+1,eqy+y-y0+1,eqx+x0-x+1,eqy+y-y1+1,draw_line_color);
	    draw_line(eqx+x0-x+decal,eqy+y-y0+1,eqx+x0-x+decal,eqy+y-y1+1,draw_line_color);
	    draw_line(eqx+x0-x+1,eqy+y-y0+1,eqx+x0-x+fontsize/4,eqy+y-y0+1,draw_line_color);
	    draw_line(eqx+x0-x+1,eqy+y-y1+1,eqx+x0-x+fontsize/4,eqy+y-y1+1,draw_line_color);
	  }
	  x0 += w.dx ;
	  if (eqx+x0-x-1<LCD_WIDTH_PX){
	    draw_line(eqx+x0-x-1,eqy+y-y0+1,eqx+x0-x-1,eqy+y-y1+1,draw_line_color);
	    draw_line(eqx+x0-x-decal,eqy+y-y0+1,eqx+x0-x-decal,eqy+y-y1+1,draw_line_color);
	    draw_line(eqx+x0-x-1,eqy+y-y0+1,eqx+x0-x-fontsize/4,eqy+y-y0+1,draw_line_color);
	    draw_line(eqx+x0-x-1,eqy+y-y1+1,eqx+x0-x-fontsize/4,eqy+y-y1+1,draw_line_color);
	  }
	} // end if oper.subtype!=SEQ__VECT
	if (oper.subtype!=_MATRIX__VECT && oper.subtype!=_PRINT__VECT){
	  // print commas between args
	  it=v.begin(),itend=v.end()-2;
	  for (;it!=itend;++it){
	    eqwdata varg2=Equation_total_size(*it);
	    fontsize=varg2.eqw_attributs.fontsize;
	    if (varg2.x+varg2.dx<rightx)
	      text_print(fontsize,",",eqx+varg2.x+varg2.dx-x+1,eqy+y-varg2.baseline,text_color,background,mode);
	  }
	}
	return;
      }
      int lpsize=int(text_width(fontsize,("(")));
      int rpsize=int(text_width(fontsize,(")")));
      eqwdata tmp=Equation_total_size(v.front()); // tmp= 1st arg eqwdata
      if (u==at_sto)
	tmp=Equation_total_size(v[1]);
      x0=w.x-x;
      y0=y-w.baseline;
      if (u==at_pow){
	if (!need_parenthesis(tmp.g)&& tmp.g!=at_pow && tmp.g!=at_prod && tmp.g!=at_division)
	  return;
	if (tmp.g==at_pow){
	  fontsize=tmp.eqw_attributs.fontsize+2;
	}
	if (tmp.x-lpsize<rightx)
	  text_print(fontsize,"(",eqx+tmp.x-x-lpsize,eqy+y-tmp.baseline,text_color,background,mode);
	if (tmp.x+tmp.dx<rightx)
	  text_print(fontsize,")",eqx+tmp.x+tmp.dx-x,eqy+y-tmp.baseline,text_color,background,mode);
	return;
      }
      if (u==at_program){
	if (tmp.x+tmp.dx<rightx)
	  text_print(fontsize,"->",eqx+tmp.x+tmp.dx-x,eqy+y-tmp.baseline,text_color,background,mode);
	return;
      }
#if 1
      if (u==at_sum){
	if (x0<rightx){
	  draw_line(eqx+x0,eqy+y0,eqx+x0+(2*fontsize)/3,eqy+y0,draw_line_color);
	  draw_line(eqx+x0,eqy+y0-fontsize,eqx+x0+(2*fontsize)/3,eqy+y0-fontsize,draw_line_color);
	  draw_line(eqx+x0,eqy+y0,eqx+x0+fontsize/2,eqy+y0-fontsize/2,draw_line_color);
	  draw_line(eqx+x0+fontsize/2,eqy+y0-fontsize/2,eqx+x0,eqy+y0-fontsize,draw_line_color);
	  if (v.size()>2){ // draw the =
	    eqwdata ptmp=Equation_total_size(v[1]);
	    if (ptmp.x+ptmp.dx<rightx)
	      text_print(fontsize,"=",eqx+ptmp.x+ptmp.dx-x-2,eqy+y-ptmp.baseline,text_color,background,mode);
	  }
	}
	return;
      }
#endif
      if (u==at_abs){
	y0 =1+y-w.y;
	int h=w.dy;
	if (x0<rightx){
	  draw_line(eqx+x0+2,eqy+y0-1,eqx+x0+2,eqy+y0-h+3,draw_line_color);
	  draw_line(eqx+x0+1,eqy+y0-1,eqx+x0+1,eqy+y0-h+3,draw_line_color);
	  draw_line(eqx+x0+w.dx-1,eqy+y0-1,eqx+x0+w.dx-1,eqy+y0-h+3,draw_line_color);
	  draw_line(eqx+x0+w.dx,eqy+y0-1,eqx+x0+w.dx,eqy+y0-h+3,draw_line_color);
	}
	return;
      }
      if (u==at_sqrt){
	y0 =1+y-w.y;
	int h=w.dy;
	if (x0<rightx){
	  draw_line(eqx+x0+2,eqy+y0-h/2,eqx+x0+fontsize/2,eqy+y0-1,draw_line_color);
	  draw_line(eqx+x0+fontsize/2,eqy+y0-1,eqx+x0+fontsize,eqy+y0-h+3,draw_line_color);
	  draw_line(eqx+x0+fontsize,eqy+y0-h+3,eqx+x0+w.dx-1,eqy+y0-h+3,draw_line_color);
	  ++y0;
	  draw_line(eqx+x0+2,eqy+y0-h/2,eqx+x0+fontsize/2,eqy+y0-1,draw_line_color);
	  draw_line(eqx+x0+fontsize/2,eqy+y0-1,eqx+x0+fontsize,eqy+y0-h+3,draw_line_color);
	  draw_line(eqx+x0+fontsize,eqy+y0-h+3,eqx+x0+w.dx-1,eqy+y0-h+3,draw_line_color);
	}
	return;
      }
      if (u==at_factorial){
	text_print(fontsize,"!",eqx+w.x+w.dx-4-x,eqy+y-w.baseline,text_color,background,mode);
	if (!need_parenthesis(tmp.g)
	    && tmp.g!=at_pow && tmp.g!=at_prod && tmp.g!=at_division
	    )
	  return;
	if (tmp.x-lpsize<rightx)
	  text_print(fontsize,"(",eqx+tmp.x-x-lpsize,eqy+y-tmp.baseline,text_color,background,mode);
	if (tmp.x+tmp.dx<rightx)
	  text_print(fontsize,")",eqx+tmp.x+tmp.dx-x,eqy+y-tmp.baseline,text_color,background,mode);
	return;
      }
#if 1
      if (u==at_integrate){
	x0+=2;
	y0+=fontsize/2;
	if (x0<rightx){
	  fl_arc(eqx+x0,eqy+y0,fontsize/3,fontsize/3,180,360,draw_line_color);
	  draw_line(eqx+x0+fontsize/3,eqy+y0,eqx+x0+fontsize/3,eqy+y0-2*fontsize+4,draw_line_color);
	  fl_arc(eqx+x0+fontsize/3,eqy+y0-2*fontsize+3,fontsize/3,fontsize/3,0,180,draw_line_color);
	}
	if (v.size()!=2){ // if arg has size > 1 draw the d
	  eqwdata ptmp=Equation_total_size(v[1]);
	  if (ptmp.x<rightx)
	    text_print(fontsize," d",eqx+ptmp.x-x-int(text_width(fontsize,(" d"))),eqy+y-ptmp.baseline,text_color,background,mode);
	}
	else {
	  eqwdata ptmp=Equation_total_size(v[0]);
	  if (ptmp.x+ptmp.dx<rightx)
	    text_print(fontsize," dx",eqx+ptmp.x+ptmp.dx-x,eqy+y-ptmp.baseline,text_color,background,mode);
	}
	return;
      }
#endif
      if (u==at_division){
	if (x0<rightx){
	  int yy=eqy+y0-8;
	  draw_line(eqx+x0+2,yy,eqx+x0+w.dx-2,yy,draw_line_color);
	  ++yy;
	  draw_line(eqx+x0+2,yy,eqx+x0+w.dx-2,yy,draw_line_color);
	}
	return;
      }
#if 1
      if (u==at_limit && v.size()>=4){
	if (x0<rightx)
	  text_print(fontsize,"lim",eqx+w.x-x,eqy+y-w.baseline,text_color,background,mode);
	gen arg2=v[1]; // 2nd arg of limit, i.e. the variable
	if (arg2.type==_EQW){ 
	  eqwdata & varg2=*arg2._EQWptr;
	  if (varg2.x+varg2.dx+2<rightx)
	    text_print(fontsize,"\x1e",eqx+varg2.x+varg2.dx+2-x,eqy+y-varg2.y,text_color,background,mode);
	}
	if (v.size()>=5){
	  arg2=v[2]; // 3rd arg of lim, the point, draw a comma after if dir.
	  if (arg2.type==_EQW){ 
	    eqwdata & varg2=*arg2._EQWptr;
	    if (varg2.x+varg2.dx<rightx)
	      text_print(fontsize,",",eqx+varg2.x+varg2.dx-x,eqy+y-varg2.baseline,text_color,background,mode);
	  }
	}
	return;
      } // limit
#endif
      bool parenthesis=true;
      string opstring(",");
      if (u.ptr()->printsommet==&printsommetasoperator || binary_op(u) ){
	if (u==at_normalmod && python_compat(contextptr))
	  opstring=" mod";
	else
	  opstring=u.ptr()->s;
      }
      else {
	if (u==at_sto)
	  opstring=":=";
	parenthesis=false;
      }
      // int yy=y0; // y0 is the lower coordinate of the whole eqwdata
      // int opsize=int(text_width(fontsize,(opstring.c_str())))+3;
      it=v.begin();
      itend=v.end()-1;
      // Reminder: here tmp is the 1st arg eqwdata, w the whole eqwdata
      if ( (itend-it==1) && ( (u==at_neg) 
			      || (u==at_plus) // uncommented for +infinity
			      ) ){ 
	if ( (u==at_neg &&need_parenthesis(tmp.g) && tmp.g!=at_prod)){
	  if (tmp.x-lpsize<rightx)
	    text_print(fontsize,"(",eqx+tmp.x-x-lpsize,eqy+y-tmp.baseline,text_color,background,mode);
	  if (tmp.x+tmp.dx<rightx)
	    text_print(fontsize,")",eqx+tmp.x-x+tmp.dx,eqy+y-tmp.baseline,text_color,background,mode);
	}
	if (w.x<rightx){
	  text_print(fontsize,u.ptr()->s,eqx+w.x-x,eqy+y-w.baseline,text_color,background,mode);
	}
	return;
      }
      // write first open parenthesis
      if (u==at_plus && tmp.g!=at_equal)
	parenthesis=false;
      else {
	if (parenthesis && need_parenthesis(tmp.g)){
	  if (w.x<rightx){
	    int pfontsize=max(fontsize,(fontsize+(tmp.baseline-tmp.y))/2);
	    text_print(pfontsize,"(",eqx+w.x-x,eqy+y-tmp.baseline,text_color,background,mode);
	  }
	}
      }
      for (;;){
	// write close parenthesis at end
	int xx=tmp.dx+tmp.x-x;
	if (parenthesis && need_parenthesis(tmp.g)){
	  if (xx<rightx){
	    int pfontsize=min(max(fontsize,(fontsize+(tmp.baseline-tmp.y))/2),fontsize*2);
	    int deltapary=(2*(pfontsize-fontsize))/3;
	    text_print(pfontsize,")",eqx+xx,eqy+y-tmp.baseline+deltapary,text_color,background,mode);
	  }
	  xx +=rpsize;
	}
	++it;
	if (it==itend){
	  if (u.ptr()->printsommet==&printsommetasoperator || u==at_sto || binary_op(u))
	    return;
	  else
	    break;
	}
	// write operator
	if (u==at_prod){
	  // text_print(fontsize,".",eqx+xx+3,eqy+y-tmp.baseline-fontsize/3);
	  text_print(fontsize,opstring.c_str(),eqx+xx+1,eqy+y-tmp.baseline,text_color,background,mode);
	}
	else {
	  gen tmpgen;
	  if (u==at_plus && ( 
			     (it->type==_VECT && it->_VECTptr->back().type==_EQW && it->_VECTptr->back()._EQWptr->g==at_neg) 
			     || 
			     ( it->type==_EQW && (is_integer(it->_EQWptr->g) || it->_EQWptr->g.type==_DOUBLE_) && is_strictly_positive(-it->_EQWptr->g,contextptr) ) 
			      )
	      )
	    ;
	  else {
	    if (xx+1<rightx)
	      // fl_draw(opstring.c_str(),xx+1,y-tmp.y-tmp.dy/2+fontsize/2);
	      text_print(fontsize,opstring.c_str(),eqx+xx+1,eqy+y-tmp.baseline,text_color,background,mode);
	  }
	}
	// write right parent, update tmp
	tmp=Equation_total_size(*it);
	if (parenthesis && (need_parenthesis(tmp.g)) ){
	  if (tmp.x-lpsize<rightx){
	    int pfontsize=min(max(fontsize,(fontsize+(tmp.baseline-tmp.y))/2),fontsize*2);
	    int deltapary=(2*(pfontsize-fontsize))/3;
	    text_print(pfontsize,"(",eqx+tmp.x-pfontsize*lpsize/fontsize-x,eqy+y-tmp.baseline+deltapary,text_color,background,mode);
	  }
	}
      } // end for (;;)
      if (w.x<rightx){
	s = u.ptr()->s;
	s += '(';
	text_print(fontsize,s.c_str(),eqx+w.x-x,eqy+y-w.baseline,text_color,background,mode);
      }
      if (w.x+w.dx-rpsize<rightx)
	text_print(fontsize,")",eqx+w.x+w.dx-x-rpsize+2,eqy+y-w.baseline,text_color,background,mode);
      return;
    }
    s=oper.print(contextptr);
    if (w.x<rightx){
      text_print(fontsize,s.c_str(),eqx+w.x-x,eqy+y-w.baseline,text_color,background,mode);
    }
  }

  Equation::Equation(int x_, int y_, const gen & g,const giac::context * cptr){
    _x=x_;
    _y=y_;
    attr=attributs(18,COLOR_WHITE,COLOR_BLACK);
    contextptr=cptr;
    if (taille(g,max_prettyprint_equation)<max_prettyprint_equation)
      data=Equation_compute_size(g,attr,LCD_WIDTH_PX,contextptr);
    else
      data=Equation_compute_size(string2gen("Object_too_large",false),attr,LCD_WIDTH_PX,contextptr);
    undodata=Equation_copy(data);
  }

  void replace_selection(Equation & eq,const gen & tmp,gen * gsel,const vector<int> * gotoptr,GIAC_CONTEXT){
    int xleft,ytop,xright,ybottom,gselpos; gen *gselparent;
    vector<int> goto_sel;
    eq.undodata=Equation_copy(eq.data);
    if (gotoptr==0){
      if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel)
	gotoptr=&goto_sel;
      else
	return;
    }
    *gsel=xcas::Equation_compute_size(tmp,eq.attr,LCD_WIDTH_PX,contextptr);
    gen value;
    xcas::do_select(eq.data,true,value);
    if (value.type==_EQW)
      eq.data=xcas::Equation_compute_size(value._EQWptr->g,eq.attr,LCD_WIDTH_PX,contextptr);
    //cout << "new value " << value << " " << eq.data << " " << *gotoptr << endl;
    xcas::Equation_select(eq.data,false);
    gen * gptr=&eq.data;
    for (int i=gotoptr->size()-1;i>=0;--i){
      int pos=(*gotoptr)[i];
      if (gptr->type==_VECT &&gptr->_VECTptr->size()>pos)
	gptr=&(*gptr->_VECTptr)[pos];
    }
    xcas::Equation_select(*gptr,true);
    //cout << "new sel " << *gptr << endl;
  }

  void display(Equation & eq,int x,int y,GIAC_CONTEXT){
    // Equation_draw(eq.data,x,y,LCD_WIDTH_PX,0,&eq,contextptr);
    int xleft,ytop,xright,ybottom,gselpos; gen * gsel,*gselparent;
    eqwdata eqdata=Equation_total_size(eq.data);
    if ( (eqdata.dx>LCD_WIDTH_PX || eqdata.dy>LCD_HEIGHT_PX-STATUS_AREA_PX) && Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos)){
      if (x<xleft){
	if (x+LCD_WIDTH_PX<xright)
	  x=giacmin(xleft,xright-LCD_WIDTH_PX);
      }
      if (x>=xleft && x+LCD_WIDTH_PX>=xright){
	if (xright-x<LCD_WIDTH_PX)
	  x=giacmax(xright-LCD_WIDTH_PX,0);
      }
#if 0
      cout << "avant " << y << " " << ytop << " " << ybottom << endl;
      if (y<ytop){
	if (y+LCD_HEIGHT_PX<ybottom)
	  y=giacmin(ytop,ybottom-LCD_HEIGHT_PX);
      }
      if (y>=ytop && y+LCD_HEIGHT_PX>=ybottom){
	if (ybottom-y<LCD_HEIGHT_PX)
	  y=giacmax(ybottom-LCD_HEIGHT_PX,0);
      }
      cout << "apres " << y << " " << ytop << " " << ybottom << endl;
#endif
    }
    int save_ymin_clip=clip_ymin;
    clip_ymin=STATUS_AREA_PX;
    Equation_draw(eq.data,x,y,RAND_MAX,0,&eq,contextptr);
    clip_ymin=save_ymin_clip;
  }
  
  /* ******************* *
   *      GRAPH          *
   * ******************* *
   */
#if 1

  double find_tick(double dx){
    double d=std::pow(10.0,std::floor(std::log10(absdouble(dx))));
    if (dx<2*d)
      d=d/5;
    else {
      if (dx<5*d)
	d=d/2;
    }
    return d;
  }

  Graph2d::Graph2d(const giac::gen & g_,const giac::context * cptr):window_xmin(gnuplot_xmin),window_xmax(gnuplot_xmax),window_ymin(gnuplot_ymin),window_ymax(gnuplot_ymax),g(g_),display_mode(0x45),show_axes(1),show_names(1),labelsize(16),contextptr(cptr) {
    update();
    autoscale();
  }
  
  void Graph2d::zoomx(double d,bool round){
    double x_center=(window_xmin+window_xmax)/2;
    double dx=(window_xmax-window_xmin);
    if (dx==0)
      dx=gnuplot_xmax-gnuplot_xmin;
    dx *= d/2;
    x_tick = find_tick(dx);
    window_xmin = x_center - dx;
    if (round) 
      window_xmin=int( window_xmin/x_tick -1)*x_tick;
    window_xmax = x_center + dx;
    if (round)
      window_xmax=int( window_xmax/x_tick +1)*x_tick;
    update();
  }

  void Graph2d::zoomy(double d,bool round){
    double y_center=(window_ymin+window_ymax)/2;
    double dy=(window_ymax-window_ymin);
    if (dy==0)
      dy=gnuplot_ymax-gnuplot_ymin;
    dy *= d/2;
    y_tick = find_tick(dy);
    window_ymin = y_center - dy;
    if (round)
      window_ymin=int( window_ymin/y_tick -1)*y_tick;
    window_ymax = y_center + dy;
    if (round)
      window_ymax=int( window_ymax/y_tick +1)*y_tick;
    update();
  }

  void Graph2d::zoom(double d){ 
    zoomx(d);
    zoomy(d);
  }

  void Graph2d::autoscale(bool fullview){
    // Find the largest and lowest x/y/z in objects (except lines/plans)
    vector<double> vx,vy,vz;
    int s;
    bool ortho=autoscaleg(g,vx,vy,vz,contextptr);
    autoscaleminmax(vx,window_xmin,window_xmax,fullview);
    zoomx(1.0);
    autoscaleminmax(vy,window_ymin,window_ymax,fullview);
    zoomy(1.0);
    if (window_xmax-window_xmin<1e-100){
      window_xmax=gnuplot_xmax;
      window_xmin=gnuplot_xmin;
    }
    if (window_ymax-window_ymin<1e-100){
      window_ymax=gnuplot_ymax;
      window_ymin=gnuplot_ymin;
    }
    bool do_ortho=ortho;
    if (!do_ortho){
      double w=LCD_WIDTH_PX;
      double h=LCD_HEIGHT_PX-STATUS_AREA_PX;
      double window_w=window_xmax-window_xmin,window_h=window_ymax-window_ymin;
      double tst=h/w*window_w/window_h;
      if (tst>0.7 && tst<1.4)
	do_ortho=true;
    }
    if (do_ortho )
      orthonormalize();
    y_tick=find_tick(window_ymax-window_ymin);
    update();
  }

  void Graph2d::orthonormalize(){ 
    // Center of the directions, orthonormalize
    double w=LCD_WIDTH_PX;
    double h=LCD_HEIGHT_PX-STATUS_AREA_PX;
    double window_w=window_xmax-window_xmin,window_h=window_ymax-window_ymin;
    double window_hsize=h/w*window_w;
    if (window_h > window_hsize*1.01){ // enlarge horizontally
      double window_xcenter=(window_xmin+window_xmax)/2;
      double window_wsize=w/h*window_h;
      window_xmin=window_xcenter-window_wsize/2;
      window_xmax=window_xcenter+window_wsize/2;
    }
    if (window_h < window_hsize*0.99) { // enlarge vertically
      double window_ycenter=(window_ymin+window_ymax)/2;
      window_ymin=window_ycenter-window_hsize/2;
      window_ymax=window_ycenter+window_hsize/2;
    }
    x_tick=find_tick(window_xmax-window_xmin);
    y_tick=find_tick(window_ymax-window_ymin);
    update();
  }

  void Graph2d::update(){
    x_scale=LCD_WIDTH_PX/(window_xmax-window_xmin);    
    y_scale=(LCD_HEIGHT_PX-STATUS_AREA_PX)/(window_ymax-window_ymin);    
  }

  bool Graph2d::findij(const gen & e0,double x_scale,double y_scale,double & i0,double & j0,GIAC_CONTEXT) const {
    gen e,f0,f1;
    evalfdouble2reim(e0,e,f0,f1,contextptr);
    if ((f0.type==_DOUBLE_) && (f1.type==_DOUBLE_)){
      if (display_mode & 0x400){
	if (f0._DOUBLE_val<=0)
	  return false;
	f0=std::log10(f0._DOUBLE_val);
      }
      i0=(f0._DOUBLE_val-window_xmin)*x_scale;
      if (display_mode & 0x800){
	if (f1._DOUBLE_val<=0)
	  return false;
	f1=std::log10(f1._DOUBLE_val);
      }
      j0=(window_ymax-f1._DOUBLE_val)*y_scale;
      return true;
    }
    // cerr << "Invalid drawing data" << endl;
    return false;
  }

  inline void swapint(int & i0,int & i1){
    int tmp=i0;
    i0=i1;
    i1=tmp;
  }

  void check_fl_draw(int fontsize,const char * ch,int i0,int j0,int imin,int jmin,int di,int dj,int delta_i,int delta_j,int c){
    /* int n=fl_size();
       if (j0>=jmin-n && j0<=jmin+dj+n) */
    // cerr << i0 << " " << j0 << endl;
    if (strlen(ch)>200)
      text_print(fontsize,"String too long",i0+delta_i,j0+delta_j,c);
    else
      text_print(fontsize,ch,i0+delta_i,j0+delta_j,c);
  }

  inline void check_fl_point(int i0,int j0,int imin,int jmin,int di,int dj,int delta_i,int delta_j,int c){
    /* if (i0>=imin && i0<=imin+di && j0>=jmin && j0<=jmin+dj) */
    os_set_pixel(i0+delta_i,j0+delta_j,c);
  }

  inline void fl_line(int x0,int y0,int x1,int y1,int c){
    draw_line(x0,y0,x1,y1,c);
  }

  inline void fl_polygon(int x0,int y0,int x1,int y1,int x2,int y2,int c){
    draw_line(x0,y0,x1,y1,c);
    draw_line(x1,y1,x2,y2,c);
    draw_line(x2,y2,x0,y0,c);
  }

  inline void check_fl_line(int i0,int j0,int i1,int j1,int imin,int jmin,int di,int dj,int delta_i,int delta_j,int c){
    fl_line(i0+delta_i,j0+delta_j,i1+delta_i,j1+delta_j,c);
  }

  int logplot_points=20;

  void checklog_fl_line(double i0,double j0,double i1,double j1,double deltax,double deltay,bool logx,bool logy,double window_xmin,double x_scale,double window_ymax,double y_scale,int c){
    if (!logx && !logy){
      fl_line(round(i0+deltax),round(j0+deltay),round(i1+deltax),round(j1+deltay),c);
      return;
    }
  }

  void find_dxdy(const string & legendes,int labelpos,int labelsize,int & dx,int & dy){
    int l=text_width(labelsize,legendes.c_str());
    dx=3;
    dy=1;
    switch (labelpos){
    case 1:
      dx=-l-3;
      break;
    case 2:
      dx=-l-3;
      dy=labelsize-2;
      break;
    case 3:
      dy=labelsize-2;
      break;
    }
    //dy += labelsize;
  }

  void draw_legende(const vecteur & f,int i0,int j0,int labelpos,const Graph2d * iptr,int clip_x,int clip_y,int clip_w,int clip_h,int deltax,int deltay,int c,GIAC_CONTEXT){
    if (f.empty() ||!iptr->show_names )
      return;
    string legendes;
    if (f[0].is_symb_of_sommet(at_curve)){
      gen & f0=f[0]._SYMBptr->feuille;
      if (f0.type==_VECT && !f0._VECTptr->empty()){
	gen & f1 = f0._VECTptr->front();
	if (f1.type==_VECT && f1._VECTptr->size()>4 && (!is_zero((*f1._VECTptr)[4]) || (iptr->show_names & 2)) ){
	  gen legende=f1._VECTptr->front();
	  gen var=(*f1._VECTptr)[1];
	  gen r=re(legende,contextptr),i=im(legende,contextptr),a,b;
	  if (var.type==_IDNT && is_linear_wrt(r,*var._IDNTptr,a,b,contextptr)){
	    i=subst(i,var,(var-b)/a,false,contextptr);
	    legendes=i.print(contextptr);
	  }
	  else
	    legendes=r.print(contextptr)+","+i.print(contextptr);
	  if (legendes.size()>18){
	    if (legendes.size()>30)
	      legendes="";
	    else
	      legendes=legendes.substr(0,16)+"...";
	  }
	}
      }
    }
    if (f.size()>2)
      legendes=gen2string(f[2])+(legendes.empty()?"":":")+legendes;
    if (legendes.empty())
      return;
    int fontsize=iptr->labelsize;
    int dx=3,dy=1;
    find_dxdy(legendes,labelpos,fontsize,dx,dy);
    check_fl_draw(fontsize,legendes.c_str(),i0+dx,j0+dy,clip_x,clip_y,clip_w,clip_h,deltax,deltay,c);
  }

  void petite_fleche(double i1,double j1,double dx,double dy,int deltax,int deltay,int width,int c){
    double dxy=std::sqrt(dx*dx+dy*dy);
    if (dxy){
      dxy/=max(2,min(5,int(dxy/10)))+width;
      dx/=dxy;
      dy/=dxy;
      double dxp=-dy,dyp=dx; // perpendicular
      dx*=std::sqrt(3.0);
      dy*=sqrt(3.0);
      fl_polygon(round(i1)+deltax,round(j1)+deltay,round(i1+dx+dxp)+deltax,round(j1+dy+dyp)+deltay,round(i1+dx-dxp)+deltax,round(j1+dy-dyp)+deltay,c);
    }
  }

  void fltk_point(int deltax,int deltay,int i0,int j0,int epaisseur_point,int type_point,int c){
    switch (type_point){
    case 1: // losange
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0,deltay+j0-epaisseur_point,c);
      fl_line(deltax+i0,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0,deltay+j0+epaisseur_point,c);
      fl_line(deltax+i0,deltay+j0+epaisseur_point,deltax+i0+epaisseur_point,deltay+j0,c);
      break;
    case 2: // croix verticale
      fl_line(deltax+i0,deltay+j0-epaisseur_point,deltax+i0,deltay+j0+epaisseur_point,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0+epaisseur_point,deltay+j0,c);
      break;
    case 3: // carre
      fl_line(deltax+i0-epaisseur_point,deltay+j0-epaisseur_point,deltax+i0-epaisseur_point,deltay+j0+epaisseur_point,c);
      fl_line(deltax+i0+epaisseur_point,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0+epaisseur_point,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0-epaisseur_point,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0+epaisseur_point,deltax+i0+epaisseur_point,deltay+j0+epaisseur_point,c);
      break;
    case 5: // triangle
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0,deltay+j0-epaisseur_point,c);
      fl_line(deltax+i0,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0+epaisseur_point,deltay+j0,c);
      break;
    case 7: // point
      if (epaisseur_point>2)
	fl_arc(deltax+i0-(epaisseur_point-1),deltay+j0-(epaisseur_point-1),2*(epaisseur_point-1),2*(epaisseur_point-1),0,360,c);
      else
	fl_line(deltax+i0,deltay+j0,deltax+i0+1,deltay+j0,c);
      break;
    case 6: // etoile
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0+epaisseur_point,deltay+j0,c);
      // no break to add the following lines
    case 0: // 0 croix diagonale
      fl_line(deltax+i0-epaisseur_point,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0+epaisseur_point,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0+epaisseur_point,deltax+i0+epaisseur_point,deltay+j0-epaisseur_point,c);
      break;
    default: // 4 nothing drawn
      break;
    }
  }

  int horiz_or_vert(const_iterateur jt,GIAC_CONTEXT){
    gen tmp(*(jt+1)-*jt),r,i;
    reim(tmp,r,i,contextptr);
    if (is_zero(r,contextptr)) return 1;
    if (is_zero(i,contextptr)) return 2;
    return 0;
  }

  void fltk_draw(Graph2d & Mon_image,const gen & g,double x_scale,double y_scale,int clip_x,int clip_y,int clip_w,int clip_h,GIAC_CONTEXT){
    int deltax=0,deltay=STATUS_AREA_PX,fontsize=Mon_image.labelsize;
    if (g.type==_VECT){
      const vecteur & v=*g._VECTptr;
      const_iterateur it=v.begin(),itend=v.end();
      for (;it!=itend;++it)
	fltk_draw(Mon_image,*it,x_scale,y_scale,clip_x,clip_y,clip_w,clip_h,contextptr);
    }
    if (g.type!=_SYMB)
      return;
    unary_function_ptr s=g._SYMBptr->sommet;
    if (g._SYMBptr->feuille.type!=_VECT)
      return;
    vecteur f=*g._SYMBptr->feuille._VECTptr;
    int mxw=LCD_WIDTH_PX,myw=LCD_HEIGHT_PX-STATUS_AREA_PX;
    double i0,j0,i0save,j0save,i1,j1;
    int fs=f.size();
    if ((fs==4) && (s==at_parameter)){
      return ;
    }
    string the_legend;
    vecteur style(get_style(f,the_legend));
    int styles=style.size();
    // color
    int ensemble_attributs = style.front().val;
    bool hidden_name = false;
    if (style.front().type==_ZINT){
      ensemble_attributs = mpz_get_si(*style.front()._ZINTptr);
      hidden_name=true;
    }
    else
      hidden_name=ensemble_attributs<0;
    int width           =(ensemble_attributs & 0x00070000) >> 16; // 3 bits
    int epaisseur_point =(ensemble_attributs & 0x00380000) >> 19; // 3 bits
    int type_line       =(ensemble_attributs & 0x01c00000) >> 22; // 3 bits
    if (type_line>4)
      type_line=(type_line-4)<<8;
    int type_point      =(ensemble_attributs & 0x0e000000) >> 25; // 3 bits
    int labelpos        =(ensemble_attributs & 0x30000000) >> 28; // 2 bits
    bool fill_polygon   =(ensemble_attributs & 0x40000000) >> 30;
    int couleur         =(ensemble_attributs & 0x0007ffff);
    epaisseur_point += 2;
    if (s==at_pnt){ 
      // f[0]=complex pnt or vector of complex pnts or symbolic
      // f[1] -> style 
      // f[2] optional=label
      gen point=f[0];
      if (point.type==_VECT && point.subtype==_POINT__VECT)
	return;
      if ( (f[0].type==_SYMB) && (f[0]._SYMBptr->sommet==at_curve) && (f[0]._SYMBptr->feuille.type==_VECT) && (f[0]._SYMBptr->feuille._VECTptr->size()) ){
	// Mon_image.show_mouse_on_object=false;
	point=f[0]._SYMBptr->feuille._VECTptr->back();
	if (type_line>=4 && point.type==_VECT && point._VECTptr->size()>2){
	  vecteur v=*point._VECTptr;
	  int vs=v.size()/2; // 3 -> 1
	  if (Mon_image.findij(v[vs],x_scale,y_scale,i0,j0,contextptr) && Mon_image.findij(v[vs+1],x_scale,y_scale,i1,j1,contextptr)){
	    bool logx=Mon_image.display_mode & 0x400,logy=Mon_image.display_mode & 0x800;
	    checklog_fl_line(i0,j0,i1,j1,deltax,deltay,logx,logy,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	    double dx=i0-i1,dy=j0-j1;
	    petite_fleche(i1,j1,dx,dy,deltax,deltay,width+3,couleur);
	  }
	}
      }
      if (is_undef(point))
	return;
      // fl_line_style(type_line,width+1,0); 
      if (point.type==_SYMB) {
	if (point._SYMBptr->sommet==at_cercle){
	  vecteur v=*point._SYMBptr->feuille._VECTptr;
	  gen diametre=remove_at_pnt(v[0]);
	  gen e1=diametre._VECTptr->front().evalf_double(1,contextptr),e2=diametre._VECTptr->back().evalf_double(1,contextptr);
	  gen centre=rdiv(e1+e2,2.0,contextptr);
	  gen e12=e2-e1;
	  double ex=evalf_double(re(e12,contextptr),1,contextptr)._DOUBLE_val,ey=evalf_double(im(e12,contextptr),1,contextptr)._DOUBLE_val;
	  if (!Mon_image.findij(centre,x_scale,y_scale,i0,j0,contextptr))
	    return;
	  gen diam=std::sqrt(ex*ex+ey*ey);
	  gen angle=std::atan2(ey,ex);
	  gen a1=v[1].evalf_double(1,contextptr),a2=v[2].evalf_double(1,contextptr);
	  bool full=v[1]==0 && v[2]==cst_two_pi;
	  if ( (diam.type==_DOUBLE_) && (a1.type==_DOUBLE_) && (a2.type==_DOUBLE_) ){
	    i1=diam._DOUBLE_val*x_scale/2.0;
	    j1=diam._DOUBLE_val*y_scale/2.0;
	    double a1d=a1._DOUBLE_val,a2d=a2._DOUBLE_val,angled=angle._DOUBLE_val;
	    bool changer_sens=a1d>a2d;
	    if (changer_sens){
	      double tmp=a1d;
	      a1d=a2d;
	      a2d=tmp;
	    }
	    double anglei=(angled+a1d),anglef=(angled+a2d),anglem=(anglei+anglef)/2;
	    if (fill_polygon)
	      fl_pie(deltax+round(i0-i1),deltay+round(j0-j1),round(2*i1),round(2*j1),full?0:anglei*180/M_PI+.5,full?360:anglef*180/M_PI+.5,couleur,false);
	    else {
	      fl_arc(deltax+round(i0-i1),deltay+round(j0-j1),round(2*i1),round(2*j1),full?0:anglei*180/M_PI+.5,full?360:anglef*180/M_PI+.5,couleur);
	      if (v.size()>=4){ // if cercle has the optionnal 5th arg
		if (v[3]==2)
		  petite_fleche(i0+i1*std::cos(anglem),j0-j1*std::sin(anglem),-i1*std::sin(anglem),-j1*std::cos(anglem),deltax,deltay,width,couleur);
		else {
		  if (changer_sens)
		    petite_fleche(i0+i1*std::cos(anglei),j0-j1*std::sin(anglei),-i1*std::sin(anglei),-j1*std::cos(anglei),deltax,deltay,width,couleur);
		  else
		    petite_fleche(i0+i1*std::cos(anglef),j0-j1*std::sin(anglef),i1*std::sin(anglef),j1*std::cos(anglef),deltax,deltay,width,couleur);
		}
	      }
	    }
	    // Label a few degrees from the start angle, 
	    // FIXME should use labelpos
	    double anglel=angled+a1d+0.3;
	    if (v.size()>=4 && v[3]==2)
	      anglel=angled+(0.45*a1d+0.55*a2d);
	    i0=i0+i1*std::cos(anglel); 
	    j0=j0-j1*std::sin(anglel);
	    if (!hidden_name)
	      draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	    return;
	  }
	} // end circle
#if 0
	if (point._SYMBptr->sommet==at_legende){
	  gen & f=point._SYMBptr->feuille;
	  if (f.type==_VECT && f._VECTptr->size()==3){
	    vecteur & fv=*f._VECTptr;
	    if (fv[0].type==_VECT && fv[0]._VECTptr->size()>=2 && fv[1].type==_STRNG && fv[2].type==_INT_){
	      vecteur & fvv=*fv[0]._VECTptr;
	      if (fvv[0].type==_INT_ && fvv[1].type==_INT_){
		int dx=0,dy=0;
		string legendes(*fv[1]._STRNGptr);
		find_dxdy(legendes,labelpos,fontsize,dx,dy);
		text_print(fontsize,legendes.c_str(),deltax+fvv[0].val+dx,deltay+fvv[1].val+dy,fv[2].val);
	      }
	    }
	  }
	}
#endif
      } // end point.type==_SYMB
      if (point.type!=_VECT || (point.type==_VECT && (point.subtype==_GROUP__VECT || point.subtype==_VECTOR__VECT) && point._VECTptr->size()==2 && is_zero(point._VECTptr->back()-point._VECTptr->front())) ){ // single point
	if (!Mon_image.findij((point.type==_VECT?point._VECTptr->front():point),x_scale,y_scale,i0,j0,contextptr))
	  return;
	if (i0>0 && i0<mxw && j0>0 && j0<myw)
	  fltk_point(deltax,deltay,round(i0),round(j0),epaisseur_point,type_point,couleur);
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	return;
      }
      // path
      const_iterateur jt=point._VECTptr->begin(),jtend=point._VECTptr->end();
      if (jt==jtend)
	return;
      bool logx=Mon_image.display_mode & 0x400,logy=Mon_image.display_mode & 0x800;
      if (jt->type==_VECT)
	return;
      if ( (type_point || epaisseur_point>2) && type_line==0 && width==0){
	for (;jt!=jtend;++jt){
	  if (!Mon_image.findij(*jt,x_scale,y_scale,i0,j0,contextptr))
	    return;
	  if (i0>0 && i0<mxw && j0>0 && j0<myw)
	    fltk_point(deltax,deltay,round(i0),round(j0),epaisseur_point,type_point,couleur);
	}
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	return;
      }
      // initial point
      if (!Mon_image.findij(*jt,x_scale,y_scale,i0,j0,contextptr))
	return;
      i0save=i0;
      j0save=j0;
      if (fill_polygon){
	if (jtend-jt==5 && *(jt+4)==*jt){
	  // check rectangle parallel to axes -> draw_rectangle (filled)
	  int cote1=horiz_or_vert(jt,contextptr);
	  if (cote1 && horiz_or_vert(jt+1,contextptr)==3-cote1 && horiz_or_vert(jt+2,contextptr)==cote1 && horiz_or_vert(jt+3,contextptr)==3-cote1){
	    if (!Mon_image.findij(*(jt+2),x_scale,y_scale,i0,j0,contextptr))
	      return;
	    int x,y,w,h;
	    if (i0<i0save){
	      x=i0;
	      w=i0save-i0;
	    }
	    else {
	      x=i0save;
	      w=i0-i0save;
	    }
	    if (j0<j0save){
	      y=j0;
	      h=j0save-j0;
	    }
	    else {
	      y=j0save;
	      h=j0-j0save;
	    }
	    draw_rectangle(deltax+x,deltay+y,w,h,couleur);
	    if (!hidden_name)
	      draw_legende(f,deltax+x,deltay+y,labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	    return;
	  }
	} // end rectangle check
	bool closed=*jt==*(jtend-1);
	vector< vector<int> > vi(jtend-jt+(closed?0:1),vector<int>(2));
	for (int pos=0;jt!=jtend;++pos,++jt){
	  if (!Mon_image.findij(*jt,x_scale,y_scale,i0,j0,contextptr))
	    return;
	  vi[pos][0]=i0+deltax;
	  vi[pos][1]=j0+deltay;
	}
	if (!closed)
	  vi.back()=vi.front();
	draw_filled_polygon(vi,0,LCD_WIDTH_PX,0,LCD_HEIGHT_PX,couleur);
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	return;
      }
      ++jt;
      if (jt==jtend){
	if (i0>0 && i0<mxw && j0>0 && j0<myw)
	  check_fl_point(deltax+round(i0),deltay+round(j0),clip_x,clip_y,clip_w,clip_h,0,0,couleur);
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	return;
      }
      bool seghalfline=( point.subtype==_LINE__VECT || point.subtype==_HALFLINE__VECT ) && (point._VECTptr->size()==2);
      // rest of the path
      for (;;){
	if (!Mon_image.findij(*jt,x_scale,y_scale,i1,j1,contextptr))
	  return;
	if (!seghalfline){
	  checklog_fl_line(i0,j0,i1,j1,deltax,deltay,logx,logy,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	  if (point.subtype==_VECTOR__VECT){
	    double dx=i0-i1,dy=j0-j1;
	    petite_fleche(i1,j1,dx,dy,deltax,deltay,width,couleur);
	  }
	}
	++jt;
	if (jt==jtend){ // label of line at midpoint
	  if (point.subtype==_LINE__VECT){
	    i0=(6*i1-i0)/5-8;
	    j0=(6*j1-j0)/5-8;
	  }
	  else {
	    i0=(i0+i1)/2-8;
	    j0=(j0+j1)/2;
	  }
	  break;
	}
	i0=i1;
	j0=j1;
      }
      // check for a segment/halfline/line
      if ( seghalfline){
	double deltai=i1-i0save,adeltai=absdouble(deltai);
	double deltaj=j1-j0save,adeltaj=absdouble(deltaj);
	if (point.subtype==_LINE__VECT){
	  if (deltai==0)
	    checklog_fl_line(i1,0,i1,clip_h,deltax,deltay,logx,logy,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	  else {
	    if (deltaj==0)
	      checklog_fl_line(0,j1,clip_w,j1,deltax,deltay,Mon_image.display_mode & 0x400,Mon_image.display_mode & 0x800,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	    else {
	      // Find the intersections with the 4 rectangle segments
	      // Horizontal x=0 or w =i1+t*deltai: y=j1+t*deltaj
	      vector< complex<double> > pts;
	      double y0=j1-i1/deltai*deltaj,tol=clip_h*1e-6;
	      if (y0>=-tol && y0<=clip_h+tol)
		pts.push_back(complex<double>(0.0,y0));
	      double yw=j1+(clip_w-i1)/deltai*deltaj;
	      if (yw>=-tol && yw<=clip_h+tol)
		pts.push_back(complex<double>(clip_w,yw));
	      // Vertical y=0 or h=j1+t*deltaj, x=i1+t*deltai
	      double x0=i1-j1/deltaj*deltai;
	      tol=clip_w*1e-6;
	      if (x0>=-tol && x0<=clip_w+tol)
		pts.push_back(complex<double>(x0,0.0));
	      double xh=i1+(clip_h-j1)/deltaj*deltai;
	      if (xh>=-tol && xh<=clip_w+tol)
		pts.push_back(complex<double>(xh,clip_h));
	      if (pts.size()>=2)
		checklog_fl_line(pts[0].real(),pts[0].imag(),pts[1].real(),pts[1].imag(),deltax,deltay,Mon_image.display_mode & 0x400,Mon_image.display_mode & 0x800,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	    } // end else adeltai==0 , adeltaj==0
	  } // end else adeltai==0
	} // end LINE_VECT
	else {
	  double N=1;
	  if (adeltai){
	    N=clip_w/adeltai+1;
	    if (adeltaj)
	      N=max(N,clip_h/adeltaj+1);
	  }
	  else {
	    if (adeltaj)
	      N=clip_h/adeltaj+1;
	  }
	  N *= 2; // increase N since rounding might introduce too small clipping
	  while (fabs(N*deltai)>10000)
	    N /= 2;
	  while (fabs(N*deltaj)>10000)
	    N /= 2;
	  checklog_fl_line(i0save,j0save,i1+N*deltai,j1+N*deltaj,deltax,deltay,Mon_image.display_mode & 0x400,Mon_image.display_mode & 0x800,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	}
      } // end seghalfline
      if ( (point.subtype==_GROUP__VECT) && (point._VECTptr->size()==2))
	; // no legend for segment
      else {
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
      }
    } // end pnt subcase
  }
#endif

  // return a vector of values with simple decimal representation
  // between xmin/xmax or including xmin/xmax (if bounds is true)
  vecteur ticks(double xmin,double xmax,bool bounds){
    if (xmax<xmin)
      swapdouble(xmin,xmax);
    double dx=xmax-xmin;
    vecteur res;
    if (dx==0)
      return res;
    double d=std::pow(10.0,std::floor(std::log10(dx)));
    if (dx<2*d)
      d=d/5;
    else {
      if (dx<5*d)
	d=d/2;
    }
    double x1=std::floor(xmin/d)*d;
    double x2=(bounds?std::ceil(xmax/d):std::floor(xmax/d))*d;
    for (double x=x1+(bounds?0:d);x<=x2;x+=d){
      if (absdouble(x-int(x+.5))<1e-6*d)
	res.push_back(int(x+.5));
      else
	res.push_back(x);
    }
    return res;
  }

  void Graph2d::draw(){
    int save_clip_ymin=clip_ymin;
    clip_ymin=STATUS_AREA_PX;
    int horizontal_pixels=LCD_WIDTH_PX,vertical_pixels=LCD_HEIGHT_PX-STATUS_AREA_PX,deltax=0,deltay=STATUS_AREA_PX,clip_x=0,clip_y=0,clip_w=horizontal_pixels,clip_h=vertical_pixels;
    drawRectangle(0, STATUS_AREA_PX, horizontal_pixels, vertical_pixels,COLOR_WHITE);
    // Draw axis
    double I0,J0;
    findij(zero,x_scale,y_scale,I0,J0,contextptr); // origin
    int i_0=round(I0),j_0=round(J0);
    if (show_axes &&  (window_ymax>=0) && (window_ymin<=0)){ // X-axis
      vecteur aff; int affs;
      char ch[256];
      check_fl_line(deltax,deltay+j_0,deltax+horizontal_pixels,deltay+j_0,clip_x,clip_y,clip_w,clip_h,0,0,_GREEN); 
      check_fl_line(deltax+i_0,deltay+j_0,deltax+i_0+int(x_scale),deltay+j_0,clip_x,clip_y,clip_w,clip_h,0,0,_CYAN);
      aff=ticks(window_xmin,window_xmax,true);
      affs=aff.size();
      for (int i=0;i<affs;++i){
	double d=evalf_double(aff[i],1,contextptr)._DOUBLE_val;
	if (fabs(d)<1e-6) strcpy(ch,"0"); else sprint_double(ch,d);
	int delta=int(horizontal_pixels*(d-window_xmin)/(window_xmax-window_xmin));
	int taille=strlen(ch)*9;
	fl_line(delta,deltay+j_0,delta,deltay+j_0-4,_GREEN);
      }
      check_fl_draw(labelsize,"x",deltax+horizontal_pixels-40,deltay+j_0-4,clip_x,clip_y,clip_w,clip_h,0,0,_GREEN);
    }
    if ( show_axes && (window_xmax>=0) && (window_xmin<=0) ) {// Y-axis
      vecteur aff; int affs;
      char ch[256];
      check_fl_line(deltax+i_0,deltay,deltax+i_0,deltay+vertical_pixels,clip_x,clip_y,clip_w,clip_h,0,0,_RED);
      check_fl_line(deltax+i_0,deltay+j_0,deltax+i_0,deltay+j_0-int(y_scale),clip_x,clip_y,clip_w,clip_h,0,0,_CYAN);
      aff=ticks(window_ymin,window_ymax,true);
      affs=aff.size();
      int taille=5;
      for (int j=0;j<affs;++j){
	double d=evalf_double(aff[j],1,contextptr)._DOUBLE_val;
	if (fabs(d)<1e-6) strcpy(ch,"0"); else sprint_double(ch,d);
	int delta=int(vertical_pixels*(window_ymax-d)/(window_ymax-window_ymin));
	if (delta>=taille && delta<=vertical_pixels-taille){
	  fl_line(deltax+i_0,STATUS_AREA_PX+delta,deltax+i_0+4,STATUS_AREA_PX+delta,_RED);
	}
      }
      check_fl_draw(labelsize,"y",deltax+i_0+2,deltay+labelsize,clip_x,clip_y,clip_w,clip_h,0,0,_RED);
    }
#if 0 // if ticks are enabled, don't forget to set freeze to false
    // Ticks
    if (show_axes && (horizontal_pixels)/(x_scale*x_tick) < 40 && vertical_pixels/(y_tick*y_scale) <40  ){
      if (x_tick>0 && y_tick>0 ){
	double nticks=(horizontal_pixels-I0)/(x_scale*x_tick);
	double mticks=(vertical_pixels-J0)/(y_tick*y_scale);
	int count=0;
	for (int ii=int(-I0/(x_tick*x_scale));ii<=nticks;++ii){
	  int iii=int(I0+ii*x_scale*x_tick+.5);
	  for (int jj=int(-J0/(y_tick*y_scale));jj<=mticks && count<1600;++jj,++count){
	    int jjj=int(J0+jj*y_scale*y_tick+.5);
	    check_fl_point(deltax+iii,deltay+jjj,clip_x,clip_y,clip_w,clip_h,0,0,COLOR_BLACK);
	  }
	}
      }
    }
#endif
    if (show_axes){ 
      int taille,affs,delta;
      vecteur aff;
      char ch[256];
      // X
      aff=ticks(window_xmin,window_xmax,true);
      affs=aff.size();
      for (int i=0;i<affs;++i){
	double d=evalf_double(aff[i],1,contextptr)._DOUBLE_val;
	sprint_double(ch,d);
	delta=int(horizontal_pixels*(d-window_xmin)/(window_xmax-window_xmin));
	taille=strlen(ch)*9;
	fl_line(delta,vertical_pixels+STATUS_AREA_PX-6,delta,vertical_pixels+STATUS_AREA_PX-1,_GREEN);
	if (delta>=taille/2 && delta<=horizontal_pixels){
	  text_print(10,ch,delta-taille/2,vertical_pixels+STATUS_AREA_PX-7,_GREEN);
	}
      }
      // Y
      aff=ticks(window_ymin,window_ymax,true);
      affs=aff.size();
      taille=5;
      for (int j=0;j<affs;++j){
	double d=evalf_double(aff[j],1,contextptr)._DOUBLE_val;
	sprint_double(ch,d);
	delta=int(vertical_pixels*(window_ymax-d)/(window_ymax-window_ymin));
	if (delta>=taille && delta<=vertical_pixels-taille){
	  fl_line(horizontal_pixels-5,STATUS_AREA_PX+delta,horizontal_pixels-1,STATUS_AREA_PX+delta,_RED);
	  text_print(10,ch,horizontal_pixels-strlen(ch)*9,STATUS_AREA_PX+delta+taille,_RED);
	}
      }
    }
    
    // draw
    fltk_draw(*this,g,x_scale,y_scale,clip_x,clip_y,clip_w,clip_h,contextptr);
    clip_ymin=save_clip_ymin;
  }
  
  void Graph2d::left(double d){ 
    window_xmin -= d;
    window_xmax -= d;
  }

  void Graph2d::right(double d){ 
    window_xmin += d;
    window_xmax += d;
  }

  void Graph2d::up(double d){ 
    window_ymin += d;
    window_ymax += d;
  }

  void Graph2d::down(double d){ 
    window_ymin -= d;
    window_ymax -= d;
  }

  void Turtle::draw(){
    const int deltax=0,deltay=0;
    int horizontal_pixels=LCD_WIDTH_PX-2*giac::COORD_SIZE;
    // Check for fast redraw
    // Then redraw the background
    drawRectangle(deltax, deltay, LCD_WIDTH_PX, LCD_HEIGHT_PX,COLOR_WHITE);
    if (turtleptr &&
#ifdef TURTLETAB
	turtle_stack_size
#else
	!turtleptr->empty()
#endif
	){
      if (turtlezoom>8)
	turtlezoom=8;
      if (turtlezoom<0.125)
	turtlezoom=0.125;
      // check that position is not out of screen
#ifdef TURTLETAB
      logo_turtle t=turtleptr[turtle_stack_size-1];
#else
      logo_turtle t=turtleptr->back();
#endif
      double x=turtlezoom*(t.x-turtlex);
      if (x<0)
	turtlex += int(x/turtlezoom);
      if (x>=LCD_WIDTH_PX-10)
	turtlex += int((x-LCD_WIDTH_PX+10)/turtlezoom);
      double y=turtlezoom*(t.y-turtley);
      if (y<0)
	turtley += int(y/turtlezoom);
      if (y>LCD_HEIGHT_PX-10)
	turtley += int((y-LCD_HEIGHT_PX+10)/turtlezoom);
    }
#if 0
    if (maillage & 0x3){
      fl_color(FL_BLACK);
      double xdecal=std::floor(turtlex/10.0)*10;
      double ydecal=std::floor(turtley/10.0)*10;
      if ( (maillage & 0x3)==1){
	for (double i=xdecal;i<LCD_WIDTH_PX+xdecal;i+=10){
	  for (double j=ydecal;j<LCD_HEIGHT_PX+ydecal;j+=10){
	    fl_point(deltax+int((i-turtlex)*turtlezoom+.5),deltay+LCD_HEIGHT_PX-int((j-turtley)*turtlezoom+.5));
	  }
	}
      }
      else {
	double dj=std::sqrt(3.0)*10,i0=xdecal;
	for (double j=ydecal;j<LCD_HEIGHT_PX+ydecal;j+=dj){
	  int J=deltay+int(LCD_HEIGHT_PX-(j-turtley)*turtlezoom);
	  for (double i=i0;i<LCD_WIDTH_PX+xdecal;i+=10){
	    fl_point(deltax+int((i-turtlex)*turtlezoom+.5),J);
	  }
	  i0 += dj;
	  while (i0>=10)
	    i0 -= 10;
	}
      }
    }
#endif
    // Show turtle position/cap
    if (turtleptr &&
#ifdef TURTLETAB
	turtle_stack_size &&
#else
	!turtleptr->empty() &&
#endif
	!(maillage & 0x4)){
#ifdef TURTLETAB
      logo_turtle turtle=turtleptr[turtle_stack_size-1];
#else
      logo_turtle turtle=turtleptr->back();
#endif
      drawRectangle(deltax+horizontal_pixels,deltay,LCD_WIDTH_PX-horizontal_pixels,2*COORD_SIZE,_YELLOW);
      // drawRectangle(deltax, deltay, LCD_WIDTH_PX, LCD_HEIGHT_PX,COLOR_BLACK);
      char buf[32];
      sprintf(buf,"x %i   ",int(turtle.x+.5));
      text_print(18,buf,deltax+horizontal_pixels,deltay+(2*COORD_SIZE)/3-2,COLOR_BLACK,_YELLOW);
      sprintf(buf,"y %i   ",int(turtle.y+.5));
      text_print(18,buf,deltax+horizontal_pixels,deltay+(4*COORD_SIZE)/3-3,COLOR_BLACK,_YELLOW);
      sprintf(buf,"t %i   ",int(turtle.theta+.5));
      text_print(18,buf,deltax+horizontal_pixels,deltay+2*COORD_SIZE-4,COLOR_BLACK,_YELLOW);
    }
    // draw turtle Logo
    if (turtleptr){
#ifdef TURTLETAB
      int l=turtle_stack_size;
#else
      int l=turtleptr->size();
#endif
      if (l>0){
#ifdef TURTLETAB
	logo_turtle prec =turtleptr[0];
#else
	logo_turtle prec =(*turtleptr)[0];
#endif
	int sp=speed;
	for (int k=1;k<l;++k){
	  if (k>=2 && sp){
	    sync_screen();
	    for (int i=0;i<speed;++i){
	      for (int j=0;j<1000;++j){
		if (iskeydown(5) || iskeydown(4) || iskeydown(22)){
		  sp=0;
		  break;
		}
	      }
	    }
	  }
#ifdef TURTLETAB
	  logo_turtle current =(turtleptr)[k];
#else
	  logo_turtle current =(*turtleptr)[k];
#endif
#if 1
	  if (current.s>=0){ // Write a string
	    //cout << current.radius << " " << current.s << endl;
	    if (current.s<ecristab().size())
	      text_print(current.radius,ecristab()[current.s].c_str(),int(deltax+turtlezoom*(current.x-turtlex)),int(deltay+LCD_HEIGHT_PX-turtlezoom*(current.y-turtley)),current.color);
	  }
	  else
#endif
	    {
	      if (current.radius>0){
		int r=current.radius & 0x1ff; // bit 0-8
		double theta1,theta2;
		if (current.direct){
		  theta1=prec.theta+double((current.radius >> 9) & 0x1ff); // bit 9-17
		  theta2=prec.theta+double((current.radius >> 18) & 0x1ff); // bit 18-26
		}
		else {
		  theta1=prec.theta-double((current.radius >> 9) & 0x1ff); // bit 9-17
		  theta2=prec.theta-double((current.radius >> 18) & 0x1ff); // bit 18-26
		}
		bool rempli=(current.radius >> 27) & 0x1;
		bool seg=(current.radius >> 28) & 0x1;
		double angle;
		int x,y,R;
		R=int(2*turtlezoom*r+.5);
		angle = M_PI/180*(theta2-90);
		if (current.direct){
		  x=int(turtlezoom*(current.x-turtlex-r*std::cos(angle) - r)+.5);
		  y=int(turtlezoom*(current.y-turtley-r*std::sin(angle) + r)+.5);
		}
		else {
		  x=int(turtlezoom*(current.x-turtlex+r*std::cos(angle) -r)+.5);
		  y=int(turtlezoom*(current.y-turtley+r*std::sin(angle) +r)+.5);
		}
		if (current.direct){
		  if (rempli)
		    fl_pie(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,theta1-90,theta2-90,current.color,seg);
		  else
		    fl_arc(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,theta1-90,theta2-90,current.color);
		}
		else {
		  if (rempli)
		    fl_pie(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,90+theta2,90+theta1,current.color,seg);
		  else
		    fl_arc(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,90+theta2,90+theta1,current.color);
		}
	      } // end radius>0
	      else {
		if (prec.mark){
		  fl_line(deltax+int(turtlezoom*(prec.x-turtlex)+.5),deltay+int(LCD_HEIGHT_PX+turtlezoom*(turtley-prec.y)+.5),deltax+int(turtlezoom*(current.x-turtlex)+.5),deltay+int(LCD_HEIGHT_PX+turtlezoom*(turtley-current.y)+.5),prec.color);
		}
	      }
	      if (current.radius<-1 && k+current.radius>=0){
		// poly-line from (*turtleptr)[k+current.radius] to (*turtleptr)[k]
		vector< vector<int> > vi(1-current.radius,vector<int>(2));
		for (int i=0;i>=current.radius;--i){
#ifdef TURTLETAB
		  logo_turtle & t=(turtleptr)[k+i];
#else
		  logo_turtle & t=(*turtleptr)[k+i];
#endif
		  vi[-i][0]=deltax+turtlezoom*(t.x-turtlex);
		  vi[-i][1]=deltay+LCD_HEIGHT_PX+turtlezoom*(turtley-t.y);
		  //*logptr(contextptr) << i << " " << vi[-i][0] << " " << vi[-i][1] << endl;
		}
		//vi.back()=vi.front();
		draw_filled_polygon(vi,0,LCD_WIDTH_PX,24,LCD_HEIGHT_PX,current.color);
	      }
	    } // end else (non-string turtle record)
	  prec=current;
	} // end for (all turtle records)
#ifdef TURTLETAB
	logo_turtle & t = (turtleptr)[l-1];
#else
	logo_turtle & t = (*turtleptr)[l-1];
#endif
	int x=int(turtlezoom*(t.x-turtlex)+.5);
	int y=int(turtlezoom*(t.y-turtley)+.5);
	double cost=std::cos(t.theta*deg2rad_d);
	double sint=std::sin(t.theta*deg2rad_d);
	int Dx=int(turtlezoom*t.turtle_length*cost/2+.5);
	int Dy=int(turtlezoom*t.turtle_length*sint/2+.5);
	if (t.visible){
	  fl_line(deltax+x+Dy,deltay+LCD_HEIGHT_PX-(y-Dx),deltax+x-Dy,deltay+LCD_HEIGHT_PX-(y+Dx),t.color);
	  int c=t.color;
	  if (!t.mark)
	    c=t.color ^ 0x7777;
	  fl_line(deltax+x+Dy,deltay+LCD_HEIGHT_PX-(y-Dx),deltax+x+3*Dx,deltay+LCD_HEIGHT_PX-(y+3*Dy),c);
	  fl_line(deltax+x-Dy,deltay+LCD_HEIGHT_PX-(y+Dx),deltax+x+3*Dx,deltay+LCD_HEIGHT_PX-(y+3*Dy),c);
	}
      }
      return;
    } // End logo mode
  }  
  

  int displaygraph(const giac::gen & ge,GIAC_CONTEXT){
    // graph display
    //if (aborttimer > 0) { Timer_Stop(aborttimer); Timer_Deinstall(aborttimer);}
    xcas::Graph2d gr(ge,contextptr);
    gr.show_axes=global_show_axes;
    // initial setting for x and y
    if (ge.type==_VECT){
      const_iterateur it=ge._VECTptr->begin(),itend=ge._VECTptr->end();
      for (;it!=itend;++it){
	if (it->is_symb_of_sommet(at_equal)){
	  const gen & f=it->_SYMBptr->feuille;
	  gen & optname = f._VECTptr->front();
	  gen & optvalue= f._VECTptr->back();
	  if (optname.val==_AXES && optvalue.type==_INT_)
	    gr.show_axes=optvalue.val;
	  if (optname.type==_INT_ && optname.subtype == _INT_PLOT && optname.val>=_GL_X && optname.val<=_GL_Z && optvalue.is_symb_of_sommet(at_interval)){
	    //*logptr(contextptr) << optname << " " << optvalue << endl;
	    gen optvf=evalf_double(optvalue._SYMBptr->feuille,1,contextptr);
	    if (optvf.type==_VECT && optvf._VECTptr->size()==2){
	      gen a=optvf._VECTptr->front();
	      gen b=optvf._VECTptr->back();
	      if (a.type==_DOUBLE_ && b.type==_DOUBLE_){
		switch (optname.val){
		case _GL_X:
		  gr.window_xmin=a._DOUBLE_val;
		  gr.window_xmax=b._DOUBLE_val;
		  gr.update();
		  break;
		case _GL_Y:
		  gr.window_ymin=a._DOUBLE_val;
		  gr.window_ymax=b._DOUBLE_val;
		  gr.update();
		  break;
		}
	      }
	    }
	  }
	}
      }
    }
    // UI
#ifdef NSPIRE_NEWLIB
    DefineStatusMessage((char*)"+-: zoom, pad: move, esc: quit", 1, 0, 0);
#else
    DefineStatusMessage((char*)"+-: zoom, pad: move, EXIT: quit", 1, 0, 0);
#endif
    // EnableStatusArea(2);
    for (;;){
      gr.draw();
      DisplayStatusArea();
      // int x=0,y=LCD_HEIGHT_PX-STATUS_AREA_PX-17;
      // PrintMini(&x,&y,(unsigned char *)"menu",0x04,0xffffffff,0,0,COLOR_BLACK,COLOR_WHITE,1,0);
      int key=-1;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;
#if 1
      if (key==KEY_CTRL_CATALOG || key==KEY_BOOK){
	char menu_xmin[32],menu_xmax[32],menu_ymin[32],menu_ymax[32];
	string s;
	s="xmin "+print_DOUBLE_(gr.window_xmin,contextptr);
	strcpy(menu_xmin,s.c_str());
	s="xmax "+print_DOUBLE_(gr.window_xmax,contextptr);
	strcpy(menu_xmax,s.c_str());
	s="ymin "+print_DOUBLE_(gr.window_ymin,contextptr);
	strcpy(menu_ymin,s.c_str());
	s="ymax "+print_DOUBLE_(gr.window_ymax,contextptr);
	strcpy(menu_ymax,s.c_str());
	Menu smallmenu;
	smallmenu.numitems=12;
	MenuItem smallmenuitems[smallmenu.numitems];
	smallmenu.items=smallmenuitems;
	smallmenu.height=12;
	//smallmenu.title = "KhiCAS";
	smallmenuitems[0].text = (char *) menu_xmin;
	smallmenuitems[1].text = (char *) menu_xmax;
	smallmenuitems[2].text = (char *) menu_ymin;
	smallmenuitems[3].text = (char *) menu_ymax;
	smallmenuitems[4].text = (char*) "Orthonormalize /";
	smallmenuitems[5].text = (char*) "Autoscale *";
	smallmenuitems[6].text = (char *) ("Zoom in +");
	smallmenuitems[7].text = (char *) ("Zoom out -");
	smallmenuitems[8].text = (char *) ("Y-Zoom out (-)");
	smallmenuitems[9].text = (char*) ((lang==1)?"Voir axes":"Show axes");
	smallmenuitems[10].text = (char*) ((lang==1)?"Cacher axes":"Hide axes");
	smallmenuitems[11].text = (char*)((lang==1)?"Quitter":"Quit");
	int sres = doMenu(&smallmenu);
	if(sres == MENU_RETURN_SELECTION || sres==KEY_CTRL_EXE) {
	  const char * ptr=0;
	  string s1; double d;
	  if (smallmenu.selection==1){
	    if (inputdouble(menu_xmin,d,contextptr)){
	      gr.window_xmin=d;
	      gr.update();
	    }
	  }
	  if (smallmenu.selection==2){
	    if (inputdouble(menu_xmax,d,contextptr)){
	      gr.window_xmax=d;
	      gr.update();
	    }
	  }
	  if (smallmenu.selection==3){
	    if (inputdouble(menu_ymin,d,contextptr)){
	      gr.window_ymin=d;
	      gr.update();
	    }
	  }
	  if (smallmenu.selection==4){
	    if (inputdouble(menu_ymax,d,contextptr)){
	      gr.window_ymax=d;
	      gr.update();
	    }
	  }
	  if (smallmenu.selection==5)
	    gr.orthonormalize();
	  if (smallmenu.selection==6)
	    gr.autoscale();	
	  if (smallmenu.selection==7)
	    gr.zoom(0.7);	
	  if (smallmenu.selection==8)
	    gr.zoom(1/0.7);	
	  if (smallmenu.selection==9)
	    gr.zoomy(1/0.7);
	  if (smallmenu.selection==10)
	    gr.show_axes=true;	
	  if (smallmenu.selection==11)
	    gr.show_axes=false;	
	  if (smallmenu.selection==12)
	    break;
	}
      }
#endif
      if (key==KEY_CTRL_EXIT || key==KEY_CTRL_OK){
	os_hide_graph();
	break;
      }
      if (key==KEY_CTRL_UP){ gr.up((gr.window_ymax-gr.window_ymin)/5); }
      if (key==KEY_CTRL_PAGEUP) { gr.up((gr.window_ymax-gr.window_ymin)/2); }
      if (key==KEY_CTRL_DOWN) { gr.down((gr.window_ymax-gr.window_ymin)/5); }
      if (key==KEY_CTRL_PAGEDOWN) { gr.down((gr.window_ymax-gr.window_ymin)/2);}
      if (key==KEY_CTRL_LEFT) { gr.left((gr.window_xmax-gr.window_xmin)/5); }
      if (key==KEY_SHIFT_LEFT) { gr.left((gr.window_xmax-gr.window_xmin)/2); }
      if (key==KEY_CTRL_RIGHT) { gr.right((gr.window_xmax-gr.window_xmin)/5); }
      if (key==KEY_SHIFT_RIGHT) { gr.right((gr.window_xmax-gr.window_xmin)/5); }
      if (key==KEY_CHAR_PLUS) {
	gr.zoom(0.7);
      }
      if (key==KEY_CHAR_MINUS){
	gr.zoom(1/0.7);
      }
      if (key==KEY_CHAR_PMINUS){
	gr.zoomy(1/0.7);
      }
      if (key==KEY_CHAR_MULT){
	gr.autoscale();
      }
      if (key==KEY_CHAR_DIV) {
	gr.orthonormalize();
      }
      if (key==KEY_CTRL_VARS) {
	gr.show_axes=!gr.show_axes;
      }
    }
    // aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
    return 0;
  }

  int displaylogo(){
#ifdef TURTLETAB
    xcas::Turtle t={tablogo,0,0,1,1,(short) turtle_speed};
#else
    xcas::Turtle t={&turtle_stack(),0,0,1,1,(short) turtle_speed};
#endif
#ifdef NSPIRE_NEWLIB
    DefineStatusMessage((char*)"+-: zoom, pad: move, esc: quit", 1, 0, 0);
#else
    DefineStatusMessage((char*)"+-: zoom, pad: move, EXIT: quit", 1, 0, 0);
#endif
    DisplayStatusArea();
    bool redraw=true;
    while (1){
      int save_ymin=clip_ymin;
      clip_ymin=24;
      if (redraw)
	t.draw();
      redraw=false;
      clip_ymin=save_ymin;
      int key;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;
      if (key==KEY_CTRL_EXIT || key==KEY_CTRL_OK || key==KEY_PRGM_ACON || key==KEY_CTRL_MENU || key==KEY_CTRL_EXE || key==KEY_CTRL_VARS || key==KEY_CHAR_ANS)
	break;
      if (key==KEY_CTRL_UP){ t.turtley += 10; redraw=true; }
      if (key==KEY_CTRL_PAGEUP) { t.turtley += 100; redraw=true;}
      if (key==KEY_CTRL_DOWN) { t.turtley -= 10; redraw=true;}
      if (key==KEY_CTRL_PAGEDOWN) { t.turtley -= 100;redraw=true;}
      if (key==KEY_CTRL_LEFT) { t.turtlex -= 10; redraw=true;}
      if (key==KEY_SHIFT_LEFT) { t.turtlex -= 100; redraw=true;}
      if (key==KEY_CTRL_RIGHT) { t.turtlex += 10; redraw=true;}
      if (key==KEY_SHIFT_RIGHT) { t.turtlex += 100;redraw=true;}
      if (key==KEY_CHAR_PLUS) { t.turtlezoom *= 2;redraw=true;}
      if (key==KEY_CHAR_MINUS){ t.turtlezoom /= 2; redraw=true; }
      if (key==KEY_CHAR_MULT){ if (t.speed) t.speed *=2; else t.speed=10; redraw=true; }
      if (key==KEY_CHAR_DIV){ t.speed /=2; redraw=true; }
      if (key=='='){ redraw=true; }
    }
    os_hide_graph();
    return 0;
  }

  bool ispnt(const gen & g){
    if (g.is_symb_of_sommet(giac::at_pnt))
      return true;
    if (g.type!=_VECT || g._VECTptr->empty())
      return false;
    return ispnt(g._VECTptr->back());
  }

  void translate_fkey(int & input_key){
    if (input_key==KEY_CTRL_MIXEDFRAC) input_key=KEY_CTRL_F10;
    if (input_key==KEY_CTRL_FRACCNVRT) input_key=KEY_CTRL_F7;
    if (input_key==KEY_CHAR_LIST) input_key=KEY_CTRL_F9;
    if (input_key==KEY_CHAR_MAT) input_key=KEY_CTRL_F8;
    if (input_key==KEY_CTRL_PRGM) input_key=KEY_CTRL_F12;
    if (input_key==KEY_CTRL_FD) input_key=KEY_CTRL_F11;
    if (input_key==KEY_CHAR_ANGLE) input_key=KEY_CTRL_F13;
    if (input_key==KEY_CHAR_FRAC) input_key=KEY_CTRL_F14;
  }

  giac::gen eqw(const giac::gen & ge,bool editable,GIAC_CONTEXT){
    if (ge.is_symb_of_sommet(at_equal) && ge._SYMBptr->feuille.type==_VECT && ge._SYMBptr->feuille._VECTptr->size()==2 && ge._SYMBptr->feuille._VECTptr->front().type==_INT_ && ge._SYMBptr->feuille._VECTptr->back().type==_INT_){
      global_show_axes=ge._SYMBptr->feuille._VECTptr->back().val;
      return ge;
    }
    if (ge.is_symb_of_sommet(at_erase)){
      global_show_axes=1;
      return ge;
    }
    bool edited=false;
    const int margin=16;
#ifdef CURSOR
    Cursor_SetFlashOff();
#endif
    giac::gen geq(_copy(ge,contextptr));
    // if (ge.type!=giac::_DOUBLE_ && giac::has_evalf(ge,geq,1,contextptr)) geq=giac::symb_equal(ge,geq);
    int line=-1,col=-1,nlines=0,ncols=0,listormat=0;
    xcas::Equation eq(0,0,geq,contextptr);
    giac::eqwdata eqdata=xcas::Equation_total_size(eq.data);
    if (eqdata.dx>1.5*LCD_WIDTH_PX || eqdata.dy>1.5*LCD_HEIGHT_PX){
      if (eqdata.dx>2.25*LCD_WIDTH_PX || eqdata.dy>2.25*LCD_HEIGHT_PX)
	eq.attr=giac::attributs(14,COLOR_WHITE,COLOR_BLACK);
      else
	eq.attr=giac::attributs(16,COLOR_WHITE,COLOR_BLACK);
      eq.data=0; // clear memory
      eq.data=xcas::Equation_compute_size(geq,eq.attr,LCD_WIDTH_PX,contextptr);
      eqdata=xcas::Equation_total_size(eq.data);
    }
    int dx=(eqdata.dx-LCD_WIDTH_PX)/2,dy=LCD_HEIGHT_PX-2*margin+eqdata.y;
    if (geq.type==_VECT){
      nlines=geq._VECTptr->size();
      if (eqdata.dx>=LCD_WIDTH_PX)
	dx=-20; // line=nlines/2;
      //else
      if (geq.subtype!=_SEQ__VECT){
	line=0;
	listormat=1;
	if (ckmatrix(geq)){
	  ncols=geq._VECTptr->front()._VECTptr->size();
	  if (eqdata.dy>=LCD_HEIGHT_PX-margin)
	    dy=eqdata.y+eqdata.dy+32;// col=ncols/2;
	  // else
	  col=0;
	  listormat=2;
	}
      }
    }
    if (!listormat){
      xcas::Equation_select(eq.data,true);
      xcas::eqw_select_down(eq.data);
    }
    //cout << eq.data << endl;
    int firstrun=2;
    for (;;){
#if 1
      if (firstrun==2){
#ifdef NSPIRE_NEWLIB
	DefineStatusMessage((char*)((lang==1)?"ctrl enter: eval, esc: quitte, ":"ctrl enter: eval, esc: exit"), 1, 0, 0);
#else
	DefineStatusMessage((char*)((lang==1)?"EXE: quitte, resultat dans last":"EXE: quit, result stored in last"), 1, 0, 0);
#endif
	DisplayStatusArea();
	firstrun=1;
      }
      else
	set_xcas_status();
#else
      DefineStatusMessage((char*)"+-: zoom, pad: move, EXIT: quit", 1, 0, 0);
      EnableStatusArea(2);
      DisplayStatusArea();
#endif
      gen value;
      if (listormat) // select line l, col c
	xcas::eqw_select(eq.data,line,col,true,value);
      if (eqdata.dx>LCD_WIDTH_PX){
	if (dx<-20)
	  dx=-20;
	if (dx>eqdata.dx-LCD_WIDTH_PX+20)
	  dx=eqdata.dx-LCD_WIDTH_PX+20;
      }
#define EQW_TAILLE 18
      if (eqdata.dy>LCD_HEIGHT_PX-2*EQW_TAILLE){
	if (dy-eqdata.y<LCD_HEIGHT_PX-2*EQW_TAILLE)
	  dy=eqdata.y+LCD_HEIGHT_PX-2*EQW_TAILLE;
	if (dy-eqdata.y>eqdata.dy+32)
	  dy=eqdata.y+eqdata.dy+32;
      }
      waitforvblank();
      drawRectangle(0, 0, LCD_WIDTH_PX, 205,COLOR_WHITE);
      // Bdisp_AllClr_VRAM();
      int save_clip_ymin=clip_ymin;
      clip_ymin=STATUS_AREA_PX;
      xcas::display(eq,dx,dy,contextptr);
#if 1
      string menu("shift-1 ");
      menu += string(menu_f1);
      menu += "|2 ";
      menu += string(menu_f2);
      menu += "|3 undo|4 edt|5 +-|6 approx";
      drawRectangle(0,205,LCD_WIDTH_PX,17,22222);
      PrintMiniMini(0,205,menu.c_str(),4,22222,giac::_BLACK);
#endif
      //draw_menu(2);
      clip_ymin=save_clip_ymin;
      int keyflag = GetSetupSetting( (unsigned int)0x14);
      if (firstrun){ // workaround for e.g. 1+x/2 partly not displayed
	firstrun=0;
	continue;
      }
      int key;
      //cout << eq.data << endl;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return undef;
      bool alph=alphawasactive(&key);
      if (key==KEY_CTRL_OK || key==KEY_CTRL_MENU){
	os_hide_graph();
	if (edited && xcas::do_select(eq.data,true,value) && value.type==_EQW){
	  //cout << "ok " << value._EQWptr->g << endl;
	  DefineStatusMessage(((lang==1)?"resultat stocke dans last":"result stored in last"), 1, 0, 0);
	  //DisplayStatusArea();
	  giac::sto(value._EQWptr->g,giac::gen("last",contextptr),contextptr);
	  return value._EQWptr->g;
	}
	//cout << "no " << eq.data << endl; if (value.type==_EQW) cout << value._EQWptr->g << endl ;
	return geq;
      }
      if (key==KEY_CTRL_EXIT || key==KEY_CTRL_AC ){
	if (!edited){
	  os_hide_graph();
	  return geq;
	}
	if (confirm(
#ifdef NSPIRE_NEWLIB
		    (lang==1)?"Vraiment abandonner?":"Really leave",(lang==1)?"esc: editeur,  enter: confirmer":"esc: editor,  enter: confirm"
#else
		    (lang==1)?"Vraiment abandonner?":"Really leave",(lang==1)?"Back: editeur,  OK: confirmer":"Back: editor,  OK: confirm"
#endif
		    )==KEY_CTRL_F1){
	  os_hide_graph();
	  return geq;
	}
      }
      if (key==KEY_CTRL_F3)
	key=KEY_CTRL_UNDO;
      if (key==KEY_CTRL_UNDO){
	giac::swapgen(eq.undodata,eq.data);
	if (listormat){
	  xcas::do_select(eq.data,true,value);
	  if (value.type==_EQW){
	    gen g=eval(value._EQWptr->g,1,contextptr);
	    if (g.type==_VECT){
	      const vecteur & v=*g._VECTptr;
	      nlines=v.size();
	      if (line >= nlines)
		line=nlines-1;
	      if (col!=-1 &&v.front().type==_VECT){
		ncols=v.front()._VECTptr->size();
		if (col>=ncols)
		  col=ncols-1;
	      }
	      xcas::do_select(eq.data,false,value);
	      xcas::eqw_select(eq.data,line,col,true,value);
	    }
	  }
	}
	continue;
      }
      int redo=0; 
      if (listormat){
	if (key==KEY_CHAR_COMMA || key==KEY_CTRL_DEL ){
	  xcas::do_select(eq.data,true,value);
	  if (value.type==_EQW){
	    gen g=eval(value._EQWptr->g,1,contextptr);
	    if (g.type==_VECT){
	      edited=true; eq.undodata=Equation_copy(eq.data);
	      vecteur v=*g._VECTptr;
	      if (key==KEY_CHAR_COMMA){
		if (col==-1 || (line>0 && line==nlines-1)){
		  v.insert(v.begin()+line+1,0*v.front());
		  ++line; ++nlines;
		}
		else {
		  v=mtran(v);
		  v.insert(v.begin()+col+1,0*v.front());
		  v=mtran(v);
		  ++col; ++ncols;
		}
	      }
	      else {
		if (col==-1 || (nlines>=3 && line==nlines-1)){
		  if (nlines>=(col==-1?2:3)){
		    v.erase(v.begin()+line,v.begin()+line+1);
		    if (line) --line;
		    --nlines;
		  }
		}
		else {
		  if (ncols>=2){
		    v=mtran(v);
		    v.erase(v.begin()+col,v.begin()+col+1);
		    v=mtran(v);
		    if (col) --col; --ncols;
		  }
		}
	      }
	      geq=gen(v,g.subtype);
	      key=0; redo=1;
	      // continue;
	    }
	  }
	}
      }
      bool ins=key==KEY_CHAR_STORE  || key==KEY_CHAR_RPAR || key==KEY_CHAR_LPAR || key==KEY_CHAR_COMMA || key==KEY_CTRL_PASTE || key==KEY_CTRL_F4;
      int xleft,ytop,xright,ybottom,gselpos; gen * gsel=0,*gselparent=0;
      if (key==KEY_CTRL_CLIP){
	xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,0);
	if (gsel==0)
	  gsel==&eq.data;
	// cout << "var " << g << " " << eq.data << endl;
	if (xcas::do_select(*gsel,true,value) && value.type==_EQW){
	  //cout << g << ":=" << value._EQWptr->g << endl;
	  copy_clipboard(value._EQWptr->g.print(contextptr),true);
	  continue;
	}
      }
      if (key==KEY_CHAR_STORE){
	int keyflag = GetSetupSetting( (unsigned int)0x14);
	if (keyflag==0)
	  handle_f5();
	std::string varname;
	if (inputline(((lang==1)?"Stocker la selection dans":"Save selection in",(lang==1)?"Nom de variable: ":"Variable name: "),0,varname,false,65,contextptr) && !varname.empty() && isalpha(varname[0])){
	  giac::gen g(varname,contextptr);
	  giac::gen ge(protecteval(g,1,contextptr));
	  if (g.type!=_IDNT){
	    invalid_varname();
	    continue;
	  }
	  if (ge==g || confirm_overwrite()){
	    vector<int> goto_sel;
	    xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel);
	    if (gsel==0)
	      gsel==&eq.data;
	    // cout << "var " << g << " " << eq.data << endl;
	    if (xcas::do_select(*gsel,true,value) && value.type==_EQW){
	      //cout << g << ":=" << value._EQWptr->g << endl;
	      giac::gen gg(value._EQWptr->g);
	      if (gg.is_symb_of_sommet(at_makevector))
		gg=giac::eval(gg,1,contextptr);
	      giac::sto(gg,g,contextptr);
	    }
	  }
	}
	continue;
      }
      if (key==KEY_CTRL_DEL){
	vector<int> goto_sel;
	if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel && xcas::do_select(*gsel,true,value) && value.type==_EQW){
	  value=value._EQWptr->g;
	  if (value.type==_SYMB){
	    gen tmp=value._SYMBptr->feuille;
	    if (tmp.type!=_VECT || tmp.subtype!=_SEQ__VECT){
	      xcas::replace_selection(eq,tmp,gsel,&goto_sel,contextptr);
	      continue;
	    }
	  }
	  if (!goto_sel.empty() && gselparent && gselparent->type==_VECT && !gselparent->_VECTptr->empty()){
	    vecteur & v=*gselparent->_VECTptr;
	    if (v.back().type==_EQW){
	      gen opg=v.back()._EQWptr->g;
	      if (opg.type==_FUNC){
		int i=0;
		for (;i<v.size()-1;++i){
		  if (&v[i]==gsel)
		    break;
		}
		if (i<v.size()-1){
		  if (v.size()==5 && (opg==at_integrate || opg==at_sum) && i>=2)
		    v.erase(v.begin()+2,v.begin()+4);
		  else
		    v.erase(v.begin()+i);
		  xcas::do_select(*gselparent,true,value);
		  if (value.type==_EQW){
		    value=value._EQWptr->g;
		    // cout << goto_sel << " " << value << endl; continue;
		    if (v.size()==2 && (opg==at_plus || opg==at_prod))
		      value=protecteval(value,1,contextptr);
		    goto_sel.erase(goto_sel.begin());
		    xcas::replace_selection(eq,value,gselparent,&goto_sel,contextptr);
		    continue;
		  }
		}
	      }
	    }
	  }
	}
      }
      if (key=='\\' || key==KEY_CTRL_F5){
	xcas::do_select(eq.data,true,value);
	if (value.type==_EQW)
	  geq=value._EQWptr->g;
	if (eq.attr.fontsize<=14)
	  eq.attr.fontsize=18;
	else
	  eq.attr.fontsize=14;
	redo=1;
      }
      if (key==KEY_CHAR_IMGNRY)
	key='i';
      const char keybuf[2]={(key==KEY_CHAR_PMINUS?'-':char(key)),0};
      const char * adds=(key==KEY_CTRL_F4 || key==KEY_CHAR_PMINUS ||
			 (key==char(key) && (isalphanum(key)|| key=='.' ))
			 )?keybuf:keytostring(key,keyflag,contextptr);
      if (adds && !strcmp(adds,"VARS()"))
	continue;
      translate_fkey(key);
      if ( key==KEY_CTRL_F1 || key==KEY_CTRL_F2 ||
	   (key>=KEY_CTRL_F7 && key<=KEY_CTRL_F14)){
	adds=console_menu(key,fmenu_cfg,1);//alph?"simplify":(keyflag==1?"factor":"partfrac");
	// workaround for infinitiy
	if (!adds) continue;
	if (strlen(adds)>=2 && adds[0]=='o' && adds[1]=='o')
	  key=KEY_CTRL_F3;      
      }
      if (key==KEY_CTRL_F6 || key==KEY_CTRL_EXE){
	adds= (key==KEY_CTRL_F6?"evalf":"eval");
      }
      if (key==KEY_CHAR_MINUS)
	adds="-";
      if (key==KEY_CHAR_EQUAL)
	adds="=";
      if (key==KEY_CHAR_RECIP)
	adds="inv";
      if (key==KEY_CHAR_SQUARE)
	adds="sq";
      if (key==KEY_CHAR_POWROOT)
	adds="surd";
      if (key==KEY_CHAR_CUBEROOT)
	adds="surd";
      if (key==KEY_CHAR_FACTOR)
	adds="factor";
      if (key==KEY_CHAR_NORMAL)
	adds="normal";
      int addssize=adds?strlen(adds):0;
      // cout << addssize << " " << adds << endl;
      if (0 && key==KEY_CTRL_EXE){
	if (xcas::do_select(eq.data,true,value) && value.type==_EQW){
	  //cout << "ok " << value._EQWptr->g << endl;
	  DefineStatusMessage(((lang==1)?"resultat stocke dans last":"result stored in last"), 1, 0, 0);
	  //DisplayStatusArea();
	  giac::sto(value._EQWptr->g,giac::gen("last",contextptr),contextptr);
	  return value._EQWptr->g;
	}
	//cout << "no " << eq.data << endl; if (value.type==_EQW) cout << value._EQWptr->g << endl ;
	return geq;
      }
      if ( key!=KEY_CHAR_MINUS && key!=KEY_CHAR_EQUAL && key!=0 &&
	   (ins || key==KEY_CHAR_PI || key==KEY_CTRL_VARS || key==KEY_CTRL_F3 || (addssize==1 && (isalphanum(adds[0])|| adds[0]=='.' || adds[0]=='-') ) )
	   ){
	edited=true;
	if (line>=0 && xcas::eqw_select(eq.data,line,col,true,value)){
	  string s;
	  if (ins){
	    if (key==KEY_CTRL_PASTE)
	      s=paste_clipboard();
	    else {
	      if (value.type==_EQW){
		s=value._EQWptr->g.print(contextptr);
	      }
	      else
		s=value.print(contextptr);
	    }
	  }
	  else
	    s = adds;
	  string msg("Line ");
	  msg += print_INT_(line+1);
	  msg += " Col ";
	  msg += print_INT_(col+1);
	  if (inputline(msg.c_str(),0,s,false,65,contextptr)==KEY_CTRL_EXE){
	    value=gen(s,contextptr);
	    if (col<0)
	      (*geq._VECTptr)[line]=value;
	    else
	      (*((*geq._VECTptr)[line]._VECTptr))[col]=value;
	    redo=2;
	    key=KEY_SHIFT_RIGHT;
	  }
	  else
	    continue;
	}
	else {
	  vector<int> goto_sel;
	  if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel && xcas::do_select(*gsel,true,value) && value.type==_EQW){
	    string s;
	    if (ins){
	      if (key==KEY_CTRL_PASTE)
		s=paste_clipboard();
	      else {
		s = value._EQWptr->g.print(contextptr);
		if (key==KEY_CHAR_COMMA)
		  s += ',';
	      }
	    }
	    else
	      s = adds;
	    if (inputline(value._EQWptr->g.print(contextptr).c_str(),0,s,false)==KEY_CTRL_EXE){
	      value=gen(s,contextptr);
	      //cout << value << " goto " << goto_sel << endl;
	      xcas::replace_selection(eq,value,gsel,&goto_sel,contextptr);
	      firstrun=-1; // workaround, force 2 times display
	    }
	    continue;
	  }
	}
      }
      if (redo){
	eq.data=0; // clear memory
	eq.data=xcas::Equation_compute_size(geq,eq.attr,LCD_WIDTH_PX,contextptr);
	eqdata=xcas::Equation_total_size(eq.data);
	if (redo==1){
	  dx=(eqdata.dx-LCD_WIDTH_PX)/2;
	  dy=LCD_HEIGHT_PX-2*margin+eqdata.y;
	  if (listormat) // select line l, col c
	    xcas::eqw_select(eq.data,line,col,true,value);
	  else {
	    xcas::Equation_select(eq.data,true);
	    xcas::eqw_select_down(eq.data);
	  }
	  continue;
	}
      }
      bool doit=eqdata.dx>=LCD_WIDTH_PX;
      int delta=0;
      if (listormat){
	if (key==KEY_CTRL_LEFT  || (!doit && key==KEY_SHIFT_LEFT)){
	  if (line>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	    if (col>=0){
	      --col;
	      if (col<0){
		col=ncols-1;
		if (line>0){
		  --line;
		  dy += value._EQWptr->dy+eq.attr.fontsize/2;
		}
	      }
	    }
	    else {
	      if (line>0)
		--line;
	    }
	    xcas::eqw_select(eq.data,line,col,true,value);
	    if (doit) dx -= value._EQWptr->dx;
	  }
	  continue;
	}
	if (doit && key==KEY_SHIFT_LEFT){
	  dx -= 20;
	  continue;
	}
	if (key==KEY_CTRL_RIGHT  || (!doit && key==KEY_SHIFT_RIGHT)) {
	  if (line>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	    if (doit)
	      dx += value._EQWptr->dx;
	    if (col>=0){
	      ++col;
	      if (col==ncols){
		col=0;
		if (line<nlines-1){
		  ++line;
		  dy -= value._EQWptr->dy+eq.attr.fontsize/2;
		}		  
	      }
	    } else {
	      if (line<nlines-1)
		++line;
	    }
	    xcas::eqw_select(eq.data,line,col,true,value);
	  }
	  continue;
	}
	if (key==KEY_SHIFT_RIGHT && doit){
	  dx += 20;
	  continue;
	}
	doit=eqdata.dy>=LCD_HEIGHT_PX-2*margin;
	if (key==KEY_CTRL_UP || (!doit && key==KEY_CTRL_PAGEUP)){
	  if (line>0 && col>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	    --line;
	    xcas::eqw_select(eq.data,line,col,true,value);
	    if (doit)
	      dy += value._EQWptr->dy+eq.attr.fontsize/2;
	  }
	  continue;
	}
	if (key==KEY_CTRL_PAGEUP && doit){
	  dy += 10;
	  continue;
	}
	if (key==KEY_CTRL_DOWN  || (!doit && key==KEY_CTRL_PAGEDOWN)){
	  if (line<nlines-1 && col>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	    if (doit)
	      dy -= value._EQWptr->dy+eq.attr.fontsize/2;
	    ++line;
	    xcas::eqw_select(eq.data,line,col,true,value);
	  }
	  continue;
	}
	if ( key==KEY_CTRL_PAGEDOWN && doit){
	  dy -= 10;
	  continue;
	}
      }
      else { // else listormat
	if (key==KEY_CTRL_LEFT){
	  delta=xcas::eqw_select_leftright(eq,true,alph?2:0,contextptr);
	  // cout << "left " << delta << endl;
	  if (doit) dx += (delta?delta:-20);
	  continue;
	}
	if (key==KEY_SHIFT_LEFT){
	  delta=xcas::eqw_select_leftright(eq,true,1,contextptr);
	  vector<int> goto_sel;
	  if (doit) dx += (delta?delta:-20);
	  continue;
	}
	if (key==KEY_CTRL_RIGHT){
	  delta=xcas::eqw_select_leftright(eq,false,alph?2:0,contextptr);
	  // cout << "right " << delta << endl;
	  if (doit)
	    dx += (delta?delta:20);
	  continue;
	}
	if (key==KEY_SHIFT_RIGHT){
	  delta=xcas::eqw_select_leftright(eq,false,1,contextptr);
	  // cout << "right " << delta << endl;
	  if (doit)
	    dx += (delta?delta:20);
	  // dx=eqdata.dx-LCD_WIDTH_PX+20;
	  continue;
	}
	doit=eqdata.dy>=LCD_HEIGHT_PX-2*margin;
	if (key==KEY_CTRL_UP){
	  delta=xcas::eqw_select_up(eq.data);
	  // cout << "up " << delta << endl;
	  continue;
	}
	//cout << "up " << eq.data << endl;
	if (key==KEY_CTRL_PAGEUP && doit){
	  dy=eqdata.y+eqdata.dy+20;
	  continue;
	}
	if (key==KEY_CTRL_DOWN){
	  delta=xcas::eqw_select_down(eq.data);
	  // cout << "down " << delta << endl;
	  continue;
	}
	//cout << "down " << eq.data << endl;
	if ( key==KEY_CTRL_PAGEDOWN && doit){
	  dy=eqdata.y+LCD_HEIGHT_PX-margin;
	  continue;
	}
      }
      if (adds){
	edited=true;
	if (strcmp(adds,"'")==0)
	  adds="diff";
	if (strcmp(adds,"^2")==0)
	  adds="sq";
	if (strcmp(adds,">")==0)
	  adds="simplify";
	if (strcmp(adds,"<")==0)
	  adds="factor";
	if (strcmp(adds,"#")==0)
	  adds="partfrac";
	string cmd(adds);
	if (cmd.size() && cmd[cmd.size()-1]=='(')
	  cmd ='\''+cmd.substr(0,cmd.size()-1)+'\'';
	vector<int> goto_sel;
	if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel){
	  gen op;
	  int addarg=0;
	  if (addssize==1){
	    switch (adds[0]){
	    case '+':
	      addarg=1;
	      op=at_plus;
	      break;
	    case '^':
	      addarg=1;
	      op=at_pow;
	      break;
	    case '=':
	      addarg=1;
	      op=at_equal;
	      break;
	    case '-':
	      addarg=1;
	      op=at_binary_minus;
	      break;
	    case '*':
	      addarg=1;
	      op=at_prod;
	      break;
	    case '/':
	      addarg=1;
	      op=at_division;
	      break;
	    case '\'':
	      addarg=1;
	      op=at_derive;
	      break;
	    }
	  }
	  if (op==0)
	    op=gen(cmd,contextptr);
	  if (op.type==_SYMB)
	    op=op._SYMBptr->sommet;
	  // cout << "keyed " << adds << " " << op << " " << op.type << endl;
	  if (op.type==_FUNC){
	    edited=true;
	    // execute command on selection
	    gen tmp,value;
	    if (xcas::do_select(*gsel,true,value) && value.type==_EQW){
	      if (op==at_integrate || op==at_sum)
		addarg=3;
	      if (op==at_limit)
		addarg=2;
	      gen args=protecteval(value._EQWptr->g,1,contextptr);
	      gen vx=xthetat?t__IDNT_e:x__IDNT_e;
	      if (addarg==1)
		args=makesequence(args,0);
	      if (addarg==2)
		args=makesequence(args,vx_var,0);
	      if (addarg==3)
		args=makesequence(args,vx_var,0,1);
	      if (op==at_surd)
		args=makesequence(args,key==KEY_CHAR_CUBEROOT?3:4);
	      if (op==at_subst)
		args=makesequence(args,giac::symb_equal(vx_var,0));
	      unary_function_ptr immediate_op[]={*at_eval,*at_evalf,*at_evalc,*at_regrouper,*at_simplify,*at_normal,*at_ratnormal,*at_factor,*at_cfactor,*at_partfrac,*at_cpartfrac,*at_expand,*at_canonical_form,*at_exp2trig,*at_trig2exp,*at_sincos,*at_lin,*at_tlin,*at_tcollect,*at_texpand,*at_trigexpand,*at_trigcos,*at_trigsin,*at_trigtan,*at_halftan};
	      if (equalposcomp(immediate_op,*op._FUNCptr)){
		set_abort();
		tmp=(*op._FUNCptr)(args,contextptr);
		clear_abort();
		esc_flag=0;
		giac::ctrl_c=false;
		kbd_interrupted=giac::interrupted=false;
	      }
	      else
		tmp=symbolic(*op._FUNCptr,args);
	      //cout << "sel " << value._EQWptr->g << " " << tmp << " " << goto_sel << endl;
	      esc_flag=0;
	      giac::ctrl_c=false;
	      kbd_interrupted=giac::interrupted=false;
	      if (!is_undef(tmp)){
		xcas::replace_selection(eq,tmp,gsel,&goto_sel,contextptr);
		if (addarg){
		  xcas::eqw_select_down(eq.data);
		  xcas::eqw_select_leftright(eq,false,0,contextptr);
		}
		eqdata=xcas::Equation_total_size(eq.data);
		dx=(eqdata.dx-LCD_WIDTH_PX)/2;
		dy=LCD_HEIGHT_PX-2*margin+eqdata.y;
		firstrun=-1; // workaround, force 2 times display
	      }
	    }
	  } // if (op.type==_FUNC)
	  
	} // if adjust_xy
      } // if (adds)
    }
    //*logptr(contextptr) << eq.data << endl;
  }
  
  void clear_turtle_history(GIAC_CONTEXT){
    history_in(contextptr).clear();
    history_out(contextptr).clear();
    turtle_stack()=vector<logo_turtle>(1,logo_turtle());
  }    
  
  void do_restart(GIAC_CONTEXT){
    if (contextptr){
      if (contextptr->globalcontextptr && contextptr->globalcontextptr->tabptr)
	contextptr->globalcontextptr->tabptr->clear();
    }
    else
      _restart(0,contextptr);
  }

  void switch_to_micropy(GIAC_CONTEXT){
    xcas_python_eval=1;
    python_compat(4|python_compat(contextptr),contextptr);
    if (edptr)
      edptr->python=1;
    if (do_confirm((lang==1)?"Effacer les variables Xcas?":"Clear Xcas variables?"))
      do_restart(contextptr);
    *logptr(contextptr) << "Micropython interpreter\n";
    Console_FMenu_Init(contextptr);
  }

  void do_run(const char * s,gen & g,gen & ge,const context * & contextptr){
    warn_nr=os_shell=true;
    if (!contextptr)
      contextptr=new giac::context;
    if (!strcmp(s,"restart")){
      clear_turtle_history(contextptr);
      do_restart(contextptr);
      return;
    }
    int S=strlen(s);
    char buf[S+1];
    buf[S]=0;
    for (int i=0;i<S;++i){
      char c=s[i];
      if (c==0x1e || c==char(0x9c))
	buf[i]='\n';
      else {
	if (c==0x0d)
	  buf[i]=' ';
	else
	  buf[i]=c;
      }
    }
    g=gen(buf,contextptr);
    //Console_Output(g.print(contextptr).c_str()); return ;
    giac::freeze=false;
    // execution_in_progress = 1;
    set_abort();
    ge=protecteval(equaltosto(g,contextptr),1,contextptr);
    clear_abort();
    // execution_in_progress = 0;
    if (esc_flag || ctrl_c){
      esc_flag=ctrl_c=interrupted=false;
      while (confirm("Interrupted","OK",true)==-1)
	; // insure ON has been removed from keyboard buffer
      ge=string2gen("Interrupted",false);
      // memory full?
      if (!kbd_interrupted){
	// clear turtle, display msg
	clear_turtle_history(contextptr);
	int res=confirm((lang==1)?"Memoire remplie! Purger":"Memory full. Purge",
#ifdef NSPIRE_NEWLIB
			(lang==1)?"enter: variable, esc: tout.":"enter: variables, esc: all",
#else
			(lang==1)?"EXE variable, Back: tout.":"EXE variables, Back: all",
#endif
			false);
	if (res==KEY_CTRL_F1 && select_var(contextptr).type==_IDNT){
	  size_t savestackptr = stackptr;
#ifdef x86_64
	  stackptr=0xffffffffffffffff;
#else
	  stackptr=0xffffffff;
#endif
	  _purge(g,contextptr);
	  stackptr=savestackptr;
	}
	else
	  do_restart(contextptr);
      }
    }
    //Console_Output("Done"); return ;
    esc_flag=0;
    giac::ctrl_c=false;
    giac::kbd_interrupted=giac::interrupted=false;
  }

#ifdef NSPIRE_NEWLIB
  const unsigned char rsa_n_tab[]=
  {
   0xf2,0x0e,0xd4,0x9d,0x44,0x04,0xc4,0xc8,0x6a,0x5b,0xc6,0x9a,0xd6,0xdf,
   0x9c,0xf5,0x56,0xf2,0x0d,0xad,0x6c,0x34,0xb4,0x48,0xf7,0xa7,0xa8,0x27,0xa0,
   0xc8,0xbe,0x36,0xb1,0xc0,0x95,0xf8,0xc2,0x72,0xfb,0x78,0x0f,0x3f,0x15,0x22,
   0xaf,0x51,0x96,0xe3,0xdc,0x39,0xb4,0xc6,0x40,0x6d,0x58,0x56,0x1f,0xad,0x55,
   0x55,0x08,0xf1,0xde,0x5a,0xbc,0xd3,0xcc,0x16,0x3d,0x33,0xee,0x83,0x3f,0x32,
   0xa7,0xa7,0xb8,0x95,0x2f,0x35,0xeb,0xf6,0x32,0x4d,0x22,0xd9,0x60,0xb7,0x5e,
   0xbd,0xea,0xa5,0xcb,0x9c,0x69,0xeb,0xfd,0x9f,0x2b,0x5f,0x3d,0x38,0x5a,0xe1,
   0x2b,0x63,0xf8,0x92,0x35,0x91,0xea,0x77,0x07,0xcc,0x4b,0x7a,0xbc,0xe0,0xa0,
   0x8b,0x82,0x98,0xa2,0x87,0x10,0x2c,0xe2,0x23,0x53,0x2f,0x70,0x03,0xec,0x2d,
   0x22,0x34,0x72,0x57,0x4d,0x24,0x2e,0x97,0xc9,0xfb,0x23,0xb0,0x05,0xff,0x87,
   0x6e,0xbf,0x94,0x2d,0xf0,0x36,0xed,0xd7,0x9a,0xac,0x0c,0x21,0x94,0xa2,0x75,
   0xfc,0x39,0x9b,0xba,0xf2,0xc6,0xc9,0x34,0xa0,0xb2,0x66,0x5a,0xcc,0xc9,0x5c,
   0xc7,0xdb,0xce,0xfb,0x3a,0x10,0xee,0xc1,0x82,0x9a,0x43,0xef,0xed,0x87,0xbd,
   0x6c,0xe4,0xc1,0x36,0xd0,0x0a,0x85,0x6e,0xca,0xcd,0x13,0x29,0x65,0xb5,0xd4,
   0x13,0x4a,0x14,0xaa,0x65,0xac,0x0e,0x6f,0x19,0xb0,0x62,0x47,0x65,0x0e,0x40,
   0x82,0x37,0xd6,0xf0,0x17,0x48,0xaa,0x8c,0x7b,0xc4,0x5e,0x4a,0x72,0x26,0xa6,
   0x08,0x2e,0xff,0x2d,0x9d,0x0e,0x2e,0x19,0xe9,0x6a,0x4c,0x7c,0x3e,0xe9,0xbc,
   0x78,0x95
  };

  gen tabunsignedchar2gen(const unsigned char tab[],int len){
    gen res=0;
    for (int i=0;i<len;++i){
      res=256*res;
      res+=tab[i];
    }
    return res;
  }
  // rsa_check will return a number of keys if the file is made of encrypted sha256
  // fingerprints, and will set the list of keys accordingly
  // decrypted sha256 keys must be written in basis 256
  // as decimal strings of 3 digits
  // a key has 32 bytes -> 96 digits -> crypted as a 96 hexadecimal BCD number
  // 96 bytes = 768 bits + 1280 leadings bits ignored
  // if after decryption one byte is not in '0'..'9' then the key file is wrong
  int rsa_check(const char * filename,int maxkeys,BYTE hash[][SHA256_BLOCK_SIZE]){
    // 2048 bits key
    //gen rsa_n("30556983006074777238153119417050033796377803388439527860005340326999902386793820226251074714511561407075812479599501874865302578278319769475202313110451510448783794266461205935851713896070734772609406958034158877973097041361961511770051269836310307170258399115935233789006376756279696914861909994161265089406023979340582770078210602481999222884431385627202086122099546391904669923221616360112943964540439315592530076604901633280666259500385969154248745363924897530806256116825070881718288938659701112718863366914419207811508217802754887145264781681001930842410022363032920896943814827354941650810105635438172850387093",context0);
    gen rsa_n(tabunsignedchar2gen(rsa_n_tab,sizeof(rsa_n_tab)));
    gen N=pow(gen(2),768),q;
    // read by blocks of 2048 bits=256 bytes
    FILE * f=fopen(filename,"r");
    if (!f)
      return -1;
    for (int i=0;;++i){
      gen key=0;
      // skip 0x prefix
      for (;;){
	unsigned char c=fgetc(f);
	if (feof(f))
	  break;
	if (c=='\n' || c==' ' || c=='0')
	  continue;
	if (c=='x')
	  break;
	// invalid char
	unlink(filename);
	return -2;
      }
      if (feof(f)){
	fclose(f);
	return i;
      }
      for (int j=0;j<256;++j){
	key = 256*key;
	unsigned char c=fgetc(f);
	if (feof(f)){
	  fclose(f);
	  if (j!=0){ unlink(filename); return 0; }
	  return i;
	}
	if (c==' ' || c=='\n')
	  break;
	if (c>='0' && c<='9')
	  c=c-'0';
	else {
	  if (c>='a' && c<='f')
	    c=10+c-'a';
	  else {
	    fclose(f);
	    unlink(filename);
	    return -3;
	  }
	}
	unsigned char d=fgetc(f);
	if (feof(f)){
	  fclose(f);
	  unlink(filename);
	  return -4;
	}
	if (d==' ' || d=='\n'){
	  key = key/16+int(c);
	  break;
	}
	if (d>='0' && d<='9')
	  d=d-'0';
	else {
	  if (d>='a' && d<='f')
	    d=10+d-'a';
	  else {
	    fclose(f);
	    unlink(filename);
	    return -5;
	  }
	}
	key = key+int(c)*16+int(d);
      }
      // public key decrypt and keep only 768 low bits
      //std::cout << key << '\n' ;
      key=powmod(key,65537,rsa_n);
      key=irem(key,N,q); // q should be irem(rsa_n,12345)
      if (q!=12345){
	fclose(f);
	//unlink(filename);
	return -6;
      }
      // check that key is valid and write in hash[i]
      for (int j=0;j<32;++j){
	// divide 3 times by 256, remainder must be in '0'..'9'
	int o=0;
	int tab[]={1,10,100};
	for (int k=0;k<3;++k){
	  gen r=irem(key,256,q);
	  key=q;
	  if (r.type!=_INT_ || r.val>'9' || r.val<'0'){
	    fclose(f);
	    unlink(filename);
	    return -7;
	  }
	  o+=(r.val-'0')*tab[k];
	}
	if (o<0 || o>255){
	  fclose(f);
	  unlink(filename);
	  return -8;
	}
	if (i<maxkeys)
	  hash[i][31-j]=o;
      }
    }
    fclose(f);
    return maxkeys;
  }

  bool sha_check(const char * filename,int nkeys,BYTE hash[][SHA256_BLOCK_SIZE]){
    // must contain sha256 hash for ndless and khicas files (max 32 hash keys)
    // if more keys are needed modify maxkeys here and in buildsha.cc
    // Keys are generated with buildsha.cc (private program)
    // ./a.out ndless/* khicas*tns luagiac.luax.tns 
    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    string text;
    FILE * f=fopen(filename,"r");
    if (!f)
      return false;
    for (;;){
      unsigned char c=fgetc(f);
      if (feof(f))
	break;
      text += c;
    }
    fclose(f);
    unsigned char * ptr=(unsigned char *)text.c_str();
    sha256_init(&ctx);
    sha256_update(&ctx, ptr, text.size());
    sha256_final(&ctx, buf);
    for (int i=0;i<nkeys;++i){
      if (!memcmp(hash[i], buf, SHA256_BLOCK_SIZE))
	return true;
    }
    return false;
  }
  
  DIR * nspire_clear_data(const char * dirname,int nkeys,BYTE hash[][SHA256_BLOCK_SIZE],GIAC_CONTEXT){
    bool toplevel=strcmp(dirname,"/exammode/usr")==0;
    bool ndless=strcmp(dirname,"/exammode/usr/ndless")==0;
    DIR *dp;
    struct dirent *ep;
    dp = opendir (dirname);
    if (!dp)
      return dp;
    string s;
    int t;
    while ( (ep = readdir (dp)) ){
      s=ep->d_name;
      t=s.size();
      if (s=="." || s==".." || s=="NspireLogs.zip" || s=="themes.csv")
	continue;
      int res=0;
      if (t<4 || s.substr(t-4,4)!=".tns"){ // dev, tmp, phoenix, documents, logs, widgets, ptt, metric, wlan, temp_ccp, images
	s=dirname+("/"+s);
	DIR * ptr=nspire_clear_data(s.c_str(),nkeys,hash,contextptr);
	if (ptr && s!="/exammode/usr/ndless" && s!="/exammode/usr/Press-to-Test")
	  res=rmdir(s.c_str());
	// else *logptr(contextptr) << s << '\n'; //
      }
      else {
	if (toplevel || ndless){
	  if (ndless && s=="shakeys.tns")
	    continue;
	  if ( (s=="khicas.tns" || s=="luagiac.luax.tns" || s=="khicaslua.tns" || s=="ptt.tns")){
	    string ss=dirname+("/"+s);
	    if (sha_check(ss.c_str(),nkeys,hash))
	      continue;
	  }
	}
	if (ndless){
	  if ((s.substr(0,17)=="ndless_installer_" || s=="ndless_resources.tns" || s=="ndless.cfg.tns") ){
	    string ss=dirname+("/"+s);
	    if (sha_check(ss.c_str(),nkeys,hash))
	      continue;
	  }
	}
	s=dirname+("/"+s);
	res=unlink(s.c_str());//*logptr(contextptr) << s << '\n'; //
      }
    }
    closedir (dp);
    return dp;
  }
  
  void nspire_clear_data(GIAC_CONTEXT){
    int maxkeys=32;
    BYTE hash[maxkeys][SHA256_BLOCK_SIZE]={
    };
    int nkeys=rsa_check("/exammode/usr/ndless/shakeys.tns",maxkeys,hash);
    if (nkeys<=0)
      nkeys=rsa_check("/documents/ndless/shakeys.tns",maxkeys,hash);
    if (lang==1)
      *logptr(contextptr) << "Il y a " << nkeys << " empreintes cryptees de fichiers autorises\n";
    else
      *logptr(contextptr) << "Found " << nkeys << " valid crypted keys of secure files\n";
    for (int i=0;i<nkeys;++i){
      *logptr(contextptr) << "{";
      for (int j=0;j<SHA256_BLOCK_SIZE;j++){
	*logptr(contextptr) << hash[i][j] <<",";
      }
      *logptr(contextptr) << (lang==1?"Teste et efface les fichiers non autorisess\n":"}\nChecking and clearing non secure files\n");
    }
    nspire_clear_data("/exammode/usr",nkeys,hash,contextptr);
    *logptr(contextptr) << (lang==1?"Fichiers non autorises effaces\nTapez menu menu pour relancer le mode examen\n":"Filesystem checked.\nPress menu menu to restart exam mode\n");
  }
#endif

  // maybe we should use int nl_exec(const char *prgm_path, int argsn, char *args[])
#ifdef NSPIRE_LED
#include "kled.cc"
#else
#ifdef NSPIRE_NEWLIB
  // #include "ptt"
  void set_exam_mode(int i,GIAC_CONTEXT){
    unsigned NSPIRE_RTC_ADDR=0x90090000;
    unsigned t1= * (volatile unsigned *) NSPIRE_RTC_ADDR;
    gen n=tabunsignedchar2gen(rsa_n_tab,sizeof(rsa_n_tab));
    gen key=powmod(longlong(t1),65537,n);
    key.uncoerce();
    // char exec[]="/documents/ndless/ptt.tns";
    char exec[]="/exammode/usr/ndless/ptt.tns";
    char clef[]="/documents/rtc.tns";
    char mode[2]="0";
    char * args[]={clef,mode};
    mode[0] += i;
    FILE * f=fopen(clef,"w");
    mpz_out_str(f,10,*key._ZINTptr);
    fclose(f);
    // main_ptt(1,0);
    int res=nl_exec(exec,2,args);
    //int res=nl_exec(exec,1,0);
    // int res=nl_exec("/documents/ndless/ptt.tns",1,filenames);
    unlink(clef);
    exam_mode=i;
  }
#else
  void set_exam_mode(int i,GIAC_CONTEXT){
    exam_mode=i;
  }
#endif
#endif
  string print_duration(double & duration){
    if (duration<=0)
      return "";
    int s=std::floor(duration+.5);
    int h=s/3600;
    int m=((s+30)%3600)/60;
    char ch[6]="00h00";
    ch[0] += h/10;
    ch[1] += h%10;
    ch[3] += m/10;
    ch[4] += m%10;
    duration=h+m/100.0;
    return ch;
  }
  bool islogo(const gen & g){
    if (g.type!=_VECT || g._VECTptr->empty()) return false;
    if (g.subtype==_LOGO__VECT) return true;
    const vecteur & v=*g._VECTptr;
    if (islogo(v.back()))
      return true;
    for (size_t i=0;i<v.size();++i){
      if (v[i].type==_VECT && v[i].subtype==_LOGO__VECT)
	return true;
    }
    return false;
  }

  const char conf_standard[] = "F1 algb\nsimplify(\nfactor(\npartfrac(\ntcollect(\ntexpand(\nsum(\noo\nproduct(\nF2 calc\n'\ndiff(\nintegrate(\nlimit(\nseries(\nsolve(\ndesolve(\nrsolve(\nF5  2d \nreserved\nF4 menu\nreserved\nF6 reg\nlinear_regression_plot(\nlogarithmic_regression_plot(\nexponential_regression_plot(\npower_regression_plot(\npolynomial_regression_plot(\nsin_regression_plot(\nscatterplot(\nmatrix(\nF< poly\nproot(\npcoeff(\nquo(\nrem(\ngcd(\negcd(\nresultant(\nGF(\nF9 arit\n mod \nirem(\nifactor(\ngcd(\nisprime(\nnextprime(\npowmod(\niegcd(\nF7 lin\nmatrix(\ndet(\nmatpow(\nranm(\nrref(\ntran(\negvl(\negv(\nF= list\nmakelist(\nrange(\nseq(\nlen(\nappend(\nranv(\nsort(\napply(\nF3 plot\nplot(\nplotseq(\nplotlist(\nplotparam(\nplotpolar(\nplotfield(\nhistogram(\nbarplot(\nF; real\nexact(\napprox(\nfloor(\nceil(\nround(\nsign(\nmax(\nmin(\nF> prog\n:\n&\n#\nhexprint(\nbinprint(\nf(x):=\ndebug(\npython(\nF8 cplx\nabs(\narg(\nre(\nim(\nconj(\ncsolve(\ncfactor(\ncpartfrac(\nF: misc\n!\nrand(\nbinomial(\nnormald(\nexponentiald(\n\\\n % \nperiodic_table\n";

  const char python_conf_standard[] = "F1 misc\nprint(\ninput(\n;\n:\n[]\ndef f(x): return \ntime()\nfrom time import *\nF2 math\nfloor(\nceil(\nround(\nmin(\nmax(\nabs(\nsqrt(\nfrom math import *\nF3 c&rand\nrandint(\nrandom()\nchoice(\nfrom random import *\n.real\n.imag\nphase(\nfrom cmath import *;i=1j\nF4 menu\nreserved\nF5  2d\nreserved\nF6 tortue\nforward(\nbackward(\nleft(\nright(\npencolor(\ncircle(\nreset()\nfrom turtle import *\nF7 linalg\nmatrix(\nadd(\nsub(\nmul(\ninv(\nrref(\ntranspose(\nfrom linalg import *;i=1j\nF8 numpy\narray(\nreshape(\narange(\nlinspace(\nsolve(\neig(\ninv(\nfrom numpy import *;i=1j\nF9 arit\npow(\nisprime(\nnextprime(\nifactor(\ngcd(\nlcm(\niegcd(\nfrom arit import *\nF< color\nred\nblue\ngreen\ncyan\nyellow\nmagenta\nblack\nwhite\nF; draw\nclear_screen();\nshow_screen();\nset_pixel(\ndraw_line(\ndraw_rectangle(\n\ndraw_circle(\ndraw_string(\nfrom graphic import *\nF: plot\nclf()\nplot(\ntext(\narrow(\nscatter(\nbar(\nshow()\nfrom matplotl import *\nF= list\nlist(\nrange(\nlen(\nappend(\nzip(\nsorted(\nmap(\nreversed(\nF> prog\n|\n&\n#\nhex(\nbin(\ndebug(\nfrom cas import *\ncaseval(\"\")\n";
  
  int eqws(char * s,bool eval,GIAC_CONTEXT){ // s buffer must be at least 512 char
    gen g,ge;
    int dconsole_save=dconsole_mode;
    int ss=strlen(s);
    for (int i=0;i<ss;++i){
      if (s[i]==char(0x9c))
	s[i]='\n';
    }
    if (ss>=2 && (s[0]=='#' || s[0]=='"' ||
		  (s[0]=='/' && (s[1]=='/' || s[1]=='*'))
		  ))
      return textedit(s,giacmax(512,ss),contextptr);
    dconsole_mode=0;
    if (eval)
      do_run(s,g,ge,contextptr);
    else {
      if (s[0]==0)
	ge=0;
      else
	ge=gen(s,contextptr);
    }
    dconsole_mode=dconsole_save;
    if (is_undef(ge))
      return textedit(s,giacmax(512,ss),contextptr);
    if (ge.type==giac::_SYMB || (ge.type==giac::_VECT && !ge._VECTptr->empty() && !is_numericv(*ge._VECTptr)) ){
      if (islogo(ge)){
	if (displaylogo()==KEY_SHUTDOWN)
	  return KEY_SHUTDOWN;
	return 0;
      }
      if (ispnt(ge)){
	if (displaygraph(ge,contextptr)==KEY_SHUTDOWN)
	  return KEY_SHUTDOWN;
	// aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
	return 0;
      }
      if (ge.is_symb_of_sommet(at_program))
	return textedit(s,giacmax(ss,512),contextptr);
      if (taille(ge,256)>=256)
	return 0;
    }
    int xp=xcas_python_eval;
    xcas_python_eval=0;
    Console_FMenu_Init(contextptr);
    gen tmp=eqw(ge,true,contextptr);
    xcas_python_eval=xp;
    Console_FMenu_Init(contextptr);
    if (is_undef(tmp) || tmp==ge || taille(ge,64)>=64)
      return 0;
    string S(tmp.print(contextptr));
    if (S.size()>=512)
      return 0;
    strcpy(s,S.c_str());
    return 1;
  }

  
#define GIAC_TEXTAREA 1
#if GIAC_TEXTAREA
  textArea * edptr=0;
#ifdef SCROLLBAR
  typedef scrollbar TScrollbar;
#endif

  int get_line_number(const char * msg1,const char * msg2){
    string s;
    int res=inputline(msg1,msg2,s,false);
    if (res==KEY_CTRL_EXIT)
      return 0;
    res=strtol(s.c_str(),0,10);
    return res;
  }

  void warn_python(int mode,bool autochange){
    if (mode==0)
      confirm(autochange?((lang==1)?"Source en syntaxe Xcas detecte.":"Xcas syntax source code detected."):((lang==1)?"Syntaxe Xcas.":"Xcas syntax."),
#ifdef NSPIRE_NEWLIB
	      "enter: ok"
#else
	      "OK: ok"
#endif
	      );
    if (mode==1)
      if (autochange)
	confirm((lang==1)?"Source en syntaxe Python. Passage":"Python syntax source detected. Setting",
#ifdef NSPIRE_NEWLIB
		(lang==1)?"en Python avec ^=**, enter: ok":"Python mode with ^=**, enter:ok"
#else
		(lang==1)?"en Python avec ^=**, OK: ok":"Python mode with ^=**, OK:ok"
#endif
		);
      else
	confirm((lang==1)?"Syntaxe Python avec ^==**, tapez":"Python syntax with ^==**, type",
#ifdef NSPIRE_NEWLIB
		(lang==1)?"python_compat(2) pour xor. enter: ok":"python_compat(2) for xor. enter: ok"
#else
		(lang==1)?"python_compat(2) pour xor. OK: ok":"python_compat(2) for xor. OK: ok"
#endif
		);
    if (mode==2){
      confirm((lang==1)?"Syntaxe Python avec ^==xor":"Python syntax with ^==xor",
#ifdef NSPIRE_NEWLIB
	      (lang==1)?"python_compat(1) pour **. enter: ok":"python_compat(1) for **. enter: ok"
#else
	      (lang==1)?"python_compat(1) pour **. OK: ok":"python_compat(1) for **. OK: ok"
#endif	      
	      );
    }
    if (mode & 4){
      confirm((lang==1)?"Interpreteur MicroPython":"MicroPython interpreter",
#ifdef NSPIRE_NEWLIB
	      (lang==1)?"enter: ok":"enter: ok"
#else
	      (lang==1)?"OK: ok":"OK: ok"
#endif	      
	      );
    }
  }

  int check_do_graph(giac::gen & ge,int do_logo_graph_eqw,GIAC_CONTEXT) {
    if (ge.type==giac::_SYMB || (ge.type==giac::_VECT && !ge._VECTptr->empty() && !is_numericv(*ge._VECTptr)) ){
      if (islogo(ge)){
	if (do_logo_graph_eqw & 4){
	  if (displaylogo()==KEY_SHUTDOWN)
	    return KEY_SHUTDOWN;
	}
	return 0;
      }
      if (ispnt(ge)){
	if (do_logo_graph_eqw & 2){
	  if (displaygraph(ge,contextptr)==KEY_SHUTDOWN)
	    return KEY_SHUTDOWN;
	}
	// aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
	return 0;
      }
      if ( do_logo_graph_eqw % 2 ==0)
	return 0;
      if (taille(ge,256)>=256 || ge.is_symb_of_sommet(at_program))
	return 0; // sizeof(eqwdata)=44
      gen tmp=eqw(ge,false,contextptr);
      if (!is_undef(tmp) && tmp!=ge){
	//dConsolePutChar(147);
	*giac::logptr(contextptr) << ge.print(contextptr) << '\n';
	ge=tmp;
      }
    }
    return 0;
  }

  void process_freeze(){
    if (freezeturtle){
      displaylogo();
      freezeturtle=false;
      return;
    }
    if (giac::freeze){
      giac::freeze=false;
      for (;;){
#ifdef NSPIRE_NEWLIB
	DefineStatusMessage((char*)((lang==1)?"Ecran fige. Taper esc":"Screen freezed. Press esc."), 1, 0, 0);
#else
	DefineStatusMessage((char*)((lang==1)?"Ecran fige. Taper clear":"Screen freezed. Press clear."), 1, 0, 0);
#endif
	DisplayStatusArea();
	int key;
	GetKey(&key);
	if (key==KEY_CTRL_EXIT || key==KEY_CTRL_AC)
	  break;
      }
    }
  }    

  // called from editor, return 
  int check_parse(textArea * text,const std::vector<textElement> & v,int python,GIAC_CONTEXT){
    if (v.empty())
      return 0;
    char status[256];
    for (int i=0;i<sizeof(status);++i)
      status[i]=0;
    int shift=0;
#ifdef MICROPY_LIB
    if (xcas_python_eval==1){
#if 0
      if (text->changed){
	std::string tmp=text->filename;
	tmp += (lang==1)?" a ete modifie!":" was modified!";
	if (confirm(tmp.c_str(),
#ifdef NSPIRE_NEWLIB
		    (lang==1)?"enter: sauvegarder, esc: tant pis":"enter: save, esc: discard changes"
#else
		    (lang==1)?"OK: sauvegarder, Back: tant pis":"OK: save, Back: discard changes"
#endif
		    )==KEY_CTRL_F1){
	  save_script(text->filename.c_str(),merge_area(text->elements));
	  text->changed=false;
	}
      }
      string tmp="from "+remove_extension(text->filename)+" import *"; // os error 2 ??
      micropy_ck_eval(tmp.c_str());
#else
      freezeturtle=false;
#if 1
      string s=merge_area(vector<textElement>(v.begin(),v.end()));
      micropy_ck_eval(s.c_str());
#else
      // newlines do not work correctly unless we cut the input
      for (int i=0;i<=v.size();++i){
	if (i==v.size() || (v[i].s.size() && v[i].s[0]!=' ')){
	  string s=merge_area(vector<textElement>(v.begin()+shift,v.begin()+i));
	  micropy_ck_eval(s.c_str());
	  if (parser_errorline>0){
	    parser_errorline += shift;
	    break;
	  }
	  shift=i;
	}
      }
#endif
#endif
      // should detect syntax errors here and return line number
      if (parser_errorline>0){
	//--parser_errorline; // ?? something strange 
	sprintf(status,(lang==1)?"Erreur ligne %i":"Error line %i",parser_errorline);	
      }
      else {
	process_freeze();
	sprintf(status,"%s",(lang==1)?"Syntaxe correcte":"Parse OK");
      }
      DefineStatusMessage(status,1,0,0);
      return parser_errorline;
    }
#endif
    std::string s=merge_area(v); 
    giac::python_compat(python,contextptr);
    if (python) s="@@"+s; // force Python translation
    freeze=true;
    giac::gen g(s,contextptr);
    freeze=false;
    int lineerr=giac::first_error_line(contextptr);
    if (lineerr){
      std::string tok=giac::error_token_name(contextptr);
      int pos=-1;
      if (lineerr>=1 && lineerr<=v.size()){
	pos=v[lineerr-1].s.find(tok);
	const std::string & err=v[lineerr-1].s;
	if (pos>=err.size())
	  pos=-1;
	if (python){
	  // find 1st token, check if it's def/if/elseif/for/while
	  size_t i=0,j=0;
	  for (;i<err.size();++i){
	    if (err[i]!=' ')
	      break;
	  }
	  std::string firsterr;
	  for (j=i;j<err.size();++j){
	    if (!isalpha(err[j]))
	      break;
	    firsterr += err[j];
	  }
	  // if there is no : at end set pos=-2
	  if (firsterr=="for" || firsterr=="def" || firsterr=="if" || firsterr=="elseif" || firsterr=="while"){
	    for (i=err.size()-1;i>0;--i){
	      if (err[i]!=' ')
		break;
	    }
	    if (err[i]!=':')
	      pos=-2;
	  }
	}
      }
      else {
	lineerr=v.size();
	tok=(lang==1)?"la fin":"end";
	pos=0;
      }
      if (pos>=0)
	sprintf(status,(lang==1)?"Erreur ligne %i a %s":"Error line %i at %s",lineerr,tok.c_str());
      else
	sprintf(status,(lang==1)?"Erreur ligne %i %s":"Error line %i %s",lineerr,(pos==-2?((lang==1)?", : manquant ?":", missing :?"):""));
      DefineStatusMessage(status,1,0,0);
    }
    else {
      set_abort();
      g=protecteval(g,1,contextptr);
      clear_abort();
      giac::ctrl_c=false;
      kbd_interrupted=giac::interrupted=false;
      // define the function
      if (check_do_graph(g,7,contextptr)==KEY_SHUTDOWN)
	return KEY_SHUTDOWN;
      DefineStatusMessage((char *)((lang==1)?"Syntaxe correcte":"Parse OK"),1,0,0);
    }
    DisplayStatusArea();    
    return lineerr;
  }

  void fix_newlines(textArea * edptr){
    edptr->elements[0].newLine=0;
    for (size_t i=1;i<edptr->elements.size();++i){
      edptr->elements[i].newLine=1;
    }
  }

  void fix_mini(textArea * edptr){
    bool minimini=edptr->elements[0].minimini;
    for (size_t i=1;i<edptr->elements.size();++i){
      edptr->elements[i].minimini=minimini;
    }
  }

  int end_do_then(const std::string & s){
    // skip spaces from end
    int l=s.size(),i,i0;
    const char * ptr=s.c_str();
    for (i=l-1;i>0;--i){
      if (ptr[i]!=' '){
	if (ptr[i]==':' || ptr[i]=='{')
	  return 1;
	if (ptr[i]=='}')
	  return -1;
	break;
      }
    }
    if (i>0){
      for (i0=i;i0>=0;--i0){
	if (!isalphanum(ptr[i0]) && ptr[i0]!=';' && ptr[i0]!=':')
	  break;
      }
      if (i>i0+2){
	if (ptr[i]==';')
	  --i;
	if (ptr[i]==':')
	  --i;
      }
      std::string keyw(ptr+i0+1,ptr+i+1);
      const char * ptr=keyw.c_str();
      if (strcmp(ptr,"faire")==0 || strcmp(ptr,"do")==0 || strcmp(ptr,"alors")==0 || strcmp(ptr,"then")==0)
	return 1;
      if (strcmp(ptr,"fsi")==0 || strcmp(ptr,"end")==0 || strcmp(ptr,"fi")==0 || strcmp(ptr,"od")==0 || strcmp(ptr,"ftantque")==0 || strcmp(ptr,"fpour")==0 || strcmp(ptr,"ffonction")==0 || strcmp(ptr,"ffunction")==0)
	return -1;
    }
    return 0;
  }

  void add(textArea *edptr,const std::string & s){
    int r=1;
    for (size_t i=0;i<s.size();++i){
      if (s[i]=='\n' || s[i]==char(0x9c))
	++r;
    }
    edptr->elements.reserve(edptr->elements.size()+r);
    textElement cur;
    cur.lineSpacing=2;
    for (size_t i=0;i<s.size();++i){
      char c=s[i];
      if (c!='\n' && c!=char(0x9c)){
	if (c!=char(0x0d))
	  cur.s += c;
	continue;
      }
      string tmp=string(cur.s.begin(),cur.s.end());
      cur.s.swap(tmp);
      edptr->elements.push_back(cur);
      ++edptr->line;
      cur.s="";
    }
    if (cur.s.size()){
      edptr->elements.push_back(cur);
      ++edptr->line;
    }
    fix_newlines(edptr);
  }

  int find_indentation(const std::string & s){
    size_t indent=0;
    for (;indent<s.size();++indent){
      if (s[indent]!=' ')
	break;
    }
    return indent;
  }

  void add_indented_line(std::vector<textElement> & v,int & textline,int & textpos){
    // add line
    v.insert(v.begin()+textline+1,v[textline]);
    std::string & s=v[textline].s;
    int indent=find_indentation(s);
    if (!s.empty())
      indent += 2*end_do_then(s);
    //cout << indent << s << ":" << endl;
    if (indent<0)
      indent=0;
    v[textline+1].s=std::string(indent,' ')+s.substr(textpos,s.size()-textpos);
    v[textline+1].newLine=1;
    v[textline].s=s.substr(0,textpos);
    ++textline;
    v[textline].nlines=1; // will be recomputed by cursor moves
    textpos=indent;
  }

  void undo(textArea * text){
    if (text->undoelements.empty())
      return;
    giac::swapint(text->line,text->undoline);
    giac::swapint(text->pos,text->undopos);
    giac::swapint(text->clipline,text->undoclipline);
    giac::swapint(text->clippos,text->undoclippos);
    swap(text->elements,text->undoelements);
  }

  void set_undo(textArea * text){
    text->changed=true;
    text->undoelements=text->elements;
    text->undopos=text->pos;
    text->undoline=text->line;
    text->undoclippos=text->clippos;
    text->undoclipline=text->clipline;
  }

  void add_nl(textArea * text,const std::string & ins){
    std::vector<textElement> & v=text->elements;
    std::vector<textElement> w(v.begin()+text->line+1,v.end());
    v.erase(v.begin()+text->line+1,v.end());
    add(text,ins);
    for (size_t i=0;i<w.size();++i)
      v.push_back(w[i]);
    fix_newlines(text);
    text->changed=true;
  }

  void insert(textArea * text,const char * adds,bool indent){
    size_t n=strlen(adds),i=0;
    if (!n)
      return;
    set_undo(text);
    int l=text->line;
    if (l<0 || l>=text->elements.size())
      return; // invalid line number
    std::string & s=text->elements[l].s;
    int ss=int(s.size());
    int & pos=text->pos;
    if (pos>ss)
      pos=ss;
    std::string ins=s.substr(0,pos);
    for (;i<n;++i){
      if (adds[i]=='\n' || adds[i]==0x1e) {
	break;
      }
      else {
	if (adds[i]!=char(0x0d))
	  ins += adds[i];
      }
    }
    if (i==n){ // no newline in inserted string
      s=ins+s.substr(pos,ss-pos);
      pos += n;
      return;
    }
    std::string S(adds+i+1);
    int decal=ss-pos;
    S += s.substr(pos,decal);
    // cout << S << " " << ins << endl;
    s=ins;
    if (indent){
      pos=s.size();
      int debut=0;
      for (i=0;i<S.size();++i){
	if (S[i]=='\n' || S[i]==0x1e){
	  add_indented_line(text->elements,text->line,pos);
	  // cout << S.substr(debut,i-debut) << endl;
	  text->elements[text->line].s += S.substr(debut,i-debut);
	  pos = text->elements[text->line].s.size();
	  debut=i+1;
	}
      }
      //cout << S << " " << debut << " " << i << S.c_str()+debut << endl;
      add_indented_line(text->elements,text->line,pos);
      text->elements[text->line].s += (S.c_str()+debut);
      fix_newlines(text);
    }
    else 
      add_nl(text,S);
    pos = text->elements[text->line].s.size()-decal;
    fix_mini(text);
  }

  std::string merge_area(const std::vector<textElement> & v){
    std::string s;
    size_t l=0;
    for (size_t i=0;i<v.size();++i)
      l += v[i].s.size()+1;
    s.reserve(l);
    for (size_t i=0;i<v.size();++i){
      s += v[i].s;
      s += '\n';
    }
    return s;
  }

  bool isalphanum(char c){
    return isalpha(c) || (c>='0' && c<='9');
  }

  void search_msg(){
#ifdef NSPIRE_NEWLIB
    DefineStatusMessage((char *)((lang==1)?"enter: suivant, DEL: annuler":"enter: next, DEL: cancel"),1,0,0);
#else
    DefineStatusMessage((char *)((lang==1)?"enter: suivant, DEL: annuler":"enter: next, DEL: cancel"),1,0,0);
#endif
    DisplayStatusArea();    	    
  }  


  void show_status(textArea * text,const std::string & search,const std::string & replace){
    if (text->editable && text->clipline>=0)
      DefineStatusMessage((char *)"PAD: select, COPY: copy, DEL: cut",1,0,0);
    else {
      std::string status("edit ");
#ifdef GIAC_SHOWTIME
      int d=(int(millis()/60000) +time_shift) % (24*60); // minutes
      int heure=d/60;
      int minute=d%60;
      status += char('0'+heure/10);
      status += char('0'+(heure%10));
      status += ':';
      status += char('0'+(minute/10));
      status += char('0'+(minute%10));
#endif
      if (text->editable){
#ifndef NSPIRE_NEWLIB
	status += (xthetat?" t":" x");
#endif
	if (text->python & 4)
	  status += " MicroPython ";
	else
	  status += text->python?(text->python==2?" Py ^xor ":" Py ^=** "):" Xcas ";
	status += giac::remove_extension(text->filename.c_str());
	status += text->changed?" * ":" - ";
	status += giac::printint(text->line+1);
	status += '/';
	status += giac::printint(text->elements.size());
      }
      if (search.size()){
#ifdef NSPIRE_NEWLIB
	status += " enter: " + search;
#else
	status += " EXE: " + search;
#endif
	if (replace.size())
	  status += "->"+replace;
      }
      DefineStatusMessage((char *)status.c_str(), 1, 0, 0);
    }
    DisplayStatusArea();    
  }

  bool chk_replace(textArea * text,const std::string & search,const std::string & replace){
    if (replace.size()){
#ifdef NSPIRE_NEWLIB      
      DefineStatusMessage((char *)((lang==1)?"Remplacer? enter: Oui, 8 ou N: Non":"Replace? enter: Yes, 8 or N: No"),1,0,0);
#else
      DefineStatusMessage((char *)((lang==1)?"Remplacer? EXE: Oui, 8 ou N: Non":"Replace? EXE: Yes, 8 or N: No"),1,0,0);
#endif
    }
    else
      search_msg();
    DisplayStatusArea();
    for (;;){
      int key;
      GetKey(&key);
      if (key==KEY_CHAR_MINUS || key==KEY_CHAR_Y || key==KEY_CHAR_9 || key==KEY_CHAR_O || key==KEY_CTRL_EXE || key==KEY_CTRL_OK){
	if (replace.size()){
	  set_undo(text);
	  std::string & s = text->elements[text->line].s;
	  s=s.substr(0,text->pos-search.size())+replace+s.substr(text->pos,s.size()-text->pos);
	  search_msg();
	}
	return true;
      }
      if (key==KEY_CTRL_DEL || (replace.empty() && key==KEY_CTRL_EXIT) || key==KEY_CTRL_LEFT || key==KEY_CTRL_RIGHT || key==KEY_CTRL_UP || key==KEY_CTRL_DOWN){
	show_status(text,search,replace);
	return false;
      }
      if (key==KEY_CHAR_8 || key==KEY_CHAR_N || key==KEY_CTRL_EXIT){
	search_msg();
	return true;
      }
    }
  }

  int check_leave(textArea * text){
    if (nspire_exam_mode==2)
      return 0;
    if (text->editable && text->filename.size()){
      if (text->changed){
	// save or cancel?
	std::string tmp=text->filename;
	if (strcmp(tmp.c_str(),"temp.py")==0){
	  if (confirm((lang==1)?"Les modifications seront perdues":"Changes will be lost",
#ifdef NSPIRE_NEWLIB
		      (lang==1)?"enter: annuler, esc: tant pis":"enter: cancel, esc: confirm"
#else
		      (lang==1)?"OK: annuler, Back: tant pis":"OK: cancel, Back: confirm"
#endif
		      )==KEY_CTRL_F1)
	    return 2;
	  else {
	    return 0;
	  }
	}
	tmp += (lang==1)?" a ete modifie!":" was modified!";
	if (confirm(tmp.c_str(),
#ifdef NSPIRE_NEWLIB
		    (lang==1)?"enter: sauvegarder, esc: tant pis":"enter: save, esc: discard changes"
#else
		    (lang==1)?"OK: sauvegarder, Back: tant pis":"OK: save, Back: discard changes"
#endif
		    )==KEY_CTRL_F1){
	  save_script(text->filename.c_str(),merge_area(text->elements));
	  text->changed=false;
	  return 1;
	}
	return 0;
      }
      return 1;
    }
    return 0;
  }

  void print(int &X,int&Y,const char * buf_,int color,bool revert,bool fake,bool minimini){
    int s=strlen(buf_);
    char buf[s+1];
    strcpy(buf,buf_);
    for (int i=0;i<s;++i){
      char & ch=buf[i];
      if (ch=='\n')
	ch='\\';
    }
    if(minimini) 
      X=PrintMiniMini(X, Y, buf, revert?4:0, color, COLOR_WHITE,fake);
    else
      X=PrintMini(X, Y, buf, revert?4:0, color, COLOR_WHITE, fake);
  }

  void match_print(char * singleword,int delta,int X,int Y,bool match,bool minimini){
    // char buflog[128];sprintf(buflog,"%i %i %s               ",delta,(int)match,singleword);puts(buflog);
    char ch=singleword[delta];
    singleword[delta]=0;
    print(X,Y,singleword,0,false,/* fake*/true,minimini);
    singleword[delta]=ch;
    char buf[4];
    buf[0]=ch;
    buf[1]=0;
    // inverted print: colors are reverted too!
    int color;
    if (minimini)
      color=match?TEXT_COLOR_GREEN:TEXT_COLOR_RED;
    else
      color=match?COLOR_GREEN:COLOR_RED;
    print(X,Y,buf,color,true,/*fake*/false,minimini);
  }

  bool match(textArea * text,int pos,int & line1,int & pos1,int & line2,int & pos2){
    line2=-1;line1=-1;
    int linepos=text->line;
    const std::vector<textElement> & v=text->elements;
    if (linepos<0 || linepos>=v.size()) return false;
    const std::string * s=&v[linepos].s;
    int ss=s->size();
    if (pos<0 || pos>=ss) return false;
    char ch=(*s)[pos];
    int open1=0,open2=0,open3=0,inc=0;
    if (ch=='(' || ch=='['
	|| ch=='{'
	){
      line1=linepos;
      pos1=pos;
      inc=1;
    }
    if (
	ch=='}' ||
	ch==']' || ch==')'
	){
      line2=linepos;
      pos2=pos;
      inc=-1;
    }
    if (!inc) return false;
    bool instring=false;
    for (;;){
      for (;pos>=0 && pos<ss;pos+=inc){
	if ((*s)[pos]=='"' && (pos==0 || (*s)[pos-1]!='\\'))
	  instring=!instring;
	if (instring)
	  continue;
	switch ((*s)[pos]){
	case '(':
	  open1++;
	  break;
	case '[':
	  open2++;
	  break;
	case '{':
	  open3++;
	  break;
	case ')':
	  open1--;
	  break;
	case ']':
	  open2--;
	  break;
	case '}':
	  open3--;
	  break;
	}
	if (open1==0 && open2==0 && open3==0){
	  //char buf[128];sprintf(buf,"%i %i",pos_orig,pos);puts(buf);
	  if (inc>0){
	    line2=linepos; pos2=pos;
	  }
	  else {
	    line1=linepos; pos1=pos;
	  }
	  return true;
	} // end if
      } // end for pos
      linepos+=inc;
      if (linepos<0 || linepos>=v.size())
	return false;
      s=&v[linepos].s;
      ss=s->size();
      pos=inc>0?0:ss-1;
    } // end for linepos
    return false;
  }

  std::string get_selection(textArea * text,bool erase){
    int sel_line1,sel_line2,sel_pos1,sel_pos2;
    int clipline=text->clipline,clippos=text->clippos,textline=text->line,textpos=text->pos;
    if (clipline>=0){
      if (clipline<textline || (clipline==textline && clippos<textpos)){
	sel_line1=clipline;
	sel_line2=textline;
	sel_pos1=clippos;
	sel_pos2=textpos;
      }
      else {
	sel_line1=textline;
	sel_line2=clipline;
	sel_pos1=textpos;
	sel_pos2=clippos;
      }
    }
    std::string s(text->elements[sel_line1].s);
    if (erase){
      set_undo(text);
      text->line=sel_line1;
      text->pos=sel_pos1;
      text->elements[sel_line1].s=s.substr(0,sel_pos1)+text->elements[sel_line2].s.substr(sel_pos2,text->elements[sel_line2].s.size()-sel_pos2);
    }
    if (sel_line1==sel_line2){
      return s.substr(sel_pos1,sel_pos2-sel_pos1);
    }
    s=s.substr(sel_pos1,s.size()-sel_pos1)+'\n';
    int sel_line1_=sel_line1;
    for (sel_line1++;sel_line1<sel_line2;sel_line1++){
      s += text->elements[sel_line1].s;
      s += '\n';
    }
    s += text->elements[sel_line2].s.substr(0,sel_pos2);
    if (erase)
      text->elements.erase(text->elements.begin()+sel_line1_+1,text->elements.begin()+sel_line2+1);
    return s;
  }

  void change_mode(textArea * text,int flag,GIAC_CONTEXT){
    if (bool(text->python)!=bool(flag)){
      text->python=flag;
      python_compat(text->python,contextptr);
      if (text->python & 4)
	xcas_python_eval=1;
      show_status(text,"","");
      warn_python(flag,true);
    }
  }  

  void clearLine(int x, int y, color_t color=_WHITE) {
    // clear text line. x and y are text cursor coordinates
    // this is meant to achieve the same effect as using PrintXY with a line full of spaces (except it doesn't waste strings).
    int width=LCD_WIDTH_PX;
    if(x>1) width = 24*(21-x);
    drawRectangle((x-1)*18, y*24, width, 24, color);
  }

  void mPrintXY(int x, int y, char*msg, int mode, int color) {
    char nmsg[50];
    nmsg[0] = 0x20;
    nmsg[1] = 0x20;
    nmsg[2] = '\0';
    strncat(nmsg, msg, 48);
    PrintXY(x, y, nmsg, mode ,color);
  }

  void drawScreenTitle(char* title, char* subtitle=0) {
    if(title != NULL) mPrintXY(1, 1, title, TEXT_MODE_NORMAL, TEXT_COLOR_BLUE);
    if(subtitle != NULL) mPrintXY(1, 2, subtitle, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
  }

  int find_color(const char * s,GIAC_CONTEXT){
    if (s[0]=='"')
      return 4;
    if (!isalpha(s[0]))
      return 0;
    char buf[256];
    const char * ptr=s;
    for (int i=0;i<255 && (isalphanum(*ptr) || *ptr=='_'); ++i){
      ++ptr;
    }
    strncpy(buf,s,ptr-s);
    buf[ptr-s]=0;
    if (strcmp(buf,"def")==0 || strcmp(buf,"import")==0)
      return 1;
#ifdef MICROPY_LIB
    if (is_python_builtin(buf))
      return 3;
#endif
    //int pos=dichotomic_search(keywords,sizeof(keywords),buf);
    //if (pos>=0) return 1;
    gen g;
    int token=find_or_make_symbol(buf,g,0,false,contextptr);
    //*logptr(contextptr) << s << " " << buf << " " << token << " " << g << endl;
 #ifdef MICROPY_LIB
    if (xcas_python_eval==1){
      micropy_ck_eval("");
      int tok=mp_token(buf);
      if (tok) return tok;
      if (token==T_NUMBER)
	return 2;
      if (token==T_UNARY_OP || token==T_UNARY_OP_38 || token==T_LOGO)
	return 0;
      if (token!=T_SYMBOL)
	return 1;
      return 0;
    }
#endif
   if (token==T_UNARY_OP || token==T_UNARY_OP_38 || token==T_LOGO){
      return 3;
    }
    if (token==T_NUMBER)
      return 2;
    if (token!=T_SYMBOL)
      return 1;
    return 0;
  }

  int strncasecmp_duplicate(const char *s1, const char *s2, size_t n)
  {
    if (n != 0) {
      const unsigned char *us1 = (const unsigned char *)s1;
      const unsigned char *us2 = (const unsigned char *)s2;

      do {
	if (tolower(*us1) != tolower(*us2++))
	  return (tolower(*us1) - tolower(*--us2));
	if (*us1++ == '\0')
	  break;
      } while (--n != 0);
    }
    return (0);
  }
  char *strcasestr_duplicate(const char *s, const char *find)
  {
    char c;

    if ((c = *find++) != 0) {
      c = tolower((unsigned char)c);
      size_t len = strlen(find);
      do {
	char sc;
	do {
	  if ((sc = *s++) == 0)
	    return (NULL);
	} while ((char)tolower((unsigned char)sc) != c);
      } while (strncasecmp(s, find, len) != 0);
      s--;
    }
    return ((char *)s);
  }


  /* copy over the next token from an input string, WITHOUT
     skipping leading blanks. The token is terminated by the
     first appearance of tokchar, or by the end of the source
     string.

     The caller must supply sufficient space in token to
     receive any token, Otherwise tokens will be truncated.

     Returns: a pointer past the terminating tokchar.

     This will happily return an infinity of empty tokens if
     called with src pointing to the end of a string. Tokens
     will never include a copy of tokchar.

     A better name would be "strtkn", except that is reserved
     for the system namespace. Change to that at your risk.

     released to Public Domain, by C.B. Falconer.
     Published 2006-02-20. Attribution appreciated.
     Modified by gbl08ma not to skip blanks at the beginning.
  */

  const unsigned char *toksplit(const unsigned char *src, /* Source of tokens */
				char tokchar, /* token delimiting char */
				unsigned char *token, /* receiver of parsed token */
				int lgh) /* length token can receive */
  /* not including final '\0' */
  {
    if (src) {
      while (*src && (tokchar != *src)) {
	if (lgh) {
	  *token++ = *src;
	  --lgh;
	}
	src++;
      }
      if (*src && (tokchar == *src)) src++;
    }
    *token = '\0';
    return src;
  } /* toksplit */


  void display(textArea * text,int & isFirstDraw,int & totalTextY,int & scroll,int & textY,GIAC_CONTEXT){
#ifdef CURSOR  
    Cursor_SetFlashOff();
#endif
    // waitforvblank(); drawRectangle(text->x, text->y, LCD_WIDTH_PX, LCD_HEIGHT_PX-text->y, COLOR_WHITE);
    bool editable=text->editable;
    int showtitle = !editable && (text->title != NULL);
    std::vector<textElement> & v=text->elements;
    //drawRectangle(text->x, text->y+24, text->width, LCD_HEIGHT_PX-24, COLOR_WHITE);
    // insure cursor is visible
    if (editable && !isFirstDraw){
      int linesbefore=0,cur;
      for (cur=0;cur<text->line;++cur){
	linesbefore += (v[cur].newLine+(v[cur].nlines-1))*(text->lineHeight+v[cur].lineSpacing); //*logptr(contextptr) << cur << "," << v[cur].nlines << " ";
      }
      // line begin Y is at scroll+linesbefore*17, must be positive
      if (linesbefore+scroll<0)
	scroll = -linesbefore;
      linesbefore += (v[cur].newLine+(v[cur].nlines-1))*(text->lineHeight+v[cur].lineSpacing); //*logptr(contextptr) << '\n';
      // after line Y is at scroll+linesbefore*17
      if (linesbefore+scroll>148)
	scroll = 148-linesbefore;
    }
    textY = scroll+(showtitle ? 24 : 0)+text->y; // 24 pixels for title (or not)
    int deltax=0;
    if (editable){
      if (v.size()<10){
	deltax=9;
      }
      else {
	if (v.size()<100)
	  deltax=18;
	else
	  deltax=27;
      }
    }
    int & clipline=text->clipline;
    int & clippos=text->clippos;
    int & textline=text->line;
    int & textpos=text->pos;
    if (textline<0) textline=0;
    if (textline>=text->elements.size())
      textline=text->elements.size()-1;
    if (textpos<0) textpos=0;
    if (textpos>text->elements[textline].s.size())
      textpos=text->elements[textline].s.size();
    //char bufpos[512];  sprintf(bufpos,"%i,%i:%i,%i       ",textpos,textline,text->elements[textline].s.size(),text->elements.size());  puts(bufpos);
    if (clipline>=0){
      if (clipline>=v.size())
	clipline=-1;
      else {
	if (clippos<0)
	  clippos=0;
	if (clippos>=v[clipline].s.size())
	  clippos=v[clipline].s.size()-1;
      }
    }
    int line1,line2,pos1=0,pos2=0;
    if (!match(text,text->pos,line1,pos1,line2,pos2) && line1==-1 && line2==-1)
      match(text,text->pos-1,line1,pos1,line2,pos2);
    //char bufpos[512];  sprintf(bufpos,"%i,%i:%i,%i       ",line1,pos1,line2,pos2);  puts(bufpos);
    bool firstrect=true;
    for (int cur=0;cur < v.size();++cur) {
      const char* src = v[cur].s.c_str();
      if (cur==0){
	int l=v[cur].s.size();
	if (l>=1 && src[0]=='#')
	  change_mode(text,1,contextptr); // text->python=true;
	if (l>=2 && src[0]=='/' && src[1]=='/')
	  change_mode(text,0,contextptr); // text->python=false;
	if (l>=8 && src[0]=='f' && (src[1]=='o' || src[1]=='u') && src[2]=='n' && src[3]=='c' && src[4]=='t' && src[5]=='i' && src[6]=='o' && src[7]=='n')
	  change_mode(text,0,contextptr); // text->python=false;
	if (l>=4 && src[0]=='d' && src[1]=='e' && src[2]=='f' && src[3]==' ')
	  change_mode(text,1,contextptr); // text->python=true;
	//drawRectangle(text->x, text->y, text->width, LCD_HEIGHT_PX-(editable?17:0), COLOR_WHITE);
      }
      if (cur%3==0 && textY>=(showtitle?24:0) && textY<LCD_HEIGHT_PX)
	waitforvblank();
      int textX=text->x,saveY=textY;
      if(v[cur].newLine) {
	textY=textY+text->lineHeight+v[cur].lineSpacing;
      }
      if (!isFirstDraw && clipline==-1){
	// check if we can skip directly to the next line
	int y=textY+(v[cur].nlines-1)*(text->lineHeight+v[cur].lineSpacing);
	if (y<-text->lineHeight){
	  textY=y;
	  continue;
	}
      }
      int dh=18+v[cur].lineSpacing;
      if (textY+dh+(editable?17:0)>LCD_HEIGHT_PX){
	if (isFirstDraw)
	  dh -= textY+dh+(editable?17:0)-LCD_HEIGHT_PX;
	else {
	  textY = saveY;
	  break;
	}
      }
      if (dh>0 && textY>=(showtitle?24:0)){
	if (firstrect){
	  drawRectangle(textX,text->y,LCD_WIDTH_PX,textY-text->y,COLOR_WHITE);
	  firstrect=false;
	}
	drawRectangle(textX, textY, LCD_WIDTH_PX, dh, COLOR_WHITE);
      }
      if (editable && textY>=(showtitle?24:0)){
	char line_s[16];
	sprint_int(line_s,cur+1);
	os_draw_string_small(textX,textY,COLOR_MAGENTA,_WHITE,line_s);
      }
      textX=text->x+deltax;
      int tlen = v[cur].s.size();
      char singleword[tlen+32];
      // char* singleword = (char*)malloc(tlen+1); // because of this, a single text element can't have more bytes than malloc can provide
      if (cur==textline){
	if (textpos<0 || textpos>tlen)
	  textpos=tlen;
	if (tlen==0 && text->editable){ // cursor on empty line
	  drawRectangle(textX,textY,3,16,COLOR_BLACK);
	  text->cursorx=textX; text->cursory=textY;
	}
      }
      bool chksel=false;
      int sel_line1,sel_line2,sel_pos1,sel_pos2;
      if (clipline>=0){
	if (clipline<textline || (clipline==textline && clippos<textpos)){
	  sel_line1=clipline;
	  sel_line2=textline;
	  sel_pos1=clippos;
	  sel_pos2=textpos;
	}
	else {
	  sel_line1=textline;
	  sel_line2=clipline;
	  sel_pos1=textpos;
	  sel_pos2=clippos;
	}
	chksel=(sel_line1<=cur && cur<=sel_line2);
      }
      const char * match1=0; // matching parenthesis (or brackets?)
      const char * match2=0;
      if (cur==line1)
	match1=v[cur].s.c_str()+pos1;
      else
	match1=0;
      if (cur==line2)
	match2=v[cur].s.c_str()+pos2;
      else
	match2=0;
      // if (cur==textline && !match(v[cur].s.c_str(),textpos,match1,match2) && !match1 && !match2) match(v[cur].s.c_str(),textpos-1,match1,match2);
      // char buf[128];sprintf(buf,"%i %i %i        ",cur,(int)match1,(int)match2);puts(buf);
      const char * srcpos=src+textpos;
      bool minimini=v[cur].minimini;
      int couleur=v[cur].color;
      int nlines=1;
      bool linecomment=false;
      while (*src){
	const char * oldsrc=src;
	if ( (text->python && *src=='#') ||
	     (!text->python && *src=='/' && *(src+1)=='/')){
	  linecomment=true;
	  couleur=4;
	}
	if (linecomment || !text->editable)
	  src = (char*)toksplit((unsigned char*)src, ' ', (unsigned char*)singleword, minimini?50:35); //break into words; next word
	else { // skip string (only with delimiters " ")
	  if (*src=='"'){
	    for (++src;*src;++src){
	      if (*src=='"' && *(src-1)!='\\')
		break;
	    }
	    if (*src=='"')
	      ++src;
	    int i=src-oldsrc;
	    strncpy(singleword,oldsrc,i);
	    singleword[i]=0;
	  }
	  else {
	    size_t i=0;
	    for (;*src==' ';++src){ // skip initial whitespaces
	      ++i;
	    }
	    if (i==0){
	      if (isalpha(*src)){ // skip keyword
		for (;isalphanum(*src) || *src=='_';++src){
		  ++i;
		}
	      }
	      // go to next space or alphabetic char
	      for (;*src;++i,++src){
		if (*src==' ' || (i && *src>=' ' && *src<='/') || (text->python && *src=='#') || (!text->python && *src=='/' && *(src+1)=='/')|| *src=='"' || isalpha(*src))
		  break;
	      }
	    }
	    strncpy(singleword,oldsrc,i);
	    singleword[i]=0;
	    if (i==0){
	      puts(src); // free(singleword);
	      return ; // FIXME KEY_CTRL_F2;
	    }
	  } // end normal case
	} // end else linecomment case
	// take care of selection
	bool invert=false;
	if (chksel){
	  if (cur<sel_line1 || cur>sel_line2)
	    invert=false;
	  else {
	    int printpos1=oldsrc-v[cur].s.c_str();
	    int printpos2=src-v[cur].s.c_str();
	    if (cur==sel_line1 && printpos1<sel_pos1 && printpos2>sel_pos1){
	      // cut word in 2 parts: first part not selected
	      src=oldsrc+sel_pos1-printpos1;
	      singleword[sel_pos1-printpos1]=0;
	      printpos2=sel_pos1;
	    }
	    if (cur==sel_line2 && printpos1<sel_pos2 && printpos2>sel_pos2){
	      src=oldsrc+sel_pos2-printpos1;
	      singleword[sel_pos2-printpos1]=0;
	      printpos2=sel_pos2;
	    }
	    // now singleword is totally unselected or totally selected
	    // which one?
	    if (cur==sel_line1){
	      if (cur==sel_line2)
		invert=printpos1>=sel_pos1 && printpos2<=sel_pos2;
	      else
		invert=printpos1>=sel_pos1;
	    }
	    else {
	      if (cur==sel_line2)
		invert=printpos2<=sel_pos2;
	      else
		invert=true;
	    }
	  }
	}
	//check if printing this word would go off the screen, with fake PrintMini drawing:
	int temptextX = 0,temptextY=0;
	print(temptextX,temptextY,singleword,couleur,false,/*fake*/true,minimini);
	if(temptextX<text->width && temptextX + textX > text->width-6) {
	  if (editable)
	    textX=PrintMini(textX, textY, ">", 4, COLOR_MAGENTA, COLOR_WHITE);	  
	  //time for a new line
	  textX=text->x+deltax;
	  textY=textY+text->lineHeight+v[cur].lineSpacing;
	  if (textY>=(showtitle?24:0)){
	    if (firstrect){
	      drawRectangle(textX,text->y,LCD_WIDTH_PX,textY-text->y,COLOR_WHITE);
	      firstrect=false;
	    }
	    drawRectangle(text->x, textY, LCD_WIDTH_PX, 18+v[cur].lineSpacing, COLOR_WHITE);
	  }
	  ++nlines;
	} //else still fits, print new word normally (or just increment textX, if we are not "on stage" yet)
	if(textY >= (showtitle?24:0) && textY < LCD_HEIGHT_PX) {
	  temptextX=textX;
	  if (editable){
	    couleur=linecomment?5:find_color(singleword,contextptr);
	    if (couleur==1) couleur=COLOR_BLUE;
	    if (couleur==2) couleur=COLOR_YELLOWDARK;
	    if (couleur==3) couleur=51712;//33024;
	    if (couleur==4) couleur=COLOR_MAGENTA;
	    if (couleur==5) couleur=COLOR_GREEN;
	    //char ch[32];
	    //sprint_int(ch,couleur);
	    //puts(singleword); puts(ch);
	  }
	  if (linecomment || !text->editable || singleword[0]=='"')
	    print(textX,textY,singleword,couleur,invert,/*fake*/false,minimini);
	  else { // print two parts, commandname in color and remain in black
	    char * ptr=singleword;
	    if (isalpha(*ptr)){
	      while (isalphanum(*ptr) || *ptr=='_')
		++ptr;
	    }
	    char ch=*ptr;
	    *ptr=0;
	    print(textX,textY,singleword,couleur,invert,/*fake*/false,minimini);
	    *ptr=ch;
	    print(textX,textY,ptr,COLOR_BLACK,invert,/*fake*/false,minimini);
	  }
	  // ?add a space removed from token
	  if( ((linecomment || !text->editable)?*src:*src==' ') || v[cur].spaceAtEnd){
	    if (*src==' ')
	      ++src;
	    print(textX,textY," ",COLOR_BLACK,invert,false,minimini);
	  }
	  // ?print cursor, and par. matching
	  if (editable){
	    if (match1 && oldsrc<=match1 && match1<src)
	      match_print(singleword,match1-oldsrc,temptextX,textY,
			  line2!=-1,
			  // match2,
			  minimini);
	    if (match2 && oldsrc<=match2 && match2<src)
	      match_print(singleword,match2-oldsrc,temptextX,textY,
			  line1!=-1,
			  //match1,
			  minimini);
	  }
	  if (editable && cur==textline){
	    if (oldsrc<=srcpos && (srcpos<src || (srcpos==src && textpos==tlen))){
	      if (textpos>=2 && v[cur].s[textpos-1]==' ' && v[cur].s[textpos-2]!=' ' && srcpos-oldsrc==strlen(singleword)+1){ // fix cursor position after space
		//char ch[512];
		//sprintf(ch,"%s %i %i %i %i",singleword,strlen(singleword),srcpos-oldsrc,textpos,v[cur].s[textpos-2]);
		//puts(ch);
		singleword[srcpos-oldsrc-1]=' ';
	      }
	      singleword[srcpos-oldsrc]=0;
	      print(temptextX,temptextY,singleword,couleur,false,/*fake*/true,minimini);
	      //drawLine(temptextX, textY+14, temptextX, textY-14, COLOR_BLACK);
	      //drawLine(temptextX+1, textY+14, temptextX+1, textY-14, COLOR_BLACK);
	      drawRectangle(temptextX-1,textY,3,16,COLOR_BLACK);
	      text->cursorx=temptextX-1; text->cursory=textY;
	    }
	  }
	} // end if testY visible
	else {
	  textX += temptextX;
	  if(*src || v[cur].spaceAtEnd) textX += 7; // size of a PrintMini space
	}
      } // end while (*src)
      // free(singleword);
      v[cur].nlines=nlines; //if (cur<6) *logptr(contextptr) << cur << ":" << src << nlines << '\n';
      if (isFirstDraw) 
	totalTextY = textY+(showtitle ? 0 : 24);
    } // end main draw loop (for cur<v.size())
    int dh=LCD_HEIGHT_PX-textY-text->lineHeight-(editable?17:0);
    if (dh>0)
      drawRectangle(0, textY+text->lineHeight, LCD_WIDTH_PX, dh, COLOR_WHITE);
    isFirstDraw=0;
    if(showtitle) {
      waitforvblank();
      drawRectangle(0, 0, LCD_WIDTH_PX, 24, _WHITE);
      drawScreenTitle((char*)text->title);
    }
    //if (editable)
    if (editable){
      waitforvblank();
      drawRectangle(0,205,LCD_WIDTH_PX,17,44444);
      PrintMiniMini(0,205,text->python?"shift-1 test|2 loop|3 undo|4 misc|5 +-|6 logo|7 lin|8 list|9arit":"shift-1 test|2 loop|3 undo|4 misc|5 +-|6 logo|7 matr|8 cplx",4,44444,giac::_BLACK);
      //draw_menu(1);
    }
#ifdef SCROLLBAR
    int scrollableHeight = LCD_HEIGHT_PX-24*(showtitle ? 2 : 1)-text->y;
    //draw a scrollbar:
    if(text->scrollbar) {
      TScrollbar sb;
      sb.I1 = 0;
      sb.I5 = 0;
      sb.indicatormaximum = totalTextY;
      sb.indicatorheight = scrollableHeight;
      sb.indicatorpos = -scroll;
      sb.barheight = scrollableHeight;
      sb.bartop = (showtitle ? 24 : 0)+text->y;
      sb.barleft = text->width - 6;
      sb.barwidth = 6;
    
      Scrollbar(&sb);
    }
#endif
  }  

  bool move_to_word(textArea * text,const std::string & s,const std::string & replace,int & isFirstDraw,int & totalTextY,int & scroll,int & textY,GIAC_CONTEXT){
    if (!s.size())
      return false;
    int line=text->line,pos=text->pos;
    if (line>=text->elements.size())
      line=0;
    if (pos>=text->elements[line].s.size())
      pos=0;
    for (;line<text->elements.size();++line){
      int p=text->elements[line].s.find(s,pos);
      if (p>=0 && p<text->elements[line].s.size()){
	text->line=line;
	text->clipline=line;
	text->clippos=p;
	text->pos=p+s.size();
	display(text,isFirstDraw,totalTextY,scroll,textY,contextptr); // this modifies text->elements[].nlines (no idea why), 2 calls insure scrolling is adequate
	display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	text->clipline=-1;
	return chk_replace(text,s,replace);
      }
      pos=0;
    }
    for (line=0;line<text->line;++line){
      int p=text->elements[line].s.find(s,0);
      if (p>=0 && p<text->elements[line].s.size()){
	text->line=line;
	text->clipline=line;
	text->clippos=p;
	text->pos=p+s.size();
	display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	display(text,isFirstDraw,totalTextY,scroll,textY,contextptr); // 2 callslike above
	text->clipline=-1;
	return chk_replace(text,s,replace);
      }
    }
    return false;
  }

  int load_script(const char * filename,std::string & s){
    const char * ch =read_file(filename);
    s=ch?ch:"";
    return 1;
  }

  void save_script(const char * filename,const string & s){
    if (nspire_exam_mode==2)
      return;
#ifdef NUMWORKS
    char buf[s.size()+2];
    buf[0]=1;
    strcpy(buf+1,s.c_str());
#else
    char buf[s.size()+1];
    strcpy(buf,s.c_str());    
#endif
#ifdef NSPIRE_NEWLIB
    char filenametns[strlen(filename)+5];
    strcpy(filenametns,filename);
    int l=strlen(filenametns);
    if (l<4 || strncmp(filename+l-4,".tns",4))
      strcpy(filenametns+strlen(filename),".tns");
    write_file(filenametns,buf);
#else
    write_file(filename,buf);
#endif
  }

  bool textedit(char * s,int bufsize,bool OKparse,const giac::context * contextptr,const char * filename){
    if (!s)
      return false;
    int ss=strlen(s);
    if (ss==0){
      *s=' ';
      s[1]=0;
      ss=1;
    }
    textArea ta;
    ta.elements.clear();
    ta.editable=true;
    ta.clipline=-1;
    ta.changed=false;
    ta.filename=filename?filename:"temp.py";
    ta.y=0;
    ta.python=true;
    ta.allowEXE=false;//true; // set back to true later
    ta.OKparse=OKparse;
    bool str=s[0]=='"' && s[ss-1]=='"';
    if (str){
      s[ss-1]=0;
      add(&ta,s+1);
    }
    else
      add(&ta,s);
    ta.line=0;
    ta.pos=ta.elements[ta.line].s.size();
    int res=doTextArea(&ta,contextptr);
    drawRectangle(0,0,LCD_WIDTH_PX,LCD_HEIGHT_PX,_WHITE);
    os_hide_graph();
    if (res==TEXTAREA_RETURN_EXIT)
      return false;
    string S(merge_area(ta.elements));
    if (str)
      S='"'+S+'"';
    int Ssize=S.size();
    if (Ssize<bufsize){
      strcpy(s,S.c_str());
      for (--Ssize;Ssize>=0;--Ssize){
	if ((unsigned char)s[Ssize]==0x9c || s[Ssize]=='\n')
	  s[Ssize]=0;
	if (s[Ssize]!=' ')
	  break;
      }
      return true;
    }
    return false;
  }

  bool textedit(char * s,int bufsize,const giac::context * contextptr){
    return textedit(s,bufsize,false,contextptr);
  }

#if 0
  int get_filename(char * filename,const char * extension){
    return 0;
  }
#else
  int get_filename(char * filename,const char * extension){
    handle_f5();
    string str;
#ifdef NSPIRE_NEWLIB
    int res=inputline((lang==1)?"esc ou chaine vide: annulation":"esc or empty string: cancel",(lang==1)?"Nom de fichier:":"Filename:",str,false);
#else
    int res=inputline((lang==1)?"EXIT ou chaine vide: annulation":"EXIT or empty string: cancel",(lang==1)?"Nom de fichier:":"Filename:",str,false);
#endif
    if (res==KEY_CTRL_EXIT || str.empty())
      return 0;
    strcpy(filename,str.c_str());
    int s=strlen(filename);
    if (strcmp(filename+s-3,extension))
      strcpy(filename+s,extension);
    // if file already exists, warn, otherwise create
    if (!file_exists(filename))
      return 1;
    if (confirm((lang==1)?"  Le fichier existe!":"  File exists!",
#ifdef NSPIRE_NEWLIB
		(lang==1)?"enter: ecraser, esc: annuler":"enter:overwrite, esc: cancel"
#else
		(lang==1)?"OK: ecraser,Back: annuler":"OK:overwrite, Back: cancel"
#endif
		)==KEY_CTRL_F1)
      return 1;
    return 0;
  }
#endif

  const char * input_matrix(const gen &g,gen & ge,GIAC_CONTEXT){
#ifdef MICROPY_LIB
    if (xcas_python_eval==1){
      if (ge.type==_VECT)
	ge.subtype=0;
      static string input_matrix_s=g.print(contextptr)+'='+ge.print(contextptr);
      return input_matrix_s.c_str();
    }
#endif
    if (ge.type==giac::_VECT)
      sto(ge,g,contextptr);
    return "";
  }    
  
  const char * input_matrix(bool list,GIAC_CONTEXT){
    static std::string * sptr=0;
    if (!sptr)
      sptr=new std::string;
    *sptr="";
    giac::gen v(giac::_VARS(0,contextptr));
    giac::vecteur w;
    if (v.type==giac::_VECT){
      for (size_t i=0;i<v._VECTptr->size();++i){
	giac::gen & tmp = (*v._VECTptr)[i];
	if (tmp.type==giac::_IDNT){
	  giac::gen tmpe(protecteval(tmp,1,contextptr));
	  if (list){
	    if (tmpe.type==giac::_VECT && !ckmatrix(tmpe))
	      w.push_back(tmp);
	  }
	  else {
	    if (ckmatrix(tmpe))
	      w.push_back(tmp);
	  }
	}
      }
    }
    std::string msg;
    if (w.empty())
      msg=(lang==1)?(list?"Creer nouvelle liste":"Creer nouvelle matrice"):(list?"Create new list":"Create new matrix");
    else
      msg=(((lang==1)?"Creer nouveau ou editer ":"Create new or edit ")+(w.size()==1?w.front():giac::gen(w,giac::_SEQ__VECT)).print(contextptr));
    handle_f5();
    if (inputline(msg.c_str(),((lang==1)?"Nom de variable:":"Variable name:"),*sptr,false) && !sptr->empty() && isalpha((*sptr)[0])){
      giac::gen g(*sptr,contextptr);
      giac::gen ge(protecteval(g,1,contextptr));
      if (g.type==giac::_IDNT){
	if (ge.type==giac::_VECT){
	  ge=eqw(ge,true,contextptr);
	  ge=protecteval(ge,1,contextptr);
	  return input_matrix(g,ge,contextptr);
	  if (ge.type==giac::_VECT)
	    sto(ge,g,contextptr);
	  else
	    cout << "edited " << ge << endl;
	  return ""; // return sptr->c_str();
	}
	if (ge==g || confirm_overwrite()){
	  *sptr="";
	  if (inputline(((lang==1)?(list?"Nombre d'elements":"Nombre de lignes"):(list?"Elements number":"Line number")),"",*sptr,true)){
	    int l=strtol(sptr->c_str(),0,10);
	    if (l>0 && l<256){
	      int c;
	      if (list)
		c=0;
	      else {
		std::string tmp(*sptr+((lang==1)?" lignes.":" lines."));
		*sptr="";
		inputline(tmp.c_str(),(lang==1)?"Colonnes:":"Columns:",*sptr,true);
		c=strtol(sptr->c_str(),0,10);
	      }
	      if (c==0){
		ge=giac::vecteur(l);
	      }
	      else {
		if (c>0 && l*c<256)
		  ge=giac::_matrix(giac::makesequence(l,c),contextptr);
	      }
	      ge=eqw(ge,true,contextptr);
	      ge=protecteval(ge,1,contextptr);
	      return input_matrix(g,ge,contextptr);
	    } // l<256
	  }
	} // ge==g || overwrite confirmed
      } // g.type==_IDNT
      else {
	invalid_varname();
      }	
    } // isalpha
    return 0;
  }

  std::string get_searchitem(std::string & replace){
    replace="";
    std::string search;
    handle_f5();
#ifdef NSPIRE_NEWLIB
    int res=inputline((lang==1)?"esc ou chaine vide: annulation":"esc or empty string: cancel",(lang==1)?"Chercher:":"Search:",search,false);
    if (search.empty() || res==KEY_CTRL_EXIT)
      return "";
    replace="";
    std::string tmp=((lang==1)?"esc: recherche seule de ":"esc: search only ")+search;
#else
    int res=inputline((lang==1)?"EXIT ou chaine vide: annulation":"EXIT or empty string: cancel",(lang==1)?"Chercher:":"Search:",search,false);
    if (search.empty() || res==KEY_CTRL_EXIT)
      return "";
    replace="";
    std::string tmp=((lang==1)?"EXIT: recherche seule de ":"EXIT: search only ")+search;
#endif
    handle_f5();
    res=inputline(tmp.c_str(),(lang==1)?"Remplacer par:":"Replace by:",replace,false);
    if (res==KEY_CTRL_EXIT)
      replace="";
    return search;
  }

  bool tooltip(int x,int y,int pos,const char * editline,GIAC_CONTEXT){
    char cmdline[strlen(editline)+1];
    strcpy(cmdline,editline);
    cmdline[pos]=0;
    int l=strlen(cmdline);
    char buf[l+1];
    strcpy(buf,cmdline);
    bool openpar=l && buf[l-1]=='(';
    if (openpar){
      buf[l-1]=0;
      --l;
    }
    for (;l>0;--l){
      if (!isalphanum(buf[l-1]) && buf[l-1]!='_')
	break;
    }
    // cmdname in buf+l
    const char * cmdname=buf+l,*cmdnameorig=cmdname;
    int l1=strlen(cmdname);
    if (l1<2)
      return false;
    const char * howto=0,*syntax=0,*related=0,*examples=0;
    if (l1>0 && has_static_help(cmdname,lang | 0x100,howto,syntax,related,examples) && examples){
      // display tooltip
      if (x<0)
	x=os_draw_string(0,y,_BLACK,1234,editline,true); // fake print -> x position // replaced cmdline by editline so that tooltip is at end
      x+=2;
      y+=4;
      drawRectangle(x,y,6,10,65529);
      draw_line(x,y,x+6,y,_BLACK);
      draw_line(x,y,x+3,y+3,_BLACK);
      draw_line(x+6,y,x+3,y+3,_BLACK);
      y-=4;
      x+=7;
      int bg=65529; // background
      x=os_draw_string_small(x,y,_BLACK,bg,": ",false);
      if (howto && strlen(howto)){
#ifdef NSPIRE_NEWLIB
	y-=2;
#endif
	os_draw_string_small(x,y,_BLACK,bg,
#ifdef NUMWORKS
			     remove_accents(howto).c_str(),
#else
			     howto,
#endif
			     false);
#ifdef NSPIRE_NEWLIB
	y+=12;
#else
	y+=11;
#endif
      }
      string toolt;
      if (related && strlen(related)){
	toolt += cmdname;
	toolt += '(';
	if (syntax && strlen(syntax))
	  toolt += syntax;
	else
	  toolt += "arg";
	toolt += ')';
	toolt += ' ';
	if (related)
	  toolt += related;
      }
      else
	toolt+=examples;
      os_draw_string_small(x,y,_BLACK,bg,toolt.c_str(),false);
      return true;
    }
    return false;
  }

  void textarea_help_insert(textArea * text,int exec,GIAC_CONTEXT){
    string curs=text->elements[text->line].s.substr(0,text->pos);
    if (!curs.empty()){
      int b;
      string adds=help_insert(curs.c_str(),b,exec,contextptr);
      if (!adds.empty()){
	if (b>0){
	  std::string & s=text->elements[text->line].s;
	  if (b>text->pos)
	    b=text->pos;
	  if (b>s.size())
	    b=s.size();
	  s=s.substr(0,text->pos-b)+s.substr(text->pos,s.size()-text->pos);//+s.substr(b,s.size()-b);
	}
	insert(text,adds.c_str(),false);
      }
    }
  }
  
  int doTextArea(textArea* text,GIAC_CONTEXT) {
    int scroll = 0;
    int isFirstDraw = 1;
    int totalTextY = 0,textY=0;
    bool editable=text->editable;
    int showtitle = !editable && (text->title != NULL);
    int scrollableHeight = LCD_HEIGHT_PX-24*(showtitle ? 2 : 1)-text->y;
    std::vector<textElement> & v=text->elements;
    std::string search,replace;
    show_status(text,search,replace);
    if (text->line>=v.size())
      text->line=0;
    display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
    bool keytooltip=false;
    while(1) {
      if (text->line>=v.size())
	text->line=0;
      if (!keytooltip)
	display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
      if(text->type == TEXTAREATYPE_INSTANT_RETURN) return 0;
      int keyflag = GetSetupSetting( (unsigned int)0x14);
      int key;
      GetKey(&key);
      if (keytooltip){
	keytooltip=false;
	if (key==KEY_CTRL_RIGHT && text->pos==text->elements[text->line].s.size())
	  key=KEY_CTRL_OK;
	if (key==KEY_CTRL_EXIT)
	  continue;
	if (key==KEY_CTRL_DOWN || key==KEY_CTRL_VARS)
	  key=KEY_BOOK;
	if (key==KEY_CTRL_OK ){
	  textarea_help_insert(text,key,contextptr);
	  continue;
	}
      }
      if (key==KEY_SHUTDOWN)
	return key;
      if (key==KEY_CTRL_F3) // Numworks has no UNDO key
	key=KEY_CTRL_UNDO;
#if 1
      if (key == KEY_CTRL_SETUP) {
	menu_setup(contextptr);
	continue;
      }
#endif
      if (key!=KEY_CTRL_PRGM && key!=KEY_CHAR_FRAC)
	translate_fkey(key);    
      //char keylog[32];sprint_int(keylog,key); puts(keylog);
      show_status(text,search,replace);
      int & clipline=text->clipline;
      int & clippos=text->clippos;
      int & textline=text->line;
      int & textpos=text->pos;
      if (key==KEY_CTRL_CUT && clipline<0) // if no selection, CUT -> pixel menu
	key=KEY_CTRL_F3;
      if (!editable && (key==KEY_CHAR_ANS || key==KEY_BOOK || key=='\t' || key==KEY_CTRL_EXE))
	return key;
      if (editable){
	if (key=='\t'){
	  int indent=0; // indent deduced from prev line
	  if (textline!=0){
	    std::string & s=v[textline-1].s;
	    indent=find_indentation(s);
	    if (!s.empty())
	      indent+=2*end_do_then(s);
	  }
	  std::string & s=v[textline].s;
	  int curindent=find_indentation(s);
	  int diff=curindent-indent;
	  if (diff>0){
	    s=s.substr(diff,s.size()-diff);
	    if (textpos>diff)
	      textpos -= diff;
	    else
	      textpos = 0;
	    continue;
	  }
	  if (diff<0){
	    s=string(-diff,' ')+s;
	    textpos += -diff;
	    continue;
	  }
	  key=KEY_BOOK;
	}
	if (key==KEY_BOOK){
	  textarea_help_insert(text,0,contextptr);
	  continue;
	}
	if (key==KEY_CHAR_FRAC && clipline<0){
	  if (textline==0) continue;
	  std::string & s=v[textline].s;
	  std::string & prev_s=v[textline-1].s;
	  int indent=find_indentation(s),prev_indent=find_indentation(prev_s);
	  if (!prev_s.empty())
	    prev_indent += 2*end_do_then(prev_s);
	  int diff=indent-prev_indent; 
	  if (diff>0 && diff<=s.size())
	    s=s.substr(diff,s.size()-diff);
	  if (diff<0)
	    s=string(-diff,' ')+s;
	  textpos -= diff;
	  continue;
	}
	if (key==KEY_CHAR_ANS){
	  displaylogo();
	  continue;
	}
	if (key>=KEY_SELECT_LEFT && key<=KEY_SELECT_RIGHT){
	  if (clipline<0){
	    clipline=textline;
	    clippos=textpos;
	    show_status(text,search,replace);
	  }
	  if (key==KEY_SELECT_LEFT){
	    if (textpos)
	      --textpos;
	    else {
	      if (textline){
		--textline;
		textpos=v[textline].s.size();
	      }
	    }
	  }
	  if (key==KEY_SELECT_RIGHT){
	    if (textpos<v[textline].s.size())
	      ++textpos;
	    else {
	      if (textline<v.size()){
		++textline;
		textpos=0;
	      }
	    }
	  }
	  if (key==KEY_SELECT_UP){
	    if (textline){
	      --textline;
	      textpos=giacmin(textpos,v[textline].s.size());
	    }
	  }
	  if (key==KEY_SELECT_DOWN){
	    if (textline<v.size()){
	      ++textline;
	      textpos=giacmin(textpos,v[textline].s.size());
	    }
	  }
	}
	if (key==KEY_CTRL_CLIP) {
#if 1
	  if (clipline>=0){
	    copy_clipboard(get_selection(text,false),true);
	    clipline=-1;
	  }
	  else {
	    clipline=textline;
	    clippos=textpos;
	    show_status(text,search,replace);
	  }
#else
	  copy_clipboard(v[textline].s,false);
	  DefineStatusMessage((char*)"Line copied to clipboard", 1, 0, 0);
	  DisplayStatusArea();
#endif
	  continue;
	}
	if (key==KEY_CTRL_F5){
	  bool minimini=!v[0].minimini;
	  for (int i=0;i<v.size();++i)
	    v[i].minimini=minimini;
	  text->lineHeight=minimini?13:17;
	  isFirstDraw=1;
	  display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	  continue;
	}
	if (clipline<0){
	  const char * adds;
#if 1
	  if ( (key>=KEY_CTRL_F1 && key<=KEY_CTRL_F4) ||
	       (key >= KEY_CTRL_F6 && key <= KEY_CTRL_F14)
	       ){
	    string le_menu=xcas_python_eval?//text->python?
	      "F1 test\nif \nelse \n<\n>\n==\n!=\n&&\n||\nF2 loop\nfor \nfor in\nrange(\nwhile \nbreak\ndef\nreturn \n#\nF4 misc\n:\n;\n_\n!\n%\nfrom  import *\nprint(\ninput(\nF6 tortue\nforward(\nbackward(\nleft(\nright(\npencolor(\ncircle(\nreset()\nfrom turtle import *\nF: plot\nplot(\ntext(\narrow(\nlinear_regression_plot(\nscatter(\naxis(\nbar(\nfrom matplotl import *\nF7 linalg\nadd(\nsub(\nmul(\ninv(\ndet(\nrref(\ntranspose(\nfrom linalg import *\nF< color\nred\nblue\ngreen\ncyan\nyellow\nmagenta\nblack\nwhite\nF; draw\nset_pixel(\ndraw_line(\ndraw_rectangle(\nfill_rect(\ndraw_polygon(\ndraw_circle(\ndraw_string(\nfrom graphic import *\nF8 numpy\narray(\nreshape(\narange(\nlinspace(\nsolve(\neig(\ninv(\nfrom numpy import *\nF9 arit\npow(\nisprime(\nnextprime(\nifactor(\ngcd(\nlcm(\niegcd(\nfrom arit import *\n":
	      "F1 test\nif \nelse \n<\n>\n==\n!=\nand\nor\nF2 loop\nfor \nfor in\nrange(\nwhile \nbreak\nf(x):=\nreturn \nlocal\nF4 misc\n;\n:\n_\n!\n%\n&\nprint(\ninput(\nF6 tortue\navance\nrecule\ntourne_gauche\ntourne_droite\nrond\ndisque\nrepete\nefface\nF7 lin\nmatrix(\ndet(\nmatpow(\nranm(\nrref(\ntran(\negvl(\negv(\nF9 arit\n mod \nirem(\nifactor(\ngcd(\nisprime(\nnextprime(\npowmod(\niegcd(\nF< plot\nplot(\nplotseq(\nplotlist(\nplotparam(\nplotpolar(\nplotfield(\nhistogram(\nbarplot(\nF: misc\n<\n>\n_\n!\n % \nrand(\nbinomial(\nnormald(\nF8 cplx\nabs(\narg(\nre(\nim(\nconj(\ncsolve(\ncfactor(\ncpartfrac(\n";
	    le_menu += "F= list\nmakelist(\nrange(\nseq(\nlen(\nappend(\nranv(\nsort(\napply(\nF; real\nexact(\napprox(\nfloor(\nceil(\nround(\nsign(\nmax(\nmin(\nF> prog\n;\n:\n\\\n&\n?\n!\ndebug(\npython(\n";
	    const char * ptr=console_menu(key,(char*)(le_menu.c_str()),2);
	    if (!ptr){
	      show_status(text,search,replace);
	      continue;
	    }
	    adds=ptr;
	  }
	  else
#endif
	    adds=keytostring(key,keyflag,text->python,contextptr);
	  if (key!=KEY_CHAR_ANS && adds){
	    bool isex=adds[0]=='\n';
	    if (isex)
	      ++adds;
	    bool isif=strcmp(adds,"if ")==0,
	      iselse=strcmp(adds,"else ")==0,
	      isfor=strcmp(adds,"for ")==0,
	      isforin=strcmp(adds,"for in")==0,
	      isdef=strcmp(adds,"f(x):=")==0 || strcmp(adds,"def")==0,
	      iswhile=strcmp(adds,"while ")==0,
	      islist=strcmp(adds,"list ")==0,
	      ismat=strcmp(adds,"matrix ")==0;
	    if (islist){
	      input_matrix(true,contextptr);
	      continue;
	    }
	    if (ismat){
	      input_matrix(false,contextptr);
	      continue;
	    }
	    if (text->python){
	      if (isif)
		adds=isex?"if x<0:\nx=-x":"if :\n";
	      if (iselse)
		adds="else:\n";
	      if (isfor)
		adds=isex?"for j in range(10):\nprint(j*j)":"for  in range():\n";
	      if (isforin)
		adds=isex?"for j in [1,4,9,16]:\nprint(j)":"for  in :\n";
	      if (iswhile && isex)
		adds="a,b=25,15\nwhile b!=0:\na,b=b,a%b";
	      if (isdef)
		adds=isex?"def f(x):\nreturn x*x*x\n":"def f(x):\n\nreturn\n";
	    } else {
	      if (isif)
		adds=(lang==1)?(isex?"si x<0 alors x:=-x; fsi;":"si  alors\n\nsinon\n\nfsi;"):(isex?"if x<0 then x:=-x; fi;":"if  then\n\nelse\n\nfi;");
	      if (lang && iselse)
		adds="sinon ";
	      if (isfor)
		adds=(lang==1)?(isex?"pour j de 1 jusque 10 faire\nprint(j*j);\nfpour;":"pour  de  jusque  faire\n\nfpour;"):(isex?"for j from 1 to 10 do\nprint(j*j);\nod;":"for  from  to  do\n\nod;");
	      if (isforin)
		adds=(lang==1)?(isex?"pour j in [1,4,9,16] faire\nprint(j)\nfpour;":"pour  in  faire\n\nfpour;"):(isex?"for j in [1,4,9,16] do\nprint(j);od;":"for  in  do\n\nod;");
	      if (iswhile)
		adds=(lang==1)?(isex?"a,b:=25,15;\ntantque b!=0 faire\na,b:=b,irem(a,b);\nftantque;a;":"tantque  faire\n\nftantque;"):(isex?"a,b:=25,15;\nwhile b!=0 do\na,b:=b,irem(a,b);\nod;a;":"while  do\n\nod;");
	      if (isdef)
		adds=(lang==1)?(isex?"fonction f(x)\nlocal j;\nj:=x*x;\nreturn j;\nffonction:;\n":"fonction f(x)\nlocal j;\n\nreturn ;\nffonction:;"):(isex?"function f(x)\nlocal j;\nj:=x*x;\nreturn j;\nffunction:;\n":"function f(x)\n  local j;\n\n return ;\nffunction:;");
	    }
	    insert(text,adds,key!=KEY_CTRL_PASTE); // was true, but we should not indent when pasting
	    display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	    const string & s=v[textline].s;
	    int cx=text->cursorx,cy=text->cursory,cp=text->pos;
	    if (tooltip(cx,cy,cp,s.substr(0,cp).c_str(),contextptr)){
	      keytooltip=true;
	    }
	    if (text->pos>0 && (key==KEY_CHAR_ACCOLADES || key==KEY_CHAR_CROCHETS))
	      --text->pos;
	    show_status(text,search,replace);
	    continue;
	  }
	}
      }
      textElement * ptr=& v[textline];
      const int interligne=16;
      switch(key){
      case KEY_CTRL_DEL:
	if (clipline>=0){
	  copy_clipboard(get_selection(text,true),true);
	  // erase selection
	  clipline=-1;
	}
	else {
	  if (editable){
	    if (textpos){
	      set_undo(text);
	      std::string & s=v[textline].s;
	      int nextpos=textpos-1;
	      if (textpos==find_indentation(s)){
		for (int line=textline-1;line>=0;--line){
		  int ind=find_indentation(v[line].s);
		  if (textpos>ind){
		    nextpos=ind;
		    break;
		  }
		}
	      }
	      s.erase(s.begin()+nextpos,s.begin()+textpos);
	      textpos=nextpos;
	    }
	    else {
	      if (textline){
		set_undo(text);
		--textline;
		textpos=v[textline].s.size();
		v[textline].s += v[textline+1].s;
		v[textline].nlines += v[textline+1].nlines;
		v.erase(v.begin()+textline+1);
	      }
	    }
	  }
	  show_status(text,search,replace);
	}
	break;
      case KEY_CTRL_S:
	display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	search=get_searchitem(replace);
	if (!search.empty()){
	  for (;;){
	    if (!move_to_word(text,search,replace,isFirstDraw,totalTextY,scroll,textY,contextptr)){
	      break;
	    }
	  }
	  show_status(text,search,replace);
	}
	continue;
      case KEY_CTRL_OK:
	if (text->allowEXE || !text->editable) return TEXTAREA_RETURN_EXE;
	if (search.size()){
	  for (;;){
	    if (!move_to_word(text,search,replace,isFirstDraw,totalTextY,scroll,textY,contextptr))
	      break;
	  }
	  show_status(text,search,replace);
	  continue;
	}
	else {
          if (!text->OKparse) return TEXTAREA_RETURN_EXE;
	  int err=check_parse(text,v,text->python,contextptr);
	  if (err==KEY_SHUTDOWN)
	    return err;
	  if (err) // move cursor to the error line
	    textline=err-1;
	  continue;
	}
	break;
      case KEY_CTRL_EXE: 
	if (search.size()){
	  for (;;){
	    if (!move_to_word(text,search,replace,isFirstDraw,totalTextY,scroll,textY,contextptr))
	      break;
	  }
	  show_status(text,search,replace);
	  continue;
	}
	if (clipline<0 && editable){
	  set_undo(text);
	  add_indented_line(v,textline,textpos);
	  show_status(text,search,replace);
	}
	break;
      case KEY_CTRL_UNDO:
	undo(text);
	break;
      case KEY_SHIFT_LEFT:
	textpos=0;
	break;
      case KEY_SHIFT_RIGHT:
	textpos=v[textline].s.size();
	break;
      case KEY_CTRL_LEFT:
	if (editable){
	  --textpos;
	  if (textpos<0){
	    if (textline==0)
	      textpos=0;
	    else {
	      --textline;
	      show_status(text,search,replace);
	      textpos=v[textline].s.size();
	    }
	  }
	  if (textpos>=0)
	    break;
	}
      case KEY_CTRL_UP:
	if (editable){
	  if (textline>0){
	    --textline;
	    show_status(text,search,replace);
	  }
	  else {
	    textline=0;
	    textpos=0;
	  }
	} else {
	  if (scroll < 0) {
	    scroll = scroll + interligne;
	    if(scroll > 0) scroll = 0;
	  }
	}
	break;
      case KEY_CTRL_RIGHT:
	++textpos;
	if (textpos<=ptr->s.size())
	  break;
	if (textline==v.size()-1){
	  textpos=ptr->s.size();
	  break;
	}
	textpos=0;
      case KEY_CTRL_DOWN:
	if (editable){
	  if (textline<v.size()-1)
	    ++textline;
	  else {
	    textline=v.size()-1;
	    textpos=v[textline].s.size();
	  }
	  show_status(text,search,replace);
	}
	else {
	  if (textY > scrollableHeight-(showtitle ? 0 : interligne)) {
	    scroll = scroll - interligne;
	    if(scroll < -totalTextY+scrollableHeight-(showtitle ? 0 : interligne)) scroll = -totalTextY+scrollableHeight-(showtitle ? 0 : interligne);
	  }
	}
	break;
      case KEY_CTRL_PAGEDOWN:
	if (editable){
	  textline=v.size()-1;
	  textpos=v[textline].s.size();
	}
	else {
	  if (textY > scrollableHeight-(showtitle ? 0 : interligne)) {
	    scroll = scroll - scrollableHeight;
	    if(scroll < -totalTextY+scrollableHeight-(showtitle ? 0 : interligne)) scroll = -totalTextY+scrollableHeight-(showtitle ? 0 : interligne);
	  }
	}
	break;
      case KEY_CTRL_PAGEUP:
	if (editable)
	  textline=0;
	else {
	  if (scroll < 0) {
	    scroll = scroll + scrollableHeight;
	    if(scroll > 0) scroll = 0;
	  }
	}
	break;
      case KEY_SAVE:
	if (nspire_exam_mode==2)
	  continue;
	save_script(text->filename.c_str(),merge_area(v));
	text->changed=false;
	char status[256];
	sprintf(status,(lang==1)?"%s sauvegarde":"%s saved",text->filename.c_str());
	DefineStatusMessage(status, 1, 0, 0);
	DisplayStatusArea();    	    
	continue;      
      case KEY_CTRL_F1:
	if(text->allowF1) return KEY_CTRL_F1;
	break;
      case KEY_CTRL_MENU: case KEY_CTRL_F6:
	// case KEY_CHAR_ANS:
	if (!text->editable) 	return TEXTAREA_RETURN_EXIT;
	if (clipline<0 && text->editable && text->filename.size()){
	  Menu smallmenu;
	  smallmenu.numitems=12;
	  MenuItem smallmenuitems[smallmenu.numitems];
	  smallmenu.items=smallmenuitems;
	  smallmenu.height=12;
	  smallmenu.scrollbar=0;
	  //smallmenu.title = "KhiCAS";
	  smallmenuitems[0].text = (char*)((lang==1)?"Tester syntaxe":"Check syntax");
	  smallmenuitems[1].text = (char*)((lang==1)?"Sauvegarder":"Save");
	  smallmenuitems[2].text = (char*)((lang==1)?"Sauvegarder comme":"Save as");
	  if (nspire_exam_mode==2) smallmenuitems[1].text = (char*)(lang==1?"Sauvegarde desactivee":"Saving disabled");
	  if (exam_mode || nspire_exam_mode==2) smallmenuitems[2].text = (char*)"";
	  smallmenuitems[3].text = (char*)((lang==1)?"Inserer":"Insert");
	  smallmenuitems[4].text = (char*)((lang==1)?"Effacer":"Clear");
	  smallmenuitems[5].text = (char*)((lang==1)?"Chercher,remplacer":"Search, replace");
	  smallmenuitems[6].text = (char*)((lang==1)?"Aller a la ligne":"Goto line");
	  int p=python_compat(contextptr);
	  if (p&4)
	    smallmenuitems[7].text = (char*)"Syntax [MicroPython]";
	  else {
	    if (p==0)
	      smallmenuitems[7].text = (char*)"Syntax [Xcas francais]";
	    if (p==1)
	      smallmenuitems[7].text = (char*)"Syntax [Xcas comp Python ^=**]";
	    if (p==2)
	      smallmenuitems[7].text = (char*)"Syntax [Xcas comp Python ^=xor]";
	  }
	  smallmenuitems[8].text = (char *)((lang==1)?"Changer taille caracteres":"Change fontsize");
	  smallmenuitems[9].text = (char *)aide_khicas_string;
	  smallmenuitems[10].text = (char *)((lang==1)?"A propos":"About");
	  smallmenuitems[11].text = (char*)((lang==1)?"Quitter":"Quit");
	  int sres = doMenu(&smallmenu);
	  if(sres == MENU_RETURN_SELECTION || sres==KEY_CTRL_EXE) {
	    sres=smallmenu.selection;
	    if (sres==12){
	      int res=check_leave(text);
	      if (res==2)
		continue;
	      return TEXTAREA_RETURN_EXIT;
	    }
	    if(sres >= 10) {
	      textArea text;
	      text.editable=false;
	      text.clipline=-1;
	      text.title = smallmenuitems[sres-1].text;
	      add(&text,smallmenu.selection==10?((lang==1)?shortcuts_fr_string:shortcuts_en_string):((lang==1)?apropos_fr_string:apropos_en_string));
	      if (doTextArea(&text,contextptr)==KEY_SHUTDOWN)
		return KEY_SHUTDOWN;
	      continue;
	    }
	    if (sres==9 && editable){
	      bool minimini=!v[0].minimini;
	      for (int i=0;i<v.size();++i)
		v[i].minimini=minimini;
	      text->lineHeight=minimini?13:17;
	      continue;
	    }
	    if (sres==1){
	      int err=check_parse(text,v,text->python,contextptr);
	      if (err==KEY_SHUTDOWN)
		return err;
	      if (err) // move cursor to the error line
		textline=err-1;
	    } 
	    if (sres==3 && exam_mode==0 && nspire_exam_mode!=2){
	      char filename[MAX_FILENAME_SIZE+1];
	      if (get_filename(filename,".py")){
		text->filename=filename;
		sres=2;
	      }
	    }
	    if(sres == 2 && nspire_exam_mode!=2) {
	      save_script(text->filename.c_str(),merge_area(v));
	      text->changed=false;
	      char status[256];
	      sprintf(status,(lang==1)?"%s sauvegarde":"%s saved",text->filename.c_str());
	      DefineStatusMessage(status, 1, 0, 0);
	      DisplayStatusArea();    	    
	    }
	    if (sres==4){
	      char filename[MAX_FILENAME_SIZE+1];
	      std::string ins;
	      if (giac_filebrowser(filename, "py", "Scripts") && load_script(filename,ins))
		insert(text,ins.c_str(),false);//add_nl(text,ins);
	    }
	    if (sres==5){
	      std::string s(merge_area(v));
#if 0
	      for (size_t i=0;i<s.size();++i){
		if (s[i]=='\n')
		  s[i]=0x1e;
	      }
	      CLIP_Store(s.c_str(),s.size()+1);
#endif
	      copy_clipboard(s,false);
	      set_undo(text);
	      v.resize(1);
	      v[0].s="";
	      textline=0;
	    }
	    if (sres==6){
	      display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	      search=get_searchitem(replace);
	      if (!search.empty()){
		for (;;){
		  if (!move_to_word(text,search,replace,isFirstDraw,totalTextY,scroll,textY,contextptr)){
		    break;
		  }
		}
		show_status(text,search,replace);
	      }
	    }
	    if (sres==7){
	      display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	      int l=get_line_number((lang==1)?"Negatif: en partant de la fin":"Negative: counted from the end",(lang==1)?"Numero de ligne:":"Line number:");
	      if (l>0)
		text->line=l-1;
	      if (l<0)
		text->line=v.size()+l;
	    }
	    if (sres==8){
	      int c=select_interpreter();
	      if (c>=0){
		int p=text->python;
		if (c==3)
		  p |= 0x4;
		else 
		  p=c;
		giac::python_compat(p,contextptr);
		text->python=p;
		xcas_python_eval=c==3;
		show_status(text,search,replace);
		warn_python(text->python,false);
		drawRectangle(0,205,LCD_WIDTH_PX,17,44444);
		PrintMiniMini(0,205,"shift-1 test|2 loop|3 undo|4 misc|5 +- |      ",4,44444,giac::_BLACK);
	      }
	    }
	  }
	}
	break;
      case KEY_CTRL_SETUP: // inactive
	text->python=text->python?0:1;
	show_status(text,search,replace);
	python_compat(text->python,contextptr);
	warn_python(text->python,false);
	drawRectangle(0,205,LCD_WIDTH_PX,17,44444);
	PrintMiniMini(0,205,"shift-1 test|2 loop|3 undo|4 misc|5 +- |      ",4,44444,giac::_BLACK);
	continue;
      case KEY_CTRL_F2:
	if (clipline<0)
	  return KEY_CTRL_F2;
      case KEY_CTRL_EXIT:
	if (clipline>=0){
	  clipline=-1;
	  show_status(text,search,replace);
	  continue;
	}
	if (!search.empty()){
	  search="";
	  show_status(text,search,replace);
	  continue;
	}
	if (check_leave(text)==2)
	  continue;
	return TEXTAREA_RETURN_EXIT;
      case KEY_CTRL_INS:
	break;
      default:
	if (clipline<0 && key>=32 && key<128 && editable){
	  char buf[2]={char(key),0};
	  insert(text,buf,false);
	  show_status(text,search,replace);
	}
	if (key==KEY_CTRL_AC){
	  if (clipline>=0){
	    clipline=-1;
	    show_status(text,search,replace);
	  }
	  else {
	    if (search.size()){
	      search="";
	      show_status(text,search,replace);
	    }
	    else {
	      copy_clipboard(v[textline].s+'\n',true);
	      if (v.size()==1)
		v[0].s="";
	      else {
		v.erase(v.begin()+textline);
		if (textline>=v.size())
		  --textline;
	      }
	      DefineStatusMessage((char*)"Line cut and copied to clipboard", 1, 0, 0);
	      DisplayStatusArea();
	    }
	  }
	}
      }
    }
  }

  void reload_edptr(const char * filename,textArea *edptr,GIAC_CONTEXT){
    if (edptr){
      std::string s(merge_area(edptr->elements));
      copy_clipboard(s,true);
      s="\n";
      edptr->elements.clear();
      edptr->clipline=-1;
      edptr->filename=remove_path(giac::remove_extension(filename))+".py";
      load_script((char *)edptr->filename.c_str(),s);
      if (s.empty())
	s="\n";
      // cout << "script " << edptr->filename << endl;
      edptr->editable=true;
      edptr->changed=false;
      edptr->python=python_compat(contextptr);
      edptr->elements.clear();
      edptr->y=7;
      add(edptr,s);
      edptr->line=0;
      edptr->pos=0;
    }
  }  

  console_line * Line=0;//[_LINE_MAX];//={data_line};
  char menu_f1[8]={0},menu_f2[8]={0},menu_f3[8]={0},menu_f4[8]={0},menu_f5[8]={0},menu_f6[8];
  char session_filename[MAX_FILENAME_SIZE+1]="session";
  char * FMenu_entries_name[6]={menu_f1,menu_f2,menu_f3,menu_f4,menu_f5,menu_f6};
  location Cursor;
  char *Edit_Line=0;
  int Start_Line, Last_Line,editline_cursor;
  int Case;
  int console_changed=0; // 1 if something new in history
  int dconsole_mode=1; // 0 disables dConsole commands

#define Current_Line (Start_Line + Cursor.y)
#define Current_Col (Line[Cursor.y + Start_Line].start_col + Cursor.x)

  void console_disp_status(GIAC_CONTEXT){
    int i=python_compat(contextptr);
    string msg("shell ");
    if (i&4)
      msg+="MicroPython";
    else {
      if (i==0)
	msg+="Xcas";
      else {
	if (i==1)
	  msg+="Py ^=**";
	else
	  msg+="Py ^=xor";
      }
    }
    if (angle_radian(contextptr))
      msg += " RAD ";
    else
      msg += " DEG ";
    msg += session_filename;
    if (console_changed)
      msg += " *";
    statuslinemsg(msg.c_str());
    set_xcas_status();
    Bdisp_PutDisp_DD();
  }

  void leave_exam_mode(GIAC_CONTEXT){
#ifdef NSPIRE_NEWLIB
    // FIXME test USB connection instead
    unsigned NSPIRE_RTC_ADDR=0x90090000;
    unsigned t1= * (volatile unsigned *) NSPIRE_RTC_ADDR;
    int chk=0;
    if (exam_duration<=0 || (t1-exam_start<exam_duration)){
      chk=-1;
#if 1 // checkin the  power management addresses range
      unsigned poweraddr=0x900b0028;
      unsigned u=*(unsigned *)poweraddr;
      //*logptr(contextptr) << "power " << u << '\n';
      if ( is_cx2 || (u&0xff0000)==0x070000) // connected 0x11070114, disconnected 0x11110114
	chk=0;
#endif
#if 0 /// check connection, works only if graph link connection before
      unsigned powermanagement_lockaddr=0x900b0018;
      // Bit 5: #B0000000 - USB OTG controller
      // Bit 6: #B4000000 - USB HOST controller
      *(unsigned *)powermanagement_lockaddr=0x8400a5d;
      unsigned HW_USBCTRL_PORTSC1=0xb0000184;
      unsigned u=*(unsigned *) HW_USBCTRL_PORTSC1;
      if ( (u&0xff000000)==0x11000000) // 0x11000805 vs 0x1d000004
	chk=0;
      // B00001A4 might be used as well: HW_USBCTRL_OTGSC 1f202d20 vs 1f3c1120
#endif
#if 0 // check USB does not work
      nn_ch_t ch = NULL;
      nn_oh_t oh = NULL;
      nn_nh_t nh = NULL;
      oh = TI_NN_CreateOperationHandle();
      int ans=TI_NN_NodeEnumInit((nn_ch_t) oh);//(ch);
      *logptr(contextptr) << "enuminit" << ans << '\n';
      if (ans>=0){
	ans=TI_NN_NodeEnumNext(oh, &nh);
	*logptr(contextptr) << "enumnext" << ans << '\n';
	if (ans>=0){
	  ans=TI_NN_Connect(nh, 0x4060, &ch);
	  *logptr(contextptr) << "connect" << ans << '\n';
	  if (ans>=0){
	    if(ch){
	      TI_NN_Disconnect(ch);
	      chk=0;
	    }
	  }
	}
	TI_NN_NodeEnumDone(oh);
	TI_NN_DestroyOperationHandle(oh);
      }
#endif
    }
#else
    int chk=0;
#endif
    if (chk>=0){
      set_exam_mode(0,contextptr);
    }
    if (exam_mode)
      confirm((lang==1)?"Pour arreter le mode examen":"To stop exam mode",(lang==1)?"branchez la calculatrice puis menu menu":"plug in the calculator then menu menu");
    else
      confirm((lang==1)?"Fin du mode examen":"End exam mode","enter: OK");
  }    

  
  void menu_setup(GIAC_CONTEXT){
    Menu smallmenu;
    smallmenu.numitems=15;
    MenuItem smallmenuitems[smallmenu.numitems];
    smallmenu.items=smallmenuitems;
    smallmenu.height=12;
    smallmenu.scrollbar=1;
    smallmenu.scrollout=1;
    smallmenu.title = (char*)"Config";
#ifdef NUMWORKS
    smallmenuitems[0].type = MENUITEM_CHECKBOX;
    smallmenuitems[0].text = (char*)"x,n,t -> t";
#endif
    smallmenuitems[1].text = (char*)"Syntaxe (Xcas/Python)";
    smallmenuitems[2].type = MENUITEM_CHECKBOX;
    smallmenuitems[2].text = (char*)"Radians (in Xcas)";
    smallmenuitems[3].type = MENUITEM_CHECKBOX;
    smallmenuitems[3].text = (char*)"Sqrt (in Xcas)";
    smallmenuitems[4].text = (char*)"Francais";
    smallmenuitems[5].text = (char*)"English";
    smallmenuitems[6].text = (char*)"Spanish&English";
    smallmenuitems[7].text = (char*)"Greek&English";
    smallmenuitems[8].text = (char*)"Deutsch&English";
    smallmenuitems[9].text = (char *) ((lang==1)?"Raccourcis clavier (0)":"Shortcuts (0)");
    smallmenuitems[10].text = (char*) ((lang==1)?"Mode examen (e^x)":"Exam mode (e^x)");
    smallmenuitems[11].text = (char*) ((lang==1)?"A propos":"About");
    smallmenuitems[14].text = (char*) "Quit";
    if (exam_mode)
      smallmenuitems[14].text = (char*)((lang==1)?"Quitter le mode examen":"Quit exam mode");
    
    // smallmenuitems[2].text = (char*)(isRecording ? "Stop Recording" : "Record Script");
    while(1) {
#ifdef NUMWORKS
      smallmenuitems[0].value = xthetat;
#else
      string dig("Digits (in Xcas): ");
      dig += print_INT_(decimal_digits(contextptr));
      smallmenuitems[0].text = (char*)dig.c_str();
#endif
      string heaps("Micropython heap "+print_INT_(python_heap_size/1024)+"K");
      smallmenuitems[12].text = (char *) heaps.c_str();
      string stacks("-------------");
      // string stacks("Micropython stack "+print_INT_(python_stack_size/1024)+"K"); // enable in micropython mpconfig.h + call to pystack_init + remove continue below
      smallmenuitems[13].text = (char *) stacks.c_str();
      int p=python_compat(contextptr);
      if (p&4)
	smallmenuitems[1].text = (char*)"Change syntax (MicroPython)";
      else {
	if (p==0)
	  smallmenuitems[1].text = (char*)"Change syntax (Xcas)";
	if (p==1)
	  smallmenuitems[1].text = (char*)"Change syntax (Xcas comp Python ^=**)";
	if (p==2)
	  smallmenuitems[1].text = (char*)"Change syntax (Xcas comp Python ^=xor)";
      }
      smallmenuitems[2].value = giac::angle_radian(contextptr);
      smallmenuitems[3].value = giac::withsqrt(contextptr);
      int sres = doMenu(&smallmenu);
      if (sres==MENU_RETURN_EXIT)
	break;
      if (sres == MENU_RETURN_SELECTION  || sres==KEY_CTRL_EXE) {
	if (smallmenu.selection == 1){
#ifdef NUMWORKS	 
	  xthetat=1-xthetat;
#else
	  double d=decimal_digits(contextptr);
	  if (inputdouble("Nombre de digits?",d,contextptr) && d==int(d) && d>0){
	    decimal_digits(d,contextptr);
	  }
#endif
	  continue;
	}
#ifdef MICROPY_LIB
	if (smallmenu.selection == 2){
	  int c=select_interpreter();
	  if (c>=0){
	    int p=giac::python_compat(contextptr);
	    if (c==3)
	      p |= 0x4;
	    else
	      p=c;
	    int old_xcas_python_eval=xcas_python_eval;
	    xcas_python_eval=c==3;
	    giac::python_compat(p,contextptr);
	    if (edptr)
	      edptr->python=p;
	    if (xcas_python_eval!=old_xcas_python_eval){
	      if (xcas_python_eval){
		if (do_confirm((lang==1)?"Effacer les variables Xcas?":"Clear Xcas variables?"))
		  do_restart(contextptr);
	      }
	      else {
#ifdef MICROPY_LIB
		if (do_confirm((lang==1)?"Effacer le tas MicroPython?":"Clear MicroPython heap?"))
		  python_free();
#endif
	      }
	    }
	    warn_python(p,false);
	    Console_FMenu_Init(contextptr);
	    console_disp_status(contextptr);
	    break;
	  }
	}
#endif
	if (smallmenu.selection == 3){
	  giac::angle_radian(!giac::angle_radian(contextptr),contextptr);
	  os_set_angle_unit(giac::angle_radian(contextptr)?0:1);
	  statusline(2*xcas_python_eval);
	  continue;
	}
	if (smallmenu.selection == 4){
	  giac::withsqrt(!giac::withsqrt(contextptr),contextptr);
	  continue;
	}
	if (smallmenu.selection>=5 && smallmenu.selection<=9){
	  lang=smallmenu.selection-4;
	  giac::language(lang,contextptr);
	  break;
	}
	if (smallmenu.selection == 11){
#ifdef NSPIRE_NEWLIB
	  if (nspire_exam_mode==1){
	    if (confirm((lang==1?"Quitter Xcas pour relancer le mode examen":"Leave Xcas to re-enter exam mode"),(lang==1?"!enter OK, esc annul":"enter OK, esc cancel."))!=KEY_CTRL_F1)
	      break;
	    do_restart(contextptr);
	    clear_turtle_history(contextptr);
	    Console_Init(contextptr);
	    Console_Clear_EditLine();
	    console_changed=0;
	    nspire_clear_data(contextptr);
	    nspire_exam_mode=2;
	    set_exam_mode(0,contextptr);
	    break;
	  }
	  else {
	    //nspire_clear_data(contextptr);
	    //set_exam_mode(0,contextptr);
	    if (1
		|| is_cx2
		){
	      textArea text;
	      text.editable=false;
	      text.clipline=-1;
	      text.title = lang==1?"KhiCAS et mode examen":"KhiCAS and exam mode";
	      add(&text,(lang==1)?
		  "Attention, verifiez que le calcul formel est autorise avant d'utiliser KhiCAS en mode examen. En France, c'est en principe autorise lorsque la calculatrice graphique l'est (par exemple au bac)":
		  "Warning! Check that CAS is allowed before running KhiCAS in exam mode.");
	      const char exam_mode_fr_string[]="Pour utiliser KhiCAS en mode examen, il faut effectuer une preparation chez soi quelques heures avant avec une connection PC ou quelques minutes avant l'examen avec un autre etudiant ayant une Nspire CX ou CX II.\nLancer le mode examen sur la calculatrice cible (esc-on), recopier ndless et khicas.tns (ou luagiac.luax.tns et khicaslua.tns) sur la calculatrice cible en mode examen. Avec 2 calculatrices, recommencez sur l'autre calculatrice (mettre l'autre calculatrice en mode examen et copiez dessus ndless et khicas).\nActiver ndless (cable debranche) puis lancez KhiCAS puis touche calculatrice (en-dessous de esc) puis selectionner l'item 11. mode examen, valider : ceci va effacer les donnees et desactiver le clignotement des leds.\n\nAu debut de l'examen, lorsque le surveillant demande d'activer le mode examen, quittez KhiCAS en tapant menu menu (ou appuyez sur reset), le mode examen sera a nouveau actif et les leds clignoteront. Vous pouvez activer ndless et lancez KhiCAS.\nPour les institutions n'acceptant pas KhiCAS en mode examen: demandez a vos etudiants de redemarrer la calculatrice, puis faire esc-on et reinitialiser le mode examen.";
	      const char exam_mode_en_string[]="Running KhiCAS in exam mode requires preparation at home with a PC or a few minutes with another student having a Nspire CX/CXII.\nActivate exam mode on the target calculator (esc-on), connect the PC or the other calculator, copy ndless and khicas.tns (or luagiac.luax.tns and khicaslua.tns) to the target calc (kept in exam mode). With 2 calculators, repeat on the other calculator.\n Activate ndless (disconnect the link) and run KhiCAS. Type the calculator key below esc then select 11. Exam mode. This will desactivate leds blinking and clear data. When exam begins, quit KhiCAS (menu menu) or press reset, exam mode will be active again and leds will blink. Activate ndless and run KhiCAS.\n\nFor institutions who do not want to allow KhiCAS, ask your students to reset their calculator, press esc-on and restart exam mode, this will clear ndless and KhiCAS.";
	      add(&text,(lang==1)?exam_mode_fr_string:exam_mode_en_string);
	      if (doTextArea(&text,contextptr)==KEY_SHUTDOWN)
		return ;
	      break;
	    }
	  }
#endif // NSPIRE_NEWLIB
	  if (!exam_mode && confirm((lang==1?"Verifiez que le calcul formel est autorise.":"Please check that the CAS is allowed."),(lang==1?"France: autorise au bac. Enter: ok, esc: annul":"enter: yes, esc: no"))!=KEY_CTRL_F1)
	    break;
#ifdef NUMWORKS
	  if (do_confirm(lang==1?"Le mode examen se lance depuis Parametres":"Enter Exam mode from Settings"))
	    shutdown_state=1;
	  break;
#endif
	  // confirmation, duree (>=0 French indicative, else not indicative)
	  double duration=exam_mode?absint(exam_duration):0;
	  string msg=(lang==1)?"Compte a rebours en h.min ou 0 pour horloge":"Exam duration in h.min (0: end by pluging)";
	  msg += print_duration(duration);
	  if (inputdouble(msg.c_str(),duration,contextptr)){
	    bool indicative=lang==1?duration>=0:duration<=0;
	    if (exam_mode)
	      indicative=exam_duration<=0;
	    else {
	      if (lang==1 && !indicative && confirm("Attention, mode non conforme au bac en France","enter: corriger, esc: tant pis")!=KEY_CTRL_F6)
		indicative=true;
	    }
	    if (duration<0)
	      duration=-duration;
	    if (duration>10)
	      duration=duration/60;
	    else
	      duration=std::floor(duration)+100.0/60*(duration-std::floor(duration));
	    if (duration){
	      msg=lang==1?"Duree compte a rebours ":"Exam duration ";
	      double d=giacmax(duration*3600,absint(exam_duration));
	      msg += print_duration(d);
	    }
	    else
	      msg="Mode examen.";
	    if (indicative)
	      msg += lang==1?" Fin par branchement":" Exit by pluging";
	    if (confirm(msg.c_str(),(lang==1?"!Blocage dans Xcas en mode exam! enter OK, esc annul":"!Trapped in Xcas in exam mode! enter OK, esc cancel."))==KEY_CTRL_F1){
#ifdef NSPIRE_NEWLIB
	      if (exam_mode) 
		exam_duration=duration?giacmax(absint(exam_duration),duration*3600+30):0;
	      else {
		unsigned NSPIRE_RTC_ADDR=0x90090000;
		exam_start= * (volatile unsigned *) NSPIRE_RTC_ADDR;
		exam_duration = duration?duration*3600+30:0;
	      }
	      if (indicative)
		exam_duration=-absint(exam_duration);
#else
	      exam_start=0;
	      exam_duration=1;
#endif
	      do_restart(contextptr);
	      clear_turtle_history(contextptr);
	      Console_Init(contextptr);
	      Console_Clear_EditLine();
	      set_exam_mode(1,contextptr);
	      strcpy(session_filename,"session.xw");
	      console_changed=0;
	      save_session(contextptr);
	      if (edptr){
		edptr->elements.resize(1);
		edptr->elements[0].s="";
		edptr->undoelements=edptr->elements;
		edptr->line=0;
		edptr->pos=0;
	      }
	      save_script("session.py","");
	    }
	  }
	  break;
	}
#ifdef MICROPY_LIB
	if (smallmenu.selection==13){
	  double d=python_heap_size/1024;
	  if (inputdouble(
#if defined NUMWORKS && defined DEVICE
			  "Tas MicroPython en K (16-64)?"
#else
			  "Tas MicroPython en K (64-4096)?"
#endif
			  ,d,contextptr) && d==int(d) &&
#if defined NUMWORKS && defined DEVICE
	      d>=16 && d<=64
#else
	      d>=64 && d<=4096
#endif
	      ){
	    python_heap_size=d*1024;
	    python_free();
	  }
	  continue;
	}
	if (smallmenu.selection==14){
	  continue;
	  double d=python_stack_size/1024;
	  if (inputdouble(
#if defined NUMWORKS && defined DEVICE
			  "Pile MicroPython en K (8-20)?"
#else
			  "Pile MicroPython en K (32-512)?"
#endif
			  ,d,contextptr) && d==int(d) &&
#if defined NUMWORKS && defined DEVICE
	      d>=8 && d<=20
#else
	      d>=32 && d<=512
#endif
	      ){
	    python_stack_size=d*1024;
	    python_free();
	  }
	  continue;
	}
#endif // MICROPY_LIB
	if (smallmenu.selection == 15){
	  if (exam_mode)
	    leave_exam_mode(contextptr);
	  break;
	}
	if (smallmenu.selection >= 10) {
	  textArea text;
	  text.editable=false;
	  text.clipline=-1;
	  text.title = smallmenuitems[smallmenu.selection-1].text;
	  add(&text,smallmenu.selection==10?((lang==1)?shortcuts_fr_string:shortcuts_en_string):((lang==1)?apropos_fr_string:apropos_en_string));
	  if (doTextArea(&text,contextptr)==KEY_SHUTDOWN)
	    return ;
	  continue;
	} 
      }	
    }      
  }

  void * console_malloc(unsigned s){
    return new char [s];
    // return malloc(s);
  }

  void console_free(void * ptr){
    delete [] (char *) ptr;
    // free(ptr);
  }

  void cleanup(std::string & s){
    for (size_t i=0;i<s.size();++i){
      if (s[i]=='\n')
	s[i]=' ';
    }
  }

  const int max_lines_saved=50;

  int run(const char * s,int do_logo_graph_eqw,GIAC_CONTEXT){
    if (strlen(s)>=2 && (s[0]=='#' ||
			 (s[0]=='/' && (s[1]=='/' || s[1]=='*'))
			 ))
      return 0;
    if (strcmp(s,"caseval(\"\")")==0 || strcmp(s,"eval_expr(\"\")")==0 || (strlen(s)>=4 && strlen(s)<6 && strncmp(s,"xcas",4)==0)){
      xcas_python_eval=0;
      int p=python_compat(contextptr)&3;
      python_compat(p,contextptr);
      if (edptr)
	edptr->python=p;
#ifdef MICROPY_LIB
      if (do_confirm((lang==1)?"Effacer le tas MicroPython?":"Clear MicroPython heap?"))
	python_free();
#endif
      *logptr(contextptr) << "Xcas interpreter\n";
      Console_FMenu_Init(contextptr);
      return 0;
    }
    gen g,ge;
#ifdef MICROPY_LIB
    if (strlen(s)>=6 && strlen(s)<8 && strncmp(s,"python",6)==0){
      switch_to_micropy(contextptr);
      return 0;
    }
    if (xcas_python_eval==1){
      freezeturtle=false;
      micropy_ck_eval(s);
    }
    else 
      do_run(s,g,ge,contextptr);
#else
    do_run(s,g,ge,contextptr);
#endif
    process_freeze();
#ifdef MICROPY_LIB
    if (xcas_python_eval==1)
      return 0;
#endif
    int t=giac::taille(g,GIAC_HISTORY_MAX_TAILLE);  
    int te=giac::taille(ge,GIAC_HISTORY_MAX_TAILLE);
    bool do_tex=false;
    if (t<GIAC_HISTORY_MAX_TAILLE && te<GIAC_HISTORY_MAX_TAILLE){
      giac::vecteur &vin=history_in(contextptr);
      giac::vecteur &vout=history_out(contextptr);
      if (vin.size()>GIAC_HISTORY_SIZE)
	vin.erase(vin.begin());
      vin.push_back(g);
      if (vout.size()>GIAC_HISTORY_SIZE)
	vout.erase(vout.begin());
      vout.push_back(ge);
    }
    if (check_do_graph(ge,do_logo_graph_eqw,contextptr)==KEY_SHUTDOWN)
      return KEY_SHUTDOWN;
    string s_;
    if (ge.type==giac::_STRNG)
      s_='"'+*ge._STRNGptr+'"';
    else {
      if (te>256)
	s_="Object too large";
      else {
	if (ge.is_symb_of_sommet(giac::at_pnt) || (ge.type==giac::_VECT && !ge._VECTptr->empty() && ge._VECTptr->back().is_symb_of_sommet(giac::at_pnt)))
	  s_="Graphic object";
	else {
	  //do_tex=ge.type==giac::_SYMB && has_op(ge,*giac::at_inv);
	  // tex support has been disabled!
	  s_=ge.print(contextptr);
	  // translate to tex? set do_tex to true
	}
      }
    }
#ifdef NUMWORKS
    if (s_.size()>512)
      s_=s_.substr(0,509)+"...";
#else
    if (s_.size()>8192)
      s_=s_.substr(0,8189)+"...";
#endif
    char* edit_line = (char*)Console_GetEditLine();
    Console_Output((const char*)s_.c_str());
    return 0; 
  }

  int run_session(int start,GIAC_CONTEXT){
    std::vector<std::string> v;
    for (int i=start;i<Last_Line;++i){
      if (Line[i].type==LINE_TYPE_INPUT)
	v.push_back((const char *)Line[i].str);
      console_free(Line[i].str);
      Line[i].str=0;
      Line[i].readonly = 0;
      Line[i].type = LINE_TYPE_INPUT;
      Line[i].start_col = 0;
      Line[i].disp_len = 0;
    }
    Line[Last_Line].str=0;
    Last_Line=start;
    Start_Line=Last_Line>LINE_DISP_MAX?Last_Line-LINE_DISP_MAX:0;
    Cursor.x=0;
    Cursor.y=start;
    Line[start].str=Edit_Line;
    Edit_Line[0]=0;
    if (v.empty()) return 0;
    //Console_Init(contextptr);
    for (int i=0;i<v.size();++i){
      Console_Output((const char *)v[i].c_str());
      //int j=Last_Line;
      Console_NewLine(LINE_TYPE_INPUT, 1);
      // Line[j].type=LINE_TYPE_INPUT;
      run(v[i].c_str(),6,contextptr); /* show logo and graph but not eqw */
      // j=Last_Line;
      Console_NewLine(LINE_TYPE_OUTPUT, 1);    
      // Line[j].type=LINE_TYPE_OUTPUT;
      Console_Disp(1,contextptr);
      Bdisp_PutDisp_DD();
    }
    return 0;
  }


  string khicas_state(GIAC_CONTEXT){
    giac::gen g(giac::_VARS(-1,contextptr)); 
    int b=python_compat(contextptr);
    python_compat(0,contextptr);
#if 1
#ifdef NSPIRE_NEWLIB
    char *buf=nspire_filebuf;
    buf[0]=0;
    int bufsize=NSPIRE_FILEBUFFER;
#else
    char buf[6144]="";
    int bufsize=sizeof(buf);
#endif
    if (g.type==giac::_VECT){
      bool ok=true;
      for (int i=0;i<g._VECTptr->size();++i){
	string s((*g._VECTptr)[i].print(contextptr));
	if (strlen(buf)+s.size()+128<bufsize){
	  strcat(buf,s.c_str());
	  strcat(buf,":;");
	}
	else
	  ok=false;
      }
      if (!ok){
	confirm((lang==1)?"Contexte trop lourd, non sauvegarde":"Context too havy, not saved.",(lang==1)?"Re-executez scripts au chargement (esc enter)":"Re-run scripts at load time (esc enter)",true,64);
	buf[0]=0;
      }
    }
    python_compat(b,contextptr);
    if (strlen(buf)+184<bufsize){
      strcat(buf,"python_compat(");
      strcat(buf,giac::print_INT_(b).c_str());
      strcat(buf,",");
      strcat(buf,giac::print_INT_(python_heap_size).c_str());
      strcat(buf,",");
      strcat(buf,giac::print_INT_(python_stack_size).c_str());
      strcat(buf,");angle_radian(");
      strcat(buf,angle_radian(contextptr)?"1":"0");
      strcat(buf,");with_sqrt(");
      strcat(buf,withsqrt(contextptr)?"1":"0");
      strcat(buf,");integer_format(");
      strcat(buf,integer_format(contextptr)==16?"16":"10");
      strcat(buf,");set_language(");
      char l[]="0";
      l[0]+=lang;
      strcat(buf,l);
      strcat(buf,");");
    }
    if (sheetptr){
      string s(current_sheet(vecteur(0),contextptr).print(contextptr));
      if (strlen(buf)+s.size()+20<bufsize){
	strcat(buf,"current_sheet(");
	strcat(buf,s.c_str());
	strcat(buf,");");
      }
    }
    return buf;
#else
    string s(g.print(contextptr));
    python_compat(b,contextptr);
    s += "; python_compat(";
    s +=  giac::print_INT_(b);
    s += ");angle_radian(";
    s += angle_radian(contextptr)?'1':'0';
    s += ");with_sqrt(";
    s += withsqrt(contextptr)?'1':'0';
    s += ");";
    return s;
#endif
  }
  void Bfile_WriteFile_OS(char * & buf,const void * ptr,size_t len){
    memcpy(buf,ptr,len);
    buf += len;
  }
  void Bfile_WriteFile_OS4(char * & buf,size_t n){
    buf[0]= n>>24;
    buf[1]= (n>>16) & 0xff;
    buf[2]= (n & 0xffff)>>8;
    buf[3]= n & 0xff;
    buf += 4;
  }
  void Bfile_WriteFile_OS2(char * & buf,unsigned short n){
    buf[0]= n>>8;
    buf[1]= n & 0xff;
    buf += 2;
  }
  void save_console_state_smem(const char * filename,bool xwaspy,GIAC_CONTEXT){
    console_changed=0;
    string state(khicas_state(contextptr));
    int statesize=state.size();
    string script;
    if (edptr)
      script=merge_area(edptr->elements);
    int scriptsize=script.size();
    // save format: line_size (2), start_col(2), line_type (1), readonly (1), line
    int size=2*sizeof(int)+statesize+scriptsize;
    int start_row=Last_Line-max_lines_saved; 
    if (start_row<0) start_row=0;
    for (int i=start_row;i<=Last_Line;++i){
      size += 2*sizeof(short)+2*sizeof(char)+strlen((const char *)Line[i].str);
    }
    char savebuf[size+4];
#ifdef NUMWORKS
    char * hFile=savebuf+1;
#else
    char * hFile=savebuf;
#endif
    // save variables and modes
    Bfile_WriteFile_OS4(hFile, statesize);
    Bfile_WriteFile_OS(hFile, state.c_str(), statesize);
    // save script
    Bfile_WriteFile_OS4(hFile, scriptsize);
    Bfile_WriteFile_OS(hFile, script.c_str(), scriptsize);
    // save console state
    // save console state
    for (int i=start_row;i<=Last_Line;++i){
      console_line & cur=Line[i];
      unsigned short l=strlen((const char *)cur.str);
      Bfile_WriteFile_OS2(hFile, l);
      unsigned short s=cur.start_col;
      Bfile_WriteFile_OS2(hFile, s);
      unsigned char c=cur.type;
      Bfile_WriteFile_OS(hFile, &c, sizeof(c));
      c=1;//cur.readonly;
      Bfile_WriteFile_OS(hFile, &c, sizeof(c));
      unsigned char buf[l+1];
      buf[l]=0;
      strcpy((char *)buf,(const char*)cur.str); 
      unsigned char *ptr=buf,*strend=ptr+l;
      for (;ptr<strend;++ptr){
	if (*ptr==0x9c)
	  *ptr='\n';
      }
      Bfile_WriteFile_OS(hFile, buf, l);
    }
    char BUF[2]={0,0};
    Bfile_WriteFile_OS(hFile, BUF, sizeof(BUF));
#ifdef NUMWORKS
    savebuf[0]=1;
#endif
    int len=hFile-savebuf;
    if (
#ifdef XWASPY
	xwaspy && len<8192
#else
	0
#endif
	){
      // save as an ascii file beginning with #xwaspy
#ifdef NUMWORKS 
      --len;
      char * buf=savebuf+1;
      int newlen=4*(len+2)/3+11; // 4/3 oldlen + 8(#swaspy\n) +1 + 2 for ending  zeros
      char newbuf[newlen];
      strcpy(newbuf,"##xwaspy\n");
      newbuf[0]=1;
      hFile=newbuf+9;
#else
      char * buf=savebuf;
      int newlen=4*(len+2)/3+10;
      char newbuf[newlen];
      strcpy(newbuf,"#xwaspy\n");
      hFile=newbuf+8;
#endif
      for (int i=0;i<len;i+=3,hFile+=4){
	// keep space \n and a..z chars
	char c;
	while (i<len && ((c=buf[i])==' ' || c=='\n' || c=='{' || c==')' || c==';' || c==':' || c=='\n' || (c>='a' && c<='z')) ){
	  if (c==')')
	    c='}';
	  if (c==':')
	    c='~';
	  if (c==';')
	    c='|';
	  *hFile=c;
	  ++hFile;
	  ++i;
	}
	unsigned char a=buf[i],b=i+1<len?buf[i+1]:0,C=i+2<len?buf[i+2]:0;
	hFile[0]=xwaspy_shift+(a>>2);
	hFile[1]=xwaspy_shift+(((a&3)<<4)|(b>>4));
	hFile[2]=xwaspy_shift+(((b&0xf)<<2)|(C>>6));
	hFile[3]=xwaspy_shift+(C&0x3f);
      }
      //*hFile=0; ++hFile; 
      //*hFile=0; ++hFile; 
      write_file(filename,newbuf,hFile-newbuf);
    }
    else {
      write_file(filename,savebuf,len);
    }
  }

  size_t Bfile_ReadFile_OS4(const char * & hf_){
    const unsigned char * hf=(const unsigned char *)hf_;
    size_t n=(((((hf[0]<<8)+hf[1])<<8)+hf[2])<<8)+hf[3];
    hf_ += 4;
    return n;
  }

  size_t Bfile_ReadFile_OS2(const char * & hf_){
    const unsigned char * hf=(const unsigned char *)hf_;
    size_t n=(hf[0]<<8)+hf[1];
    hf_ += 2;
    return n;
  }

  void Bfile_ReadFile_OS(const char * &hf,char * dest,size_t len){
    memcpy(dest,hf,len);
    hf += len;
  }

  bool load_console_state_smem(const char * filename,GIAC_CONTEXT){
    const char * hf=read_file(filename);
    if (!hf) return false;
    string str;
    if (strncmp(hf,"#xwaspy\n",8)==0){
      hf+=8;
      const char * source=hf;
      for (;*source;source+=4){
	while (*source=='\n' || *source==' ' || (*source>='a' && *source<='~')){
	  char c=*source;
	  if (c=='}')
	    c=')';
	  if (c=='|')
	    c=';';
	  if (c=='~')
	    c=':';
	  str += c;
	  ++source;
	}
	if (!*source)
	  break;
	unsigned char a=source[0]-xwaspy_shift,b=source[1]-xwaspy_shift,c=source[2]-xwaspy_shift,d=source[3]-xwaspy_shift;
	str += (a<<2)|(b>>4);
	str += (b<<4)|(c>>2);
	str += (c<<6)|d;
      }
      hf=str.c_str();
    }
    size_t L=Bfile_ReadFile_OS4(hf);
    char BUF[L+4];
    BUF[1]=BUF[0]='/'; // avoid trying python compat.
    BUF[2]='\n';
    Bfile_ReadFile_OS(hf,BUF+3,L);
    BUF[L+3]=0;
    giac::gen g,ge;
    dconsole_mode=0; python_compat(contextptr)=0; xcas_mode(contextptr)=0;
    bool bi=try_parse_i(contextptr);
    try_parse_i(false,contextptr);
    do_run((char*)BUF,g,ge,contextptr);
    try_parse_i(bi,contextptr);
    dconsole_mode=1;
    // read script
    L=Bfile_ReadFile_OS4(hf);
    if (L>0){
      char bufscript[L+1];
      Bfile_ReadFile_OS(hf,bufscript,L);
      bufscript[L]=0;
      if (edptr==0)
	edptr=new textArea;
      if (edptr){
	edptr->elements.clear();
	edptr->clipline=-1;
	edptr->filename=remove_path(giac::remove_extension(filename))+".py";
	//cout << "script " << edptr->filename << endl;
	edptr->editable=true;
	edptr->changed=false;
	edptr->python=python_compat(contextptr);
	edptr->elements.clear();
	edptr->y=0;
	add(edptr,bufscript);
	edptr->line=0;
	//edptr->line=edptr->elements.size()-1;
	edptr->pos=0;
      }    
    }
    // read console state
    // insure parse messages are cleared
    Console_Init(contextptr);
    Console_Clear_EditLine();
    for (int pos=0;;++pos){
      unsigned short int l,curs;
      unsigned char type,readonly;
      if ( (l=Bfile_ReadFile_OS2(hf))==0) break;
      curs=Bfile_ReadFile_OS2(hf);
      type = *hf; ++hf;
      readonly=*hf; ++hf;
      char buf[l+1];
      Bfile_ReadFile_OS(hf,buf,l);
      buf[l]=0;
      // ok line ready in buf
      while (Line[Current_Line].readonly)
	Console_MoveCursor(CURSOR_DOWN);
      Console_Input(buf);
      Console_NewLine(LINE_TYPE_INPUT, 1);
#if 1
      if (Current_Line>0){
	console_line & cur=Line[Current_Line-1];
	cur.type=type;
	cur.readonly=readonly;
	cur.start_col+=curs;
      }
#endif
    }
    console_changed=0;
    if (python_compat(contextptr)&4){
      xcas_python_eval=1;
      if (edptr){
	check_parse(edptr,edptr->elements,python_compat(contextptr),contextptr);
      }
    }
    else
      xcas_python_eval=0;
    Console_FMenu_Init(contextptr); // insure the menus are sync-ed
    return true;
  }

  /*

    The following functions will be used to specify the location before deleting a string of n characters altogether. Among them, a wide character (2 bytes) will be counted as a character.
	
    For example, we have the following string str:
	
    Location  |  0  |  1  |  2  |  3  |  4  |  5  | 6 |
    Character | 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 0 |

    After the call Console_DelStr (str, 3, 2), position 1 and 2 characters will be deleted, then the characters will be in advance.
	
    Results are as follows:

    Location  |  0  |  1  |  2  |  3  | 4 |  5  | 6 |
    Character | 'a' | 'd' | 'e' | 'f' | 0 | 'f' | 0 |

    (Note: the extra positions will not be filled with '\ 0', but '\ 0' will be a copy of the original end of the string.)

  */

  int Console_DelStr(char *str, int end_pos, int n)
  {
    int str_len, actual_end_pos, start_pos, actual_start_pos, del_len, i;

    str_len = strlen((const char *)str);
    if ((start_pos = end_pos - n) < 0) return CONSOLE_ARG_ERR;

    if ((actual_end_pos = Console_GetActualPos(str, end_pos)) == CONSOLE_ARG_ERR) return CONSOLE_ARG_ERR;
    if ((actual_start_pos = Console_GetActualPos(str, start_pos)) == CONSOLE_ARG_ERR) return CONSOLE_ARG_ERR;

    del_len = actual_end_pos - actual_start_pos;

    for (i = actual_start_pos; i < str_len; i++)
      {
	str[i] = str[i + del_len];
      }

    return CONSOLE_SUCCEEDED;
  }

  /*

    The following functions are used to specify the location of the insertion in the specified string.
    (Note: This refers to the position of the printing position when, rather than the actual position.)
  */

  int Console_InsStr(char *dest, const char *src, int disp_pos)
  {
    int i, ins_len, str_len, actual_pos;

    ins_len = strlen((const char *)src);
    str_len = strlen((const char *)dest);

    actual_pos = Console_GetActualPos(dest, disp_pos);

    if (ins_len + str_len >= EDIT_LINE_MAX) return CONSOLE_MEM_ERR;
    if (actual_pos > str_len) return CONSOLE_ARG_ERR;

    for (i = str_len; i >= actual_pos; i--)
      {
	dest[i + ins_len] = dest[i];
      }

    for (i = 0; i < ins_len; i++)
      {
	char c=src[i];
	if (c=='\n') c=0x9c;
	dest[actual_pos + i] = (c==0x0a?' ':c);
      }

    return CONSOLE_SUCCEEDED;
  }

  /*

    The following function is used to determine the true position of the string corresponding to the printing position.
    For example, in the following this string str contains wide characters, the location of the print is as follows:

    Location  | 00  |  01 |   02  |  03  | 04   | 05  | 06  |
    Character | one | two | three | four | five | six | \ 0 |

    The actual storage location is as follows:

    Location | 	00  | 01   |  02  |  03  |  04  |  05  |  06  |  07  |  08  |  09  |  10  |  11  |
    Value 	 | 0xD2 | 0xBB | 0xB6 | 0xFE | 0xC8 | 0xFD | 0xCB | 0xC4 | 0xCE | 0xE5 | 0xC1 | 0xF9 |

    You can find the first four characters 'five' is actually stored in the eighth position.
    So, when you call Console_GetActualPos (str, 4), it will return 8.
  */

  int Console_GetActualPos(const char *str, int disp_pos)
  {
    int actual_pos, count;

    for (actual_pos = count = 0; count < disp_pos; count++)
      {
	if (str[actual_pos] == '\0') return CONSOLE_ARG_ERR;

	if (is_wchar(str[actual_pos]))
	  {
	    actual_pos += 2;
	  }
	else
	  {
	    actual_pos++;
	  }
      }

    return actual_pos;
  }

  /*
    The following functions are used to obtain a string of print length, ie, a wide character (2 bytes) recorded as a character.
  */

  int Console_GetDispLen(const char *str)
  {
    int i, len;

    for (i = len = 0; str[i]!='\0'; len++)
      {
	if (is_wchar(str[i]))
	  {
	    i += 2;
	  }
	else
	  {
	    i++;
	  }
      }

    return len;
  }

  /*
    The following functions are used to move the cursor.
  */

  int Console_MoveCursor(int direction)
  {
    switch (direction)
      {
      case CURSOR_UP:
	if (Current_Line==Last_Line)
	  editline_cursor=Cursor.x;
	//If you need to operate.
	if ((Cursor.y > 0) || (Start_Line > 0)){
	  //If the current line is not read-only, then Edit_Line copy to the current line.
	  if (!Line[Current_Line].readonly){
	    if ((Line[Current_Line].str = (char *)console_malloc(strlen((const char *)Edit_Line) + 1)) == NULL) return CONSOLE_MEM_ERR;
	    strcpy((char *)Line[Current_Line].str, (const char *)Edit_Line);
	    Line[Current_Line].disp_len = Console_GetDispLen(Line[Current_Line].str);
	    Line[Current_Line].type = LINE_TYPE_INPUT;
	  }
	  //If the cursor does not move to the top of, directly move the cursor upward.
	  if (Cursor.y > 0)
	    Cursor.y--;
	  //Otherwise, the number of rows, if the screen's first line is not the first line, then began to show minus one.
	  else {
	    if (Start_Line > 0)
	      Start_Line--;
	  }
	  //End if the horizontal position after moving the cursor over the line, then move the cursor to the end of the line.
	  if (Cursor.x > Line[Current_Line].disp_len){
	    Cursor.x = Line[Current_Line].disp_len;
	  }
	  else {
	    if (Line[Current_Line].disp_len - Line[Current_Line].start_col > COL_DISP_MAX){
	      if (Cursor.x == COL_DISP_MAX)
		Cursor.x = COL_DISP_MAX - 1;
	    }
	  }
	  //If you move the cursor to the line after the first, and the front of the line there is a character does not appear, then move the cursor to position 1.
	  if (Cursor.x == 0 && Line[Current_Line].start_col > 0)
	    Cursor.x = 1;
	  //If the current cursor line is not read-only, then it is a string copy to Edit_Line for editing.
	  if (!Line[Current_Line].readonly){
	    strcpy((char *)Edit_Line, (const char *)Line[Current_Line].str);
	    console_free(Line[Current_Line].str);
	    Line[Current_Line].str = Edit_Line;
	  }
	}
	break;
      case CURSOR_ALPHA_UP:{
	int pos1=Start_Line+Cursor.y;
	Console_MoveCursor(CURSOR_UP);
	int pos2=Start_Line+Cursor.y;
	if (pos1<Last_Line && pos2<Last_Line && pos1!=pos2){
	  console_line curline=Line[pos1];
	  Line[pos1]=Line[pos2];
	  Line[pos2]=curline;
	}
	break;
      }
      case CURSOR_ALPHA_DOWN: {
	int pos1=Start_Line+Cursor.y;
	Console_MoveCursor(CURSOR_DOWN);
	int pos2=Start_Line+Cursor.y;
	if (pos1<Last_Line && pos2<Last_Line && pos1!=pos2){
	  console_line curline=Line[pos1];
	  Line[pos1]=Line[pos2];
	  Line[pos2]=curline;
	}
	break;
      }
      case CURSOR_DOWN:
	if (Current_Line==Last_Line)
	  editline_cursor=Cursor.x;
	//If you need to operate.
	if ((Cursor.y < LINE_DISP_MAX - 1) && (Current_Line < Last_Line) || (Start_Line + LINE_DISP_MAX - 1 < Last_Line))
	  {
	    //If the current line is not read-only, then Edit_Line copy to the current line.
	    if (!Line[Current_Line].readonly)
	      {
		if ((Line[Current_Line].str = (char *)console_malloc(strlen((const char *)Edit_Line) + 1)) == NULL) return CONSOLE_MEM_ERR;
		strcpy((char *)Line[Current_Line].str, (const char *)Edit_Line);
		Line[Current_Line].disp_len = Console_GetDispLen(Line[Current_Line].str);
		Line[Current_Line].type = LINE_TYPE_INPUT;
	      }

	    //If the cursor does not move to the bottom, the cursor moves down directly.
	    if (Cursor.y < LINE_DISP_MAX - 1 && Current_Line < Last_Line)
	      {
		Cursor.y++;
	      }
	    //The number of rows Otherwise, if the last line is not the last line on the screen, it will begin to show a plus.
	    else if (Start_Line + LINE_DISP_MAX - 1 < Last_Line)
	      {
		Start_Line++;
	      }

	    //If you move the cursor after the end of the horizontal position over the line, then move the cursor to the end of the line.
	    if (Cursor.x > Line[Current_Line].disp_len)
	      {
		Cursor.x = Line[Current_Line].disp_len;
	      }
	    else if (Line[Current_Line].disp_len - Line[Current_Line].start_col >= COL_DISP_MAX)
	      {
		if (Cursor.x == COL_DISP_MAX) Cursor.x = COL_DISP_MAX - 1;
	      }

	    //If you move the cursor to the line after the first, and the front of the line there is a character does not appear, then move the cursor to position 1.
	    if (Cursor.x == 0 && Line[Current_Line].start_col > 0) Cursor.x = 1;

	    //If the current cursor line is not read-only, then it is a string copy to Edit_Line for editing.
	    if (!Line[Current_Line].readonly)
	      {
		strcpy((char *)Edit_Line, (const char *)Line[Current_Line].str);
		console_free(Line[Current_Line].str);
		Line[Current_Line].str = Edit_Line;
	      }
	  }
	break;
      case CURSOR_LEFT:
	if (Line[Current_Line].readonly){
	  if (Line[Current_Line].start_col > 0){
	    Line[Current_Line].start_col--;
	  }
	  break;
	}
	else {
	  if (Line[Current_Line].start_col > 0){
	    if (Cursor.x > 1)
	      Cursor.x--;
	    else
	      Line[Current_Line].start_col--;
	    break;
	  }
	  if (Cursor.x > 0){
	    Cursor.x--;
	    break;
	  }
	}
      case CURSOR_SHIFT_RIGHT:
	if (!Line[Current_Line].readonly)
	  Cursor.x=giacmin(Line[Current_Line].disp_len,COL_DISP_MAX);
	if (Line[Current_Line].disp_len > COL_DISP_MAX)
	  Line[Current_Line].start_col = Line[Current_Line].disp_len - COL_DISP_MAX;
	break;
      case CURSOR_RIGHT:
	if (Line[Current_Line].readonly){
	  if (Line[Current_Line].disp_len - Line[Current_Line].start_col > COL_DISP_MAX){
	    Line[Current_Line].start_col++;
	  }
	  break;
	}
	else {
	  if (Line[Current_Line].disp_len - Line[Current_Line].start_col > COL_DISP_MAX){
	    if (Cursor.x < COL_DISP_MAX - 1)
	      Cursor.x++;
	    else
	      Line[Current_Line].start_col++;
	    break;	  
	  }
	  if (Cursor.x < Line[Current_Line].disp_len - Line[Current_Line].start_col){
	    Cursor.x++;
	    break;
	  }
	}
      case CURSOR_SHIFT_LEFT:
	if (!Line[Current_Line].readonly)
	  Cursor.x=0;
	Line[Current_Line].start_col=0;
	break;
      default:
	return CONSOLE_ARG_ERR;
	break;
      }
    return CONSOLE_SUCCEEDED;
  }

  /*
    The following function is used for input.
    String input to the cursor, the cursor will automatically move.
  */

  int Console_Input(const char *str)
  {
    console_changed=1;
    int old_len,i,return_val;

    if (!Line[Current_Line].readonly)
      {
	old_len = Line[Current_Line].disp_len;
	return_val = Console_InsStr(Edit_Line, str, Current_Col);
	if (return_val != CONSOLE_SUCCEEDED) return return_val;
	if ((Line[Current_Line].disp_len = Console_GetDispLen(Edit_Line)) == CONSOLE_ARG_ERR) return CONSOLE_ARG_ERR;
	for (i = 0; i < Line[Current_Line].disp_len - old_len; i++)
	  {
	    Console_MoveCursor(CURSOR_RIGHT);
	  }
	return CONSOLE_SUCCEEDED;
      }
    else
      {
	return CONSOLE_ARG_ERR;
      }
  }

  /*
    The following functions are used to output the string to the current line.
  */

  int Console_Output(const char *str)  {
    if (!Line) return 0;
    console_changed=1;
    int return_val, old_len, i;

    if (!Line[Current_Line].readonly)
      {
	old_len = Line[Current_Line].disp_len;

	return_val = Console_InsStr(Edit_Line, str, Current_Col);
	if (return_val != CONSOLE_SUCCEEDED) return return_val;
	if ((Line[Current_Line].disp_len = Console_GetDispLen(Edit_Line)) == CONSOLE_ARG_ERR) return CONSOLE_ARG_ERR;
	Line[Current_Line].type = LINE_TYPE_OUTPUT;

	for (i = 0; i < Line[Current_Line].disp_len - old_len; i++)
	  {
	    Console_MoveCursor(CURSOR_RIGHT);
	  }
	return CONSOLE_SUCCEEDED;
      }
    else
      {
	return CONSOLE_ARG_ERR;
      }
  }

  void dConsolePut(const char * S){
    if (!dconsole_mode)
      return;
    int l=strlen(S);
    char s[l+1];
    strcpy(s,S);
    for (int i=0;i<l;++i){
      if (s[i]=='\n' ||
	  s[i]==10)
	s[i]=' ';
    }
    Console_Output((const char *)s);
    if (l && S[l-1]=='\n'){
      Console_NewLine(LINE_TYPE_OUTPUT, 1);
      if (!freeze)
	Console_Disp(1,0);
    }
  }

  void dPuts(const char * s){
    dConsolePut(s);
  }

#define PUTCHAR_LEN 35
  static char putchar_buf[PUTCHAR_LEN+2];
  static int putchar_pos=0;
  void dConsolePutChar(const char ch){
    if (!dconsole_mode)
      return;
    if (putchar_pos==PUTCHAR_LEN)
      dConsolePutChar('\n');
    if (ch=='\n'){
      putchar_buf[putchar_pos]='\n';
      putchar_buf[putchar_pos+1]=0;
      putchar_pos=0;
      dConsolePut(putchar_buf);
    }
    else {
      putchar_buf[putchar_pos]=ch;
      ++putchar_pos;
    }
  }

  /*
    Clear the current output line
  */

  void Console_Clear_EditLine()
  {
    if(!Line[Current_Line].readonly) {
      Edit_Line[0] = '\0';
      Line[Current_Line].start_col = 0;
      Line[Current_Line].disp_len = 0;
      Cursor.x = 0;
    }
  }

  /*

    The following functions are used to create a new line.
    Pre_line_type type parameter is used to specify the line, pre_line_readonly parameter is used to specify the line is read-only.
    New_line_type parameter is used to specify the type of the next line, new_line_readonly parameter is used to specify the next line is read-only.
  */

  int Console_NewLine(int pre_line_type, int pre_line_readonly)
  {
    if (!Line) return 0;
    console_changed=1;
    int i;

    if (strlen((const char *)Edit_Line)||Line[Current_Line].type==LINE_TYPE_OUTPUT)
      {
	//Èç¹ûÒÑ¾­ÊÇËùÄÜ´æ´¢µÄ×îºóÒ»ÐÐ£¬ÔòÉ¾³ýµÚÒ»ÐÐ¡£
	//If this is the last line we can store, delete the first line.
	if (Last_Line == _LINE_MAX - 1)
	  {
	    for (i = 0; i < Last_Line; i++)
	      {
		Line[i].disp_len = Line[i + 1].disp_len;
		Line[i].readonly = Line[i + 1].readonly;
		Line[i].start_col = Line[i + 1].start_col;
		Line[i].str = Line[i + 1].str;
		Line[i].type = Line[i + 1].type;
	      }
	    Last_Line--;

	    if (Start_Line > 0) Start_Line--;
	  }

	if (Line[Last_Line].type == LINE_TYPE_OUTPUT && strlen((const char *)Edit_Line) == 0) Console_Output((const char *)"Done");

	//Edit_Line copy the contents to the last line.

	if ((Line[Last_Line].str = (char *)console_malloc(strlen((const char *)Edit_Line) + 1)) == NULL) return CONSOLE_MEM_ERR;
	strcpy((char *)Line[Last_Line].str, (const char *)Edit_Line);

	if ((Line[Last_Line].disp_len = Console_GetDispLen(Line[Last_Line].str)) == CONSOLE_ARG_ERR) return CONSOLE_ARG_ERR;
	Line[Last_Line].type = pre_line_type;
	Line[Last_Line].readonly = pre_line_readonly;
	Line[Last_Line].start_col = 0;

	Edit_Line[0] = '\0';

	Last_Line++;

	Cursor.x = 0;

	if ((Last_Line - Start_Line) == LINE_DISP_MAX)
	  {
	    Start_Line++;
	  }
	else
	  {
	    Cursor.y++;
	  }

	Line[Last_Line].str = Edit_Line;
	Line[Last_Line].readonly = 0;
	Line[Last_Line].type = LINE_TYPE_INPUT;
	Line[Last_Line].start_col = 0;
	Line[Last_Line].disp_len = 0;

	return CONSOLE_NEW_LINE_SET;
      }
    else
      {
	return CONSOLE_NO_EVENT;
      }
  }

  void Console_Insert_Line(){
    if (Last_Line>=_LINE_MAX-1)
      return;
    for (int i=Last_Line;i>=Current_Line;--i){
      Line[i+1]=Line[i];
    }
    ++Last_Line;
    int i=Current_Line;
    console_line & l=Line[i];
    l.str=(char *)console_malloc(2);
    strcpy((char *)l.str,"0");
    l.type=Line[i+1].type==LINE_TYPE_INPUT?LINE_TYPE_OUTPUT:LINE_TYPE_INPUT;
    l.start_col=0;
    l.readonly=1;
    l.disp_len=Console_GetDispLen(l.str);
  }

  /*
    The following function is used to delete a character before the cursor.
  */

  int Console_Backspace(GIAC_CONTEXT){
    console_changed=1;
    if (Last_Line>0 && Current_Line<Last_Line){
      int i=Current_Line;
      if (Edit_Line==Line[i].str)
	Edit_Line=Line[i+1].str;
      if (Line[i].str){
	copy_clipboard((const char *)Line[i].str,true);
	console_free(Line[i].str);
      }
      for (;i<Last_Line;++i){
	Line[i]=Line[i+1];
      }
      Line[i].readonly = 0;
      Line[i].type = LINE_TYPE_INPUT;
      Line[i].start_col = 0;
      Line[i].disp_len = 0;
      Line[i].str=0;
      --Last_Line;
      if (Start_Line>0)
	--Start_Line;
      else {
	if (Cursor.y>0)
	  --Cursor.y;
      }
#if 1
      if (Last_Line==0 && Current_Line==0){ // workaround
	char buf[strlen((const char*)Edit_Line)+1];
	strcpy(buf,(const char*)Edit_Line);
	Console_Init(contextptr);
	Console_Clear_EditLine();
	if (buf[0])
	  Console_Input((const char *)buf);
	//std::string status(giac::print_INT_(Last_Line)+" "+(giac::print_INT_(Current_Line)+" ")+giac::print_INT_(Line[Current_Line].str)+" "+(const char*)Line[Current_Line].str);
	//DefineStatusMessage(status.c_str(),1,0,0);
	//DisplayStatusArea();
      }
#endif
      Console_Disp(1,0);
      return CONSOLE_SUCCEEDED;
    }
    int return_val;
    return_val = Console_DelStr(Edit_Line, Current_Col, 1);
    if (return_val != CONSOLE_SUCCEEDED) return return_val;
    Line[Current_Line].disp_len = Console_GetDispLen(Edit_Line);
    return Console_MoveCursor(CURSOR_LEFT);
  }

  /*
    The following functions are used to deal with the key.
  */

  void chk_clearscreen(GIAC_CONTEXT){
    drawRectangle(0, 24, LCD_WIDTH_PX, LCD_HEIGHT_PX-24, COLOR_WHITE);
    if (confirm((lang==1)?"Effacer l'historique?":"Clear history?",
#ifdef NSPIRE_NEWLIB
		(lang==1)?"enter: oui, esc: conserver":"enter: yes, esc: keep",
#else
		(lang==1)?"OK: oui, Back: conserver":"OK: yes, Back: keep",
#endif
		false)==KEY_CTRL_F1){
      Console_Init(contextptr);
      Console_Clear_EditLine();
    }    
    Console_Disp(1,0);
  }


  /*
    int handle_f5(){
    int keyflag = GetSetupSetting( (unsigned int)0x14);
    if (keyflag == 0x04 || keyflag == 0x08 || keyflag == 0x84 || keyflag == 0x88) {
    // ^only applies if some sort of alpha (not locked) is already on
    if (keyflag == 0x08 || keyflag == 0x88) { //if lowercase
    SetSetupSetting( (unsigned int)0x14, keyflag-0x04);
    DisplayStatusArea();
    return 1; //do not process the key, because otherwise we will leave alpha status
    } else {
    SetSetupSetting( (unsigned int)0x14, keyflag+0x04);
    DisplayStatusArea();
    return 1; //do not process the key, because otherwise we will leave alpha status
    }
    }
    if (keyflag==0) {
    SetSetupSetting( (unsigned int)0x14, 0x88);	
    DisplayStatusArea();
    }
    return 0;
    }
  */

  int Console_Eval(const char * buf,GIAC_CONTEXT){
    int start=Current_Line;
    console_free(Line[start].str);
    Line[start].str=(char *)console_malloc(strlen(buf)+1);
    strcpy((char *)Line[start].str,buf);
    run_session(start,contextptr);
    int move_line = Last_Line - start;
    for (int i = 0; i < move_line; i++)
      Console_MoveCursor(CURSOR_UP);
    return CONSOLE_SUCCEEDED;
  }


  void save(const char * fname,GIAC_CONTEXT){
    if (nspire_exam_mode==2)
      return;
    clear_abort();
#if 0
    return;
#else
    string filename(remove_path(remove_extension(fname)));
#if defined NUMWORKS && defined XWASPY
    bool xwaspy=filename!="session"; // xw will be saved as a fake .py file
#else
    bool xwaspy=false;
#endif
    if (xwaspy){
      if (filename.size()>3 && filename.substr(filename.size()-3,3)=="_xw")
	filename += ".py";
      else
	filename += "_xw.py";
    }
    else
      filename+=".xw";
#ifdef NSPIRE_NEWLIB
    filename+=".tns";
#endif
    save_console_state_smem(filename.c_str(),xwaspy,contextptr); // call before save_khicas_symbols_smem(), because this calls create_data_folder if necessary!
    // save_khicas_symbols_smem(("\\\\fls0\\"+filename+".xw").c_str());
    if (edptr)
      check_leave(edptr);
#endif
  }

  int restore_script(string &filename,bool msg,GIAC_CONTEXT){
    // it's not a session, but a script, restore last session settings and load script
#ifdef NSPIRE_NEWLIB
    const char sessionname[]="session.xw.tns";
#else
    const char sessionname[]="session.xw";
#endif 
    if (file_exists(sessionname)){
      load_console_state_smem(sessionname,contextptr);
      Console_Init(contextptr);
      Console_Clear_EditLine();
    }
    else python_compat(1,contextptr);
    //return 1;
    string s;
    filename=remove_path(remove_extension(filename));
    if (msg && filename!="session"){
      *logptr(contextptr) << (lang==1?"shift ) 8 ou python/xcas pour changer d'interpreteur\n":"shift ) 8 or python/xcas to change interpreter\n");
      *logptr(contextptr) << (lang==1?"Taper esc pour editeur ou avec Micropython executez\n":"Press esc for editor or in MicroPython exec\n");
      *logptr(contextptr) << "from "+filename+" import *\n";
    }
#ifdef NSPIRE_NEWLIB
    filename += ".py.tns";
#else
    filename += ".py";
#endif
    load_script(filename.c_str(),s);
    if (s.empty())
      s="\n";
    if (edptr==0)
      edptr=new textArea;
    edptr->filename=filename;
    edptr->editable=true;
    edptr->changed=false;
    edptr->python=python_compat(contextptr);
    edptr->elements.clear();
    edptr->y=7;
    add(edptr,s);
    edptr->line=0;
    edptr->pos=0;
    return 2;
  }

  int restore_session(const char * fname,GIAC_CONTEXT){
    // cout << "0" << fname << endl; Console_Disp(1); GetKey(&key);
    string filename(fname); //filename="mandel.py.tns";
    if (filename.size()>4 && filename.substr(filename.size()-4,4)==".tns")
      filename=filename.substr(0,filename.size()-4);
    if (filename.size()>3 && filename.substr(filename.size()-3,3)==".py")
      return restore_script(filename,true,contextptr);
    filename=remove_path(remove_extension(fname));
#ifdef NSPIRE_NEWLIB
    if (file_exists((filename+".xw.tns").c_str())){
      strcpy(session_filename,filename.c_str());
      filename += ".xw.tns";
    }
    else {
      if (file_exists((filename+".py.tns").c_str()))
	return restore_script(filename,true,contextptr);
    }
#else
    if (file_exists((filename+".xw").c_str())){
      strcpy(session_filename,filename.c_str());
      filename += ".xw";
    }
    else {
      if (file_exists((filename+".py").c_str()))
	return restore_script(filename,true,contextptr);
    }
#endif
    if (!load_console_state_smem(filename.c_str(),contextptr)){
      int x=0,y=0;
      PrintMini(x,y,"KhiCAS 1.6 (c) 2020 B. Parisse",TEXT_MODE_NORMAL, COLOR_BLACK, COLOR_WHITE);
      y +=18;
      PrintMini(x,y,"et al, License GPL 2",TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
      y += 18;
#ifdef NSPIRE_NEWLIB
      PrintMini(x,y,((lang==1)?"Taper menu plusieurs fois":"Type menu several times"),TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
#else
      PrintMini(x,y,((lang==1)?"Taper HOME plusieurs fois":"Type HOME several times"),TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
#endif
      y += 18;
      PrintMini(x,y,((lang==1)?"pour quitter KhiCAS.":"to leave KhiCAS."),TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
      y += 18;
      PrintMini(x,y,(lang==1)?"Si le calcul formel est interdit":"If CAS is forbidden!",TEXT_MODE_NORMAL, COLOR_RED, COLOR_WHITE);
      y += 18;
#ifdef NSPIRE_NEWLIB
      PrintMini(x,y,(lang==1)?"quittez Khicas (menu menu menu)":"Leave Khicas (menu menu menu)",TEXT_MODE_NORMAL, COLOR_RED, COLOR_WHITE);
      if (confirm("Interpreter? enter: Xcas, esc: MicroPython",(lang==1?"Peut se modifier depuis menu configuration":"May be changed later from menu configuration"),false,130)==KEY_CTRL_F6){
	python_compat(4,contextptr);
	xcas_python_eval=1;
	*logptr(contextptr) << "Micropython interpreter\n";
	Console_FMenu_Init(contextptr);
      }
      else {
	python_compat(1,contextptr);
	*logptr(contextptr) << "Xcas interpreter, Python compatible mode\n";
      }
#else
      PrintMini(x,y,(lang==1)?"quittez Khicas (HOME HOME HOME)":"Leave Khicas (HOME HOME HOME)",TEXT_MODE_NORMAL, COLOR_RED, COLOR_WHITE);
      if (confirm("Interpreter? OK: Xcas, Back: MicroPython",(lang==1?"Peut se modifier depuis menu configuration":"May be changed later from menu configuration"),false,130)==KEY_CTRL_F6){
	python_compat(4,contextptr);
	xcas_python_eval=1;
	*logptr(contextptr) << "Micropython interpreter\n";
	Console_FMenu_Init(contextptr);
      }
      else {
	python_compat(1,contextptr);
	// fake lexer required to initialize color syntax
	gen g("abs",contextptr);
	*logptr(contextptr) << "Xcas interpreter, Python compatible mode\n";
      }
#endif
      Bdisp_AllClr_VRAM();
#if defined GIAC_SHOWTIME || defined NSPIRE_NEWLIB
      Console_Output("Reglage de l'heure, exemple");
      Console_NewLine(LINE_TYPE_OUTPUT, 1);          
      Console_Output("12,37=>,");
      Console_NewLine(LINE_TYPE_OUTPUT, 1);
#endif
      //menu_about();
      return 0;
    }
    return 1;
  }

  string extract_name(const char * s){
    int l=strlen(s),i,j;
    for (i=l-1;i>=0;--i){
      if (s[i]=='.')
	break;
    }
    if (i<=0)
      return "f";
    for (j=i-1;j>=0;--j){
      if (s[j]=='\\')
	break;
    }
    if (j<0)
      return "f";
    return string(s+j+1).substr(0,i-j-1);
  }

  int giac_filebrowser(char * filename,const char * extension,const char * title){
    const char * filenames[MAX_NUMBER_OF_FILENAMES+1];
#if 1 // def XWASPY
    int n,choix;
    bool isxw=strcmp(extension,"xw")==0,ispy=strcmp(extension,"py")==0;
    if (isxw || ispy){
      n=os_file_browser(filenames,MAX_NUMBER_OF_FILENAMES,"py");
      if (n==0 && ispy) return 0;
      int N=0;
      // isxw: keep only filenames ending with _xw
      // ispy: remove filenames ending with _xw
      const char * fnames[MAX_NUMBER_OF_FILENAMES+1];
      for (int i=0;i<n;++i){
	const char * f=filenames[i];
	f+=strlen(f)-6;
	bool isfxw=strcmp(f,"_xw.py")==0;
	if (isxw?isfxw:!isfxw){
	  fnames[N]=filenames[i];
	  ++N;
	}
      }
      if (isxw){ // add regular .xw extensions
	n=os_file_browser(filenames,MAX_NUMBER_OF_FILENAMES,"xw");
	if (n+N>MAX_NUMBER_OF_FILENAMES)
	  n=MAX_NUMBER_OF_FILENAMES-N;
	for (int i=0;i<n;++i,++N){
	  fnames[N]=filenames[i];
	}
      }
      fnames[N]=0;
      choix=select_item(fnames,title?title:"Scripts");
      if (choix<0 || choix>=N) return 0;
      strcpy(filename,fnames[choix]);
      return choix+1;
    }
    else 
      choix=select_item(filenames,title?title:"Scripts");
#else
    int n=os_file_browser(filenames,MAX_NUMBER_OF_FILENAMES,extension);
    if (n==0) return 0;
    int choix=select_item(filenames,title?title:"Scripts");
#endif
    if (choix<0 || choix>=n) return 0;
    strcpy(filename,filenames[choix]);
    return choix+1;
  }
  
  void erase_script(){
    char filename[MAX_FILENAME_SIZE+1];
    int res=giac_filebrowser(filename, "py", "Scripts");
    if (res && do_confirm((lang==1)?"Vraiment effacer":"Really erase?")){
      erase_file(filename);
    }
  }

  int run_script(const char* filename,GIAC_CONTEXT) {
#if 0
    return 1;
#else
    string s;
    load_script(filename,s);
    // execution_in_progress = 1;
    run(s.c_str(),7,contextptr);
    // execution_in_progress = 0;
    if (s.size()>=4){
      if (s[0]=='#' || (s[0]=='d' && s[1]=='e' && s[2]=='f' && s[3]==' '))
	return 2;
      if ( (s[0]=='/' && s[1]=='/') ||
	   (s.size()>8 && s[0]=='f' && (s[1]=='o' || s[1]=='u') && s[2]=='n' && s[3]=='c' && s[4]=='t' && s[5]=='i' && s[6]=='o' && s[7]=='n' && s[8]==' ')
	   )
	return 3;
    }
    return 1;
#endif
  }

  int edit_script(char * fname,GIAC_CONTEXT){
    char fname_[MAX_FILENAME_SIZE+1];
    char * filename=0;
    int res=1;
    if (fname)
      filename=fname;
    else {
      res=giac_filebrowser(fname_, "py", "Scripts");
      filename=fname_;
    }
    if(res) {
      string s;
      load_script(filename,s);
      if (s.empty()){
	s=python_compat(contextptr)?((lang==1)?"Prog. Python, sinon taper":"Python prog., for Xcas"):((lang==1)?"Prog. Xcas, sinon taper":"Xcas prog., for Python");
	s += " AC F6 12";
	int k=confirm(s.c_str(),
#ifdef NSPIRE_NEWLIB
		      "enter: Prog, esc: Tortue"
#else
		      "OK: Prog, Back: Tortue"
#endif
		      );
	if (k==-1)
	  return 0;
	if (k==KEY_CTRL_F6)
	  s=python_compat(contextptr)?"from turtle import *\nreset()\n":"\nefface;\n ";
	else
	  s=python_compat(contextptr)?"def "+extract_name(filename)+"(x):\n  \n  return x":"function "+extract_name(filename)+"(x)\nlocal j;\n  \n  return x;\nffunction";
      }
      // split s at newlines
      if (edptr==0)
	edptr=new textArea;
      if (!edptr) return -1;
      edptr->elements.clear();
      edptr->clipline=-1;
      edptr->filename=filename;
      edptr->editable=true;
      edptr->changed=false;
      edptr->python=python_compat(contextptr);
      edptr->elements.clear();
      add(edptr,s);
      s.clear();
      edptr->line=0;
      //edptr->line=edptr->elements.size()-1;
      edptr->pos=0;
      int res=doTextArea(edptr,contextptr);
      if (res==KEY_SHUTDOWN)
	return res;
      if (res==-1)
	python_compat(edptr->python,contextptr);
      dConsolePutChar('\x1e');
    }
    return 0;
  }

  void chk_restart(GIAC_CONTEXT){
    drawRectangle(0, 24, LCD_WIDTH_PX, LCD_HEIGHT_PX-24, COLOR_WHITE);
    if (confirm((lang==1)?"Conserver les variables?":"Keep variables?",
#ifdef NSPIRE_NEWLIB
		(lang==1)?"enter: conserver, esc: effacer":"enter: keep, esc: erase"
#else
		(lang==1)?"OK: conserver, Back: effacer":"OK: keep, Back: erase"
#endif
		)==KEY_CTRL_F6)
      do_restart(contextptr);
  }

  void load(GIAC_CONTEXT){
    char filename[MAX_FILENAME_SIZE+1];
    if (giac_filebrowser(filename, "xw", "Sessions")){
      if (console_changed==0 ||
	  strcmp(session_filename,"session")==0 ||
	  confirm((lang==1)?"Session courante perdue?":"Current session will be lost",
#ifdef NSPIRE_NEWLIB
		  (lang==1)?"enter: ok, esc: annul":"enter: ok, esc: cancel"
#else
		  (lang==1)?"OK: ok, Back: annul":"OK: ok, Back: cancel"
#endif
		  )==KEY_CTRL_F1){
#ifndef NUMWORKS
	giac::_restart(giac::gen(giac::vecteur(0),giac::_SEQ__VECT),contextptr);
#endif
	restore_session(filename,contextptr);
	clip_pasted=true;
	strcpy(session_filename,remove_path(giac::remove_extension(filename)).c_str());
	static bool ctrl_r=true;
	if (ctrl_r){
#ifdef NSPIRE_NEWLIB
	  confirm((lang==1)?"Taper ctrl puis r pour executer session ":"Type ctrl then r to run session","Enter: OK");
#endif
#ifdef NUMWORKS
	  confirm((lang==1)?"Taper shift EXE pour executer session ":"Type shift then EXE to run session","Enter: OK");
#endif
	  ctrl_r=false;
	}
	Console_Disp(0,contextptr);
	// reload_edptr(session_filename,edptr);
      }     
    }
  }

  bool Console_tooltip(GIAC_CONTEXT){
    if (Current_Line==Last_Line && !Line[Current_Line].readonly && Current_Col>0){
      int y=(Current_Line>10?180:Current_Line*18);
      return tooltip(-1 /* means compute size before cursor*/,y,Cursor.x,Edit_Line,contextptr);
    }
    return false;
  }

  bool console_help_insert(int exec,GIAC_CONTEXT){
    if (!Edit_Line)
      return false;
    char buf[strlen(Edit_Line)+1];
    strcpy(buf,Edit_Line);
    buf[Cursor.x]=0;
    int back;
    string s=help_insert(buf,back,exec,contextptr);
    if (s.empty())
      return false;
    for (int i=0;i<back;++i)
      Console_Backspace(contextptr);
    Console_Input(s.c_str());
    Console_Disp(1,contextptr);
    return true;
  }

#ifdef NSPIRE_NEWLIB
  void check_nspire_exam_mode(GIAC_CONTEXT){
    refresh_osscr();
    if (nspire_exam_mode==2){
      // reset
      if (is_cx2)
	*(unsigned *) 0x90140020=8*16;
      else
	*(unsigned *) 0x900a0008=2;
    }
    if (nspire_exam_mode==1){
      set_exam_mode(3,contextptr); exam_mode=0;
    }
  }
#else
  void check_nspire_exam_mode(GIAC_CONTEXT){}
#endif

  int Console_GetKey(GIAC_CONTEXT){
    int key;
    bool keytooltip=false;
    unsigned int i, move_line, move_col;
    char tmp_str[2];
    char *tmp;
    for (;;){
      if (shutdown_state)
	return KEY_SHUTDOWN;
      int keyflag = GetSetupSetting(0x14);
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;
      if (keytooltip){
	keytooltip=false;
	if (key==KEY_CTRL_EXIT){
	  Console_Disp(1,contextptr);
	  continue;
	}
	if (Current_Line==Last_Line && Line[Current_Line].start_col+Cursor.x==strlen(Edit_Line) && (key==KEY_CTRL_OK || key==KEY_CHAR_ANS || key==KEY_CTRL_RIGHT)){
	  if (key==KEY_CTRL_RIGHT)
	    key=KEY_CTRL_OK;
	  if (console_help_insert(key,contextptr)){
	    Console_Disp(1,contextptr);
	    keytooltip=Console_tooltip(contextptr);
	    continue;
	  }
	}
	if (key==KEY_CTRL_VARS)
	  key=KEY_BOOK;	
      }
      bool alph=alphawasactive(&key);
      if (key==KEY_PRGM_ACON)
	Console_Disp(1,contextptr);
      translate_fkey(key);
      if (key==KEY_CTRL_PASTE)
	return Console_Input((const char*) paste_clipboard());
      if ( (key==KEY_CHAR_PLUS || key==KEY_CHAR_MINUS || key==KEY_CHAR_MULT || key==KEY_CHAR_DIV) && Current_Line<Last_Line-1){
	console_line * nxt=&Line[Current_Line];
	if (strncmp((const char *)nxt->str,"parameter([",11)==0)
	  Console_MoveCursor(CURSOR_UP);
	nxt=&Line[Current_Line+1];
	if (strncmp((const char *)nxt->str,"parameter([",11)==0){
	  giac::gen g((const char *)nxt->str,contextptr);
	  if (g.is_symb_of_sommet(giac::at_parameter)){
	    g=g._SYMBptr->feuille;
	    if (g.type==giac::_VECT && g._VECTptr->size()>=5){
	      giac::vecteur & v=*g._VECTptr;
	      for (int i=1;i<v.size();++i)
		v[i]=evalf_double(v[i],1,contextptr);
	      if (v[0].type==giac::_IDNT && v[1].type==giac::_DOUBLE_ && v[2].type==giac::_DOUBLE_ && v[3].type==giac::_DOUBLE_ && v[4].type==giac::_DOUBLE_){
		std::string s("assume(");
		s += v[0]._IDNTptr->id_name;
		s += "=[";
		int val=1;
		if (key==KEY_CHAR_MINUS) val=-1;
		if (key==KEY_CHAR_MULT) val=5;
		if (key==KEY_CHAR_DIV) val=-5;
		s += giac::print_DOUBLE_(v[3]._DOUBLE_val + val*v[4]._DOUBLE_val,contextptr);
		s += ',';
		s += giac::print_DOUBLE_(v[1]._DOUBLE_val,contextptr);
		s += ',';
		s += giac::print_DOUBLE_(v[2]._DOUBLE_val,contextptr);
		s += ',';
		s += giac::print_DOUBLE_(v[4]._DOUBLE_val,contextptr);
		s += "])";
		return Console_Eval(s.c_str(),contextptr);
	      }
	    }
	  }
	}
      }
      if (key==KEY_CHAR_ACCOLADES || key==KEY_CHAR_CROCHETS){
	Console_Input(key==KEY_CHAR_ACCOLADES?"{}":"[]");
	Console_MoveCursor(CURSOR_LEFT);
	Console_Disp(1,contextptr);
	continue;	
      }
      if ( (key >= ' ' && key <= '~' )
	   // (key>='0' && key<='9')|| (key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z')
	   ){
	tmp_str[0] = key;
	tmp_str[1] = '\0';
	Console_Input(tmp_str);
	Console_Disp(1,contextptr);
	// tooltip
	keytooltip=Console_tooltip(contextptr);
	continue;
      }
      if (key == KEY_CTRL_F5 || key==KEY_EQW_TEMPLATE || key==KEY_CTRL_F4 || ( (key==KEY_CTRL_RIGHT || key==KEY_CTRL_LEFT) && Current_Line<Last_Line) ){
	int l=Current_Line;
	bool graph=strcmp((const char *)Line[l].str,"Graphic object")==0;
	if (graph && l>0) --l;
	char buf[giacmax(512,strlen((const char *)Line[l].str+1))];
	strcpy(buf,(const char *)Line[l].str);
	int ret=(alph || key==KEY_CTRL_RIGHT || key==KEY_CTRL_F4) ?textedit(buf,512,false,contextptr):eqws(buf,graph,contextptr);
	if (ret==KEY_SHUTDOWN)
	  return ret;
	if (ret){
	  if (Current_Line==Last_Line){
	    Console_Clear_EditLine();
	    return Console_Input((const char *)buf);
	  }
	  else {
#if 1
	    if (Line[l].type==LINE_TYPE_INPUT && l<Last_Line-1 && Line[l+1].type==LINE_TYPE_OUTPUT)
	      return Console_Eval(buf,contextptr);
	    else {
	      console_free(Line[l].str);
	      Line[l].str=(char*)console_malloc(strlen(buf)+1);
	      Line[l].disp_len = Console_GetDispLen(Line[l].str);
	      strcpy((char *)Line[l].str,buf);
	    }
#else
	    int x=editline_cursor;
	    move_line = Last_Line - Current_Line;
	    for (i = 0; i <=move_line; i++) Console_MoveCursor(CURSOR_DOWN);
	    Cursor.x=x;
	    return Console_Input((const char *)buf);
#endif
	  }	  
	}
	Console_Disp(1,contextptr);
	continue;
      }
      if (0 &&key==KEY_CTRL_F6){
	char buf[512];
	if (!showCatalog(buf,0,0))
	  buf[0]=0;
	return Console_Input((const char*)buf);
      }
      if (key==KEY_CTRL_S || key==KEY_CTRL_T){
	giac::gen g=sheet(contextptr);
	if (g.type==_INT_ && g.val==KEY_SHUTDOWN)
	  return KEY_SHUTDOWN;
	if (g.type==_VECT)
	  return Console_Input(g.print(contextptr).c_str());
	Console_Disp(1,contextptr);
	continue;
      }
      if (key==KEY_SAVE){
	save(session_filename,contextptr);
	console_changed=false;
	console_disp_status(contextptr);
	continue;
      }
      if (key==KEY_LOAD){
	load(contextptr);
	Console_Disp(1,contextptr);
	continue;
      }
      if (key==KEY_CTRL_MENU){
#if 1
	Menu smallmenu;
	smallmenu.numitems=17;
	MenuItem smallmenuitems[smallmenu.numitems];
      
	smallmenu.items=smallmenuitems;
	smallmenu.height=12;
	smallmenu.scrollbar=1;
	smallmenu.scrollout=1;
	//smallmenu.title = "KhiCAS";
	// smallmenuitems[2].text = (char*)(isRecording ? "Stop Recording" : "Record Script");
	while(1) {
	  // moved inside the loop because lang might change
	  smallmenuitems[0].text = (char*)"Applications (shift ANS)";
	  string sess=(lang==1)?"Enregistrer ":"Save ";
	  sess += session_filename;
	  smallmenuitems[1].text = (char *) (sess.c_str());
	  smallmenuitems[2].text = (char *) ((lang==1)?"Enregistrer sous":"Save session as");
	  if (nspire_exam_mode==2) smallmenuitems[1].text = (char*)(lang==1?"Sauvegarde desactivee":"Saving disabled");
	  if (exam_mode || nspire_exam_mode==2)
	    smallmenuitems[2].text = (char *) "";
	  smallmenuitems[3].text = (char*) ((lang==1)?"Charger session":"Load session");
	  smallmenuitems[4].text = (char*)((lang==1)?"Nouvelle session":"New session");
	  smallmenuitems[5].text = (char*)((lang==1)?"Executer session":"Run session");
	  smallmenuitems[6].text = (char*)((lang==1)?"Editeur script":"Script editor");
	  smallmenuitems[7].text = (char*)((lang==1)?"Ouvrir script":"Open script");
	  smallmenuitems[8].text = (char*)((lang==1)?"Executer script":"Run script");
	  smallmenuitems[9].text = (char*)((lang==1)?"Effacer historique (0)":"Clear history");
	  smallmenuitems[10].text = (char*)((lang==1)?"Effacer script (e^)":"Clear script");
	  smallmenuitems[11].text = (char*)"Configuration/examen (ln)";
	  smallmenuitems[12].text = (char *) ((lang==1)?"Aide interface (log)":"Shortcuts");
	  smallmenuitems[13].text = (char*)((lang==1)?"Editer matrice (i)":"Matrix editor");
	  smallmenuitems[14].text = (char*) ((lang==1)?"Creer parametre (,)":"Create slider (,)");
	  smallmenuitems[15].text = (char*) ((lang==1)?"A propos (x^y)":"About");
#ifdef NSPIRE_NEWLIB
	  smallmenuitems[16].text = (char*) ((lang==1)?"Quitter (menu)":"Quit");
#else
	  smallmenuitems[16].text = (char*) ((lang==1)?"Quitter (HOME)":"Quit");
#endif
	  if (exam_mode)
	    smallmenuitems[16].text = (char*)((lang==1)?"Quitter le mode examen":"Quit exam mode");
	  if (nspire_exam_mode==2)
	    smallmenuitems[16].text = (char*)((lang==1)?"Relancer le mode examen":"Restart exam mode");
	  if (shutdown_state)
	    return KEY_SHUTDOWN;
	  int sres = doMenu(&smallmenu);
	  if(sres == MENU_RETURN_SELECTION || sres==KEY_CTRL_EXE) {
	    if (smallmenu.selection==smallmenu.numitems){
	      if (nspire_exam_mode==2)
		check_nspire_exam_mode(contextptr);
	      if (!exam_mode)
		return KEY_CTRL_MENU;
	      leave_exam_mode(contextptr);
	      break;
	    }
	    const char * ptr=0;
	    if (smallmenu.selection==1){
	      key=KEY_SHIFT_ANS;
	      break;
	    }
	    if (smallmenu.selection==2){
	      if (strcmp(session_filename,"session")==0)
		smallmenu.selection=3;
	      else {
		save(session_filename,contextptr);
		break;
	      }
	    }
	    if (smallmenu.selection==3 && !exam_mode && nspire_exam_mode!=2){
	      char buf[270];
	      if (get_filename(buf,".xw")){
		save(buf,contextptr);
		string fname(remove_path(giac::remove_extension(buf)));
		strcpy(session_filename,fname.c_str());
		if (edptr)
		  edptr->filename=fname+".py";
	      }
	      break;
	    }
	    if (smallmenu.selection==4){
	      load(contextptr);
	      break;
	    }
	    if (0 && smallmenu.selection==5) {
	      // FIXME: make a menu catalog?
	      char buf[512];
	      if (doCatalogMenu(buf,(char*)"CATALOG",0,contextptr))
		return Console_Input((const char *)buf);
	      break;
	    }
	    if (smallmenu.selection==5) {
	      if (exam_mode){
		if (do_confirm((lang==1)?"Tout effacer?":"Really clear?")){
		  Console_Init(contextptr);
		  Console_Clear_EditLine();
		  giac::_restart(giac::gen(giac::vecteur(0),giac::_SEQ__VECT),contextptr);
		}
	      }
	      else {
		char filename[MAX_FILENAME_SIZE+1];
		drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, COLOR_WHITE);
		if (get_filename(filename,".xw")){
		  if (console_changed==0 ||
		      strcmp(session_filename,"session")==0 ||
		      confirm((lang==1)?"Session courante perdue?":"Current session will be lost",
#ifdef NSPIRE_NEWLIB
			      (lang==1)?"enter: annul, esc: ok":"enter: cancel, esc: ok"
#else
			      (lang==1)?"OK: annul, Back: ok":"OK: cancel, Back: ok"
#endif
			      )==KEY_CTRL_F6){
		    clip_pasted=true;
		    Console_Init(contextptr);
		    Console_Clear_EditLine();
		    giac::_restart(giac::gen(giac::vecteur(0),giac::_SEQ__VECT),contextptr);
		    std::string s(remove_path(giac::remove_extension(filename)));
		    strcpy(session_filename,s.c_str());
		    reload_edptr(session_filename,edptr,contextptr);
		  }
		}
	      }
	      break;
	    }
	    if (smallmenu.selection==6) {
	      run_session(0,contextptr);
	      break;
	    }
	    if (smallmenu.selection==7) {
	      if (!edptr || merge_area(edptr->elements).size()<2)
		edit_script((char *)(giac::remove_extension(session_filename)+".py").c_str(),contextptr);
	      else
		doTextArea(edptr,contextptr);
	      break;
	    }
	    if (smallmenu.selection==8) {
	      char filename[MAX_FILENAME_SIZE+1];
	      drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX-8, COLOR_WHITE);
	      if (giac_filebrowser(filename, "py", "Scripts"))
		edit_script(filename,contextptr);
	      break;
	    }
	    if (smallmenu.selection==9) {
	      char filename[MAX_FILENAME_SIZE+1];
	      drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX-8, COLOR_WHITE);
	      if (giac_filebrowser(filename, "py", "Scripts"))
		run_script(filename,contextptr);
	      Console_Clear_EditLine();
	      break;
	    }
	    if(smallmenu.selection == 10) {
	      chk_restart(contextptr);
	      Console_Init(contextptr);
	      Console_Clear_EditLine();
	      break;
	    }
	    if (smallmenu.selection==11){
	      erase_script();
	      break;
	    }
	    if (smallmenu.selection == 12){
	      menu_setup(contextptr);
	      continue;
	    }
	    if(smallmenu.selection == 13 ||smallmenu.selection == 16 ) {
	      textArea text;
	      text.editable=false;
	      text.clipline=-1;
	      text.title = smallmenuitems[smallmenu.selection-1].text;
	      add(&text,smallmenu.selection==13?((lang==1)?shortcuts_fr_string:shortcuts_en_string):((lang==1)?apropos_fr_string:apropos_en_string));
	      doTextArea(&text,contextptr);
	      continue;
	    } 
	    if (smallmenu.selection==14){
	      drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX-8, COLOR_WHITE);
	      if (ptr=input_matrix(false,contextptr)) {
		return Console_Input((const char *)ptr);
	      }
	      break;
	    }
	    if (smallmenu.selection == 15){
	      Menu paramenu;
	      paramenu.numitems=6;
	      MenuItem paramenuitems[paramenu.numitems];
	      paramenu.items=paramenuitems;
	      paramenu.height=12;
	      paramenu.title = (char *)"Parameter";
	      char menu_xcur[32],menu_xmin[32],menu_xmax[32],menu_xstep[32],menu_name[16]="name a";
	      static char curname='a';
	      menu_name[5]=curname;
	      ++curname;
	      double pcur=0,pmin=-5,pmax=5,pstep=0.1;
	      std::string s;
	      bool doit;
	      for (;;){
		s="cur "+giac::print_DOUBLE_(pcur,contextptr);
		strcpy(menu_xcur,s.c_str());
		s="min "+giac::print_DOUBLE_(pmin,contextptr);
		strcpy(menu_xmin,s.c_str());
		s="max "+giac::print_DOUBLE_(pmax,contextptr);
		strcpy(menu_xmax,s.c_str());
		s="step "+giac::print_DOUBLE_(pstep,contextptr);
		strcpy(menu_xstep,s.c_str());
		paramenuitems[0].text = (char *) "OK";
		paramenuitems[1].text = (char *) menu_name;
		paramenuitems[2].text = (char *) menu_xcur;
		paramenuitems[3].text = (char *) menu_xmin;
		paramenuitems[4].text = (char *) menu_xmax;
		paramenuitems[5].text = (char *) menu_xstep;
		int sres = doMenu(&paramenu);
		doit = sres==MENU_RETURN_SELECTION  || sres==KEY_CTRL_EXE;
		if (doit) {
		  std::string s1; double d;
		  if (paramenu.selection==2){
		    handle_f5();
		    if (inputline(menu_name,(lang==1)?"Nouvelle valeur?":"New value?",s1,false)==KEY_CTRL_EXE && s1.size()>0 && isalpha(s1[0])){
		      if (s1.size()>10)
			s1=s1.substr(0,10);
		      strcpy(menu_name,("name "+s1).c_str());
		    }
		    continue;
		  }	
		  if (paramenu.selection==3){
		    inputdouble(menu_xcur,pcur,contextptr);
		    continue;
		  }
		  if (paramenu.selection==4){
		    inputdouble(menu_xmin,pmin,contextptr);
		    continue;
		  }
		  if (paramenu.selection==5){
		    inputdouble(menu_xmax,pmax,contextptr);
		    continue;
		  }
		  if (paramenu.selection==6){
		    inputdouble(menu_xstep,pstep,contextptr);
		    pstep=fabs(pstep);
		    continue;
		  }
		  // if (paramenu.selection==6) break;
		} // end menu
		break;
	      } // end for (;;)
	      if (doit && pmin<pmax && pstep>0){
		s="assume(";
		s += (menu_name+5);
		s += "=[";
		s += (menu_xcur+4);
		s += ',';
		s += (menu_xmin+4);
		s += ',';
		s += (menu_xmax+4);
		s += ',';
		s += (menu_xstep+5);
		s += "])";
		return Console_Input((const char *)s.c_str());
	      }
	      continue;
	    }
	  }
	  break;
	} // end while(1)
	if (key!=KEY_SHIFT_ANS){
	  Console_Disp(1,contextptr);
	  return CONSOLE_SUCCEEDED;
	}
#else
	char filename[MAX_FILENAME_SIZE+1];
	//drawRectangle(0, 24, LCD_WIDTH_PX, LCD_HEIGHT_PX-24, COLOR_WHITE);
	if (get_filename(filename))
	  edit_script(filename,contextptr);
	//edit_script(0);
	return CONSOLE_SUCCEEDED;
#endif
      }
      if (key==KEY_SHIFT_ANS){ // 3rd party app
	int res=khicas_addins_menu(contextptr);
	if (res==KEY_CTRL_MENU)
	  return res;
	Console_Disp(1,contextptr);
	return CONSOLE_SUCCEEDED;
      }
      if ( (key >= KEY_CTRL_F1 && key <= KEY_CTRL_F6) ||
	   (key >= KEY_CTRL_F7 && key <= KEY_CTRL_F14) 
	   ){
	Console_FMenu(key,contextptr);
	Console_Disp(1,contextptr);
	keytooltip=Console_tooltip(contextptr);
	continue;
      }
      if (key==KEY_CTRL_PAGEDOWN){
	int j=0;
	for (int i=0;i<10;++i)
	  j=Console_MoveCursor(CURSOR_DOWN);
	return j;
      }
      if (key==KEY_CTRL_PAGEUP){
	int j=0;
	for (int i=0;i<10;++i)
	  j=Console_MoveCursor(CURSOR_UP);
	return j;
      }
      if (key == KEY_CTRL_UP)
	return Console_MoveCursor(alph?CURSOR_ALPHA_UP:CURSOR_UP);
      if (key == KEY_CTRL_DOWN || key=='\t'
	  // FIREBIRDEMU
	  || key==KEY_BOOK
	  ){
	if (Current_Line==Last_Line && !Line[Current_Line].readonly && Current_Col>0){
	  console_help_insert(0,contextptr);
	  Console_Disp(1,contextptr);	  
	  continue;
	}
	return Console_MoveCursor(alph?CURSOR_ALPHA_DOWN:CURSOR_DOWN);
      }
      //if (key == KEY_CTRL_PAGEUP)  return Console_MoveCursor(CURSOR_ALPHA_UP);
      //if (key == KEY_CTRL_PAGEDOWN) return Console_MoveCursor(CURSOR_ALPHA_DOWN);
      if (key == KEY_CTRL_LEFT)
	Console_MoveCursor(CURSOR_LEFT);
      if (key == KEY_CTRL_RIGHT)
	Console_MoveCursor(CURSOR_RIGHT);
      if (key == KEY_SHIFT_LEFT)
	Console_MoveCursor(CURSOR_SHIFT_LEFT);
      if (key == KEY_SHIFT_RIGHT)
	Console_MoveCursor(CURSOR_SHIFT_RIGHT);
      if (key == KEY_SHIFT_RIGHT || key == KEY_SHIFT_LEFT ||
	  key == KEY_CTRL_RIGHT || key == KEY_CTRL_LEFT){
	Console_Disp(0,contextptr);
	continue;
      }
      if (key == KEY_CTRL_EXIT){
	if (Last_Line==Current_Line){
	  if (!edptr)
	    edit_script((char *)(giac::remove_extension(session_filename)+".py").c_str(),contextptr);
	  else {
	    edptr->y=0;
	    doTextArea(edptr,contextptr);
	  }
	  Console_Disp(1,contextptr);
	}
	else {
	  move_line = Last_Line - Current_Line;
	  for (i = 0; i <= move_line; i++) Console_MoveCursor(CURSOR_DOWN);
	}
	return CONSOLE_SUCCEEDED;
      }
      if (key == KEY_CTRL_AC)
	{
	  if (Line[Current_Line].readonly){
	    move_line = Last_Line - Current_Line;
	    for (i = 0; i <= move_line; i++) Console_MoveCursor(CURSOR_DOWN);
	    return CONSOLE_SUCCEEDED;
	  }
	  if (Edit_Line[0]=='\0'){
	    //return Console_Input((const char *)"restart");
	    chk_clearscreen(contextptr);
	    continue;
	  }
	  Edit_Line[0] = '\0';
	  Line[Current_Line].start_col = 0;
	  Line[Current_Line].type = LINE_TYPE_INPUT;
	  Line[Current_Line].disp_len = 0;
	  Cursor.x = 0;
	  return CONSOLE_SUCCEEDED;
	}

      if (key == KEY_CTRL_INS) {
	if (Current_Line<Last_Line){
	  Console_Insert_Line();
	  Console_Insert_Line();
	}
	else
	  Console_Input((const char*)":=");
	Console_Disp(1,contextptr);
	continue;
      }
      if (key==KEY_AFFECT){
	Console_Input((const char*)":=");
	Console_Disp(1,contextptr);
	continue;
      }	
      if (key==KEY_CTRL_D){
	Console_Input((const char*)"debug(");
	Console_Disp(1,contextptr);
	continue;
      }	
      if (key == KEY_CTRL_SETUP) {
	menu_setup(contextptr);
	Console_Disp(1,contextptr);
	continue;
      }

      if (key == KEY_CTRL_EXE || key==KEY_CTRL_OK){
	if (Current_Line == Last_Line)
	  {
	    return Console_NewLine(LINE_TYPE_INPUT, 1);
	  }
      }
      if (key == KEY_CTRL_DEL){
	Console_Backspace(contextptr);
	Console_Disp(1,contextptr);
	keytooltip=Console_tooltip(contextptr);
	continue;	
      }
      if (key == KEY_CTRL_R && (Current_Line!=Last_Line || Cursor.x==0)){
	run_session(0,contextptr);
	return 0;
      }
      if (key == KEY_CTRL_CLIP){
	copy_clipboard((const char *)Line[Current_Line].str,true);
      }
      if (key==KEY_CTRL_EXE || key==KEY_CTRL_OK){
	tmp = Line[Current_Line].str;
      
#if 1
	int x=editline_cursor;
	move_line = Last_Line - Current_Line;
	for (i = 0; i <= move_line; i++) Console_MoveCursor(CURSOR_DOWN);
	Cursor.x=x;
	if (Cursor.x>COL_DISP_MAX)
	  Line[Last_Line].start_col=Cursor.x-COL_DISP_MAX;
#else
	move_line = Last_Line - Current_Line;
	for (i = 0; i <= move_line; i++) Console_MoveCursor(CURSOR_DOWN);
	move_col = Line[Current_Line].disp_len - Current_Col;
	for (i = 0; i <= move_col; i++) Console_MoveCursor(CURSOR_RIGHT);
#endif
	return Console_Input(tmp);
      }
      const char * ptr=keytostring(key,keyflag,0,contextptr);
      if (ptr){
	Console_Input((const char *)ptr);
	Console_Disp(1,contextptr);
	keytooltip=Console_tooltip(contextptr);
	continue;
      }
    
    }
    return CONSOLE_NO_EVENT;
  }

  int Console_FMenu(int key,GIAC_CONTEXT){
    const char * s=console_menu(key,fmenu_cfg,0),*ptr=0;
    if (!s){
      //cout << "console " << unsigned(s) << endl;
      return CONSOLE_NO_EVENT;
    }
    if (strcmp("matrix(",s)==0 && (ptr=input_matrix(false,contextptr)) )
      s=ptr;
    if (strcmp("makelist(",s)==0 && (ptr=input_matrix(true,contextptr)) )
      s=ptr;
    return Console_Input((const char *)s);
  }

  const char * console_menu(int key,int active_app){
    return console_menu(key,fmenu_cfg,active_app);
  }

  const char * console_menu(int key,char* cfg_,int active_app){
    char * cfg=cfg_;
    int i, matched = 0;
    const char * ret=0;
    const int maxentry_size=64;
    static char console_buf[maxentry_size];
    char temp[maxentry_size],menu1[maxentry_size],menu2[maxentry_size],menu3[maxentry_size],menu4[maxentry_size],menu5[maxentry_size],menu6[maxentry_size],menu7[maxentry_size],menu8[maxentry_size];
    char * tabmenu[8]={menu1,menu2,menu3,menu4,menu5,menu6,menu7,menu8};
    struct FMenu entry = {0,tabmenu,0};
    // char* cfg = (char *)memory_load((char *)"\\\\fls0\\FMENU.cfg");

    while (*cfg) {
      //Get each line
      for(i=0; i<maxentry_size-1 && *cfg && *cfg!='\r' && *cfg!='\n'; i++, cfg++) {
	temp[i] = *cfg;
      }
      temp[i]=0;
      //If starting by 'F' followed by the right number, start filling the structure.
      if (temp[0] == 'F' && temp[1]==(key-KEY_CTRL_F1)+'1'){
	matched = 1;
	continue;
      }
      if (temp[0] == 'F' && temp[1]!=(key-KEY_CTRL_F1)+'0'){
	matched = 0;
	continue;
      }
      //Fill the structure
      if (matched && temp[0] && entry.count<8) {
	strcpy(tabmenu[entry.count], temp);
	entry.count++;
      }
      cfg++;
    }
    if(entry.count > 0) {
      ret = Console_Draw_FMenu(key, &entry,cfg,active_app);
      // cout << "console0 " << (unsigned) ret << endl;
      if (!ret) return ret;
      if (!strcmp("periodic_table",ret)){
	const char * name,*symbol;
	char protons[32],nucleons[32],mass[32],electroneg[32];
	int res=periodic_table(name,symbol,protons,nucleons,mass,electroneg);
	if (!res)
	  return 0;
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
	}
	if (res & 32){
	  if (res&31)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,electroneg+4)+strlen(ptr);
	}
      }
      else
	strcpy(console_buf,ret);
      return console_buf;
    }
    return 0;
  }

  char *Console_Make_Entry(const char* str)
  {
    char* entry = NULL;
    entry = (char*)calloc((strlen((const char *)str)+1), sizeof(char*));
    if(entry) memcpy(entry, (const char *)str, strlen((const char *)str)+1);

    return entry;
  }

  void PrintMini(int x,int y,const char * s,int mode){
    x *=3;
    y *=3;
    PrintMini(x,y,(char *)s,mode,COLOR_BLACK, COLOR_WHITE);
  }

  //Draws and runs the asked for menu.
  const char * Console_Draw_FMenu(int key, struct FMenu* menu,char * cfg,int active_app)
  {
    int i, nb_entries = 0, selector = 0, position_number, position_x, ret, longest = 0;
    int input_key;
    char quick[] = "*: ";
    int quick_len = 2;
    char **entries;
    DISPBOX box,box3;
    
    position_number = key - KEY_CTRL_F1;
    if (position_number<0 || position_number>=5)
      position_number=4;
    
    entries  = menu->str;
    nb_entries = menu->count;
    
    for(i=0; i<nb_entries; i++)
      if(strlen(entries[i]) > longest) longest = strlen(entries[i]);
    if (longest>15)
      longest=15;
    // screen resolution Graph90 384x(216-24), Graph35 128x64
    // factor 3x3
    position_x = 17*position_number;
    if(position_x + longest*4 + quick_len*4 > 115) position_x = 115 - longest*4 - quick_len*4;
    
    box.left = position_x;
    box.right = position_x + longest*4 + quick_len*4  + 6;
    box.bottom = 63-7;
    box.top = 63-7-nb_entries*7;
    box3.left=3*box.left;
    box3.right=3*box.right;
    box3.bottom=3*box.bottom+22;
    box3.top=3*box.top+20;
  
    drawRectangle(box3.left,box3.top,box3.right-box3.left,box3.bottom-box3.top,COLOR_WHITE);
    drawLine(box3.left, box3.top, box3.right, box3.top,COLOR_BLACK);
    drawLine(box3.left, box3.bottom, box3.left, box3.top,COLOR_BLACK);
    drawLine(box3.right, box3.bottom, box3.right, box3.top,COLOR_BLACK);
    drawLine(box3.left, box3.bottom, box3.right, box3.bottom,COLOR_BLACK);
    
    // Cursor_SetFlashOff();
    
    for (;;){
      for(i=0; i<nb_entries; i++) {
	quick[0] = '0'+(i+1);
	PrintMini(3+position_x, box.bottom-7*i, quick, 0);
	PrintMini(3+position_x+quick_len*4, box.bottom-7*i, entries[i], 0);
      }
      PrintMini(3+position_x+quick_len*4,box.bottom-7*selector, entries[selector], 4);
      GetKey(&input_key);
      if (input_key==KEY_PRGM_ACON) Console_Disp(1,0);
      if (input_key == KEY_CTRL_EXIT || input_key==KEY_CTRL_AC) return 0;
      if (input_key == KEY_CTRL_UP && selector < nb_entries-1) selector++;	
      if (input_key == KEY_CTRL_DOWN && selector > 0) selector--;
      
      if (input_key == KEY_CTRL_EXE || input_key==KEY_CTRL_OK) return entries[selector];
      
      if (input_key >= KEY_CHAR_1 && input_key < KEY_CHAR_1 + nb_entries) return entries[input_key-KEY_CHAR_1];
      
      translate_fkey(input_key);
      
      if ( active_app==0 &&
	   ((input_key >= KEY_CTRL_F1 && input_key <= KEY_CTRL_F6) ||
	    (input_key >= KEY_CTRL_F7 && input_key <= KEY_CTRL_F12) )
	   ){
	Console_Disp(1,0);
	key=input_key;
	return console_menu(key,cfg,active_app);
      }
    } // end while input_key!=EXE/EXIT

    return 0; // never reached
  }

  void Console_Free(){
    for (int i = 0; i < _LINE_MAX; i++){
      if (Line[i].str){
	if (Line[i].str==Edit_Line)
	  Edit_Line=0;
	console_free(Line[i].str);
	Line[i].str=0;
      }
    }
    if (Edit_Line)
      console_free(Edit_Line);
    if (Line){
      delete [] Line;
      Line = 0;
    }
  }

  int Console_Init(GIAC_CONTEXT){
    console_changed=1;
    int i;
    if (!Line){
      Line=new console_line[_LINE_MAX];
      for (i = 0; i < _LINE_MAX; i++){
	Line[i].str=0;
      }
    }
    Start_Line = 0;
    Last_Line = 0;

    for (i = 0; i < _LINE_MAX; i++){
      if (Line[i].str){
	if (Line[i].str==Edit_Line)
	  Edit_Line=0;
	console_free(Line[i].str);
	Line[i].str=0;
      }
      Line[i].readonly = 0;
      Line[i].type = LINE_TYPE_INPUT;
      Line[i].start_col = 0;
      Line[i].disp_len = 0;
    }
    if (Edit_Line)
      console_free(Edit_Line);
    if ((Edit_Line = (char *)console_malloc(EDIT_LINE_MAX + 1)) == NULL) return CONSOLE_MEM_ERR;
    Edit_Line[0]=0;
    Line[0].str = Edit_Line;

    Cursor.x = 0;
    Cursor.y = 0;

    Case = LOWER_CASE;

    /*for(i = 0; i < 6; i++) {
      FMenu_entries[i].name = NULL;
      FMenu_entries[i].count = 0;
      }*/

    Console_FMenu_Init(contextptr);

    return CONSOLE_SUCCEEDED;
  }

  // Loads the FMenus' data into memory, from a cfg file
  void Console_FMenu_Init(GIAC_CONTEXT)
  {
    char temp[32] = {'\0'};
#if 0
    if (!fmenu_cfg){
      fmenu_cfg = (char *)conf_standard;
      std::string cfg_s;
      // Does the file exists ?
      if (load_script((char*)"FMENU.cfg",cfg_s)){
	char * ptr=new char[cfg_s.size()+1];
	strcpy(ptr,cfg_s.c_str());
	fmenu_cfg=(char *)ptr;
      }
      if(!fmenu_cfg) {
	save_script((const char *)"FMENU.cfg",conf_standard);
	fmenu_cfg = (char *)conf_standard;
      }
    }
#else
    if (xcas_python_eval==1){
      fmenu_cfg=(char *)python_conf_standard;
    }
    else {
      fmenu_cfg=(char *)conf_standard;
    }
#endif
    const char *cfg=fmenu_cfg;
    while(*cfg) {
      //Get each line
      int i;
      for(i=0; i<20 && *cfg && *cfg!='\r' && *cfg!='\n'; i++, cfg++) {
	temp[i] = *cfg;
      }
      temp[i]=0;
      //If starting by 'F', adjust the number and eventually set the name of the menu
      if(temp[0] == 'F' && temp[1]>='1' && temp[1]<='6') {
	int number = temp[1]-'0' - 1;
	if(temp[3] && number<6) {
	  strcpy(FMenu_entries_name[number], (char*)temp+3);
	  //FMenu_entries[number].name[4] = '\0';
	}
      }

      memset(temp, '\0', 20);
      cfg++;
    }
    //free(fmenu_cfg);
  }

  /*
    The following functions are used to display all lines.
    Note: After calling this function, the first clear the memory.
  */

#ifndef CURSOR
  int print_x=0,print_y=0,vfontsize=18,hfontsize=12;
#endif

  void locate(int x,int y){
#ifdef CURSOR
    return locate_OS(x,y);
#else
    print_x=(x-1)*hfontsize;
    print_y=(y-1)*vfontsize;
#endif
  }

  void Cursor_SetPosition(int x,int y){
    return locate(x+1,y+1);
  }

  int print_color(int print_x,int print_y,const char *s,int color,bool invert,bool minimini,GIAC_CONTEXT){
    int python=python_compat(contextptr);
    const char * src=s;
    char singleword[128];
    bool linecomment=false;
    int couleur=color;
    while (*src && print_y<LCD_WIDTH_PX){
      const char * oldsrc=src;
      if ( (python && *src=='#') ||
	   (!python && *src=='/' && *(src+1)=='/')){
	linecomment=true;
	couleur=4;
      }
      if (linecomment)
	src = (char*)toksplit((unsigned char*)src, ' ', (unsigned char*)singleword, minimini?50:35); //break into words; next word
      else { // skip string (only with delimiters " ")
	if (*src=='"'){
	  for (++src;*src;++src){
	    if (*src=='"' && *(src-1)!='\\')
	      break;
	  }
	  if (*src=='"')
	    ++src;
	  int i=src-oldsrc;
	  strncpy(singleword,oldsrc,i);
	  singleword[i]=0;
	}
	else {
	  size_t i=0;
	  for (;*src==' ';++src){ // skip initial whitespaces
	    ++i;
	  }
	  if (i==0){
	    if (isalpha(*src)){ // skip keyword
	      for (;isalphanum(*src) || *src=='_';++src){
		++i;
	      }
	    }
	    // go to next space or alphabetic char
	    for (;*src;++i,++src){
	      if (*src==' ' || (i && *src>=' ' && *src<='/') || (python && *src=='#') || (!python && *src=='/' && *(src+1)=='/')|| *src=='"' || isalpha(*src))
		break;
	    }
	  }
	  strncpy(singleword,oldsrc,i);
	  singleword[i]=0;
	  if (i==0){
	    puts(src); // free(singleword);
	    return print_x; // FIXME KEY_CTRL_F2;
	  }
	} // end normal case
      } // end else linecomment case
      couleur=linecomment?5:find_color(singleword,contextptr);
      if (couleur==1) couleur=COLOR_BLUE;
      if (couleur==2) couleur=COLOR_YELLOWDARK;
      if (couleur==3) couleur=51712;//33024;
      if (couleur==4) couleur=COLOR_MAGENTA;
      if (couleur==5) couleur=COLOR_GREEN;
      if (linecomment || singleword[0]=='"')
	print(print_x,print_y,singleword,couleur,invert,/*fake*/false,minimini);
      else { // print two parts, commandname in color and remain in black
	char * ptr=singleword;
	if (isalpha(*ptr)){
	  while (isalphanum(*ptr) || *ptr=='_')
	    ++ptr;
	}
	char ch=*ptr;
	*ptr=0;
	print(print_x,print_y,singleword,couleur,invert,/*fake*/false,minimini);
	*ptr=ch;
	print(print_x,print_y,ptr,COLOR_BLACK,invert,/*fake*/false,minimini);
      }
      // ?add a space removed from token
      if( linecomment?*src:*src==' ' ){
	if (*src==' ')
	  ++src;
	print(print_x,print_y," ",COLOR_BLACK,invert,false,minimini);
      }
    }
    return print_x;
  }    

  void print_color(const char *s,int color,bool invert,bool minimini,GIAC_CONTEXT){
    print_x=print_color(print_x,print_y,s,color,invert,minimini,contextptr);
  }

  void PrintRev(const char * s,int color,bool colorsyntax,GIAC_CONTEXT){
#ifdef CURSOR
    Print_OS((char *)s,TEXT_MODE_INVERT,0);
#else
    print(print_x,print_y,(const char *)s,color,true/* revert*/,false,false);
#endif  
  }

  void Print(const char * s,int color,bool colorsyntax,GIAC_CONTEXT){
#ifdef CURSOR
    Print_OS((char *)s,TEXT_MODE_NORMAL,0);
#else
    if (!colorsyntax || (strlen(s)==1 && (s[0]=='>' || s[0]=='<')))
      print(print_x,print_y,(const char *)s,color,false,false,false);
    else
      print_color(s,color,false,false,contextptr);
#endif
  }

  // redraw_mode=1 clear area
  int Console_Disp(int redraw_mode,GIAC_CONTEXT){
    bool minimini=false;
    unsigned int* pBitmap;
    int i, alpha_shift_status;
    DISPBOX ficon;
    int print_y = 0; //pixel y cursor
    int print_y_locate;

    // if (redraw_mode & 1) Bdisp_AllClr_VRAM();

    //GetFKeyIconPointer( 0x01BE, &ficon );
    //DisplayFKeyIcon( i, ficon);

    //Reading each "line" that will be printed
    for (i = 0; (i < LINE_DISP_MAX) && (i + Start_Line <= Last_Line); i++){
      console_line & curline=Line[i+Start_Line];
      bool colorsyntax=curline.type == LINE_TYPE_INPUT;
      if (i == Cursor.y){
	// cursor line
	//if ((redraw_mode & 1)==0)
	  drawRectangle(0,i*vfontsize,LCD_WIDTH_PX,vfontsize,_WHITE);
	if (curline.type == LINE_TYPE_INPUT || curline.type == LINE_TYPE_OUTPUT && curline.disp_len >= COL_DISP_MAX){
	  locate(1, i + 1);
	  if (curline.readonly){
#ifdef CURSOR
	    Cursor_SetFlashOff();
#endif
	    PrintRev(curline.str + curline.start_col,TEXT_COLOR_BLACK,colorsyntax,contextptr);
	  }
	  else 
	    Print(curline.str+curline.start_col+(Cursor.x>COL_DISP_MAX-1?1:0),TEXT_COLOR_BLACK,colorsyntax,contextptr);
	}
	else {
	  locate(1, i + 1);
	  print(print_x,print_y,(const char *)curline.str,TEXT_COLOR_BLACK,false,true/*fake*/,minimini); // fake print
	  print_x=LCD_WIDTH_PX-print_x;
	  if (curline.readonly){
#ifdef CURSOR
	    Cursor_SetFlashOff();
#endif
	    PrintRev(curline.str,TEXT_COLOR_BLACK,colorsyntax,contextptr);
	  }
	  else 
	    Print(curline.str,TEXT_COLOR_BLACK,colorsyntax,contextptr);
	}

	if (
#if 1 //def CURSOR
	    curline.disp_len - curline.start_col > COL_DISP_MAX-1
#else
	    print_x>LCD_WIDTH_PX-hfontsize
#endif
	    ){
#ifdef CURSOR
	  locate(COL_DISP_MAX, i + 1);
#else
	  print_y=i*vfontsize;
	  print_x=LCD_WIDTH_PX+2-hfontsize;
#endif
	  if (curline.readonly){
	    if(curline.disp_len - curline.start_col != COL_DISP_MAX) {
#ifdef CURSOR
	      Cursor_SetFlashOff();
#endif
	      PrintRev((char *)">",COLOR_MAGENTA,colorsyntax,contextptr);
	    }
	  }
	  else if (Cursor.x < COL_DISP_MAX-1){
	    Print((char *)">",COLOR_MAGENTA,colorsyntax,contextptr);
	  }
	}

	if (curline.start_col > 0){
	  locate(1, i + 1);	
	  if (curline.readonly){
#ifdef CURSOR
	    Cursor_SetFlashOff();
#endif		  
	    PrintRev((char *)"<",COLOR_MAGENTA,colorsyntax,contextptr);
	  }
	  else {
	    Print((char *)"<",COLOR_MAGENTA,colorsyntax,contextptr);
	  }
	}

	if (!curline.readonly){
	  int fakestart=curline.start_col+(Cursor.x > COL_DISP_MAX-1?1:0);
	  int fakex,fakey=Cursor.y*vfontsize;
	  string fakes;
	  // parenthese match
	  const char * str=curline.str;
	  int pos=Cursor.x+fakestart,pos2;
	  int l=strlen(str);
	  char ch=0;
	  if (pos<l)
	    ch=str[pos];
	  int matchdirection=0,paren=0,crochet=0,accolade=0;
	  if (ch=='(' || ch=='[' || ch=='{')
	    matchdirection=1;
	  if (ch=='}' || ch==']' || ch==')')
	    matchdirection=-1;
	  if (!matchdirection && pos){
	    --pos;
	    ch=str[pos];
	    if (ch=='(' || ch=='[' || ch=='{')
	      matchdirection=1;
	    if (ch=='}' || ch==']' || ch==')')
	      matchdirection=-1;
	  }
	  if (matchdirection){
	    char buf[2]={0,0};
	    bool ok=true;
	    for (pos2=pos;ok && (pos2>=0 && pos2<l);pos2+=matchdirection){
	      ch=str[pos2];
	      if (ch=='(') ++paren;
	      if (ch==')') --paren;
	      if (ch=='[') ++crochet;
	      if (ch==']') --crochet;
	      if (ch=='{') ++accolade;
	      if (ch=='}') --accolade;
	      if (matchdirection>0 && (paren<0 || crochet<0 || accolade<0) )
		ok=false;
	      if (matchdirection<0 && (paren>0 || crochet>0 || accolade>0) )
		ok=false;
	      if (paren==0 && crochet==0 && accolade==0)
		break;
	    }
	    ok = paren==0 && crochet==0 && accolade==0;
	    if (pos>=fakestart){
	      fakex=0;
	      buf[0]=str[pos];
	      fakes=string((const char *)curline.str).substr(fakestart,pos-fakestart);
	      print(fakex,fakey,fakes.c_str(),TEXT_COLOR_BLACK,false,true/* fake*/,minimini); // fake print
	      print(fakex,fakey,buf,ok?TEXT_COLOR_GREEN:TEXT_COLOR_RED,true/* revert*/,false,minimini);
	    }
	    if (ok){
	      fakex=0;
	      if (pos2>fakestart){
		fakes=string((const char *)curline.str).substr(fakestart,pos2-fakestart);
		print(fakex,fakey,fakes.c_str(),TEXT_COLOR_BLACK,false,true/* fake*/,false); // fake print
		buf[0]=str[pos2];
		print(fakex,fakey,buf,TEXT_COLOR_GREEN,true/* revert*/,false,minimini);
	      }
	    }
	  }
#ifdef CURSOR
	  switch(GetSetupSetting( (unsigned int)0x14)) {
	  case 0: 
	    alpha_shift_status = 0;
	    break;
	  case 1: //Shift enabled
	    alpha_shift_status = 1;
	    break;
	  case 4: case 0x84:	//Alpha enabled
	    alpha_shift_status = 2;
	    break;
	  case 8: case 0x88:
	    alpha_shift_status = 4;
	    break;
	  default: 
	    alpha_shift_status = 0;
	    break;
	  }
	  Cursor_SetPosition(Cursor.x, Cursor.y);
	  Cursor_SetFlashOn(alpha_shift_status);
	  //Cursor_SetFlashStyle(alpha_shift_status); //Potential 2.00 OS incompatibilty (cf Simon's doc)
#else
	  //locate(Cursor.x+1,Cursor.y+1);
	  //DefineStatusMessage((giac::print_DOUBLE_(Cursor.y,6)+","+giac::print_DOUBLE_(print_y,6)).c_str(),1,0,0);
	  //DisplayStatusArea();
	  fakes=string((const char *)curline.str).substr(fakestart,Cursor.x);
	  fakex=0;
	  print(fakex,fakey,fakes.c_str(),TEXT_COLOR_BLACK,false,true/* fake*/,minimini); // fake print
	  drawRectangle(fakex,fakey,2,vfontsize,COLOR_BLACK);
	  //drawRectangle(Cursor.x*hfontsize,24+Cursor.y*vfontsize,2,vfontsize,COLOR_BLACK);
#endif
	}
      } // end cursor line
      else {
	if ((redraw_mode & 1)==0)
	  continue;
	drawRectangle(0,i*vfontsize,LCD_WIDTH_PX,vfontsize,_WHITE);
	bool bigoutput = curline.type==LINE_TYPE_OUTPUT && curline.disp_len>=COL_DISP_MAX-3;
	locate(bigoutput?3:1,i+1);
	if (curline.type==LINE_TYPE_INPUT || bigoutput)
	  Print(curline.str + curline.start_col,TEXT_COLOR_BLACK,colorsyntax,contextptr);
	else {
#ifdef CURSOR
	  locate(COL_DISP_MAX - Line[i + Start_Line].disp_len + 1, i + 1);
#else
	  print(print_x,print_y,(const char *)curline.str,TEXT_COLOR_BLACK,false,true/*fake*/,minimini);
	  print_x=LCD_WIDTH_PX-print_x;
#endif
	  Print(curline.str,TEXT_COLOR_BLACK,colorsyntax,contextptr);
	}
	if (curline.disp_len - curline.start_col > COL_DISP_MAX){
#ifdef CURSOR
	  locate(COL_DISP_MAX, i + 1);
#else
	  print_x=LCD_WIDTH_PX+2-hfontsize;
#endif
	  Print((char *)">",COLOR_BLUE,colorsyntax,contextptr);
	}
	if (curline.start_col > 0){
#ifdef CURSOR
	  locate(1, i + 1);
#else
	  print_x=0;
#endif
	  Print((char *)"<",COLOR_BLUE,colorsyntax,contextptr);
	}      
      } // end non cursor line
    } // end loop on all lines
    drawRectangle(0,i*vfontsize,LCD_WIDTH_PX,205-i*vfontsize,_WHITE);

    if ((redraw_mode & 1)==1){
      for (; (i < LINE_DISP_MAX) ; i++)
	drawRectangle(0,i*vfontsize,LCD_WIDTH_PX,vfontsize,_WHITE);
#if 0 // def NUMWORKS
      string menu("shift-Ans help|1 ");
#else
      string menu("shift-1 ");
#endif
      menu += string(menu_f1);
      menu += "|2 ";
      menu += string(menu_f2);
      menu += "|3 ";
      menu += string(menu_f3);
      menu += xcas_python_eval==1?"|4 edt|5 2d|6 logo|7 lin|8 matr|9arit|0 plt":"|4 edt|5 2d|6 regr|7 matr|8 cplx|9 arit|0 rand";
      int xcas_color=65055,python_color=52832;
      int interp_color=xcas_python_eval?python_color:xcas_color;
      drawRectangle(0,205,LCD_WIDTH_PX,17,interp_color);
      PrintMiniMini(0,205,menu.c_str(),0,giac::_BLACK,interp_color);
    }
  
    // status, clock,
    console_disp_status(contextptr);
    return CONSOLE_SUCCEEDED;
  }

  void dConsoleRedraw(){
    Console_Disp(1,0);
  }

  char *Console_GetLine(GIAC_CONTEXT)
  {
    int return_val;
	
    do
      {
	return_val = Console_GetKey(contextptr);
	if (return_val==KEY_SHUTDOWN)
	  return 0;
	Console_Disp(1,contextptr);
	if (return_val == KEY_CTRL_MENU) return 0;
	if (return_val == CONSOLE_MEM_ERR) return NULL;
      } while (return_val != CONSOLE_NEW_LINE_SET);

    return Line[Current_Line - 1].str;
  }

  /*
    Simple accessor to the Edit_Line buffer.
  */
  char* Console_GetEditLine()
  {
    return Edit_Line;
  }

  void save_session(GIAC_CONTEXT){
    if (nspire_exam_mode==2)
      return;
    if (strcmp(session_filename,"session") && console_changed){
      string tmp(session_filename);
      tmp += (lang==1)?" a ete modifie!":" was modified!";
      if (confirm(tmp.c_str(),
#ifdef NSPIRE_NEWLIB
		  (lang==1)?"enter: sauve, esc: tant pis":"enter: save, esc: discard changes"
#else
		  (lang==1)?"OK: sauve, Back: tant pis":"OK: save, Back: discard changes"
#endif
		  )==KEY_CTRL_F1){
	save(session_filename,contextptr);
	console_changed=0;
      }    
    }
    save("session",contextptr);
    // this is only called on exit, no need to reinstall the check_execution_abort timer.
    if (edptr && edptr->changed && edptr->filename!="session.py"){
      if (!check_leave(edptr)){
	save_script("lastprg.py",merge_area(edptr->elements));
      }
    }
  }

#ifdef NSPIRE_NEWLIB
  bool nspire_fr(){
    char16_t input_w[] = u"getLangInfo()";
    void *math_expr = nullptr;
    int str_offset = 0;
    
    int error = TI_MS_evaluateExpr_ACBER(NULL, NULL, (const uint16_t*)input_w, &math_expr, &str_offset);
    if (error)
      return false;
    
    char16_t *output_w;
    error = TI_MS_MathExprToStr(math_expr, NULL, (uint16_t**)&output_w);
    syscall<e_free, void>(math_expr); // Should be TI_MS_DeleteMathExpr
    
    if (error)
      return false;
    int l=0;
    for (l=0;l<64;++l){
      if (output_w[l]==0)
	break;
    }
    bool b=l==4 && output_w[1]=='f' && output_w[2]=='r';
    // Do something with output_w, it's u"42." here
    
    syscall<e_free, void>(output_w);
    return b;
  }
#endif

  tableur * sheetptr=0;
#ifdef NUMWORKS
  extern "C" void mp_stack_ctrl_init();
  extern "C" void mp_stack_set_top(void *);
  extern "C" void mp_stack_set_limit(size_t);
#endif

  int console_main(GIAC_CONTEXT,const char * sessionname){
#ifdef NUMWORKS
    mp_stack_ctrl_init();
    //volatile int stackTop;
    //mp_stack_set_top((void *)(&stackTop));
    //mp_stack_set_limit(24*1024);
#endif
    giac::micropy_ptr=micropy_ck_eval;
    python_heap=0;
    sheetptr=0;
    shutdown=do_shutdown;
#ifdef NSPIRE_NEWLIB
    // detect if leds are blinking
    unsigned green=*(unsigned *) 0x90110b04;
    unsigned red=*(unsigned *) 0x90110b0c;
    if (green || red){
      nspire_exam_mode=1;
      if (1 || is_cx2){
	if (!do_confirm(lang?"Le CAS est-il autorise en examen?":"Is CAS allowed during exam?"))
	  return 0;
      }
    }
    // CX and CX II we should modify the led colors to match CAS exam mode
    // red value should be the same as green value -> yellow
    // try to detect emulator or real calc
    unsigned NSPIRE_SPEED=0x900B0000;
    unsigned speed=*(unsigned *)NSPIRE_SPEED;
    nspireemu= (speed==1445890);
    mkdir("Xcas",0755);
    //mkdir("/Xcas",0755);
    //mkdir("A:/Xcas",0755);
    //mkdir("A:\\Xcas",0755);
    int err=chdir("Xcas");
    if (err)
      err=chdir("ndless");
    bool b=nspire_fr();
    lang=b?1:0;
#endif
    // SetQuitHandler(save_session); // automatically save session when exiting
    if (!turtleptr){
      turtle();
      _efface_logo(vecteur(0),contextptr);
    }
    caseval("floor"); // init xcas parser for Python syntax coloration (!)
    int key;
    Console_Init(contextptr);
    Bdisp_AllClr_VRAM();
    rand_seed(millis(),contextptr);
    restore_session(sessionname,contextptr);
    giac::angle_radian(os_get_angle_unit()==0,contextptr);
    //GetKey(&key);
    Console_Disp(1,contextptr);
    if (nspire_exam_mode){ // must save LED state for restoration at end
      set_exam_mode(2,contextptr); exam_mode=0;
    }
    // GetKey(&key);
    char *expr=0;
#ifndef NO_STDEXCEPT
    try {
#endif    
    while(1){
      if ((expr=Console_GetLine(contextptr))==NULL){
	save_session(contextptr);
#ifdef NUMWORKS
	return 0;
#endif
	check_nspire_exam_mode(contextptr);
#ifdef MICROPY_LIB
	python_free();
#endif
	Console_Free();
	release_globals();
	if (sheetptr){
	  // sheetptr->m.clear();
	  delete sheetptr;
	  sheetptr=0;
	}
	return 0;
      }
      if (strcmp((const char *)expr,"restart")==0){
	if (confirm((lang==1)?"Effacer variables?":"Clear variables?",
#ifdef NSPIRE_NEWLIB
		    (lang==1)?"enter: confirmer,  esc: annuler":"enter: confirm,  esc: cancel"
#else
		    (lang==1)?"OK: confirmer,  Back: annuler":"OK: confirm,  Back: cancel"
#endif
		    )!=KEY_CTRL_F1){
	  Console_Output(" cancelled");
	  Console_NewLine(LINE_TYPE_OUTPUT,1);
	  //GetKey(&key);
	  Console_Disp(1,contextptr);
	  continue;
	}
      }
      // should save in another file
      if (strcmp((const char *)expr,"=>")==0 || strcmp((const char *)expr,"=>\n")==0){
	save_session(contextptr);
	Console_Output("Session saved");
      }
      else 
	run(expr,7,contextptr);
      //print_mem_info();
      Console_NewLine(LINE_TYPE_OUTPUT,1);
      //GetKey(&key);
      Console_Disp(1,contextptr);
    }
#ifndef NO_STDEXCEPT
    } catch(autoshutdown & e) {
    }
#endif    
#ifdef NUMWORKS
    return 0;
#endif
    check_nspire_exam_mode(contextptr);
    Console_Free();
    release_globals();
#ifdef MICROPY_LIB
    python_free();
#endif
    if (sheetptr){
      // sheetptr->m.clear();
      delete sheetptr;
      sheetptr=0;
    }
    return 0;
  }


#endif // TEXTAREA

  int rgb24to16(int c){
    int r=(c>>16)&0xff,g=(c>>8)&0xff,b=c&0xff;
    return (((r*32)/256)<<11) | (((g*64)/256)<<5) | (b*32/256);
  }

  // table periodique, code adapte de https://github.com/M4xi1m3/nw-atom
  // avec l'aimable autorisation de diffusion sous licence GPL de Maxime Friess
  // https://tiplanet.org/forum/viewtopic.php?f=97&t=23094&p=247471#p247471
enum AtomType {
  ALKALI_METAL,
  ALKALI_EARTH_METAL,
  LANTHANIDE,
  ACTINIDE,
  TRANSITION_METAL,
  POST_TRANSITION_METAL,
  METALLOID,
  HALOGEN,
  REACTIVE_NONMETAL,
  NOBLE_GAS,
  UNKNOWN
};

struct AtomDef {
  uint8_t num;
  uint8_t x;
  uint8_t y;
  AtomType type;
  const char* name;
  const char* symbol;
  uint8_t neutrons;
  double mass;
  double electroneg;
};

const AtomDef atomsdefs[] = {
  {  1,  0,  0, REACTIVE_NONMETAL       , "Hydrogen"     , "H"   ,   0, 1.00784     , 2.2   },
  {  2, 17,  0, NOBLE_GAS               , "Helium"       , "He"  ,   2, 4.002602    , -1    },
  
  
  {  3,  0,  1, ALKALI_METAL            , "Lithium"      , "Li"  ,   4, 6.938       , 0.98  },
  {  4,  1,  1, ALKALI_EARTH_METAL      , "Beryllium"    , "Be"  ,   5, 9.012182    , 1.57  },
  {  5, 12,  1, METALLOID               , "Boron"        , "B"   ,   6, 10.806      , 2.04  },
  {  6, 13,  1, REACTIVE_NONMETAL       , "Carbon"       , "C"   ,   6, 12.0096     , 2.55  },
  {  7, 14,  1, REACTIVE_NONMETAL       , "Nitrogen"     , "N"   ,   7, 14.00643    , 3.04  },
  {  8, 15,  1, REACTIVE_NONMETAL       , "Oxygen"       , "O"   ,   8, 15.99903    , 3.44  },
  {  9, 16,  1, HALOGEN                 , "Fluorine"     , "F"   ,  10, 18.9984032  , 3.98  },
  { 10, 17,  1, NOBLE_GAS               , "Neon"         , "Ne"  ,  10, 20.1797     , -1    },
  
  
  { 11,  0,  2, ALKALI_METAL            , "Sodium"       , "Na"  ,  12, 22.9897693  , 0.93  },
  { 12,  1,  2, ALKALI_EARTH_METAL      , "Magnesium"    , "Mg"  ,  12, 24.3050     , 1.31  },
  { 13, 12,  2, POST_TRANSITION_METAL   , "Aluminium"    , "Al"  ,  14, 26.9815386  , 1.61  },
  { 14, 13,  2, METALLOID               , "Silicon"      , "Si"  ,  14, 28.084      , 1.9   },
  { 15, 14,  2, REACTIVE_NONMETAL       , "Phosphorus"   , "P"   ,  16, 30.973762   , 2.19  },
  { 16, 15,  2, REACTIVE_NONMETAL       , "Sulfur"       , "S"   ,  16, 32.059      , 2.58  },
  { 17, 16,  2, HALOGEN                 , "Chlorine"     , "Cl"  ,  18, 35.446      , 3.16  },
  { 18, 17,  2, NOBLE_GAS               , "Argon"        , "Ar"  ,  22, 39.948      , -1    },
  
  
  { 19,  0,  3, ALKALI_METAL            , "Potassium"    , "K"   ,  20, 39.0983     , 0.82  },
  { 20,  1,  3, ALKALI_EARTH_METAL      , "Calcium"      , "Ca"  ,  20, 40.078      , 1     },
  { 21,  2,  3, TRANSITION_METAL        , "Scandium"     , "Sc"  ,  24, 44.955912   , 1.36  },
  { 22,  3,  3, TRANSITION_METAL        , "Titanium"     , "Ti"  ,  26, 47.867      , 1.54  },
  { 23,  4,  3, TRANSITION_METAL        , "Vanadium"     , "V"   ,  28, 50.9415     , 1.63  },
  { 24,  5,  3, TRANSITION_METAL        , "Chromium"     , "Cr"  ,  28, 51.9961     , 1.66  },
  { 25,  6,  3, TRANSITION_METAL        , "Manganese"    , "Mn"  ,  30, 54.938045   , 1.55  },
  { 26,  7,  3, TRANSITION_METAL        , "Iron"         , "Fe"  ,  30, 55.845      , 1.83  },
  { 27,  8,  3, TRANSITION_METAL        , "Cobalt"       , "Co"  ,  32, 58.933195   , 1.88  },
  { 28,  9,  3, TRANSITION_METAL        , "Nickel"       , "Ni"  ,  30, 58.6934     , 1.91  },
  { 29, 10,  3, TRANSITION_METAL        , "Copper"       , "Cu"  ,  34, 63.546      , 1.9   },
  { 30, 11,  3, POST_TRANSITION_METAL   , "Zinc"         , "Zn"  ,  34, 65.38       , 1.65  },
  { 31, 12,  3, POST_TRANSITION_METAL   , "Gallium"      , "Ga"  ,  38, 69.723      , 1.81  },
  { 32, 13,  3, METALLOID               , "Germanium"    , "Ge"  ,  42, 72.63       , 2.01  },
  { 33, 14,  3, METALLOID               , "Arsenic"      , "As"  ,  42, 74.92160    , 2.18  },
  { 34, 15,  3, REACTIVE_NONMETAL       , "Selenium"     , "Se"  ,  46, 78.96       , 2.55  },
  { 35, 16,  3, HALOGEN                 , "Bromine"      , "Br"  ,  44, 79.904      , 2.96  },
  { 36, 17,  3, NOBLE_GAS               , "Krypton"      , "Kr"  ,  48, 83.798      , -1    },
  
  { 37,  0,  4, ALKALI_METAL            , "Rubidium"     , "Rb"  ,  48, 85.4678     , 0.82  },
  { 38,  1,  4, ALKALI_EARTH_METAL      , "Strontium"    , "Sr"  ,  50, 87.62       , 0.95  },
  { 39,  2,  4, TRANSITION_METAL        , "Yttrium"      , "Y"   ,  50, 88.90585    , 1.22  },
  { 40,  3,  4, TRANSITION_METAL        , "Zirconium"    , "Zr"  ,  50, 91.224      , 1.33  },
  { 41,  4,  4, TRANSITION_METAL        , "Niobium"      , "Nb"  ,  52, 92.90638    , 1.6   },
  { 42,  5,  4, TRANSITION_METAL        , "Molybdenum"   , "Mo"  ,  56, 95.96       , 2.16  },
  { 43,  6,  4, TRANSITION_METAL        , "Technetium"   , "Tc"  ,  55, 98          , 2.10  },
  { 44,  7,  4, TRANSITION_METAL        , "Ruthemium"    , "Ru"  ,  58, 101.07      , 2.2   },
  { 45,  8,  4, TRANSITION_METAL        , "Rhodium"      , "Rh"  ,  58, 102.90550   , 2.28  },
  { 46,  9,  4, TRANSITION_METAL        , "Palladium"    , "Pd"  ,  60, 106.42      , 2.20  },
  { 47, 10,  4, TRANSITION_METAL        , "Silver"       , "Ag"  ,  60, 107.8682    , 1.93  },
  { 48, 11,  4, POST_TRANSITION_METAL   , "Cadmium"      , "Cd"  ,  66, 112.411     , 1.69  },
  { 49, 12,  4, POST_TRANSITION_METAL   , "Indium"       , "In"  ,  66, 114.818     , 1.78  },
  { 50, 13,  4, POST_TRANSITION_METAL   , "Tin"          , "Sn"  ,  70, 118.710     , 1.96  },
  { 51, 14,  4, METALLOID               , "Antimony"     , "Sb"  ,  70, 121.760     , 2.05  },
  { 52, 15,  4, METALLOID               , "Tellurium"    , "Te"  ,  78, 127.60      , 2.1   },
  { 53, 16,  4, HALOGEN                 , "Indine"       , "I"   ,  74, 126.90447   , 2.66  },
  { 54, 17,  4, NOBLE_GAS               , "Xenon"        , "Xe"  ,  78, 131.293     , 2.60  },
  
  
  { 55,  0,  5, ALKALI_METAL            , "Caesium"      , "Cs"  ,  78, 132.905452  , 0.79  },
  { 56,  1,  5, ALKALI_EARTH_METAL      , "Barium"       , "Ba"  ,  82, 137.327     , 0.89  },

  { 57,  3,  7, LANTHANIDE              , "Lanthanum"    , "La"  ,  82, 138.90547   , 1.10  },
  { 58,  4,  7, LANTHANIDE              , "Cerium"       , "Ce"  ,  82, 140.116     , 1.12  },
  { 59,  5,  7, LANTHANIDE              , "Praseodymium" , "Pr"  ,  82, 140.90765   , 1.13  },
  { 60,  6,  7, LANTHANIDE              , "Neodymium"    , "Nd"  ,  82, 144.242     , 1.14  },
  { 61,  7,  7, LANTHANIDE              , "Promethium"   , "Pm"  ,  84, 145         , 1.13  },
  { 62,  8,  7, LANTHANIDE              , "Samarium"     , "Sm"  ,  90, 150.36      , 1.17  },
  { 63,  9,  7, LANTHANIDE              , "Europium"     , "Eu"  ,  90, 151.964     , 1.12  },
  { 64, 10,  7, LANTHANIDE              , "Gadolinium"   , "Gd"  ,  94, 157.25      , 1.20  },
  { 65, 11,  7, LANTHANIDE              , "Terbium"      , "Tb"  ,  94, 158.92535   , 1.12  },
  { 66, 12,  7, LANTHANIDE              , "Dyxprosium"   , "Dy"  ,  98, 162.500     , 1.22  },
  { 67, 13,  7, LANTHANIDE              , "Holmium"      , "Ho"  ,  98, 164.93032   , 1.23  },
  { 68, 14,  7, LANTHANIDE              , "Erbium"       , "Er"  ,  98, 167.259     , 1.24  },
  { 69, 15,  7, LANTHANIDE              , "Thulium"      , "Tm"  , 100, 168.93421   , 1.25  },
  { 70, 16,  7, LANTHANIDE              , "Ytterbium"    , "Yb"  , 104, 173.054     , 1.1   },
  { 71, 17,  7, LANTHANIDE              , "Lutetium"     , "Lu"  , 104, 174.9668    , 1.0   },

  { 72,  3,  5, TRANSITION_METAL        , "Hafnium"      , "Hf"  , 108, 178.49      , 1.3   },
  { 73,  4,  5, TRANSITION_METAL        , "Tantalum"     , "Ta"  , 108, 180.94788   , 1.5   },
  { 74,  5,  5, TRANSITION_METAL        , "Tungsten"     , "W"   , 110, 183.84      , 1.7   },
  { 75,  6,  5, TRANSITION_METAL        , "Rhenium"      , "Re"  , 112, 186.207     , 1.9   },
  { 76,  7,  5, TRANSITION_METAL        , "Osmium"       , "Os"  , 116, 190.23      , 2.2   },
  { 77,  8,  5, TRANSITION_METAL        , "Iridium"      , "Ir"  , 116, 192.217     , 2.2   },
  { 78,  9,  5, TRANSITION_METAL        , "Platinum"     , "Pt"  , 117, 195.084     , 2.2   },
  { 79, 10,  5, TRANSITION_METAL        , "Gold"         , "Au"  , 118, 196.966569  , 2.4   },
  { 80, 11,  5, POST_TRANSITION_METAL   , "Mercury"      , "Hg"  , 122, 200.59      , 1.9   },
  { 81, 12,  5, POST_TRANSITION_METAL   , "Thalium"      , "Tl"  , 124, 204.382     , 1.8   },
  { 82, 13,  5, POST_TRANSITION_METAL   , "Lead"         , "Pb"  , 126, 207.2       , 1.8   },
  { 83, 14,  5, POST_TRANSITION_METAL   , "Bismuth"      , "Bi"  , 126, 208.98040   , 1.9   },
  { 84, 15,  5, POST_TRANSITION_METAL   , "Polonium"     , "Po"  , 126, 209         , 2.0   },
  { 85, 16,  5, HALOGEN                 , "Astatine"     , "At"  , 125, 210         , 2.2   },
  { 86, 17,  5, NOBLE_GAS               , "Radon"        , "Rn"  , 136, 222         , 2.2   },
  
  
  { 87,  0,  6, ALKALI_METAL            , "Francium"     , "Fr"  , 136, 223         , 0.7   },
  { 88,  1,  6, ALKALI_EARTH_METAL      , "Radium"       , "Ra"  , 138, 226         , 0.9   },

  { 89,  3,  8, ACTINIDE                , "Actinium"     , "Ac"  , 138, 227         , 1.1   },
  { 90,  4,  8, ACTINIDE                , "Thorium"      , "Th"  , 142, 232.03806   , 1.3   },
  { 91,  5,  8, ACTINIDE                , "Protactinium" , "Pa"  , 140, 231.03588   , 1.5   },
  { 92,  6,  8, ACTINIDE                , "Uranium"      , "U"   , 146, 238.02891   , 1.38   },
  { 93,  7,  8, ACTINIDE                , "Neptunium"    , "Np"  , 144, 237         , 1.36   },
  { 94,  8,  8, ACTINIDE                , "Plutonium"    , "Pu"  , 150, 244         , 1.28   },
  { 95,  9,  8, ACTINIDE                , "Americium"    , "Am"  , 148, 243         , 1.13  },
  { 96, 10,  8, ACTINIDE                , "Curium"       , "Cm"  , 151, 247         , 1.28  },
  { 97, 11,  8, ACTINIDE                , "Berkellum"    , "Bk"  , 150, 247         , 1.3   },
  { 98, 12,  8, ACTINIDE                , "Californium"  , "Cf"  , 153, 251         , 1.3   },
  { 99, 13,  8, ACTINIDE                , "Einsteinium"  , "Es"  , 153, 252         , 1.3   },
  {100, 14,  8, ACTINIDE                , "Fermium"      , "Fm"  , 157, 257         , 1.3   },
  {101, 15,  8, ACTINIDE                , "Mendelevium"  , "Md"  , 157, 258         , 1.3   },
  {102, 16,  8, ACTINIDE                , "Nobelium"     , "No"  , 157, 259         , 1.3   },
  {103, 17,  8, ACTINIDE                , "Lawrencium"   , "Lr"  , 163, 262         , 1.3   },

  {104,  3,  6, TRANSITION_METAL        , "Rutherfordium", "Rf"  , 163, 261         , -1    },
  {105,  4,  6, TRANSITION_METAL        , "Dubnium"      , "Db"  , 163, 262         , -1    },
  {106,  5,  6, TRANSITION_METAL        , "Seaborgium"   , "Sg"  , 163, 263         , -1    },
  {107,  6,  6, TRANSITION_METAL        , "Bohrium"      , "Bh"  , 163, 264         , -1    },
  {108,  7,  6, TRANSITION_METAL        , "Hassium"      , "Hs"  , 169, 265         , -1    },
  {109,  8,  6, UNKNOWN                 , "Meitnerium"   , "Mt"  , 169, 268         , -1    },
  {110,  9,  6, UNKNOWN                 , "Damstadtium"  , "Ds"  , 171, 281         , -1    },
  {111, 10,  6, UNKNOWN                 , "Roentgenium"  , "Rg"  , 171, 273         , -1    },
  {112, 11,  6, POST_TRANSITION_METAL   , "Coppernicium" , "Cn"  , 173, 277         , -1    },
  {113, 12,  6, UNKNOWN                 , "Nihonium"     , "Nh"  , 173, 283         , -1    },
  {114, 13,  6, UNKNOWN                 , "Flerovium"    , "Fl"  , 175, 285         , -1    },
  {115, 14,  6, UNKNOWN                 , "Moscovium"    , "Mv"  , 174, 287         , -1    },
  {116, 15,  6, UNKNOWN                 , "Livermorium"  , "Lv"  , 177, 289         , -1    },
  {117, 16,  6, UNKNOWN                 , "Tennessine"   , "Ts"  , 177, 294         , -1    },
  {118, 17,  6, NOBLE_GAS               , "Oganesson"    , "Og"  , 176, 293         , -1    },
  
};
  
void drawAtom(uint8_t id) {
  int fill = rgb24to16(0xeeeeee);

  switch(atomsdefs[id].type) {
    case ALKALI_METAL:
      fill = rgb24to16(0xffaa00);
      break;
    case ALKALI_EARTH_METAL:
      fill = rgb24to16(0xf6f200);
      break;
    case LANTHANIDE:
      fill = rgb24to16(0xffaa8b);
      break;
    case ACTINIDE:
      fill = rgb24to16(0xdeaacd);
      break;
    case TRANSITION_METAL:
      fill = rgb24to16(0xde999c);
      break;
    case POST_TRANSITION_METAL:
      fill = rgb24to16(0x9cbaac);
      break;
    case METALLOID:
      fill = rgb24to16(0x52ce8b);
      break;
    case REACTIVE_NONMETAL:
      fill = rgb24to16(0x00ee00);
      break;
    case NOBLE_GAS:
      fill = rgb24to16(0x8baaff);
      break;
    case HALOGEN:
      fill = rgb24to16(0x00debd);
      break;
    default:
      break;
  }

  if (atomsdefs[id].y >= 7) {
    drawRectangle(6 + atomsdefs[id].x * 17, 15 + atomsdefs[id].y * 17, 18, 18, fill);
    stroke_rectangle(6 + atomsdefs[id].x * 17, 15 + atomsdefs[id].y * 17, 18, 18, rgb24to16(0x525552));
    os_draw_string_small(8 + atomsdefs[id].x * 17, 17 + atomsdefs[id].y * 17, _BLACK, fill, atomsdefs[id].symbol);
  } else {
    drawRectangle(6 + atomsdefs[id].x * 17, 6 + atomsdefs[id].y * 17, 18, 18, fill);
    stroke_rectangle(6 + atomsdefs[id].x * 17, 6 + atomsdefs[id].y * 17, 18, 18, rgb24to16(0x525552));
    os_draw_string_small(8 + atomsdefs[id].x * 17, 8 + atomsdefs[id].y * 17, _BLACK, fill, atomsdefs[id].symbol);
  }
}

  int periodic_table(const char * & name,const char * & symbol,char * protons,char * nucleons,char * mass,char * electroneg){
    bool partial_draw=false,redraw=true;
    int cursor_pos=0;
    const int ATOM_NUMS=sizeof(atomsdefs)/sizeof(AtomDef);
    for (;;){
      if (redraw){
	if (partial_draw) {
	  partial_draw = false;
	  drawRectangle(50, 0, 169, 57, _WHITE);
	  drawRectangle(0, 185, LCD_WIDTH_PX, 15, _WHITE);
	} else {
	  drawRectangle(0,0,LCD_WIDTH_PX,LCD_HEIGHT_PX,_WHITE);
	}
#ifdef NSPIRE_NEWLIB
	os_draw_string_small_(0,200,gettext("enter: tout, P:protons, N:nucleons, M:mass, E:khi"));
#else
	os_draw_string_small_(0,200,gettext("OK: tout, P:protons, N:nucleons, M:mass, E:khi"));
#endif
	for(int i = 0; i < ATOM_NUMS; i++) {
	  drawAtom(i);
	}
	if (atomsdefs[cursor_pos].y >= 7) {
	  stroke_rectangle(6 + atomsdefs[cursor_pos].x * 17, 15 + atomsdefs[cursor_pos].y * 17, 18, 18, 0x000000);
	  stroke_rectangle(7 + atomsdefs[cursor_pos].x * 17, 16 + atomsdefs[cursor_pos].y * 17, 16, 16, 0x000000);
	} else {
	  stroke_rectangle(6 + atomsdefs[cursor_pos].x * 17, 6 + atomsdefs[cursor_pos].y * 17, 18, 18, 0x000000);
	  stroke_rectangle(7 + atomsdefs[cursor_pos].x * 17, 7 + atomsdefs[cursor_pos].y * 17, 16, 16, 0x000000);
	}
  
	drawRectangle(48,  99, 2, 61,rgb24to16(0x525552));
	drawRectangle(48, 141, 9,  2, rgb24to16(0x525552));
	drawRectangle(48, 158, 9,  2, rgb24to16(0x525552));

	int prot=atomsdefs[cursor_pos].num;
	sprint_int(protons,prot);
	int nuc=atomsdefs[cursor_pos].neutrons+atomsdefs[cursor_pos].num;
	sprint_int(nucleons,nuc);
	
	symbol=atomsdefs[cursor_pos].symbol;
	os_draw_string_(73,23,symbol);
	name=atomsdefs[cursor_pos].name;
	os_draw_string_small_(110,27,gettext(name));
	os_draw_string_small_(50,18,nucleons);
	os_draw_string_small_(50,31,protons);
	strcpy(mass,"M:");
	strcpy(electroneg,"khi:");
	sprint_double(mass+2,atomsdefs[cursor_pos].mass);
	os_draw_string_small_(0,186,mass);
	sprint_double(electroneg+4,atomsdefs[cursor_pos].electroneg);
	os_draw_string_small_(160,186,electroneg);
      }
      redraw=false;
      int key;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;
      if (key==KEY_PRGM_ACON)
	redraw=true;
      if (key==KEY_CTRL_EXIT)
	return 0;
      if (key==KEY_CTRL_EXE || key==KEY_CTRL_OK)
	return 1|4|8|16|32;
      if (key=='s' || key==KEY_CHAR_5)
	return 2;
      if (key=='p' || key==KEY_CHAR_LPAR)
	return 4;
      if (key=='n' || key==KEY_CHAR_8)
	return 8;
      if (key=='m' || key==KEY_CHAR_7)
	return 16;
      if (key=='e' || key==KEY_CHAR_COMMA)
	return 32;
      if (key==KEY_CTRL_LEFT){
	if (cursor_pos>0)
	  --cursor_pos;
	redraw=partial_draw=true;
      }
      if (key==KEY_CTRL_RIGHT){
	if (cursor_pos< ATOM_NUMS-1)
	  ++cursor_pos;
	redraw=partial_draw=true;
      }
      if (key==KEY_CTRL_UP){
	uint8_t curr_x = atomsdefs[cursor_pos].x;
	uint8_t curr_y = atomsdefs[cursor_pos].y;
	bool updated = false;
	
	if (curr_y > 0 && curr_y <= 9) {
	  for(uint8_t i = 0; i < ATOM_NUMS; i++) {
	    if (atomsdefs[i].x == curr_x && atomsdefs[i].y == curr_y - 1) {
	      cursor_pos = i;
	      redraw=partial_draw = true;
	    }
	  }
	}
	
      }
      if (key==KEY_CTRL_DOWN){
	uint8_t curr_x = atomsdefs[cursor_pos].x;
	uint8_t curr_y = atomsdefs[cursor_pos].y;
	bool updated = false;
	
	if (curr_y >= 0 && curr_y < 9) {
	  for (uint8_t i = 0; i < ATOM_NUMS; i++) {
	    if (atomsdefs[i].x == curr_x && atomsdefs[i].y == curr_y + 1) {
	      cursor_pos = i;
	      redraw=partial_draw = true;
	      break;
	    }
	  }
	}
      }
    } // end endless for
  } // end periodic_table

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

void console_output(const char * s,int l){
  char buf[l+1];
  strncpy(buf,s,l);
  buf[l]=0;
  xcas::dConsolePut(buf);
}

const char * console_input(const char * msg1,const char * msg2,bool numeric,int ypos){
  static string str;
  if (!giac::inputline(msg1,msg2,str,numeric,ypos,context0))
    return 0;
  return str.c_str();
}

void c_draw_rectangle(int x,int y,int w,int h,int c){
  giac::freeze=true;
  xcas::draw_line(x,y,x+w,y,c);
  xcas::draw_line(x+w,y,x+w,y+h,c);
  xcas::draw_line(x,y+h,x+w,y+h,c);
  xcas::draw_line(x,y,x,y+h,c);
}
void c_draw_line(int x0,int y0,int x1,int y1,int c){
  giac::freeze=true;
  xcas::draw_line(x0,y0,x1,y1,c);
}
void c_draw_circle(int xc,int yc,int r,int color,bool q1,bool q2,bool q3,bool q4){
  giac::freeze=true;
  xcas::draw_circle(xc,yc,r,color,q1,q2,q3,q4);
}
void c_draw_filled_circle(int xc,int yc,int r,int color,bool left,bool right){
  giac::freeze=true;
  xcas::draw_filled_circle(xc,yc,r,color,left,right);
}
void c_convert(int *x,int*y,vector< vector<int> > & v){
  for (int i=0;i<v.size();++i,++x,++y){
    v[i].push_back(*x);
    v[i].push_back(*y);
  }
}
void c_draw_polygon(int * x,int *y ,int n,int color){
  giac::freeze=true;
  vector< vector<int> > v(n);
  c_convert(x,y,v);
  xcas::draw_polygon(v,color);
}
void c_draw_filled_polygon(int * x,int *y, int n,int xmin,int xmax,int ymin,int ymax,int color){
  giac::freeze=true;
  vector< vector<int> > v(n);
  c_convert(x,y,v);
  xcas::draw_filled_polygon(v,xmin,xmax,ymin,ymax,color);
}
void c_draw_arc(int xc,int yc,int rx,int ry,int color,double theta1, double theta2){
  giac::freeze=true;
  xcas::draw_arc(xc,yc,rx,ry,color,theta1,theta2);
}
void c_draw_filled_arc(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int color,int xmin,int xmax,int ymin,int ymax,bool segment){
  giac::freeze=true;
  xcas::draw_filled_arc(x,y,rx,ry,theta1_deg,theta2_deg,color,xmin,xmax,ymin,ymax,segment);
}
void c_set_pixel(int x,int y,int c){
  giac::freeze=true;
  os_set_pixel(x,y,c);
}
void c_fill_rect(int x,int y,int w,int h,int c){
  giac::freeze=true;
  os_fill_rect(x,y,w,h,c);
}
int c_draw_string(int x,int y,int c,int bg,const char * s,bool fake){
  giac::freeze=true;
  return os_draw_string(x,y,c,bg,s,fake);
}
int c_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake){
  giac::freeze=true;
  return os_draw_string_small(x,y,c,bg,s,fake);
}
int c_draw_string_medium(int x,int y,int c,int bg,const char * s,bool fake){
  giac::freeze=true;
  return os_draw_string_medium(x,y,c,bg,s,fake);
}

int select_item(const char ** ptr,const char * title,bool askfor1){
  int nitems=0;
  for (const char ** p=ptr;*p;++p)
    ++nitems;
  if (nitems==0 || nitems>=256)
    return -1;
  if (!askfor1 && nitems==1)
    return 0;
  MenuItem smallmenuitems[nitems];
  for (int i=0;i<nitems;++i){
    smallmenuitems[i].text=(char *) ptr[i];
  }
  Menu smallmenu;
  smallmenu.numitems=nitems; 
  smallmenu.items=smallmenuitems;
  smallmenu.height=12;
  smallmenu.scrollbar=1;
  smallmenu.scrollout=1;
  smallmenu.title = (char*) title;
  //MsgBoxPush(5);
  int sres = doMenu(&smallmenu);
  //MsgBoxPop();
  if (sres!=MENU_RETURN_SELECTION && sres!=KEY_CTRL_EXE)
    return -1;
  return smallmenu.selection-1;
}

int select_interpreter(){
  const char * choix[]={"Xcas interpreter","Xcas compat Python ^=**","Xcas compat Python ^=xor","MicroPython interpreter",0};
  return select_item(choix,"Syntax",false);
}

ulonglong double2gen(double d){
  giac::gen g(d);
  return *(ulonglong *) &g;
}

ulonglong int2gen(int d){
  giac::gen g(d);
  return *(ulonglong *) &g;
}

void turtle_freeze(){
  freezeturtle=true;
}

void doubleptr2matrice(double * x,int n,int m,giac::matrice & M){
  M.resize(n);
  for (int i=0;i<n;++i){
    M[i]=giac::vecteur(m);
    giac::vecteur & w=*M[i]._VECTptr;
    for (int j=0;j<m;++j){
      w[j]=*x;
      ++x;
    }
  }
}

// x must have enough space!
bool matrice2doubleptr(const giac::matrice &M,double *x){
  int n=M.size();
  if (n==0 || M.front().type!=giac::_VECT)
    return false;
  int m=M.front()._VECTptr->size();
  for (int i=0;i<n;++i){
    if (M[i].type!=giac::_VECT || M[i]._VECTptr->size()!=m)
      return false;
    giac::vecteur & w=*M[i]._VECTptr;
    for (int j=0;j<m;++j){
      giac::gen g =giac::evalf_double(w[j],1,giac::context0);
      if (g.type!=giac::_DOUBLE_)
	return false;
      *x=g._DOUBLE_val;
      ++x;
    }
  }
  return true;
}

bool r_inv(double * x,int n){
  giac::matrice M(n);
  doubleptr2matrice(x,n,n,M);
  M=giac::minv(M,giac::context0);
  return matrice2doubleptr(M,x);
}


bool r_rref(double * x,int n,int m){
  giac::matrice M(n);
  doubleptr2matrice(x,n,m,M);
  giac::gen g=giac::_rref(M,giac::context0);
  if (g.type!=giac::_VECT)
    return false;
  return matrice2doubleptr(*g._VECTptr,x);
}

double r_det(double *x,int n){
  giac::matrice M(n);
  doubleptr2matrice(x,n,n,M);
  giac::gen g=giac::mdet(M,giac::context0);
  g=giac::evalf_double(g,1,giac::context0);
  double d=1.0,e=1.0;
  if (g.type!=_DOUBLE_)
    return 0.0/(d-e);
  return g._DOUBLE_val;
}

void c_complexptr2matrice(c_complex * x,int n,int m,giac::matrice & M){
  M.resize(n);
  for (int i=0;i<n;++i){
    if (m==0){
      M[i]=gen(x->r,x->i);
      ++x;
      continue;
    }
    M[i]=giac::vecteur(m);
    giac::vecteur & w=*M[i]._VECTptr;
    for (int j=0;j<m;++j){
      w[j]=gen(x->r,x->i);
      ++x;
    }
  }
}

c_complex gen2c_complex(giac::gen & g){
  double d=1.0,e=1.0;
  c_complex c={0,0};
  if (g.type!=giac::_DOUBLE_ && g.type!=giac::_CPLX)
    c.r=c.i=0.0/(d-e);
  else {
    if (g.type==giac::_DOUBLE_)
      c.r=g._DOUBLE_val;
    else {
      if (g.subtype!=3)
	c.r=c.i=0.0/(d-e);
      c.r=g._CPLXptr->_DOUBLE_val;
      c.i=(g._CPLXptr+1)->_DOUBLE_val;
    }
  }
  return c;
}

// x must have enough space!
bool matrice2c_complexptr(const giac::matrice &M,c_complex *x){
  int n=M.size();
  if (n==0)
    return false;
  if (M.front().type!=giac::_VECT){
    for (int i=0;i<n;++i){
      giac::gen g =giac::evalf_double(M[i],1,giac::context0);
      if (g.type!=giac::_DOUBLE_ && g.type!=giac::_CPLX)
	return false;
      *x=gen2c_complex(g);
      ++x;
    }
    return true;
  }
  int m=M.front()._VECTptr->size();
  for (int i=0;i<n;++i){
    if (M[i].type!=giac::_VECT || M[i]._VECTptr->size()!=m)
      return false;
    giac::vecteur & w=*M[i]._VECTptr;
    for (int j=0;j<m;++j){
      giac::gen g =giac::evalf_double(w[j],1,giac::context0);
      if (g.type!=giac::_DOUBLE_ && g.type!=giac::_CPLX)
	return false;
      *x=gen2c_complex(g);
      ++x;
    }
  }
  return true;
}

bool c_inv(c_complex * x,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,n,M);
  M=giac::minv(M,giac::context0);
  return matrice2c_complexptr(M,x);
}

bool c_proot(c_complex * x,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,0,M);
  M=giac::proot(M);
  return matrice2c_complexptr(M,x);
}

bool c_pcoeff(c_complex * x,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,0,M);
  M=giac::pcoeff(M);
  return matrice2c_complexptr(M,x);
}

bool c_fft(c_complex * x,int n,bool inverse){
#if 1
  complex<double> * X=(complex<double> *) x;
  double theta=2*M_PI/n;
  if (!inverse)
    theta=-theta;
  fft2(X,n,theta);
  if (inverse){
    for (int i=0;i<n;++i)
      X[i]=X[i]/double(n);
  }
  return true;
#else
  giac::matrice M(n);
  c_complexptr2matrice(x,n,0,M);
  gen g=inverse?giac::_ifft(M,giac::context0):giac::_fft(M,giac::context0);
  if (g.type!=_VECT)
    return false;
  return matrice2c_complexptr(*g._VECTptr,x);
#endif
}

bool c_egv(c_complex * x,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,n,M);
  gen g=giac::_egv(M,giac::context0);
  if (!ckmatrix(g))
    return false;
  return matrice2c_complexptr(*g._VECTptr,x);
}

bool c_eig(c_complex * x,c_complex * d,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,n,M);
  gen g=giac::_jordan(M,giac::context0);
  if (g.type!=_VECT || g._VECTptr->size()!=2 || !ckmatrix(g[0]) || !ckmatrix(g[1]))
    return false;
  return matrice2c_complexptr(*g[0]._VECTptr,x) && matrice2c_complexptr(*g[1]._VECTptr,d);
}

bool c_rref(c_complex * x,int n,int m){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,m,M);
  giac::gen g=giac::_rref(M,giac::context0);
  if (g.type!=giac::_VECT)
    return false;
  return matrice2c_complexptr(*g._VECTptr,x);
}

c_complex c_det(c_complex *x,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,n,M);
  giac::gen g=giac::mdet(M,giac::context0);
  g=giac::evalf_double(g,1,giac::context0);
  return gen2c_complex(g);
}

void c_sprint_double(char * s,double d){
  giac::sprint_double(s,d);
}

// auto-shutdown
int do_shutdown(){
  xcas::save_console_state_smem("session.xw.tns",false,giac::context0);
#ifdef NO_STDEXCEPT
  return 1;
#else
  throw autoshutdown();
#endif
}

// string translations
#ifdef NUMWORKS
#include "numworks_translate.h"
#else
#include "aspen_translate.h"
#endif
bool tri2(const char4 & a,const char4 & b){
  int res= strcmp(a[0],b[0]);
  return res<0;
}

int giac2aspen(int lang){
  switch (lang){
  case 0: case 2:
    return 1;
  case 1:
    return 3;
  case 3:
    return 5;
  case 6:
    return 7;
  case 8:
    return 2;
  case 5:
    return 4;
  }
  return 0;
}

const char * gettext(const char * s) { 
  // 0 and 2 english 1 french 3 sp 4 el 5 de 6 it 7 tr 8 zh 9 pt
  int aspenlang=giac2aspen(lang);
  char4 s4={s};
  std::pair<char4 * const,char4 *const> pp=equal_range(aspen_giac_translations,aspen_giac_translations+aspen_giac_records,s4,tri2);
  if (pp.first!=pp.second && 
      pp.second!=aspen_giac_translations+aspen_giac_records &&
      (*pp.first)[aspenlang]){
    return (*pp.first)[aspenlang];
  }
  return s;
}

#endif // KHICAS
