Tetris: Tetris.o Tetromino.o
	cc -lm -lncurses Tetris.o Tetromino.o -o Tetris

Tetris.o: Tetris.c Tetromino.h
	cc -c Tetris.c 

Tetromino.o: Tetromino.c Tetromino.h
	cc -c Tetromino.c 

TetrominoTest.o: TetrominoTest.c Tetromino.h
	cc -c TetrominoTest.c

.PHONY: tests clean 

tests: Tetromino.o TetrominoTest.o
	cc -lncurses Tetromino.o TetrominoTest.o -o TetrominoTest

clean: 
	-rm *.o
