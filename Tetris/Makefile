OBJS = tetris.o tetromino.o
TEST_OBJS = tetromino_test.o tetromino.o

all: tests run

run: tetris
	./tetris

tetris: ${OBJS}
	cc -o tetris ${OBJS} -lm -lncurses

tetris.o: tetris.c tetromino.h
	cc -c tetris.c

tetromino.o: tetromino.c tetromino.h
	cc -c tetromino.c

tetromino_test.o: tetromino_test.c tetromino.h
	cc -c tetromino_test.c

tests: ${TEST_OBJS}
	cc -o tetromino_test ${TEST_OBJS} -lncurses

clean: 
	-rm *.o tetromino_test tetris
