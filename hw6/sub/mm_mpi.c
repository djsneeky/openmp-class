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
        b = makeArray(ROWS, COLS);
        c = makeArray(ROWS, COLS);


        printf("Array a:\r\n");
        printArray(a, ROWS, COLS);

        execTime = -MPI_Wtime();
    }

    /**
     * Send data to procs
     * 
     * STRIPES rows of A and STRIPES cols of B
     * ALL cols of A and ALL rows of B
     * STRIPES rows of C and a ALL cols of C
     */
    // a_offset = 0;
    // b_offset = 0;
    // c_offset = 0;
    // for (int r = 1; r < size; r++)
    // {
    //     for (int i = t * stripeSize; i < min(t * stripeSize + stripeSize, ROWS); i++)
    //     MPI_Send(a + a_offset, stripeSize, MPI_INT, r, 1, MPI_COMM_WORLD);
    //     MPI_Send(b + b_offset, stripeSize, MPI_INT, r, 1, MPI_COMM_WORLD);
    //     TODO: isn't this defeating the purpose of sending stripes?
    //     MPI_Send(a, sizeof(a), MPI_INT, r, 1, MPI_COMM_WORLD);
    //     MPI_Send(b, sizeof(b), MPI_INT, r, 1, MPI_COMM_WORLD);
    //     MPI_Send(c + c_offset, stripeSize, MPI_INT, r, 1, MPI_COMM_WORLD);
    //     MPI_Send(c, sizeof(c), MPI_INT, r, 1, MPI_COMM_WORLD);
    // }

    const int a_stripe_cnt = stripe_width * COLS;
    double *a_stripe = (double *)malloc(a_stripe_cnt * sizeof(double));
    const int b_stripe_cnt = stripe_width * ROWS;
    double *b_stripe = (double *)malloc(b_stripe_cnt * sizeof(double));

    if (rank == 0)
    {
        printf("Scattering data...\r\n");
    }
    MPI_Scatter(a, a_stripe_cnt, MPI_DOUBLE, a_stripe, a_stripe_cnt, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // compute
    printf("rank %d a_stripe: ", rank);
    printArray(a_stripe, stripe_width, COLS);

    double *a_reconstruct = NULL;
    if (rank == 0)
    {
        a_reconstruct = (double *)malloc(NUM_ELEMENTS * sizeof(double));
    }
    MPI_Gather(a_stripe, a_stripe_cnt, MPI_DOUBLE, a_reconstruct, a_stripe_cnt, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("Gathering data...\r\n");
        printArray(a_reconstruct, ROWS, COLS);
    }

    // WORKER SECTION

    // multiplication of elements
    // for (int j = 0; j < stripe_col_size; j++)
    // {
    //     double comp = 0.;
    //     for (int k = 0; k < stripe_col_size; k++)
    //     {
    //         comp += *(a + i * stripe_col_size + k) * *(b + k * stripe_col_size + j);
    //     }
    //     *(c + i * stripe_col_size + j) = comp;
    // }

    // send result to master task

    // iterate over number of tasks
    // for (int t = 0; t < tasks; t++)
    // {
    //     // iterate over stripes
    //     for (int i = t * stripeSize; i < min(t * stripeSize + stripeSize, ROWS); i++)
    //     {
    //         // multiplication of elements
    //         for (int j = 0; j < COLS; j++)
    //         {
    //             double comp = 0.;
    //             for (int k = 0; k < COLS; k++)
    //             {
    //                 comp += *(a + i * COLS + k) * *(b + k * COLS + j);
    //             }
    //             *(c + i * COLS + j) = comp;
    //         }
    //     }
    // }

    if (rank == 0)
    {
        execTime += MPI_Wtime();
        printf("Time taken for matrix multiply - mpi: %.2lf\r\n", execTime);
    }

    // printArray(c, ROWS, COLS);

    MPI_Finalize();                         /* terminate MPI       */
    return 0;
}
