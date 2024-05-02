//
// Created by matthew on 4/3/24.
//

#include "../include/load_data.h"
#include "../include/simulate.h"
#include "../include/data_rep.h"

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>


extern void __cudaFree(void* ptr);


int main(int argc, char **argv) {

    int rank, num_ranks;
    unsigned short *data;
    int iterations;
    int checkpoint_iterations;
    char *in_filename, *out_filename;

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 4) {
        printf("Usage: %s <input file> <output file> <iterations> [ckpt_iters]\n", argv[0]);
        return 1;
    }

    in_filename = argv[1];
    out_filename = argv[2];
    iterations = atoi(argv[3]);
    checkpoint_iterations = argc == 5 ? atoi(argv[4]) : iterations+1;

    struct DataDims data_dims;
    load_data_dims_mpi(in_filename, rank, num_ranks, &data_dims);

    load_data_mpi(in_filename, rank, num_ranks, data_dims, &data);

    simulate(out_filename, iterations, checkpoint_iterations, data_dims, rank, num_ranks, &data);

    save_data_mpi(out_filename, iterations, rank, num_ranks, data_dims, data);

    // Finalize the MPI environment
    MPI_Finalize();

    __cudaFree(data);

    return 0;
}
