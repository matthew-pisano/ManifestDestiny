//
// Created by matthew on 4/9/24.
//

#include "../include/simulate.h"

#include "../include/load_data.h"

#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include <string.h>


extern void __cudaMalloc(void** ptr, size_t size);
extern void __cudaMemcpy(void* dst, void* src, size_t size);
extern void __cudaFree(void* ptr);
extern void launch_kernel(int iteration, int rank, int thread_count, struct DataDims data_dims,
        struct GhostCols ghost_cols, unsigned short *data, unsigned short *result_data);


#define LAST_COL_TAG 0
#define FIRST_COL_TAG 1
#define NO_RANK -1


/**
 * Swap the pointers of two buffers
 * @param a The first buffer
 * @param b The second buffer
 */
static inline void swap(unsigned short **a, unsigned short **b) {
    unsigned short *temp = *a;
    *a = *b;
    *b = temp;
}


void simulate(const char *filename, int iterations, int checkpoint_iterations, struct DataDims data_dims, int rank, int num_ranks, unsigned short **data) {

    // Create a new buffer to store the result of the simulation
    // Copy the data from the original buffer to the result buffer before beginning the simulation
    unsigned short *result_data = NULL;
    __cudaMalloc((void **) &result_data, data_dims.cell_dim * data_dims.row_dim * data_dims.col_dim * sizeof(unsigned short));
    __cudaMemcpy(result_data, *data, data_dims.cell_dim * data_dims.row_dim * data_dims.col_dim * sizeof(unsigned short));

    // Allocate temporary buffers for the first and last rows for sharing with neighboring ranks
    unsigned short *first_col = calloc(data_dims.col_dim * data_dims.cell_dim, sizeof(unsigned short));
    unsigned short *last_col = calloc(data_dims.col_dim * data_dims.cell_dim, sizeof(unsigned short));

    // Allocate temporary buffers for the ghost columns to get data from neighboring ranks
    unsigned short* west_ghost_col = NULL;
    unsigned short* east_ghost_col = NULL;
    __cudaMalloc((void **) &west_ghost_col, data_dims.col_dim * data_dims.cell_dim * sizeof(unsigned short));
    __cudaMalloc((void **) &east_ghost_col, data_dims.col_dim * data_dims.cell_dim * sizeof(unsigned short));

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

        launch_kernel(it_num, rank, 256, data_dims, ghost_cols, *data, result_data);
        swap(data, &result_data);

        // Save the data to a checkpoint file if this is a checkpoint iteration
        if (it_num > 0 && it_num < iterations-1 && it_num % checkpoint_iterations == 0)
            save_data_mpi(filename, it_num, rank, num_ranks, data_dims, *data);
    }

    // Free the temporary buffers for the first and last rows
    free(first_col);
    free(last_col);
    // Free the temporary buffers for the ghost columns
    __cudaFree(west_ghost_col);
    __cudaFree(east_ghost_col);
    // Free the result buffer
    __cudaFree(result_data);
}
