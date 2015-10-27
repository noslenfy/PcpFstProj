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

int* makeNonSparseMultiplication(int **nonSparseMatrix, int *vector, int lines, int nZeroes	, int n_threads){
	omp_set_num_threads(n_threads);

	int* result = (int*)malloc(sizeof(int)*lines);
	int i;

	#pragma omp parallel for private(i)
	for(i=0; i<lines; i++){
		result[i] = 0;
	}

	//#pragma omp parallel for schedule(auto) 
	//for(int i=0; i<nZeroes; i++){
	//	#pragma omp atomic
	//	result[nonSparseMatrix[i][0]] += nonSparseMatrix[i][2] * vector[nonSparseMatrix[i][1]];
	//}

	#pragma omp parallel for schedule(auto) 
	for(int i=omp_get_thread_num()*(nZeroes/n_threads); i<nZeroes; i++) {
		int line_number = nonSparseMatrix[i][0];
		for(int j=0; nonSparseMatrix[j][0]==line_number; j++) {
			#pragma omp atomic	
			result[nonSparseMatrix[j][0]] += nonSparseMatrix[j][2] * vector[nonSparseMatrix[j][1]];
		}
	}

	return result;
}

int** unMakeSparseMatrix(int** matrix, int lines, int columns, int nZeroes){
	
	int** nonSparse = (int**)malloc(sizeof(int*)*nZeroes);
	int lineCounter = 0;

	//int id = omp_get_thread_num();	
	for(int i=0; i<lines; i++ ){
		//printf("Sou a thread: %d e vou iterar na linha %d \n",id,i);
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

void freeMatrix(MDATA mData) {
	for(int i=0; i < mData->nZeroes; i++) {
	    free(mData->dataMatrix[i]);
	}
	free(mData->dataMatrix);
	mData->dataMatrix = NULL;
}



int main(int argc, char *argv[]){
	#define N_TIMES 5
	#define N_CONFIGURATIONS 7
	int n_threads[N_CONFIGURATIONS] = {1,2,4,8,16,24,48};

	// used to  measure times
	double start=0, finish=0, total=0;

	int lines = atoi(argv[2]), columns=atoi(argv[3]);
	
	// load matrix from file
	start = omp_get_wtime();
	MDATA mData = callLoadMatrix(argv[1],lines,columns);
	printf("Carreguei a matrix em: %fs\n", omp_get_wtime()-start); 

	if(!mData){
		return -1;
	}

	start = omp_get_wtime();
	mData->dataMatrix = unMakeSparseMatrix(mData->matrix,mData->lines,mData->columns,mData->nZeroes);
	finish = omp_get_wtime()-start;
	printf("Demorei %f a converter a matriz\n", finish);


	// start computation
	for (int i=0; i<N_CONFIGURATIONS;i++) {
		for(int j=0; j<N_TIMES;j++) {
			start = omp_get_wtime();
			if (mData->multiplicationResult) free(mData->multiplicationResult);
			mData->multiplicationResult = makeNonSparseMultiplication(mData->dataMatrix,mData->multiplicationVector, mData->lines, mData->nZeroes, n_threads[i]);
			finish = omp_get_wtime()-start;
			for(int f=0; f<mData->lines; f++){
				printf("%d;  ", mData->multiplicationResult[f]);
			}
			putchar('\n');	
			total += finish;
			printf("%d - execution with %d threads took:\t %f \n", j+1, n_threads[i], finish);

		}
		printf("\nAverage execution with %d threads took:\t %f\n\n", n_threads[i], (total/N_TIMES));
		total=0;
	}
	
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