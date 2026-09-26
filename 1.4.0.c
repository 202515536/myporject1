#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ncurses.h>
#define MAX_LINE 1024
 int main(int fc,char *fv[]){
    FILE *fp=NULL;
    char buffer[MAX_LINE];
    if(fc!=2){
        printf("正确用法%s <filename>\n",fv[0]);
        return 1;
    }
    fp=fopen(fv[1],"r");
    if(fp==NULL){
        printf("无法打开文件%s\n",fv[1]);
        return 1;
    }//正确打开文件
    initscr();
    raw();
    noecho();
    keypad(stdscr,TRUE);
    intrflush(stdscr,FALSE);
    nodelay(stdscr,FALSE);
    int y=0,x=0;
    int ch;
    int max_y,max_x;
    getmaxyx(stdscr,max_y,max_x);
    //初始化ncurses
    while(fgets(buffer,MAX_LINE,fp)!=NULL){
        if(y>=max_y-1) break;
        mvaddstr(y,0,buffer);//打印buffer中内容
        y++;
    }//读取文件中所有内容
    fclose(fp);//关闭文件
    move(0,0);
    refresh();
    //读取文件在终端中正确显示  
    while(1){
        ch=getch();
        getmaxyx(stdscr,max_y,max_x); //获取面板边界
        switch(ch){
            case KEY_UP:
            if(y>0) y--;
            break;
            case KEY_DOWN:
            if(y<max_y-1) y++;
            break;
            case KEY_LEFT:
            if(x>0) x--;
            break;
            case KEY_RIGHT:
            if(x<max_x-1) x++;
            break;
            case 17://ctrl+Q的ASCII码是17
            endwin();//恢复终端，退出
            return 0;
            default:
            break;
        }
        move(y,x);
        refresh();//刷新屏幕
    }
    endwin();
    return 0;
 }