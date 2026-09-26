#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#define MAX_LINE_LEN 1024
#define INIT_LINE_CNT 128

// 存储文本
typedef struct {
    char **lines;
    int line_count;//一共有多少行
    int line_capacity;//数组容量
} TextBuffer;

// 全局变量
TextBuffer buf;
int text_y=0, text_x=0; //光标在文本中的真实位置
int view_y=0; //屏幕顶部对应文件第几行

// 初始化文本缓冲区
void buf_init(TextBuffer *b)
{
    b->line_capacity=INIT_LINE_CNT;
    b->line_count=0;
    b->lines=malloc(sizeof(char*)*b->line_capacity);
}

// 添加一行文本
void buf_append_line(TextBuffer *b, const char *s)
{
    if (b->line_count>=b->line_capacity)
    {
        b->line_capacity*=2;
        b->lines=realloc(b->lines,sizeof(char*)*b->line_capacity);//重新分配内存
    }
    b->lines[b->line_count]=strdup(s);//存入第b->line_count行
    b->line_count++;
}

// 从文件加载到内存缓冲区
int load_file(TextBuffer *b, const char *filename)
{
    FILE *fp=fopen(filename, "r");
    if (!fp) return 1;
    char tmp[MAX_LINE_LEN];
    while (fgets(tmp, MAX_LINE_LEN, fp))
    {
        // 去掉末尾换行符
        size_t len=strlen(tmp);//获取这一行字符串长度
        if(len>0 && tmp[len-1] == '\n') tmp[len-1] = '\0';
        buf_append_line(b, tmp);
    }
    fclose(fp);
    return 0;
}

// 重绘屏幕
void redraw()
{
    clear();
    int scr_rows, scr_cols;
    getmaxyx(stdscr, scr_rows, scr_cols);
    int display_lines=scr_rows - 1; // 预留一行给底部

    // 绘制可视区域内的文本
    for(int i=0; i<display_lines; i++)
    {
        int file_line=view_y + i;
        if(file_line>=buf.line_count) break;
        mvprintw(i, 0, "%s", buf.lines[file_line]);
    }

    // 将文本光标投到屏幕坐标
    int screen_y=text_y-view_y;
    move(screen_y,text_x);
    refresh();
}

// 自动调整视口：光标超出屏幕就滚动
void adjust_viewport()
{
    int scr_rows,scr_cols;
    getmaxyx(stdscr,scr_rows,scr_cols);
    int display_rows=scr_rows-1;

    // 光标在屏幕上方
    if(text_y<view_y)
        view_y=text_y;
    // 光标在屏幕下方
    if(text_y>=view_y+display_rows)//加上最后一行
        view_y=text_y-display_rows+1;
}

int main(int fc, char *fv[])
{
    if(fc!=2)
    {
        printf("用法：%s 文件名\n", fv[0]);
        return 1;
    }

    buf_init(&buf);
    if(load_file(&buf, fv[1])!=0)
    {
        printf("打开文件失败\n");
        return 1;
    }

    initscr();
    raw();
    noecho();
    keypad(stdscr,TRUE);

    int ch;
    while(1)
    {
        adjust_viewport();
        redraw();
        ch = getch();

        if(ch == 17) // Ctrl-Q 退出
            break;

        switch(ch)
        {
            case KEY_UP:
                if(text_y>0)
                    text_y--;
                // x 取当前行的长度，防止x越界
                if(text_x>(int)strlen(buf.lines[text_y]))
                    text_x=strlen(buf.lines[text_y]);
                break;
            case KEY_DOWN:
                if(text_y<buf.line_count -1)
                    text_y++;
                if(text_x >(int)strlen(buf.lines[text_y]))
                    text_x=strlen(buf.lines[text_y]);
                break;
            case KEY_LEFT:
                if(text_x>0)
                    text_x--;
                break;
            case KEY_RIGHT:
                if(text_x<(int)strlen(buf.lines[text_y]))
                    text_x++;
                break;
            case KEY_RESIZE: // 窗口大小改变
                break;
        }
    }

    endwin();
    return 0;
}