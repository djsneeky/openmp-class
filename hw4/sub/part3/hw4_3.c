#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void doWork(int t)
{
  usleep(t * 1000);
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
  int n = 1000;
  int *w = initWork(n);

  double start, end;

  // sequential
  start = omp_get_wtime();
  for (int i = 0; i < n; i++)
  {
    doWork(w[i]);
  }
  end = omp_get_wtime();
  printf("Sequential time: %lf\n", end - start);

  // parallel - assumes 4 threads
  start = omp_get_wtime();
#pragma omp parallel sections
  {
#pragma omp section
    {
      for (int i = 0; i < n / 4; i++)
      {
        doWork(w[i]);
      }
    }
#pragma omp section
    {
      for (int i = n / 4; i < n / 4 * 2; i++)
      {
        doWork(w[i]);
      }
    }
#pragma omp section
    {
      for (int i = n / 4 * 2; i < n / 4 * 3; i++)
      {
        doWork(w[i]);
      }
    }
#pragma omp section
    {
      for (int i = n / 4 * 3; i < n; i++)
      {
        doWork(w[i]);
      }
    }
  }
  end = omp_get_wtime();
  printf("Parallel sections time: %lf\n", end - start);
}
