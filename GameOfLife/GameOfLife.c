#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <ncurses.h>
#include <unistd.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

const double ratio = 0.5; 
int height, width; 
int starty, startx;

const char title[] = "Game of Life"; 
WINDOW *game_win; 

struct cell
{
    int y, x; /* drawing coordinates only */ 
    bool alive; 
    int neighbors; 
}; 

int rows, cols; 
struct cell **grid; 

struct cell **create_grid(int, int, double); 
void delete_grid(struct cell **, int); 
void draw_grid(); 
bool is_valid(int, int); 
int alive_neighbors(int, int); 
int step(); 

int main(int argc, char **argv) 
{
    double distribution = argc >= 2 ? atof(argv[1]) : 0.5; 
    int ch; 
    int total; 
    char status[80]; 

    srand(time(0)); 
    initscr(); 
    refresh(); 
    cbreak(); 
    nodelay(stdscr, TRUE); 
    keypad(stdscr, TRUE); 
    curs_set(0); 

    printw("Press F1 to exit"); 
    height = MIN(LINES, COLS/2) * ratio; 
    width = height * 2; 
    starty = (LINES - height) / 2; 
    startx = (COLS - width) / 2; 
    game_win = newwin(height, width, starty, startx);
    box(game_win, 0, 0); 
    wrefresh(game_win); 
    mvprintw(starty-1, startx + (width - strlen(title))/2, "%s", title); 
    refresh(); 

    rows = height - 2; 
    cols = (width - 2) / 2; 
    total = rows * cols; 
    grid = create_grid(rows, cols, distribution); 

    while((ch = getch()) != KEY_F(1))
    {
        draw_grid(); 
        move(starty+height, startx); 
        clrtoeol(); 
        mvprintw(starty+height, startx + (width - strlen(status))/2, "%s", status); 
        refresh(); 
        sleep(1); 
        int n = step(); 
        sprintf(status, "%d/%d alive", n, total);    
    }

    delete_grid(grid, rows); 
    delwin(game_win); 
    endwin(); 
}

struct cell **create_grid(int rows, int cols, double distribution)
{
    struct cell **gr = (struct cell **) malloc(rows * sizeof(struct cell *)); 
    for(int i = 0; i < rows; ++i)
    {
        gr[i] = (struct cell *) malloc(cols * sizeof(struct cell));  
        for(int j = 0; j < cols; ++j)
        {
            gr[i][j].y = i+1; 
            gr[i][j].x = 2*j + 1; 
            gr[i][j].alive = (double) rand() / RAND_MAX <= distribution; 
        }
    }
    return gr; 
}

void delete_grid(struct cell **gr, int rows)
{
    for(int i = 0; i < rows; ++i)
        free(gr[i]); 
    free(gr); 
}

void draw_grid()
{
    for(int i = 0; i < rows; ++i)
        for(int j = 0; j < cols; ++j)
        {
            struct cell c = grid[i][j]; 
            if(c.alive)
                wattrset(game_win, A_REVERSE); 
            else 
                wattrset(game_win, A_NORMAL); 
            mvwprintw(game_win, c.y, c.x, "  "); 
        }
    wrefresh(game_win); 
}

bool is_valid(int y, int x)
{
    return y > 0 && x > 0 &&
        y < rows && x < cols; 
}

int alive_neighbors(int r, int c)
{
    int n = 0; 
    for(int i = r-1; i < r+2; ++i)
        for(int j = c-1; j < c+2; ++j)
            if(is_valid(i, j)) n += grid[i][j].alive;
    return n - grid[r][c].alive; 
}

int step()
{
    // neighbors must be counted before updating
    for(int i = 0; i < rows; ++i)
        for(int j = 0; j < cols; ++j)
            grid[i][j].neighbors = alive_neighbors(i, j); 

    int alive = 0; 
    for(int i = 0; i < rows; ++i)
        for(int j = 0; j < cols; ++j)
        {
            int n = grid[i][j].neighbors; 
            if(grid[i][j].alive && (n < 2 || n > 3))
                grid[i][j].alive = false; 
            else if(!grid[i][j].alive && n == 3)
                grid[i][j].alive = true; 
            alive += grid[i][j].alive; 
        }
    return alive; 
}
