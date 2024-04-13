//
// Created by matthew on 4/9/24.
//

#include "../include/cuda_kernel.cuh"


#include<stdio.h>
#include<cuda.h>
#include<cuda_runtime.h>

#include "../include/populate.h"


void cudaMallocManaged_wrapper(void** ptr, size_t size) {
    cudaMallocManaged(ptr, size);
}


void cudaFree_wrapper(void* ptr) {
    cudaFree(ptr);
}


__global__ void cuda_kernel(struct DataDims data_dims, struct GhostCols ghost_cols, unsigned short *data, unsigned short *result_data) {
    // int device;
    // cudaGetDevice(&device);

    for (size_t i = blockIdx.x * blockDim.x + threadIdx.x; i<data_dims.row_dim * data_dims.col_dim; i+=blockDim.x * gridDim.x) {
        size_t cell_index = i * data_dims.cell_dim;
        unsigned short new_pop = calc_cell_population(cell_index, data_dims, ghost_cols, data);
        result_data[i+7] = new_pop;
    }
}


void launch_kernel(int rank, int thread_count, struct DataDims data_dims, struct GhostCols ghost_cols,
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
    cuda_kernel<<<block_count, (size_t) thread_count>>>(data_dims, ghost_cols, data, result_data);
    // Synchronize the device after each launch
    cudaDeviceSynchronize();
}
