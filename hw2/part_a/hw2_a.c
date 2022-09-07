#include <stdio.h>
#include <unistd.h>
#include <omp.h>

int main () {
    /* Serial Region  (master thread)                                */
    /* Parameters of the Application                                 */
    int len=30;
    char name[30];

    /* OpenMP Parameters */
    int id, nthreads, ncores;

    // get number of cores
    ncores = omp_get_num_procs();
    printf("Number of cores: %d\n", ncores);

    /* Master thread obtains information about itself and its environment. */
    nthreads = omp_get_num_threads();       /* get number of threads */
    id = omp_get_thread_num();              /* get thread            */
    gethostname(name,len);                  /* get run-host name     */
    printf("Master Thread:     Runhost:%s   Thread:%d of %d threads\n", name,id,nthreads);

    /* Open parallel region. */
    /* Each thread obtains information about itself and its environment. */
    #pragma omp parallel private(name,id,nthreads)
    {
        #pragma omp single
        {
            nthreads = omp_get_num_threads();      /* get number of threads */
            id = omp_get_thread_num();             /* get thread            */
            gethostname(name,len);                 /* get run-host name     */
            printf("Single Thread:   Runhost:%s   Thread:%d of %d threads\n", name,id,nthreads);
        }
        
    }
    /* Close parallel region. */

    /* Serial Region  (master thread)                                */
    printf("Master Thread:     Runhost:%s   Thread:%d of %d thread\n", name,id,nthreads);
    return 0;

    return 0;
}
