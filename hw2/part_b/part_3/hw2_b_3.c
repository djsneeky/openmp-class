#include <stdio.h>
#include <stdlib.h>
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
    int res;
    start = omp_get_wtime();
    #pragma omp parallel for reduction(+:res)
    for (int i = 0; i < 1000000; i++) {
        res += a[i];
    }
    end = omp_get_wtime();

    printf("Reduction Sum: %d\tElapsed seconds: %f", res, end-start);

    // free vars
    free(a);

    return 0;
}
