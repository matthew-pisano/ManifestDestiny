//
// Created by matthew on 4/11/24.
//

#ifndef MANIFEST_DESTINY_DATA_REP_H
#define MANIFEST_DESTINY_DATA_REP_H


/**
 * The dimensions of the data
 */
struct DataDims {
    /// The number of elements in one cell
    int cell_dim;
    /// The number of cells in one row
    int row_dim;
    /// The number of cells in one row of the global input data
    int global_row_dim;
    /// The number of cells in one column
    int col_dim;
};

/// The ghost columns for the data to view neighboring columns
struct GhostCols {
    unsigned short *west;
    unsigned short *east;
};

#endif //MANIFEST_DESTINY_DATA_REP_H
