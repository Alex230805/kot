


all: kot

kot: ./main.c ilxer.o misc.o kot.o
	gcc -O3 -I./lib -c $< -o lib/main.o
	gcc -O3 -I./lib -o $@ lib/*.o

ilxer.o: ./lib/ilxer.c ./lib/ilxer.h
	gcc -O3 -DILXER_PRECISE_MODE -I./lib -c $< -o lib/$@

misc.o: ./lib/misc.c ./lib/misc.h
	gcc -O3 -I./lib -c $< -o lib/$@

kot.o: ./lib/kot.c ./lib/kot.h
	gcc -O3 -I./lib -c $< -o lib/$@

clean: 
	rm lib/*.o
	rm kot
