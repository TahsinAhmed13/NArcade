#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/time.h>
#include <ncurses.h>

#include "Tetromino.h"

#define USEC_IN_SEC 0.000001

#define CONTINUE 0
#define RESTART 1
#define QUIT 2

const char title[] = "Tetris"; 

WINDOW *g_main_win; 
WINDOW *g_game_win; 
WINDOW *g_score_win; 
WINDOW *g_next_win; 

void setup(); 

int play(); 
bool play_again(); 

void cleanup(); 

int main()
{
    setup(); 
    srand(time(0)); 
    int status; 
    do
    {
        status = play(); 
        if(status == QUIT) break; 
    }
    while(status == RESTART || play_again()); 
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

void reset_win(WINDOW *w)
{
    werase(w); 
    box(w, 0, 0); 
    wrefresh(w); 
}

void reset_wins()
{
    reset_win(g_main_win); 
    reset_win(g_game_win); 
    reset_win(g_score_win); 
    reset_win(g_next_win); 
}

void setup_color()
{
    start_color(); 
    use_default_colors(); 

    init_pair(1, -1, 51);   // I, cyan 
    init_pair(2, -1, 226);  // O, yellow
    init_pair(3, -1, 201);  // T, magenta
    init_pair(4, -1, 214);  // L, orange
    init_pair(5, -1, 21);   // J, blue
    init_pair(6, -1, 46);   // S, green
    init_pair(7, -1, 196);  // Z, red
}

void setup()
{
    initscr(); 
    printw("Press Space to options"); 
    refresh(); 
    cbreak(); 
    noecho(); 
    curs_set(0); 

    gen_wins(); 
    print_titles(); 
    refresh(); 
    keypad(g_game_win, TRUE); 
    nodelay(g_game_win, TRUE); 

    if(has_colors()) setup_color(); 
}

void redraw(WINDOW *w)
{
    for(int i = 0; i < getmaxy(w); ++i)
        for(int j = 0; j < getmaxx(w); ++j)
        {
            chtype ch = mvwinch(w, i, j); 
            waddch(w, ch); 
        }
}

int show_propmt(const char *prompt, int n, char **opts)
{
    int pady = getmaxy(g_game_win) * 0.05; 
    int height = pady * 2 + n; 
    int width = getmaxx(g_game_win) * 0.85;
    int starty = (LINES - height) / 2; 
    int startx = (COLS - width) / 2; 

    WINDOW *w = newwin(height, width, starty, startx); 
    keypad(w, TRUE); 
    box(w, 0, 0); 
    mvwprintw(w, 0, (width - strlen(title)) / 2, prompt); 
    for(int i = 0; i < n; ++i)
    {
        int y = pady + i;  
        int x = (width - strlen(opts[i])) / 2; 
        mvwprintw(w, y, x, opts[i]);  
    }
    wrefresh(w); 

    int ch = 0; 
    int i = 0; 
    do
    {
        mvwchgat(w, pady + i, 1, 
                width - 2, A_NORMAL, 0, NULL); 
        if(ch == KEY_UP && i > 0)           --i;  
        else if(ch == KEY_DOWN && i < n-1)  ++i; 
        mvwchgat(w, pady + i, 1, 
                width - 2, A_REVERSE, 0, NULL); 
        wrefresh(w); 
    } while((ch = wgetch(w)) != 10); 

    werase(w); 
    wrefresh(w); 
    delwin(w); 
    return i; 
}

int pause()
{
    char prompt[] = "Paused"; 
    char opt1[] = "Continue"; 
    char opt2[] = "Start Over";  
    char opt3[] = "Quit"; 
    char *opts[] = {opt1, opt2, opt3}; 
    return show_propmt(prompt, 3, opts); 
}

void draw_tetromino(WINDOW *win, chtype ch, bool **t, int sy, int sx)
{
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            if(t[i][j])
                mvwhline(win, sy + i, sx + 2*j, ch, 2); 
}

bool is_valid(const WINDOW *win, int y, int x)
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
            int color = ch & A_COLOR; 
            bool valid = is_valid(g_game_win, y + i, x + 2*j); 

            if(t[i][j] && (color != COLOR_PAIR(0) || !valid))
                return false; 
        }
    }
    return true; 
}

