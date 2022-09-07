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

    /* Master thread obtains information about itself and its environment. */
    nthreads = omp_get_num_threads();       /* get number of threads */
    id = omp_get_thread_num();              /* get thread            */
    gethostname(name,len);                  /* get run-host name     */
    printf("Master Thread:     Runhost:%s   Thread:%d of %d threads\n", name,id,nthreads);

    // sequential reduction
    int n = 1000000;
    int* a = newArray(n);
    int seq_sum = 0;
    double start, end;
    start = omp_get_wtime();
    #pragma omp parallel for private(id,nthreads)
    for (int i = 0; i < n; i++) {
        #pragma omp critical
        seq_sum = seq_sum + a[i];
	nthreads = omp_get_num_threads();
	id = omp_get_thread_num();
	gethostname(name,len);
        printf("Parallel Thread:   Runhost:%s   Thread:%d of %d threads\n", name,id,nthreads);
    }
    end = omp_get_wtime();
    printf("Sequential Reduction Sum: %d\tElapsed seconds: %f", seq_sum, end-start);

    // free vars
    free(a);

    return 0;
}
