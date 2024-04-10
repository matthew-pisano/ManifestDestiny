//
// Created by matthew on 4/9/24.
//

#include <stdlib.h>
#include <mpi.h>

#define LAST_COL_TAG 0
#define FIRST_COL_TAG 1
#define NO_RANK -1

static inline void swap(unsigned short **a, unsigned short **b) {
    unsigned short *temp = *a;
    *a = *b;
    *b = temp;
}

// Counts the values present in all eight cell neighbors.  The offset of the given target cell from a cell boundary is maintained in the calculations.
// If the target is offset by 7 from a cell boundary, all counts will be offset by 7 within the neighboring cells.
static inline int count_neighbors(int target_cell, int cell_dim, int row_dim, int col_dim, unsigned short *data, unsigned short *result_data, unsigned short* west_ghost_col, unsigned short* east_ghost_col) {
    int count = 0;
    int col_len = col_dim * cell_dim;
    int pos_in_col = target_cell % col_len;


    if (pos_in_col > cell_dim && pos_in_col < col_len - cell_dim) {
        count += data[target_cell - cell_dim] + data[target_cell + cell_dim];

        if (target_cell >= col_len)
            count += data[target_cell - col_len - cell_dim] + data[target_cell - col_len] + data[target_cell - col_len + cell_dim];
        else if (west_ghost_col != nullptr)
            count += west_ghost_col[pos_in_col - cell_dim] + west_ghost_col[pos_in_col] + west_ghost_col[pos_in_col + cell_dim];

        if (target_cell < col_len * (row_dim - 1))
            count += data[target_cell + col_len - cell_dim] + data[target_cell + col_len] + data[target_cell + col_len + cell_dim];
        else if (east_ghost_col != nullptr)
            count += east_ghost_col[pos_in_col - cell_dim] + east_ghost_col[pos_in_col] + east_ghost_col[pos_in_col + cell_dim];
    }

    return count;
}

int simulate_step(int cell_dim, int row_dim, int col_dim, unsigned short *data, unsigned short *result_data, unsigned short* west_ghost_col, unsigned short* east_ghost_col) {
    int col_len = col_dim * cell_dim;

    // TODO: Implement the simulation logic here
    for (int i=7; i<row_dim * col_dim; i+=cell_dim) {
        // Test to shift all cities one cell to the left
        if (i >= col_len * (row_dim - 1))
            result_data[i] = data[i];
        else
            result_data[i] = data[i+col_len];
    }

    swap(&data, &result_data);

    return 0;
}

int simulate(int iterations, int cell_dim, int row_dim, int col_dim, int rank, int num_ranks, unsigned short *data) {

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
            first_col[i] = data[i];
            last_col[i] = data[col_dim * (row_dim - 1) * cell_dim + i];
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

        simulate_step(cell_dim, row_dim, col_dim, data, result_data,
                      (west_rank != NO_RANK) ? west_ghost_col : nullptr,
                      (east_rank != NO_RANK) ? east_ghost_col : nullptr);
    }
    // Free the temporary buffers for the first and last rows
    free(first_col);
    free(last_col);
    free(west_ghost_col);
    free(east_ghost_col);
    free(result_data);

  return 0;
}