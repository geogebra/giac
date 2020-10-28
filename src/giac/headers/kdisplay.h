// -*- mode:C++ ; compile-command: "g++ -I.. -g -c Equation.cc" -*-
#ifndef _KDISPLAY_H
#define _KDISPLAY_H
#include "config.h"
#include "giacPCH.h"
#ifdef KHICAS
#include "misc.h"

#include <exception>

class autoshutdown : public std::exception{
  const char * what () const throw ()
  {
    return "autoshutdown";
  }
};

extern  const int LCD_WIDTH_PX;
extern   const int LCD_HEIGHT_PX;
extern char* fmenu_cfg;
#define STATUS_AREA_PX 0 // 24
#define GIAC_HISTORY_MAX_TAILLE 32
#define GIAC_HISTORY_SIZE 2

#ifdef MICROPY_LIB
extern "C" {
  int do_file(const char *file);
  char * micropy_init(int stack_size,int heap_size);
  int micropy_eval(const char * line);
  void  mp_deinit();
  void mp_stack_ctrl_init();
  extern int parser_errorline,parser_errorcol;
  void python_free();
}
int micropy_ck_eval(const char *line);
#endif

#include "k_csdk.h"

extern "C" {
  int do_shutdown(); // auto-shutdown
  void console_output(const char *,int );
  const char * console_input(const char * msg1,const char * msg2,bool numeric,int ypos);
  void c_draw_rectangle(int x,int y,int w,int h,int c);
  void c_fill_rect(int x,int y,int w,int h,int c);
  void c_draw_line(int x0,int y0,int x1,int y1,int c);
  void c_draw_circle(int xc,int yc,int r,int color,bool q1,bool q2,bool q3,bool q4);
  void c_draw_filled_circle(int xc,int yc,int r,int color,bool left,bool right);
  void c_draw_polygon(int * x,int *y ,int n,int color);
  void c_draw_filled_polygon(int * x,int *y, int n,int xmin,int xmax,int ymin,int ymax,int color);
  void c_draw_arc(int xc,int yc,int rx,int ry,int color,double theta1, double theta2);
  void c_draw_filled_arc(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int color,int xmin,int xmax,int ymin,int ymax,bool segment);
  void c_set_pixel(int x,int y,int c);
  int c_draw_string(int x,int y,int c,int bg,const char * s,bool fake);
  int c_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake);
  int c_draw_string_medium(int x,int y,int c,int bg,const char * s,bool fake);
  int select_item(const char ** ptr,const char * title,bool askfor1=true);
  // C conversion to gen from atomic data type 
  unsigned long long c_double2gen(double); 
  unsigned long long c_int2gen(int);
  // linalg on double matrices
  void doubleptr2matrice(double * x,int n,giac::matrice & m);
  bool matrice2doubleptr(const giac::matrice &M,double *x); // x must have enough space
  bool r_inv(double *,int n);
  bool r_rref(double *,int n,int m);
  double r_det(double *,int);
  struct double_pair {
    double r,i;
  } ;
  typedef struct double_pair c_complex;
  bool matrice2c_complexptr(const giac::matrice &M,c_complex *x);
  void c_complexptr2matrice(c_complex * x,int n,int m,giac::matrice & M);  
  bool c_inv(c_complex *,int n);
  bool c_rref(c_complex *,int n,int m);
  c_complex c_det(c_complex *,int);
  bool c_egv(c_complex * x,int n); // eigenvectors
  bool c_eig(c_complex * x,c_complex * d,int n); // x eigenvect, d reduced mat
  bool c_proot(c_complex * x,int n); // poly root
  bool c_pcoeff(c_complex * x,int n); // root->coeffs
  bool c_fft(c_complex * x,int n,bool inverse); // FFT
  void turtle_freeze();
  void c_sprint_double(char * s,double d);
  extern int python_stack_size,python_heap_size;
  extern int xcas_python_eval;
  extern char * python_heap;
  int python_init(int stack_size,int heap_size);
}
extern int lang;
extern short int nspirelua;
extern bool warn_nr;
int select_interpreter(); // 0 Xcas, 1|2 Xcas python_compat(1|2), 3 MicroPython 
const char * gettext(const char * s) ;

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS
  void set_exam_mode(int i,const giac::context *);
  int giac_filebrowser(char * filename,const char * extension,const char * title);
  void draw_rectangle(int x,int y,int w,int h,int c);
  void draw_line(int x0,int y0,int x1,int y1,int c);
  void draw_circle(int xc,int yc,int r,int color,bool q1=true,bool q2=true,bool q3=true,bool q4=true);
  void draw_filled_circle(int xc,int yc,int r,int color,bool left=true,bool right=true);
  void draw_polygon(std::vector< std::vector<int> > & v1,int color);
  void draw_filled_polygon(std::vector< vector<int> > &L,int xmin,int xmax,int ymin,int ymax,int color);
  void draw_arc(int xc,int yc,int rx,int ry,int color,double theta1, double theta2);
  void draw_filled_arc(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int color,int xmin,int xmax,int ymin,int ymax,bool segment);

  //syntax colored printing
  int print_color(int print_x,int print_y,const char *s,int color,bool invert,bool minimini,const giac::context * contextptr);
  bool tooltip(int x,int y,int pos,const char * editline,const giac::context * contextptr);
			   
  bool textedit(char * s,int bufsize,bool OKparse,const giac::context * contextptr,const char * filename=0);
  bool textedit(char * s,int bufsize,const giac::context * contextptr);
  // maximum "size" of symbolics displayed in an Equation (pretty print)
  extern unsigned max_prettyprint_equation;
  // matrix select
  bool eqw_select(const giac::gen & eq,int l,int c,bool select,giac::gen & value);
  void Equation_select(giac::gen & eql,bool select);
  int eqw_select_down(giac::gen & g);
  int eqw_select_up(giac::gen & g);

  giac::gen Equation_compute_size(const giac::gen & g,const giac::attributs & a,int windowhsize,const giac::context * contextptr);
  giac::eqwdata Equation_total_size(const giac::gen & g);  
  // Equation_translate(giac::gen & g,int deltax,int deltay);
  void Equation_vertical_adjust(int hp,int yp,int & h,int & y);
  bool Equation_find_vector_pos(giac::const_iterateur it,giac::const_iterateur itend,int & i,int &nrows);
  bool Equation_adjust_xy(giac::gen & g,int & xleft,int & ytop,int & xright,int & ybottom,giac::gen * & gsel,giac::gen * & gselparent,int &gselpos,std::vector<int> * gotosel=0);
  // select and set value from eqwdata in eql
  bool do_select(giac::gen & eql,bool select,giac::gen & value);

  class Equation {
    int _x,_y;
  public:
    giac::gen data,undodata; // of type eqwdata or undef if empty
    giac::attributs attr;
    const giac::context * contextptr;
    int x() const { return _x;}
    int y() const { return _y;}
    Equation(int x_, int y_, const giac::gen & g,const giac::context *);
  };

  void display(Equation &eq ,int x,int y,const giac::context *);
  // replace selection in eq by tmp
  void replace_selection(Equation & eq,const giac::gen & tmp,giac::gen * gsel,const std::vector<int> * gotoptr,const giac::context *);
  int eqw_select_leftright(xcas::Equation & g,bool left,int exchange,const giac::context *);

  class Graph2d{
  public:
    double window_xmin,window_xmax,window_ymin,window_ymax,
      x_scale,y_scale,x_tick,y_tick;
    int display_mode,show_axes,show_names,labelsize;
    giac::gen g;
    const giac::context * contextptr;
    bool findij(const giac::gen & e0,double x_scale,double y_scale,double & i0,double & j0,const giac::context * ) const;
    void update();
    void zoomx(double d,bool round=false);
    void zoomy(double d,bool round=false);
    void zoom(double);
    void left(double d);
    void right(double d);
    void up(double d);
    void down(double d);
    void autoscale(bool fullview=false);
    void orthonormalize();
    void draw();
    Graph2d(const giac::gen & g_,const giac::context * );
  };

  struct Turtle {
    void draw();
#ifdef TURTLETAB
    giac::logo_turtle * turtleptr;
#else
    std::vector<giac::logo_turtle> * turtleptr;
#endif
    int turtlex,turtley; // Turtle translate
    double turtlezoom; // Zoom factor for turtle screen
    short int maillage=0; // 0 (none), 1 (square), 2 (triangle), bit3 used for printing
    short int speed=0;
  };
  
  int run(const char * s,int do_logo_graph_eqw,const giac::context * contextptr);
  int displaygraph(const giac::gen & ge, const giac::context * contextptr);
  int displaylogo();
  giac::gen eqw(const giac::gen & ge,bool editable,const giac::context * contextptr);
  typedef short int color_t;
  typedef struct
  {
    std::string s;
    color_t color=giac::_BLACK;
    short int newLine=0; // if 1, new line will be drawn before the text
    short int spaceAtEnd=0;
    short int lineSpacing=0;
    short int minimini=0;
    int nlines=1;
  } textElement;

