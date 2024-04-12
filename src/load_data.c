//
// Created by matthew on 4/3/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

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

int load_data_seq(const char *filename, struct DataDims data_dims, unsigned short **data) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return 1;
    }

    *data = (unsigned short *)calloc(data_dims.cell_dim * data_dims.row_dim * data_dims.col_dim, sizeof(unsigned short));
    if (*data == NULL) {
        printf("Error: Could not allocate memory for data\n");
        return 1;
    }

    for (int i = 0; i<data_dims.cell_dim * data_dims.row_dim * data_dims.col_dim; i++) {
        // Read in two bytes from the file and saves them as an unsigned short
        fread((*data)+i, sizeof(unsigned short), 1, file);
    }

    fclose(file);
    return 0;
}


int save_data_seq(const char *filename, struct DataDims data_dims, unsigned short *data) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return 1;
    }

    for (int i = 0; i<data_dims.cell_dim * data_dims.row_dim * data_dims.col_dim; i++) {
        // Write two bytes to the file
        fwrite(&data[i], sizeof(unsigned short), 1, file);
    }

    fclose(file);
    return 0;
}


int load_data_dims_mpi(const char *filename, struct DataDims* data_dims) {

    MPI_File file;
    MPI_Status status;
    int err;
    if ((err = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file))) {
        printf("Error: Could not open file %s\n", filename);
        return err;
    }

    MPI_Offset file_size;
    // Get the size of the file
    if ((err = MPI_File_get_size(file, &file_size))) {
        printf("Error: Could not get file size\n");
        return err;
    }

    // Load in the dimensions of the data from the end of the file
    if ((err = MPI_File_seek(file, file_size - (3 * sizeof(unsigned short)), MPI_SEEK_SET))) {
        printf("Error: Could not seek to position in file\n");
        return err;
    }
    unsigned short dims[3];
    if ((err = MPI_File_read(file, dims, 3, MPI_UNSIGNED_SHORT, &status))) {
        printf("Error: Could not read data dimensions from file\n");
        return err;
    }
    data_dims->cell_dim = dims[2];
    data_dims->row_dim = dims[0];
    data_dims->col_dim = dims[1];

    if ((err = MPI_File_close(&file))) {
        printf("Error: Could not close file\n");
        return err;
    }

    return 0;
}


int load_data_mpi(const char *filename, int col_offset, struct DataDims data_dims, unsigned short **data) {

    *data = (unsigned short *)calloc(data_dims.row_dim * data_dims.cell_dim * data_dims.col_dim, sizeof(unsigned short));
    if (*data == NULL) {
        printf("Error: Could not allocate memory for data\n");
        return 1;
    }

    MPI_File file;
    MPI_Status status;
    int err;
    if ((err = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file))) {
        printf("Error: Could not open file %s\n", filename);
        return err;
    }
    if ((err = MPI_File_seek(file, col_offset * data_dims.cell_dim * data_dims.col_dim * sizeof(unsigned short), MPI_SEEK_SET))) {
        printf("Error: Could not seek to position in file\n");
        return err;
    }
    if ((err = MPI_File_read(file, *data, data_dims.row_dim * data_dims.cell_dim * data_dims.col_dim, MPI_UNSIGNED_SHORT, &status))) {
        printf("Error: Could not read data from file\n");
        return err;
    }
    if ((err = MPI_File_close(&file))) {
        printf("Error: Could not close file\n");
        return err;
    }

    // printf("Read %lu bytes from rank %d\n", read_cols * cell_dim * row_dim * sizeof(unsigned short), rank);

    return 0;
}

int save_data_dims_mpi(const char *filename, struct DataDims data_dims) {

    MPI_File file;
    MPI_Status status;
    int err;
    if ((err = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file))) {
        printf("Error: Could not open file %s\n", filename);
        return err;
    }

    MPI_Offset file_size;
    // Get the size of the file
    if ((err = MPI_File_get_size(file, &file_size))) {
        printf("Error: Could not get file size\n");
        return err;
    }

    // Write the dimensions of the data to the end of the file
    if ((err = MPI_File_seek(file, file_size, MPI_SEEK_SET))) {
        printf("Error: Could not seek to position in file\n");
        return err;
    }
    unsigned short dims[3] = {data_dims.row_dim, data_dims.col_dim, data_dims.cell_dim};
    if ((err = MPI_File_write(file, dims, 3, MPI_UNSIGNED_SHORT, &status))) {
        printf("Error: Could not write data dimensions to file\n");
        return err;
    }
    if ((err = MPI_File_close(&file))) {
        printf("Error: Could not close file\n");
        return err;
    }

    return 0;
}



int save_data_mpi(const char *filename, int col_offset, struct DataDims data_dims, unsigned short *data) {

    MPI_File file;
    MPI_Status status;
    int err;
    if ((err = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file))) {
        printf("Error: Could not open file %s\n", filename);
        return err;
    }
    if ((err = MPI_File_seek(file, col_offset * data_dims.cell_dim * data_dims.col_dim * sizeof(unsigned short), MPI_SEEK_SET))) {
        printf("Error: Could not seek to position in file\n");
        return err;
    }
    if ((err = MPI_File_write(file, data, data_dims.row_dim * data_dims.cell_dim * data_dims.col_dim, MPI_UNSIGNED_SHORT, &status))) {
        printf("Error: Could not write data to file\n");
        return err;
    }
    if ((err = MPI_File_close(&file))) {
        printf("Error: Could not close file\n");
        return err;
    }

    // printf("Wrote %lu bytes from rank %d\n", write_cols * cell_dim * row_dim * sizeof(unsigned short), rank);

    return 0;
}