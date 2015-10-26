#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "fileToMatrix.h"

MDATA initializeMatrixData(int lines, int columns){
	MDATA mData = (MDATA)malloc(sizeof(struct sMatrixData));
	mData->matrix = NULL;
	mData->dataMatrix = NULL;
	mData->multiplicationVector=NULL;
	mData->multiplicationResult=NULL;
	mData->nZeroes = 0;
	mData->lines = lines;
	mData->columns = columns;

	return mData;
}

MDATA callLoadMatrix(char *fileName, int lines, int columns){
	MDATA mData = initializeMatrixData(lines,columns);

	if(!mData){
		return NULL;
	}

	FILE *fp = fopen(fileName,"r");

	if(fp == NULL){
		return NULL;
	}

	loadMatrix(mData, fp);

	fclose(fp);

	return mData;
}

int* makeStandardMultiplication(int **matrix, int*vector, int lines, int columns){
	int* result = (int*)malloc(sizeof(int)*lines);

	for(int i=0; i<lines; i++){
		int lineResult = 0;
		for(int j=0; j<columns; j++){
			lineResult += matrix[i][j] * vector[j];
		}
		result[i] = lineResult;
	}

	return result;
}

int* makeNonSparseMultiplication(int **nonSparseMatrix, int *vector, int lines, int nZeroes){
	int* result = (int*)malloc(sizeof(int)*lines);

	for(int i=0; i<lines; i++){
		result[i] = 0;
	}

	for(int i=0; i<nZeroes; i++){
		result[nonSparseMatrix[i][0]] += nonSparseMatrix[i][2] * vector[nonSparseMatrix[i][1]];
	}

	return result;
}

int** unMakeSparseMatrix(int** matrix, int lines, int columns, int nZeroes){
	//pos[0] = nrRow, pos[1] = nrColumn, pos[2] = value
	
	
	printf("nZeroes: %d \n",nZeroes);
	double start = omp_get_wtime();

	int** nonSparse = (int**)malloc(sizeof(int*)*nZeroes);
	int lineCounter = 0;

	//#pragma omp for
	//#pragma omp parallel

	#pragma omp parallel

	for(int i=0; i<lines; i++){
		int id = omp_get_thread_num();
		printf("Sou a thread: %d e vou iterar na linha %d \n",id,i);
		for(int j=0; j<columns; j++){
			if(matrix[i][j] != 0){
				nonSparse[lineCounter] = (int*)malloc(sizeof(int)*3);
				nonSparse[lineCounter][0] = i;
				nonSparse[lineCounter][1] = j;
				nonSparse[lineCounter][2] = matrix[i][j];
				lineCounter ++;
			}
		}
	}
	printf("Time: \t %f \n", omp_get_wtime()-start); 
	printf("LineCounterResult: %d \n",lineCounter);
	return nonSparse;
}

void testerStandardMultiplication(int **matrix, int lines, int columns){
	//int array[10] = {1,2,3,4,5,6,7,8,9,10};
	int arrayProf[4] = {2,5,1,8};
	int* result = makeStandardMultiplication(matrix,arrayProf,lines,columns);

	if(!result){
		puts("Unable to create matrix \n");
	}
}

int main(int argc, char *argv[]){
	

	int lines = atoi(argv[2]), columns=atoi(argv[3]);

	MDATA mData = callLoadMatrix(argv[1],lines,columns);

	if(!mData){
		return -1;
	}

	mData->dataMatrix = unMakeSparseMatrix(mData->matrix,mData->lines,mData->columns,mData->nZeroes);
	
	
	/*puts("Impressão da matriz contida:");
	for(int i=0; i<mData->nZeroes; i++){
		for(int j=0; j<3; j++){
			printf("%d \t",mData->dataMatrix[i][j]);
		}
		putchar('\n');
	}


	mData->multiplicationResult = makeStandardMultiplication(mData->matrix, mData->multiplicationVector, mData->lines, mData->columns);
	puts("Vector de multiplicação:");
	for(int i=0; i<mData->lines; i++){
		printf("%d; ",mData->multiplicationVector[i]);
	}

	putchar('\n');

	puts("Resultados da multiplicação Standard: \n");
	for(int i=0; i<mData->lines; i++){
		printf("%d;  ",mData->multiplicationResult[i]);
	}
	putchar('\n');

	puts("Resultados da multiplicação da matriz não espersa: \n");
	free(mData->multiplicationResult);
	mData->multiplicationResult = makeNonSparseMultiplication(mData->dataMatrix,mData->multiplicationVector, mData->lines, mData->nZeroes);

	for(int i=0; i<mData->lines; i++){
		printf("%d;  ", mData->multiplicationResult[i]);
	}
	putchar('\n');*/

	return 0;
}