#define TEXTAREATYPE_NORMAL 0
#define TEXTAREATYPE_INSTANT_RETURN 1
  typedef struct
  {
    int x=0;
    int y=0;
    int line=0,undoline=0;
    int pos=0,undopos=0;
    int clipline,undoclipline;
    int clippos,undoclippos;
    int width=LCD_WIDTH_PX;
    int lineHeight=17;
    std::vector<textElement> elements,undoelements;
    const char* title = NULL;
    std::string filename;
    int scrollbar=1;
    bool allowEXE=false; //whether to allow EXE to exit the screen
    bool allowF1=false; //whether to allow F1 to exit the screen
    bool OKparse=true;
    bool editable=false;
    bool changed=false;
    int python=0;
    int type=TEXTAREATYPE_NORMAL;
    int cursorx,cursory; // where the last cursor was displayed
  } textArea;

#define TEXTAREA_RETURN_EXIT 0
#define TEXTAREA_RETURN_EXE 1
#define TEXTAREA_RETURN_F1 2
  int doTextArea(textArea* text,const giac::context * contextptr); //returns 0 when user EXITs, 1 when allowEXE is true and user presses EXE, 2 when allowF1 is true and user presses F1.
  std::string merge_area(const std::vector<textElement> & v);
  void save_script(const char * filename,const std::string & s);
  void add(textArea *edptr,const std::string & s);

  extern textArea * edptr;
  std::string get_searchitem(std::string & replace);
  int check_leave(textArea * text);
  void reload_edptr(const char * filename,textArea *edptr,const giac::context *);
  void print(int &X,int&Y,const char * buf,int color,bool revert,bool fake,bool minimini);

  void save_session(const giac::context * );
