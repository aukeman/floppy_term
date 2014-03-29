floppy_term: floppy_term.c
	gcc -g -O3 -o floppy_term floppy_term.c -lncurses 

clean: 
	rm -f floppy_term