#include <stdbool.h>
#include <stdlib.h>

#include "Tetromino.h"

bool const g_tetrominos[7][4][4] = {
    // straight
    {{1, 1, 1, 1}, 
    {0, 0, 0, 0}, 
    {0, 0, 0, 0}, 
    {0, 0, 0, 0}}, 

    // square 
    {{1, 1, 0, 0}, 
    {1, 1, 0, 0}, 
    {0, 0, 0, 0}, 
    {0, 0, 0, 0}}, 

    // T-pose
    {{1, 1, 1, 0}, 
    {0, 1, 0, 0}, 
    {0, 0, 0, 0}, 
    {0, 0, 0, 0}}, 
    
    // left L
    {{1, 0, 0, 0}, 
    {1, 0, 0, 0},
    {1, 1, 0, 0}, 
    {0, 0, 0, 0}}, 

    // right L
    {{0, 1, 0, 0}, 
    {0, 1, 0, 0},
    {1, 1, 0, 0}, 
    {0, 0, 0, 0}}, 

    // forward skew
    {{0, 1, 1, 0}, 
    {1, 1, 0, 0}, 
    {0, 0, 0, 0}, 
    {0, 0, 0, 0}}, 
    
    // backward skew
    {{1, 1, 0, 0}, 
    {0, 1, 1, 0}, 
    {0, 0, 0, 0}, 
    {0, 0, 0, 0}} 
}; 

bool **get_copy(int n)
{
    n %= sizeof(g_tetrominos) / sizeof(g_tetrominos[0]); 
    bool **copy = (bool **) malloc(4 * sizeof(bool *)); 
    for(int i = 0; i < 4; ++i)
    {
        copy[i] = (bool *) malloc(4 * sizeof(bool)); 
        for(int j = 0; j < 4; ++j)
            copy[i][j] = g_tetrominos[n][i][j]; 
    }
    return copy; 
}

void del_copy(bool **copy)
{
    for(int i = 0; i < 4; ++i)
        free(copy[i]); 
    free(copy); 
}

int get_height(bool **tetromino)
{
    int h = 0; 
    for(int i = 0; i < 4; ++i)
    {
        bool tmp = false; 
        for(int j = 0; j < 4 && !tmp; ++j)
            tmp = tmp || tetromino[i][j]; 
        if(tmp) ++h; 
        else    break; 
    }
    return h; 
}

int get_width(bool **tetromino)
{
    int w = 0; 
    for(int j = 0; j < 4; ++j)
    {
        bool tmp = false; 
        for(int i = 0; i < 4 && !tmp; ++i)
            tmp = tmp || tetromino[i][j]; 
        if(tmp) ++w; 
        else    break; 
    }
    return w; 
}

void rotate(bool **tetromino)
{
    int shift = get_width(tetromino) - 1; 
    bool rotated[4][4] = {{0}}; 
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            if(tetromino[i][j])
                rotated[shift-j][i] = true; 

    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            tetromino[i][j] = rotated[i][j]; 
}
