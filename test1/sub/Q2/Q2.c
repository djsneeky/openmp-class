#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define MEMSIZE 100000
#define DATASIZE 10
#define NUMTHREADS 4

typedef struct node
{
    int valIdx;
    int left;
    int right;
} nodeT;

int memIdx = 0;
nodeT *memory;
int data[DATASIZE];

void initData()
{
    for (int i = 0; i < 10; i++)
    {
        data[i] = 0;
    }
}

void init()
{
    memIdx = 0;
    memory = (nodeT *)malloc(sizeof(nodeT) * MEMSIZE);
    if (memory == NULL)
    {
        printf("invalid memory allocation\n");
        fflush(stdout);
    }
    initData();
}

int myMalloc()
{
    if (memIdx < MEMSIZE)
    {
        return memIdx++;
    }
    return -1;
}

int build(int count)
{
    int me;
    if ((me = myMalloc()) < 0)
    {
        return -1;
    }

    count = ++count % DATASIZE;
    memory[me].valIdx = count;
    memory[me].left = build(count);
    memory[me].right = build(count);
    return me;
}

int main()
{

    init();
    build(-1);

    int sum = 0;

    // SEQUENTIAL SECTION
    for (int i = 0; i < MEMSIZE; i++)
    {
        data[memory[i].valIdx]++;
    }

    // print result of data sequential
    printf("Sequential data results: [ ");
    for (int i = 0; i < DATASIZE; i++)
    {
        printf("%d ", data[i]);
        sum += data[i];
    }
    printf("]\r\n");
    printf("Sequential sum: %d", sum);

    // PARALLEL SECTION
    initData();
    sum = 0;
    // array of locks
    omp_lock_t lock[DATASIZE];
    for (int i = 0; i < DATASIZE; i++)
    {
        omp_init_lock(&(lock[i]));
    }

    // lock only the index we care about
    #pragma omp parallel for
    for (int i = 0; i < MEMSIZE; i++)
    {
        omp_set_lock(&(lock[memory[i].valIdx]));
        data[memory[i].valIdx]++;
        omp_unset_lock(&(lock[memory[i].valIdx]));
    }

    // print result of data parallel
    printf("Sequential data results: [ ");
    for (int i = 0; i < DATASIZE; i++)
    {
        printf("%d ", data[i]);
        sum += data[i];
    }
    printf("]\r\n");
    printf("Sequential sum: %d", sum);
}
