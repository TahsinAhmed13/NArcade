#include <ncurses.h>

#include "Tetromino.h"

int main()
{
    initscr(); 
    cbreak(); 
    noecho(); 
    curs_set(0); 
    printw("Press F1 to exit"); 
    refresh(); 

    WINDOW *win = newwin(6, 10, (LINES-6) / 2, (COLS-10) / 2); 
    keypad(win, TRUE); 

    int ch = 0; 
    int n = 0; 
    bool **tetrimino; 
    tetrimino = get_copy(n); 
    do 
    {
        if(ch == KEY_UP)
            rotate(tetrimino); 
        else if(ch == KEY_LEFT && n > 0)
        {
            del_copy(tetrimino); 
            tetrimino = get_copy(--n); 
        }
        else if(ch == KEY_RIGHT && n < 6)
        {
            del_copy(tetrimino); 
            tetrimino = get_copy(++n); 
        }

        werase(win); 
        wattrset(win, A_NORMAL); 
        box(win, 0, 0); 
        wattrset(win, A_REVERSE);
        for(int i = 0; i < 4; ++i)
            for(int j = 0; j < 4; ++j)
                if(tetrimino[i][j])
                    mvwprintw(win, 1 + i, 1 + 2*j, "  "); 
        wrefresh(win); 
    }while((ch = wgetch(win)) != KEY_F(1)); 

    delwin(win); 
    endwin(); 
}
