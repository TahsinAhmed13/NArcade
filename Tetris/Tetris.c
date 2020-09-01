#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/time.h>
#include <ncurses.h>

#include "Tetromino.h"

#define USEC_IN_SEC 0.000001

const char title[] = "Tetris"; 

WINDOW *g_main_win; 
WINDOW *g_game_win; 
WINDOW *g_score_win; 
WINDOW *g_next_win; 

void setup(); 

void draw_tetromino(WINDOW *, const chtype, bool **const, int, int);

int drop_tetromino(bool **); 
int scan_lines(); 
bool game_over(); 
int calc_score(int); 

void cleanup(); 

int main()
{
    setup(); 

    srand(time(0)); 
    int score = 0; 
    mvwprintw(g_score_win, 
            (getmaxy(g_score_win)-1) / 2, 
            (getmaxx(g_score_win)-1) / 2, 
            "%d", score); 
    wrefresh(g_score_win); 
    bool **tetromino; 
    bool **n_tetromino = get_copy(rand()); 

    while(true)
    {
        tetromino = n_tetromino; 

        n_tetromino = get_copy(rand()); 
        werase(g_next_win); 
        box(g_next_win, 0, 0); 
        int n_sy = (getmaxy(g_next_win) - get_height(n_tetromino)) / 2; 
        int n_sx = (getmaxx(g_next_win) - 2*get_width(n_tetromino)) / 2; 
        draw_tetromino(g_next_win, ' ' | A_REVERSE, n_tetromino, n_sy, n_sx);  
        wrefresh(g_next_win); 

        if(drop_tetromino(tetromino) == ERR) 
            break; 
        if(!game_over())
            score += calc_score(scan_lines()); 
        else                
            break;  
        int digits = ceil(log10(score)); 
        mvwprintw(g_score_win, 
                getmaxy(g_score_win) / 2, 
                (getmaxx(g_score_win) - digits) / 2, 
                "%d", score); 
        wrefresh(g_score_win); 

        del_copy(tetromino); 
    }

    if(tetromino != NULL) 
        del_copy(tetromino); 
    del_copy(n_tetromino); 
    cleanup(); 
}

double get_time()
{
    struct timeval t; 
    gettimeofday(&t, NULL); 
    return t.tv_sec + t.tv_usec * USEC_IN_SEC; 
}

WINDOW *gen_win(int h, int w, int y, int x)
{
    WINDOW *win = newwin(h, w, y, x); 
    box(win, 0, 0); 
    wrefresh(win); 
    return win; 
}

void gen_wins() 
{
    int main_height = LINES * 0.7; 
    int main_width = main_height * 4; 
    int main_starty = (LINES - main_height) / 2; 
    int main_startx = (COLS - main_width) / 2; 
    g_main_win = gen_win(main_height, main_width, main_starty, main_startx); 

    int game_width = main_width * 0.3; 
    int game_startx = main_startx + (main_width - game_width) / 2; 
    g_game_win = gen_win(main_height, game_width, main_starty, game_startx); 

    int score_height = main_height * 0.15; 
    int next_height = main_height * 0.35; 
    int vspace = main_height * 0.1; 

    int score_starty = main_starty + (main_height 
            - (score_height + next_height + vspace + 2)) / 2; 
    int next_starty = score_starty + score_height + 1 
        + vspace; 

    int score_width = main_width * 0.2; 
    int score_startx = game_startx + game_width 
        + ((main_width - game_width) / 2 - score_width) / 2; 
    
    g_score_win = gen_win(score_height, score_width, score_starty, score_startx); 
    g_next_win = gen_win(next_height, score_width, next_starty, score_startx); 
}

void print_title(WINDOW *win, const char *title)
{
    int h, w, y, x; 
    getmaxyx(win, h, w); 
    getbegyx(win, y, x); 
    mvprintw(y-1, x + (w-strlen(title)) / 2, "%s", title); 
}

void print_titles()
{
    print_title(g_main_win, title); 
    print_title(g_score_win, "Score");  
    print_title(g_next_win, "Next"); 
}

void setup()
{
    initscr(); 
    printw("Press F1 to exit"); 
    refresh(); 
    cbreak(); 
    noecho(); 
    curs_set(0); 

    gen_wins(); 
    print_titles(); 
    refresh(); 
    keypad(g_game_win, TRUE); 
    nodelay(g_game_win, TRUE); 
}

