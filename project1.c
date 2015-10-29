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

int* makeNonSparseMultiplicationSequential(int **nonSparseMatrix, int *vector, int lines, int nZeroes){
	puts("Vou agora pa esta \n");
	int* result = (int*)malloc(sizeof(int)*lines);

	for(int i=0; i<lines; i++){
		result[i] = 0;
	}

	/*
	for(int i=0; i<nZeroes; i++){
		result[nonSparseMatrix[i][0]] += nonSparseMatrix[i][2] * vector[nonSparseMatrix[i][1]];
	}*/

	for(int i=0; i<nZeroes; i++){
	//	#pragma omp atomic
		result[nonSparseMatrix[i][0]] += nonSparseMatrix[i][2] * vector[nonSparseMatrix[i][1]];
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

	/*
	#pragma omp parallel for schedule(auto) 
	for(int i=omp_get_thread_num()*(nZeroes/n_threads); i<nZeroes; i++) {
		int line_number = nonSparseMatrix[i][0];
		for(int j=0; nonSparseMatrix[j][0]==line_number; j++) {
			#pragma omp atomic	
			result[nonSparseMatrix[j][0]] += nonSparseMatrix[j][2] * vector[nonSparseMatrix[j][1]];
		}
	}*/

	/*
	#pragma omp parallel for schedule(auto)
	for(int i=omp_get_thread_num()*(nZeroes/n_threads); i<nZeroes; i+=(nZeroes/n_threads)){
		int numLinha = i;
		printf("Sou a thread %d e vou trabalhar nas linhas %d - %d \n",omp_get_thread_num(),omp_get_thread_num()*(nZeroes/n_threads),numLinha + (nZeroes/n_threads));

		for(int j=numLinha; j<numLinha + (nZeroes/n_threads) && j<nZeroes; j++){
			#pragma omp atomic
			result[nonSparseMatrix[j][0]] += nonSparseMatrix[j][2] * vector[nonSparseMatrix[j][1]];	
		}
	}*/
	
	#pragma omp parallel for schedule(auto)
	for(int i=0; i<nZeroes; i+=(nZeroes/n_threads)){
		//puts("Estou pela primeira vez no for \n");
		for(int j=i; j<nZeroes && j<(i+(nZeroes/n_threads)); j++){
			#pragma omp atomic
			result[nonSparseMatrix[j][0]] += nonSparseMatrix[j][2] * vector[nonSparseMatrix[j][1]];
		}

	}

	return result;
}

int** unMakeSparseMatrix(int** matrix, int lines, int columns, int nZeroes){
	//printf("NZEROES: %d -> Lines: %d\n",nZeroes,lines);
	long int aux = (sizeof(int)*nZeroes);
	printf("Tamanho do malloc que vou fazer: %ld \n",(aux/1024));
	int** nonSparse = (int**)malloc(sizeof(int*)*nZeroes);
	//puts("Consegui fazer o malloc");
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

//equalVectors(stdMultResult,mData->multiplicationResult,mData->lines)
int equalVectors(int *vector1, int *vector2, int lines){
	int result = 1;

	for(int i=0; i<lines; i++){
		if(vector1[i] != vector2[i]){
			result = 0;
		}
	}

	return result;
}

int main(int argc, char *argv[]){
	#define N_TIMES 5
	#define N_CONFIGURATIONS 3
	int n_threads[N_CONFIGURATIONS] = {1,2,4};

	// used to  measure times
	double start=0, finish=0, total=0;

	int lines = atoi(argv[2]), columns=atoi(argv[3]);
	
	// load matrix from file
	start = omp_get_wtime();
	MDATA mData = callLoadMatrix(argv[1],lines,columns);
	printf("Carreguei a matrix em: %fs\n", omp_get_wtime()-start); 
	printf("NZEROES: %d \n",mData->nZeroes);

	if(!mData){
		return -1;
	}

	start = omp_get_wtime();
	mData->dataMatrix = unMakeSparseMatrix(mData->matrix,mData->lines,mData->columns,mData->nZeroes);
	finish = omp_get_wtime()-start;
	printf("Demorei %f a converter a matriz\n", finish);

	// start computation
	for (int i=0; i<N_CONFIGURATIONS;i++) {
		printf("Configuration: %d \n",n_threads[i]);
		for(int j=0; j<N_TIMES;j++) {
			start = omp_get_wtime();
			if (mData->multiplicationResult) free(mData->multiplicationResult);
			mData->multiplicationResult = makeNonSparseMultiplication(mData->dataMatrix,mData->multiplicationVector, mData->lines, mData->nZeroes, n_threads[i]);
			finish = omp_get_wtime()-start;
			/*for(int f=0; f<mData->lines; f++){
				printf("%d;  ", mData->multiplicationResult[f]);
			}*/
			putchar('\n');	
			total += finish;
			printf("%d - execution with %d threads took:\t %f \n", j+1, n_threads[i], finish);

		}
		printf("\nAverage execution with %d threads took:\t %f\n\n", n_threads[i], (total/N_TIMES));
		total=0;
	}
	
	start = omp_get_wtime();
	int *stdMultResult = makeStandardMultiplication(mData->matrix,mData->multiplicationVector, mData->lines, mData->columns);
	finish = omp_get_wtime()-start;
	printf("Demorei %f segundos a fazer a multiplicacao normal \n",finish);
	//int* makeNonSparseMultiplicationSequential(int **nonSparseMatrix, int *vector, int lines, int nZeroes){

	start = omp_get_wtime();
	int *stdMultResultNonSparse = makeNonSparseMultiplicationSequential(mData->dataMatrix,mData->multiplicationVector, mData->lines, mData->nZeroes);
	finish = omp_get_wtime()-start;
	printf("Demorei %f segundos a fazer a multiplicacao nonSparseSequencial \n",finish);

	if(equalVectors(stdMultResult,mData->multiplicationResult,mData->lines)){
		//if(equalVectors(stdMultResult,stdMultResultNonSparse,mData->lines))
		/*puts("Impressão do grande resultado \n");
		for(int i=0; i<mData->lines; i++){
			printf("%d; ",stdMultResult[i]);
		}
		putchar('\n');*/
		puts("Fucking results Match \n");
	}
	else{
		puts("Fucking results don't match \n");
	}

	if(equalVectors(stdMultResult,stdMultResultNonSparse,mData->lines)){
		puts("Fucking second results Match \n");
	}
	else{
		puts("Fucking second results don't match \n");
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