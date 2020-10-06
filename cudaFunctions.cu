#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "general.h"
#include "cudaFunctions.h"
#include "ompFunctions.h"
__constant__ const char CONSERVATIVE[9][4] = { { 'N', 'D', 'E', 'Q' },
		{ 'N', 'E', 'Q', 'K' }, { 'S', 'T', 'A', '^' }, { 'M', 'I', 'L', 'V' },
		{ 'Q', 'H', 'R', 'K' }, { 'N', 'H', 'Q', 'K' }, { 'F', 'Y', 'W', '^' },
		{ 'H', 'Y', '^', '^' }, { 'M', 'I', 'L', 'F' } };
__constant__ const char SEMI_CONSERVATIVE[11][6] = { { 'S', 'A', 'G', '^', '^', '^' }, { 'A',
		'T', 'V', '^', '^', '^' }, { 'C', 'S', 'A', '^', '^', '^' }, { 'S', 'G',
		'N', 'D', '^', '^' }, { 'S', 'T', 'P', 'A', '^', '^' }, { 'S', 'T', 'N',
		'K', '^', '^' }, { 'N', 'E', 'Q', 'H', 'R', 'K' }, { 'N', 'D', 'E', 'Q',
		'H', 'K' }, { 'S', 'N', 'D', 'E', 'Q', 'K' }, { 'H', 'F', 'Y', '^', '^',
		'^' }, { 'F', 'V', 'L', 'I', 'M', '^' } };
__constant__  int ROW_CONSERVATIVE = 9;
__constant__  int COL_CONSERVATIVE = 4;
__constant__  int ROW_SEMI_CONSERVATIVE = 11;
__constant__  int COL_SEMI_CONSERVATIVE = 6;
__device__ int checkColonSign(char a, char b) {
	int aFound;
	int bFound;
	int i, j;
	for (i = 0; i < ROW_CONSERVATIVE; i++) {
		aFound = 0;
		bFound = 0;
		for (j = 0; j < COL_CONSERVATIVE; j++) {
			if (!aFound && a == CONSERVATIVE[i][j]) {
				aFound = 1;
			}
			if (!bFound && b == CONSERVATIVE[i][j]) {
				bFound = 1;
			}
			if (aFound && bFound) {
				return 1;
			}
		}
	}
	return 0;
}
__device__ int checkPointSign(char a, char b) {
	int aFound;
	int bFound;
	int i, j;
	for (i = 0; i < ROW_SEMI_CONSERVATIVE; i++) {
		aFound = 0;
		bFound = 0;
		for (j = 0; j < COL_SEMI_CONSERVATIVE; j++) {
			if (!aFound && a == SEMI_CONSERVATIVE[i][j]) {
				aFound = 1;
			}
			if (!bFound && b == SEMI_CONSERVATIVE[i][j]) {
				bFound = 1;
			}
			if (aFound && bFound) {
				return 1;
			}
		}
	}
	return 0;
}

__global__ void getSignOfTwoLettersCompare(char* seq1_cuda, char* seq2_cuda, char* signs_cuda, int length, int n) {
    int i = blockDim.x * blockIdx.x + threadIdx.x;
	if (i < length) {
		signs_cuda[i] = ' ';
		if (seq2_cuda[i] == '-') {
			return;
		}
		if (seq1_cuda[i+n] == seq2_cuda[i]) {
			signs_cuda[i] = '*';
		}
		else if (checkColonSign(seq1_cuda[i+n], seq2_cuda[i])) {
			signs_cuda[i] = ':';
		}
		else if (checkPointSign(seq1_cuda[i+n], seq2_cuda[i])) {
			signs_cuda[i] = '.';
		}
	}
}

BestScore cudaFunction(char *seq1, char *seq2, int sizeOfSeq1, int sizeOfSeq2,int k ,double *w) {
        
    // Error code to check return values for CUDA calls
    cudaError_t err = cudaSuccess;

    // Allocate memory on GPU to copy the data from the host
    char* seq1_cuda;
char* seq2_cuda;
char* signs_cuda;
    err = cudaMalloc((void **)&seq1_cuda, sizeOfSeq1);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
 }
    err = cudaMalloc((void **)&seq2_cuda, sizeOfSeq2);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
 }
    err = cudaMalloc((void **)&signs_cuda, sizeOfSeq2);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
 }
    // Copy data from host to the GPU memory
    err = cudaMemcpy(seq1_cuda,seq1, sizeOfSeq1,cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to copy data from host to device - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    err = cudaMemcpy(seq2_cuda,seq2, sizeOfSeq2,cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to copy data from host to device - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    // Launch the Kernel
    int threadsPerBlock = (sizeOfSeq2 > 1024 ? 1024 : sizeOfSeq2);
    int blocksPerGrid =(sizeOfSeq2 / threadsPerBlock) +(sizeOfSeq2 % threadsPerBlock != 0);

/* Loop Start */
int i;
	BestScore best, tempBest;
	best.score = -INFINITY;
   	char* stringSigns;
	for (i = 0; i < sizeOfSeq1 - sizeOfSeq2 + 1; i++) {
   	getSignOfTwoLettersCompare<<<blocksPerGrid, threadsPerBlock>>>(seq1_cuda,seq2_cuda,signs_cuda, sizeOfSeq2, i);		
    	err = cudaGetLastError();
    	if (err != cudaSuccess) {
        fprintf(stderr, "Failed to launch vectorAdd kernel -  %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
   	}	

	//copy signs_cuda to CPU memory
	stringSigns =(char*) malloc(sizeOfSeq2);
    	err = cudaMemcpy(stringSigns,signs_cuda,sizeOfSeq2,cudaMemcpyDeviceToHost);
 	if (err != cudaSuccess) {
        fprintf(stderr, "Failed to copy data from host to device - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
   	}
	tempBest.score = getAlignmentScore(stringSigns, sizeOfSeq2,w);
		if (tempBest.score > best.score) {
			best.score = tempBest.score;
			best.bestN = i;
			best.bestK = k;
		}
}
/* Loop End */

    // Free allocated memory on GPU
    if (cudaFree(seq1_cuda) != cudaSuccess) {
        fprintf(stderr, "Failed to free device data - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    if (cudaFree(seq2_cuda) != cudaSuccess) {
        fprintf(stderr, "Failed to free device data - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    if (cudaFree(signs_cuda) != cudaSuccess) {
        fprintf(stderr, "Failed to free device data - %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

return best;

}


