#include<ncurses.h>
int main(){
    initscr();//初始化ncurses,调出终端面板
    cbreak();//按下按键立刻开始不用回车
    noecho();//关闭自带字符
    keypad(stdscr,TRUE);//识别方向键

}