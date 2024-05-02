//
// Created by matthew on 4/9/24.
//

#include "../include/data_rep.h"

#include<stdio.h>
#include<cuda.h>
#include<cuda_runtime.h>


// Linear Congruential Generator (LCG) parameters
#define LCG_M 2147483647 // Modulus
#define LCG_A 1103515245 // Multiplier
#define LCG_C 12345      // Increment


/**
 * Use CUDA malloc managed to allocate memory on the device.
 * @param ptr The pointer to the memory to allocate.
 * @param size The size of the memory to allocate.
 */
extern "C" void __cudaMalloc(void** ptr, size_t size) {
    cudaMallocManaged(ptr, size);
}

/**
 * Use CUDA memcpy to copy memory between the host and device.
 * @param dst The destination pointer to copy to.
 * @param src The source pointer to copy from.
 * @param size The size of the memory to copy.
 */
extern "C" void __cudaMemcpy(void* dst, void* src, size_t size) {
    cudaMemcpy(dst, src, size, cudaMemcpyDefault);
}

/**
 * Use CUDA free to deallocate memory on the device.
 * @param ptr The pointer to the memory to deallocate.
 */
extern "C" void __cudaFree(void* ptr) {
    cudaFree(ptr);
}


/**
 * Struct to hold the count, max, average, and min values of the neighbors of a cell
 */
struct Neighborhood {
    /// The total sum of the values of the neighbors
    unsigned short count;
    /// The maximum value of the neighbors
    unsigned short max;
    /// The average value of the neighbors
    unsigned short avg;
    /// The minimum value of the neighbors
    unsigned short min;
};


/**
 * Count the values of the neighbors of a cell, within a given radius
 * @param target_index The index of the cell to count the neighbors of
 * @param radius The radius of the neighborhood to count
 * @param data_dims The dimensions of the data
 * @param ghost_cols The ghost columns for the data
 * @param data The buffer containing the data
 * @return A struct containing the count, max, average, and min values of the neighbors
 */
__device__ Neighborhood count_neighbor_values(int target_index, int radius, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data) {
    int cells_seen = 0;
    int count = 0;
    unsigned short max_value = 0;
    unsigned short min_value = 65535;
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
            unsigned short cell_value = 0;
            // If the neighbor is within the bounds of the world, get the value of the cell
            if (neighbor_index >= 0 && neighbor_index < world_len && neighbor_index != target_index && is_y_bounded) {
                cell_value = data[neighbor_index];
                cells_seen++;
            }
            // If the neighbor is outside the right bound of the world, get the value from the West ghost column
            else if (neighbor_index < 0 && is_y_bounded && ghost_cols.west != NULL) {
                cell_value = ghost_cols.west[pos_in_col + y_offset];
                cells_seen++;
            }
            // If the neighbor is outside the left bound of the world, get the value from the East ghost column
            else if (neighbor_index >= world_len && is_y_bounded && ghost_cols.east != NULL) {
                cell_value = ghost_cols.east[pos_in_col + y_offset];
                cells_seen++;
            }

            // Update the count, max, and min values
            count += cell_value;
            if (cell_value > max_value) max_value = cell_value;
            if (cell_value < min_value) min_value = cell_value;
        }
    }
    // Ensure the count is within the bounds of an unsigned short
    count = count > 65535 ? 65535 : count;

    unsigned short avg = (unsigned short) (count / cells_seen);

    struct Neighborhood neighborhood = {(unsigned short)count, max_value, avg, min_value};
    return neighborhood;
}


/**
 * Generate a random jitter value based on a seed and a maximum jitter value
 * @param seed The seed to generate the jitter from
 * @param max_jitter The maximum value of the jitter
 * @return The generated jitter value
 */
__device__ int generate_jitter(unsigned int seed, int max_jitter) {

    // Calculate the next seed value using the LCG parameters
    seed = (LCG_A * seed + LCG_C) % LCG_M;
    // Shift jitter by 100 to better match the distribution of the C rand() function
    return (seed-100) % max_jitter;
}

/**
 * Get the new population of a cell based on the values of its neighbors
 * @param target_cell The base (zero) index of the cell to update
 * @param data_dims The dimensions of the data
 * @param ghost_cols The ghost columns for the data
 * @param data The buffer containing the data
 * @return The new population of the cell
 */
