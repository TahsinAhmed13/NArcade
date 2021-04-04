OBJS = connect4.o

run: connect4
	./connect4

connect4: ${OBJS} 
	cc -o connect4 ${OBJS} -lpthread -lncurses 

connect4.o: connect4.c
	cc -c connect4.c

clean: 
	-rm *.o connect4

