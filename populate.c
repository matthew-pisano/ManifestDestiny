//
// Created by matthew on 4/11/24.
//

#include <stddef.h>
#include <stdlib.h>

#include "populate.h"


// Counts the values present in all eight cell neighbors.  The offset of the given target cell from a cell boundary is maintained in the calculations.
// If the target is offset by 7 from a cell boundary, all counts will be offset by 7 within the neighboring cells.
static inline int count_neighbor_values(int target_index, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data) {
    int count = 0;
    int col_len = data_dims.col_dim * data_dims.cell_dim;
    int pos_in_col = target_index % col_len;

    if (pos_in_col > data_dims.cell_dim && pos_in_col < col_len - data_dims.cell_dim) {
        count += data[target_index - data_dims.cell_dim] + data[target_index + data_dims.cell_dim];

        if (target_index >= col_len)
            count += data[target_index - col_len - data_dims.cell_dim] + data[target_index - col_len] + data[target_index - col_len + data_dims.cell_dim];
        else if (ghost_cols.west != NULL)
            count += ghost_cols.west[pos_in_col - data_dims.cell_dim] + ghost_cols.west[pos_in_col] + ghost_cols.west[pos_in_col + data_dims.cell_dim];

        if (target_index < col_len * (data_dims.row_dim - 1))
            count += data[target_index + col_len - data_dims.cell_dim] + data[target_index + col_len] + data[target_index + col_len + data_dims.cell_dim];
        else if (ghost_cols.east != NULL)
            count += ghost_cols.east[pos_in_col - data_dims.cell_dim] + ghost_cols.east[pos_in_col] + ghost_cols.east[pos_in_col + data_dims.cell_dim];
    }

    return count;
}


unsigned short waterMod = 5;
unsigned short slopeMod = 2;
unsigned short popMod = 10;
unsigned short tempMod_ext = 5;
unsigned short tempMod_mid = 3;
unsigned short rainMod = 5;
int calc_cell_population(int target_cell, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data) {

    //chance is the chance that the population of this cell increases
    short chance = 0;

    //get all neighbor information
    int nGradient = count_neighbor_values(target_cell+1, data_dims, ghost_cols, data);
    int nWater = count_neighbor_values(target_index+2, data_dims, ghost_cols, data);
    int nRain = count_neighbor_values(target_cell+4, data_dims, ghost_cols, data);
    int nResource = count_neighbor_values(target_cell+5, data_dims, ghost_cols, data);
    int nPop = count_neighbor_values(target_cell+7, data_dims, ghost_cols, data);
    
    //don't settle cities inside of oceans or lakes
    if(data[target_cell+2] != 0){
        return 0;
    }

    //do settle cities that are next to oceans or lakes, but are not islands.
    if (nWater > 0 && nWater < 8){
        chance += waterMod;
    }

    //don't settle cities on the side of cliffs
    if(neighborSlopes > maxSlope){
        chance -= slopeMod;
    }

    //do settle cities that are near other people.
    short pop = data[target_cell + 7];
    if(nPop > 1 || pop > 1){
        chance += popMod;
    }

    //don't settle in super hot or super cold places
    short temp = data[target_cell+3]
    if(temp > 10 || temp < 0){
        chance -= tempMod_ext;
    }
    else if (temp > 8 || temp < 2){
        chance -= tempMod_mid;
    }

    //do settle in places with resources or places near resources
    short resources = data[target_cell+5];
    chance += resources + nResource;

    //do not settle the Amazon or the Sahara
    short totalRain = nRain + data[target_cell+4];
    if( totalRain < 9 || totalRain > 27){
        chance -= rainMod;
    }
    
    //if the chance is high enough to spawn, set to pop = 10.
    if(pop == 0 && chance>0){
        return 10;
    }
    //otherwise, grow accordingly.
    if(chance > 10){
        return (chance-10) * pop;
    }
    return pop;
    
}
