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


int calc_cell_population(int target_cell, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data) {
    return data[target_cell + 7];
}

//TODO: Fix compiler errors with below code

//int slopeThresh = 20;
//float basePercent = 50;
//int *slopePenalty;
//
//int *tempMin;
//int *tempMax;
//int *tempPenalty;
//
//int *rainMax;
//int *rainMin;
//int *rainPenalty;
//
//float waterBuff;

/* short slope,
short temp,
short rain,
short elevation,
short water,
short resources,
short biome */


//preData is the ghost row before
//postData is the ghost row after
//int rateSpot(char* d_data, char* preData, char* postData, int cell_dim, size_t x0, size_t x, size_t x2, size_t y0, size_t y1, size_t y2) {
//    int cityChance = basePercent;
//
//    short slope = d_data[x];
//    short temp = d_data[x+1];
//    short rainfall = d_data[x+2];
//    short elevation = d_data[x+3];
//    short water = d_data[x+4];
//    short[] neighborWater = [d_data[y0+x0+4], d_data[y0+x+4], d_data[y0+x2+4], d_data[y1+x0+4], d_data[y1+x2+4], d_data[y2+x0+4], d_data[y2+x+4], d_data[y2+x2+4],]
//    short resource = d_data[x+5];
//    short biome = d_data[x+6];
//
//    //being underwater is bad
//    if(water != 0){
//        return 0;
//    }
//
//    //count the number of neighbors that are underwater. Coastline, good, peninsula fine, island questionable
//    short neighborsUnderwater = 0;
//    for(int i = 0; i<neighborWater.size(); i++){
//        if(neighborWater[i] != 0){
//            neighborsUnderwater += 1;
//        }
//    }
//
//    //add buffs accordingly
//    if (neighborsUnderwater > 5){
//        cityChance += *peninsulaBuff;
//    }
//    elif(neighborsUnderwater > 2){
//        cityChance += *coastBuff;
//    }
//    elif(neighborsUnderwater > 0){
//        cityChance += *waterBuff;
//    }
//
//    //add up the elevation differences between this cell and its neighbors, if greater than slopeThresh, sub
//    float slopeSum = 0;
//    for(int i = 0; i<slope.size(); i++){
//        slopeSum += abs(slope[i] - elevation);
//    }
//    if(slopeSum > slopeThresh){
//        cityChance -= (slopeSum * (*slopePenalty));
//    }
//
//    //very hot or very cold is bad
//    if(temp > *tempMax || temp < *tempMin){
//        cityChance -= *tempPenalty;
//    }
//
//    //rainforest and desert are bad
//    if(rain > *rainMax || rain < *rainMin){
//        cityChance -= *rainPenalty;
//    }
//
//    //resources good
//    for(int r = 0; r<resources.size(); r++){
//        cityChance += resourceMap(resources[r]);
//    }
//
//
//    return cityChance;
//
//}
