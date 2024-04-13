//
// Created by matthew on 4/9/24.
//

#ifndef MANIFEST_DESTINY_CUDA_UTILS_CUH
#define MANIFEST_DESTINY_CUDA_UTILS_CUH

void launch_kernel(int rank, int thread_count, struct DataDims data_dims, struct GhostCols ghost_cols,
        unsigned short *data, unsigned short *result_data);

void cudaMallocManaged_wrapper(void** ptr, size_t size);
void cudaFree_wrapper(void* ptr);

#endif //MANIFEST_DESTINY_CUDA_UTILS_CUH