void draw_tetromino(WINDOW *win, const chtype ch, bool **const t, int sy, int sx)
{
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            if(t[i][j])
                mvwhline(win, sy + i, sx + 2*j, ch, 2); 
}

bool is_valid(WINDOW *win, int y, int x)
{
    return 0 < y && y < getmaxy(win)-1 
        && 0 < x && x < getmaxx(win)-2; 
}

bool can_move(bool **t, int y, int x)
{
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            chtype ch = mvwinch(g_game_win, y + i, x + 2*j); 
            int attr = ch & A_ATTRIBUTES; 
            bool valid = is_valid(g_game_win, y + i, x + 2*j); 

            if(t[i][j] && (attr != A_NORMAL || !valid))
                return false; 
        }
    }
    return true; 
}

int drop_tetromino(bool **t)
{   
    const int w = getmaxx(g_game_win); 
    const double drop_rate = 0.5; // one line per sec

    int ch; 

    int sy = 1;
    int sx = (w - 4) / 2;
    int th = get_height(t); 
    int tw = get_width(t); 

    double drop_factor; 
    double cycle_start = get_time(); 

    while(true)
    {
        ch = wgetch(g_game_win); 
        draw_tetromino(g_game_win, ' ' | A_NORMAL, t, sy, sx); 

        drop_factor = 1; 
        if(ch == KEY_LEFT && can_move(t, sy, sx-2))
            sx -= 2; 
        else if(ch == KEY_RIGHT && can_move(t, sy, sx+2)) 
            sx += 2; 
        else if(ch == KEY_UP && sx + 2*th <= w - 3)
        {
            int tmp = th; 
            th = tw, tw = tmp; 
            rotate(t); 
        }
        else if(ch == KEY_DOWN)
            drop_factor = 0.1;  
        else if(ch == KEY_F(1))
            return ERR; 

        if(get_time() - cycle_start >= drop_rate * drop_factor) 
        {
            if(can_move(t, sy+1, sx)) ++sy;  
            else                      break; 
            cycle_start = get_time(); 
        }

        draw_tetromino(g_game_win, ' ' | A_REVERSE, t, sy, sx); 
        wrefresh(g_game_win); 
    }

    draw_tetromino(g_game_win, ' ' | A_REVERSE, t, sy, sx); 
    wrefresh(g_game_win); 
    return OK; 
}

void delete_line(int n)
{
    for(int i = n; i > 0; --i)
    {
        bool finished = true; 
        for(int j = 1; j < getmaxx(g_game_win)-1; ++j)
        {
            chtype tmp = mvwinch(g_game_win, i-1, j); 
            mvwaddch(g_game_win, i, j, tmp); 
            finished = finished && (tmp & A_ATTRIBUTES) == A_NORMAL; 
        }
        if(finished) break; 
    }
}

int scan_lines()
{
    int cols = (int) (getmaxx(g_game_win) / 2) - 1; 
    int cleared = 0; 
    int i = getmaxy(g_game_win) - 2; 
    while(i > 1) 
    {
        bool full = true, empty = true; 
        for(int j = 0; j < cols; ++j)
        {
            chtype ch = mvwinch(g_game_win, i, 2*j+1); 
            int attr = ch & A_ATTRIBUTES; 
            full = full && attr != A_NORMAL; 
            empty = empty && attr == A_NORMAL; 
        }
        if(full)        
        {
            delete_line(i); 
            ++cleared; 
        }
        else if(empty)  break; 
        else            --i; 
    }
    wrefresh(g_game_win); 
    return cleared; 
}

bool game_over()
{
    int cols = (int) (getmaxx(g_game_win) / 2) - 1; 
    for(int j = 0; j < cols; ++j)
    {
        chtype ch = mvwinch(g_game_win, 1, 2*j+1); 
        int attr = ch & A_ATTRIBUTES; 
        if(attr != A_NORMAL)
            return true; 
    }
    return false; 
}

int calc_score(int n)
{
    switch(n)
    {
        case 0: 
            return 0; 
        case 1:
            return 40; 
        case 2: 
            return 100; 
        case 3: 
            return 300; 
        default: 
            return 1200; 
    }
}

void del_wins()
{
    delwin(g_main_win); 
    delwin(g_game_win); 
    delwin(g_score_win); 
    delwin(g_next_win); 
}

void cleanup()
{
    del_wins(); 
    endwin(); 
}
