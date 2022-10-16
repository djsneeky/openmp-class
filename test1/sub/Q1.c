#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define PROBLEMSIZE 10000000
#define NUMTHREADS 4

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

    float result[NUMTHREADS];
    for (int i = 0; i < NUMTHREADS; i++)
    {
        result[i] = 0;
    }

    // sequential execution to check the answer
    execTime = -omp_get_wtime();
    res = dproduct(a, b, 0,      /* lower bound in the array of where to start the computation */
                   PROBLEMSIZE); /* upper bound in the array of the last element+1 to process */
    execTime += omp_get_wtime();
    printf("dot product sequential result: %lf, time taken %lf\n", res, execTime);
    fflush(stdout);

    // parallel version with tasks
    execTime = -omp_get_wtime();
#pragma omp parallel
    {
#pragma omp single
        {
#pragma omp task
            {
                result[0] = dproduct(a, b, 0, PROBLEMSIZE / NUMTHREADS);
            }
#pragma omp task
            {
                result[1] = dproduct(a, b, PROBLEMSIZE / NUMTHREADS, PROBLEMSIZE / NUMTHREADS * 2);
            }
#pragma omp task
            {
                result[2] = dproduct(a, b, PROBLEMSIZE / NUMTHREADS * 2, PROBLEMSIZE / NUMTHREADS * 3);
            }
#pragma omp task
            {
                result[3] = dproduct(a, b, PROBLEMSIZE / NUMTHREADS * 3, PROBLEMSIZE);
            }
#pragma omp taskwait
        }
    }
    res = result[0] + result[1] + result[2] + result[3];
    execTime += omp_get_wtime();
    printf("dot product parallel result: %lf, time taken %lf\n", res, execTime);
    fflush(stdout);

    // OMP Reduction version
    execTime = -omp_get_wtime();
#pragma omp parallel for reduction(+:res)
    for (int i = 0; i < PROBLEMSIZE; i++)
    {
        res += (a[i] * b[i]);
    }
    execTime += omp_get_wtime();
    printf("dot product omp reduction result: %lf, time taken %lf\n", res, execTime);

    free(a);
    free(b);
}
