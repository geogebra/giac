// implementation of the minimal C SDK for KhiCAS

int sdk_ctrl_c=0;

double loopsleep(int ms){
  double n=ms*(3000),j=0.0;
  for (double i=0;i<n;++i){
    j+=i;
  }
  return j;
}

int (*khicas_shutdown)()=0;

short shutdown_state=0;
short exam_mode=0,nspire_exam_mode=0;
unsigned exam_start=0; // RTC start
int exam_duration=0;
// <0: indicative duration, ==0 time displayed during exam, >0 end exam_mode after
const int exam_bg1=0x4321,exam_bg2=0x1234;
int exam_bg(){
  return exam_mode?(exam_duration>0?exam_bg1:exam_bg2):0x50719;
}

void SetQuitHandler( void (*f)(void)){}

/*
  ******************
  *  HP PRIME G1  *
  ******************
*/
#ifdef HPG1
#include <string.h> 
#include "k_csdk.h"
#include "muteki.h"

void (*offon_handler)(const void * param)=0;
const void * offon_param=0;
volatile unsigned prime_keydown;
volatile unsigned prime_mousex,prime_mousey,prime_event;
volatile unsigned prime_keydown=0;
volatile unsigned prime_pushx=0,prime_pushy=0,prime_mousex=0,prime_mousey=0,prime_event=0;
int prime_statusarea=18;

unsigned long SetSystemVariable(unsigned short type,unsigned short mode,unsigned long value);

void * ev_threadptr;

unsigned * fb_screen=0;

typedef struct {
  uint16_t SomeVal;
  uint16_t x_res;
  uint16_t y_res;
  uint16_t pixel_bits;
  uint16_t bytes_per_line;
  uint16_t brightness_level;
  uint32_t unk1_0;
  uint32_t window1_bufferstart;
} LCD_MAGIC;

#define SWI_GET_LCD             0x1008D

#define NAKED_SWI(code)				\
  __asm volatile (				\
        "push {r0}\n\t" \
        "push {lr}\n\t" \
        "swi %0\n\t" \
        :: "i" (code) \
    )

LCD_MAGIC** __attribute__((target("arm"), naked)) sys_get_lcd(void) { NAKED_SWI(SWI_GET_LCD); }

int get_fb(){
  LCD_MAGIC** ppLcd = sys_get_lcd();
  if (!ppLcd)
    return 1;
  //int screen_w = (*ppLcd)->x_res;
  fb_screen = (unsigned *)(*ppLcd)->window1_bufferstart;
  if (!fb_screen)
    return 2;
  ClearScreen(0);
  // vGL_putString(0,0,"INIT",0xffffff,0,16); loopsleep(2000);
  return 0;
}

int sdk_init(){
  ev_threadptr=OSCreateThread(event_manager,0 /* pointer passed to event_manager*/,0 /* system default stack size*/,0 /* run immediatly*/);
  int res=get_fb();
  return res;
}

void sdk_end(){
  OSTerminateThread((thread_t*) ev_threadptr,0);
}


unsigned getbrightness(){
  devio_descriptor_t * backlight_ptr=CreateFile( "\\\\?\\BACK", 3, 0, NULL, 3, 0, NULL );
  if (!backlight_ptr) return 128;
  unsigned u=SetSystemVariable(6, (unsigned short)-1 /*GET*/, 0 );
  CloseHandle(backlight_ptr);
  return u;
}

short setbrightness(short level){
  devio_descriptor_t * backlight_ptr=CreateFile( "\\\\?\\BACK", 3, 0, NULL, 3, 0, NULL );
  if (!backlight_ptr) return -1;
  unsigned m;
  DeviceIoControl(backlight_ptr, 265 /*BLDEV_IO_GETMAXLEVEL*/, NULL, 0, &m, 4, NULL, NULL); // max level
  if (level<=(int)m)
    level=SetSystemVariable(6, (unsigned short) 2 /* SET*/,level);
  CloseHandle(backlight_ptr);
  return level;
}

typedef short (*usb_cb_t)(unsigned short , unsigned short);
void USBMassStorageRun(usb_cb_t usb_cb,unsigned short u);
short usb_cb(unsigned short u, unsigned short v){
  return 0;
}

int event_manager(void * p){
  ClearAllEvents();
  prime_keydown=0;prime_mousex=0,prime_mousey=0,prime_event=0;
  ui_event_t e;
  for (int i=0;;++i){
    if (GetEvent(&e)){
      unsigned * ptr=(unsigned *) &e;
      prime_event=ptr[7];
      // ui_event_prime_s in include/ui/common.h
      // ptr[0]==destinataire, ignored
      if (prime_event==0x10){
	if (p)
	  Printf("keydown: %i 0 %x %x %x %x %x %x %x %x\n",i,ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7],ptr[8]);
	prime_keydown=ptr[8]>>16;
	OSSleep(30);
      }
      else if (prime_event==0x100000){
	// if (p) Printf("\fkeyup: %i 0 %x %x %x %x %x %x %x %x\n",i,ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7],ptr[8]);
	//if (1){ Printf("\fkeyup: %x\n",ptr[8]>>16); loopsleep(200); }
	if (prime_keydown==0x83){
	  rgbSetColor(0xff0000);  rgbSetBkColor(0);  Printf("\f  *** ON pressed *** "); // loopsleep(200);
	  sdk_ctrl_c=1;
	}
	prime_keydown=0;
	OSSleep(10);
      }
      else if (prime_event==0x1){
	prime_pushx=prime_mousex=ptr[8]>>16;
	prime_pushy=prime_mousey=ptr[9];
	if (prime_mousey>=220)
	  prime_keydown=KEY_CTRL_F1+prime_mousex/54;
	else
	  prime_keydown=KEY_MOUSE_PUSH;
	if (p) Printf("touchpush: %i pos=%i,%i 0 %x %x %x %x %x %x %x %x %x %x\n",i,prime_mousex,prime_mousey,ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7],ptr[8],ptr[9],ptr[10]);
	OSSleep(30);
      }
      else if (prime_event==0x2){
	prime_mousex=ptr[8]>>16;
	prime_mousey=ptr[9];
	if (p) Printf("touchmove: %i pos=%i,%i 0 %x %x %x %x %x %x %x %x %x %x\n",i,prime_mousex,prime_mousey,ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7],ptr[8],ptr[9],ptr[10]);
	if (prime_keydown<KEY_CTRL_F1 || prime_keydown>KEY_CTRL_F6)
	  prime_keydown=KEY_MOUSE_MOVE;
      }
      else if (prime_event==0x8){
	prime_mousex=ptr[8]>>16;
	prime_mousey=ptr[9];
	prime_keydown=0;
	if (p) Printf("touchup: %i pos=%i,%i 0 %x %x %x %x %x %x %x %x %x %x\n",i,prime_mousex,prime_mousey,ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7],ptr[8],ptr[9],ptr[10]);
      }
      else {
	if (ptr[1]==0x4000 ){
	  // Printf("Battery status\n");
	  // battery status
	}
	else if (ptr[1]==0x100){
	  if (ptr[2]==0x24){
	    // Printf("Disable Com\n");
	  }
	  else if (ptr[2]==0x2d){
	    // Printf("Enable Com\n");
	    loopsleep(500);
	    os_fill_rect(10,80,300,80,0);
	    os_draw_string(20,95,0x1f,0,"Prime storage as PC USB key?",false);
	    os_draw_string(20,115,0x1f,0,"Press Enter to run",false);
	    os_draw_string(20,135,0xffff,0,"Any other key to cancel",false);
	    for (;;){
	      if (GetEvent(&e))
		break;
	    }
	    //Printf("\f%x %x %x\n",ptr[1],ptr[7],ptr[8]>>16);
	    if (ptr[1]==0x4000 ){
	      for (;;){
		if (GetEvent(&e))
		  break;
	      }
	    }
	    prime_event=ptr[7];
	    //Printf("%x %x %x\n",ptr[1],prime_event,ptr[8]>>16);
	    if (prime_event==0x10){
	      os_draw_string(20,115,0x1f<<11,0,"Now release the key",false);
	      os_draw_string(20,135,0xffff,0,"                       ",false);
	      for (;;){
		if (GetEvent(&e))
		  break;
	      }
	      ptr=(unsigned *) &e;
	      prime_event=ptr[7];
	      prime_keydown=ptr[8]>>16;
	      //Printf("%x %x %x\n",ptr[1],prime_event,prime_keydown);
	    }
	    if (prime_event==0x100000 && prime_keydown==KEY_ENTER){
	      os_fill_rect(10,80,300,80,0);
	      os_draw_string(20,95,0xffff,0,"Prime storage as PC USB key",false);
	      os_draw_string(20,115,0xffff,0,"Eject key on PC to end exchange",false);
	      os_draw_string(20,135,0xffff,0,"Then disconnect the Prime",false);
	      USBMassStorageRun(usb_cb,false);
	      os_fill_rect(10,80,300,80,0);
	      os_draw_string(20,120,0xffff,0,"USB ended",false);
	    }
	    else {
	      os_fill_rect(10,80,300,80,0);
	      os_draw_string(20,120,0xffff,0,"USB cancelled",false);
	    }
	    prime_keydown=0;
	  }
	  else if (1|| p){
	    Printf("other: %x %x %x %x %x %x %x %x\n",ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]);
	    Printf("   %x %x %x %x %x %x %x %x\n",ptr[8],ptr[9],ptr[10],ptr[11],ptr[12],ptr[13],ptr[14],ptr[15]);
	  }
	}
      }
    }
  }  
}

int waitforvblank(){
  return 0;
}

int back_key_pressed(){
  return prime_keydown==KEY_ESC;
}
// next 3 functions may be void if not inside a window class hierarchy
void os_show_graph(){} // show graph inside Python shell (Numworks), not used
void os_hide_graph(){} // hide graph, not used anymore
void os_redraw(){} // force redraw of window class hierarchy

int os_set_angle_unit(int mode){
  return false;
}
int os_get_angle_unit(){
  return 0;
}

double millis(){ // increasing time value
  datetime_t dt;
  GetSysTime(&dt);
  return ((dt.hour*60.+dt.minute)*60+dt.second)*1000.+dt.millis;
}


void get_hms(int *h,int *m,int *s){
  datetime_t dt;
  GetSysTime(&dt);
  *h=dt.hour;
  *m=dt.minute;
  *s=dt.second;
}

void get_time(int *h,int *m){
  int s;
  get_hms(h,m,&s);
}

void set_time(int h,int m){
  // FIXME
  datetime_t dt;
  GetSysTime(&dt);
  dt.hour=h;
  dt.minute=m;
  SetSysTime(&dt);
}

void ck_msleep(int ms){
  OSSleep(ms);
  // loopsleep(ms);
}

void os_wait_1ms(int ms){
#if 1
  OSSleep(ms);
#else
  ck_msleep(ms);
#endif
}

int file_exists(const char * filename){
  char buf[256]="c:\\xcas\\";
  strcat(buf,filename);
  //rgbSetColor(0xff0000); Printf("file_exist %s\n",buf); OSSleep(2000);
  file_descriptor_t * f = _afopen(buf,"rb");
  if (!f)
    return 0;
  _fclose(f);
  return 1;
}

int erase_file(const char * filename){
  char buf[256]="c:\\xcas\\";
  strcat(buf,filename);
  return _aremove(buf);
}

char hpg1_filebuf[HPG1_FILEBUFFER];
int _feof(file_descriptor_t *);
int _fgetc(file_descriptor_t *);
int _fputc(int,file_descriptor_t *);

const char * read_file_size(const char * filename,unsigned * sizeptr){
  if (sizeptr) *sizeptr=0;
  if (exam_mode &&
      (strcmp(filename,"session.xw")!=0 &&
       strcmp(filename,"session.xw.tns")!=0 &&
       strcmp(filename,"session.py")!=0 &&
       strcmp(filename,"session.py.tns")!=0 
       )
      )
    return 0;
  char buf[256]="c:\\xcas\\";
  strcat(buf,filename);
  file_descriptor_t * f = _afopen(buf,"rb");
  if (!f) return 0;
  __fseek(f,0L,SEEK_END);
  unsigned s = _ftell(f);
  __fseek(f,0L,SEEK_SET);
  //rgbSetColor(0xff0000); Printf("_afopen rb %s %i\n",buf,s); OSSleep(2000);
  if (s>HPG1_FILEBUFFER-1){
    _fclose(f);    
    return 0;
  }
#if 1
  int lus=_fread(hpg1_filebuf,1,s,f);
  hpg1_filebuf[s]=0;
  _fclose(f);
  if (lus==s && sizeptr)
    *sizeptr=s;
  return lus!=s?0:hpg1_filebuf;
#else
  for (int i=0;i<s;++i){
    if (_feof(f))
      break;
    hpg1_filebuf[i]=_fgetc(f);
  }
  hpg1_filebuf[s]=0;
  _fclose(f);
  return hpg1_filebuf;
#endif
}

const char * read_file(const char * filename){
  unsigned s;
  return read_file_size(filename,&s);
}

int write_file(const char * filename,const char * s,int len){
  if (exam_mode &&
      (strcmp(filename,"session.xw")!=0 &&
       strcmp(filename,"session.xw.tns")!=0 &&
       strcmp(filename,"session.py")!=0 &&
       strcmp(filename,"session.py.tns")!=0 
       )
      )
    return false;
  char buf[256]="c:\\xcas\\";
  strcat(buf,filename);
  // if (1) {rgbSetColor(0xff0000); rgbSetBkColor(0); Printf("_afopen w %s %i\n",buf,len); OSSleep(2000);}
  file_descriptor_t * f=_afopen(buf,"wb+");
  if (!f) return false;
  if (!len) len=strlen(s);
  for (int i=0;i<len;++i)
    _fputc(s[i],f);
  _fclose(f);
  return true;
}

#define FILENAME_MAXRECORDS 64
#define FILENAME_MAXSIZE 16
char os_filenames[FILENAME_MAXRECORDS][FILENAME_MAXSIZE];
int c_trialpha(const void *p1,const void * p2){
  int i=strcmp(* (char * const *) p1, * (char * const *) p2);
  return i;
}

char minuscule(char c){
  if (c>='a' && c<='z')
    return c-0x20;
  return c;
}

int my_strcmp(const char *s1, const char *s2) {
  while (*s1 && minuscule(*s1) == minuscule(*s2)) {
    s1++;
    s2++;
  }
  return minuscule(*s1) - minuscule(*s2);
}

int os_file_browser(const char ** filenames,int maxrecords,const char * extension,int storage){ // storage is ignored on hpg1
  find_context_t ctx;
  char motif[256]="c:\\xcas\\*.*";
  short s=_afindfirst(motif,&ctx,0);
  if (s==-1){
    filenames[0]=0;
    return 0;
  }
  int cur=0;
  while (1){
    const char * s_=ctx.filename,*ext=0;
    int l=strlen(s_),j;
    char s[l+1];
    strcpy(s,s_);
    for (j=l-1;j>0;--j){
      if (s[j]=='.'){
	ext=s+j+1;
	break;
      }
    }
    //Printf("find ext=%s f=%s\n",ext,s_); OSSleep(2000);
    if (ext && my_strcmp(ext,"tns")==0){
      s[j]=0;
      for (;j>0;--j){
	if (s[j]=='.'){
	  ext=s+j+1;
	  break;
	}
      }
    }
    if (ext && my_strcmp(ext,extension)==0){
      if (exam_mode &&
	  (my_strcmp(s_,"session.xw")!=0 &&
	   my_strcmp(s_,"session.xw.tns")!=0 &&
	   my_strcmp(s_,"session.py")!=0 &&
	   my_strcmp(s_,"session.py.tns")!=0 
	   )
	  )
	continue;
      //Printf("find ext=%s f=%s\n",ext,s_); OSSleep(2000);
      strncpy(os_filenames[cur],s_,FILENAME_MAXSIZE);
      filenames[cur]=os_filenames[cur];
      ++cur;
    }
    if (_afindnext(&ctx)!=0)
      break;
  }
  _findclose (&ctx);
  filenames[cur]=NULL;
#if 1
  qsort(filenames,cur,sizeof(char *),c_trialpha);
#else
  // qsort would be faster for large n, but here n<FILENAME_MAXRECORDS
  for (;;){
    int finished=true;
    for (int i=1;i<cur;++i){
      if (strcmp(filenames[i-1],filenames[i])>0){
	finished=false;
	const char * tmp=filenames[i-1];
	filenames[i-1]=filenames[i];
	filenames[i]=tmp;
      }
    }
    if (finished)
      break;
  }
#endif
  return cur;
}

int c_rgb565to888(int c){
  c &= 0xffff;
  int r=(c>>11)&0x1f,g=(c>>5)&0x3f,b=c&0x1f;
  return (r<<19)|(g<<10)|(b<<3);
}

int c_rgb888to565(int c){
  c &= 0xffffff;
  int r=(c>>19),g=((c>>8)&0xff)>>2,b=(c&0xff)>>3;
  return (r<<11)|(g<<5)|(b);
}

void os_set_pixel(int x,int y,int c){
  if (x>=0 && x<LCD_WIDTH_PX && y>=-prime_statusarea && y<LCD_HEIGHT_PX-prime_statusarea){
    rgbSetColor(c_rgb565to888(c));
    FillRect(x,y+prime_statusarea,x,y+prime_statusarea,0);
  }
}

void os_fill_rect(int x,int y,int w,int h,int c){
  y += prime_statusarea;
  if (x<0){ w+=x; x=0;}
  if (x+w>=LCD_WIDTH_PX){w=LCD_WIDTH_PX-x;}
  if (y<0){ h+=y; y=0;}
  if (y+h>=LCD_HEIGHT_PX){h=LCD_HEIGHT_PX-y;}
  if (w>0 && h>0){
    rgbSetColor(c_rgb565to888(c));
    FillRect(x,y,x+w-1,y+h-1,0);
  }
}

int os_get_pixel(int x,int y){
  if (x<0 || x>=LCD_WIDTH_PX || y<0 || y>=LCD_HEIGHT_PX)
    return -1;
  return c_rgb888to565(GetPixel(x,y));
}

int write_string(int x,int y,const char * s,int fake){
  if (!s) return x;
  if (fake){
    return x+8*strlen(s);
  }
  for (int i=0;;++i){
    unsigned char c=s[i];
    if (c==0 || x>=LCD_WIDTH_PX)
      break;
    if (x<0 || c>=0x80 || c<0x20) // avoid crash (missing char in besta font)!
      x+=8;
    else
      x=WriteChar(x,y,c,0);
  }
  return x;
}

int os_draw_string(int x,int y,int c,int bg,const char * s,int fake){
#if 0
  if (fake)
    return x+strlen(s)*8;
  return vGL_putString(x,y+prime_statusarea,s,c_rgb565to888(c),c_rgb565to888(bg),16);
#endif
  // old code
  rgbSetColor(c_rgb565to888(c));
  rgbSetBkColor(c_rgb565to888(bg));
  //Printf("FontType %x\n",GetFontType());
  // SetFontType(10); // 0, 4, 8
  return write_string(x,y+prime_statusarea,s,fake); //??
}

int os_draw_string_medium(int x,int y,int c,int bg,const char * s,int fake){
#if 0
  if (fake)
    return x+strlen(s)*7;
  return vGL_putString(x,y+prime_statusarea,s,c_rgb565to888(c),c_rgb565to888(bg),14);
#endif
  rgbSetColor(c_rgb565to888(c));
  rgbSetBkColor(c_rgb565to888(bg));
  // SetFontType(4); // 0, 4, 8
  return write_string(x,y+prime_statusarea,s,fake?0x40:0); //??
}

int os_draw_string_small(int x,int y,int c,int bg,const char * s,int fake){
  if (fake)
    return x+strlen(s)*6;
  return vGL_putString(x,y+prime_statusarea,s,c_rgb565to888(c),c_rgb565to888(bg),12);
  rgbSetColor(c_rgb565to888(c));
  rgbSetBkColor(c_rgb565to888(bg));
  // SetFontType(0); // 0, 4, 8
  return write_string(x,y+prime_statusarea,s,fake?0x40:0); //??
}

const int statuscolor=COLOR_CYAN;
void statuslinemsg(const char * msg){
  rgbSetColor(c_rgb565to888(statuscolor));
  FillRect(0,0,230,prime_statusarea,0);
  os_draw_string(0,-prime_statusarea,SDK_BLACK,statuscolor,msg,false);
}

void display_time(){
  int h,m,s;
  get_hms(&h,&m,&s);
  char msg[10];
  msg[0]=' ';
  msg[1]='0'+(h/10);
  msg[2]='0'+(h%10);
  msg[3]= 'h';
  msg[4]= ('0'+(m/10));
  msg[5]= ('0'+(m%10));
  msg[6]=0;
  //msg[6]= 'm';
  //msg[7] = ('0'+(s/10));
  //msg[8] = ('0'+(s%10));
  //msg[9]=0;
  int bg=exam_bg();
  rgbSetColor(c_rgb565to888(bg));
  FillRect(270,0,LCD_WIDTH_PX,prime_statusarea,0);
  rgbSetColor(0);
  write_string(270,0,msg,false);
}

void sync_screen(){
}

int prime_shift=0;
int prime_alpha=0,prevalpha=0;
int prime_select=false;
void statusline(int mode){
  char *msg=0;
  if (prime_alpha==1)
    msg="1alpha";
  else if (prime_alpha==3)
    msg="1ALPHA";
  else if (prime_alpha==4)
    msg="alock";
  else if (prime_alpha==6)
    msg="ALOCK";  
  else if (prime_shift)
      msg="shift";
  else {
    power_battery_status_t g1_bat={0};
    GetBatteryValue(0,&g1_bat);
    int i=g1_bat.level;
    if (i<=1)
      msg=" --- ";
    else if (i==2)
      msg=" +-- ";
    else if (i==3)
      msg=" ++- ";
    else
      msg=" +++ ";
  }
  int bg=exam_bg();
  rgbSetColor(c_rgb565to888(bg));
  FillRect(230,0,LCD_WIDTH_PX,prime_statusarea,0);
  rgbSetColor(0);
  rgbSetBkColor(c_rgb565to888(bg));
  write_string(230,0,msg,false);
  //rgbSetColor(0xff0000);
  //write_string(190,0," CAS ",false);    
  display_time();
  if (mode==0)
    return;
  sync_screen();
}

int isKeyPressed(int key){
  return prime_keydown==key;
}

#define SHIFTALPHA(x, y, z) (prime_alpha ? ((prime_alpha&2)?z-0x20:z) : prime_shift ? (y) : (x))
#define SHIFTCTRL(x, y, z) (prime_alpha ? (z) : prime_shift ? (y) : (x))
#define SHIFT(x, y) SHIFTCTRL(x, y, x)
#define CTRL(x, y) SHIFTCTRL(x, x, y)
#define NORMAL(x) SHIFTCTRL(x, x, x)

