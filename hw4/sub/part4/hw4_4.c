#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Q
{
  int *q;
  int pos;
  int size;
} Q;

struct Q *initQ(int n)
{
  int i;
  struct Q *newQ = (struct Q *)malloc(sizeof(Q));
  newQ->q = (int *)malloc(sizeof(int) * n);
  newQ->pos = -1;
  newQ->size = n - 1;
  return newQ;
}

void putWork(struct Q *workQ)
{
  if (workQ->pos < (workQ->size))
  {
    workQ->pos++;
    workQ->q[workQ->pos] = (int)(rand() % 2 * (workQ->pos / 1000));
  }
  else
    printf("ERROR: attempt to add Q element%d\n", workQ->pos + 1);
}

int getWork(struct Q *workQ)
{
  if (workQ->pos > -1)
  {
    int w = workQ->q[workQ->pos];
    workQ->pos--;
    return w;
  }
  else
    printf("ERROR: attempt to get work from empty Q%d\n", workQ->pos);
}

void doWork(int t)
{
  usleep(t * 1000);
}

int main(int argc, char *argv[])
{
  int n = 1000;
  double start, end;

  struct Q *work_q = initQ(n);

  for (int i = 0; i < n; i++)
  {
    putWork(work_q);
  }

  // sequential work
  start = omp_get_wtime();
  for (int i = 0; i < n; i++)
  {
    doWork(getWork(work_q));
  }
  end = omp_get_wtime();
  printf("Sequential elapsed seconds: %lf\n", end - start);

  // go back to end of queue
  work_q->pos = work_q->size;

  // parallel work
  start = omp_get_wtime();
#pragma omp parallel for
  {
    for (int i = 0; i < n; i++)
    {
#pragma omp critical
      int w = getWork(work_q);
      doWork(w);
    }
  }
  end = omp_get_wtime();
  printf("Parallel elapsed seconds: %lf\n", end - start);
}