#if 1
#define MAX_FILENAME_SIZE 63
  void save_console_state_smem(const char * filename,bool xwaspy,const giac::context *);
  bool load_console_state_smem(const char * filename,const giac::context *);

  struct DISPBOX {
    int     left;
    int     top;
    int     right;
    int     bottom;
    unsigned char mode;
  } ;


  enum CONSOLE_RETURN_VAL {
			   CONSOLE_NEW_LINE_SET = 1,
			   CONSOLE_SUCCEEDED = 0,
			   CONSOLE_MEM_ERR = -1,
			   CONSOLE_ARG_ERR = -2,
			   CONSOLE_NO_EVENT = -3,
  };

  enum CONSOLE_CURSOR_DIRECTION{
				CURSOR_UP,
				CURSOR_DOWN,
				CURSOR_LEFT,
				CURSOR_RIGHT,
				CURSOR_SHIFT_LEFT,
				CURSOR_SHIFT_RIGHT,
				CURSOR_ALPHA_UP,
				CURSOR_ALPHA_DOWN,
  };

  enum CONSOLE_LINE_TYPE{
			 LINE_TYPE_INPUT=0,
			 LINE_TYPE_OUTPUT=1
  };

  enum CONSOLE_CASE{
		    LOWER_CASE,
		    UPPER_CASE
  };

  enum CONSOLE_SCREEN_SPEC {
#ifdef NSPIRE_NEWLIB
			    _LINE_MAX = 128,
			    COL_DISP_MAX = 32,
#else
			    _LINE_MAX = 48,
			    COL_DISP_MAX = 30,//32
#endif
			    LINE_DISP_MAX = 11,
			    EDIT_LINE_MAX = 2048
  };
  
  struct console_line {
    char *str;
    short int readonly;
    short int type;
    int start_col;
    int disp_len;
  };

  struct FMenu{
    char* name;
    char** str;
    unsigned char count;
  };

  struct location{
    int x;
    int y;
  };

#define MAX_FMENU_ITEMS 8
#define FMENU_TITLE_LENGHT 4

