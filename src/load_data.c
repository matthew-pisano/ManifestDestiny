//
// Created by matthew on 4/3/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <string.h>

#include "../include/load_data.h"

// NOTE: Data is flattened!  If mapped to an x,y coordinate system is in column-major order
// Each cell is composed of several features, multiply by cell_dim to get the index of the next cell
/*                   North
 * (0, 0) ->||  *->||  *->||  *->||
 *          ||  *  ||  *  ||  *  ||
 *    West  ||  *  ||  *  ||  *  ||  East
 *          ||  *  ||  *  ||  *  ||
 *          ||->*  ||->*  ||->*  ||-> (993, 622)
 *                   South
 */


extern void __cudaMalloc(void** ptr, size_t size);


void load_data_dims_mpi(const char *filename, int rank, int num_ranks, struct DataDims* data_dims) {

    MPI_File file;
    MPI_Status status;
    int err;
    if ((err = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file))) {
        printf("Error: Could not open file %s\n", filename);
        exit(err);
    }

    MPI_Offset file_size;
    // Get the size of the file
    if ((err = MPI_File_get_size(file, &file_size))) {
        printf("Error: Could not get file size\n");

        if ((err = MPI_File_close(&file))) {
            printf("Error: Could not close file\n");
            exit(err);
        }
        exit(err);
    }

    // Load in the dimensions of the data from the end of the file
    if ((err = MPI_File_seek(file, file_size - (3 * sizeof(unsigned short)), MPI_SEEK_SET))) {
        printf("Error: Could not seek to position in file\n");

        if ((err = MPI_File_close(&file))) {
            printf("Error: Could not close file\n");
            exit(err);
        }
        exit(err);
    }
    unsigned short dims[3];
    if ((err = MPI_File_read(file, dims, 3, MPI_UNSIGNED_SHORT, &status))) {
        printf("Error: Could not read data dimensions from file\n");

        if ((err = MPI_File_close(&file))) {
            printf("Error: Could not close file\n");
            exit(err);
        }
        exit(err);
    }

    int cols_per_rank = dims[0] / num_ranks;
    int read_cols = cols_per_rank;
    if (rank == num_ranks - 1)
        read_cols += dims[0] % num_ranks;

    data_dims->cell_dim = dims[2];
    data_dims->row_dim = read_cols;
    data_dims->global_row_dim = dims[0];
    data_dims->col_dim = dims[1];

    if ((err = MPI_File_close(&file))) {
        printf("Error: Could not close file\n");
        exit(err);
    }
}


void load_data_mpi(const char *filename, int rank, int num_ranks, struct DataDims data_dims, unsigned short **data) {

    __cudaMalloc((void **) data, data_dims.row_dim * data_dims.cell_dim * data_dims.col_dim * sizeof(unsigned short));
    if (*data == NULL) {
        printf("Error: Could not allocate memory for data\n");
        exit(1);
    }

    MPI_File file;
    MPI_Status status;
    int err;
    if ((err = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file))) {
        printf("Error: Could not open file %s\n", filename);
        exit(err);
    }
    int col_offset = rank * data_dims.global_row_dim / num_ranks;
    if ((err = MPI_File_seek(file, col_offset * data_dims.cell_dim * data_dims.col_dim * sizeof(unsigned short), MPI_SEEK_SET))) {
        printf("Error: Could not seek to position in file\n");

        if ((err = MPI_File_close(&file))) {
            printf("Error while processing above: Could not close file\n");
            exit(err);
        }
        exit(err);
    }
    if ((err = MPI_File_read(file, *data, data_dims.row_dim * data_dims.cell_dim * data_dims.col_dim, MPI_UNSIGNED_SHORT, &status))) {
        printf("Error: Could not read data from file\n");

        if ((err = MPI_File_close(&file))) {
            printf("Error while processing above: Could not close file\n");
            exit(err);
        }
        exit(err);
    }
    if ((err = MPI_File_close(&file))) {
        printf("Error: Could not close file\n");
        exit(err);
    }

    // printf("Read %lu bytes from rank %d\n", read_cols * cell_dim * row_dim * sizeof(unsigned short), rank);
}

