cmake_minimum_required(VERSION 3.27)
project(manifest_destiny CUDA)

set(CMAKE_CUDA_STANDARD 17)

add_executable(manifest_destiny main.cu)

set_target_properties(manifest_destiny PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON)
