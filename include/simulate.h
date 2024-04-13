//
// Created by matthew on 4/9/24.
//

#ifndef MANIFEST_DESTINY_SIMULATE_H
#define MANIFEST_DESTINY_SIMULATE_H

#include "data_rep.h"


/**
 * Simulate population changes in the data buffer
 * @param iterations The number of iterations to simulate
 * @param data_dims The dimensions of the data
 * @param rank The rank of the calling process
 * @param num_ranks The total number of processes
 * @param data A pointer to the buffer containing the data to simulate
 */
void simulate(int iterations, struct DataDims data_dims, int rank, int num_ranks, unsigned short **data);


/**
 * Simulate a single step of population changes in the data buffer
 * @param data_dims The dimensions of the data
 * @param ghost_cols The ghost columns for the data
 * @param data The buffer containing the data to simulate
 * @param result_data The buffer to store the result of the simulation
 */
void simulate_step(struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data,
                  unsigned short *result_data);

#endif //MANIFEST_DESTINY_SIMULATE_H
