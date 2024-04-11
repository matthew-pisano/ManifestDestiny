//
// Created by matthew on 4/9/24.
//

#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>

#include "simulate.h"
#include "populate.h"

#define LAST_COL_TAG 0
#define FIRST_COL_TAG 1
#define NO_RANK -1

static inline void swap(unsigned short **a, unsigned short **b) {
    unsigned short *temp = *a;
    *a = *b;
    *b = temp;
}

int simulate_step(int cell_dim, int row_dim, int col_dim, unsigned short **data, unsigned short **result_data, unsigned short* west_ghost_col, unsigned short* east_ghost_col) {

    for (int i=7; i<cell_dim * row_dim * col_dim; i+=cell_dim) {
        int new_pop = update_cell_population(i, cell_dim, row_dim, col_dim, *data, west_ghost_col, east_ghost_col);
        (*result_data)[i] = new_pop;
    }

    swap(data, result_data);

    return 0;
}

int simulate(int iterations, int cell_dim, int row_dim, int col_dim, int rank, int num_ranks, unsigned short **data) {

    unsigned short *result_data = calloc(cell_dim * row_dim * col_dim, sizeof(unsigned short));

    unsigned short *first_col = calloc(col_dim * cell_dim, sizeof(unsigned short));
    unsigned short *last_col = calloc(col_dim * cell_dim, sizeof(unsigned short));

    unsigned short* west_ghost_col = calloc(col_dim * cell_dim, sizeof(unsigned short));
    unsigned short* east_ghost_col = calloc(col_dim * cell_dim, sizeof(unsigned short));

    for (int it_num = 0; it_num < iterations; it_num++) {

        // Determine the absolute ranks of the west and east ranks, relative to this process
        int west_rank = rank > 0 ? rank - 1 : NO_RANK;
        int east_rank = rank < num_ranks - 1 ? rank + 1 : NO_RANK;

        // Fill in the buffer rows with data from the current frame for sending to other ranks
        for(int i = 0; i < col_dim * cell_dim; i++) {
            first_col[i] = (*data)[i];
            last_col[i] = (*data)[col_dim * (row_dim - 1) * cell_dim + i];
        }

        // Send ghost rows asynchronously to avoid deadlock, receive the ghost rows from other ranks asynchronously, and wait for them to finish
        MPI_Request send_request, recv_request;
        // Send the first and last rows to the neighboring ranks
        if (west_rank != NO_RANK)
            MPI_Isend(first_col, col_dim * cell_dim, MPI_UNSIGNED_SHORT, west_rank, FIRST_COL_TAG, MPI_COMM_WORLD, &send_request);
        if (east_rank != NO_RANK)
            MPI_Isend(last_col, col_dim * cell_dim, MPI_UNSIGNED_SHORT, east_rank, LAST_COL_TAG, MPI_COMM_WORLD, &send_request);

        // Receive the last row from the previous rank and wait for it to finish
        if (west_rank != NO_RANK) {
            MPI_Irecv(west_ghost_col, col_dim * cell_dim, MPI_UNSIGNED_SHORT, west_rank, LAST_COL_TAG, MPI_COMM_WORLD, &recv_request);
            MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
        }
        // Receive the first row from the next rank and wait for it to finish
        if (east_rank != NO_RANK) {
            MPI_Irecv(east_ghost_col, col_dim * cell_dim, MPI_UNSIGNED_SHORT, east_rank, FIRST_COL_TAG, MPI_COMM_WORLD, &recv_request);
            MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
        }

        simulate_step(cell_dim, row_dim, col_dim, data, &result_data,
                      (west_rank != NO_RANK) ? west_ghost_col : NULL,
                      (east_rank != NO_RANK) ? east_ghost_col : NULL);
    }
    // Free the temporary buffers for the first and last rows
    free(first_col);
    free(last_col);
    free(west_ghost_col);
    free(east_ghost_col);
    free(result_data);

  return 0;
}