int prime_translate(int k){
  if (k==KEY_MOUSE_PUSH || k==KEY_MOUSE_MOVE){
    // Printf("\f\nmouse\n"); OSSleep(1000);
    return k;
  }
  if (k>=KEY_CTRL_F1 && k<=KEY_CTRL_F6){
    if (prime_shift)
      return k+(KEY_CTRL_F7-KEY_CTRL_F1);
    else if (prime_alpha)
      return k+(KEY_CTRL_F13-KEY_CTRL_F1);
    else return k;
  }
#if 1
  if (k==KEY_LEFT)
    return SHIFT(KEY_CTRL_LEFT,KEY_SHIFT_LEFT);
  if (k==KEY_RIGHT)
    return SHIFT(KEY_CTRL_RIGHT,KEY_SHIFT_RIGHT);
  if (k==KEY_UP)
    return SHIFT(KEY_CTRL_UP,KEY_CTRL_PAGEUP);
  if (k==KEY_DOWN)
    return SHIFT(KEY_CTRL_DOWN,KEY_CTRL_PAGEDOWN);
#else
  if (k==KEY_LEFT)
    return SHIFTCTRL(KEY_CTRL_LEFT,KEY_SHIFT_LEFT,KEY_LEFT_CTRL);
  if (k==KEY_RIGHT)
    return SHIFTCTRL(KEY_CTRL_RIGHT,KEY_SHIFT_RIGHT,KEY_RIGHT_CTRL);
  if (k==KEY_UP)
    return SHIFTCTRL(KEY_CTRL_UP,KEY_CTRL_PAGEUP,KEY_UP_CTRL);
  if (k==KEY_DOWN)
    return SHIFTCTRL(KEY_CTRL_DOWN,KEY_CTRL_PAGEDOWN,KEY_DOWN_CTRL);
#endif
  if (k==13) return SHIFT(KEY_CTRL_EXE,'\n');
  if (k==12) return KEY_CTRL_DEL ;   
  if (k==177) return KEY_SHIFT_ANS ;   
  if (k==57415) return SHIFT(KEY_CTRL_HOME,KEY_CTRL_SETUP) ;
  if (k==145) return SHIFT(KEY_CTRL_F1,KEY_CTRL_F7); // KEY_CTRL_SYMB ;   
  if (k==178) return SHIFT(KEY_CTRL_F3,KEY_CTRL_F8); // KEY_CTRL_PLOT ;   
  if (k==179) return SHIFT(KEY_CTRL_F11,KEY_CTRL_F9); // KEY_CTRL_NUM ;   
  if (k==149) return SHIFT('\t',KEY_CTRL_USER); // KEY_CTRL_HELP ;   
  if (k==180) return SHIFT(KEY_CTRL_VIEW,KEY_CTRL_CLIP); // KEY_CTRL_VIEW or F16
  if (k==KEY_PRIME_MENU) return SHIFT(KEY_CTRL_MENU,KEY_CTRL_PASTE) ;
  if (k==KEY_ESC) return SHIFT(KEY_CTRL_EXIT,KEY_CTRL_AC) ;
  if (k==181) return SHIFT(KEY_CTRL_CAS,KEY_CTRL_F10); // KEY_CTRL_CAS ;  
  if (k==KEY_A) return SHIFTALPHA(KEY_CTRL_VARS,KEY_CTRL_INS,'a') ;
  if (k==KEY_B) return SHIFTALPHA(KEY_CTRL_CATALOG,'B','b') ;
  if (k==KEY_C) return SHIFTALPHA(KEY_EQW_TEMPLATE,'C','c') ;
  if (k==KEY_D) return SHIFTALPHA('x','t','d') ;
  if (k==KEY_E) return SHIFTALPHA(KEY_CTRL_MIXEDFRAC,'E','e') ;
  if (k==KEY_F) return SHIFTALPHA(KEY_CHAR_POW,KEY_CHAR_POWROOT,'f') ;
  if (k==KEY_G) return SHIFTALPHA(KEY_CHAR_SIN,KEY_CHAR_ASIN,'g') ;
  if (k==KEY_H) return SHIFTALPHA(KEY_CHAR_COS,KEY_CHAR_ACOS,'h') ;
  if (k==KEY_I) return SHIFTALPHA(KEY_CHAR_TAN,KEY_CHAR_ATAN,'i') ;
  if (k==KEY_J) return SHIFTALPHA(KEY_CHAR_LN,KEY_CHAR_EXPN,'j') ;
  if (k==KEY_K) return SHIFTALPHA(KEY_CHAR_LOG,KEY_CHAR_EXPN10,'k') ;
  if (k==KEY_L) return SHIFTALPHA(KEY_CHAR_SQUARE,KEY_CHAR_ROOT,'l') ;
  if (k==KEY_M) return SHIFTALPHA(KEY_CHAR_LPAR,'|','m') ;
  if (k==KEY_N) return SHIFTALPHA(KEY_CHAR_RPAR,'\'','n') ;
  if (k==KEY_O) return SHIFTALPHA(',','\\','o') ;
  if (k==KEY_P) return SHIFTALPHA(KEY_CHAR_EXP,KEY_CHAR_STORE,'p') ;
  if (k==KEY_Q) return SHIFTALPHA('7',KEY_CTRL_F13,'q') ;
  if (k==KEY_R) return SHIFTALPHA('8',KEY_CHAR_ACCOLADES,'r') ;
  if (k==KEY_S) return SHIFTALPHA('9','!','s') ;
  if (k==KEY_T) return SHIFTALPHA('/',KEY_CHAR_RECIP,'t') ;
  if (k==KEY_U) return SHIFTALPHA('4',KEY_CTRL_F7,'u') ;
  if (k==KEY_V) return SHIFTALPHA('5',KEY_CHAR_CROCHETS,'v') ;
  if (k==KEY_W) return SHIFTALPHA('6','<','w') ;
  if (k==KEY_X) return SHIFTALPHA('*','>','x') ;
  if (k==KEY_Y) return SHIFTALPHA('1',KEY_CTRL_PRGM,'y') ;
  if (k==KEY_Z) return SHIFTALPHA('2','i','z') ;
  if (k=='3') return SHIFTALPHA('3',KEY_CHAR_PI,'#') ;
  if (k==183) return SHIFTALPHA('-',KEY_CHAR_PI,':') ;
  if (k=='0') return SHIFTALPHA('0','\'','"') ;
  if (k==184) return SHIFT('.','=') ;
  if (k==' ') return SHIFT(' ','_') ;
  if (k==185) return SHIFTALPHA('+',KEY_CHAR_ANS,';') ;  
  if (k==0x83) return SHIFT(KEY_CTRL_AC,KEY_PRGM_ACON) ;   // KEY_SHUTDOWN?
  return 0; // k;
}

int nspire_scan(int * adaptive_cursor_state){
  if (isKeyPressed(KEY_PRIME_ALPHA)){
    while (isKeyPressed(KEY_PRIME_ALPHA))
      OSSleep(25);
    prime_alpha=!prime_alpha;
    prime_shift=0;
    statusline(1);
    return -2;
  }
  if (isKeyPressed(KEY_SHIFT)){
    while (isKeyPressed(KEY_SHIFT))
      ;
    prime_shift=!prime_shift;
    prime_alpha=0;
    statusline(1);
    return -1;
  }
  *adaptive_cursor_state = SHIFTCTRL(0, 1, 4);
  return prime_translate(prime_keydown);
}

int handle_f5(){
  if (prime_alpha&4)
    prime_alpha ^=2;
  else
    prime_alpha |= 4;
  return 0;
}
int iskeydown(int key){
  return prime_keydown==key;
}

int ascii_get(int* adaptive_cursor_state){
  int res=nspire_scan(adaptive_cursor_state);;
  return res;
}

void wait_no_key_pressed(){
  while (1){
    DMB;
    if (prime_keydown==0)
      break;
    OSSleep(25);
  }
}

int wait_key_released(int T){
  for (int t=0;t<T;t+=25){
    DMB;
    if (prime_keydown==0)
      return 1;
    OSSleep(25);
  }
  return 0;
}

void displaymenu(const char * menu,int menubg){
  statusline(0);
  if (!menu) return;
  os_fill_rect(0,LCD_HEIGHT_PX-2*prime_statusarea,LCD_WIDTH_PX,prime_statusarea,menubg);
  os_draw_string(0,LCD_HEIGHT_PX-2*prime_statusarea,0,menubg,menu,false);
}

#if 1
int menu_getkey(int allow_suspend,const char * menu,const char * menu_shift,const char * menu_alpha,int menubg){
  if (shutdown_state)
    return KEY_SHUTDOWN;
  prevalpha=prime_alpha;
  static bool already_pressed=false;
  displaymenu(prime_shift?menu_shift:(prime_alpha?menu_alpha:menu),menubg);
  for (;;){
    DMB; // data memory barrier
    int k=prime_keydown;
    if (k==0){
      OSSleep(25);
      continue;
    }
    if (k==KEY_MOUSE_PUSH)
      return k;
    if (k==KEY_MOUSE_MOVE){
      OSSleep(100);
      return k;
    }
    else if (k==KEY_LEFT || k==KEY_RIGHT)
      already_pressed=!wait_key_released(already_pressed?100:300); // about 10 repetition/second
    else if  (k==KEY_UP || k==KEY_DOWN)
      already_pressed=!wait_key_released(already_pressed?150:300); // about 6-7 repetition/second
    else if (k==12) // del key
      already_pressed=!wait_key_released(already_pressed?175:300); // about 5-6 repetition/second
    else {
      wait_no_key_pressed();
      already_pressed=false;
    }
    if (allow_suspend && k==0x83 && prime_shift){ // OFF
      //OSSuspendThread(ev_threadptr);
      if (offon_handler)
	offon_handler(offon_param);
      SysPowerOff();
      //OSResumeThread(ev_threadptr);
      prime_shift=false;
      displaymenu(prime_shift?menu_shift:(prime_alpha?menu_alpha:menu),menubg);
      continue;
    }
    if (k==KEY_SHIFT){
      if (prime_alpha){
	prime_shift=false;
	prime_alpha ^= 2;
      }
      else
	prime_shift=!prime_shift;	
      displaymenu(prime_shift?menu_shift:(prime_alpha?menu_alpha:menu),menubg);
      continue;
    }
    if (k==KEY_PRIME_ALPHA){
      if (prime_alpha & 4)
	prime_alpha =0; // unlock alpha
      else if (prime_alpha & 1)
	prime_alpha=4; // lock alpha
      else
	prime_alpha=1; // 1-alpha
      displaymenu(prime_shift?menu_shift:(prime_alpha?menu_alpha:menu),menubg);
      continue;
    }
    k=prime_translate(k);
    if (prime_alpha || prime_shift){
      prime_shift=false;
      if ((prime_alpha&4)==0) // alpha not locked
	prime_alpha=0;
      displaymenu(prime_shift?menu_shift:(prime_alpha?menu_alpha:menu),menubg);
    }
    return k;
  }
  // void send_key_event(struct s_ns_event* eventbuf, unsigned short keycode_asciicode, INT is_key_up, INT unknown): since r721. Simulate a key event
}

int getkey(int allow_suspend){
  return menu_getkey(allow_suspend,0,0,0,0);
}
#else
int getkey(int allow_suspend){
  sync_screen();
  if (shutdown_state)
    return KEY_SHUTDOWN;
  int lastkey=-1;
  for (;;){
    if (prime_keydown==0){
      OSSleep(25);
      continue;
    }
    int cursor_state=0;
    int i=0;
    if (prime_keydown==KEY_SHIFT){
      while (i==0 && prime_keydown==KEY_SHIFT){
	// will not work now, requires a prime_secondkey 
	if (isKeyPressed(KEY_LEFT))
	  i=KEY_SELECT_LEFT;
	if (isKeyPressed(KEY_RIGHT))
	  i=KEY_SELECT_RIGHT;
	if (isKeyPressed(KEY_UP))
	  i=KEY_SELECT_UP;
	if (isKeyPressed(KEY_DOWN))
	  i=KEY_SELECT_DOWN;
	OSSleep(25); // wait shift release
      }
      if (i!=0){
	prime_select=true;
      }
      if (i==0){
	prime_shift=!prime_shift;
	statusline(0);
	sync_screen();
	i=-1;
      }
    }
    else {
      if (prime_select){
	prime_select=prime_shift=false;
	statusline(0);
	sync_screen();
	continue;
      }
      i=ascii_get(&cursor_state);
    }
    if (i<0){
      wait_no_key_pressed();
      continue;
    }
    if ( (i>=KEY_CTRL_LEFT && i<=KEY_CTRL_RIGHT) ||
	 (i>=KEY_UP_CTRL && i<=KEY_RIGHT_CTRL) ||
	 (i>=KEY_SELECT_LEFT && i<=KEY_SELECT_RIGHT) ||
	 i==KEY_CTRL_DEL){
      int delay=(lastkey==i)?5:60,j;
      for (j=0;j<delay && prime_keydown!=0;++j){
	if (0) // (nspireemu)
	  ck_msleep(14);
	else
	  ck_msleep(1);
      }
      if (prime_keydown!=0)
	lastkey=i;
      else 
	lastkey=-1;
    }
    else {
      wait_no_key_pressed();
      lastkey=-1;
    }
    if (prime_alpha || prime_shift){
      prime_alpha=prime_shift=false;
      statusline(0);
      sync_screen();
    }
    return i;
  }
  // void send_key_event(struct s_ns_event* eventbuf, unsigned short keycode_asciicode, INT is_key_up, INT unknown): since r721. Simulate a key event
}
#endif

void GetKey(int * key){
  *key=getkey(true);
}

int alphawasactive(int * key){
#if 1
  return prevalpha;
#else // for keyboards without alpha key (like nspire)
  if (*key==KEY_DOWN_CTRL){
    *key=KEY_CTRL_DOWN;
    return true;
  }
  if (*key==KEY_UP_CTRL){
    *key=KEY_CTRL_UP;
    return true;
  }
  if (*key==KEY_LEFT_CTRL){
    *key=KEY_CTRL_LEFT;
    return true;
  }
  if (*key==KEY_RIGHT_CTRL){
    *key=KEY_CTRL_RIGHT;
    return true;
  }
  return false;
#endif
}

int isalphaactive(){
  return prime_alpha;
}

void lock_alpha(){
  prime_alpha=4;
}

int GetSetupSetting(int k){
  if (k!=0x14) return -1;
  if (!isalphaactive()) return 0;
  return 4;
}

void reset_kbd(){
  prime_alpha=prime_shift=0;
}

int on_key_enabled=true;

void enable_back_interrupt(){
  on_key_enabled=true;
}

void disable_back_interrupt(){
  on_key_enabled=false;
}
#endif  // HPG1

#ifdef TICE
int clip_ymin=0;
// TI83
const int STATUS_AREA_PX=18;
// debug: dbg_printf() Add #include <debug.h> to a source file, and use make debug instead of make to build a debug program. You may need to run make clean beforehand in order to ensure all source files are rebuilt.
// ASM syscalls: https://wikiti.brandonw.net/index.php?title=Category:84PCE:Syscalls:By_Name
// doc: https://ce-programming.github.io/toolchain/index.html
// Makefile options https://ce-programming.github.io/toolchain/static/makefile-options.html
// memory layout: https://ce-programming.github.io/toolchain/static/faq.html
// parameters are in CEdev/meta (and app_tools if present)
// makefile.mk:
// BSSHEAP_LOW ?= D052C6
// BSSHEAP_HIGH ?= D13FD8
// STACK_HIGH ?= D1A87E
// INIT_LOC ?= D1A87F
// Can we set STACK_HIGH to another value? I think the global area stack+data could be "reversed", I mean stack top at 0xD2A87F and data(+code+ro_data for RAM programs) at a new position: INIT_LOC=D1987E (maybe +1 or +2)

// TI stack 4K D1A87Eh: Top of the SPL stack.
// change stack pointer (if STACK_HIGH change does not work)
// requires assembly code (https://0x04.net/~mwk/doc/z80/eZ80.pdf),
// save stack pointer
// LD (Mmn), SP
// set HL to the new stack address (top of the area-3)
// LD SP,HL
// call main
// restore stack pointer
// LD SP,(Mmn)
// 1023 bytes: uint8_t[1023] os_RamCode (do not use if flash write occurs)
// 0xD052C6: 60989 bytes used for bss+heap (temp buffers in TI OS)
// 0xD1A881: Start of UserMem. 64K for code, data, ro data
// size_t os_MemChk(void **free) size and position of free ram area
// Or we could create a VarApp in RAM with no real data inside and use this area for temporary storage.
// 0xD40000: Start of VRAM. 320x240x2 bytes = 153600 bytes.
// half may be used in 8 bits palette mode (graphx)
#include "k_csdk.h"
#include <ti/getkey.h>
#include <keypadc.h>
#include <ti/getcsc.h>
#include <ti/screen.h>
#include <ti/flags.h>
#include <sys/rtc.h> // boot_GetTime(uint8_t *seconds, uint8_t *minutes, uint8_t *hours), boot_SetTime(uint8_t seconds, uint8_t minutes, uint8_t hours)
#include <sys/timers.h>
#include <graphx.h>
#include <fileioc.h>
#include <string.h>
#include <debug.h>
#define FILENAME_MAXRECORDS 32
#define FILENAME_MAXSIZE 9
#define FILE_MAXSIZE 16384
char os_filenames[FILENAME_MAXRECORDS][FILENAME_MAXSIZE];

void sdk_init(){
  dbg_printf("SDK Init\n");
  gfx_Begin();
  unsigned short * addr=gfx_palette;
  for (int r=0;r<4;r++){
    for (int g=0;g<8;g++){
      for (int b=0;b<4;b++){
        int R=r*255/3,G=g*255/7,B=b*255/3;
        addr[(r<<5)|(g<<2)|b]=gfx_RGBTo1555(R,G,B);
        // dbg_printf("palette %i %i %i %i\n",(r<<5)|(g<<2)|b,R,G,B);
      }
    }
  }
  // 128-254 arc-en-ciel? 255 should remain white
}

void sdk_end(){
  dbg_printf("SDK End\n");
  gfx_End();
}

void clear_screen(void){
  gfx_FillScreen(255); // gfx_ZeroScreen(void);
}

int alpha=0,alphalock=0,prevalpha=0,shift=0;
int handle_f5(){
  if (alphalock)
    alphalock=3-alphalock;
  else
    alphalock=2;
  return alphalock;
}
void dbgprint(int i){
  char buf[16]={0};
  buf[0]='0'+i/100;
  buf[1]='0'+(i % 100)/10;
  buf[2]='0'+(i % 10);
  os_draw_string(20,60,SDK_WHITE,SDK_BLACK,buf,false);
}
int getkey(int allow_suspend){
  sync_screen();
  statusline(0);
  for (;;){
    int i=0;
    while (!i){
      i=os_GetCSC();
    }
    // dbgprint(i);
    int decal=(alpha>>1)<<5; // 0 or 32 for upper or lowercase
    int Alpha=alpha,Shift=shift;
    shift=0; prevalpha=alpha;
    if (!alphalock)
      alpha=0;
    switch (i){
    case sk_Fx:
      return Alpha?KEY_CTRL_F11:Shift?KEY_CTRL_F6:KEY_CTRL_F1;
    case sk_Fenetre:
      return Alpha?KEY_CTRL_F12:Shift?KEY_CTRL_F7:KEY_CTRL_F2;
    case sk_Zoom:
      return Alpha?KEY_CTRL_F13:Shift?KEY_CTRL_F8:KEY_CTRL_F3;      
    case sk_Trace:
      return Alpha?KEY_CTRL_F14:Shift?KEY_CTRL_F9:KEY_CTRL_F4;      
    case sk_Graph:
      return Alpha?KEY_CTRL_F15:Shift?KEY_CTRL_F10:KEY_CTRL_F5;      
    case sk_Mode:
      return KEY_CTRL_SETUP;
    case sk_Del:
      return KEY_CTRL_DEL;
    case sk_GraphVar:
      return KEY_CTRL_XTT;
      // sk_Stats
    case sk_Right:
      return Shift?KEY_SHIFT_RIGHT:KEY_CTRL_RIGHT;
    case sk_Left:
      return Shift?KEY_SHIFT_LEFT:KEY_CTRL_LEFT;
    case sk_Up:
      return Shift?KEY_CTRL_PAGEUP:KEY_CTRL_UP;
    case sk_Down:
      return Shift?KEY_CTRL_PAGEDOWN:KEY_CTRL_DOWN;
    case sk_Enter:
      return Alpha?KEY_SHIFT_ANS:KEY_CTRL_EXE;    
    case sk_Alpha:
      if (alphalock){
        alpha=alphalock=0;
      }
      else {
        if (Shift)
          alphalock=alpha=2;
        else {
          alpha=2;
          if (prevalpha)
            alphalock=alpha=prevalpha;            
        }
      }
      statusline(0);
      continue;
    case sk_2nd:
      if (alphalock)
        alpha=3-alpha; // maj <> min
      else
        shift=!Shift;
      statusline(0);
      continue;
    case sk_Math:
      return Alpha?KEY_CHAR_A+decal:KEY_CTRL_F6;
    case sk_Matrice:
      return Alpha?KEY_CHAR_B+decal:KEY_CHAR_MAT;
    case sk_Prgm:
      return Alpha?KEY_CHAR_C+decal:KEY_CTRL_PRGM;
    case sk_Vars:
      return KEY_CTRL_VARS;
    case sk_Annul:
      return Shift?KEY_CTRL_AC:KEY_CTRL_EXIT;
    case sk_TglExact:
      return KEY_CHAR_D+decal;
    case sk_Trig:
      return Alpha?KEY_CHAR_E+decal:(Shift?KEY_CHAR_PI:KEY_CHAR_SIN);
    case sk_Cos:
      return Alpha?KEY_CHAR_F+decal:KEY_CHAR_COS;
    case sk_Tan:
      return Alpha?KEY_CHAR_G+decal:KEY_CHAR_TAN;
    case sk_Power:
      return Alpha?KEY_CHAR_H+decal:KEY_CHAR_POW;
    case sk_Square:
      return Alpha?KEY_CHAR_I+decal:Shift?KEY_CHAR_ROOT:KEY_CHAR_SQUARE;
    case sk_Comma:
      return Alpha?KEY_CHAR_J+decal:Shift?KEY_CHAR_E:KEY_CHAR_COMMA;      
    case sk_LParen:
      return Alpha?KEY_CHAR_K+decal:Shift?KEY_CHAR_LBRACE:KEY_CHAR_LPAR;      
    case sk_RParen:
      return Alpha?KEY_CHAR_L+decal:Shift?KEY_CHAR_RBRACE:KEY_CHAR_RPAR;      
    case sk_Div:
      return Alpha?KEY_CHAR_M+decal:Shift?KEY_CHAR_E+32:KEY_CHAR_DIV;
    case sk_Log:
      return Alpha?KEY_CHAR_N+decal:Shift?KEY_CHAR_EXPN10:KEY_CHAR_LOG;
    case sk_7:
      return Alpha?KEY_CHAR_O+decal:KEY_CHAR_7;
    case sk_8:
      return Alpha?KEY_CHAR_P+decal:KEY_CHAR_8;
    case sk_9:
      return Alpha?KEY_CHAR_Q+decal:KEY_CHAR_9;
    case sk_Mul:
      return Alpha?KEY_CHAR_R+decal:Shift?KEY_CHAR_LBRCKT:KEY_CHAR_MULT;
    case sk_Ln:
      return Alpha?KEY_CHAR_S+decal:Shift?KEY_CHAR_EXP:KEY_CHAR_LN;
    case sk_4:
      return Alpha?KEY_CHAR_T+decal:KEY_CHAR_4;
    case sk_5:
      return Alpha?KEY_CHAR_U+decal:KEY_CHAR_5;
    case sk_6:
      return Alpha?KEY_CHAR_V+decal:KEY_CHAR_6;
    case sk_Sub:
      return Alpha?KEY_CHAR_W+decal:Shift?KEY_CHAR_RBRCKT:KEY_CHAR_MINUS;
    case sk_Store:
      return Alpha?KEY_CHAR_X+decal:KEY_CHAR_STORE;
    case sk_1:
      return Alpha?KEY_CHAR_Y+decal:KEY_CHAR_1;
    case sk_2:
      return Alpha?KEY_CHAR_Z+decal:KEY_CHAR_2;
    case sk_3:
      return Alpha?KEY_CHAR_THETA:KEY_CHAR_3;
    case sk_Add:
      return KEY_CHAR_PLUS;
    case sk_0:
      return Alpha?KEY_CHAR_SPACE:Shift?KEY_CTRL_CATALOG:KEY_CHAR_0;
    case sk_DecPnt:
      return Alpha?':':Shift?KEY_CHAR_I+32:KEY_CHAR_DP;
    case sk_Chs:
      return Alpha?'?':Shift?KEY_CHAR_ANS:KEY_CHAR_PMINUS;
    default:
      return i;
    }
  }
}
void GetKey(int * key){
  *key=getkey(0);
}
int iskeydown(int key){
  kb_Scan();
  return kb_IsDown(key);
}

