#include <stdio.h>
#include <stdlib.h>

#include "load_data.h"


int main() {
    unsigned short *data;
    int cell_dim = 7;
    int row_dim = 623;
    int col_dim = 994;

    data = (unsigned short *)malloc(cell_dim * row_dim * col_dim * sizeof(unsigned short));
    if (data == NULL) {
        printf("Error: Could not allocate memory for data\n");
        return 1;
    }

    const char *filename = "scripts/out/feature_matrix.npy";
    if (load_data(data, cell_dim, row_dim, col_dim, filename) != 0) {
        return 1;
    }

    return 0;
}
