#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "load_data.h"
#include "simulate.h"


int load_test_seq() {

    unsigned short *data;
    int cell_dim = 7;
    int row_dim = 623;
    int col_dim = 994;

    const char *in_filename = "scripts/out/initial_state.npy";
    const char *out_filename = "scripts/out/final_state.npy";
    int err;
    if ((err = load_data_seq(in_filename, cell_dim, row_dim, col_dim, &data))) {
        printf("Error: Could not load data from file %s\n", in_filename);
        return err;
    }
    else {
        if ((err = save_data_seq(out_filename, cell_dim, row_dim, col_dim, data))) {
            printf("Error: Could not save data to file %s\n", out_filename);
            return err;
        }
    }

    return 0;
}


int main(int argc, char **argv) {

    int rank, num_ranks;
    unsigned short *data;
    int cell_dim = 7;
    int row_dim = 623;
    int col_dim = 994;
    char *in_filename, *out_filename;

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    in_filename = argv[1];
    out_filename = argv[2];
    int err;
    if ((err = load_data_mpi(in_filename, cell_dim, row_dim, col_dim, rank, num_ranks, &data))) {
        printf("Error: Could not load data from file %s\n", in_filename);
        return err;
    }

    simulate(1, cell_dim, row_dim, col_dim, rank, num_ranks, data);

    if ((err = save_data_mpi(out_filename, cell_dim, row_dim, col_dim, rank, num_ranks, data))) {
        printf("Error: Could not save data to file %s\n", out_filename);
        return err;
    }

    // Finalize the MPI environment
    MPI_Finalize();

    free(data);

    return 0;
}
