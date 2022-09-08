#include <stdio.h>
#include <unistd.h>
#include <omp.h>

int main () {
    /* OpenMP Parameters */
    int nthreads, ncores;

    // get number of cores
    ncores = omp_get_num_procs();
    printf("SEQUENTIAL: Number of cores: %d\n", ncores);

    // get number of threads
    nthreads = omp_get_max_threads();
    printf("SEQUENTIAL: Number of threads: %d\n", nthreads);

    /* Open parallel region. */
    #pragma omp parallel
    {
        printf("PARALLEL: Other Thread:%d\n", omp_get_thread_num());

        #pragma omp master
        printf("PARALLEL: Master Thread:%d\n", omp_get_thread_num());

        #pragma omp single
        printf("PARALLEL: Single Thread: %d\n", omp_get_thread_num());
    }
    /* Close parallel region. */

    return 0;
}
