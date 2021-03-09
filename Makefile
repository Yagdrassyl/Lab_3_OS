all:compile

compile:SudokuValidator.c
	gcc -pthread -o SudokuValidator.o SudokuValidator.c

omp:SudokuValidator.c
	gcc -fopenmp SudokuValidator.c -o SudokuValidator.o