#define is_wchar(c) ((c == 0x7F) || (c == 0xF7) || (c == 0xF9) || (c == 0xE5) || (c == 0xE6) || (c == 0xE7))
#define printf(s) Console_Output((const char *)s);

  int Console_DelStr(char *str, int end_pos, int n);
  int Console_InsStr(char *dest, const char *src, int disp_pos);
  int Console_GetActualPos(const char *str, int disp_pos);
  int Console_GetDispLen(const char *str);
  int Console_MoveCursor(int direction);
  int Console_Input(const char *str);
  int Console_Output(const char *str);
  void Console_Clear_EditLine();
  int Console_NewLine(int pre_line_type, int pre_line_readonly);
  int Console_Backspace(const giac::context *);
  int Console_GetKey(const giac::context *);
  int Console_Init(const giac::context *);
  void Console_Free();
  int Console_Disp(int redraw_mode,const giac::context*ptr);
  int Console_FMenu(int key,const giac::context *);
  extern char menu_f1[8],menu_f2[8],menu_f3[8],menu_f4[8],menu_f5[8],menu_f6[8];
  const char * console_menu(int key,char* cfg,int active_app);
  void Console_FMenu_Init(const giac::context *);
  const char * Console_Draw_FMenu(int key, struct FMenu* menu,char * cfg_,int active_app);
  char *Console_Make_Entry(const char* str);
  char *Console_GetLine(const giac::context *);
  char* Console_GetEditLine();
  void dConsolePut(const char *);
  void dConsolePutChar(const char );
  void dConsoleRedraw(void);
  extern int dconsole_mode;
  extern int console_changed; // 1 if something new in history
  extern char session_filename[MAX_FILENAME_SIZE+1];
  const char * input_matrix(bool list,const giac::context *);
  void warn_python(int python,bool autochange=false);
  // void draw_menu(int editor); // 0 console, 1 editor
  int get_set_session_setting(int value);
  void menu_setup(const giac::context *);
  int console_main(const giac::context *,const char * sessionname="session");
#endif
  int periodic_table(const char * & name,const char * & symbol,char * protons,char * nucleons,char * mass,char * electroneg);

  struct tableur {
    giac::matrice m,clip,undo;
    giac::gen var;
    int nrows,ncols;
    int cur_row,cur_col,disp_row_begin,disp_col_begin;
    int sel_row_begin,sel_col_begin;
    std::string cmdline,filename;
    int cmd_pos,cmd_row,cmd_col; // row/col of current cmdline, -1 if not active
    bool changed,recompute,matrix_fill_cells,movedown,keytooltip;
  } ;
  extern tableur * sheetptr;

  int check_do_graph(giac::gen & ge,int do_logo_graph_eqw,const giac::context *);
  int get_filename(char * filename,const char * extension);
  int find_color(const char * s,const giac::context *);

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

giac::gen sheet(const giac::context *); // in kadd.cc
/* ************************************************************
**************************************************************
***********************************************************  */


#ifndef NO_NAMESPACE_XCAS
namespace giac {
#endif // ndef NO_NAMESPACE_XCAS
  // back: number of char that should be deleted,
  // exec=0 or MENU_RETURN_SELECTION, KEY_CHAR_ANS or KEY_CTRL_EXE
  std::string help_insert(const char * cmdline,int & back,int exec,const giac::context *);
  void copy_clipboard(const std::string & s,bool status);
#define TEXT_MODE_NORMAL 0
#define TEXT_MODE_INVERT 1
#define MENUITEM_NORMAL 0
#define MENUITEM_CHECKBOX 1
#define MENUITEM_SEPARATOR 2
#define MENUITEM_VALUE_NONE 0
#define MENUITEM_VALUE_CHECKED 1
  typedef struct
  {
    char* text; // text to be shown on screen. mandatory, must be a valid pointer to a string.
    int token; // for syntax help on keywords not in the catalog
    short int isfolder=0; // for file browsers, this will signal the item is a folder
    signed char color=giac::_BLACK; // color of the menu item (use TEXT_COLOR_* to define)
    signed char type=MENUITEM_NORMAL; // type of the menu item. use MENUITEM_* to set this
    signed char value=MENUITEM_VALUE_NONE; // value of the menu item. For example, if type is MENUITEM_CHECKBOX and the checkbox is checked, the value of this var will be MENUITEM_VALUE_CHECKED
    // The following two settings require the menu type to be set to MENUTYPE_MULTISELECT
    signed char isselected=0; // for file browsers and other multi-select screens, this will show an arrow before the item
    signed char icon=-1; //for file browsers, to show a file icon. -1 shows no icon (default)
  } MenuItem;

