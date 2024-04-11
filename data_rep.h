//
// Created by matthew on 4/11/24.
//

#ifndef MANIFEST_DESTINY_DATA_REP_H
#define MANIFEST_DESTINY_DATA_REP_H

struct DataDims {
    int cell_dim;
    int row_dim;
    int col_dim;
};

struct Ghosts {
    unsigned short *west_ghost_col;
    unsigned short *east_ghost_col;
};

#endif //MANIFEST_DESTINY_DATA_REP_H
