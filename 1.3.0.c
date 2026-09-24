#include<ncurses.h>
int main(){
    int ch;
    int x=0,y=0;
    int max_x,max_y;
    initscr();//初始化ncurses,调出终端面板
    cbreak();//按下按键立刻开始不用回车
    noecho();//关闭自带字符
    keypad(stdscr,TRUE);//识别方向键
    curs_set(1);//显示光标
    getmaxyx(stdscr,max_y,max_x);//获取终端大小
    while(1){
        ch=getch();
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