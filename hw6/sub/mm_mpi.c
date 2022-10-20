#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

// offset into row, then over to correct column
#define idx(u, r, c, max_cols) *(u + r*max_cols + c)

#define ROWS            1600
#define COLS            1600
#define NUM_ELEMENTS    (ROWS * COLS)

void printArray(double *a, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%.2f ", *(a + i * cols + j));
        }
        printf("\r\n");
    }
    printf("\r\n");
}

double *makeArray(int rows, int cols)
{
    double *arr = (double *)malloc(rows * cols * sizeof(double));

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            idx(arr, r, c, cols) = (double)(rows * r + c);
        }
    }

    return arr;
}

double *makeArrayOnes(int rows, int cols)
{
    double *arr = (double *)malloc(rows * cols * sizeof(double));

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            idx(arr, r, c, cols) = 1.0;
        }
    }

    return arr;
}

int min(int i, int j)
{
    return ((i) < (j) ? (i) : (j));
}

int main(int argc, char *argv[])
{

    /* MPI Parameters */
    int rank, size, len;
    char name[MPI_MAX_PROCESSOR_NAME];

    /* All ranks initiate the message-passing environment. */
    /* Each rank obtains information about itself and its environment. */
    MPI_Init(&argc, &argv);                 /* start MPI           */
    MPI_Comm_size(MPI_COMM_WORLD, &size);   /* get number of ranks */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);   /* get rank            */
    MPI_Get_processor_name(name, &len);     /* get run-host name   */

    double *a = NULL;
    const int stripe_width = COLS / size;
    double execTime;

    if (rank == 0)
    {
        printf("mm_mpi has started with %d ranks.\r\n",size);
    }

    // block all tasks to get accurate timing
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
    {
        execTime = -MPI_Wtime();
        // create arrays
        a = makeArray(ROWS, COLS);

        printf("Array a:\r\n");
        printArray(a, ROWS, COLS);
    }

    // pointer for a_stripe used after scatter
    const int a_stripe_cnt = stripe_width * COLS;
    double *a_stripe = (double *)malloc(a_stripe_cnt * sizeof(double));
    // pointer for b stripe, generated locally
    // double buffered to prevent deadlock
    const int b_stripe_cnt = stripe_width * ROWS;
    double *b_stripe = makeArrayOnes(ROWS, stripe_width);
    double *b_stripe_new = (double *)malloc(b_stripe_cnt * sizeof(double));
    // pointer for c stripe, generated locally
    const int c_stripe_cnt = a_stripe_cnt;
    double *c_stripe = (double *)malloc(c_stripe_cnt * sizeof(double));

    MPI_Scatter(a, a_stripe_cnt, MPI_DOUBLE, a_stripe, a_stripe_cnt, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // COMPUTE

    // Generated internally once
    int a_row_offset = rank * stripe_width;
    // b col is generated internally to each proc
    // initial offset set here, then updated after send/recv
    int b_col_offset = rank * stripe_width;
    // destination rank, loops around with mod
    int dest_rank = (rank + 1) % size;
    int prev_rank = ((rank - 1) + size) % size;

    if (rank == 0)
    {
        printf("a_row_offset on rank %d: %d\r\n", rank, a_row_offset);
        printf("a_stripe on rank %d:\r\n", rank);
        printArray(a_stripe, stripe_width, COLS);
        printf("b_stripe on rank %d:\r\n", rank);
        printArray(b_stripe, ROWS, stripe_width);
    }

    for (int rank_cnt = 0; rank_cnt < size; rank_cnt++)
    {
        if (rank == 0)
        {
            printf("b_col_offset on rank %d iteration %d: %d\r\n", rank, rank_cnt, b_col_offset);
        }

        // iterating over rows of a and c
        for (int i = 0; i < stripe_width; i++)
        {
            // iterating over cols of b and c
            for (int j = 0; j < stripe_width; j++)
            {
                double comp = 0.;
                // iterating over cols of a and rows of b
                for (int k = 0; k < COLS; k++)
                {
                    comp += idx(a_stripe,i,k,COLS) * idx(b_stripe,j,k,stripe_width);
                }
                // storing result in row and col of c
                idx(c_stripe,i,j + b_col_offset,COLS) = comp;
            }
        }

        // deadlock prevention
        if (rank % 2 == 0)
        {
            // send b_stripe to the right
            MPI_Send(b_stripe, b_stripe_cnt, MPI_DOUBLE, dest_rank, 1, MPI_COMM_WORLD);
            // receive a new b_stripe from the left
            MPI_Recv(b_stripe_new, b_stripe_cnt, MPI_DOUBLE, prev_rank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else
        {
            // receive a new b_stripe from the left
            MPI_Recv(b_stripe_new, b_stripe_cnt, MPI_DOUBLE, prev_rank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // send b_stripe to the right
            MPI_Send(b_stripe, b_stripe_cnt, MPI_DOUBLE, dest_rank, 1, MPI_COMM_WORLD);
        }

        // update offsets
        b_col_offset = ((b_col_offset - stripe_width) + COLS) % COLS;

        // update pointers for b_stripe data
        b_stripe = b_stripe_new;
    }

    if (rank == 0)
    {
        printf("c_stripe on rank %d:\r\n", rank);
        printArray(c_stripe, stripe_width, COLS);
    }

    // GATHER
    // full stripe of c computed and located at a_row_offset

    double *c_build = NULL;
    if (rank == 0)
    {
        c_build = (double *)malloc(NUM_ELEMENTS * sizeof(double));
    }
    MPI_Gather(c_stripe, c_stripe_cnt, MPI_DOUBLE, c_build, c_stripe_cnt, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // block all tasks to get accurate timing
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
    {
        execTime += MPI_Wtime();
        printf("Result array c:\r\n");
        printArray(c_build, ROWS, COLS);
        printf("Time taken for matrix multiply - mpi: %.2lf\r\n", execTime);
    }

    MPI_Finalize();                         /* terminate MPI       */
    return 0;
}
