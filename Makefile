debug:
	mpicc -g -Wall -Wextra -o bin/manifest-debug src/load_data.c src/simulate.c src/populate.c src/main.c

release:
	mpicc -03 -Wall -Wextra -o bin/manifest src/load_data.c src/simulate.c src/populate.c src/main.c