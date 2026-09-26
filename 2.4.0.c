#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#define MAX_LINE_LEN 1024
#define INIT_LINE_CNT 128

// 存储文本
typedef struct {
    char **lines;
    int line_count;       //一共有多少行
    int line_capacity;    //数组容量
} TextBuffer;

// 全局变量
TextBuffer buf;
int text_y=0, text_x=0;   //光标在文本中的真实位置
int view_y=0;             //屏幕顶部对应文件第几行
char filename[256];       //2.3新增：保存文件名
int is_modified = 0;      //2.3新增：标记是否有未保存修改
int warn_quit = 0;        //2.4新增：标记是否等待二次确认退出

// 初始化文本缓冲区
void buf_init(TextBuffer *b)
{
    b->line_capacity=INIT_LINE_CNT;
    b->line_count=0;
    b->lines=malloc(sizeof(char*) * b->line_capacity);
}

// 添加一行文本
void buf_append_line(TextBuffer *b, const char *s)
{
    if (b->line_count>=b->line_capacity)
    {
        b->line_capacity *=2;
        b->lines=realloc(b->lines, sizeof(char*) * b->line_capacity);//重新分配内存
    }
    b->lines[b->line_count]=strdup(s);//存入第b->line_count行
    b->line_count++;
}

// 在y行的x位置 插入单个字符
void buf_insert_char(TextBuffer *b, int y, int x, char ch)
{
    char *line=b->lines[y];
    int len=strlen(line);
    if(x > len) x=len;//防止越界
    char *newline=malloc(len+2); //多申请1个字符空间
    strcpy(newline,line);
    // x位置及其后面的字符整体后移一位
    for(int i=len;i>=x;i--)
        newline[i+1]=newline[i];
    newline[x]=ch;//写入新字符
    b->lines[y]=newline;
    free(line);//释放旧行
    is_modified=1;
}

//删除光标前面的字符
void buf_backspace(TextBuffer *b, int y, int x)
{
    if(x == 0)
    {
        // 光标在行首：当前行合并到上一行
        if(y <= 0) return;
        char *curr=b->lines[y];
        char *prev=b->lines[y-1];
        int prev_len=strlen(prev);
        char *newprev=realloc(prev, prev_len+strlen(curr)+1);
        if(newprev==NULL) return;
        strcat(newprev, curr);//上一行末尾接上当前行
        b->lines[y-1]=newprev;
        free(b->lines[y]);
        for(int i = y;i<b->line_count-1;i++)
            b->lines[i]=b->lines[i+1];
        b->line_count--;
        text_y=y-1;              // 光标移到上一行末尾
        text_x=prev_len;
        is_modified=1;
        return;
    }
    char *line=b->lines[y];
    int len=strlen(line);
    for(int i=x - 1; i < len; i++)
        line[i]=line[i+1];         // 从x-1开始整体左移
    text_x--;
    is_modified=1;
}

//删除光标后面的字符，Del键
void buf_delete_char(TextBuffer *b, int y, int x)
{
    char *line=b->lines[y];
    int len=strlen(line);
    if(x>=len)
    {
        // 光标在行尾：把下一行合并到当前行
        if(y>=b->line_count-1) return;
        char *curr=b->lines[y];
        char *next=b->lines[y+1];
        int curr_len=strlen(curr);
        char *newcurr=realloc(curr, curr_len + strlen(next) + 1);
        if(newcurr==NULL) return;
        strcat(newcurr, next);
        b->lines[y]=newcurr;
        free(b->lines[y+1]);
        for(int i=y+1; i<b->line_count-1;i++)
            b->lines[i]=b->lines[i+1];
        b->line_count--;
        is_modified=1;
        return;
    }
    for(int i=x; i<len;i++)
        line[i]=line[i+1];//从x开始整体左移
    is_modified=1;
}

// 回车：在y行x位置拆分成两行
void buf_split_line(TextBuffer *b, int y, int x)
{
    char *line=b->lines[y];
    int len=strlen(line);
    if(x>len) x=len;
    char first_part[MAX_LINE_LEN];
    char second_part[MAX_LINE_LEN];
    strncpy(first_part, line, x);//光标前部分
    first_part[x]='\0';
    strcpy(second_part,line+x); //光标后部分

    if (b->line_count>=b->line_capacity)
    {
        b->line_capacity*=2;
        b->lines=realloc(b->lines, sizeof(char*) * b->line_capacity);
    }
    //y行之后的各行整体后移一位
    for(int i=b->line_count; i>y + 1;i--)
        b->lines[i]=b->lines[i-1];
    free(b->lines[y]);//释放原整行
    b->lines[y]=strdup(first_part);
    b->lines[y+1]=strdup(second_part);
    b->line_count++;
    is_modified=1;
}

