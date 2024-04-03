//
// Created by matthew on 4/3/24.
//

#include <stdio.h>

#include "load_data.h"

// NOTE: Data is flattened!  If mapped to an x,y coordinate system is in column-major order
// Each cell is composed of several features, multiply by cell_dim to get the index of the next cell
/* (0, 0)       North
 *       |  |->|  |->|  |->|
 * West  |  |  |  |  |  |  |  East
 *       |  |  |  |  |  |  |
 *       |->|  |->|  |->|  |-> (993, 622)
 *              South
 */

int load_data(unsigned short *data, int cell_dim, int row_dim, int col_dim, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return 1;
    }

    for (int i = 0; i<cell_dim * row_dim * col_dim; i++) {
        // Read in two bytes from the file and saves them as an unsigned short
        fread(&data[i], sizeof(unsigned short), 1, file);
    }

    fclose(file);
    return 0;
}