#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define BOARD_ROWS 6
#define BOARD_COLS 7

#define RED 1
#define YELLOW 2

WINDOW *game_win; 
int GAME_LINES, GAME_COLS; 
int GAME_START_Y, GAME_START_X; 
const char title[] = "Connect 4"; 

int BOARD_HEIGHT, BOARD_WIDTH; 
int CHIP_HEIGHT, CHIP_WIDTH; 
int SEP_HEIGHT, SEP_WIDTH; 
int OFFSET_Y, OFFSET_X; 

typedef struct CHIP 
{
    int y, x; 
    int c; 
}CHIP;

void setup(); 

void draw_board(const CHIP [BOARD_ROWS][BOARD_COLS]); 

CHIP *play(CHIP [BOARD_ROWS][BOARD_COLS]); 
void print_win_msg(int c); 

bool play_again(); 
void reset(CHIP [BOARD_ROWS][BOARD_COLS]); 
void *blink(void *); 

void cleanup(); 

int main() 
{
    setup(); 

    CHIP board[BOARD_ROWS][BOARD_COLS]; 
    for(int i = 0; i < BOARD_ROWS; ++i)
        for(int j = 0; j < BOARD_COLS; ++j)
        {
            board[i][j].y = OFFSET_Y + ((i+1) * CHIP_HEIGHT) + ((i+1) * SEP_HEIGHT); 
            board[i][j].x = OFFSET_X + (j * CHIP_WIDTH) + (j * SEP_WIDTH); 
            board[i][j].c = A_REVERSE; 
        }

    pthread_t id; 
    do
    {
        CHIP *win_chips; 
        win_chips = play(board);  
        if(win_chips != NULL)
            print_win_msg(win_chips[0].c); 
        else 
            print_win_msg(-1); 
        refresh(); 

        // TODO: replace with forking and signals
        pthread_create(&id, NULL, blink, (void *) win_chips); 
        if(!play_again())
        {
            if(win_chips != NULL) free(win_chips); 
            break; 
        }
        pthread_cancel(id); 
        move(GAME_START_Y + GAME_LINES, 0);
        clrtoeol(); 
        refresh(); 
        reset(board); 
    }while(true); 
    
    cleanup(); 
}

void init_pairs() 
{
    init_pair(RED, -1, COLOR_RED); 
    init_pair(YELLOW, -1, COLOR_YELLOW); 
}

void calculate() 
{
    GAME_LINES = LINES * 0.6; 
    GAME_COLS = COLS * 0.5; 

    BOARD_HEIGHT = GAME_LINES * 0.7; 

    CHIP_HEIGHT = BOARD_HEIGHT * 0.8 / (BOARD_ROWS + 1); 
    CHIP_WIDTH = CHIP_HEIGHT * 2; 

    SEP_HEIGHT = (BOARD_HEIGHT - CHIP_HEIGHT * BOARD_ROWS) / BOARD_ROWS; 
    SEP_WIDTH = SEP_HEIGHT * 2; 

    BOARD_WIDTH = CHIP_WIDTH * BOARD_COLS + SEP_WIDTH * (BOARD_COLS - 1); 

    GAME_START_Y = (LINES - GAME_LINES) / 2; 
    GAME_START_X = (COLS - GAME_COLS) / 2; 

    OFFSET_Y = (GAME_LINES - BOARD_HEIGHT) / 2; 
    OFFSET_X = (GAME_COLS - BOARD_WIDTH) / 2; 
}

void setup()
{
    initscr(); 
    refresh(); 
    cbreak(); 
    noecho(); 
    curs_set(0); 
    keypad(stdscr, TRUE); 

    start_color(); 
    use_default_colors();  
    init_pairs(); 

    calculate(); 
    game_win = newwin(GAME_LINES, GAME_COLS, GAME_START_Y, GAME_START_X); 
    printw("PRESS F1 to exit"); 
    mvprintw(GAME_START_Y-1, GAME_START_X + (GAME_COLS - strlen(title)) / 2, "%s", title); 
    refresh(); 
    keypad(game_win, TRUE); 
}

void draw_rect(int attr, int height, int width, int starty, int startx)
{
    wattrset(game_win, attr); 
    for(int i = 0; i < height; ++i)
        mvwhline(game_win, starty + i, startx, ' ', width); 
    wattrset(game_win, A_NORMAL); 
}

