#include "mpi.h" 
#include "mpiFunction.h"
#include "general.h"

MPI_Datatype generateMPIDataTypeForStructScore() {

	MPI_Datatype MPI_SCORE;
	MPI_Datatype dataTypesScore[3] = { MPI_INT, MPI_INT, MPI_DOUBLE };
	MPI_Aint offsetsScore[3];
	int blockLengthsScore[3] = { 1, 1, 1 };
	offsetsScore[0] = offsetof(BestScore, bestN);
	offsetsScore[1] = offsetof(BestScore, bestK);
	offsetsScore[2] = offsetof(BestScore, score);
	MPI_Type_create_struct(3, blockLengthsScore, offsetsScore, dataTypesScore,
			&MPI_SCORE);
	MPI_Type_commit(&MPI_SCORE);
	return MPI_SCORE;
}
