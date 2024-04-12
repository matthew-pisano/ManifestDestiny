#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "load_data.h"
#include "simulate.h"
#include "data_rep.h"


int main(int argc, char **argv) {

    int rank, num_ranks;
    unsigned short *data;
    int iterations;
    char *in_filename, *out_filename;

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc != 4) {
        printf("Usage: %s <input file> <output file> <iterations>\n", argv[0]);
        return 1;
    }

    in_filename = argv[1];
    out_filename = argv[2];
    iterations = atoi(argv[3]);

    struct DataDims data_dims;
    int err;
    if ((err = load_data_dims_mpi(in_filename, &data_dims))) {
        printf("Error: Could not load data dimensions from file %s\n", in_filename);
        return err;
    }

    printf("Loaded data dimensions: %d x %d x %d\n", data_dims.row_dim, data_dims.col_dim, data_dims.cell_dim);

    int cols_per_rank = data_dims.row_dim / num_ranks;
    int read_cols = cols_per_rank;
    if (rank == num_ranks - 1)
        read_cols += data_dims.row_dim % num_ranks;

    // Reset the number of columns to read to the number of columns for this rank
    data_dims.row_dim = read_cols;

    if ((err = load_data_mpi(in_filename, cols_per_rank*rank, data_dims, &data))) {
        printf("Error: Could not load data from file %s\n", in_filename);
        return err;
    }

    simulate(iterations, data_dims, rank, num_ranks, &data);

    if ((err = save_data_mpi(out_filename, cols_per_rank*rank, data_dims, data))) {
        printf("Error: Could not save data to file %s\n", out_filename);
        return err;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0 && (err = save_data_dims_mpi(out_filename, data_dims))) {
        printf("Error: Could not save data dimensions to file %s\n", out_filename);
        return err;
    }

    // Finalize the MPI environment
    MPI_Finalize();

    free(data);

    return 0;
}
