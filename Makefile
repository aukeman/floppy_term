flappy_term: flappy_term.c
	gcc -g -o flappy_term flappy_term.c -lncurses 

clean: 
	rm -f flappy_term