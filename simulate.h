//
// Created by matthew on 4/9/24.
//

#ifndef MANIFEST_DESTINY_SIMULATE_H
#define MANIFEST_DESTINY_SIMULATE_H


int simulate(int iterations, int cell_dim, int row_dim, int col_dim, int rank, int num_ranks, unsigned short *data);


int simulate_step(int cell_dim, int row_dim, int col_dim, unsigned short *data, unsigned short *result_data, unsigned short* west_ghost_col, unsigned short* east_ghost_col);

#endif //MANIFEST_DESTINY_SIMULATE_H
