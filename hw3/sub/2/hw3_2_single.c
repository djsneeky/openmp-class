#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
int main (int argc, char *argv[]) {
  int count = 0;
  printf("count before parallel: %d, address: %p\n",count, (void*) &count); 
  #pragma omp parallel
  {
    #pragma omp single
    {
      count++;
      printf("count during parallel: %d, address: %p\n",count, (void*) &count); 
    }
  }
  printf("count after parallel: %d, address: %p\n",count, (void*) &count); 
}