// if (kb_On) ...
void enable_back_interrupt(){
  kb_EnableOnLatch();
}
void disable_back_interrupt(){
  kb_DisableOnLatch();
}
int isalphaactive(){
  return alpha;
}
int alphawasactive(int * key){
  return prevalpha;
}
void lock_alpha(){
  alpha=alphalock=1;
}
void reset_kbd(){
  shift=alpha=alphalock=0;
}
int GetSetupSetting(int k){
  if (k!=0x14) return -1;
  if (!alpha) return 0;
  if (!alphalock) return alpha==2?8:4;
  return alpha==2?0x88:0x84;
}

void os_wait_1ms(int ms){
  msleep(ms); // delay(ms)?
}
double millis(){
  return rtc_Days*86400.0+rtc_Hours*3600.+rtc_Minutes*60.+rtc_Seconds;
}
int os_set_angle_unit(int mode){
  if (mode) os_ResetFlag(TRIG,DEGREES); else os_SetFlag(TRIG,DEGREES);
  return true;
}

int os_get_angle_unit(){
  int i=os_TestFlag(TRIG,DEGREES);
  return i?0:1;
}
int file_exists(const char * filename){
  int h=ti_Open(filename, "r");
  if (!h)
    return false;
  ti_Close(h);
  return true;
}
int erase_file(const char * filename){
  if (!file_exists(filename))
    return false;
  ti_Delete(filename);
  return true;
}
const char * read_file(const char * filename){
  const char * ext=0;
  int l=strlen(filename);
  char var[9]={0};
  strncpy(var,filename,8);
  for (--l;l>0;--l){
    if (filename[l]=='.'){
      ext=filename+l+1;
      if (l<9)
        var[l]=0;
      break;
    }
  }
  int h=ti_Open(var, "r");
  if (!h)
    return 0;
  int s=ti_GetSize(h);
  if (s>7){
    //unsigned short u;
    //ti_Read(&u,1,2,h);
    char subtype[8]={0};
    ti_Read(subtype,1,4,h);
    if (strncmp(subtype,"PYCD",4)==0 || strncmp(subtype,"XCAS",4)==0){
      unsigned char dx;
      ti_Read(&dx,1,1,h);
      if (dx!=0){
        // skip desktop filename
        char buf[256]={0};
        ti_Read(buf,1,1,dx);
        s -= 4+dx;
        dbg_printf("subtype=%s filename=%s %i %i\n",subtype,buf,dx,s);
      }
      else
        s -= 4;
    }
    else
      ti_Seek(0,SEEK_SET,h);
  }
  char * ptr=0;
#if 0
  // Direct access to the data, ptr should not be used if any change to the TI variables occurs, unfortunately there is no 0 at end of string
  ptr= ti_GetDataPtr(h);
  ti_Close(h);
  dbg_printf("data=%x %x %x %x %x %x %x %x\n",ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]);
  return ptr;
#endif
  // Code requiring a copy
  // if it starts with 
  // char * ptr=(char *) gfx_vram+LCD_WIDTH_PX*LCD_HEIGHT_PX; // pointer in vram buffer
  int S=os_MemChk((void **)&ptr);
  if (s>=S)
    return 0;
  S=ti_Read(ptr,1,s,h);
  ptr[S]=0;
  ti_Close(h);
  dbg_printf("data=%s\n",ptr);
  return ptr;
}
int write_file(const char * filename,const char * s,int len){
  // find extension
  const char * ext=0;
  int l=strlen(filename);
  char var[9]={0};
  strncpy(var,filename,8);
  for (--l;l>0;--l){
    if (filename[l]=='.'){
      ext=filename+l+1;
      if (l<9)
        var[l]=0;
      break;
    }
  }
  int h=ti_Open(var,"w");
  if (!h) return false;
  if (ext){
    bool ispy=strncmp(ext,"py",2)==0;
    bool isxw=strncmp(ext,"xw",2)==0;
    if (ispy || isxw){
      const char * subtype=isxw?"XCAS":"PYCD";
      ti_Write(subtype,strlen(subtype),1,h);
      unsigned char dx=strlen(filename)+1;
      ti_Write(&dx,1,1,h);
      ti_Write(filename,dx-1,1,h);
    }
  }
  int Len=ti_Write(s,1,len,h);
  ti_Close(h);
  return Len==len;
}

int os_file_browser(const char ** filenames,int maxrecords,const char * extension,int storage){
  if (maxrecords>FILENAME_MAXRECORDS)
    maxrecords=FILENAME_MAXRECORDS;
  void * ptr=os_GetSymTablePtr();
  int cur=0;
  for (int count=0;cur<maxrecords && ptr;count++){
    uint24_t type, l,j;
    char s[16]={0};
    char * dataptr=0;
    char * ext=0;
    ptr=os_NextSymEntry(ptr, &type, &l, s,&dataptr);
    if (l>=FILENAME_MAXSIZE || !dataptr)
      continue;
    s[l]=0;
    dbg_printf("filebrowser %s %i %x %x %x %x %x %x %x %x %x %x %x %x %x\n",s,type,dataptr[0]&0xff,dataptr[1]&0xff,dataptr[2]&0xff,dataptr[3]&0xff,dataptr[4]&0xff,dataptr[5]&0xff,dataptr[6]&0xff,dataptr[7]&0xff,dataptr[8]&0xff,dataptr[9]&0xff,dataptr[10]&0xff,dataptr[11]&0xff,dataptr[12]&0xff);
    // if type==21 dataptr[1]*256+dataptr[0]==size, then data
    // xcas session begins with 4 bytes size, on the 83 should be 00 00 xx xx
    if (type==21 && dataptr[2]==0 && dataptr[3]==0)
      ext="xw";
    // python app, starts with 2 bytes size, "PYCD" or "PYSC"
    // the script ifself begins at data.begin() + 6 + scriptOffset
    // where scriptOffset = dataptr[6] + 1
    if (!ext){
      if (strncmp(&dataptr[2],"PYCD",4)==0 || strncmp(&dataptr[2],"PYSC",4)==0)
        ext="py";
      else if (strncmp(&dataptr[2],"XCAS",4)==0)
        ext="xw";
      else { // extension from filename _xw or _py or _...
        //dbg_printf("os_file_browser %i %i %x\n",type,l,dataptr);
        //dbg_printf("filename %i %s\n",count,s);
        for (j=l-1;j>0;--j){
          if (s[j]=='_'){
            ext=s+j+1;
            break;
          }
        }
      }
    }
    if (ext && strcmp(ext,extension)==0){
      if (exam_mode &&
          (strcmp(s,"session")!=0
           )
          )
        continue;
      strncpy(os_filenames[cur],s,FILENAME_MAXSIZE);
      filenames[cur]=os_filenames[cur];
      dbg_printf("extension match %i %s %s\n",cur,s,filenames[cur]);
      ++cur;
    }
  }
  dbg_printf("filebrowser %i\n",cur);
  return cur;
}
// gfx_Begin, gfx_SetDrawBuffer(); gfx_End
// GFX_LCD_WIDTH, HEIGHT, gfx_vbuffer=LCD RAM buffer 76800 bytes
// gfx_vram Total of 153600 bytes in size = 320x240x2
// gfx_SetDrawBuffer()gfx_SetDrawScreen()
// uint8_t gfx_SetColor(uint8_t index)
// gfx_SetPixel(uint24_t x, uint8_t y)
// uint8_t gfx_GetPixel(uint24_t x, uint8_t y)
// gfx_FillRectangle(int x, int y, int width, int height)
// gfx_FillRectangle_NoClip(uint24_t x, uint8_t y, uint24_t width, uint8_t height)
// gfx_Wait(void)
// gfx_PrintStringXY(const char *string, int x, int y)
//gfx_SetTextFGColor(uint8_t color)
// gfx_SetTextScale(uint8_t width_scale, uint8_t height_scale)
// gfx_SetTextConfig
void sync_screen(){
  //gfx_Wait();
  // gfx_BlitBuffer(); // shoud be done if gfx_SetDrawBuffer() is active;
}
int c_rgb565to888(int c){
  c &= 0xffff;
  int r=(c>>11)&0x1f,g=(c>>5)&0x3f,b=c&0x1f;
  return (r<<19)|(g<<10)|(b<<3);
}

int convertcolor(int c){
  // convert 16 bits to default palette
  c &= 0xffff;
  int r=(c>>11)&0x1f,g=(c>>5)&0x3f,b=c&0x1f;
  int R = ((r>>3)<<5) | ((g>>3)<<2) | (b>>3);
  //dbg_printf("convert %i r=%i g=%i b=%i to %i\n",c,r,g,b,R);
  return R;
}
void setcolor(int c){
  gfx_SetColor(convertcolor(c));
  //gfx_SetTextTransparentColor(0);
}
void os_set_pixel(int x,int y,int c){
  setcolor(c);
  gfx_SetPixel(x,y);
}
void os_fill_rect(int x,int y,int w,int h,int c){
  setcolor(c);
  gfx_FillRectangle(x,y,w,h);
}
int os_get_pixel(int x,int y){
  return gfx_GetPixel(x,y);
}

// FIXME? use gfx_SetTransparentColor with a value != FG and BG instead of fill rectangle
int os_draw_string_small(int x,int y,int c,int bg,const char * s,int fake){
  y+=STATUS_AREA_PX;
  gfx_SetTextScale(1,1);
  int dx=gfx_GetStringWidth(s);
  if (!fake){
    gfx_SetColor(bg);
    gfx_FillRectangle(x,y,dx,8);
    int c_=gfx_SetTextFGColor(c);
    int bg_=gfx_SetTextBGColor(bg);
    gfx_PrintStringXY(s,x,y);
    gfx_SetTextFGColor(c_);
    gfx_SetTextBGColor(bg_);
  }
  return x+dx; 
}
int os_draw_string_medium(int x,int y,int c,int bg,const char * s,int fake){
  y+=STATUS_AREA_PX;
  gfx_SetTextScale(1,2);
  //gfx_SetFontHeight(12);
  int dx=gfx_GetStringWidth(s);
  if (!fake){
    gfx_SetColor(bg);
    gfx_FillRectangle(x,y,dx,16);
    int c_=gfx_SetTextFGColor(c);
    int bg_=gfx_SetTextBGColor(bg);
    gfx_PrintStringXY(s,x,y);
    gfx_SetTextFGColor(c_);
    gfx_SetTextBGColor(bg_);
  }
  return x+dx; 
}
int os_draw_string(int x,int y,int c,int bg,const char * s,int fake){
  y+=STATUS_AREA_PX;
  gfx_SetTextScale(2,2);
  int dx=gfx_GetStringWidth(s);
  if (!fake){
    gfx_SetColor(bg);
    gfx_FillRectangle(x,y,dx,16);
    int c_=gfx_SetTextFGColor(c);
    int bg_=gfx_SetTextBGColor(bg);
    gfx_PrintStringXY(s,x,y);
    gfx_SetTextFGColor(c_);
    gfx_SetTextBGColor(bg_);
  }
  return x+dx; 
}

const int statuscolor=12345;
void statuslinemsg(const char * msg){
  os_draw_string(0,-STATUS_AREA_PX,statuscolor,SDK_BLACK,msg,false);
}

void set_time(int h,int m){
  rtc_Set(rtc_Seconds,m,h,rtc_Days);
}

void get_time(int *h,int *m){
  *h=rtc_Hours;
  *m=rtc_Minutes;
}

void display_time(){
  int h=rtc_Hours,m=rtc_Minutes;
  char msg[10];
  msg[0]=' ';
  msg[1]='0'+(h/10);
  msg[2]='0'+(h%10);
  msg[3]= 'h';
  msg[4]= ('0'+(m/10));
  msg[5]= ('0'+(m%10));
  msg[6]=0;
  //msg[6]= 'm';
  //msg[7] = ('0'+(s/10));
  //msg[8] = ('0'+(s%10));
  //msg[9]=0;
  os_fill_rect(270,0,LCD_WIDTH_PX-270,15,SDK_BLACK);
  os_draw_string_medium(270,-STATUS_AREA_PX,statuscolor,SDK_BLACK,msg,false);
}

void statusflags(){
  char *msg=0;
  if (alpha==2){
      msg=alphalock?"alock":"alpha";
  }
  else if (alpha==1){
      msg=alphalock?"ALOCK":"ALPHA";
  }
  else {
    if (shift)
      msg="2nd";
    else
      msg="";
  }
  os_fill_rect(0,0,LCD_WIDTH_PX,16,SDK_BLACK);
  os_draw_string_medium(225,-STATUS_AREA_PX,statuscolor,SDK_BLACK,msg,false);
  os_draw_string_medium(160,-STATUS_AREA_PX,statuscolor,SDK_BLACK,os_get_angle_unit()?" rad ":" deg ",false);  
  display_time();
}
void statusline(int mode){
  statusflags();
  if (mode==0)
    os_draw_string_medium(190,-STATUS_AREA_PX,statuscolor,SDK_BLACK," CAS ",false);
  if (mode==0)
    return;
  sync_screen();
}
#endif

#ifdef NSPIRE_NEWLIB
// NB changes for the nspire cx ii
// on_key_pressed() should be modified (returns always true)
// https://hackspire.org/index.php?title=Memory-mapped_I/O_ports_on_CX_II#90140000_-_Power_management
// cx ii power management 0x90140000,
// cx 900B0018 (R/W), 900B0020 (?)
// cx ii 90140050 (R/W): Disable bus access to peripherals. Reads will just return the last word read from anywhere in the address range, and writes will be ignored.
// cx 900F0020 (R/W): LCD contrast/backlight. Valid range for contrast: 0x11a to 0x1ce; normal value is 0x174. However, it can range from 0x100 (backlight off) to about 0x1d0 (about max brightness).
// -> cx ii The OS controls the LCD backlight by writing to 90130018.
#include <libndls.h>
#include "os.h" // Ndless/ndless-sdk/include/os.h
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <ngc.h>
#include "k_defs.h"

void sdk_init(void){
  lcd_init(lcd_type()); // clrscr();
}

void sdk_end(void){
  lcd_init(SCR_TYPE_INVALID);
  refresh_osscr();
}

int c_rgb565to888(int c){
  c &= 0xffff;
  int r=(c>>11)&0x1f,g=(c>>5)&0x3f,b=c&0x1f;
  return (r<<19)|(g<<10)|(b<<3);
}

const int nspire_statusarea=18;
int nspireemu=false;

int waitforvblank(){
  return 0;
}

int back_key_pressed(){
  return isKeyPressed(KEY_NSPIRE_DEL);
}
// next 3 functions may be void if not inside a window class hierarchy
void os_show_graph(){} // show graph inside Python shell (Numworks), not used
void os_hide_graph(){} // hide graph, not used anymore
void os_redraw(){} // force redraw of window class hierarchy

int os_set_angle_unit(int mode){
  return false;
}
int os_get_angle_unit(){
  return 0;
}

double millis(){
  unsigned NSPIRE_RTC_ADDR=0x90090000;
  unsigned t1= * (volatile unsigned *) NSPIRE_RTC_ADDR;
  return 1000.0*t1;
}


void get_hms(int *h,int *m,int *s){
  unsigned NSPIRE_RTC_ADDR=0x90090000;
  unsigned t1= * (volatile unsigned *) NSPIRE_RTC_ADDR;
  if (exam_mode){
    unsigned t=t1-exam_start;
    if (exam_duration>0 && t>exam_duration){
      ;//set_exam_mode(0);
    }
    else {
      if (exam_duration>0)
	t1=exam_duration-t;
      else {
	if (exam_duration<0 && t<-exam_duration)
	  t1=-exam_duration-t;
      }	
    }
  }
  unsigned d=t1/86400;
  *s=t1%86400;
  *h=*s/3600;
  *m=(*s-3600* *h)/60;
  *s%=60;
}

void get_time(int *h,int *m){
  int s;
  get_hms(h,m,&s);
}

void set_time(int h,int m){
  // FIXME
}

#ifndef is_cx2
#define is_cx2 false
#endif

double loopsleep(int ms){
  double n=ms*(is_cx2?3000:1000),j=0.0;
  for (double i=0;i<n;++i){
    j+=i;
  }
  return j;
}

void ck_msleep(int ms){
  //msleep(ms);
  loopsleep(ms);
}

void os_wait_1ms(int ms){
  ck_msleep(ms);
}

int file_exists(const char * filename){
  if (access(filename,R_OK))
    return false;
  return true;
}

int erase_file(const char * filename){
  return remove(filename)==0;
}

char nspire_filebuf[NSPIRE_FILEBUFFER];

const char * read_file(const char * filename){
  if (exam_mode &&
      (strcmp(filename,"session.xw")!=0 &&
       strcmp(filename,"session.xw.tns")!=0 &&
       strcmp(filename,"session.py")!=0 &&
       strcmp(filename,"session.py.tns")!=0 
       )
      )
    return 0;
  FILE * f = fopen(filename,"r");
  if (!f) return 0;
  fseek(f,0L,SEEK_END);
  unsigned s = ftell(f);
  fseek(f,0L,SEEK_SET);
  if (s>NSPIRE_FILEBUFFER-1){
    fclose(f);    
    return 0;
  }
  for (int i=0;i<s;++i){
    if (feof(f))
      break;
    nspire_filebuf[i]=fgetc(f);
  }
  nspire_filebuf[s]=0;
  fclose(f);
  return nspire_filebuf;
}

int write_file(const char * filename,const char * s,int len){
  if (exam_mode &&
      (strcmp(filename,"session.xw")!=0 &&
       strcmp(filename,"session.xw.tns")!=0 &&
       strcmp(filename,"session.py")!=0 &&
       strcmp(filename,"session.py.tns")!=0 
       )
      )
    return 0;
  FILE * f=fopen(filename,"wb");
  if (!f) return false;
  if (!len) len=strlen(s);
  for (int i=0;i<len;++i)
    fputc(s[i],f);
  fclose(f);
  return 1;
}

#define FILENAME_MAXRECORDS 64
#define FILENAME_MAXSIZE 16
char os_filenames[FILENAME_MAXRECORDS][FILENAME_MAXSIZE];
int c_trialpha(const void *p1,const void * p2){
  int i=strcmp(* (char * const *) p1, * (char * const *) p2);
  return i;
}
int os_file_browser(const char ** filenames,int maxrecords,const char * extension,int storage){ // storage is ignored on nspire
  DIR *dp;
  struct dirent *ep;
  if (maxrecords>FILENAME_MAXRECORDS-1)
    maxrecords=FILENAME_MAXRECORDS-1;
  dp = opendir (".");
  if (dp == NULL){
    filenames[0]=0;
    return 0;
  }
  int cur=0;
  while ( (ep = readdir (dp)) && cur<maxrecords){
    const char * s_=ep->d_name,*ext=0;
    int l=strlen(s_),j;
    char s[l+1];
    strcpy(s,s_);
    for (j=l-1;j>0;--j){
      if (s[j]=='.'){
	ext=s+j+1;
	break;
      }
    }
    if (ext && strcmp(ext,"tns")==0){
      s[j]=0;
      for (;j>0;--j){
	if (s[j]=='.'){
	  ext=s+j+1;
	  break;
	}
      }
    }
    if (ext && strcmp(ext,extension)==0){
      if (exam_mode &&
	  (strcmp(s_,"session.xw")!=0 &&
	   strcmp(s_,"session.xw.tns")!=0 &&
	   strcmp(s_,"session.py")!=0 &&
	   strcmp(s_,"session.py.tns")!=0 
	   )
	  )
	continue;
      strncpy(os_filenames[cur],s_,FILENAME_MAXSIZE);
      filenames[cur]=os_filenames[cur];
      ++cur;
    }
  }
  closedir (dp);
  filenames[cur]=NULL;
#if 0
  qsort(filenames,cur,sizeof(char *),c_trialpha);
#else
  // qsort would be faster for large n, but here n<FILENAME_MAXRECORDS
  for (;;){
    int finished=true;
    for (int i=1;i<cur;++i){
      if (strcmp(filenames[i-1],filenames[i])>0){
	finished=false;
	const char * tmp=filenames[i-1];
	filenames[i-1]=filenames[i];
	filenames[i]=tmp;
      }
    }
    if (finished)
      break;
  }
#endif
  return cur;
}

