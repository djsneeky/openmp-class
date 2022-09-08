#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

int* newArray(int n) {
    int* a = (int*) malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        a[i] = 1;
    }
    return a;
}

int main () {
    /* Serial Region  (master thread)                                */
    /* Parameters of the Application                                 */
    int len=30;
    char name[30];

    /* OpenMP Parameters */
    int id, nthreads, ncores;

    // set number of threads to number of processors
    ncores = omp_get_num_procs();
    omp_set_num_threads(ncores);

    // sequential reduction
    int n = 1000000;
    int* a = newArray(n);
    int seq_sum = 0;
    double start, end;
    start = omp_get_wtime();
    for (int i = 0; i < n; i++) {
        seq_sum = seq_sum + a[i];
    }
    end = omp_get_wtime();
    printf("Sequential Reduction Sum: %d\tElapsed seconds: %f", seq_sum, end-start);

    // free vars
    free(a);

    return 0;
}
