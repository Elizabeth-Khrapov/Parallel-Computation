#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include "mpi.h"
#include <math.h>
#include "general.h"
#include "cudaFunctions.h"


int countSigns(char *stringSigns, int size, char sign) {
	int i , count,tid,numOfThreads;
	int* counts;

#pragma omp parallel private(tid)
{
	tid = omp_get_thread_num();	
	numOfThreads= omp_get_num_threads();

#pragma omp single
	counts = (int*)calloc(numOfThreads,sizeof(int));

#pragma omp for
	for (i=0; i < size; i++) {
		if (sign == stringSigns[i])
			counts[tid]++;
	}
}
	count=0;
	for (i=0; i < numOfThreads; i++) {
		count+=counts[i];
	}
	return count;
}

double getAlignmentScore(char *stringSigns, int size, double *w) {

	int stars = countSigns(stringSigns, size, '*');
	int colons = countSigns(stringSigns, size, ':');
	int points = countSigns(stringSigns, size, '.');
	int space = size - stars - points - colons;
	return (w[0] * stars) - (w[1] * colons) - (w[2] * points) - (w[3] * space);

}

BestScore runForAllHyphen(char *seq1, char *seq2, int sizeOfSeq1,
		int sizeOfSeq2, double *w) {
	BestScore best, tempBest;
	best.bestN = -1;
	best.bestK = -1;
	best.score = -INFINITY;
	char *seq2WithHyphen;

#pragma omp parallel private(tempBest, seq2WithHyphen)
	{
		seq2WithHyphen = (char*) malloc(sizeOfSeq2);
		if (!seq2WithHyphen) {
			printf("NOT MALLOCED IN RUN FOR ALL HYPHEN FUNCTION!!!");
			exit(1);
		}
#pragma omp for
		for (int i = 0; i <= sizeOfSeq2; i++) {
			if (i == 0) {
				tempBest = cudaFunction(seq1, seq2, sizeOfSeq1, sizeOfSeq2,
						i, w);
			} else {
				memcpy(seq2WithHyphen, seq2, sizeof(char) * (i));
				memcpy(seq2WithHyphen + (i) + 1, seq2 + (i),
						sizeof(char) * (sizeOfSeq2 - (i)));
				seq2WithHyphen[i] = '-';
				tempBest = cudaFunction(seq1, seq2WithHyphen, sizeOfSeq1,
						sizeOfSeq2, i, w);
			}
#pragma omp critical
			if (tempBest.score > best.score) {
				best = tempBest;
			}
		}
	}
	return best;

}

