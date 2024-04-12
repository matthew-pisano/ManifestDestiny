//
// Created by matthew on 4/11/24.
//

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include "populate.h"


/**
 * Count the values of the neighbors of a cell, within a given radius
 * @param target_index The index of the cell to count the neighbors of
 * @param radius The radius of the neighborhood to count
 * @param data_dims The dimensions of the data
 * @param ghost_cols The ghost columns for the data
 * @param data The buffer containing the data
 * @return The count of the values of the neighbors
 */
static inline int count_neighbor_values(int target_index, int radius, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data) {
    int count = 0;
    int col_len = data_dims.col_dim * data_dims.cell_dim;
    int world_len = col_len * data_dims.row_dim;
    // The offset of the given target cell from a cell boundary is maintained in the calculations.
    // If the target is offset by 7 from a cell boundary, all counts will be offset by 7 within the neighboring cells.
    int pos_in_col = target_index % col_len;

    // Count for all cells within the radius of the target cell
    for (int x_offset=-radius*col_len; x_offset<=radius*col_len; x_offset+=col_len) {
        for (int y_offset=-radius*data_dims.cell_dim; y_offset<=radius*data_dims.cell_dim; y_offset+=data_dims.cell_dim) {

            int neighbor_index = target_index + x_offset + y_offset;
            bool is_y_bounded = pos_in_col + y_offset >= 0 && pos_in_col + y_offset < col_len;

            if (neighbor_index >= 0 && neighbor_index < world_len && neighbor_index != target_index && is_y_bounded)
                count += data[neighbor_index];
            else if (neighbor_index < 0 && is_y_bounded && ghost_cols.west != NULL)
                count += ghost_cols.west[pos_in_col + y_offset];
            else if (neighbor_index >= world_len && is_y_bounded && ghost_cols.east != NULL)
                count += ghost_cols.east[pos_in_col + y_offset];
        }
    }

    return count;
}


int calc_cell_population(int target_cell, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data) {
    return data[target_cell + 7];
    //return data[target_cell + 7] + count_neighbor_values(target_cell+7, 2, data_dims, ghost_cols, data);
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
