debug:
	nvcc -g -o bin/manifest-debug load_data.c main.c

release:
	nvcc -03 -o bin/manifest load_data.c main.c