Gc nspire_gc=0;

void reset_gc(){
  if (nspire_gc){
    gui_gc_finish(nspire_gc);
    //gui_gc_free(nspire_gc);
  }
  nspire_gc=0;
}

Gc * get_gc(){
  if (!nspire_gc){
    nspire_gc=gui_gc_global_GC();
    gui_gc_setRegion(nspire_gc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    gui_gc_begin(nspire_gc);
  }
  return &nspire_gc;
}

void os_set_pixel(int x,int y,int c){
  get_gc();
  gui_gc_setColor(nspire_gc,c_rgb565to888(c));
  gui_gc_drawRect(nspire_gc,x,y+nspire_statusarea,0,0);
}

void os_fill_rect(int x,int y,int w,int h,int c){
  get_gc();
  gui_gc_setColor(nspire_gc,c_rgb565to888(c));
  gui_gc_fillRect(nspire_gc,x,y+nspire_statusarea,w,h);
}

int os_get_pixel(int x,int y){
  if (x<0 || x>=SCREEN_WIDTH || y<0 || y>=SCREEN_HEIGHT)
    return -1;
#if 1
  get_gc();
  char ** off_buff = ((((char *****)nspire_gc)[9])[0])[0x8];
  int res = *(unsigned short *) (off_buff[y+nspire_statusarea] + 2*x);
  return res;
#else
  unsigned short * addr=*(unsigned short **) 0xC0000010;
  int r=addr[(y+nspire_statusarea)*SCREEN_WIDTH+x];
  return r;
#endif
}

int nspire_draw_string(int x,int y,int c,int bg,int f,const char * s,int fake){
  // void ascii2utf16(void *buf, const char *str, int max_size): converts the UTF-8 string str to the UTF-16 string buf of size max_size.
  int l=strlen(s);
  char utf16[2*l+2];
  ascii2utf16(utf16,s,l);
  utf16[2*l]=0;
  utf16[2*l+1]=0;
  get_gc();
  gui_gc_setFont(nspire_gc,f);
  int dx=gui_gc_getStringWidth(nspire_gc, f, utf16, 0, l) ;
  if (fake)
    return x+dx;
  int dy=17;
  if (f==Regular9)
    dy=13;
  if (f==Regular11)
    dy=16;
  gui_gc_setColor(nspire_gc,c_rgb565to888(bg));
  gui_gc_fillRect(nspire_gc,x,y,dx,dy);
  gui_gc_setColor(nspire_gc,c_rgb565to888(c));
  //gui_gc_setPen(nspire_gc, GC_PS_MEDIUM, GC_PM_SMOOTH);
  gui_gc_drawString(nspire_gc, utf16, x, y-1, GC_SM_NORMAL | GC_SM_TOP); // normal mode
  return x+dx;
}

int os_draw_string(int x,int y,int c,int bg,const char * s,int fake){
  get_gc();
  gui_gc_clipRect(nspire_gc,0,nspire_statusarea,SCREEN_WIDTH,SCREEN_HEIGHT-nspire_statusarea,0);
  int i=nspire_draw_string(x,y+nspire_statusarea,c,bg,Regular12,s,fake);
  gui_gc_clipRect(nspire_gc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,GC_CRO_RESET);
  return i;
}
int os_draw_string_small(int x,int y,int c,int bg,const char * s,int fake){
  get_gc();
  gui_gc_clipRect(nspire_gc,0,nspire_statusarea,SCREEN_WIDTH,SCREEN_HEIGHT-nspire_statusarea,GC_CRO_SET);
  int i=nspire_draw_string(x,y+nspire_statusarea,c,bg,Regular9,s,fake);
  gui_gc_clipRect(nspire_gc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,GC_CRO_RESET);
  return i;
}

int os_draw_string_medium(int x,int y,int c,int bg,const char * s,int fake){
  get_gc();
  gui_gc_clipRect(nspire_gc,0,nspire_statusarea,SCREEN_WIDTH,SCREEN_HEIGHT-nspire_statusarea,GC_CRO_SET);
  int i=nspire_draw_string(x,y+nspire_statusarea,c,bg,Regular11,s,fake);
  gui_gc_clipRect(nspire_gc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,GC_CRO_RESET);
  return i;
}

void statuslinemsg(const char * msg){
  get_gc();
  int bg=exam_bg();
  gui_gc_setColor(nspire_gc,c_rgb565to888(bg));
  gui_gc_fillRect(nspire_gc,0,0,SCREEN_WIDTH,nspire_statusarea);
  nspire_draw_string(0,0,exam_mode?0xffff:0,bg,Regular9,msg,false);
  if (nspireemu)
    nspire_draw_string(190,0,exam_mode?0xffff:0,bg,Regular9," emu ",false);
  else
    nspire_draw_string(190,0,exam_mode?0xffff:0,bg,Regular9," CAS ",false);    
}

void display_time(){
  int h,m,s;
  get_hms(&h,&m,&s);
  char msg[10];
  msg[0]=' ';
  msg[1]='0'+(h/10);
  msg[2]='0'+(h%10);
  msg[3]= 'h';
  msg[4]= ('0'+(m/10));
  msg[5]= ('0'+(m%10));
  msg[6]=0;
  //msg[6]= 'm';
  //msg[7] = ('0'+(s/10));
  //msg[8] = ('0'+(s%10));
  //msg[9]=0;
  int bg=exam_bg();
  gui_gc_setColor(nspire_gc,c_rgb565to888(bg));
  gui_gc_fillRect(nspire_gc,270,0,SCREEN_WIDTH-270,nspire_statusarea);
  nspire_draw_string(270,0,exam_mode?0xffff:0,bg,Regular9,msg,false);
}

void sync_screen(){
  get_gc();
  //gui_gc_finish(nspire_gc);
  gui_gc_blit_to_screen(nspire_gc);
  ck_msleep(10);
  //nspire_gc=0;
  // gui_gc_begin(nspire_gc);
}

// Nspire peripheral reset :
// https://github.com/nDroidProject/nDroid-bootloader/blob/master/kernel.c
// https://hackspire.org/index.php?title=Memory-mapped_I/O_ports_on_CX#CC000000_-_SHA-256_hash_generator
// hardware ports
// https://hackspire.org/index.php?title=Memory-mapped_I/O_ports_on_CX

int nspire_shift=0;
int nspire_ctrl=0;
int nspire_select=false;
void statusflags(){
  char *msg=0;
  if (nspire_ctrl){
    if (nspire_shift)
      msg="shift ctrl";
    else
      msg="        ctrl";
  }
  else {
    if (nspire_shift)
      msg="shift";
    else
      msg="";
  }
  int bg=exam_bg();
  gui_gc_setColor(nspire_gc,c_rgb565to888(bg));
  gui_gc_fillRect(nspire_gc,210,0,SCREEN_WIDTH-210,nspire_statusarea);
  nspire_draw_string(224,0,exam_mode?0xffff:0,bg,Regular9,msg,false);
  if (nspireemu)
    nspire_draw_string(190,0,exam_mode?0xffff:0,bg,Regular9," emu ",false);
  else
    nspire_draw_string(190,0,0xf800,bg,Regular9," CAS ",false);    
}
void statusline(int mode){
  statusflags();
  display_time();
  if (mode==0)
    return;
  sync_screen();
}

#define EVENT_QUEUE_SIZE 64 // Doit de préférence être une puissance de 2

typedef struct {
  SDL_Event events[EVENT_QUEUE_SIZE];
  volatile int head; // Seul le thread d'entrée écrit ici
  volatile int tail; // Seul le thread principal écrit ici
} event_queue_t;

static volatile event_queue_t g_event_queue = { .head = 0, .tail = 0 };

int push_event(const SDL_Event *event) {
  if (0) PrintfXY(0,0,"push %x %x                  ",event->type,event->key.sym); // loopsleep(1000);
  int next_head = (g_event_queue.head + 1) % EVENT_QUEUE_SIZE;
  // Si la file est pleine, on abandonne l'événement (head rattrape tail)
  if (next_head == g_event_queue.tail) 
    return 0; 
  g_event_queue.events[g_event_queue.head] = *event;
  DMB; // Barrière mémoire 
  g_event_queue.head = next_head;
  return 1;
}

static void enqueue_sdl_mouse_event(Uint8 type, Uint8 button, int x, int y) {
  if (0) PrintfXY(0,0,"mouse %x %x                  ",x,y); // loopsleep(1000);
  SDL_Event ev;
  ev.type = type;
  // Recadrage Y et clamp pour rester dans les bornes du jeu (320x200)
  if (y < 0) y = 0;
  if (y >= LCD_HEIGHT_PX) game_y = LCD_HEIGHT_PX-1;
  if (x < 0) x = 0;
  if (x >= LCD_WIDTH_PX) x = LCD_WIDTH_PX-1;
  if (type == SDL_MOUSEMOTION) {
    ev.motion.x = x;
    ev.motion.y = game_y;
  } else { // SDL_MOUSEBUTTONDOWN / SDL_MOUSEBUTTONUP
    ev.button.button = button;
    ev.button.x = x;
    ev.button.y = game_y;
  }
  push_event(&ev);
}

int queue_event_manager(void *ptr) {
  ui_event_t e;
  for (int i = 0;; i++) {
    if (GetEvent(&e)) {
      unsigned *ptr_ev = (unsigned *)&e;
      unsigned prime_event = ptr_ev[7];
      if (prime_event == 0x10) { // Key down
	prime_keydown = ptr_ev[8] >> 16;
	unsigned k = prime_translate(prime_keydown);
	SDL_Event ev;
	ev.type = SDL_KEYDOWN;
	ev.key.sym = k;
	push_event(&ev);
      } else if (prime_event == 0x100000) { // Key up
	if (prime_keydown==0x83){
	  rgbSetColor(0xff0000);  rgbSetBkColor(0);  Printf("\f  *** ON pressed *** "); // loopsleep(200);
	  sdk_ctrl_c=1;
	}
	unsigned k = prime_translate(ptr_ev[8] >> 16);
	SDL_Event ev;
	ev.type = SDL_KEYUP;
	ev.key.sym = k;
	push_event(&ev);
	prime_keydown = 0;
      }
      else if (prime_event == 0x1) { // Mouse Push
	prime_mousex = ptr_ev[8] >> 16;
	prime_mousey = ptr_ev[9];
	prime_mousedown = 1;
	// 1. Déplacer la souris sous le point de contact
	enqueue_sdl_mouse_event(SDL_MOUSEMOTION, 0, prime_mousex, prime_mousey);
	// 2. Enfoncer le bouton gauche
	enqueue_sdl_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, prime_mousex, prime_mousey);
      } else if (prime_event == 0x2 && prime_mousedown) { // Mouse Drag
	prime_mousex = ptr_ev[8] >> 16;
	prime_mousey = ptr_ev[9];
	// Déplacement de la souris pendant l'appui
	enqueue_sdl_mouse_event(SDL_MOUSEMOTION, 0, prime_mousex, prime_mousey);
      } else if (prime_event == 0x8 && prime_mousedown) { // Mouse Release
	prime_mousex = ptr_ev[8] >> 16;
	prime_mousey = ptr_ev[9];
	prime_mousedown = 0;
	// Relâchement du bouton gauche
	enqueue_sdl_mouse_event(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, prime_mousex, prime_mousey);
      }
    }
    // Petite pause pour relâcher le CPU si la file GetEvent est vide
    OSSleep(5);
  }
  return 0;
}

// SDL_PollEvent: non blocking read of events
int SDL_PollEvent(SDL_Event *event) {
  DMB;
  if (g_event_queue.head == g_event_queue.tail) 
    return 0;    // Si head == tail, la file est vide
  //Printf("\fpollevent %i",g_event_queue.tail); loopsleep(1000);
  *event = g_event_queue.events[g_event_queue.tail];
  g_event_queue.tail = (g_event_queue.tail + 1) % EVENT_QUEUE_SIZE;
  return 1;
}

int SDL_Init(unsigned flags) {
  if (get_fb() == 2) 
    return -1; // Échec d'initialisation de l'affichage
  start_ticks = millis(); 
  // reset queue
  g_event_queue.head = 0;
  g_event_queue.tail = 0;
  if (!ev_threadptr){
    ev_threadptr = OSCreateThread(queue_event_manager, 0,0,0);
    if (ev_threadptr == NULL) 
      return -1;
  }  
  ++flags; // does not do anything
  return 0; // 0 = Succès selon la spécification SDL
}

void SDL_Quit(void) {
  if (ev_threadptr != NULL) {
    OSTerminateThread((thread_t*) ev_threadptr,0);
    ev_threadptr = NULL;
  }
}

#define SHIFTCTRL(x, y, z) (nspire_ctrl ? (z) : nspire_shift ? (y) : (x))
#define SHIFT(x, y) SHIFTCTRL(x, y, x)
#define CTRL(x, y) SHIFTCTRL(x, x, y)
#define NORMAL(x) SHIFTCTRL(x, x, x)

int ascii_get(int* adaptive_cursor_state){
  if (isKeyPressed(KEY_NSPIRE_CTRL)){
    nspire_ctrl=!nspire_ctrl;
    statusline(0);
    sync_screen();
    return -2;
  }
  if (isKeyPressed(KEY_NSPIRE_SHIFT)){
    nspire_shift=!nspire_shift;
    statusline(0);
    sync_screen();
    return -1;
  }
  *adaptive_cursor_state = SHIFTCTRL(0, 1, 4);
  if (isKeyPressed(KEY_NSPIRE_LEFT)|| isKeyPressed(KEY_NSPIRE_LEFTUP) || isKeyPressed(KEY_NSPIRE_DOWNLEFT))		return SHIFTCTRL(KEY_CTRL_LEFT,KEY_SHIFT_LEFT,KEY_LEFT_CTRL);
  if (isKeyPressed(KEY_NSPIRE_RIGHT)|| isKeyPressed(KEY_NSPIRE_UPRIGHT) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN))		return SHIFTCTRL(KEY_CTRL_RIGHT,KEY_SHIFT_RIGHT,KEY_RIGHT_CTRL);
  if (isKeyPressed(KEY_NSPIRE_UP))		return SHIFTCTRL(KEY_CTRL_UP,KEY_CTRL_PAGEUP,KEY_UP_CTRL);
  if (isKeyPressed(KEY_NSPIRE_DOWN))		return SHIFTCTRL(KEY_CTRL_DOWN,KEY_CTRL_PAGEDOWN,KEY_DOWN_CTRL);
	
  if (isKeyPressed(KEY_NSPIRE_ESC)) return KEY_CTRL_EXIT ;
  if (isKeyPressed(KEY_NSPIRE_HOME)) return KEY_CTRL_MENU ;
  if (isKeyPressed(KEY_NSPIRE_MENU)) return KEY_CTRL_CATALOG ;
  if (isKeyPressed(KEY_NSPIRE_SIN))		return SHIFT(KEY_CHAR_SIN,KEY_CHAR_ASIN);
  if (isKeyPressed(KEY_NSPIRE_COS))		return SHIFT(KEY_CHAR_COS,KEY_CHAR_ACOS);
  if (isKeyPressed(KEY_NSPIRE_TAN))		return SHIFT(KEY_CHAR_TAN,KEY_CHAR_ATAN);
  
  // Characters
  if (isKeyPressed(KEY_NSPIRE_A)) return SHIFTCTRL('a','A',KEY_CTRL_A);
  if (isKeyPressed(KEY_NSPIRE_B)) return SHIFTCTRL('b','B',KEY_BOOK);
  if (isKeyPressed(KEY_NSPIRE_C)) return SHIFTCTRL('c','C',KEY_CTRL_CLIP);
  if (isKeyPressed(KEY_NSPIRE_D)) return SHIFTCTRL('d','D',KEY_CTRL_D);
  if (isKeyPressed(KEY_NSPIRE_E)) return SHIFTCTRL('e','E',KEY_CTRL_F10);
  if (isKeyPressed(KEY_NSPIRE_F)) return SHIFTCTRL('f','F',KEY_CTRL_F11);
  if (isKeyPressed(KEY_NSPIRE_G)) return SHIFTCTRL('g','G',KEY_CTRL_F12);
  if (isKeyPressed(KEY_NSPIRE_H)) return SHIFTCTRL('h','H',KEY_CTRL_F13);
  if (isKeyPressed(KEY_NSPIRE_I)) return SHIFTCTRL('i','I',KEY_CTRL_F14);
  if (isKeyPressed(KEY_NSPIRE_J)) return SHIFTCTRL('j','J',KEY_CTRL_F15);
  if (isKeyPressed(KEY_NSPIRE_K)) return SHIFTCTRL('k','K',KEY_CTRL_AC);
  if (isKeyPressed(KEY_NSPIRE_L)) return SHIFTCTRL('l','L',KEY_CTRL_F14);
  if (isKeyPressed(KEY_NSPIRE_M)) return SHIFTCTRL('m','M',KEY_CTRL_CATALOG);
  if (isKeyPressed(KEY_NSPIRE_N)) return SHIFTCTRL('n','N',KEY_CTRL_N);
  if (isKeyPressed(KEY_NSPIRE_O)) return SHIFTCTRL('o','O',KEY_SHIFT_OPTN);
  if (isKeyPressed(KEY_NSPIRE_P)) return SHIFTCTRL('p','P',KEY_CTRL_PRGM);
  if (isKeyPressed(KEY_NSPIRE_Q)) return SHIFT('q','Q');
  if (isKeyPressed(KEY_NSPIRE_R)) return SHIFTCTRL('r','R',KEY_CTRL_R);
  if (isKeyPressed(KEY_NSPIRE_S)) return SHIFTCTRL('s','S',KEY_CTRL_S);
  if (isKeyPressed(KEY_NSPIRE_T)) return SHIFTCTRL('t','T',KEY_CTRL_T);
  if (isKeyPressed(KEY_NSPIRE_U)) return SHIFTCTRL('u','U',KEY_CTRL_F13);
  if (isKeyPressed(KEY_NSPIRE_V)) return SHIFTCTRL('v','V',KEY_CTRL_PASTE);
  if (isKeyPressed(KEY_NSPIRE_W)) return SHIFT('w','W');
  if (isKeyPressed(KEY_NSPIRE_X)) return SHIFTCTRL('x','X',KEY_CTRL_CUT);
  if (isKeyPressed(KEY_NSPIRE_Y)) return SHIFT('y','Y');
  if (isKeyPressed(KEY_NSPIRE_Z)) return SHIFTCTRL('z','Z',KEY_CTRL_UNDO);

  // Numbers
  if (nspireemu){ // for firebird, redefine ctrl
    if (isKeyPressed(KEY_NSPIRE_0)) return SHIFTCTRL('0',KEY_CTRL_F10,')');
    if (isKeyPressed(KEY_NSPIRE_1)) return SHIFTCTRL('1',KEY_CTRL_F1,'!');
    if (isKeyPressed(KEY_NSPIRE_2)) return SHIFTCTRL('2',KEY_CTRL_F2,'@');
    if (isKeyPressed(KEY_NSPIRE_3)) return SHIFTCTRL('3',KEY_CTRL_F3,'#');
    if (isKeyPressed(KEY_NSPIRE_4)) return SHIFTCTRL('4',KEY_CTRL_F4,'$');
    if (isKeyPressed(KEY_NSPIRE_5)) return SHIFTCTRL('5',KEY_CTRL_F5,'%');
    if (isKeyPressed(KEY_NSPIRE_6)) return SHIFTCTRL('6',KEY_CTRL_F6,'^');
    if (isKeyPressed(KEY_NSPIRE_7)) return SHIFTCTRL('7',KEY_CTRL_F7,'&');
    if (isKeyPressed(KEY_NSPIRE_8)) return SHIFTCTRL('8',KEY_CTRL_F8,'*');
    if (isKeyPressed(KEY_NSPIRE_9)) return SHIFTCTRL('9',KEY_CTRL_F9,'(');
  }
  else {
    if (isKeyPressed(KEY_NSPIRE_0)) return SHIFTCTRL('0',KEY_CTRL_F10,KEY_CTRL_F10);
    if (isKeyPressed(KEY_NSPIRE_1)) return SHIFTCTRL('1',KEY_CTRL_F1,KEY_CTRL_F1);
    if (isKeyPressed(KEY_NSPIRE_2)) return SHIFTCTRL('2',KEY_CTRL_F2,KEY_CTRL_F2);
    if (isKeyPressed(KEY_NSPIRE_3)) return SHIFTCTRL('3',KEY_CTRL_F3,KEY_CTRL_F3);
    if (isKeyPressed(KEY_NSPIRE_4)) return SHIFTCTRL('4',KEY_CTRL_F4,KEY_CTRL_F4);
    if (isKeyPressed(KEY_NSPIRE_5)) return SHIFTCTRL('5',KEY_CTRL_F5,KEY_CTRL_F5);
    if (isKeyPressed(KEY_NSPIRE_6)) return SHIFTCTRL('6',KEY_CTRL_F6,KEY_CTRL_F6);
    if (isKeyPressed(KEY_NSPIRE_7)) return SHIFTCTRL('7',KEY_CTRL_F7,KEY_CTRL_F7);
    if (isKeyPressed(KEY_NSPIRE_8)) return SHIFTCTRL('8',KEY_CTRL_F8,KEY_CTRL_F8);
    if (isKeyPressed(KEY_NSPIRE_9)) return SHIFTCTRL('9',KEY_CTRL_F9,KEY_CTRL_F9);
  }
  
  // Symbols
  if (isKeyPressed(KEY_NSPIRE_FRAC)) return SHIFTCTRL(KEY_EQW_TEMPLATE,KEY_AFFECT,KEY_AFFECT);
  if (isKeyPressed(KEY_NSPIRE_SQU)) return CTRL(KEY_CHAR_SQUARE,KEY_CHAR_ROOT);
  if (isKeyPressed(KEY_NSPIRE_TENX)) return CTRL(KEY_CHAR_EXPN10,KEY_CHAR_LOG);
  if (isKeyPressed(KEY_NSPIRE_eEXP)) return CTRL(KEY_CHAR_EXPN,KEY_CHAR_LN);
  if (isKeyPressed(KEY_NSPIRE_COMMA))		return SHIFTCTRL(',',';',':');
  if (isKeyPressed(KEY_NSPIRE_PERIOD)) 	return SHIFTCTRL('.',KEY_CTRL_F11,':');
  if (isKeyPressed(KEY_NSPIRE_COLON))		return NORMAL(':');
  if (isKeyPressed(KEY_NSPIRE_LP))			return SHIFTCTRL('(',KEY_CTRL_F13,KEY_CHAR_CROCHETS);
  if (isKeyPressed(KEY_NSPIRE_RP))			return SHIFTCTRL(')',KEY_CTRL_F14,KEY_CHAR_ACCOLADES);
  if (isKeyPressed(KEY_NSPIRE_SPACE))		return SHIFTCTRL(' ','_','_');
  if (isKeyPressed(KEY_NSPIRE_DIVIDE))
    return SHIFTCTRL('/','%','\\');
  if (isKeyPressed(KEY_NSPIRE_MULTIPLY))	return SHIFTCTRL('*','\'','\"');
  if (isKeyPressed(KEY_NSPIRE_MINUS))		return SHIFTCTRL('-','_', '<');
  if (isKeyPressed(KEY_NSPIRE_NEGATIVE))	return SHIFTCTRL('-',KEY_CTRL_F12,KEY_CHAR_ANS);
  if (isKeyPressed(KEY_NSPIRE_PLUS))		return SHIFTCTRL('+', KEY_CHAR_NORMAL,'>');
  if (isKeyPressed(KEY_NSPIRE_EQU))		return SHIFTCTRL('=', '|',KEY_CHAR_STORE);
  if (isKeyPressed(KEY_NSPIRE_LTHAN))		return NORMAL('<');
  if (isKeyPressed(KEY_NSPIRE_GTHAN))		return NORMAL('>');
  if (isKeyPressed(KEY_NSPIRE_QUOTE))		return NORMAL('\"');
  if (isKeyPressed(KEY_NSPIRE_APOSTROPHE))	return NORMAL('\'');
  if (isKeyPressed(KEY_NSPIRE_QUES))		return SHIFTCTRL('?','|','!');
  if (isKeyPressed(KEY_NSPIRE_QUESEXCL))	return SHIFTCTRL('?','|','!');
  if (isKeyPressed(KEY_NSPIRE_BAR))		return NORMAL('|');
  if (isKeyPressed(KEY_NSPIRE_EXP))		return SHIFT('^',KEY_CHAR_RECIP);
  if (isKeyPressed(KEY_NSPIRE_EE))		return SHIFTCTRL('&','%', '@');
  if (isKeyPressed(KEY_NSPIRE_PI)) return KEY_CHAR_PI;
  if (isKeyPressed(KEY_NSPIRE_FLAG)) return SHIFTCTRL(';',':',KEY_CHAR_IMGNRY);
  if (isKeyPressed(KEY_NSPIRE_ENTER))		return SHIFTCTRL(KEY_CTRL_OK,'~',KEY_CTRL_EXE);
  if (isKeyPressed(KEY_NSPIRE_TRIG))		return SHIFTCTRL(KEY_CHAR_SIN,KEY_CHAR_COS,KEY_CHAR_TAN);
  
  // Special chars
  if (isKeyPressed(KEY_NSPIRE_SCRATCHPAD)) return SHIFTCTRL(KEY_CTRL_SETUP,KEY_LOAD,KEY_SAVE);
  if (isKeyPressed(KEY_NSPIRE_VAR)) return SHIFTCTRL(KEY_CTRL_VARS,KEY_CHAR_FACTOR,KEY_CHAR_STORE);
  if (isKeyPressed(KEY_NSPIRE_DOC))		return SHIFTCTRL(KEY_CTRL_MENU,KEY_CTRL_SD,KEY_CTRL_INS);
  if (isKeyPressed(KEY_NSPIRE_CAT))		return KEY_BOOK;
  if (isKeyPressed(KEY_NSPIRE_DEL))		return SHIFTCTRL(KEY_CTRL_DEL,KEY_CTRL_DEL,KEY_CTRL_AC);
  if (isKeyPressed(KEY_NSPIRE_RET))		return KEY_CTRL_EXE;
  if (isKeyPressed(KEY_NSPIRE_TAB))		return '\t';
  
  return 0;
}

