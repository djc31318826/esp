#include "font16x16_gb2312.h"

#define disp_x 128
#define disp_y 8
#define ROWS_EXT disp_y*5
#define COLS_EXT disp_x*5

//8行，16列
extern unsigned char frame_buf1[disp_y][disp_x];
//下面是扩展的显示缓存,以10倍扩展
extern unsigned char frame_buf2[ROWS_EXT][COLS_EXT];

extern unsigned char *g_font_ptr;
extern int frame_x_offset;
extern int frame_y_offset;

void set_font(const unsigned char* font_in);
void get_text_ext(char *str,int *x,int *y);
void text_out(char *str,int x,int y);
void invalidate();
void scroll_x();
void clr_disp();