#ifndef TETROMINO_H
#define TETROMINO_H

typedef int ** TETROMINO; 

TETROMINO get_copy(int n); 
void del_copy(TETROMINO copy); 

int get_height(TETROMINO tetromino); 
int get_width(TETROMINO tetromino); 

TETROMINO rotate(TETROMINO tetromino); 

#endif
