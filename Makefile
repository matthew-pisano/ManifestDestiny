debug:
	mpicc -g -Wall -Wextra -o bin/manifest-debug load_data.c simulate.c main.c

release:
	mpicc -03 -Wall -Wextra -o bin/manifest load_data.c simulate.c main.c