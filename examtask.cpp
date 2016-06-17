/*
 *  Найти в массиве количество пар соседних элементов у которых одинаковые
 *  абсолютные значения, но которые отличаются по знаку.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define ARRAY_SIZE 20

int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);
    int rank;
    int processesCount;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processesCount);
    srand(time(NULL));

    int* array = new int[ARRAY_SIZE];
    if (rank == 0){
        for (int i = 0; i < ARRAY_SIZE; i++){
            int power = -1;
            int A = rand() % 2;
            for (int i = 0; i < A; i++){
                power *= -1;
            }
            array[i] = power * rand() % 5;
            printf("%d ", array[i]);
        }
        printf("\n");
    }

    int partialArraySize = ARRAY_SIZE / processesCount;
    int rest = ARRAY_SIZE % processesCount;

    int* sendCounts = new int[processesCount - 1];
    int* displacements = new int[processesCount - 1];

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

    MPI_Scatterv(
        array,
        sendCounts,
        displacements,
        MPI_INT,
        partialArray,
        sendCounts[rank],
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    int partialResult = 0;
    int totalResult = 0;

    int receive = 0;
    if (rank != 0){
        MPI_Send(&partialArray[0],
            1,
            MPI_INT,
            rank - 1,
            0,
            MPI_COMM_WORLD
        );
    }
    if (rank != processesCount - 1){
        MPI_Recv(
            &receive,
            1,
            MPI_INT,
            rank + 1,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );
    }

    for (int i = 0; i < sendCounts[rank] - 1; i++){
        if (partialArray[i] == -partialArray[i + 1] &&
                partialArray[i] != 0
                && partialArray[i + 1] != 0) {
            partialResult++;
        }
    }

    if (rank != processesCount - 1){
        if (receive == -partialArray[sendCounts[rank] - 1] &&
                receive != 0 &&
                partialArray[sendCounts[rank] - 1] != 0) {
            partialResult++;
        }
    }

    MPI_Reduce(&partialResult,
        &totalResult,
        1,
        MPI_INT,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );

    if (rank == 0){
        printf("%d\n", totalResult);
    }

    MPI_Finalize();
    return 0;
}
