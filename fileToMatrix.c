#include "fileToMatrix.h"

void insertMatrixLine(int matrixLine[], char *textLine, int line, int columns, int* nZeroes){
	char *token;

	token = strtok(textLine," ");
	int j = 0;

	while(token != NULL){
		matrixLine[j] = atoi(token);

		if(matrixLine[j]){
			*nZeroes = *nZeroes + 1;
		}

		j++;

		token = strtok(NULL, " ");
	}
}

void insertMultiplicationVector(int* vector, char *textLine, int lines){
	puts("Vou agora meter o vetor de multiplicação e vou estourar aqui \n");
	char *token;

	token = strtok(textLine, " ");
	int j=0;

	while(token !=NULL){
		vector[j] = atoi(token);
		j++;

		token = strtok(NULL, " ");
	}

}

void loadMatrix(MDATA mData, FILE *fp){
	if(!mData){
		return;
	}

	char str[LINESIZE];
	int mline = 0;

	mData->matrix = (int**)malloc(sizeof(int*)*mData->lines);

	while( fgets (str, LINESIZE, fp)!= NULL ){
		/* writing content to stdout */
		mData->matrix[mline] = (int*)malloc(sizeof(int)*mData->columns);

		if(mline == mData->lines){
			if(mData->multiplicationVector){
				free(mData->multiplicationVector);	
			}
			mData->multiplicationVector = (int*)malloc(sizeof(int)*mData->lines);
			insertMultiplicationVector(mData->multiplicationVector, str, mData->lines);
		}
		else{
			insertMatrixLine(mData->matrix[mline], str, mline, mData->columns, &mData->nZeroes);
			if((mline+1)==mData->lines){
				puts("Atenção a este possível erro \n");
			}
		}

		mline++;
   }
}