int handle_f5(){ return 0; }
int iskeydown(int key){
  t_key t=KEY_NSPIRE_SPACE;
  switch (key){
  case 0:
    t=KEY_NSPIRE_LEFT;
    break;
  case 1:
    t=KEY_NSPIRE_UP;
    break;
  case 2:
    t=KEY_NSPIRE_DOWN;
    break;
  case 3:
    t=KEY_NSPIRE_RIGHT;
    break;
  case 4:
    t=KEY_NSPIRE_ENTER;
    break;
  case 5:
    t=KEY_NSPIRE_ESC;
    break;
  case 6:
    t=KEY_NSPIRE_HOME;
    break;
  case 7:
    t=KEY_NSPIRE_MENU;
    break;
  case 12:
    t=KEY_NSPIRE_SHIFT;
    break;
  case 13:
    t=KEY_NSPIRE_CTRL;
    break;
  case 14:
    t=KEY_NSPIRE_SCRATCHPAD;
    break;
  case 15:
    t=KEY_NSPIRE_VAR;
    break;
  case 16:
    t=KEY_NSPIRE_DOC;
    break;
  case 17:
    t=KEY_NSPIRE_DEL;
    break;
  case 18:
    t=KEY_NSPIRE_eEXP;
    break;
  case 19:
    t=KEY_NSPIRE_EQU;
    break;
  case 20:
    t=KEY_NSPIRE_TENX;
    break;
  case 21:
    t=KEY_NSPIRE_I;
    break;
  case 22:
    t=KEY_NSPIRE_COMMA;
    break;
  case 23:
    t=KEY_NSPIRE_EXP;
    break;
  case 24:
    t=KEY_NSPIRE_TRIG;
    break;
  case 25:
    t=KEY_NSPIRE_C;
    break;
  case 26:
    t=KEY_NSPIRE_T;
    break;
  case 27:
    t=KEY_NSPIRE_PI;
    break;
  case 28:
    t=KEY_NSPIRE_S;
    break;
  case 29:
    t=KEY_NSPIRE_SQU;
    break;
  case 30:
    t=KEY_NSPIRE_7;
    break;
  case 31:
    t=KEY_NSPIRE_8;
    break;
  case 32:
    t=KEY_NSPIRE_9;
    break;
  case 33:
    t=KEY_NSPIRE_LP;
    break;
  case 34:
    t=KEY_NSPIRE_RP;
    break;
  case 36:
    t=KEY_NSPIRE_4;
    break;
  case 37:
    t=KEY_NSPIRE_5;
    break;
  case 38:
    t=KEY_NSPIRE_6;
    break;
  case 39:
    t=KEY_NSPIRE_MULTIPLY;
    break;
  case 40:
    t=KEY_NSPIRE_DIVIDE;
    break;
  case 42:
    t=KEY_NSPIRE_1;
    break;
  case 43:
    t=KEY_NSPIRE_2;
    break;
  case 44:
    t=KEY_NSPIRE_3;
    break;
  case 45:
    t=KEY_NSPIRE_PLUS;
    break;
  case 46:
    t=KEY_NSPIRE_MINUS;
    break;
  case 48:
    t=KEY_NSPIRE_0;
    break;
  case 49:
    t=KEY_NSPIRE_PERIOD;
    break;
  case 50:
    t=KEY_NSPIRE_EE;
    break;
  case 51:
    t=KEY_NSPIRE_NEGATIVE;
    break;
  case 52:
    t=KEY_NSPIRE_RET;
    break;
  }
  return isKeyPressed(t);
}


// ? see also ndless-sdk/thirdparty/nspire-io/arch-nspire/nspire.c nio_ascii_get
int getkey(int allow_suspend){
  sync_screen();
  if (shutdown_state)
    return KEY_SHUTDOWN;
  int lastkey=-1;
  unsigned NSPIRE_RTC_ADDR=0x90090000;
  static unsigned lastt=0;
  for (;;){
    unsigned t1= * (volatile unsigned *) NSPIRE_RTC_ADDR;
    if (lastt==0)
      lastt=t1;
    if (t1-lastt>10){
      display_time();
      sync_screen();
    }
    int autosuspend=(t1-lastt>=100);
    if (nspire_exam_mode!=2 &&
	is_cx2 && nspire_ctrl && on_key_pressed()){
      os_fill_rect(50,90,200,40,0x1234);
      nspire_draw_string(60,120,0,0xffff,Regular12,"Quit KhiCAS to shutdown",false);
      nspire_ctrl=false;
      statusline(1);
      continue;
    }
    if ( (nspire_exam_mode==2 || !is_cx2) &&
	allow_suspend && (autosuspend || (nspire_ctrl && on_key_pressed()))){
      nspire_ctrl=nspire_shift=false;
      while (!autosuspend && on_key_pressed())
	loopsleep(10);
      // somewhat OFF by setting LCD to 0
      unsigned NSPIRE_CONTRAST_ADDR=is_cx2?0x90130014:0x900f0020;
      unsigned oldval=*(volatile unsigned *)NSPIRE_CONTRAST_ADDR,oldval2;
      if (is_cx2){
	oldval2=*(volatile unsigned *) (NSPIRE_CONTRAST_ADDR+4);
	*(volatile unsigned *) (NSPIRE_CONTRAST_ADDR+4)=0xffff;
      }
      *(volatile unsigned *)NSPIRE_CONTRAST_ADDR=is_cx2?0xffff:0x100;
      static volatile uint32_t *lcd_controller = (volatile uint32_t*) 0xC0000000;
      lcd_controller[6] &= ~(0b1 << 11);
      loopsleep(20);
      lcd_controller[6] &= ~ 0b1;
      unsigned offtime=* (volatile unsigned *) NSPIRE_RTC_ADDR;
      for (int n=0;!on_key_pressed();++n){
	loopsleep(100);
	idle();
	if (!exam_mode && nspire_exam_mode!=2 && khicas_shutdown
	    // && n&0xff==0
	    ){
	  unsigned curtime=* (volatile unsigned *) NSPIRE_RTC_ADDR;
	  if (curtime-offtime>7200){
	    shutdown_state=1;
	    // after 2 hours, leave KhiCAS
	    // that way the OS will really shutdown the calc
	    lcd_controller[6] |= 0b1;
	    loopsleep(20);
	    lcd_controller[6]|= 0b1 << 11;
	    if (is_cx2)
	      *(volatile unsigned *)(NSPIRE_CONTRAST_ADDR+4)=oldval2;
	    *(volatile unsigned *)NSPIRE_CONTRAST_ADDR=oldval;
	    statuslinemsg("Press ON to disable KhiCAS auto shutdown");
	    //os_fill_rect(0,0,320,222,0xffff);
	    sync_screen();
	    int m=0,mmax=150;
	    for (;m<mmax;++m){
	      if (on_key_pressed()){
		break;
	      }
	      loopsleep(100);
	      idle();
	    }
	    if (m==mmax){
	      if (khicas_shutdown())
		return KEY_SHUTDOWN;
	    }
	    else {
	      shutdown_state=0;
	      break;
	    }
	  }
	}
      }
      if (nspire_exam_mode==2){
	os_fill_rect(20,30,280,150,0x1234);
	os_fill_rect(25,45,270,121,0xffff);
	nspire_draw_string(25,60,0,0xffff,Regular12,"Mode examen de KhiCAS, avec CAS.",false);
	nspire_draw_string(25,77,0,0xffff,Regular12,"Mode conforme à la règlementation",false);
	nspire_draw_string(25,94,0,0xffff,Regular12,"du bac en France (les fichiers non",false);
	nspire_draw_string(25,111,0,0xffff,Regular12,"autorisés ont été effacés et les",false);
	nspire_draw_string(25,128,0,0xffff,Regular12,"sauvegardes sont desactivées).",false);
	nspire_draw_string(25,150,0,0xffff,Regular12,"Quitter KhiCAS (ou appuyer sur reset)",false);
	nspire_draw_string(25,167,0,0xffff,Regular12,"relancera le clignotement des leds.",false);
      }
      lcd_controller[6] |= 0b1;
      loopsleep(20);
      lcd_controller[6]|= 0b1 << 11;
      if (is_cx2)
	*(volatile unsigned *)(NSPIRE_CONTRAST_ADDR+4)=oldval2;
      *(volatile unsigned *)NSPIRE_CONTRAST_ADDR=oldval;
      statusline(0);
      sync_screen();
      lastt=* (volatile unsigned *) NSPIRE_RTC_ADDR;
      continue;
    }
    if (!any_key_pressed()){
      if (nspireemu)
	ck_msleep(50); // 100?
      else // real calculator
	ck_msleep(1);
      continue;
    }
    lastt=t1;
    int cursor_state=0;
    int i=0;
    if (isKeyPressed(KEY_NSPIRE_SHIFT)){
      while (i==0 && isKeyPressed(KEY_NSPIRE_SHIFT)){
	if (isKeyPressed(KEY_NSPIRE_LEFT)|| isKeyPressed(KEY_NSPIRE_LEFTUP) || isKeyPressed(KEY_NSPIRE_DOWNLEFT))
	  i=KEY_SELECT_LEFT;
	if (isKeyPressed(KEY_NSPIRE_RIGHT)|| isKeyPressed(KEY_NSPIRE_UPRIGHT) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN))
	  i=KEY_SELECT_RIGHT;
	if (isKeyPressed(KEY_NSPIRE_UP))
	  i=KEY_SELECT_UP;
	if (isKeyPressed(KEY_NSPIRE_DOWN))
	  i=KEY_SELECT_DOWN;
      }
      if (i!=0){
	nspire_select=true;
      }
      if (i==0){
	nspire_shift=!nspire_shift;
	statusline(0);
	sync_screen();
	i=-1;
      }
    }
    else {
      if (nspire_select){
	nspire_select=nspire_shift=false;
	statusline(0);
	sync_screen();
	continue;
      }
      i=ascii_get(&cursor_state);
    }
    if (i<0){
      wait_no_key_pressed();
      continue;
    }
    if (i==KEY_CTRL_N){
      nspireemu=!nspireemu;
      nspire_ctrl=nspire_shift=false;
      statusline(0);
      sync_screen();
      continue;
    }
    if ( (i>=KEY_CTRL_LEFT && i<=KEY_CTRL_RIGHT) ||
	 (i>=KEY_UP_CTRL && i<=KEY_RIGHT_CTRL) ||
	 (i>=KEY_SELECT_LEFT && i<=KEY_SELECT_RIGHT) ||
	 i==KEY_CTRL_DEL){
      int delay=(lastkey==i)?5:60,j;
      for (j=0;j<delay && any_key_pressed();++j){
	if (nspireemu)
	  ck_msleep(14);
	else
	  ck_msleep(1);
      }
      if (any_key_pressed())
	lastkey=i;
      else 
	lastkey=-1;
    }
    else {
      wait_no_key_pressed();
      lastkey=-1;
    }
    if (nspire_ctrl || nspire_shift){
      nspire_ctrl=nspire_shift=false;
      statusline(0);
      sync_screen();
    }
    return i;
  }
  // void send_key_event(struct s_ns_event* eventbuf, unsigned short keycode_asciicode, INT is_key_up, INT unknown): since r721. Simulate a key event
}

// void idle(void)
// void msleep(unsigned ms)
// cfg_register_fileext(const char *ext, const char *prgm): (since v3.1 r797) associate for Ndless the file extension ext (without leading '.') to the program name prgm. Does nothing if the extension is already registered.

void GetKey(int * key){
  *key=getkey(true);
}

int alphawasactive(int * key){
  if (*key==KEY_DOWN_CTRL){
    *key=KEY_CTRL_DOWN;
    return true;
  }
  if (*key==KEY_UP_CTRL){
    *key=KEY_CTRL_UP;
    return true;
  }
  if (*key==KEY_LEFT_CTRL){
    *key=KEY_CTRL_LEFT;
    return true;
  }
  if (*key==KEY_RIGHT_CTRL){
    *key=KEY_CTRL_RIGHT;
    return true;
  }
  return false;
}

int isalphaactive(){
  return false;//nspire_ctrl;
}

void lock_alpha(){
  //nspire_ctrl=true;
}

int GetSetupSetting(int k){
  if (k!=0x14) return -1;
  if (!isalphaactive()) return 0;
  return 4;
}

void reset_kbd(){
  nspire_ctrl=nspire_shift=false;
}

int on_key_enabled=true;

void enable_back_interrupt(){
  on_key_enabled=true;
}

void disable_back_interrupt(){
  on_key_enabled=false;
}
#else // NSPIRE_NEWLIB

void set_exam_mode(int i){
  exam_mode=i;
}
#endif // NSPIRE_NEWLIB

// drawing strings without OS
#define COLOR_WHITE 0xffffff
#define COLOR_BLACK 0

extern const unsigned char VGA_Ascii_5x8[];
extern const unsigned char VGA_Ascii_6x12[];
extern const unsigned char orp_Ascii_6x12[];
extern const unsigned char VGA_Ascii_8x16[];
extern const unsigned char VGA_Ascii_7x14[];

extern fb_screen_t * fb_screen; 

static int console_x = 0;
static int console_y = 0;

void vGL_set_pixel(unsigned x,unsigned y,int c){
  //if (y==93) printf("glsetp:%d,%d,%d\n",x,y,c);
  if (x>=VIR_LCD_PIX_W || y>=VIR_LCD_PIX_H)
    return;
  fb_screen[x + y * VIR_LCD_PIX_W] = c;
}

void vGL_SetPoint(unsigned int x, unsigned int y, int c)
{
  if ((x >= VIR_LCD_PIX_W)) {
    x = VIR_LCD_PIX_W - 1;
  }
  if ((y >= VIR_LCD_PIX_H)) {
    y = VIR_LCD_PIX_H - 1;
  }
  vGL_set_pixel(x,y,c);
} 

int vGL_GetPoint(unsigned int x,unsigned int y)
{
  if ((x >= VIR_LCD_PIX_W)) {
    x = VIR_LCD_PIX_W - 1;
  }
  if ((y >= VIR_LCD_PIX_H)) {
    y = VIR_LCD_PIX_H - 1;
  }
  return fb_screen[x + y * VIR_LCD_PIX_W];
}  
    
void vGL_putChar(int x0, int y0, char ch, int fg, int bg, int fontSize) {
  int font_w;
  int font_h;
  const unsigned char *pCh;
  unsigned int x = 0, y = 0, i = 0, j = 0; 
 
  if ((ch < ' ') || (ch > '~' + 1)) {
    return;
  } 

  switch (fontSize) {
  case 8:
    font_w = 5;
    font_h = 8;
    pCh = VGA_Ascii_5x8 + (ch - ' ') * font_h;
    break;

  case 12:
    font_w = 6;
    font_h = 12;
    pCh = orp_Ascii_6x12 + (ch - ' ') * font_h;
    break;

  case 16:
    font_w = 8;
    font_h = 16;
    pCh = VGA_Ascii_8x16 + (ch - ' ') * font_h;
    break;

  case 14:
    font_w = 7;
    font_h = 14;
    pCh = VGA_Ascii_7x14 + (ch - ' ') * font_h;
    break;

  default:
    return;
  }

  while (y < font_h) {
    while (x < font_w) {
      if (((x0 + x) < VIR_LCD_PIX_W) && ((y0 + y) < VIR_LCD_PIX_H))
	//fb_screen[(x0 + x) + VIR_LCD_PIX_W * (y0 + y)] = ((*pCh << x) & 0x80U)?fg:bg;
	vGL_set_pixel(x0+x,y0+y, ((*pCh << x) & 0x80U)?fg:bg);
      x++;
    }
    x = 0;
    y++;
    pCh++;
  }

}

int vGL_putString(int x0, int y0, const char *s, int fg, int bg, int fontSize) {
  int font_w=8;
  int font_h=16;
  int len = strlen(s);
  int x = 0, y = 0;

  if (fontSize <= 16) {
    switch (fontSize) {
    case 8:
      font_w = 5;
      break;
    case 12:
      font_w = 6;
      break;
    case 14:
      font_w = 7;
      break;
    case 16:
      font_w = 8;
      break;
    default:
      font_w = 8;
      break;
    }

    font_h = fontSize;
    while (*s) {
      vGL_putChar((x0) + x, (y0) + y, *s, fg, bg, fontSize);
      s++;
      x += font_w;
      if (x > VIR_LCD_PIX_W) {
	break; // or wrap next line?
	x = 0;
	y += font_h;
	if (y > VIR_LCD_PIX_H) {
	  break;
	}
      }
    }
    return x0+x;
  }
    
}

void vGL_clearArea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1) {
  if ((x0 >= VIR_LCD_PIX_W)) {
    x0 = VIR_LCD_PIX_W - 1;
  }
  if ((y0 >= VIR_LCD_PIX_H)) {
    y0 = VIR_LCD_PIX_H - 1;
  }
  if ((x1 >= VIR_LCD_PIX_W)) {
    x1 = VIR_LCD_PIX_W - 1;
  }
  if ((y1 >= VIR_LCD_PIX_H)) {
    y1 = VIR_LCD_PIX_H - 1;
  }

  printf("clra:%d,%d,%d,%d\n", x0, x1, y0, y1);
  for (int y = y0; y < y1; y++){
    for (int x = x0; x < x1; x++) {
      vGL_set_pixel(x,y,COLOR_WHITE); // fb_screen[x + y * VIR_LCD_PIX_W] = COLOR_WHITE;
    }
  }    
    
}

void vGL_setArea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int color) 
{
  if ((x0 >= VIR_LCD_PIX_W)) {
    x0 = VIR_LCD_PIX_W - 1;
  }
  if ((y0 >= VIR_LCD_PIX_H)) {
    y0 = VIR_LCD_PIX_H - 1;
  }
  if ((x1 >= VIR_LCD_PIX_W)) {
    x1 = VIR_LCD_PIX_W - 1;
  }
  if ((y1 >= VIR_LCD_PIX_H)) {
    y1 = VIR_LCD_PIX_H - 1;
  }

  for (int y = y0; y < y1; y++)
    for (int x = x0; x < x1; x++) {
      {
	vGL_set_pixel(x,y,color); // fb_screen[x + y * VIR_LCD_PIX_W] = color;
      }
    }

}

void vGL_ConsLocate(int x, int y)
{
  console_x = x;
  console_y = y;
}

void vGL_ConsOut(char *s, int rev)
{
  if (rev)
    vGL_putString(console_x * 6, console_y * 12, (char *)s, COLOR_WHITE, COLOR_BLACK, 12);
  else
    vGL_putString(console_x * 6, console_y * 12, (char *)s, COLOR_BLACK, COLOR_WHITE, 12);
}


extern int shell_fontw,shell_fonth;


