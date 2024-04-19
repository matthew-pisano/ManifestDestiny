error:
	@echo "Please choose one of the following target: debug, release"
	@exit 2

debug: src/load_data.c src/simulate.c src/main.c src/cuda_kernel.cu
	mpicc -g -Wall -Wextra -c -o build/manifest-debug-load-data.o src/load_data.c
	mpicc -g -Wall -Wextra -c -o build/manifest-debug-simulate.o src/simulate.c
	mpicc -g -Wall -Wextra -c -o build/manifest-debug-main.o src/main.c
	nvcc -g -G -arch=sm_70 -c -o build/manifest-debug-cuda.o src/cuda_kernel.cu --compiler-options -Wall,-Wextra
	mpicc -g -o bin/manifest-debug build/manifest-debug-load-data.o build/manifest-debug-simulate.o build/manifest-debug-main.o build/manifest-debug-cuda.o -L/usr/local/cuda-11.2/lib64/ -lcudadevrt -lcudart -lstdc++

release: src/load_data.c src/simulate.c src/main.c src/cuda_kernel.cu
	mpicc -O3 -Wall -Wextra -c -o build/manifest-load-data.o src/load_data.c
	mpicc -O3 -Wall -Wextra -c -o build/manifest-simulate.o src/simulate.c
	mpicc -O3 -Wall -Wextra -c -o build/manifest-main.o src/main.c
	nvcc -Xptxas -O3,-v -arch=sm_70 -c -o build/manifest-cuda.o src/cuda_kernel.cu --compiler-options -Wall,-Wextra
	mpicc -O3 -o bin/manifest build/manifest-load-data.o build/manifest-simulate.o build/manifest-main.o build/manifest-cuda.o -L/usr/local/cuda-11.2/lib64/ -lcudadevrt -lcudart -lstdc++