void save_data_dims_mpi(const char *filename, struct DataDims data_dims) {

    MPI_File file;
    MPI_Status status;
    int err;
    if ((err = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file))) {
        printf("Error: Could not open file %s\n", filename);
        exit(err);
    }

    MPI_Offset file_size;
    // Get the size of the file
    if ((err = MPI_File_get_size(file, &file_size))) {
        printf("Error: Could not get file size\n");

        if ((err = MPI_File_close(&file))) {
            printf("Error while processing above: Could not close file\n");
            exit(err);
        }
        exit(err);
    }

    // Write the dimensions of the data to the end of the file
    if ((err = MPI_File_seek(file, file_size, MPI_SEEK_SET))) {
        printf("Error: Could not seek to position in file\n");

        if ((err = MPI_File_close(&file))) {
            printf("Error while processing above: Could not close file\n");
            exit(err);
        }
        exit(err);
    }
    unsigned short dims[3] = {data_dims.global_row_dim, data_dims.col_dim, data_dims.cell_dim};
    printf("Writing dimensions: %d x %d x %d\n", dims[0], dims[1], dims[2]);
    if ((err = MPI_File_write(file, dims, 3, MPI_UNSIGNED_SHORT, &status))) {
        printf("Error: Could not write data dimensions to file\n");

        if ((err = MPI_File_close(&file))) {
            printf("Error while processing above: Could not close file\n");
            exit(err);
        }
        exit(err);
    }
    if ((err = MPI_File_close(&file))) {
        printf("Error: Could not close file\n");
        exit(err);
    }
}



void save_data_body_mpi(const char *filename, int col_offset, struct DataDims data_dims, unsigned short *data) {

    MPI_File file;
    MPI_Status status;
    int err;
    if ((err = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file))) {
        printf("Error: Could not open file %s\n", filename);
        exit(err);
    }
    if ((err = MPI_File_seek(file, col_offset * data_dims.cell_dim * data_dims.col_dim * sizeof(unsigned short), MPI_SEEK_SET))) {
        printf("Error: Could not seek to position in file\n");

        if ((err = MPI_File_close(&file))) {
            printf("Error while processing above: Could not close file\n");
            exit(err);
        }
        exit(err);
    }
    if ((err = MPI_File_write(file, data, data_dims.row_dim * data_dims.cell_dim * data_dims.col_dim, MPI_UNSIGNED_SHORT, &status))) {
        printf("Error: Could not write data to file\n");

        if ((err = MPI_File_close(&file))) {
            printf("Error while processing above: Could not close file\n");
            exit(err);
        }
        exit(err);
    }
    if ((err = MPI_File_close(&file))) {
        printf("Error: Could not close file\n");
        exit(err);
    }

    // printf("Wrote %lu bytes from offset %d\n", data_dims.row_dim * data_dims.cell_dim * data_dims.col_dim * sizeof(unsigned short), col_offset);
}


void save_data_mpi(const char *filename, int it_num, int rank, int num_ranks, struct DataDims data_dims, unsigned short *data) {

    // New name in the form of <filename>_<it_num>.npy
    int new_len = strlen(filename) + 1 + 10;
    char *ext = strrchr(filename, '.');

    char *new_filename = (char *)malloc(new_len * sizeof(char));
    if (ext != NULL) {
        int filename_len = ext - filename;
        snprintf(new_filename, new_len, "%.*s_%d%s", filename_len, filename, it_num, ext);
    }
    else snprintf(new_filename, new_len, "%s_%d", filename, it_num);

    int err;
    // Remove the output file if it exists
    if (rank == 0 && access(new_filename, F_OK) == 0 && (err = remove(new_filename))) {
        printf("Error: Could not remove file %s\n", new_filename);
        exit(err);
    }

    int col_offset = rank * (data_dims.global_row_dim / num_ranks);
    save_data_body_mpi(new_filename, col_offset, data_dims, data);

    MPI_Barrier(MPI_COMM_WORLD);

    // Append the data dimensions to the end of the file, so it matches the input file
    save_data_dims_mpi(new_filename, data_dims);

    free(new_filename);
}