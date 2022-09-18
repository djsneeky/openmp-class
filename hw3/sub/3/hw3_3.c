#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>

#define idx(u, m, n, c) (u[n * c + m])

void initArray(int *a, int m, int n, int val);
void matrixMultiply(int *a, int *b, int *c, int r1, int c1, int r2, int c2);
void matrixDisplay(int *a, int row, int column);

int *newArrayMalloc(int n_row, int n_col, int val)
{
  int *a = (int *)malloc(n_row * n_col * sizeof(int));
  for (int i = 0; i < n_col; i++)
  {
    for (int j = 0; j < n_row; j++)
    {
      idx(a, i, j, n_col) = val;
    }
  }
  return a;
}

void initArray(int *a, int m, int n, int val)
{
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < n; j++)
    {
      idx(a, i, j, n) = val;
    }
  }
}

// r1 * c2
// i is row index for r1
// j is col index for c2
// i stays

// i inc gets row of a
// j inc gets col of b
// k inc moves down col of b

void matrixMultiply(int *a, int *b, int *c, int r1, int c1, int r2, int c2)
{
  assert(a != NULL);
  assert(b != NULL);
  assert(c != NULL);
  assert(c1 == r2);

  for (int i = 0; i < r1; ++i)
  {
    for (int j = 0; j < c2; ++j)
    {
      for (int k = 0; k < c1; ++k)
      {
        // *((c+i*c2) + j)) += *((a+i*c1) + k)) * *((b+k*c2) + j));
        idx(c, i, j, c2) += idx(a, i, k, c1) * idx(b, k, j, c2);
      }
    }
  }
}

// function to display the matrix
void matrixDisplay(int *a, int m, int n)
{
  printf("\nOutput Matrix: %d x %d\n", m, n);
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < n; j++)
    {
      printf("%d  ", idx(a, i, j, n));
      if (j == n - 1)
        printf("\n");
    }
  }
}

int main(int argc, char *argv[])
{
  // setup input col and rows
  // a
  const int r1 = 2;
  const int c1 = 3;
  // b
  const int r2 = c1;
  const int c2 = 2;
  // c
  const int r3 = c1;
  const int c3 = 1;
  // intialize arrays
  // int * a = newArray(r1,c1,1);
  // int * b = newArray(r2,c2,2);
  // int * c = newArray(r3,c3,3);
  // int * d1 = newArray(r1,c2,0);
  // int * d2 = newArray(r1,c3,0);
  int a[r1][c1];
  int b[r2][c2];
  int c[r3][c3];
  int d1[r1][c2];
  int d2[r1][c3];

  initArray((int *)a, r1, c1, 1);
  initArray((int *)b, r2, c2, 2);
  initArray((int *)c, r3, c3, 3);
  initArray((int *)d1, r1, c2, 0);
  initArray((int *)d2, r1, c3, 0);

  matrixDisplay((int *)a, r1, c1);
  matrixDisplay((int *)b, r2, c2);
  matrixDisplay((int *)c, r3, c3);

  matrixMultiply((int *)a, (int *)b, (int *)d1, r1, c1, r2, c2);
  matrixMultiply((int *)a, (int *)c, (int *)d2, r1, c1, r3, c3);

  matrixDisplay((int *)d1, r1, c2);
  matrixDisplay((int *)d2, r1, c3);

#pragma omp parallel sections
  {
#pragma omp section
    {
      matrixMultiply((int *)a, (int *)b, (int *)d1, r1, c1, r2, c2);
    }
#pragma omp section
    {
      matrixMultiply((int *)a, (int *)c, (int *)d2, r1, c1, r3, c3);
    }
  }

  matrixDisplay((int *)d1, r1, c2);
  matrixDisplay((int *)d2, r1, c3);
}