// Created from bdf2c Version 3, (c) 2009, 2010 by Lutz Sammer
//	License AGPLv3: GNU Affero General Public License version 3
/// @{ defines to have human readable font files
#define ________ 0x00
#define _______X 0x01
#define ______X_ 0x02
#define ______XX 0x03
#define _____X__ 0x04
#define _____X_X 0x05
#define _____XX_ 0x06
#define _____XXX 0x07
#define ____X___ 0x08
#define ____X__X 0x09
#define ____X_X_ 0x0A
#define ____X_XX 0x0B
#define ____XX__ 0x0C
#define ____XX_X 0x0D
#define ____XXX_ 0x0E
#define ____XXXX 0x0F
#define ___X____ 0x10
#define ___X___X 0x11
#define ___X__X_ 0x12
#define ___X__XX 0x13
#define ___X_X__ 0x14
#define ___X_X_X 0x15
#define ___X_XX_ 0x16
#define ___X_XXX 0x17
#define ___XX___ 0x18
#define ___XX__X 0x19
#define ___XX_X_ 0x1A
#define ___XX_XX 0x1B
#define ___XXX__ 0x1C
#define ___XXX_X 0x1D
#define ___XXXX_ 0x1E
#define ___XXXXX 0x1F
#define __X_____ 0x20
#define __X____X 0x21
#define __X___X_ 0x22
#define __X___XX 0x23
#define __X__X__ 0x24
#define __X__X_X 0x25
#define __X__XX_ 0x26
#define __X__XXX 0x27
#define __X_X___ 0x28
#define __X_X__X 0x29
#define __X_X_X_ 0x2A
#define __X_X_XX 0x2B
#define __X_XX__ 0x2C
#define __X_XX_X 0x2D
#define __X_XXX_ 0x2E
#define __X_XXXX 0x2F
#define __XX____ 0x30
#define __XX___X 0x31
#define __XX__X_ 0x32
#define __XX__XX 0x33
#define __XX_X__ 0x34
#define __XX_X_X 0x35
#define __XX_XX_ 0x36
#define __XX_XXX 0x37
#define __XXX___ 0x38
#define __XXX__X 0x39
#define __XXX_X_ 0x3A
#define __XXX_XX 0x3B
#define __XXXX__ 0x3C
#define __XXXX_X 0x3D
#define __XXXXX_ 0x3E
#define __XXXXXX 0x3F
#define _X______ 0x40
#define _X_____X 0x41
#define _X____X_ 0x42
#define _X____XX 0x43
#define _X___X__ 0x44
#define _X___X_X 0x45
#define _X___XX_ 0x46
#define _X___XXX 0x47
#define _X__X___ 0x48
#define _X__X__X 0x49
#define _X__X_X_ 0x4A
#define _X__X_XX 0x4B
#define _X__XX__ 0x4C
#define _X__XX_X 0x4D
#define _X__XXX_ 0x4E
#define _X__XXXX 0x4F
#define _X_X____ 0x50
#define _X_X___X 0x51
#define _X_X__X_ 0x52
#define _X_X__XX 0x53
#define _X_X_X__ 0x54
#define _X_X_X_X 0x55
#define _X_X_XX_ 0x56
#define _X_X_XXX 0x57
#define _X_XX___ 0x58
#define _X_XX__X 0x59
#define _X_XX_X_ 0x5A
#define _X_XX_XX 0x5B
#define _X_XXX__ 0x5C
#define _X_XXX_X 0x5D
#define _X_XXXX_ 0x5E
#define _X_XXXXX 0x5F
#define _XX_____ 0x60
#define _XX____X 0x61
#define _XX___X_ 0x62
#define _XX___XX 0x63
#define _XX__X__ 0x64
#define _XX__X_X 0x65
#define _XX__XX_ 0x66
#define _XX__XXX 0x67
#define _XX_X___ 0x68
#define _XX_X__X 0x69
#define _XX_X_X_ 0x6A
#define _XX_X_XX 0x6B
#define _XX_XX__ 0x6C
#define _XX_XX_X 0x6D
#define _XX_XXX_ 0x6E
#define _XX_XXXX 0x6F
#define _XXX____ 0x70
#define _XXX___X 0x71
#define _XXX__X_ 0x72
#define _XXX__XX 0x73
#define _XXX_X__ 0x74
#define _XXX_X_X 0x75
#define _XXX_XX_ 0x76
#define _XXX_XXX 0x77
#define _XXXX___ 0x78
#define _XXXX__X 0x79
#define _XXXX_X_ 0x7A
#define _XXXX_XX 0x7B
#define _XXXXX__ 0x7C
#define _XXXXX_X 0x7D
#define _XXXXXX_ 0x7E
#define _XXXXXXX 0x7F
#define X_______ 0x80
#define X______X 0x81
#define X_____X_ 0x82
#define X_____XX 0x83
#define X____X__ 0x84
#define X____X_X 0x85
#define X____XX_ 0x86
#define X____XXX 0x87
#define X___X___ 0x88
#define X___X__X 0x89
#define X___X_X_ 0x8A
#define X___X_XX 0x8B
#define X___XX__ 0x8C
#define X___XX_X 0x8D
#define X___XXX_ 0x8E
#define X___XXXX 0x8F
#define X__X____ 0x90
#define X__X___X 0x91
#define X__X__X_ 0x92
#define X__X__XX 0x93
#define X__X_X__ 0x94
#define X__X_X_X 0x95
#define X__X_XX_ 0x96
#define X__X_XXX 0x97
#define X__XX___ 0x98
#define X__XX__X 0x99
#define X__XX_X_ 0x9A
#define X__XX_XX 0x9B
#define X__XXX__ 0x9C
#define X__XXX_X 0x9D
#define X__XXXX_ 0x9E
#define X__XXXXX 0x9F
#define X_X_____ 0xA0
#define X_X____X 0xA1
#define X_X___X_ 0xA2
#define X_X___XX 0xA3
#define X_X__X__ 0xA4
#define X_X__X_X 0xA5
#define X_X__XX_ 0xA6
#define X_X__XXX 0xA7
#define X_X_X___ 0xA8
#define X_X_X__X 0xA9
#define X_X_X_X_ 0xAA
#define X_X_X_XX 0xAB
#define X_X_XX__ 0xAC
#define X_X_XX_X 0xAD
#define X_X_XXX_ 0xAE
#define X_X_XXXX 0xAF
#define X_XX____ 0xB0
#define X_XX___X 0xB1
#define X_XX__X_ 0xB2
#define X_XX__XX 0xB3
#define X_XX_X__ 0xB4
#define X_XX_X_X 0xB5
#define X_XX_XX_ 0xB6
#define X_XX_XXX 0xB7
#define X_XXX___ 0xB8
#define X_XXX__X 0xB9
#define X_XXX_X_ 0xBA
#define X_XXX_XX 0xBB
#define X_XXXX__ 0xBC
#define X_XXXX_X 0xBD
#define X_XXXXX_ 0xBE
#define X_XXXXXX 0xBF
#define XX______ 0xC0
#define XX_____X 0xC1
#define XX____X_ 0xC2
#define XX____XX 0xC3
#define XX___X__ 0xC4
#define XX___X_X 0xC5
#define XX___XX_ 0xC6
#define XX___XXX 0xC7
#define XX__X___ 0xC8
#define XX__X__X 0xC9
#define XX__X_X_ 0xCA
#define XX__X_XX 0xCB
#define XX__XX__ 0xCC
#define XX__XX_X 0xCD
#define XX__XXX_ 0xCE
#define XX__XXXX 0xCF
#define XX_X____ 0xD0
#define XX_X___X 0xD1
#define XX_X__X_ 0xD2
#define XX_X__XX 0xD3
#define XX_X_X__ 0xD4
#define XX_X_X_X 0xD5
#define XX_X_XX_ 0xD6
#define XX_X_XXX 0xD7
#define XX_XX___ 0xD8
#define XX_XX__X 0xD9
#define XX_XX_X_ 0xDA
#define XX_XX_XX 0xDB
#define XX_XXX__ 0xDC
#define XX_XXX_X 0xDD
#define XX_XXXX_ 0xDE
#define XX_XXXXX 0xDF
#define XXX_____ 0xE0
#define XXX____X 0xE1
#define XXX___X_ 0xE2
#define XXX___XX 0xE3
#define XXX__X__ 0xE4
#define XXX__X_X 0xE5
#define XXX__XX_ 0xE6
#define XXX__XXX 0xE7
#define XXX_X___ 0xE8
#define XXX_X__X 0xE9
#define XXX_X_X_ 0xEA
#define XXX_X_XX 0xEB
#define XXX_XX__ 0xEC
#define XXX_XX_X 0xED
#define XXX_XXX_ 0xEE
#define XXX_XXXX 0xEF
#define XXXX____ 0xF0
#define XXXX___X 0xF1
#define XXXX__X_ 0xF2
#define XXXX__XX 0xF3
#define XXXX_X__ 0xF4
#define XXXX_X_X 0xF5
#define XXXX_XX_ 0xF6
#define XXXX_XXX 0xF7
#define XXXXX___ 0xF8
#define XXXXX__X 0xF9
#define XXXXX_X_ 0xFA
#define XXXXX_XX 0xFB
#define XXXXXX__ 0xFC
#define XXXXXX_X 0xFD
#define XXXXXXX_ 0xFE
#define XXXXXXXX 0xFF
/// @}


/// character bitmap for each encoding
const unsigned char VGA_Ascii_7x14[] =   {           // ASCII
  //  32 $20 'char32'
  //	width 7, bbx 0, bby 0, bbw 1, bbh 1
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  //  33 $21 'char33'
  //	width 7, bbx 2, bby 0, bbw 2, bbh 10
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  ________,
  ________,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  //  34 $22 'char34'
  //	width 7, bbx 1, bby 6, bbw 5, bbh 4
  _XX_XX__,
  _XX_XX__,
  _XX_XX__,
  _XX_XX__,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  //  35 $23 'char35'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _X__X___,
  _XX_X___,
  XXXXXX__,
  XXXXXX__,
  _X__X___,
  _X__X___,
  XXXXXX__,
  XXXXXX__,
  _X_XX___,
  _X__X___,
  ________,
  ________,
  ________,
  ________,
  //  36 $24 'char36'
  //	width 7, bbx 0, bby -1, bbw 6, bbh 12
  __XX____,
  __XX____,
  _XXXXX__,
  XX__XX__,
  XX______,
  _XXX____,
  __XXX___,
  ____XX__,
  XX__XX__,
  XXXXX___,
  __XX____,
  __XX____,
  ________,
  ________,
  //  37 $25 'char37'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX__XX__,
  XX__XX__,
  ___XX___,
  ___XX___,
  __XX____,
  __XX____,
  _XX_____,
  _XX_____,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  //  38 $26 'char38'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXX____,
  XX______,
  XX__X___,
  XX_XX___,
  _XXXXX__,
  XX_XX___,
  XX_XX___,
  XX_XX___,
  XX_XX___,
  _XX_XX__,
  ________,
  ________,
  ________,
  ________,
  //  39 $27 'char39'
  //	width 7, bbx 2, bby 6, bbw 2, bbh 4
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  //  40 $28 'char40'
  //	width 7, bbx 1, bby -1, bbw 4, bbh 12
  ___XX___,
  __XX____,
  __XX____,
  _XX_____,
  _XX_____,
  _XX_____,
  _XX_____,
  _XX_____,
  _XX_____,
  __XX____,
  __XX____,
  ___XX___,
  ________,
  ________,
  //  41 $29 'char41'
  //	width 7, bbx 1, bby -1, bbw 4, bbh 12
  _XX_____,
  __XX____,
  __XX____,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  __XX____,
  __XX____,
  _XX_____,
  ________,
  ________,
  //  42 $2a 'char42'
  //	width 7, bbx 0, bby 4, bbw 6, bbh 6
  ________,
  ________,
  ________,
  ________,
  __XX____,
  __XX____,
  XXXXXX__,
  __XX____,
  _XXXX___,
  _X__X___,
  ________,
  ________,
  ________,
  ________,
  //  43 $2b 'char43'
  //	width 7, bbx 0, bby 1, bbw 6, bbh 7
  ________,
  ________,
  ________,
  __XX____,
  __XX____,
  __XX____,
  XXXXXX__,
  __XX____,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  //  44 $2c 'char44'
  //	width 7, bbx 1, bby -2, bbw 3, bbh 4
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  __XX____,
  __XX____,
  __XX____,
  _XX_____,
  ________,
  ________,
  //  45 $2d 'char45'
  //	width 7, bbx 0, bby 4, bbw 6, bbh 1
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  //  46 $2e 'char46'
  //	width 7, bbx 2, bby 0, bbw 2, bbh 2
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  //  47 $2f 'char47'
  //	width 7, bbx 0, bby -1, bbw 6, bbh 12
  ____XX__,
  ____XX__,
  ___XX___,
  ___XX___,
  ___XX___,
  __XX____,
  __XX____,
  _XX_____,
  _XX_____,
  _XX_____,
  XX______,
  XX______,
  ________,
  ________,
  //  48 $30 'char48'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXX___,
  XX__XX__,
  XX__XX__,
  XX_XXX__,
  XX_XXX__,
  XXX_XX__,
  XXX_XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  49 $31 'char49'
  //	width 7, bbx 0, bby 0, bbw 4, bbh 10
  __XX____,
  _XXX____,
  XXXX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  //  50 $32 'char50'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXX___,
  XX__XX__,
  XX__XX__,
  ____XX__,
  ___XX___,
  __XX____,
  _XX_____,
  XX______,
  XX______,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  //  51 $33 'char51'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXX___,
  XX__XX__,
  XX__XX__,
  ____XX__,
  __XXX___,
  ____XX__,
  ____XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  52 $34 'char52'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  ___XX___,
  __XXX___,
  __XXX___,
  _XXXX___,
  _X_XX___,
  XX_XX___,
  XX_XX___,
  XXXXXX__,
  ___XX___,
  ___XX___,
  ________,
  ________,
  ________,
  ________,
  //  53 $35 'char53'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXXXX__,
  XX______,
  XX______,
  XXXXX___,
  XX__XX__,
  ____XX__,
  ____XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  54 $36 'char54'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  __XX____,
  __XX____,
  _XX_____,
  _XX_____,
  XXXXX___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  55 $37 'char55'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXXXX__,
  XX__XX__,
  XX__XX__,
  ___XX___,
  ___XX___,
  ___XX___,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  //  56 $38 'char56'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXX___,
  XX__XX__,
  XX__XX__,
  _X_XX___,
  __XX____,
  _XX_X___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  57 $39 'char57'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXX___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _XXXXX__,
  ___XX___,
  ___XX___,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  //  58 $3a 'char58'
  //	width 7, bbx 2, bby 1, bbw 2, bbh 7
  ________,
  ________,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  ________,
  //  59 $3b 'char59'
  //	width 7, bbx 1, bby -1, bbw 3, bbh 9
  ________,
  ________,
  ________,
  ________,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  __XX____,
  __XX____,
  __XX____,
  _XX_____,
  ________,
  //  60 $3c 'char60'
  //	width 7, bbx 0, bby 1, bbw 5, bbh 7
  ________,
  ________,
  ________,
  ___XX___,
  __XX____,
  _XX_____,
  XX______,
  _XX_____,
  __XX____,
  ___XX___,
  ________,
  ________,
  ________,
  ________,
  //  61 $3d 'char61'
  //	width 7, bbx 0, bby 2, bbw 6, bbh 5
  ________,
  ________,
  ________,
  XXXXXX__,
  ________,
  ________,
  ________,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  //  62 $3e 'char62'
  //	width 7, bbx 1, bby 1, bbw 5, bbh 7
  ________,
  ________,
  ________,
  _XX_____,
  __XX____,
  ___XX___,
  ____XX__,
  ___XX___,
  __XX____,
  _XX_____,
  ________,
  ________,
  ________,
  ________,
  //  63 $3f 'char63'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXX___,
  XX__XX__,
  XX__XX__,
  ____XX__,
  ___XX___,
  __XX____,
  ________,
  ________,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  //  64 $40 'char64'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  __XXX___,
  _XX_XX__,
  XX___X__,
  XX_XXX__,
  XX_X_X__,
  XX_X_X__,
  XX_XXX__,
  XX__XX__,
  _XX_____,
  __XXX___,
  ________,
  ________,
  ________,
  ________,
  //  65 $41 'char65'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  __XX____,
  __XX____,
  _XXXX___,
  _X_XX___,
  _X__X___,
  XX__XX__,
  XXXXXX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  //  66 $42 'char66'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXX____,
  XX_XX___,
  XX_XX___,
  XX_XX___,
  XXXX____,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XXXXX___,
  ________,
  ________,
  ________,
  ________,
  //  67 $43 'char67'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXX___,
  XX__XX__,
  XX__XX__,
  XX______,
  XX______,
  XX______,
  XX______,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  68 $44 'char68'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXX____,
  XX_XX___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX_XX___,
  XXXX____,
  ________,
  ________,
  ________,
  ________,
  //  69 $45 'char69'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXXXX__,
  XX______,
  XX______,
  XX______,
  XXXXX___,
  XX______,
  XX______,
  XX______,
  XX______,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  //  70 $46 'char70'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXXXX__,
  XX______,
  XX______,
  XX______,
  XXXXX___,
  XX______,
  XX______,
  XX______,
  XX______,
  XX______,
  ________,
  ________,
  ________,
  ________,
  //  71 $47 'char71'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXX___,
  XX__XX__,
  XX__XX__,
  XX______,
  XX______,
  XX_XXX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _XXX_X__,
  ________,
  ________,
  ________,
  ________,
  //  72 $48 'char72'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XXXXXX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  //  73 $49 'char73'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXXXX__,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  //  74 $4a 'char74'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXXX__,
  ____XX__,
  ____XX__,
  ____XX__,
  ____XX__,
  ____XX__,
  ____XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  75 $4b 'char75'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX__XX__,
  XX__XX__,
  XX_XX___,
  XX_XX___,
  XXXX____,
  XX_XX___,
  XX_XX___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  //  76 $4c 'char76'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX______,
  XX______,
  XX______,
  XX______,
  XX______,
  XX______,
  XX______,
  XX______,
  XX______,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  //  77 $4d 'char77'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX__XX__,
  XX__XX__,
  XXXXXX__,
  XXXXXX__,
  XX_X_X__,
  XX_X_X__,
  XX_X_X__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  //  78 $4e 'char78'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX__XX__,
  XX__XX__,
  XXX_XX__,
  XXX_XX__,
  XXX_XX__,
  XX_XXX__,
  XX_XXX__,
  XX_XXX__,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  //  79 $4f 'char79'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXX___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  80 $50 'char80'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXXX___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XXXXX___,
  XX______,
  XX______,
  XX______,
  XX______,
  ________,
  ________,
  ________,
  ________,
  //  81 $51 'char81'
  //	width 7, bbx 0, bby -2, bbw 6, bbh 12
  _XXXX___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX_XX___,
  _XXX____,
  ___XXX__,
  ____XX__,
  ________,
  ________,
  //  82 $52 'char82'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXXX___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XXXXX___,
  XX_XX___,
  XX_XX___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  //  83 $53 'char83'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXXX___,
  XX__XX__,
  XX__XX__,
  XX______,
  _XX_____,
  ___XX___,
  ____XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  84 $54 'char84'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXXXX__,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  //  85 $55 'char85'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  86 $56 'char86'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _X__X___,
  _X__X___,
  _XXXX___,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  //  87 $57 'char87'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX_X_X__,
  XX_X_X__,
  XX_X_X__,
  XX_X_X__,
  XX_X_X__,
  XXXXXX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  //  88 $58 'char88'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX__XX__,
  XX__XX__,
  _X__X___,
  _XXXX___,
  __XX____,
  __XX____,
  _XXXX___,
  _X__X___,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  //  89 $59 'char89'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _X__X___,
  _XXXX___,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  //  90 $5a 'char90'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XXXXXX__,
  ____XX__,
  ___XX___,
  ___XX___,
  __XX____,
  __XX____,
  _XX_____,
  _XX_____,
  XX______,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  //  91 $5b 'char91'
  //	width 7, bbx 1, bby -1, bbw 4, bbh 12
  _XXXX___,
  _XX_____,
  _XX_____,
  _XX_____,
  _XX_____,
  _XX_____,
  _XX_____,
  _XX_____,
  _XX_____,
  _XX_____,
  _XX_____,
  _XXXX___,
  ________,
  ________,
  //  92 $5c 'char92'
  //	width 7, bbx 0, bby -1, bbw 6, bbh 12
  XX______,
  XX______,
  _XX_____,
  _XX_____,
  _XX_____,
  __XX____,
  __XX____,
  ___XX___,
  ___XX___,
  ___XX___,
  ____XX__,
  ____XX__,
  ________,
  ________,
  //  93 $5d 'char93'
  //	width 7, bbx 1, bby -1, bbw 4, bbh 12
  _XXXX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  _XXXX___,
  ________,
  ________,
  //  94 $5e 'char94'
  //	width 7, bbx 0, bby 4, bbw 6, bbh 6
  __XX____,
  __XX____,
  _XXXX___,
  _XXXX___,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  //  95 $5f 'char95'
  //	width 7, bbx 0, bby -1, bbw 6, bbh 1
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  ________,
  //  96 $60 'char96'
  //	width 7, bbx 1, bby 6, bbw 4, bbh 4
  _XX_____,
  _XXX____,
  __XXX___,
  ___XX___,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  //  97 $61 'char97'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  _XXXX___,
  XX__XX__,
  ____XX__,
  _XXXXX__,
  XX__XX__,
  XX__XX__,
  _XXXXX__,
  ________,
  ________,
  ________,
  ________,
  //  98 $62 'char98'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX______,
  XX______,
  XX______,
  XX_XX___,
  XXX_XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XXXXX___,
  ________,
  ________,
  ________,
  ________,
  //  99 $63 'char99'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  _XXXX___,
  XX__XX__,
  XX__XX__,
  XX______,
  XX______,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  // 100 $64 'char100'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  ____XX__,
  ____XX__,
  ____XX__,
  _XXXXX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX_XXX__,
  _XX_XX__,
  ________,
  ________,
  ________,
  ________,
  // 101 $65 'char101'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  _XXXX___,
  XX__XX__,
  XX__XX__,
  XXXXXX__,
  XX______,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  // 102 $66 'char102'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  ___XXX__,
  __XX____,
  __XX____,
  XXXXXX__,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  // 103 $67 'char103'
  //	width 7, bbx 0, bby -2, bbw 6, bbh 9
  ________,
  ________,
  ________,
  _XXXXX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  _XX_____,
  ___XXX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  // 104 $68 'char104'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX______,
  XX______,
  XX______,
  XX_XX___,
  XXX_XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  // 105 $69 'char105'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  __XX____,
  __XX____,
  ________,
  _XXX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  // 106 $6a 'char106'
  //	width 7, bbx 0, bby -2, bbw 5, bbh 12
  ___XX___,
  ___XX___,
  ________,
  _XXXX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  ___XX___,
  XX_XX___,
  _XXX____,
  ________,
  ________,
  // 107 $6b 'char107'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  XX______,
  XX______,
  XX______,
  XX__XX__,
  XX_XX___,
  XXXX____,
  XXXX____,
  XX_XX___,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  // 108 $6c 'char108'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  _XXX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  // 109 $6d 'char109'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  XX_XX___,
  XXXXXX__,
  XX_X_X__,
  XX_X_X__,
  XX_X_X__,
  XX_X_X__,
  XX_X_X__,
  ________,
  ________,
  ________,
  ________,
  // 110 $6e 'char110'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  XX_XX___,
  XXX_XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  // 111 $6f 'char111'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  _XXXX___,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  // 112 $70 'char112'
  //	width 7, bbx 0, bby -2, bbw 6, bbh 9
  ________,
  ________,
  ________,
  ________,
  XX_XX___,
  XXX_XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XXXXX___,
  XX______,
  XX______,
  XX______,
  ________,
  // 113 $71 'char113'
  //	width 7, bbx 0, bby -2, bbw 6, bbh 9
  ________,
  ________,
  ________,
  ________,
  _XXXXX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX_XXX__,
  _XX_XX__,
  ____XX__,
  ____XX__,
  ____XX__,
  ________,
  // 114 $72 'char114'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  XX_XX___,
  XXX_XX__,
  XX__XX__,
  XX______,
  XX______,
  XX______,
  XX______,
  ________,
  ________,
  ________,
  ________,
  // 115 $73 'char115'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  _XXXX___,
  XX__XX__,
  XX______,
  _XXXX___,
  ____XX__,
  XX__XX__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  // 116 $74 'char116'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 10
  ___X____,
  __XX____,
  __XX____,
  XXXXXX__,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  ___XXX__,
  ________,
  ________,
  ________,
  ________,
  // 117 $75 'char117'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  XX_XXX__,
  _XX_XX__,
  ________,
  ________,
  ________,
  ________,
  // 118 $76 'char118'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _X__X___,
  _XXXX___,
  __XX____,
  __XX____,
  ________,
  ________,
  ________,
  ________,
  // 119 $77 'char119'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  XX__XX__,
  XX__XX__,
  XX_X_X__,
  XX_X_X__,
  XX_X_X__,
  XX_X_X__,
  _XXXX___,
  ________,
  ________,
  ________,
  ________,
  // 120 $78 'char120'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  XX__XX__,
  XX__XX__,
  _XXXX___,
  __XX____,
  _XXXX___,
  XX__XX__,
  XX__XX__,
  ________,
  ________,
  ________,
  ________,
  // 121 $79 'char121'
  //	width 7, bbx 0, bby -2, bbw 6, bbh 9
  ________,
  ________,
  ________,
  XX__XX__,
  XX__XX__,
  XX__XX__,
  _X__X___,
  _XXXX___,
  __XX____,
  __XX____,
  _XX_____,
  _XX_____,
  ________,
  ________,
  // 122 $7a 'char122'
  //	width 7, bbx 0, bby 0, bbw 6, bbh 7
  ________,
  ________,
  ________,
  XXXXXX__,
  ____XX__,
  ___XX___,
  __XX____,
  _XX_____,
  XX______,
  XXXXXX__,
  ________,
  ________,
  ________,
  ________,
  // 123 $7b 'char123'
  //	width 7, bbx 0, bby -1, bbw 6, bbh 12
  ___XXX__,
  __XX____,
  __XX____,
  __XX____,
  ___X____,
  XXX_____,
  XXX_____,
  ___X____,
  __XX____,
  __XX____,
  __XX____,
  ___XXX__,
  ________,
  ________,
  // 124 $7c 'char124'
  //	width 7, bbx 2, bby -1, bbw 2, bbh 12
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  __XX____,
  ________,
  ________,
  // 125 $7d 'char125'
  //	width 7, bbx 0, bby -1, bbw 6, bbh 12
  XXX_____,
  __XX____,
  __XX____,
  __XX____,
  __X_____,
  ___XXX__,
  ___XXX__,
  __X_____,
  __XX____,
  __XX____,
  __XX____,
  XXX_____,
  ________,
  ________,
  // 126 $7e 'char126'
  //	width 7, bbx 0, bby 3, bbw 6, bbh 4
  _XX__X__,
  XXXX_X__,
  X_XXXX__,
  X__XX___,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  // 160 $a0 'char160'
  //	width 7, bbx 0, bby 0, bbw 1, bbh 1
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
  ________,
};

