//
// Created by matthew on 4/3/24.
//

#ifndef MANIFEST_DESTINY_LOAD_DATA_H
#define MANIFEST_DESTINY_LOAD_DATA_H


/**
 * Load data from a file into a buffer sequentially
 * @param filename The name of the file to load
 * @param cell_dim The number of features per cell
 * @param row_dim The number of rows in the data
 * @param col_dim The number of columns in the data
 * @param data A pointer to the buffer to load the data into
 * @return Zero on success, non-zero on failure
 */
int load_data_seq(const char *filename, int cell_dim, int row_dim, int col_dim, unsigned short **data);


/**
 * Save data from a buffer into a file sequentially
 * @param filename The name of the file to save to
 * @param cell_dim The number of features per cell
 * @param row_dim The number of rows in the data
 * @param col_dim The number of columns in the data
 * @param data The buffer containing the data to save
 * @return Zero on success, non-zero on failure
 */
int save_data_seq(const char *filename, int cell_dim, int row_dim, int col_dim, unsigned short *data);

/**
 * Load data from a file into a buffer using MPI parallel I/O
 * @param filename The name of the file to load
 * @param cell_dim The number of features per cell
 * @param row_dim The number of rows in the data
 * @param col_dim The number of columns in the data
 * @param rank The rank of the calling process
 * @param num_ranks The total number of processes
 * @param data A pointer to the buffer to load the data into
 * @return Zero on success, non-zero on failure
 */
int load_data_mpi(const char *filename, int cell_dim, int row_dim, int col_dim, int rank, int num_ranks, unsigned short **data);


/**
 * Save data from a buffer into a file using MPI parallel I/O
 * @param filename The name of the file to save to
 * @param cell_dim The number of features per cell
 * @param row_dim The number of rows in the data
 * @param col_dim The number of columns in the data
 * @param rank The rank of the calling process
 * @param num_ranks The total number of processes
 * @param data The buffer containing the data to save
 * @return Zero on success, non-zero on failure
 */
int save_data_mpi(const char *filename, int cell_dim, int row_dim, int col_dim, int rank, int num_ranks, unsigned short *data);


/**
 * Gather data from all ranks into a single buffer using MPI
 * @param cell_dim The number of features per cell
 * @param row_dim The number of rows in the data
 * @param col_dim The number of columns in the data
 * @param rank The rank of the calling process
 * @param num_ranks The total number of processes
 * @param rank_data The buffer containing the data for the calling rank
 * @param all_data The buffer to load the all of the gathered data into
 * @return Zero on success, non-zero on failure
 */
int gather_data_mpi(int cell_dim, int row_dim, int col_dim, int rank, int num_ranks, unsigned short *rank_data, unsigned short *all_data);

#endif //MANIFEST_DESTINY_LOAD_DATA_H
