#include<stdio.h>
#include<cuda.h>
#include<cuda_runtime.h>

// Expose CUDA functions to the MPI code
extern "C" {
    bool HL_kernelLaunch(int rank, unsigned char** d_data, unsigned char** d_resultData,
                         unsigned char** preData, unsigned char** postData,
                         size_t worldWidth, size_t worldHeight, ushort threadsCount);

    void cudaMallocWrapper(void** ptr, size_t size);
    void cudaFreeWrapper(void* ptr);
}

/**
 * Use CUDA malloc to allocate memory on the device.
 * @param ptr The pointer to the memory to allocate.
 * @param size The size of the memory to allocate.
 */
void cudaMallocWrapper(void** ptr, size_t size) {
    cudaMallocManaged(ptr, size);
}

/**
 * Use CUDA free to deallocate memory on the device.
 * @param ptr The pointer to the memory to deallocate.
 */
void cudaFreeWrapper(void* ptr) {
    cudaFree(ptr);
}


/**
 * Swap the pointers of two arrays.
 * @param pA The first array.
 * @param pB The second array.
 */
static inline void HL_swap( unsigned char **pA, unsigned char **pB) {
  unsigned char *temp = *pA;
  *pA = *pB;
  *pB = temp;
}


/**
 * Count the number of alive cells in the neighborhood of a cell.
 * @param data The current state of the world.
 * @param preData The previous ghost row.
 * @param postData The next ghost row.
 * @param x0 The x coordinate of the first column of cells.
 * @param x1 The x coordinate of the second column of cells.
 * @param x2 The x coordinate of the third column of cells.
 * @param y0 The y coordinate of the first row of cells.
 * @param y1 The y coordinate of the second row of cells.
 * @param y2 The y coordinate of the third row of cells.
 * @return The number of alive cells in the neighborhood.
 */
__device__ unsigned int HL_countAliveCells(const unsigned char* data, unsigned char* preData, unsigned char* postData,
					   size_t x0,
					   size_t x1,
					   size_t x2,
					   size_t y0,
					   size_t y1,
					   size_t y2) {

    unsigned int topSum = 0;
    unsigned int botSum = 0;

    // If y0 does not wrap around: set the top sum normally, otherwise: set the top sum based on the previous ghost row
    if (y0 < y1) topSum += data[x0 + y0] + data[x1 + y0] + data[x2 + y0];
    else topSum += preData[x0] + preData[x1] + preData[x2];

    // If y2 does not wrap around: set the bottom sum normally, otherwise: set the bottom sum based on the next ghost row
    if (y2 > y1) botSum += data[x0 + y2] + data[x1 + y2] + data[x2 + y2];
    else botSum += postData[x0] + postData[x1] + postData[x2];

    // Return the final sum of the neighborhood
    return topSum + data[x0 + y1] + data[x2 + y1] + botSum;
}

/**
 * The kernel for running HighLife in parallel on a GPU.
 * @param d_data The current state of the world.
 * @param d_resultData The next state of the world.
 * @param worldWidth The width of the world.
 * @param worldHeight The height of the world.
 */
__global__ void HL_kernel(int rank, const unsigned char* d_data, unsigned char* d_resultData,
                          unsigned char* preData, unsigned char* postData, size_t worldWidth, size_t worldHeight, int cell_dim) {
    int device;
    cudaGetDevice( &device );

    // Calculate the index of the cell in the world, striding by the total number of threads in the grid

    // **Use size_t type to avoid overflows when repeatedly adding**
    // Individual variables (blockIdx.x, etc.) likely do not need to be cast to size_t, but memory is not very constrained, so it can't hurt
    // Cast anyways for compatibility
    for(size_t index = (size_t) blockIdx.x * (size_t) blockDim.x + (size_t) threadIdx.x;
            index < worldWidth*worldHeight;
            index += (size_t) blockDim.x * (size_t) gridDim.x) {


        size_t trueWidth = worldWidth * cell_dim;
        // Calculate the x and y coordinates of the cell based on the global index
        size_t x = index % trueWidth;
        size_t y = index / trueWidth;

        // Calculate the surrounding cells exactly like the serial program
        size_t y0 = ((y + worldHeight - 1) % worldHeight) * trueWidth;
        size_t y1 = y * trueWidth;
        size_t y2 = ((y + 1) % worldHeight) * trueWidth;

        size_t x0 = (x + trueWidth - cell_dim) % trueWidth;
        size_t x2 = (x + cell_dim) % trueWidth;

        // Count the number of alive cells in the neighborhood
        unsigned int cityChance = rateSpot(d_data, preData, postData, cell_dim, x0, x, x2, y0, y1, y2);
        // rule B36/S23
        // Set the next state of the cell based on the number of alive cells in the neighborhood
        d_resultData[x + y1] = (aliveCells == 3) || (aliveCells == 6 && !d_data[x + y1])
            || (aliveCells == 2 && d_data[x + y1]) ? 1 : 0;
    }
}

