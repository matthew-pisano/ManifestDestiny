all: debug release-serial debug

debug-serial: src/load_data.c src/simulate.c src/populate.c src/main.c
	mpicc -g -Wall -Wextra -o bin/manifest-serial-debug src/load_data.c src/simulate.c src/populate.c src/main.c -lm

release-serial: src/load_data.c src/simulate.c src/populate.c src/main.c
	mpicc -O3 -Wall -Wextra -o bin/manifest-serial src/load_data.c src/simulate.c src/populate.c src/main.c -lm

debug: src/load_data.c src/simulate.c src/populate.c src/main.c src/cuda_kernel.cu
	mpicc -g -Wall -Wextra -c -o build/manifest-debug-load-data.o src/load_data.c
	mpicc -g -Wall -Wextra -c -o build/manifest-debug-simulate.o src/simulate.c
	mpicc -g -Wall -Wextra -c -o build/manifest-debug-main.o src/main.c
	mpicc -g -Wall -Wextra -c -o build/manifest-debug-populate.o src/populate.c
	nvcc -g -G -arch=sm_70 -c -o build/manifest-debug-cuda.o src/cuda_kernel.cu
	mpicc -g -o bin/manifest-debug build/manifest-debug-load-data.o build/manifest-debug-populate.o build/manifest-debug-simulate.o build/manifest-debug-main.o build/manifest-debug-cuda.o
