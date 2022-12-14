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
    // set number of threads to number of processors
    int ncores = omp_get_num_procs();
    omp_set_num_threads(ncores);

    // test reduction
    int n = 1000000;
    int* a = newArray(n);
    double start, end;
    int nt = omp_get_max_threads();
    int res[nt*8];
    start = omp_get_wtime();
    #pragma omp parallel for
    for (int i=0; i < 1000000; i++) {
        res[omp_get_thread_num()*8] += a[i];
    }
    end = omp_get_wtime();

    printf("Test Reduction Elapsed seconds: %f", end-start);

    // free vars
    free(a);

    return 0;
}
