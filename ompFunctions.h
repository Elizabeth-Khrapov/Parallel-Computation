#ifndef OMPFUNCTIONS_H_
#define OMPFUNCTIONS_H_


int countSigns(char *stringSigns, int size, char sign);
double getAlignmentScore(char *stringSigns, int size,double *w);
BestScore runForAllHyphen(char *seq1, char *seq2, int sizeOfSeq1,
		int sizeOfSeq2, double *w);

#endif /* OMPFUNCTIONS_H_ */