const unsigned char orp_Ascii_6x12[] =              // ASCII
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //    32 ' '
	0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00,0x00, //    33 '!'
	0x00,0x00,0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //    34 '"'
	0x00,0x00,0x00,0x00,0x50,0xF8,0x50,0xF8,0x50,0x00,0x00,0x00, //    35 '#'
	0x00,0x00,0x00,0x20,0x78,0xA0,0x70,0x28,0xF0,0x20,0x00,0x00, //    36 '$'
	0x00,0x00,0x00,0x00,0x00,0x90,0x20,0x40,0x90,0x00,0x00,0x00, //    37 '%'
	0x00,0x00,0x00,0x40,0xA0,0xA0,0x40,0xA8,0x90,0x68,0x00,0x00, //    38 '&'
	0x00,0x00,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //    39 '''
	0x00,0x00,0x10,0x20,0x40,0x40,0x40,0x40,0x40,0x20,0x10,0x00, //    40 '('
	0x00,0x00,0x40,0x20,0x10,0x10,0x10,0x10,0x10,0x20,0x40,0x00, //    41 ')'
	0x00,0x00,0x00,0x20,0xA8,0x70,0x70,0xA8,0x20,0x00,0x00,0x00, //    42 '*'
	0x00,0x00,0x00,0x00,0x20,0x20,0xF8,0x20,0x20,0x00,0x00,0x00, //    43 '+'
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0xC0,0x00, //    44 ','
	0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00, //    45 '-'
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x00, //    46 '.'
	0x00,0x00,0x00,0x08,0x08,0x10,0x20,0x40,0x80,0x80,0x00,0x00, //    47 '/'
	0x00,0x00,0x00,0x70,0x88,0x88,0xA8,0x88,0x88,0x70,0x00,0x00, //    48 '0'
	0x00,0x00,0x00,0x20,0x60,0xA0,0x20,0x20,0x20,0xF8,0x00,0x00, //    49 '1'
	0x00,0x00,0x00,0x70,0x88,0x08,0x10,0x20,0x40,0xF8,0x00,0x00, //    50 '2'
	0x00,0x00,0x00,0xF8,0x08,0x10,0x30,0x08,0x88,0x70,0x00,0x00, //    51 '3'
	0x00,0x00,0x00,0x10,0x30,0x50,0x90,0xF8,0x10,0x38,0x00,0x00, //    52 '4'
	0x00,0x00,0x00,0xF8,0x80,0xF0,0x08,0x08,0x88,0x70,0x00,0x00, //    53 '5'
	0x00,0x00,0x00,0x70,0x80,0xF0,0x88,0xA8,0x88,0x70,0x00,0x00, //    54 '6'
	0x00,0x00,0x00,0xF8,0x08,0x10,0x10,0x20,0x20,0x20,0x00,0x00, //    55 '7'
	0x00,0x00,0x00,0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00,0x00, //    56 '8'
	0x00,0x00,0x00,0x70,0x88,0xA8,0x88,0x78,0x08,0x70,0x00,0x00, //    57 '9'
	0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x60,0x60,0x00,0x00, //    58 ':'
	0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x60,0x60,0xC0,0x00, //    59 ';'
	0x00,0x00,0x00,0x00,0x10,0x20,0x40,0x20,0x10,0x00,0x00,0x00, //    60 '<'
	0x00,0x00,0x00,0x00,0x00,0xF8,0x00,0xF8,0x00,0x00,0x00,0x00, //    61 '='
	0x00,0x00,0x00,0x00,0x40,0x20,0x10,0x20,0x40,0x00,0x00,0x00, //    62 '>'
	0x00,0x00,0x00,0x70,0x88,0x08,0x10,0x20,0x00,0x20,0x00,0x00, //    63 '?'
	0x00,0x00,0x00,0x70,0x88,0xB8,0xA8,0xB8,0x80,0x70,0x00,0x00, //    64 '@'
	0x00,0x00,0x00,0x20,0x50,0x88,0x88,0xF8,0x88,0x88,0x00,0x00, //    65 'A'
	0x00,0x00,0x00,0xF0,0x88,0x88,0xF0,0x88,0x88,0xF0,0x00,0x00, //    66 'B'
	0x00,0x00,0x00,0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x00,0x00, //    67 'C'
	0x00,0x00,0x00,0xF0,0x88,0x88,0x88,0x88,0x88,0xF0,0x00,0x00, //    68 'D'
	0x00,0x00,0x00,0xF8,0x80,0x80,0xF0,0x80,0x80,0xF8,0x00,0x00, //    69 'E'
	0x00,0x00,0x00,0xF8,0x80,0x80,0xF0,0x80,0x80,0x80,0x00,0x00, //    70 'F'
	0x00,0x00,0x00,0x70,0x88,0x80,0x98,0x88,0x88,0x70,0x00,0x00, //    71 'G'
	0x00,0x00,0x00,0x88,0x88,0x88,0xF8,0x88,0x88,0x88,0x00,0x00, //    72 'H'
	0x00,0x00,0x00,0xF8,0x20,0x20,0x20,0x20,0x20,0xF8,0x00,0x00, //    73 'I'
	0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x88,0x70,0x00,0x00, //    74 'J'
	0x00,0x00,0x00,0x88,0x90,0xA0,0xC0,0xA0,0x90,0x88,0x00,0x00, //    75 'K'
	0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0xF8,0x00,0x00, //    76 'L'
	0x00,0x00,0x00,0x88,0xD8,0xA8,0xA8,0x88,0x88,0x88,0x00,0x00, //    77 'M'
	0x00,0x00,0x00,0x88,0xC8,0xA8,0x98,0x88,0x88,0x88,0x00,0x00, //    78 'N'
	0x00,0x00,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00, //    79 'O'
	0x00,0x00,0x00,0xF0,0x88,0x88,0xF0,0x80,0x80,0x80,0x00,0x00, //    80 'P'
	0x00,0x00,0x00,0x70,0x88,0x88,0x88,0x88,0xA8,0x70,0x08,0x00, //    81 'Q'
	0x00,0x00,0x00,0xF0,0x88,0x88,0xF0,0x88,0x88,0x88,0x00,0x00, //    82 'R'
	0x00,0x00,0x00,0x70,0x88,0x80,0x70,0x08,0x88,0x70,0x00,0x00, //    83 'S'
	0x00,0x00,0x00,0xF8,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00, //    84 'T'
	0x00,0x00,0x00,0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00, //    85 'U'
	0x00,0x00,0x00,0x88,0x88,0x88,0x88,0x50,0x50,0x20,0x00,0x00, //    86 'V'
	0x00,0x00,0x00,0x88,0x88,0x88,0x88,0xA8,0xA8,0xD8,0x00,0x00, //    87 'W'
	0x00,0x00,0x00,0x88,0x88,0x50,0x20,0x50,0x88,0x88,0x00,0x00, //    88 'X'
	0x00,0x00,0x00,0x88,0x88,0x88,0x70,0x20,0x20,0x20,0x00,0x00, //    89 'Y'
	0x00,0x00,0x00,0xF8,0x08,0x10,0x20,0x40,0x80,0xF8,0x00,0x00, //    90 'Z'
	0x00,0x00,0x70,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x70,0x00, //    91 '['
	0x00,0x00,0x00,0x80,0x80,0x40,0x20,0x10,0x08,0x08,0x00,0x00, //    92 '\'
	0x00,0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x70,0x00, //    93 ']'
	0x00,0x00,0x20,0x50,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //    94 '^'
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF8, //    95 '_'
	0x00,0x00,0x40,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //    96 '`'
	0x00,0x00,0x00,0x00,0x00,0x78,0x88,0x88,0x98,0x68,0x00,0x00, //    97 'a'
	0x00,0x00,0x00,0x80,0x80,0xF0,0x88,0x88,0x88,0xF0,0x00,0x00, //    98 'b'
	0x00,0x00,0x00,0x00,0x00,0x70,0x88,0x80,0x80,0x78,0x00,0x00, //    99 'c'
	0x00,0x00,0x00,0x08,0x08,0x78,0x88,0x88,0x88,0x78,0x00,0x00, //   100 'd'
	0x00,0x00,0x00,0x00,0x00,0x70,0x88,0xF8,0x80,0x78,0x00,0x00, //   101 'e'
	0x00,0x00,0x00,0x18,0x20,0x70,0x20,0x20,0x20,0x20,0x00,0x00, //   102 'f'
	0x00,0x00,0x00,0x00,0x00,0x78,0x88,0x88,0x88,0x78,0x88,0x70, //   103 'g'
	0x00,0x00,0x00,0x80,0x80,0xF0,0x88,0x88,0x88,0x88,0x00,0x00, //   104 'h'
	0x00,0x00,0x00,0x20,0x00,0x60,0x20,0x20,0x20,0x30,0x00,0x00, //   105 'i'
	0x00,0x00,0x00,0x20,0x00,0x60,0x20,0x20,0x20,0x20,0x20,0xC0, //   106 'j'
	0x00,0x00,0x00,0x80,0x80,0x88,0x90,0xA0,0xD0,0x88,0x00,0x00, //   107 'k'
	0x00,0x00,0x00,0x60,0x20,0x20,0x20,0x20,0x20,0x30,0x00,0x00, //   108 'l'
	0x00,0x00,0x00,0x00,0x00,0xF0,0xA8,0xA8,0xA8,0xA8,0x00,0x00, //   109 'm'
	0x00,0x00,0x00,0x00,0x00,0xB0,0xC8,0x88,0x88,0x88,0x00,0x00, //   110 'n'
	0x00,0x00,0x00,0x00,0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00, //   111 'o'
	0x00,0x00,0x00,0x00,0x00,0xF0,0x88,0x88,0x88,0xF0,0x80,0x80, //   112 'p'
	0x00,0x00,0x00,0x00,0x00,0x78,0x88,0x88,0x88,0x78,0x08,0x08, //   113 'q'
	0x00,0x00,0x00,0x00,0x00,0xB0,0xC8,0x80,0x80,0x80,0x00,0x00, //   114 'r'
	0x00,0x00,0x00,0x00,0x00,0x78,0x80,0x70,0x08,0xF0,0x00,0x00, //   115 's'
	0x00,0x00,0x00,0x20,0x20,0x70,0x20,0x20,0x20,0x18,0x00,0x00, //   116 't'
	0x00,0x00,0x00,0x00,0x00,0x88,0x88,0x88,0x98,0x68,0x00,0x00, //   117 'u'
	0x00,0x00,0x00,0x00,0x00,0x88,0x88,0x50,0x50,0x20,0x00,0x00, //   118 'v'
	0x00,0x00,0x00,0x00,0x00,0xA8,0xA8,0xA8,0xA8,0x50,0x00,0x00, //   119 'w'
	0x00,0x00,0x00,0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00,0x00, //   120 'x'
	0x00,0x00,0x00,0x00,0x00,0x88,0x88,0x88,0x88,0x78,0x88,0x70, //   121 'y'
	0x00,0x00,0x00,0x00,0x00,0xF8,0x10,0x20,0x40,0xF8,0x00,0x00, //   122 'z'
	0x00,0x00,0x10,0x20,0x20,0x20,0xC0,0x20,0x20,0x20,0x10,0x00, //   123 '{'
	0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00, //   124 '|'
	0x00,0x00,0x40,0x20,0x20,0x20,0x18,0x20,0x20,0x20,0x40,0x00, //   125 '}'
	0x00,0x00,0x00,0x00,0x00,0x48,0xA8,0x90,0x00,0x00,0x00,0x00, //   126 '~'
};

