


all: kot

kot: ./main.c ilxer.o misc.o kot.o
	gcc -I./lib -c $< -o lib/main.o
	gcc -I./lib -o $@ lib/*.o

ilxer.o: ./lib/ilxer.c ./lib/ilxer.h
	gcc -I./lib -c $< -o lib/$@

misc.o: ./lib/misc.c ./lib/misc.h
	gcc -I./lib -c $< -o lib/$@

kot.o: ./lib/kot.c ./lib/kot.h
	gcc -I./lib -c $< -o lib/$@

clean: 
	rm lib/*.o
	rm kot
