#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>

using namespace std;

int rGen(){

	double val = (double)rand() / RAND_MAX;
	double val2 = (double)rand() / RAND_MAX;
	int factorMultiplicacao = 1;

	int random;
	if (val < 0.05){       //  5% probability
		if (val2 < 0.05)
			factorMultiplicacao = 2;
		else if (val2 < 0.10){
			factorMultiplicacao = 3;
		}
		else if (val2 < 0.20){
			factorMultiplicacao = 4;
		}
		else if (val2 < 0.30){
			factorMultiplicacao = 5;
		}
		else if (val2 < 0.40){
			factorMultiplicacao = 6;
		}
		else if (val2 < 0.50){
			factorMultiplicacao = 7;
		}
		else if (val2 < 0.60){
			factorMultiplicacao = 8;
		}
		else if (val2 < 0.70){
			factorMultiplicacao = 9;
		}
		else{
			factorMultiplicacao = 10;
		}
		random = ((int)ceil(val))*((int)ceil(val2))*factorMultiplicacao;
	}
	else //
		random = 0;

	return (int)random;
}

int randomForVector(){
	double val = (double)rand() / RAND_MAX;
	double val2 = (double)rand() / RAND_MAX;
	int factorMultiplicacao = 1;

	int random;
	if (val < 0.05){       //  5% probability
		random = 0;
	}
	else{
		if (val2 < 0.05)
			factorMultiplicacao = 2;
		else if (val2 < 0.10){
			factorMultiplicacao = 3;
		}
		else if (val2 < 0.20){
			factorMultiplicacao = 4;
		}
		else if (val2 < 0.30){
			factorMultiplicacao = 5;
		}
		else if (val2 < 0.40){
			factorMultiplicacao = 6;
		}
		else if (val2 < 0.50){
			factorMultiplicacao = 7;
		}
		else if (val2 < 0.60){
			factorMultiplicacao = 8;
		}
		else if (val2 < 0.70){
			factorMultiplicacao = 9;
		}
		else{
			factorMultiplicacao = 10;
		}
		random = ((int)ceil(val))*((int)ceil(val2))*factorMultiplicacao;
	}

	return (int)random;
}

int main(int argC, char *argv[]){
	int nLines = atoi(argv[1]);
	int nColumns = atoi(argv[2]);

	printf("Linhas: %d \t Colunas: %d \n", nLines, nColumns);

	FILE *fp;
	fp = fopen(argv[3], "w+" );

	if (!fp){
		cout << "Nao consegui criar o ficheiro \n";
	}

	int nextInt;

	for (int i = 0; i < nLines; i++){
		nextInt = rGen();
		fprintf(fp, "%d", nextInt);

		for (int j = 1; j < nColumns; j++){
			nextInt = rGen();
			//cout << "Valor gerado:< " << nextInt << "\n";
			fprintf(fp, " %d", nextInt);
		}
		fprintf(fp, "\n");
	}

	nextInt = randomForVector();
	fprintf(fp,"%d", nextInt);
	for (int i = 1; i < nLines; i++){
		nextInt = randomForVector();
		fprintf(fp," %d", nextInt);
	}
	fprintf(fp,"\n");

	cout << "Ja escrevi " << (nLines*nColumns) << " valores no ficheiro; \n";
	fclose(fp);

}