#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>

int num_nodes_par = 0;

struct node
{
    double val;
    int level;
    struct node *l;
    struct node *r;
} nodeT;

#define MAXLEVEL 18

struct node *build(int level)
{
    if (level < MAXLEVEL)
    {
        struct node *p = (struct node *)malloc(sizeof(nodeT));
        p->val = (double)rand() / (RAND_MAX);
        p->level = level;
        p->l = build(level + 1);
        p->r = build(level + 1);
        return p;
    }
    else
    {
        return NULL;
    }
}

int traverse(struct node *p)
{
    int l_count = 0;
    int r_count = 0;
    int val = 0;

    if (p == NULL)
    {
        return 0;
    }

    l_count += traverse(p->l);
    r_count += traverse(p->r);

    if (p->val < 0.5)
    {
        val++;
    }

    return (val + l_count + r_count);
}

struct node *build_par(int level)
{
    if (level < MAXLEVEL)
    {
        struct node *p = (struct node *)malloc(sizeof(nodeT));
        num_nodes_par++;
        p->level = level;
        p->val = (double)rand() / (RAND_MAX);

        // limit task spawning
        if (pow(2, p->level) > omp_get_num_procs())
        {
            p->l = build(level + 1);
        }
        else
        {
#pragma omp task
            p->l = build_par(level + 1);
        }

        if (pow(2, p->level) > omp_get_num_procs())
        {
            p->r = build(level + 1);
        }
        else
        {
#pragma omp task
            p->r = build_par(level + 1);
        }

        return p;
    }
    else
    {
        return NULL;
    }
}

int traverse_par(struct node *p)
{
    int l_count = 0;
    int r_count = 0;
    int val = 0;

    if (p == NULL)
    {
        return 0;
    }

    // limit task spawning
    if (pow(2, p->level) > omp_get_num_procs())
    {
        l_count += traverse(p->l);
    }
    else
    {
#pragma omp task shared(l_count)
        l_count += traverse_par(p->l);
    }
    // limit task spawning
    if (pow(2, p->level) > omp_get_num_procs())
    {
        r_count += traverse(p->r);
    }
    else
    {
#pragma omp task shared(r_count)
        r_count += traverse_par(p->r);
    }

    #pragma omp taskwait
    if (val < 0.5)
    {
        val++;
    }

    return (val + l_count + r_count);
}

int main()
{
    struct node *p_tree;
    struct node *s_tree;
    int s_count = 0;
    int p_count = 0;
    double start, end;

    // serial
    start = omp_get_wtime();
    s_tree = build(0);
    s_count = traverse(s_tree);
    end = omp_get_wtime();
    printf("Sequential elapsed seconds: %lf\n", end - start);
    printf("Sequential count: %d\n", s_count);


    // build the tree parallelly
    start = omp_get_wtime();
#pragma omp parallel
    {
#pragma omp single
        {
            p_tree = build_par(0);
        }
    }

    // traverse the tree parallelly
#pragma omp parallel
    {
#pragma omp single
        {
            p_count = traverse_par(p_tree);
        }
    }
    end = omp_get_wtime();
    printf("Parallel elapsed seconds: %lf\n", end - start);
    printf("Parallel count: %d\n", p_count);
}
