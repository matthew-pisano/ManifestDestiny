debug:
	mpicc -g -o bin/manifest-debug load_data.c main.c

release:
	mpicc -03 -o bin/manifest load_data.c main.c