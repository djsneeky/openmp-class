#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

void printArray(double *a, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%.2f ", *(a + i * cols + j));
        }
        printf("\n");
    }
    printf("\n\n\n");
}

double *makeArray(int rows, int cols)
{
    double *arr = (double *)malloc(rows * cols * sizeof(double));

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            *(arr + r * cols + c) = (double)(rows * c + c);
        }
    }

    return arr;
}

int main(int argc, char *argv[])
{

    const int ROWS = 1600;
    const int COLS = 1600;
    const int tasks = 1600;
    const int stripeSize = COLS / tasks;

    double *a = makeArray(ROWS, COLS);
    double *b = makeArray(ROWS, COLS);
    double *c = makeArray(ROWS, COLS);
    
    printf("Array a:\r\n");
    printArray(a, ROWS, COLS);
    printf("Array b:\r\n");
    printArray(b, ROWS, COLS);

    double execTime;

    execTime = -omp_get_wtime();
    // iterate over number of tasks
    #pragma omp parallel for
    for (int t = 0; t < tasks; t++)
    {
        // iterate over stripes
        for (int i = t * stripeSize; i < min(t * stripeSize + stripeSize, ROWS); i++)
        {
            // multiplication of elements
            for (int j = 0; j < COLS; j++)
            {
                double comp = 0.;
                for (int k = 0; k < COLS; k++)
                {
                    comp += *(a + i * COLS + k) * *(b + k * COLS + j);
                }
                *(c + i * COLS + j) = comp;
            }
        }
    }
    execTime += omp_get_wtime();

    printArray(c, ROWS, COLS);
    printf("Time taken for matrix multiply - openmp: %.2lf", execTime);

}