void draw_chip(CHIP chip)
{
    draw_rect(chip.c, CHIP_HEIGHT, CHIP_WIDTH, chip.y, chip.x); 
}

void erase_chip(CHIP chip)
{
    draw_rect(A_NORMAL, CHIP_HEIGHT, CHIP_WIDTH, chip.y, chip.x); 
}

void draw_board(const CHIP board[BOARD_ROWS][BOARD_COLS])
{
    for(int i = 0; i < BOARD_ROWS; ++i)
        for(int j = 0; j < BOARD_COLS; ++j)
            draw_chip(board[i][j]); 
}

int get_empty_row(const CHIP board[BOARD_ROWS][BOARD_COLS], int c)
{
    for(int r = BOARD_ROWS-1; r >= 0; --r)
        if(board[r][c].c == A_REVERSE)
            return r; 
    return -1; 
}

bool is_valid(int r, int c)
{
    return 0 <= r && r < BOARD_ROWS &&
            0 <= c && c < BOARD_COLS; 
}

CHIP *get_win(CHIP board[BOARD_ROWS][BOARD_COLS], int r, int c, void (*next) (int *, int *))
{
    CHIP *chips = (CHIP *) malloc(4 * sizeof(CHIP)); 
    int current_color = A_NORMAL; 
    int n = 0; 
    while(is_valid(r, c))
    {
        if(board[r][c].c != A_REVERSE && board[r][c].c == current_color)
            chips[n++] = board[r][c]; 
        else
        {
            current_color = board[r][c].c; 
            chips[0] = board[r][c]; 
            n = 1; 
        }
        if(n == 4)
            return chips; 
        next(&r, &c); 
    }
    free(chips); 
    return NULL; 
}

void next_col(int *r, int *c)
{
    ++(*c); 
}

void next_row(int *r, int *c)
{
    --(*r); 
}

void next_ldiagonal(int *r, int *c)
{
    --(*r); 
    ++(*c); 
}

void next_rdiagonal(int *r, int *c)
{
    --(*r); 
    --(*c); 
}

CHIP *get_all_wins(CHIP board[BOARD_ROWS][BOARD_COLS], int r, int c)
{
    CHIP *horizontal = get_win(board, r, 0, next_col); 
    if(horizontal != NULL) return horizontal; 

    CHIP *vertical = get_win(board, BOARD_ROWS-1, c, next_row); 
    if(vertical != NULL) return vertical; 

    int lo = MIN(c - 0, BOARD_ROWS-1 - r); 
    CHIP *left_diagonal = get_win(board, r+lo, c-lo, next_ldiagonal); 
    if(left_diagonal != NULL) return left_diagonal; 

    int ro = MIN(BOARD_COLS-1 - c, BOARD_ROWS-1 - r); 
    CHIP *right_diagonal = get_win(board, r+ro, c+ro, next_rdiagonal); 
    if(right_diagonal != NULL) return right_diagonal; 

    return NULL; 
}

bool is_empty(const CHIP board[BOARD_ROWS][BOARD_COLS])
{
    for(int i = 0; i < BOARD_COLS; ++i)
        if(board[BOARD_ROWS-1][i].c != A_REVERSE)
            return false; 
    return true; 
}

bool is_full(const CHIP board[BOARD_ROWS][BOARD_COLS])
{
    for(int i = 0; i < BOARD_COLS; ++i)
        if(board[0][i].c == A_REVERSE)
            return false; 
    return true; 
}

CHIP *play(CHIP board[BOARD_ROWS][BOARD_COLS])
{
    int ch = 0; 

    int turn = COLOR_PAIR(RED); 
    int col = 0; 
    CHIP *win_chips = NULL; 

    // use a do loop to ensure board gets printed at least once
    do
    {
        // process input 
        if(ch == KEY_LEFT && col > 0)
            --col; 
        else if(ch == KEY_RIGHT && col < BOARD_COLS-1)
            ++col; 
        else if(ch == KEY_DOWN)
        {
            int row = get_empty_row(board, col); 
            if(row >= 0)
            {
                board[row][col].c = turn; 
                turn = turn == COLOR_PAIR(RED) ? COLOR_PAIR(YELLOW) : COLOR_PAIR(RED); 
                win_chips = get_all_wins(board, row, col); 
            }

        }

        // draw board 
        werase(game_win); 
        box(game_win, 0, 0); 
        draw_board(board); 
        draw_rect(turn, CHIP_HEIGHT, CHIP_WIDTH, OFFSET_Y, 
                OFFSET_X + col * CHIP_WIDTH + col * SEP_WIDTH); 
        wrefresh(game_win); 
        
        // is game finished
        if(win_chips != NULL || is_full(board)) 
            break; 

        // get input for next cycle 
        if((ch = wgetch(game_win)) == KEY_F(1))
            cleanup();  
    }while(true); 

    // remove the next chip from play
    draw_rect(A_NORMAL, CHIP_HEIGHT, CHIP_WIDTH, OFFSET_Y, 
            OFFSET_X + col * CHIP_WIDTH + col * SEP_WIDTH); 
    wrefresh(game_win); 

    return win_chips; 
}

