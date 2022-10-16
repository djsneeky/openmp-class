#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define PROBLEMSIZE 10000000

double dproduct(float *a, float *b, float low, float high)
{
    double result = 0.0;
    for (int i = low; i < high; i++)
    {
        result += (a[i] * b[i]);
    }
    return result;
}

int main()
{

    float *a;
    float *b;
    a = malloc(sizeof(float) * PROBLEMSIZE);
    b = malloc(sizeof(float) * PROBLEMSIZE);

    if (a == NULL)
    {
        printf("a is null\n");
        free(a);
        free(b);
        fflush(stdout);
    }

    if (b == NULL)
    {
        printf("b is null\n");
        free(a);
        free(b);
        fflush(stdout);
    }

    double res = 0;
    double execTime;

    for (int i = 0; i < PROBLEMSIZE; i++)
    {
        a[i] = b[i] = 1;
    }

    // for (int i = 0; i < NUMTHREADS; i++)
    // {
    //     result[i] = 0;
    // }

    // sequential execution to check the answer
    execTime = -omp_get_wtime();
    res = dproduct(a, b, 0,      /* lower bound in the array of where to start the computation */
                   PROBLEMSIZE); /* upper bound in the array of the last element+1 to process */
    execTime += omp_get_wtime();
    printf("dot product sequential result: %lf, time taken %lf\n", res, execTime);
    fflush(stdout);

    // parallel version with tasks
    int nthreads = 4;
    execTime = -omp_get_wtime();
#pragma omp parallel
    {
#pragma omp single
        {
#pragma omp task
            {
                res = dproduct(a, b, 0, PROBLEMSIZE / nthreads);
            }
#pragma omp task
            {
                res = dproduct(a, b, PROBLEMSIZE / nthreads, PROBLEMSIZE / nthreads * 2);
            }
#pragma omp task
            {
                res = dproduct(a, b, PROBLEMSIZE / nthreads * 2, PROBLEMSIZE / nthreads * 3);
            }
#pragma omp task
            {
                res = dproduct(a, b, PROBLEMSIZE / nthreads * 3, PROBLEMSIZE);
            }
#pragma omp taskwait
        }
    }
    execTime += omp_get_wtime();
    printf("dot product parallel result: %lf, time taken %lf\n", res, execTime);
    fflush(stdout);

    // OMP Reduction version
    printf("dot product omp reduction result: %lf, time taken %lf\n", res, execTime);

    free(a);
    free(b);
}
