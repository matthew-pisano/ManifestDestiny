//
// Created by matthew on 4/11/24.
//

#ifndef MANIFEST_DESTINY_POPULATE_H
#define MANIFEST_DESTINY_POPULATE_H

#include "data_rep.h"


struct Neighborhood {
    unsigned short count;
    unsigned short max;
    unsigned short avg;
    unsigned short min;
};


/**
 * Get the new population of a cell based on the values of its neighbors
 * @param target_cell The base (zero) index of the cell to update
 * @param data_dims The dimensions of the data
 * @param ghost_cols The ghost columns for the data
 * @param data The buffer containing the data
 * @return The new population of the cell
 */
unsigned short calc_cell_population(int target_cell, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data);

#endif //MANIFEST_DESTINY_POPULATE_H
