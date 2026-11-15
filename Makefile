CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra

sudoku: solver.c
	$(CC) $(CFLAGS) -o sudoku solver.c

check: sudoku
	./sudoku --check

clean:
	rm -f sudoku sudoku.exe *.o

.PHONY: check clean