const unsigned char VGA_Ascii_5x8[] =              // ASCII
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // - -

	0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00, // -!-

	0x50,0x50,0x50,0x00,0x00,0x00,0x00,0x00, // -"-

	0x50,0x50,0xF8,0x50,0xF8,0x50,0x50,0x00, // -#-

	0x20,0x78,0xC0,0x70,0x28,0xF0,0x20,0x00, // -$-

	0xC0,0xC8,0x10,0x20,0x40,0x98,0x18,0x00, // -%-

	0x40,0xA0,0xA0,0x40,0xA8,0x90,0x68,0x00, // -&-

	0x30,0x20,0x40,0x00,0x00,0x00,0x00,0x00, // -'-

	0x10,0x20,0x40,0x40,0x40,0x20,0x10,0x00, // -(-

	0x40,0x20,0x10,0x10,0x10,0x20,0x40,0x00, // -)-

	0x20,0xA8,0x70,0x20,0x70,0xA8,0x20,0x00, // -*-

	0x20,0x20,0x20,0xF8,0x20,0x20,0x20,0x00, // -+-

	0x00,0x00,0x00,0x00,0x60,0x40,0x80,0x00, // -,-

	0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00, // ---

	0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00, // -.-

	0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00, // -/-

	0x70,0x88,0x98,0xA8,0xC8,0x88,0x70,0x00, // -0-

	0x20,0x60,0x20,0x20,0x20,0x20,0x70,0x00, // -1-

	0x70,0x88,0x08,0x30,0x40,0x80,0xF8,0x00, // -2-

	0xF8,0x08,0x10,0x30,0x08,0x88,0x70,0x00, // -3-

	0x10,0x30,0x50,0x90,0xF8,0x10,0x10,0x00, // -4-

	0xF8,0x80,0xF0,0x08,0x08,0x88,0x70,0x00, // -5-

	0x38,0x40,0x80,0xF0,0x88,0x88,0x70,0x00, // -6-

	0xF8,0x08,0x10,0x20,0x40,0x40,0x40,0x00, // -7-

	0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00, // -8-

	0x70,0x88,0x88,0x78,0x08,0x10,0xE0,0x00, // -9-

	0x00,0x60,0x60,0x00,0x60,0x60,0x00,0x00, // -:-

	0x00,0x60,0x60,0x00,0x60,0x60,0x80,0x00, // -;-

	0x10,0x20,0x40,0x80,0x40,0x20,0x10,0x00, // -<-

	0x00,0x00,0xF8,0x00,0xF8,0x00,0x00,0x00, // -=-

	0x40,0x20,0x10,0x08,0x10,0x20,0x40,0x00, // ->-

	0x70,0x88,0x10,0x20,0x20,0x00,0x20,0x00, // -?-

	0x70,0x88,0xB8,0xA8,0xB8,0x80,0x78,0x00, // -@-

	0x20,0x50,0x88,0x88,0xF8,0x88,0x88,0x00, // -A-

	0xF0,0x88,0x88,0xF0,0x88,0x88,0xF0,0x00, // -B-

	0x70,0x88,0x80,0x80,0x80,0x88,0x70,0x00, // -C-

	0xF0,0x88,0x88,0x88,0x88,0x88,0xF0,0x00, // -D-

	0xF8,0x80,0x80,0xF0,0x80,0x80,0xF8,0x00, // -E-

	0xF8,0x80,0x80,0xF0,0x80,0x80,0x80,0x00, // -F-

	0x70,0x88,0x80,0x80,0xB8,0x88,0x78,0x00, // -G-

	0x88,0x88,0x88,0xF8,0x88,0x88,0x88,0x00, // -H-

	0x70,0x20,0x20,0x20,0x20,0x20,0x70,0x00, // -I-

	0x38,0x10,0x10,0x10,0x10,0x90,0x60,0x00, // -J-

	0x88,0x90,0xA0,0xC0,0xA0,0x90,0x88,0x00, // -K-

	0x80,0x80,0x80,0x80,0x80,0x80,0xF8,0x00, // -L-

	0x88,0xD8,0xA8,0xA8,0x88,0x88,0x88,0x00, // -M-

	0x88,0x88,0xC8,0xA8,0x98,0x88,0x88,0x00, // -N-

	0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00, // -O-

	0xF0,0x88,0x88,0xF0,0x80,0x80,0x80,0x00, // -P-

	0x70,0x88,0x88,0x88,0xA8,0x90,0x68,0x00, // -Q-

	0xF0,0x88,0x88,0xF0,0xA0,0x90,0x88,0x00, // -R-

	0x70,0x88,0x80,0x70,0x08,0x88,0x70,0x00, // -S-

	0xF8,0x20,0x20,0x20,0x20,0x20,0x20,0x00, // -T-

	0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00, // -U-

	0x88,0x88,0x88,0x88,0x88,0x50,0x20,0x00, // -V-

	0x88,0x88,0x88,0xA8,0xA8,0xD8,0x88,0x00, // -W-

	0x88,0x88,0x50,0x20,0x50,0x88,0x88,0x00, // -X-

	0x88,0x88,0x50,0x20,0x20,0x20,0x20,0x00, // -Y-

	0xF8,0x08,0x10,0x20,0x40,0x80,0xF8,0x00, // -Z-

	0xF0,0xC0,0xC0,0xC0,0xC0,0xC0,0xF0,0x00, // -[-

	0x00,0x80,0x40,0x20,0x10,0x08,0x00,0x00, // -\-

	0x78,0x18,0x18,0x18,0x18,0x18,0x78,0x00, // -]-

	0x20,0x70,0xA8,0x20,0x20,0x20,0x20,0x00, // -^-

	0x00,0x20,0x40,0xF8,0x40,0x20,0x00,0x00, // -_-

	0x20,0x10,0x08,0x00,0x00,0x00,0x00,0x00, // -`-

	0x00,0x00,0xE0,0x10,0x70,0x90,0x68,0x00, // -a-

	0x80,0x80,0xB0,0xC8,0x88,0xC8,0xB0,0x00, // -b-

	0x00,0x00,0x70,0x88,0x80,0x80,0x70,0x00, // -c-

	0x08,0x08,0x68,0x98,0x88,0x98,0x68,0x00, // -d-

	0x00,0x00,0x70,0x88,0xF0,0x80,0x70,0x00, // -e-

	0x30,0x48,0x40,0xF0,0x40,0x40,0x40,0x00, // -f-

	0x00,0x00,0x70,0x88,0x88,0x78,0x08,0xF0, // -g-

	0x80,0x80,0xB0,0xC8,0x88,0x88,0x88,0x00, // -h-

	0x20,0x00,0x00,0x20,0x20,0x20,0x20,0x00, // -i-

	0x10,0x00,0x00,0x30,0x10,0x10,0x10,0x60, // -j-

	0x80,0x80,0x90,0xA0,0xC0,0xA0,0x98,0x00, // -k-

	0x60,0x20,0x20,0x20,0x20,0x20,0x70,0x00, // -l-

	0x00,0x00,0x50,0xA8,0xA8,0xA8,0xA8,0x00, // -m-

	0x00,0x00,0xB0,0x48,0x48,0x48,0x48,0x00, // -n-

	0x00,0x00,0x70,0x88,0x88,0x88,0x70,0x00, // -o-

	0x00,0x00,0xF0,0x88,0x88,0xF0,0x80,0x80, // -p-

	0x00,0x00,0x78,0x88,0x88,0x78,0x08,0x08, // -q-

	0x00,0x00,0xB0,0x48,0x40,0x40,0x40,0x00, // -r-

	0x00,0x00,0x78,0x80,0x70,0x08,0xF0,0x00, // -s-

	0x40,0x40,0xF8,0x40,0x40,0x48,0x30,0x00, // -t-

	0x00,0x00,0x90,0x90,0x90,0x90,0x68,0x00, // -u-

	0x00,0x00,0x88,0x88,0x88,0x50,0x20,0x00, // -v-

	0x00,0x00,0xA8,0xA8,0xA8,0xA8,0x50,0x00, // -w-

	0x00,0x00,0x88,0x50,0x20,0x50,0x88,0x00, // -x-

	0x00,0x00,0x88,0x88,0x98,0x68,0x08,0xF0, // -y-

	0x00,0x00,0xF8,0x10,0x20,0x40,0xF8,0x00, // -z-

	0x20,0x40,0x40,0x80,0x40,0x40,0x20,0x00, // -{-

	0x20,0x20,0x20,0x00,0x20,0x20,0x20,0x00, // -|-

	0x20,0x10,0x10,0x08,0x10,0x10,0x20,0x00, // -}-

	0x00,0x00,0x40,0xA8,0x10,0x00,0x00,0x00, // -~-

	0xA8,0x50,0xA8,0x50,0xA8,0x50,0xA8,0x00, // --
};

 
const unsigned char VGA_Ascii_6x12[] =              // ASCII
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // - -
	0x00,0x00,0x00,0x00,

	0x00,0x10,0x10,0x10,0x10,0x10,0x00,0x00,  // -!-
	0x10,0x00,0x00,0x00,

	0x00,0x6C,0x48,0x48,0x00,0x00,0x00,0x00,  // -"-
	0x00,0x00,0x00,0x00,

	0x00,0x14,0x14,0x28,0x7C,0x28,0x7C,0x28,  // -#-
	0x50,0x50,0x00,0x00,

	0x00,0x10,0x38,0x40,0x40,0x38,0x48,0x70,  // -$-
	0x10,0x10,0x00,0x00,

	0x00,0x20,0x50,0x20,0x0C,0x70,0x08,0x14,  // -%-
	0x08,0x00,0x00,0x00,

	0x00,0x00,0x00,0x18,0x20,0x20,0x54,0x48,  // -&-
	0x34,0x00,0x00,0x00,

	0x00,0x10,0x10,0x10,0x10,0x00,0x00,0x00,  // -'-
	0x00,0x00,0x00,0x00,

	0x00,0x08,0x08,0x10,0x10,0x10,0x10,0x10,  // -(-
	0x10,0x08,0x08,0x00,

	0x00,0x20,0x20,0x10,0x10,0x10,0x10,0x10,  // -)-
	0x10,0x20,0x20,0x00,

	0x00,0x10,0x7C,0x10,0x28,0x28,0x00,0x00,  // -*-
	0x00,0x00,0x00,0x00,

	0x00,0x00,0x10,0x10,0x10,0xFC,0x10,0x10,  // -+-
	0x10,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,  // -,-
	0x10,0x30,0x20,0x00,

	0x00,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,  // ---
	0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,  // -.-
	0x30,0x00,0x00,0x00,

	0x00,0x04,0x04,0x08,0x08,0x10,0x10,0x20,  // -/-
	0x20,0x40,0x00,0x00,

	0x00,0x38,0x44,0x44,0x44,0x44,0x44,0x44,  // -0-
	0x38,0x00,0x00,0x00,

	0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x10,  // -1-
	0x7C,0x00,0x00,0x00,

	0x00,0x38,0x44,0x04,0x08,0x10,0x20,0x44,  // -2-
	0x7C,0x00,0x00,0x00,

	0x00,0x38,0x44,0x04,0x18,0x04,0x04,0x44,  // -3-
	0x38,0x00,0x00,0x00,

	0x00,0x0C,0x14,0x14,0x24,0x44,0x7C,0x04,  // -4-
	0x0C,0x00,0x00,0x00,

	0x00,0x3C,0x20,0x20,0x38,0x04,0x04,0x44,  // -5-
	0x38,0x00,0x00,0x00,

	0x00,0x1C,0x20,0x40,0x78,0x44,0x44,0x44,  // -6-
	0x38,0x00,0x00,0x00,

	0x00,0x7C,0x44,0x04,0x08,0x08,0x08,0x10,  // -7-
	0x10,0x00,0x00,0x00,

	0x00,0x38,0x44,0x44,0x38,0x44,0x44,0x44,  // -8-
	0x38,0x00,0x00,0x00,

	0x00,0x38,0x44,0x44,0x44,0x3C,0x04,0x08,  // -9-
	0x70,0x00,0x00,0x00,

	0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x30,  // -:-
	0x30,0x00,0x00,0x00,

	0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x18,  // -;-
	0x30,0x20,0x00,0x00,

	0x00,0x00,0x0C,0x10,0x60,0x80,0x60,0x10,  // -<-
	0x0C,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x7C,0x00,0x7C,0x00,  // -=-
	0x00,0x00,0x00,0x00,

	0x00,0x00,0xC0,0x20,0x18,0x04,0x18,0x20,  // ->-
	0xC0,0x00,0x00,0x00,

	0x00,0x00,0x18,0x24,0x04,0x08,0x10,0x00,  // -?-
	0x30,0x00,0x00,0x00,

	0x38,0x44,0x44,0x4C,0x54,0x54,0x4C,0x40,  // -@-
	0x44,0x38,0x00,0x00,

	0x00,0x30,0x10,0x28,0x28,0x28,0x7C,0x44,  // -A-
	0xEC,0x00,0x00,0x00,

	0x00,0xF8,0x44,0x44,0x78,0x44,0x44,0x44,  // -B-
	0xF8,0x00,0x00,0x00,

	0x00,0x3C,0x44,0x40,0x40,0x40,0x40,0x44,  // -C-
	0x38,0x00,0x00,0x00,

	0x00,0xF0,0x48,0x44,0x44,0x44,0x44,0x48,  // -D-
	0xF0,0x00,0x00,0x00,

	0x00,0xFC,0x44,0x50,0x70,0x50,0x40,0x44,  // -E-
	0xFC,0x00,0x00,0x00,

	0x00,0x7C,0x20,0x28,0x38,0x28,0x20,0x20,  // -F-
	0x70,0x00,0x00,0x00,

	0x00,0x3C,0x44,0x40,0x40,0x4C,0x44,0x44,  // -G-
	0x38,0x00,0x00,0x00,

	0x00,0xEC,0x44,0x44,0x7C,0x44,0x44,0x44,  // -H-
	0xEC,0x00,0x00,0x00,

	0x00,0x7C,0x10,0x10,0x10,0x10,0x10,0x10,  // -I-
	0x7C,0x00,0x00,0x00,

	0x00,0x3C,0x08,0x08,0x08,0x48,0x48,0x48,  // -J-
	0x30,0x00,0x00,0x00,

	0x00,0xEC,0x44,0x48,0x50,0x70,0x48,0x44,  // -K-
	0xE4,0x00,0x00,0x00,

	0x00,0x70,0x20,0x20,0x20,0x20,0x24,0x24,  // -L-
	0x7C,0x00,0x00,0x00,

	0x00,0xEC,0x6C,0x6C,0x54,0x54,0x44,0x44,  // -M-
	0xEC,0x00,0x00,0x00,

	0x00,0xEC,0x64,0x64,0x54,0x54,0x54,0x4C,  // -N-
	0xEC,0x00,0x00,0x00,

	0x00,0x38,0x44,0x44,0x44,0x44,0x44,0x44,  // -O-
	0x38,0x00,0x00,0x00,

	0x00,0x78,0x24,0x24,0x24,0x38,0x20,0x20,  // -P-
	0x70,0x00,0x00,0x00,

	0x00,0x38,0x44,0x44,0x44,0x44,0x44,0x44,  // -Q-
	0x38,0x1C,0x00,0x00,

	0x00,0xF8,0x44,0x44,0x44,0x78,0x48,0x44,  // -R-
	0xE0,0x00,0x00,0x00,

	0x00,0x34,0x4C,0x40,0x38,0x04,0x04,0x64,  // -S-
	0x58,0x00,0x00,0x00,

	0x00,0xFC,0x90,0x10,0x10,0x10,0x10,0x10,  // -T-
	0x38,0x00,0x00,0x00,

	0x00,0xEC,0x44,0x44,0x44,0x44,0x44,0x44,  // -U-
	0x38,0x00,0x00,0x00,

	0x00,0xEC,0x44,0x44,0x28,0x28,0x28,0x10,  // -V-
	0x10,0x00,0x00,0x00,

	0x00,0xEC,0x44,0x44,0x54,0x54,0x54,0x54,  // -W-
	0x28,0x00,0x00,0x00,

	0x00,0xC4,0x44,0x28,0x10,0x10,0x28,0x44,  // -X-
	0xC4,0x00,0x00,0x00,

	0x00,0xEC,0x44,0x28,0x28,0x10,0x10,0x10,  // -Y-
	0x38,0x00,0x00,0x00,

	0x00,0x7C,0x44,0x08,0x10,0x10,0x20,0x44,  // -Z-
	0x7C,0x00,0x00,0x00,

	0x00,0x38,0x20,0x20,0x20,0x20,0x20,0x20,  // -[-
	0x20,0x20,0x38,0x00,

	0x00,0x40,0x20,0x20,0x20,0x10,0x10,0x08,  // -\-
	0x08,0x08,0x00,0x00,

	0x00,0x38,0x08,0x08,0x08,0x08,0x08,0x08,  // -]-
	0x08,0x08,0x38,0x00,

	0x00,0x10,0x10,0x28,0x44,0x00,0x00,0x00,  // -^-
	0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // -_-
	0x00,0x00,0x00,0xFC,

	0x00,0x10,0x08,0x00,0x00,0x00,0x00,0x00,  // -`-
	0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x38,0x44,0x3C,0x44,0x44,  // -a-
	0x3C,0x00,0x00,0x00,

	0x00,0xC0,0x40,0x58,0x64,0x44,0x44,0x44,  // -b-
	0xF8,0x00,0x00,0x00,

	0x00,0x00,0x00,0x3C,0x44,0x40,0x40,0x44,  // -c-
	0x38,0x00,0x00,0x00,

	0x00,0x0C,0x04,0x34,0x4C,0x44,0x44,0x44,  // -d-
	0x3C,0x00,0x00,0x00,

	0x00,0x00,0x00,0x38,0x44,0x7C,0x40,0x40,  // -e-
	0x3C,0x00,0x00,0x00,

	0x00,0x1C,0x20,0x7C,0x20,0x20,0x20,0x20,  // -f-
	0x7C,0x00,0x00,0x00,

	0x00,0x00,0x00,0x34,0x4C,0x44,0x44,0x44,  // -g-
	0x3C,0x04,0x38,0x00,

	0x00,0xC0,0x40,0x58,0x64,0x44,0x44,0x44,  // -h-
	0xEC,0x00,0x00,0x00,

	0x00,0x10,0x00,0x70,0x10,0x10,0x10,0x10,  // -i-
	0x7C,0x00,0x00,0x00,

	0x00,0x10,0x00,0x78,0x08,0x08,0x08,0x08,  // -j-
	0x08,0x08,0x70,0x00,

	0x00,0xC0,0x40,0x5C,0x48,0x70,0x50,0x48,  // -k-
	0xDC,0x00,0x00,0x00,

	0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x10,  // -l-
	0x7C,0x00,0x00,0x00,

	0x00,0x00,0x00,0xE8,0x54,0x54,0x54,0x54,  // -m-
	0xFC,0x00,0x00,0x00,

	0x00,0x00,0x00,0xD8,0x64,0x44,0x44,0x44,  // -n-
	0xEC,0x00,0x00,0x00,

	0x00,0x00,0x00,0x38,0x44,0x44,0x44,0x44,  // -o-
	0x38,0x00,0x00,0x00,

	0x00,0x00,0x00,0xD8,0x64,0x44,0x44,0x44,  // -p-
	0x78,0x40,0xE0,0x00,

	0x00,0x00,0x00,0x34,0x4C,0x44,0x44,0x44,  // -q-
	0x3C,0x04,0x0C,0x00,

	0x00,0x00,0x00,0x6C,0x30,0x20,0x20,0x20,  // -r-
	0x7C,0x00,0x00,0x00,

	0x00,0x00,0x00,0x3C,0x44,0x38,0x04,0x44,  // -s-
	0x78,0x00,0x00,0x00,

	0x00,0x00,0x20,0x7C,0x20,0x20,0x20,0x20,  // -t-
	0x1C,0x00,0x00,0x00,

	0x00,0x00,0x00,0xCC,0x44,0x44,0x44,0x4C,  // -u-
	0x34,0x00,0x00,0x00,

	0x00,0x00,0x00,0xEC,0x44,0x44,0x28,0x28,  // -v-
	0x10,0x00,0x00,0x00,

	0x00,0x00,0x00,0xEC,0x44,0x54,0x54,0x54,  // -w-
	0x28,0x00,0x00,0x00,

	0x00,0x00,0x00,0xCC,0x48,0x30,0x30,0x48,  // -x-
	0xCC,0x00,0x00,0x00,

	0x00,0x00,0x00,0xEC,0x44,0x24,0x28,0x18,  // -y-
	0x10,0x10,0x78,0x00,

	0x00,0x00,0x00,0x7C,0x48,0x10,0x20,0x44,  // -z-
	0x7C,0x00,0x00,0x00,

	0x00,0x08,0x10,0x10,0x10,0x10,0x20,0x10,  // -{-
	0x10,0x10,0x08,0x00,

	0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,  // -|-
	0x10,0x10,0x00,0x00,

	0x00,0x20,0x10,0x10,0x10,0x10,0x08,0x10,  // -}-
	0x10,0x10,0x20,0x00,

	0x00,0x00,0x00,0x00,0x00,0x24,0x58,0x00,  // -~-
	0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x24,0x58,0x00,  // --
	0x00,0x00,0x00,0x00,
};

const unsigned char VGA_Ascii_8x16[] =              // ASCII
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // - -
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x18,0x3C,0x3C,0x3C,0x18,0x18,  // -!-
	0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00,

	0x00,0x66,0x66,0x66,0x24,0x00,0x00,0x00,  // -"-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x6C,0x6C,0xFE,0x6C,0x6C,  // -#-
	0x6C,0xFE,0x6C,0x6C,0x00,0x00,0x00,0x00,

	0x18,0x18,0x7C,0xC6,0xC2,0xC0,0x7C,0x06,  // -$-
	0x86,0xC6,0x7C,0x18,0x18,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0xC2,0xC6,0x0C,0x18,  // -%-
	0x30,0x60,0xC6,0x86,0x00,0x00,0x00,0x00,

	0x00,0x00,0x38,0x6C,0x6C,0x38,0x76,0xDC,  // -&-
	0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,

	0x00,0x30,0x30,0x30,0x60,0x00,0x00,0x00,  // -'-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x0C,0x18,0x30,0x30,0x30,0x30,  // -(-
	0x30,0x30,0x18,0x0C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x30,0x18,0x0C,0x0C,0x0C,0x0C,  // -)-
	0x0C,0x0C,0x18,0x30,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x66,0x3C,0xFF,  // -*-
	0x3C,0x66,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x7E,  // -+-
	0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // -,-
	0x00,0x18,0x18,0x18,0x30,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,  // ---
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // -.-
	0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x02,0x06,0x0C,0x18,  // -/-
	0x30,0x60,0xC0,0x80,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0xCE,0xD6,0xD6,  // -0-
	0xE6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x18,0x38,0x78,0x18,0x18,0x18,  // -1-
	0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0x06,0x0C,0x18,0x30,  // -2-
	0x60,0xC0,0xC6,0xFE,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0x06,0x06,0x3C,0x06,  // -3-
	0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,  // -4-
	0x0C,0x0C,0x0C,0x1E,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFE,0xC0,0xC0,0xC0,0xFC,0x0E,  // -5-
	0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x38,0x60,0xC0,0xC0,0xFC,0xC6,  // -6-
	0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFE,0xC6,0x06,0x06,0x0C,0x18,  // -7-
	0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0xC6,  // -8-
	0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7E,0x06,  // -9-
	0x06,0x06,0x0C,0x78,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,  // -:-
	0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,  // -;-
	0x00,0x18,0x18,0x30,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x06,0x0C,0x18,0x30,0x60,  // -<-
	0x30,0x18,0x0C,0x06,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,  // -=-
	0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x60,0x30,0x18,0x0C,0x06,  // ->-
	0x0C,0x18,0x30,0x60,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0x0C,0x18,0x18,  // -?-
	0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x7C,0xC6,0xC6,0xDE,0xDE,  // -@-
	0xDE,0xDC,0xC0,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,  // -A-
	0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x66,  // -B-
	0x66,0x66,0x66,0xFC,0x00,0x00,0x00,0x00,

	0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xC0,  // -C-
	0xC0,0xC2,0x66,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xF8,0x6C,0x66,0x66,0x66,0x66,  // -D-
	0x66,0x66,0x6C,0xF8,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,  // -E-
	0x60,0x62,0x66,0xFE,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,  // -F-
	0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,

	0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xDE,  // -G-
	0xC6,0xC6,0x66,0x3A,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xFE,0xC6,  // -H-
	0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x3C,0x18,0x18,0x18,0x18,0x18,  // -I-
	0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,  // -J-
	0xCC,0xCC,0xCC,0x78,0x00,0x00,0x00,0x00,

	0x00,0x00,0xE6,0x66,0x6C,0x6C,0x78,0x78,  // -K-
	0x6C,0x66,0x66,0xE6,0x00,0x00,0x00,0x00,

	0x00,0x00,0xF0,0x60,0x60,0x60,0x60,0x60,  // -L-
	0x60,0x62,0x66,0xFE,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,  // -M-
	0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,  // -N-
	0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x38,0x6C,0xC6,0xC6,0xC6,0xC6,  // -O-
	0xC6,0xC6,0x6C,0x38,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x60,  // -P-
	0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,  // -Q-
	0xC6,0xD6,0xDE,0x7C,0x0C,0x0E,0x00,0x00,

	0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x6C,  // -R-
	0x66,0x66,0x66,0xE6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7C,0xC6,0xC6,0x60,0x38,0x0C,  // -S-
	0x06,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x7E,0x7E,0x5A,0x18,0x18,0x18,  // -T-
	0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,  // -U-
	0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,  // -V-
	0xC6,0x6C,0x38,0x10,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xD6,  // -W-
	0xD6,0xFE,0x6C,0x6C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xC6,0xC6,0x6C,0x6C,0x38,0x38,  // -X-
	0x6C,0x6C,0xC6,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x66,0x66,0x66,0x66,0x3C,0x18,  // -Y-
	0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0xFE,0xC6,0x86,0x0C,0x18,0x30,  // -Z-
	0x60,0xC2,0xC6,0xFE,0x00,0x00,0x00,0x00,

	0x00,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,  // -[-
	0x30,0x30,0x30,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x80,0xC0,0xE0,0x70,0x38,  // -\-
	0x1C,0x0E,0x06,0x02,0x00,0x00,0x00,0x00,

	0x00,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,  // -]-
	0x0C,0x0C,0x0C,0x3C,0x00,0x00,0x00,0x00,

	0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,  // -^-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // -_-
	0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,

	0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,  // -`-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x78,0x0C,0x7C,  // -a-
	0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,

	0x00,0x00,0xE0,0x60,0x60,0x78,0x6C,0x66,  // -b-
	0x66,0x66,0x66,0xDC,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC0,  // -c-
	0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x1C,0x0C,0x0C,0x3C,0x6C,0xCC,  // -d-
	0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xFE,  // -e-
	0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x38,0x6C,0x64,0x60,0xF0,0x60,  // -f-
	0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,  // -g-
	0xCC,0xCC,0xCC,0x7C,0x0C,0xCC,0x78,0x00,

	0x00,0x00,0xE0,0x60,0x60,0x6C,0x76,0x66,  // -h-
	0x66,0x66,0x66,0xE6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x18,0x18,0x00,0x38,0x18,0x18,  // -i-
	0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x06,0x06,0x00,0x0E,0x06,0x06,  // -j-
	0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00,

	0x00,0x00,0xE0,0x60,0x60,0x66,0x6C,0x78,  // -k-
	0x78,0x6C,0x66,0xE6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,  // -l-
	0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xEC,0xFE,0xD6,  // -m-
	0xD6,0xD6,0xD6,0xD6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,  // -n-
	0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC6,  // -o-
	0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,  // -p-
	0x66,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00,

	0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,  // -q-
	0xCC,0xCC,0xCC,0x7C,0x0C,0x0C,0x1E,0x00,

	0x00,0x00,0x00,0x00,0x00,0xDC,0x76,0x62,  // -r-
	0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0x60,  // -s-
	0x38,0x0C,0xC6,0x7C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x10,0x30,0x30,0xFC,0x30,0x30,  // -t-
	0x30,0x30,0x36,0x1C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0xCC,  // -u-
	0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,  // -v-
	0x66,0x66,0x3C,0x18,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,  // -w-
	0xD6,0xD6,0xFE,0x6C,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xC6,0x6C,0x38,  // -x-
	0x38,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,  // -y-
	0xC6,0xC6,0xC6,0x7E,0x06,0x0C,0xF8,0x00,

	0x00,0x00,0x00,0x00,0x00,0xFE,0xCC,0x18,  // -z-
	0x30,0x60,0xC6,0xFE,0x00,0x00,0x00,0x00,

	0x00,0x00,0x0E,0x18,0x18,0x18,0x70,0x18,  // -{-
	0x18,0x18,0x18,0x0E,0x00,0x00,0x00,0x00,

	0x00,0x00,0x18,0x18,0x18,0x18,0x00,0x18,  // -|-
	0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00,

	0x00,0x00,0x70,0x18,0x18,0x18,0x0E,0x18,  // -}-
	0x18,0x18,0x18,0x70,0x00,0x00,0x00,0x00,

	0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00,  // -~-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x10,0x38,0x6C,0xC6,  // --
	0xC6,0xC6,0xFE,0x00,0x00,0x00,0x00,0x00,
};
