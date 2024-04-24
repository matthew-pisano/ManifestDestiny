import sys

import numpy as np
from tqdm import tqdm


if __name__ == "__main__":

    start_file = sys.argv[1]
    end_file = sys.argv[2]
    start_array = np.fromfile(start_file, dtype=np.ushort)
    end_array = np.fromfile(end_file, dtype=np.ushort)

    for i in tqdm(range(len(start_array))):
        if start_array[i] != end_array[i]:
            print(f"Index {i} differs: {start_array[i]} != {end_array[i]}")
            sys.exit(1)

    print("Arrays are equal")
