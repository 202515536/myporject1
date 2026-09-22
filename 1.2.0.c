#include<ncurses.h>
int main(){
    initscr();//初始化ncurses,调出终端面板
    cbreak();//按下按键立刻开始不用回车
    noecho();//关闭自带字符
    keypad(stdscr,TRUE);//识别方向键
    curs_set(1);//显示光标
    int ch;
    while(1){
        ch=getch();
        //ESC退出
        if(ch==27){
            break;
        }
        
        //方向键移动光标
        int x,y;
        getyx(stdscr,y,x);
        switch(ch){
            case KEY_UP:
            if(y>0) y--;
            break;
            case KEY_DOWN:
            if(y<LINES-1) y++;
            break;
            case KEY_LEFT:
            if(x>0) x--;
            break;
            case KEY_RIGHT:
            if(x<COLS-1) x++;
            break;
            //输入可打印字符，移动光标
            default:
            if(ch>=32&&ch<=126){
                mvaddch(y,x,ch);
                x++;
            }
            break;
        }
        move(y,x);
        refresh();//刷新屏幕
    }
    endwin();
    return 0;
}