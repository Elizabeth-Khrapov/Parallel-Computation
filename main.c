
#include "mpi.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include "general.h"
#include "ompFunctions.h"
#include "mpiFunction.h"

int main(int argc, char *argv[]) {
	double* w;
	char* seq1;
	char* seq2;
	char **arrySeq2;
	int amountOfSeq2,lenghtOfSeq1,i,lenghtOfSeq2;
	FILE *fp ;
	BestScore best;

// MPI
	int my_rank; /* rank of process */
	int p; /* number of processes */
	int tag = 0; /* tag for messages */
	MPI_Status status; /* return status for receive */
	/* start up MPI */

	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Datatype MPI_SCORE = generateMPIDataTypeForStructScore();
	w = (double*) malloc(4 * sizeof(double));
	if (my_rank == MASTER) {
		fp = fopen(FILE_NAME_TO_WRITE, "w");
		if (!fp) {
			fprintf(stderr, "Error opening file '%s'\n", FILE_NAME_TO_WRITE);
		}
		arrySeq2 = readFromFile(&seq1, w,&amountOfSeq2);
		//printf("Hello MPI World From process 0: Num processes: %d\n", p);
		MPI_Send(w,4,MPI_DOUBLE,1,tag,MPI_COMM_WORLD);
		lenghtOfSeq1 = (int)strlen(seq1);
		MPI_Send(&lenghtOfSeq1,1,MPI_INT,1,tag,MPI_COMM_WORLD);
		MPI_Send(seq1, lenghtOfSeq1, MPI_CHAR, 1, tag, MPI_COMM_WORLD);
		MPI_Send(&amountOfSeq2, 1, MPI_INT, 1, tag, MPI_COMM_WORLD);

	} else {
		MPI_Recv(w,4, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD,&status);
		MPI_Recv(&lenghtOfSeq1,1, MPI_INT, 0, tag, MPI_COMM_WORLD,&status);
		seq1 = (char*)malloc(sizeof(char)*lenghtOfSeq1);
		MPI_Recv(seq1,lenghtOfSeq1, MPI_CHAR, 0, tag, MPI_COMM_WORLD,
				&status);
		MPI_Recv(&amountOfSeq2,1, MPI_INT, 0, tag, MPI_COMM_WORLD,&status);
	}
	for (i = 0; i < amountOfSeq2; i++) {
		if(my_rank == MASTER){
			if(i%2==0){
				lenghtOfSeq2 = (int)strlen(arrySeq2[i]);
				MPI_Send(&lenghtOfSeq2,1,MPI_INT,1,tag,MPI_COMM_WORLD);
				MPI_Send(arrySeq2[i], lenghtOfSeq2, MPI_CHAR, 1, tag, MPI_COMM_WORLD);
				MPI_Recv(&best,1, MPI_SCORE, 1, tag,MPI_COMM_WORLD, &status);
			}else {
				best = runForAllHyphen(seq1, arrySeq2[i],
						strlen(seq1), strlen(arrySeq2[i]), w);
			}
			fprintf(fp,"The %d sequence has the best score with offset %d and hyphen %d\n",
							i+1, best.bestN, best.bestK);
		}else{
			if (i%2 == 0) {
				MPI_Recv(&lenghtOfSeq2,1, MPI_INT, 0, tag, MPI_COMM_WORLD,&status);
				seq2 = (char*)malloc(sizeof(char)*lenghtOfSeq2);
				MPI_Recv(seq2,lenghtOfSeq2, MPI_CHAR, 0, tag, MPI_COMM_WORLD,
								&status);
				best = runForAllHyphen(seq1, seq2,lenghtOfSeq1, lenghtOfSeq2, w);
				MPI_Send(&best,1, MPI_SCORE, 0, tag ,MPI_COMM_WORLD);
				free(seq2);
			}
		}
	}


	if(my_rank == MASTER){
		free(w);
		free(seq1);
		free(arrySeq2);
		fclose(fp);
}

	/* shut down MPI */
	MPI_Finalize();
	return 0;
}
