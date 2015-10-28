#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESIZE 1000000

typedef struct sMatrixData{
	int **matrix;
	int **dataMatrix;
	int *multiplicationVector;
	int *multiplicationResult;
	int nZeroes, lines, columns;
}*MDATA;

void insertMatrixLine(int matrixLine[], char *textLine, int line, int columns, int *nZeroes);
void loadMatrix(MDATA mData, FILE *fp);