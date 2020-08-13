.PHONY: tests clean 

Tetromino.o: Tetromino.c Tetromino.h
	cc -c Tetromino.c 

TetrominoTest.o: TetrominoTest.c Tetromino.h
	cc -c TetrominoTest.c

tests: Tetromino.o TetrominoTest.o
	cc -lncurses Tetromino.o TetrominoTest.o -o TetrominoTest

clean: 
	-rm *.o