__device__ unsigned short calc_cell_population(int target_cell, int iteration, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data) {

    // Get the attributes of the cell
    unsigned short elev = data[target_cell];
    unsigned short grad = data[target_cell+1];
    unsigned short water = data[target_cell+2];
    unsigned short temp = data[target_cell+3];
    unsigned short precip = data[target_cell+4];
    unsigned short resources = data[target_cell+5];
    unsigned short biome = data[target_cell+6];
    unsigned short pop = data[target_cell+7];

    // The current resolution scale of the world
    const unsigned short RES_SCALE = data_dims.col_dim / 623;
    // The square of the resolution scale, used for modifying growth factors
    const unsigned short RES_SCALE_SQ = RES_SCALE * RES_SCALE;
    // The minimum population of a cell
    const unsigned short MIN_POP = (2 * 9) / RES_SCALE_SQ;
    const int MAX_JITTER = 10000;

    // Generate a jitter value based on the target cell and the iteration
    int jitter = generate_jitter(target_cell * iteration, MAX_JITTER);
    float jitter_range = (jitter - MAX_JITTER / 2.0) / MAX_JITTER;

    // If the cell is water, too high, too steep, or too dry then return 0
    if (water > 0 || elev > 10000 || grad > 30) return 0;

    // If the cell is too populous then return 0
    if (pop > 40000) return pop;

    // Value of a cell on a scale of 0-100
    short cell_value = jitter_range * 30;
    unsigned short nearby_water = count_neighbor_values(target_cell+2, 2*RES_SCALE, data_dims, ghost_cols, data).max;

    // ~~~ CALCULATE CELL VALUE ~~~ //

    cell_value += nearby_water * 3;
    cell_value += resources;
    cell_value += 15 - (elev / 1500);
    cell_value += temp > 45 && temp < 70 ? 15 : 0;
    cell_value += precip > 30 ? 10 : precip < 10 ? -10 : 0;
    // If the biome is a swamp
    if (biome == 4) cell_value -= 10;
    // If the biome is a desert
    if (biome == 5) cell_value -= 15;

    // Clamp the cell value to 0-200
    if (cell_value < 0) cell_value = 0;
    if (cell_value > 200) cell_value = 200;

    struct Neighborhood nearby_population = count_neighbor_values(target_cell+7, 2*RES_SCALE, data_dims, ghost_cols, data);

    // ~~~ EXPLORATION PHASE ~~~ //

    // If the cell is uninhabited and the cell value is high enough, explore the cell
    float explore_chance = 0.002 + (iteration * iteration) / 8000000.0;

    if (pop == 0 && nearby_population.count > 0)
        return jitter < explore_chance * (cell_value / 100.0 + 3) * MAX_JITTER ? MIN_POP : 0;
    else if (pop == 0) return 0;

    // ~~~ SETTLEMENT PHASE ~~~ //

    // If the cell is explored with no nearby cities
    if (pop == MIN_POP && nearby_population.avg <= MIN_POP / 1.5) {
        // Settle new city
        if (jitter < 8) return 8 * MIN_POP * RES_SCALE_SQ;
    }
    // If the cell is settled with nearby cities, expand the city with this cell
    else if (pop == MIN_POP && jitter < cell_value / 7) {
        if (nearby_population.avg < 150) return MIN_POP + nearby_population.avg / 3 * RES_SCALE_SQ;
        else if (nearby_population.avg < 300) return MIN_POP + nearby_population.avg / 4 * RES_SCALE_SQ;
        else if (nearby_population.avg < 500) return MIN_POP + nearby_population.avg / 5 * RES_SCALE_SQ;
        else return MIN_POP + nearby_population.avg / 8 * RES_SCALE_SQ;
    }
    else if (pop == MIN_POP && jitter > MAX_JITTER - cell_value / 20)
        return 1.5 * MIN_POP * RES_SCALE_SQ;

    // Skip growth if the criteria for an already settled cell are not met
    if (pop == MIN_POP) return MIN_POP;

    cell_value -= grad * 2;
    if (cell_value < 0) cell_value = 0;

    // return cell_value * 500;

    // ~~~ GROWTH PHASE ~~~ //

    // Add a bonus to the cell value based on the average population of the neighbors, scaling with the cell value
    unsigned short neighbor_bonus = (cell_value / 100.0) * nearby_population.avg;
    // Grow the cell by a factor of its population, scaling with the cell value, and add the neighbor bonus
    float neighbor_growth_factor = 0.02;
    if (nearby_population.avg > 1500) neighbor_growth_factor = 0;
    else if (nearby_population.avg > 1000) neighbor_growth_factor = 0;
    else if (nearby_population.avg > 750) neighbor_growth_factor = 0.002;
    else if (nearby_population.avg > 500) neighbor_growth_factor = 0.008;

    // Scale the neighbor growth factor based on the iteration
    if (iteration < 100) neighbor_growth_factor *= 0.05;
    else if (iteration < 200) neighbor_growth_factor *= 0.2;
    else if (iteration < 400) neighbor_growth_factor *= 0.4;
    else if (iteration < 600) neighbor_growth_factor *= 0.6;

    // Give a bonus to a cell that is the largest in its neighborhood
    float city_center_bonus = 1;
    if (pop * (1+jitter_range*0.5) > nearby_population.avg) city_center_bonus = 2.1;

    // Scale the growth factor based on the population of the cell
    float growth_factor = 0.011 * city_center_bonus;
    if (pop > 3500) growth_factor *= 0.07;
    else if (pop > 2500) growth_factor *= 0.1;
    else if (pop > 1500) growth_factor *= 0.11;
    else if (pop > 1000) growth_factor *= 0.12;
    else if (pop > 750) growth_factor *= 0.14;
    else if (pop > 500) growth_factor *= 0.2;
    else if (pop > 300) growth_factor *= 0.3;

    // Scale the growth factor based on the iteration
    if (iteration < 100) growth_factor *= 0.05;
    else if (iteration < 200) growth_factor *= 0.2;
    else if (iteration < 400) growth_factor *= 0.4;
    else if (iteration < 600) growth_factor *= 0.6;

    // Adjust the growth factors to account for higher resolutions
    neighbor_growth_factor *= RES_SCALE;
    growth_factor /= RES_SCALE_SQ;

    // Cap the cell value at 77, minimizing the growth of the most valuable cells
    float cell_bonus = cell_value * 1.3;
    if (cell_bonus > 100) cell_bonus = 100;

    // Calculate the new population of the cell
    return (1 + (cell_bonus / 100.0 * growth_factor)) * pop + neighbor_growth_factor * neighbor_bonus;
}


