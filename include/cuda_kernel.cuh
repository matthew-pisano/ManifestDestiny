//
// Created by matthew on 4/9/24.
//

#ifndef MANIFEST_DESTINY_CUDA_KERNEL_CUH
#define MANIFEST_DESTINY_CUDA_KERNEL_CUH

void launch_kernel(int rank, int thread_count, struct DataDims data_dims, struct GhostCols ghost_cols,
        unsigned short *data, unsigned short *result_data);


/**
 * Use CUDA malloc to allocate memory on the device.
 * @param ptr The pointer to the memory to allocate.
 * @param size The size of the memory to allocate.
 */
void cudaMallocManaged_wrapper(void** ptr, size_t size);


/**
 * Use CUDA free to deallocate memory on the device.
 * @param ptr The pointer to the memory to deallocate.
 */
void cudaFree_wrapper(void* ptr);

#endif //MANIFEST_DESTINY_CUDA_KERNEL_CUH
