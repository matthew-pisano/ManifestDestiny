//
// Created by matthew on 4/11/24.
//

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#include "../include/populate.h"


/**
 * Count the values of the neighbors of a cell, within a given radius
 * @param target_index The index of the cell to count the neighbors of
 * @param radius The radius of the neighborhood to count
 * @param data_dims The dimensions of the data
 * @param ghost_cols The ghost columns for the data
 * @param data The buffer containing the data
 * @return The count of the values of the neighbors
 */
static inline unsigned short count_neighbor_values(int target_index, int radius, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data) {
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

    // Ensure the count is within the bounds of an unsigned short
    return count > 65535 ? 65535 : (unsigned short) count;
}



unsigned short calc_cell_population(int target_cell, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data) {

    unsigned short elev = data[target_cell];
    unsigned short grad = data[target_cell+1];
    unsigned short water = data[target_cell+2];
    unsigned short temp = data[target_cell+3];
    unsigned short precip = data[target_cell+4];
    unsigned short resources = data[target_cell+5];
    unsigned short biome = data[target_cell+6];
    unsigned short pop = data[target_cell+7];

    int jitter = rand() % 1000;

    // If the cell is water, too high, too steep, or too dry then return 0
    if (water > 0 || elev > 10000 || grad > 30 || precip < 10) return 0;

    // If the cell is too populous then return 0
    if (pop > 40000) return pop;

    // Value of a cell on a scale of 0-100
    short cell_value = 0;
    unsigned short nearby_water = count_neighbor_values(target_cell+2, 2, data_dims, ghost_cols, data);

    cell_value += nearby_water * 2 > 40 ? 40 : nearby_water * 2;
    cell_value += resources;
    cell_value += 10 - (elev / 1000);
    cell_value += 10 - (grad / 3);
    cell_value += temp > 45 && temp < 65 ? 10 : 0;
    cell_value += precip > 30 ? 10 : 0;
    // If the biome is a swamp
    if (biome == 4) cell_value -= 10;
    // If the biome is a desert
    if (biome == 5) cell_value -= 15;

    // Clamp the cell value to 0-100
    if (cell_value < 0) cell_value = 0;
    if (cell_value > 100) cell_value = 100;

    // return cell_value * 200;

    unsigned short nearby_population = count_neighbor_values(target_cell+7, 2, data_dims, ghost_cols, data);

    // ~~~ EXPLORATION PHASE ~~~ //

    // If the cell is uninhabited and the cell value is high enough, explore the cell
    if (pop == 0) {
        if (
            (nearby_population > 0 && cell_value > 50) ||
            (nearby_population > 40 && cell_value > 40) ||
            (nearby_population > 70 && cell_value > 30) ||
            (nearby_population > 100 && cell_value > 20)
        // Randomly fail to explore the cell
        ) return jitter < 900 ? 10 : 0;

        return nearby_population > 0 && jitter < 50 ? 10 : 0;
    }

    // ~~~ SETTLEMENT PHASE ~~~ //

    // Get the average population of the neighbors
    unsigned short avg_neighbor_pop = (nearby_population) / 24;

    // If the cell is explored with no nearby cities
    if (pop == 10 && avg_neighbor_pop <= 10) {
        // Settle new city
        if (jitter < cell_value / 30) return 21*avg_neighbor_pop;
    }
    // If the cell is settled with nearby cities
    else if (pop == 10 && jitter < cell_value * 0.2) {
        if (avg_neighbor_pop < 50) return 10 + avg_neighbor_pop;
        else if (avg_neighbor_pop < 100 && jitter < cell_value * 0.16) return 10 + avg_neighbor_pop;
        else if (avg_neighbor_pop < 200 && jitter < cell_value * 0.12) return 10 + avg_neighbor_pop;
        else if (jitter < cell_value * 0.09) return 10 + avg_neighbor_pop;
    }
    // Skip growth if the criteria for an already settled cell are not met
    if (pop == 10) return 10;

    // ~~~ GROWTH PHASE ~~~ //

    // Add a bonus to the cell value based on the average population of the neighbors, scaling with the cell value
    unsigned short neighbor_bonus = avg_neighbor_pop > pop ? 0.1 * cell_value / 100.0 * avg_neighbor_pop : 0;
    // Grow the cell by a factor of its population, scaling with the cell value, and add the neighbor bonus
    float growth_factor = 1;
    if (pop + avg_neighbor_pop > 500) growth_factor = 0.2;
    else if (pop + avg_neighbor_pop > 1000) growth_factor = 0.05;
    else if (pop + avg_neighbor_pop > 1500) growth_factor = 0.15;
    return (1 + (cell_value / 100.0 * 0.02 * growth_factor)) * pop + growth_factor * growth_factor * neighbor_bonus;
}