// t is a ptr to a 2D arr
int drop_tetromino(int i, bool ***t)
{   
    int w = getmaxx(g_game_win); 
    double drop_rate = 0.5; // one line per sec

    int ch; 

    int sy = 1;
    int sx = (w - 4) / 2;
    bool **rot = rotate(*t); 

    double drop_factor; 
    double cycle_start = get_time(); 

    while(true)
    {
        ch = wgetch(g_game_win); 
        draw_tetromino(g_game_win, ' ' | COLOR_PAIR(0), *t, sy, sx); 

        drop_factor = 1; 
        if(ch == KEY_LEFT && can_move(*t, sy, sx-2))
            sx -= 2; 
        else if(ch == KEY_RIGHT && can_move(*t, sy, sx+2)) 
            sx += 2; 
        else if(ch == KEY_UP && can_move(rot, sy, sx))
        {
            // address of array changes
            del_copy(*t); 
            *t = rot; 
            rot = rotate(*t); 
        }
        else if(ch == KEY_DOWN)
            drop_factor = 0.1;  
        else if(ch == ' ')
        {
            int status = pause(); 
            if(status == 0)
            {
                redraw(g_game_win); 
                wrefresh(g_game_win); 
            }
            else
            {
                del_copy(rot); 
                return status; 
            }
        }

        if(get_time() - cycle_start >= drop_rate * drop_factor) 
        {
            if(can_move(*t, sy+1, sx)) ++sy;  
            else                      break; 
            cycle_start = get_time(); 
        }

        draw_tetromino(g_game_win, ' ' | COLOR_PAIR(i+1), *t, sy, sx); 
        wrefresh(g_game_win); 
    }

    del_copy(rot); 
    draw_tetromino(g_game_win, ' ' | COLOR_PAIR(i+1), *t, sy, sx); 
    wrefresh(g_game_win); 

    return CONTINUE; 
}

void clear_line(int n)
{
    for(int i = n; i > 0; --i)
    {
        bool finished = true; 
        for(int j = 1; j < getmaxx(g_game_win)-1; ++j)
        {
            chtype tmp = mvwinch(g_game_win, i-1, j); 
            mvwaddch(g_game_win, i, j, tmp); 
            finished = finished && (tmp & A_COLOR) == COLOR_PAIR(0); 
        }
        if(finished) break; 
    }
}

int clear_lines()
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
            int color = ch & A_COLOR; 
            full = full && color != COLOR_PAIR(0); 
            empty = empty && color == COLOR_PAIR(0); 
        }
        if(full)        
        {
            clear_line(i); 
            ++cleared; 
        }
        else if(empty)  break; 
        else            --i; 
    }
    wrefresh(g_game_win); 
    return cleared; 
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

bool game_over()
{
    int cols = (int) (getmaxx(g_game_win) / 2) - 1; 
    for(int j = 0; j < cols; ++j)
    {
        chtype ch = mvwinch(g_game_win, 1, 2*j+1); 
        int color = ch & A_COLOR; 
        if(color != COLOR_PAIR(0))
            return true; 
    }
    return false; 
}

int play()
{
    reset_wins(); 

    // initialize game variables
    int score = 0; 
    mvwprintw(g_score_win, 
            (getmaxy(g_score_win)-1) / 2, 
            (getmaxx(g_score_win)-1) / 2, 
            "%d", score); 
    wrefresh(g_score_win); 
    int i; 
    bool **tetromino; 
    int ni = rand() % 7; 
    bool **n_tetromino = get_copy(ni); 

    while(true)
    {
        i = ni; 
        tetromino = n_tetromino; 

        // get and show next tetromino
        ni = rand() % 7; 
        n_tetromino = get_copy(ni); 
        werase(g_next_win); 
        box(g_next_win, 0, 0); 
        int n_sy = (getmaxy(g_next_win) - get_height(n_tetromino)) / 2; 
        int n_sx = (getmaxx(g_next_win) - 2*get_width(n_tetromino)) / 2; 
        draw_tetromino(g_next_win, ' ' | COLOR_PAIR(ni+1), n_tetromino, n_sy, n_sx);  
        wrefresh(g_next_win); 

        // play
        int status = drop_tetromino(i, &tetromino); 
        del_copy(tetromino); 
        if(status != CONTINUE) return status; 

        // update score
        if(!game_over())
            score += calc_score(clear_lines()); 
        else break;  
        int digits = ceil(log10(score)); 
        mvwprintw(g_score_win, 
                getmaxy(g_score_win) / 2, 
                (getmaxx(g_score_win) - digits) / 2, 
                "%d", score); 
        wrefresh(g_score_win); 
    }

    // delete game variables 
    del_copy(n_tetromino); 
    return CONTINUE; 
}

bool play_again()
{
    char prompt[] = "Play Again?"; 
    char opt1[] = "Yes"; 
    char opt2[] = "No"; 
    char *opts[] = {opt1, opt2}; 
    return !show_propmt(prompt, 2, opts); 
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
