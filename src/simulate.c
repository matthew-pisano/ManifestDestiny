//
// Created by matthew on 4/9/24.
//

#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>

#include "../include/simulate.h"
#include "../include/populate.h"

#define LAST_COL_TAG 0
#define FIRST_COL_TAG 1
#define NO_RANK -1


static inline void swap(unsigned short **a, unsigned short **b) {
    unsigned short *temp = *a;
    *a = *b;
    *b = temp;
}


void simulate_step(struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data, unsigned short *result_data) {

    for (int i=0; i<data_dims.cell_dim * data_dims.row_dim * data_dims.col_dim; i+=data_dims.cell_dim) {
        unsigned short new_pop = calc_cell_population(i, data_dims, ghost_cols, data);
        result_data[i+7] = new_pop;
    }
}


void simulate(int iterations, struct DataDims data_dims, int rank, int num_ranks, unsigned short **data) {

    unsigned short *result_data = calloc(data_dims.cell_dim * data_dims.row_dim * data_dims.col_dim, sizeof(unsigned short));

    unsigned short *first_col = calloc(data_dims.col_dim * data_dims.cell_dim, sizeof(unsigned short));
    unsigned short *last_col = calloc(data_dims.col_dim * data_dims.cell_dim, sizeof(unsigned short));

    unsigned short* west_ghost_col = calloc(data_dims.col_dim * data_dims.cell_dim, sizeof(unsigned short));
    unsigned short* east_ghost_col = calloc(data_dims.col_dim * data_dims.cell_dim, sizeof(unsigned short));

    for (int it_num = 0; it_num < iterations; it_num++) {

        // Determine the absolute ranks of the west and east ranks, relative to this process
        int west_rank = rank > 0 ? rank - 1 : NO_RANK;
        int east_rank = rank < num_ranks - 1 ? rank + 1 : NO_RANK;

        // Fill in the buffer rows with data from the current frame for sending to other ranks
        for(int i = 0; i < data_dims.col_dim * data_dims.cell_dim; i++) {
            first_col[i] = (*data)[i];
            last_col[i] = (*data)[data_dims.col_dim * (data_dims.row_dim - 1) * data_dims.cell_dim + i];
        }

        // Send ghost rows asynchronously to avoid deadlock, receive the ghost rows from other ranks asynchronously, and wait for them to finish
        MPI_Request send_request, recv_request;
        // Send the first and last rows to the neighboring ranks
        if (west_rank != NO_RANK)
            MPI_Isend(first_col, data_dims.col_dim * data_dims.cell_dim, MPI_UNSIGNED_SHORT, west_rank, FIRST_COL_TAG, MPI_COMM_WORLD, &send_request);
        if (east_rank != NO_RANK)
            MPI_Isend(last_col, data_dims.col_dim * data_dims.cell_dim, MPI_UNSIGNED_SHORT, east_rank, LAST_COL_TAG, MPI_COMM_WORLD, &send_request);

        // Receive the last row from the previous rank and wait for it to finish
        if (west_rank != NO_RANK) {
            MPI_Irecv(west_ghost_col, data_dims.col_dim * data_dims.cell_dim, MPI_UNSIGNED_SHORT, west_rank, LAST_COL_TAG, MPI_COMM_WORLD, &recv_request);
            MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
        }
        // Receive the first row from the next rank and wait for it to finish
        if (east_rank != NO_RANK) {
            MPI_Irecv(east_ghost_col, data_dims.col_dim * data_dims.cell_dim, MPI_UNSIGNED_SHORT, east_rank, FIRST_COL_TAG, MPI_COMM_WORLD, &recv_request);
            MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
        }

        struct GhostCols ghost_cols = {(west_rank != NO_RANK) ? west_ghost_col : NULL,
                                       (east_rank != NO_RANK) ? east_ghost_col : NULL};

        simulate_step(data_dims, ghost_cols, *data, result_data);
        swap(data, &result_data);
    }
    // Free the temporary buffers for the first and last rows
    free(first_col);
    free(last_col);
    free(west_ghost_col);
    free(east_ghost_col);
    free(result_data);
}