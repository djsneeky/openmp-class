#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char *argv[])
{
  int n = 1000;
  int total1 = 0;
  int total2 = 0;

#pragma omp parallel
  {
    int count = 0;
    for (int i = 0; i < n; i++)
    {
      count++;
#pragma omp critical
      total1++;
    }
  }
#pragma opm parallel for
  for (int i = 0; i < n; i++)
  {
#pragma omp critical
    total2++;
  }
  printf("Total par iterations: %d\n", total1);
  printf("Total parfor iterations: %d", total2);
}
