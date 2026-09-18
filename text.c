#include <ncurses.h>
int main(){
    initscr();
    printw("Hello world\n按任意键退出");
    refresh();
    getch();
    endwin();
    return 0;
}