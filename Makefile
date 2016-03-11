all:p2.c
	gcc p2.c -o mts -std=c99 -w -lpthread



clean:
	-rm -rf *.o *.exe
