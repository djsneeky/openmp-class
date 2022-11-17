#include <stdio.h>

#define TOTAL_THREADS       172032
#define THREADS_PER_BLOCK   128
#define TOTAL_BLOCKS        (TOTAL_THREADS / THREADS_PER_BLOCK) // 1344

#define VECTOR_LENGTH       (5 * TOTAL_THREADS)

__global__ void dotProduct(double *d_c, double *d_a, double *d_b, int length, int valsPerThread)
{
    // declare a buffer in shared memory to hold the partial reductions from each thread
    // in a block. You will need to use a constant value to declare this, so use the
    // number of threads/block that you have computed.
    __shared__ double partial[THREADS_PER_BLOCK];

    // declare a thread local/automatic variable (we'll call it c) in a register to hold
    // the results for each thread in the loop below.
    double c = 0;

    // compute the local dot product for each thread's values
    // each thread will do a multiply and summation across valsPerThread elements
    // of the d_a and d_b vectors. All threads in a block should access adjacent
    // elements. I would suggest all threads on the device accessing a block of
    // data, and then moving on to the next block, and doing this a total of
    // valsPerThread times.
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    int stride = blockDim.x * gridDim.x;
    for (int i = 0; i < valsPerThread; i++) {
        c += d_a[idx] * d_b[idx];
        idx += stride;
    }

    // store c into the proper thred position of the shared memory buffer declared
    // above.
    partial[threadIdx.x] = c;

    if (threadIdx.x == 0 && blockIdx.x == 0)
    {
        printf("Partial product on thread %d block %d: %lf\r\n", threadIdx.x, blockIdx.x, partial[threadIdx.x]);
    }

    // reduce the values in the buffer to have a single value in the zero element of
    // each buffer.  Use the "good" reduction described in the histogram slides
    // Remember to synchronize appropriately.
    // bad way:
    // if (threadIdx.x == 0)
    // {
    //     double sum = 0;
    //     for (int i = 0; i < THREADS_PER_BLOCK; i++)
    //     {
    //         sum += partial[i];
    //     }
    //     partial[0] = sum;
    // }
    // better reduction:
    for (unsigned int i = blockDim.x / 2; i > 0; i /= 2)
    {
        __syncthreads();
        if (threadIdx.x < i)
        {
            partial[threadIdx.x] += partial[threadIdx.x+i];
        }
    }

    if (threadIdx.x == 0 && blockIdx.x == 0)
    {
        printf("Partial sums on thread %d block %d: %lf\r\n", threadIdx.x, blockIdx.x, partial[0]);
    }

    // write the partial reduction for each block stored in element zero of the shared
    // buffer, i.e., the value produced by the reduction above, into the proper
    // location for the block in d_c.
    d_c[blockIdx.x] = partial[0];
}

double hdotProduct(double *h_c, double *h_a, double *h_b, int lengthBytes, int lengthElements,
                   int outputSize, int numBlocks, int threadsBlock)
{
    double *d_a, *d_b, *d_c;
    double sum = 0;
    int valsPerThread = 5;

    // Allocate memory on the device for the d_a, d_b and d_c arrays. Note that the
    // lengths of each are in bytes, not doubles.
    //
    // Copy the h_a and h_b arrays to the d_a and d_b arrays on the gpu.  There is no
    // need to copy d_c values as d_c only holds return values.
    cudaMalloc((void **)&d_a, lengthBytes);
    cudaMalloc((void **)&d_b, lengthBytes);
    cudaMalloc((void **)&d_c, outputSize);

    cudaMemcpy(d_a, h_a, lengthBytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, h_b, lengthBytes, cudaMemcpyHostToDevice);

    // launch the kernel. Have four warps of 32 threads (128 threads) for each block.
    // If you use print statements make sure to have a cudaDeviceSynchronize();
    // statement after the launch.
    dotProduct<<<numBlocks, threadsBlock>>>(d_c, d_a, d_b, lengthElements, valsPerThread);

    // copy the d_c array from the device into the h_c array.
    // free d_a, d_b and d_c.
    cudaMemcpy(h_c, d_c, outputSize, cudaMemcpyDeviceToHost);

    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);

    // sum the values now in h_c to get the final reduction value, and return that from
    // the function.
    for (int i = 0; i < numBlocks; i++)
    {
        sum += h_c[i];
    }

    return sum;
}

int main(int argc, char **args)
{

    // compute necessary values for the problem, such as number threads per block, etc..
    int lengthElements = VECTOR_LENGTH;
    int lengthBytes = lengthElements * (sizeof(double));
    int outputSize = TOTAL_BLOCKS * (sizeof(double));
    double host_dot = 0;
    double device_dot = 0;

    // declare and allocate h_a, h_b and h_c on the host.
    double *h_a, *h_b, *h_c;
    h_a = (double *)malloc(lengthBytes);
    h_b = (double *)malloc(lengthBytes);
    h_c = (double *)malloc(outputSize);

    // initialize h_a and h_b. I initialized one with i, the position in the array
    // being initialized, and the other with 1.
    for (int i = 0; i < VECTOR_LENGTH; i++)
    {
        h_a[i] = 1.0;
        h_b[i] = 1.0;
    }

    // compute and print the sequential solution
    for (int i = 0; i < lengthElements; i++)
    {
        host_dot += h_a[i] * h_b[i];
    }

    printf("host dotProduct: %lf\r\n", host_dot);

    // call hdotProduct, print the value of c returned (which should equal the sequential
    // value printed above, and free h_a, h_b and h_c.
    device_dot = hdotProduct(h_c, h_a, h_b, lengthBytes, lengthElements, outputSize, TOTAL_BLOCKS, THREADS_PER_BLOCK);
    cudaDeviceSynchronize();

    printf("device dotProduct: %lf\r\n", device_dot);

    free(h_a);
    free(h_b);
    free(h_c);

    return 0;
}