//从文件加载到内存缓冲区
int load_file(TextBuffer *b, const char *filename)
{
    FILE *fp=fopen(filename, "r");
    if (!fp) return 1;
    char tmp[MAX_LINE_LEN];
    while (fgets(tmp, MAX_LINE_LEN, fp))
    {
        //去掉末尾换行符
        size_t len = strlen(tmp);//获取这一行字符串长度
        if(len>0 && tmp[len-1] == '\n') tmp[len-1] = '\0';
        buf_append_line(b, tmp);
    }
    fclose(fp);
    return 0;
}
//保存文件
int save_file(TextBuffer *b, const char *fname)
{
    FILE *fp=fopen(fname, "w");
    if(!fp) return 1;
    for(int i=0;i<b->line_count; i++)
    {
        fprintf(fp, "%s\n", b->lines[i]);
    }
    fclose(fp);
    is_modified = 0; //保存成功，清除修改标记
    return 0;
}
//重绘屏幕
void redraw()
{
    clear();
    int scr_rows, scr_cols;
    getmaxyx(stdscr,scr_rows,scr_cols);
    int display_lines=scr_rows - 1; // 预留一行给底部
    // 绘制可视区域内的文本
    for(int i=0; i<display_lines;i++)
    {
        int file_line=view_y+i;
        if(file_line>=buf.line_count) break;
        mvprintw(i,0, "%s",buf.lines[file_line]);
    }
    // 将文本光标投到屏幕坐标
    int screen_y=text_y - view_y;
    move(screen_y,text_x);
    
    char status_buf[256];
    char mod_str[20];
    if(is_modified)
        strcpy(mod_str,"Modified");
    else
        strcpy(mod_str,"");
    //格式:文件名 | Modified | 行:列
    snprintf(status_buf, sizeof(status_buf), "%s | %s | %d:%d", filename, mod_str, text_y+1, text_x+1);
    mvprintw(scr_rows-1,0,"%s",status_buf);
    clrtoeol(); //清除状态栏剩余部分，防止残留旧文字
    refresh();
}

// 自动调整视口：光标超出屏幕就滚动
void adjust_viewport()
{
    int scr_rows, scr_cols;
    getmaxyx(stdscr,scr_rows,scr_cols);
    int display_rows=scr_rows-2;//add a line
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
    strncpy(filename,fv[1], sizeof(filename) - 1);//保存
    if(load_file(&buf, fv[1])!=0)
    {
        printf("打开文件失败\n");
        return 1;
    }
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    int ch;
    while(1)
    {
        adjust_viewport();
        redraw();
        ch=getch();
        if(ch==19){
            save_file(&buf, filename);
            warn_quit=0;
            continue;
        }//ctrl+s
        if(ch==17){//Ctrl-Q 退出
            if(is_modified&&warn_quit==0)
            {
                warn_quit=1;//第一次按，标记等待二次确认
                continue;
            }
            else
            {
                break;//第二次Ctrl-Q直接退出
            }
        }
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
                if(text_x>(int)strlen(buf.lines[text_y]))
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
            case KEY_BACKSPACE:   // 退格键（2.2新增）
            case 127:             // 部分终端退格返回127
                buf_backspace(&buf, text_y, text_x);
                break;
            case KEY_DC:          // Delete键（2.2新增）
                buf_delete_char(&buf, text_y, text_x);
                break;
            case '\n':            // 回车换行（2.2新增）
            case '\r':            // raw()模式下回车返回13，也处理
                buf_split_line(&buf, text_y, text_x);
                text_y++;
                text_x = 0;
                break;
            case KEY_RESIZE: // 窗口大小改变
                break;
            default:
                // 可打印字符，在光标处插入（2.2新增）
                if(ch>=32 &&ch<=126)
                {
                    buf_insert_char(&buf, text_y, text_x, ch);
                    text_x++;
                }
                break;
        }
    }
    endwin();
    return 0;
}