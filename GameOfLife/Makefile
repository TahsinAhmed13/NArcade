OBJS = game_of_life.o

run: game_of_life 
	./game_of_life

game_of_life: ${OBJS}
	cc -o game_of_life ${OBJS} -lncurses   

game_of_life.o: game_of_life.c 
	cc -c game_of_life.c 

clean : 
	-rm *.o game_of_life
    
