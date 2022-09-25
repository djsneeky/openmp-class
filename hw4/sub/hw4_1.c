#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void doWork(int t)
{
  sleep(t);
}

int *initWork(int n)
{
  int i;
  double r;
  int *wA = (int *)malloc(sizeof(int) * n);
  for (i = 0; i < n; i++)
  {
    wA[i] = rand() % 2 * i / (n / 10);
  }
  return wA;
}

int main(int argc, char *argv[])
{
  int i;
  int n = 1000;
  int *w = initWork(n);
  printf("Initial sleep values:\n");
  for (i = 0; i < n; i += 50)
  {
    printf("w[%d] = %d\n", i, w[i]);
  }

  double start, end;

  // static scheduling
  start = omp_get_wtime();
#pragma omp parallel for schedule(static)
  for (i = 0; i < n; i += 50)
  {
    doWork(w[i]);
  }
  end = omp_get_wtime();
  printf("Static scheduling - elapsed seconds: %lf", end - start);

  // static scheduling with block size 50
  start = omp_get_wtime();
#pragma omp parallel for schedule(static, 50)
  for (i = 0; i < n; i++)
  {
    doWork(w[i]);
  }
  end = omp_get_wtime();
  printf("Static scheduling block size 50 elapsed seconds: %lf", end - start);

  // dynamic scheduling
  start = omp_get_wtime();
#pragma omp parallel for schedule(dynamic)
  for (i = 0; i < n; i++)
  {
    doWork(w[i]);
  }
  end = omp_get_wtime();
  printf("Dynamic scheduling elapsed seconds: %lf", end - start);

  // dynamic scheduling block size 50
  start = omp_get_wtime();
#pragma omp parallel for schedule(dynamic, 50)
  for (i = 0; i < n; i++)
  {
    doWork(w[i]);
  }
  end = omp_get_wtime();
  printf("Dynamic scheduling block size 50 elapsed seconds: %lf", end - start);

  // guided scheduling
  start = omp_get_wtime();
#pragma omp parallel for schedule(guided)
  for (i = 0; i < n; i++)
  {
    doWork(w[i]);
  }
  end = omp_get_wtime();
  printf("Guided scheduling elapsed seconds: %lf", end - start);

  free(w);
}
