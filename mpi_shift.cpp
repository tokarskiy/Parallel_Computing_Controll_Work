
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 10

int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);
    int rank;
    int processesCount;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processesCount);

    int* array = new int[ARRAY_SIZE];
    int* result = new int[ARRAY_SIZE];
    if (rank == 0){
        for (int i = 0; i < ARRAY_SIZE; i++){
            array[i] = rand() % 100;
            printf("%d ", array[i]);
        }
        printf("\n");
    }

    int partialArraySize = ARRAY_SIZE / processesCount;
    int rest = ARRAY_SIZE % processesCount;

    int* sendCounts = new int[processesCount];
    int* displacements = new int[processesCount];

    int sendIndex = 0;
    for (int i = 0; i < rest; i++){
        sendCounts[i] = partialArraySize + 1;
        displacements[i] = sendIndex;
        sendIndex += sendCounts[i];
    }

    for (int i = rest; i < processesCount; i++){
        sendCounts[i] = partialArraySize;
        displacements[i] = sendIndex;
        sendIndex += sendCounts[i];
    }

    int* partialArray = new int[sendCounts[rank]];

    MPI_Scatterv(array,
        sendCounts,
        displacements,
        MPI_INT,
        partialArray,
        sendCounts[rank],
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    sendCounts[0]--;

    sendIndex = 0;
    for (int i = 0; i < processesCount; i++){
        displacements[i] = sendIndex;
        sendIndex += sendCounts[i];
    }

    int firstElement = partialArray[0];

    if (rank == 0){
        int* temp = partialArray;
        partialArray = new int[sendCounts[rank] - 1];
        for (int i = 0; i < sendCounts[rank]; i++){
            partialArray[i] = temp[i + 1];
        }
    }

    MPI_Gatherv(
        partialArray,
        sendCounts[rank],
        MPI_INT,
        result,
        sendCounts,
        displacements,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    if (rank == 0){
        result[ARRAY_SIZE - 1] = firstElement;

        for (int i = 0; i < ARRAY_SIZE; i++){
            printf("%d ", result[i]);
        }
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}