  typedef struct
  {
    unsigned short data[0x12*0x18];
  } MenuItemIcon;

#define MENUTYPE_NORMAL 0
#define MENUTYPE_MULTISELECT 1
#define MENUTYPE_INSTANT_RETURN 2 // this type of menu insantly returns even if user hasn't selected an option (allows for e.g. redrawing the GUI behind it). if user hasn't exited or selected an option, menu will return MENU_RETURN_INSTANT
#define MENUTYPE_NO_KEY_HANDLING 3 //this type of menu doesn't handle any keys, only draws.
#define MENUTYPE_FKEYS 4 // returns GetKey value of a Fkey when one is pressed
  typedef struct {
    char* statusText = NULL; // text to be shown on the status bar, may be empty
    char* title = NULL; // title to be shown on the first line if not null
    char* subtitle = NULL;
    int titleColor=giac::_BLUE; //color of the title
    char* nodatamsg; // message to show when there are no menu items to display
    int startX=1; //X where to start drawing the menu. NOTE this is not absolute pixel coordinates but rather character coordinates
    int startY=0; //Y where to start drawing the menu. NOTE this is not absolute pixel coordinates but rather character coordinates
    int width=30; // NOTE this is not absolute pixel coordinates but rather character coordinates
    int height=12; // NOTE this is not absolute pixel coordinates but rather character coordinates
    int scrollbar=1; // 1 to show scrollbar, 0 to not show it.
    int scrollout=0; // whether the scrollbar goes out of the menu area (1) or it overlaps some of the menu area (0)
    int numitems; // number of items in menu
    int type=MENUTYPE_NORMAL; // set to MENUTYPE_* .
    int selection=1; // currently selected item. starts counting at 1
    int scroll=0; // current scrolling position
    int fkeypage=0; // for MULTISELECT menu if it should allow file selecting and show the fkey label
    int numselitems=0; // number of selected items
    int returnOnInfiniteScrolling=0; //whether the menu should return when user reaches the last item and presses the down key (or the first item and presses the up key)
    int darken=0; // for dark theme on homeGUI menus
    int miniMiniTitle=0; // if true, title will be drawn in minimini. for calendar week view
    int pBaRtR=0; //preserve Background And Return To Redraw. Rarely used
    MenuItem* items; // items in menu
  } Menu;

#define MENU_RETURN_EXIT 0
#define MENU_RETURN_SELECTION 1
#define MENU_RETURN_INSTANT 2
#define MENU_RETURN_SCROLLING 3 //for returnOnInfiniteScrolling

  typedef struct {
    const char* name;
    const char* insert;
    const char* desc;
    const char * example;
    const char * example2;
    unsigned int category;
  } catalogFunc;

  void aide2catalogFunc(const giac::aide & a,catalogFunc & c);

  giac::gen select_var(const giac::context * contextptr);
  const char * keytostring(int key,int keyflag,bool py,const giac::context * contextptr);
  void insert(std::string & s,int pos,const char * add);
  
  int showCatalog(char* insertText,int preselect,int menupos,const giac::context * contextptr);
  int doMenu(Menu* menu, MenuItemIcon* icontable=NULL);
  void reset_alpha();
  // category=0 for CATALOG, 1 for OPTN
  // returns 0 on exit, 1 on success
  int doCatalogMenu(char* insertText, const char* title, int category,const giac::context * contextptr);
  extern const char shortcuts_fr_string[];
  extern const char shortcuts_en_string[];
  extern const char apropos_fr_string[];
  extern const char apropos_en_string[];
  void init_locale();

  gen _efface_logo(const gen & g,GIAC_CONTEXT);
  gen turtle_state(const giac::context * contextptr);
  int inputline(const char * msg1,const char * msg2,std::string & s,bool numeric,int ypos=65,const giac::context *contextptr=0);
  extern logo_turtle * turtleptr;
  bool inputdouble(const char * msg1,double & d,const giac::context *contextptr);
  bool do_confirm(const char * s);
  int confirm(const char * msg1,const char * msg2,bool acexit=false,int y=40);
  bool confirm_overwrite();
  void invalid_varname();

#ifndef NO_NAMESPACE_XCAS
} // namespace giac
#endif // ndef NO_NAMESPACE_XCAS


#define COLOR_BLACK giac::_BLACK
#define COLOR_RED giac::_RED
#define COLOR_GREEN giac::_GREEN
#define COLOR_CYAN giac::_CYAN
#define COLOR_BLUE giac::_BLUE
#define COLOR_YELLOW giac::_YELLOW
#define COLOR_MAGENTA giac::_MAGENTA
#define COLOR_WHITE giac::_WHITE
#define COLOR_YELLOWDARK 64934
#define COLOR_BROWN 65000
#define TEXT_COLOR_BLACK giac::_BLACK
#define TEXT_COLOR_RED giac::_RED
#define TEXT_COLOR_GREEN giac::_GREEN
#define TEXT_COLOR_CYAN giac::_CYAN
#define TEXT_COLOR_BLUE giac::_BLUE
#define TEXT_COLOR_YELLOW giac::_YELLOW
#define TEXT_COLOR_WHITE giac::_WHITE
#define TEXT_COLOR_MAGENTA giac::_MAGENTA


#endif // _KDISPLAY_H
#endif
