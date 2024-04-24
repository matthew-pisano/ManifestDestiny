error:
	@echo "Please choose one of the following target: debug-serial, release-serial"
	@exit 2

debug-serial: src/load_data.c src/simulate.c src/populate.c src/main.c
	mkdir -p out build bin
	mpicc -g -Wall -Wextra -o bin/manifest-serial-debug src/load_data.c src/simulate.c src/populate.c src/main.c -lm

release-serial: src/load_data.c src/simulate.c src/populate.c src/main.c
	mkdir -p out build bin
	mpicc -O3 -Wall -Wextra -o bin/manifest-serial src/load_data.c src/simulate.c src/populate.c src/main.c -lm
