#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <stdbool.h>

#define RODSIZE 100
#define STEPS 1000

float ping[RODSIZE];
float pong[RODSIZE];

float *serial_rod(float *ping, float *pong)
{
    bool flag = true;
    // for 1000 steps of temperature change
    for (int i = 0; i < STEPS; i++)
    {
        // ping pong buffer
        if (flag)
        {
            for (int i = 1; i < RODSIZE - 1; i++)
            {
                pong[i] = (ping[i - 1] + ping[i] + ping[i + 1]) / 3;
            }
            flag = false;
        }
        else
        {
            for (int i = 1; i < RODSIZE - 1; i++)
            {
                ping[i] = (pong[i - 1] + pong[i] + pong[i + 1]) / 3;
            }
            flag = true;
        }
    }

    // check which has latest data
    // if flag = true, then ping has latest, else pong
    float *res_ptr;
    if (flag)
    {
        res_ptr = ping;
    }
    else
    {
        res_ptr = pong;
    }

    return res_ptr;
}

float *parallel_rod(float *ping, float *pong)
{
    bool flag = true;
    float *res_ptr;
    float *ptr1;
    float *ptr2;
    // for 1000 steps of temperature change
    for (int i = 0; i < STEPS; i++)
    {
        // ping pong buffer
        if (flag)
        {
            ptr1 = ping;
            ptr2 = pong;
        }
        else
        {
            ptr1 = pong;
            ptr2 = ping;
        }
#pragma omp parallel for
        for (int i = 1; i < RODSIZE - 1; i++)
        {
            ptr2[i] = (ptr1[i - 1] + ptr1[i] + ptr1[i + 1]) / 3;
        }
        flag = !flag;
    }

    // check which has latest data
    // if flag = true, then ping has latest, else pong
    float *res_ptr;
    if (flag)
    {
        res_ptr = ping;
    }
    else
    {
        res_ptr = pong;
    }

    return res_ptr;
}

int main()
{
    float *res_ptr;

    // intialize the rod
    ping[0] = 0.0f;
    for (int i = 1; i < RODSIZE - 1; i++)
    {
        ping[i] = 100.0f;
    }
    ping[RODSIZE - 1] = 0.0f;

    // SERIAL
    // res_ptr = serial_rod(ping, pong);

    // PARALLEL
    res_ptr = parallel_rod(ping, pong);

    // print the result
    for (int i = 0; i < RODSIZE; i++)
    {
        printf("%.6f ", res_ptr[i]);
    }

    return 0;
}
