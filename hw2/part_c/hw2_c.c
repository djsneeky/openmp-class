#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main () {
    // constants
    const int n = 10000000;
    double start, end;
    float res = 0;

    // forward sequential
    start = omp_get_wtime();
    for (int i = 1; i < n; i++) {
        res += 1.0f / (float) i;
    }
    end = omp_get_wtime();
    printf("Sequential Forward Sum: %f\tElapsed seconds: %f\n", res, end-start);

    res = 0;
    // reverse sequential
    start = omp_get_wtime();
    for (int i = n; i > 0; i--) {
        res += 1.0f / (float) i;
    }
    end = omp_get_wtime();
    printf("Sequential Reverse Sum: %f\tElapsed seconds: %f\n", res, end-start);

    res = 0;
    // forward reduction
    start = omp_get_wtime();
    #pragma omp parallel for reduction(+:res)
    for (int i = 1; i < n; i++) {
        res += 1.0f / (float) i;
    }
    end = omp_get_wtime();
    printf("Forward reduction Sum: %f\tElapsed seconds: %f\n", res, end-start);

    return 0;
}
