//
// Created by matthew on 4/11/24.
//

#ifndef MANIFEST_DESTINY_POPULATE_H
#define MANIFEST_DESTINY_POPULATE_H

int update_cell_population(int target_cell, int cell_dim, int row_dim, int col_dim,  unsigned short *data,
                           unsigned short* west_ghost_col, unsigned short* east_ghost_col);

#endif //MANIFEST_DESTINY_POPULATE_H