void print_win_msg(int c)
{
    const char p1_win[] = "Red wins!!!"; 
    const char p2_win[] = "Yellow wins!!!"; 
    const char tie[] = "It's a tie..."; 

    if(c == COLOR_PAIR(RED))
        mvprintw(GAME_START_Y + GAME_LINES, 
                GAME_START_X + (GAME_COLS - strlen(p1_win)) / 2, "%s", p1_win); 
    else if(c == COLOR_PAIR(YELLOW))
        mvprintw(GAME_START_Y + GAME_LINES, 
                GAME_START_X + (GAME_COLS - strlen(p2_win)) / 2, "%s", p2_win); 
    else 
        mvprintw(GAME_START_Y + GAME_LINES, 
                GAME_START_X + (GAME_COLS - strlen(tie)) / 2, "%s", tie); 
}

bool play_again()
{
    const char replay[] = "Play Again"; 
    const char quit[] = "Quit Game";  

    int ch = 0; 
    int y = GAME_START_Y + GAME_LINES; 
    y += (LINES - (GAME_START_Y + GAME_LINES + 1)) / 2; 
    bool again = true; 
    do
    {
        // handle input
        if(ch == KEY_DOWN && again)         again = false; 
        else if(ch == KEY_UP && !again)     again = true; 
        else if(ch == KEY_F(1))             cleanup(); 

        // print options
        mvprintw(y, GAME_START_X + (GAME_COLS - strlen(replay)) / 2, "%s", replay); 
        mvprintw(y+1, GAME_START_X + (GAME_COLS - strlen(quit)) / 2, "%s", quit); 
        if(again)
        {
            mvchgat(y, GAME_START_X, GAME_COLS, A_REVERSE, 0, NULL);  
            mvchgat(y+1, GAME_START_X, GAME_COLS, A_NORMAL, 0, NULL); 
        }
        else
        {
            mvchgat(y, GAME_START_X, GAME_COLS, A_NORMAL, 0, NULL);  
            mvchgat(y+1, GAME_START_X, GAME_COLS, A_REVERSE, 0, NULL); 
        }
    }while((ch = getch()) != 10); 
    
    // delete options 
    move(y, 0); 
    clrtoeol(); 
    move(y+1, 0); 
    clrtoeol();

    return again; 
}

void shift_down(CHIP board[BOARD_ROWS][BOARD_COLS], int beg)
{
    int down[BOARD_ROWS][BOARD_COLS];
    for(int i = 0; i < BOARD_ROWS; ++i)
        for(int j = 0; j < BOARD_COLS; ++j)
            down[i][j] = i > beg ? board[i-1][j].c : A_REVERSE;  
    
    for(int i = 0; i < BOARD_ROWS; ++i)
        for(int j = 0; j < BOARD_COLS; ++j)
            board[i][j].c = down[i][j]; 
}

void reset(CHIP board[BOARD_ROWS][BOARD_COLS]) 
{
    for(int i = 0; i < BOARD_ROWS && !is_empty(board); ++i)
    {
        draw_board(board); 
        wrefresh(game_win); 
        shift_down(board, i); 
        sleep(1); 
    }
    draw_board(board); 
    wrefresh(game_win); 
}

void *blink(void *pargs)
{
    CHIP *chips = (CHIP *) pargs; 
    while(true)
    {
        for(int i = 0; i < 4; ++i)
            draw_chip(chips[i]); 
        wrefresh(game_win); 
        sleep(1); 
        for(int i = 0; i < 4; ++i)
            erase_chip(chips[i]); 
        wrefresh(game_win); 
        sleep(1); 
    }
}

void cleanup()
{
    delwin(game_win); 
    endwin(); 
    exit(0); 
}
