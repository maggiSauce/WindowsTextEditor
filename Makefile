wte: src/wte.c
	gcc -Wall -std=c99 -o wte src/wte.c -lm

clean:
	rm -f *.o
	rm -f wte
