project1: fileToMatrix.o project1.o
	gcc -fopenmp -o project1 fileToMatrix.o project1.o
	rm *.o

fileToMatrix.o : fileToMatrix.c fileToMatrix.h
	gcc -Wall -fopenmp -std=c99 -c fileToMatrix.c

project1.o: project1.c project1.h
	gcc -c -Wall -fopenmp -std=c99 project1.c