/**
 * Launch the kernel once for each iteration, synchronizing after each launch.
 * @param d_data The current state of the world.
 * @param d_resultData The next state of the world.
 * @param worldWidth The width of the world.
 * @param worldHeight The height of the world.
 * @param iterationsCount The number of iterations to run the algorithm.
 * @param threadsCount The number of threads to allocate to each block in the kernel.
 * @return True if the kernel was launched successfully.
 */
bool HL_kernelLaunch(int rank, unsigned char** d_data, unsigned char** d_resultData,
                     unsigned char** preData, unsigned char** postData,
                     size_t worldWidth, size_t worldHeight, int cell_dim, ushort threadsCount) {

    // Get the number of cuda devices and set the current device to the rank modulo the number of devices
    cudaError_t cudaError;
    int cudaDeviceCount;
    if( (cudaError = cudaGetDeviceCount( &cudaDeviceCount)) != cudaSuccess ) {
        printf(" Unable to determine cuda device count, error is %d, count is %d\n", cudaError, cudaDeviceCount );
        exit(-1);
    }
    if( (cudaError = cudaSetDevice( rank % cudaDeviceCount )) != cudaSuccess ) {
        printf(" Unable to have rank %d set to cuda device %d, error is %d \n", rank, (rank % cudaDeviceCount), cudaError);
        exit(-1);
    }

    // Determine how many blocks should be allocated to the kernel with a maximum of 65535
    size_t blockCount = (worldHeight * worldWidth * cell_dim) / threadsCount + 1;
    blockCount = blockCount > 65535 ? 65535 : blockCount;
    // Determine the number of threads to allocate to each block in the kernel
    size_t kernelThreads = threadsCount;
    // Launch the kernel with the determined block count and thread count
    HL_kernel<<<blockCount, kernelThreads>>>(rank, *d_data, *d_resultData, *preData, *postData, worldWidth, worldHeight, cell_dim);
    // Synchronize the device after each launch
    cudaDeviceSynchronize();

    // Swap the pointers of the current state and the next state of the world
    HL_swap(d_data, d_resultData);

    return true;
}


int slopeThresh = 20;
float basePercent = 50;
int *slopePenalty;

int *tempMin;
int *tempMax;
int *tempPenalty;

int *rainMax;
int *rainMin;
int *rainPenalty;

float waterBuff;

/* short slope, 
short temp, 
short rain, 
short elevation, 
short water, 
short resources, 
short biome */


//preData is the ghost row before
//postData is the ghost row after
int rateSpot(char* d_data, char* preData, char* postData, int cell_dim, size_t x0, size_t x, size_t x2, size_t y0, size_t y1, size_t y2);{
    int cityChance = basePercent;

    short slope = d_data[x];
    short temp = d_data[x+1];
    short rainfall = d_data[x+2];
    short elevation = d_data[x+3];
    short water = d_data[x+4];
    short[] neighborWater = [d_data[y0+x0+4], d_data[y0+x+4], d_data[y0+x2+4], d_data[y1+x0+4], d_data[y1+x2+4], d_data[y2+x0+4], d_data[y2+x+4], d_data[y2+x2+4],]
    short resource = d_data[x+5];
    short biome = d_data[x+6];

    //being underwater is bad
    if(water != 0){
        return 0;
    }

    //count the number of neighbors that are underwater. Coastline, good, peninsula fine, island questionable
    short neighborsUnderwater = 0;
    for(int i = 0; i<neighborWater.size(); i++){
        if(neighborWater[i] != 0){
            neighborsUnderwater += 1;
        }
    }

    //add buffs accordingly
    if (neighborsUnderwater > 5){
        cityChance += *peninsulaBuff;
    }
    elif(neighborsUnderwater > 2){
        cityChance += *coastBuff;
    }
    elif(neighborsUnderwater > 0){
        cityChance += *waterBuff;
    }

    //add up the elevation differences between this cell and its neighbors, if greater than slopeThresh, sub
    float slopeSum = 0;
    for(int i = 0; i<slope.size(); i++){
        slopeSum += abs(slope[i] - elevation);
    }
    if(slopeSum > slopeThresh){
        cityChance -= (slopeSum * (*slopePenalty));
    }

    //very hot or very cold is bad
    if(temp > *tempMax || temp < *tempMin){
        cityChance -= *tempPenalty;
    }    

    //rainforest and desert are bad
    if(rain > *rainMax || rain < *rainMin){
        cityChance -= *rainPenalty;
    }

    //resources good
    for(int r = 0; r<resources.size(); r++){
        cityChance += resourceMap(resources[r]);
    }


    return cityChance; 

}