__global__ void cuda_kernel(int iteration, struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data, unsigned short *result_data) {
    // Stride the kernel to allow for multiple threads to process the data
    for (size_t i = blockIdx.x * blockDim.x + threadIdx.x; i<data_dims.row_dim * data_dims.col_dim; i+=blockDim.x * gridDim.x) {

        size_t cell_index = i * data_dims.cell_dim;
        unsigned short new_pop = calc_cell_population(cell_index, iteration, data_dims, ghost_cols, data);
        // Update the population of the cell in the result data buffer
	    result_data[cell_index+7] = new_pop;
    }
}


extern "C" void launch_kernel(int iteration, int rank, int thread_count, struct DataDims data_dims, struct GhostCols ghost_cols,
        unsigned short *data, unsigned short *result_data) {

    // Get the number of cuda devices and set the current device to the rank modulo the number of devices
    cudaError_t cuda_error;
    int cuda_device_count;
    if( (cuda_error = cudaGetDeviceCount( &cuda_device_count)) != cudaSuccess ) {
        printf(" Unable to determine cuda device count, error is %d, count is %d\n", cuda_error, cuda_device_count );
        exit(cuda_error);
    }
    if( (cuda_error = cudaSetDevice( rank % cuda_device_count )) != cudaSuccess ) {
        printf(" Unable to have rank %d set to cuda device %d, error is %d \n", rank, (rank % cuda_device_count), cuda_error);
        exit(cuda_error);
    }

    // Determine how many blocks should be allocated to the kernel with a maximum of 65535
    size_t block_count = (data_dims.row_dim * data_dims.col_dim) / thread_count + 1;
    block_count = block_count > 65535 ? 65535 : block_count;
    // Launch the kernel with the determined block count and thread count
    cuda_kernel<<<block_count, (size_t) thread_count>>>(iteration, data_dims, ghost_cols, data, result_data);
    // Synchronize the device after each launch
    cudaDeviceSynchronize();
}
