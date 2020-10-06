
#ifndef GENERAL_H_
#define GENERAL_H_

#define FILE_NAME "input.txt"
#define FILE_NAME_TO_WRITE "output.txt"
#define MASTER 0

typedef struct {
	int bestN;
	int bestK;
	double score;
} BestScore;

char** readFromFile(char **seq1, double *w, int *amountOfSeq2);
#endif /* GENERAL_H_ */

