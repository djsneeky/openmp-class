#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <stdbool.h>

#define RODSIZE 100
#define STEPS 1000

float ping[RODSIZE];
float pong[RODSIZE];

int main()
{
    // intialize the rod
    ping[0] = 0.0f;
    for (int i = 1; i < RODSIZE-1; i++)
    {
        ping[i] = 100.0f;
    }
    ping[RODSIZE-1] = 0.0f;

    bool flag = true;
    // for 1000 steps of temperature change
    for (int i = 0; i < STEPS; i++)
    {
        // ping pong buffer
        if (flag)
        {
            for (int i = 1; i < RODSIZE-1; i++)
            {
                pong[i] = (ping[i-1] + ping[i] + ping[i+1]) / 3;
            }
            flag = false;
        }
        else
        {
            for (int i = 1; i < RODSIZE-1; i++)
            {
                ping[i] = (pong[i-1] + pong[i] + pong[i+1]) / 3;
            }
            flag = true;
        }
    }

    // check which has latest data
    // if flag = true, then ping has latest, else pong
    float * res_ptr;
    if (flag)
    {
        res_ptr = ping;
    }
    else
    {
        res_ptr = pong;
    }

    // print the result 
    for (int i = 0; i < RODSIZE; i++)
    {
        printf("%.6f ", res_ptr[i]);
    }

    return 0;
}
