#include "draw_func.h"
#include "stdio.h"
#include "oled.h"
unsigned char *g_font_ptr=0;
//#define DEBUG_ON
int frame_x_offset=0;
int frame_y_offset=0;

//8行，16列
unsigned char frame_buf1[disp_y][disp_x]={0};
//下面是扩展的显示缓存,以10倍扩展
unsigned char frame_buf2[ROWS_EXT][COLS_EXT]={0};

void set_font(const unsigned char* font_in)
{
    g_font_ptr=(unsigned char*)font_in;
}
void get_text_ext(char *str,int *x,int *y)
{
    int i=0;
    *x=0;
    *y=0;
    if(g_font_ptr!=0)
    {
        *y=g_font_ptr[1];
        for(i=0;;i++)
        {
            if(str[i]==0)
            {
                break;   
            }
            else
            {
                if(((unsigned char)str[0])<128)
                {
                    *x+=g_font_ptr[0]/2;
                    str++;
                }
                else
                {
                    *x+=g_font_ptr[0];
                    str=str+2;
                }
            }
        }
    }
}

int get_font_pos(char *str)
{
    int ret=0;
    if(((unsigned char *)str)[0]<128)
    {
        ret=2+(str[0])*32;
    }
    else
    {
        ret=2+128*32+((((unsigned char*)str)[0])-0xa1)*(94)*32+((((unsigned char *)str)[1])-0xa1)*32;
    }
    return ret;
}

void text_out(char *str,int x,int y)
{
    //获取点阵在字库中的偏移
    //将获取的点阵数据刷新到frame_buf2中
    //x得到列数。
    //y获取行数，但是由于y需要按点阵的精度获取才可以，当y非8的整数倍时需要处理，即将点阵数据右移动（y%8）位，放入当前行的低位，左移（8-y%8）取低8bit放入下一行。
    int ymod=y%8;
    int row_now=0;
    int i=0;
    int index=0;
    int cols_copy=0;
    unsigned char mask=0;
    if(ymod<0)
        ymod=8+ymod;
    for(i=0;str[0]!=0;i++)
    {
        index=get_font_pos(str);
#ifdef DEBUG_ON
    printf("index=%d\n",index);
    printf("str=%s\n",str);
    int p=0;
    while(str[p]!=0)
    {
        printf("%x-",((unsigned char*)str)[p]);
        p++;
    }
    printf("\n");
#endif
//printf("uc %d\n",(*(unsigned char*)str));
        if(*((unsigned char*)str)<128)
        {
            cols_copy=8;
            str++;
            //printf("not chinese\n");
        }
        else    
        {
            cols_copy=16;
            str+=2;
            //printf("chinese\n");
        }
        if(ymod==0)
        {
            row_now=y/8;
            for(i=0;i<cols_copy;i++)
            {
                if(row_now>=0)
                    frame_buf2[row_now][x+i]=g_font_ptr[index+i];
            }
            for(i=0;i<cols_copy;i++)
            {
                if(row_now+1>=0)
                    frame_buf2[row_now+1][x+i]=g_font_ptr[index+i+16];
            }
        }
        else
        {
            //
            row_now=y/8;
            //row_next=row_now+1;
            mask=255;
            mask<<=ymod;
            //printf("row_now=%d,ymod=%d\n",row_now,ymod);
            if(y>=0)
            {
                for(i=0;i<cols_copy;i++)
                {
                    
                    frame_buf2[row_now][x+i]=(frame_buf2[row_now][x+i]&(~mask))+((g_font_ptr[index+i])<<ymod);
                }
                for(i=0;i<cols_copy;i++)
                {
                    
                    frame_buf2[row_now+1][x+i]=((g_font_ptr[index+i])>>(8-ymod))+((g_font_ptr[index+i+16])<<(ymod));
                }
                for(i=0;i<cols_copy;i++)
                {
                    
                    frame_buf2[row_now+2][x+i]=(frame_buf2[row_now+2][x+i]&(mask))+((g_font_ptr[index+i+16])>>(8-ymod));
                }
            }
            else
            {
                row_now-=1;
                //mask=255;
                //mask>>=8-ymod;
                for(i=0;i<cols_copy;i++)
                {
                    if(row_now>=0)
                        frame_buf2[row_now][x+i]=(frame_buf2[row_now][x+i]&(~mask))+((g_font_ptr[index+i])<<ymod);
                }
                for(i=0;i<cols_copy;i++)
                {
                    if(row_now+1>=0)
                        frame_buf2[row_now+1][x+i]=((g_font_ptr[index+i])>>(8-ymod))+((g_font_ptr[index+i+16])<<(ymod));
                }
                for(i=0;i<cols_copy;i++)
                {
                    if(row_now+2>=0)
                        frame_buf2[row_now+2][x+i]=/*(frame_buf2[row_now+2][x+i]&(mask))+*/((g_font_ptr[index+i+16])>>(8-ymod));
                }
            }
       }
       x=x+cols_copy;
    }
    //invalidate();
}

void invalidate()
{
    int i=0;
    int j=0;
    for(i=0;i<disp_y;i++)
    for(j=0;j<disp_x;j++)
    {
        frame_buf1[i][j]=frame_buf2[i][j];
    }
    oled_show();
}

void clr_disp()
{
    int i=0;
    int j=0;
    for(i=0;i<disp_y;i++)
    for(j=0;j<disp_x;j++)
    {
        frame_buf2[i][j]=0;
    }
}

void scroll_x()
{
    int i=0;
    int j=0;
    static int x_max=256;
    static int scroll_x_value=0;
    for(i=0;i<disp_y;i++)
    for(j=0;j<disp_x;j++)
    {
        if(i<2)
        {
            frame_buf1[i][j]=frame_buf2[i][j];
        }
        else
        {
            if(j+scroll_x_value>=0)
                frame_buf1[i][j]=frame_buf2[i][j+scroll_x_value];
            else    
                frame_buf1[i][j]=0;
        }
    }
    scroll_x_value++;
    if(scroll_x_value>x_max)
        scroll_x_value=-140;
    oled_show();
}