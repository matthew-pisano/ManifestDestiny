//
// Created by matthew on 4/3/24.
//

#ifndef MANIFEST_DESTINY_LOAD_DATA_H
#define MANIFEST_DESTINY_LOAD_DATA_H

#include "data_rep.h"


/**
 * Load the dimensions of the data from a file
 * @param filename The name of the file to load
 * @param data_dims A pointer to the struct to load the dimensions into
 */
void load_data_dims_mpi(const char *filename, struct DataDims* data_dims);

/**
 * Load data from a file into a buffer using MPI parallel I/O
 * @param filename The name of the file to load
 * @param col_offset The number of columns to skip when seeking to the start of the data
 * @param data_dims The dimensions (cell size, row size, and col size) of the data to load
 * @param data A pointer to the buffer to load the data into
 */
void load_data_mpi(const char *filename, int col_offset, struct DataDims data_dims, unsigned short **data);


/**
 * Save the dimensions of the data to the end of the file
 * @param filename The name of the file to save to
 * @param data_dims The dimensions of the data to save
 */
void save_data_dims_mpi(const char *filename, struct DataDims data_dims);


/**
 * Save data from a buffer into a file using MPI parallel I/O
 * @param filename The name of the file to save to
 * @param col_offset The number of columns to skip when seeking to the start of the data
 * @param data_dims The dimensions (cell size, row size, and col size) of the data to save
 * @param data The buffer containing the data to save
 */
void save_data_mpi(const char *filename, int col_offset, struct DataDims data_dims, unsigned short *data);

#endif //MANIFEST_DESTINY_LOAD_DATA_H
