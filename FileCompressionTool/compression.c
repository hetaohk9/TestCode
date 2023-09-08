//This contain functions related to the compression algorithm.

#include<stdio.h>
#include<stdlib.h>

void compressFile(const char* inputFileName, const char* outputFileName) {
	FILE *inputFile, *outputFile;
	//open file
	inputFile = fopen(inputFileName, "r");
	outputFile = fopen(outputFileName, "w");
	
	
	if (inputFile == NULL || outputFile == NULL) {
		printf("File cannot be opened\n");
		return ;
	}

	
	char ch, prevCh;
	int count = 1;
	prevCh = fgetc(inputFile);


	while ((ch = fgetc(inputFile)) != EOF) {
		if (ch == prevCh) {
			count ++;
		} else {
			fprintf(outputFile, "%d%c", count, prevCh);
			count = 1;
			prevCh = ch;
		}
	}

	fprintf(outputFile, "%d%c", count, prevCh);

	//Close files.
	fclose(inputFile);
	fclose(outputFile);
}
