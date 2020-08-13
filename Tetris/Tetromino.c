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

void rotate(bool **tetromino)
{
    int shift = 0; 
    bool stop = false; 
    for(int j = 3; j >= 0 && !stop; --j)
        for(int i = 0; i < 4; ++i)
            if(tetromino[i][j])
            {
                shift = j; 
                stop = true; 
                break; 
            }

    bool rot[4][4] = {{0}}; 
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            if(tetromino[i][j])
                rot[shift-j][i] = true; 

    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            tetromino[i][j] = rot[i][j]; 
}
