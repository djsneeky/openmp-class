#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

// offset into row, then over to correct column
#define idx(u, r, c) (u[r*COLS + c])

#define ROWS            16
#define COLS            16
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
            idx(arr, r, c) = (double)(rows * r + c);
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
            idx(arr, r, c) = 1.0;
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
    double *b = NULL;
    double *c = NULL;
    const int stripe_width = COLS / size;
    double execTime;

    // stripes = num rows of A and cols of B for each proc
    // each proc will compute p * stripes : (p+1) * stripes of the rows of C matrix

    // data for each proc:
    // each proc will have STRIPES rows of A and STRIPES cols of B
    // each proc will have ALL cols of A and ALL rows of B
    // each proc will have STRIPES rows of C and a ALL cols of C
    
    // after compute shift STRIPES cols B left and grab more STRIPES cols of B from right proc (p + 1)
    // note: handle wrap case for p0 and pmax

    // after completion, each proc sends computed rows C to p0, which forms and prints
    // TODO: how to form result?


    if (rank == 0)
    {
        printf("mm_mpi has started with %d ranks.\r\n",size);

        // create arrays
        a = makeArray(ROWS, COLS);
        c = makeArray(ROWS, COLS);

        printf("Array a:\r\n");
        printArray(a, ROWS, COLS);

        execTime = -MPI_Wtime();
    }

    /**
     * Send data to procs
     * 
     * STRIPES rows of A and STRIPES cols of B
     * STRIPES rows of C generated from mm
     */

    // pointer for a_stripe used after scatter
    const int a_stripe_cnt = stripe_width * ROWS;
    double *a_stripe = (double *)malloc(a_stripe_cnt * sizeof(double));
    // pointer for b stripe, generated locally
    // double buffered to prevent deadlock
    const int b_stripe_cnt = stripe_width * COLS;
    double *b_stripe = makeArrayOnes(b_stripe, ROWS, stripe_width);
    double *b_stripe_new = (double *)malloc(b_stripe_cnt * sizeof(double));
    // pointer for c stripe, generated locally
    const int c_stripe_cnt = a_stripe_cnt;
    double *c_stripe = makeArrayOnes(c_stripe, stripe_width, COLS);

    if (rank == 0)
    {
        printf("Scattering, computing, and gathering data...\r\n");
    }
    MPI_Scatter(a, a_stripe_cnt, MPI_DOUBLE, a_stripe, a_stripe_cnt, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // COMPUTE

    // Generated internally once
    a_row_offset = rank * stripe_width;
    // b col is generated internally to each proc
    // initial offset set here, then updated after send/recv
    b_col_offset = rank * stripe_width;
    // destination rank, loops around with mod
    int dest_rank;
    int prev_rank;

    for (int rank_cnt = 0; rank_cnt < size; rank_cnt++)
    {
        // iterating over rows of a and c
        for (int i = a_row_offset; i < a_row_offset + stripe_width; i++)
        {
            // iterating over cols of b and c
            for (int j = b_col_offset; j < b_col_offset + stripe_width; j++)
            {
                double comp = 0.;
                // iterating over cols of a and rows of b
                for (int k = 0; k < COLS; k++)
                {
                    comp += idx(a_stripe,i,k) * idx(b_stripe,k,j);
                }
                // storing result in row and col of c
                idx(c_stripe,i,j) = comp;
            }
        }

        dest_rank = (rank + 1) % size;
        prev_rank = ((rank - 1) + size) % size;

        // deadlock prevention
        if (rank % 2 == 0)
        {
            // send b_stripe to the right
            MPI_Send(b_stripe, b_stripe_cnt, MPI_DOUBLE, dest_rank, 1, MPI_COMM_WORLD);

            // receive a new b_stripe from the left
            MPI_Recv(b_stripe_new, b_stripe_cnt, MPI_DOUBLE, prev_rank, MPI_ANY_TAG, MPI_COMM_WORLD);
        }
        else
        {
            // receive a new b_stripe from the left
            MPI_Recv(b_stripe_new, b_stripe_cnt, MPI_DOUBLE, prev_rank, MPI_ANY_TAG, MPI_COMM_WORLD);

            // send b_stripe to the right
            MPI_Send(b_stripe, b_stripe_cnt, MPI_DOUBLE, dest_rank, 1, MPI_COMM_WORLD);
        }

        // update offsets
        b_col_offset = prev_rank * stripe_width;

        // update pointers for b_stripe data
        b_stripe = b_stripe_new;
    }

    // full stripe of c computed and located at a_row_offset

    double *c_build = NULL;
    if (rank == 0)
    {
        c_build = (double *)malloc(NUM_ELEMENTS * sizeof(double));
    }
    MPI_Gather(c_stripe, c_stripe_cnt, MPI_DOUBLE, c_build, c_stripe_cnt, MPI_DOUBLE, 0, MPI_COMM_WORLD);

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
