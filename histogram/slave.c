#include <stdio.h>
#include "pvm3.h"

#define SIZE 500

void main()
{
    printf("[slave] My tid=%x\n", pvm_mytid());

    int ptid = pvm_parent();
    int histogram[256], data[SIZE], left, right, value, i;
    char* fileName;

    for (i = 0; i < 256; i++)
    {
        histogram[i] = 0;
    }

    pvm_recv(ptid, 1);
    pvm_upkint(data, 500, 1);

    printf("[slave] I got the histogram from master\n");

    left = data[SIZE - 2];
    right = data[SIZE - 1];

    int index = 0;	

    for (i = left; i < right; i++)
    {
	histogram[data[i]] ++;
    }


    printf("[slave] Sending the histogram back to master\n");

    pvm_initsend(PvmDataDefault);
    pvm_pkint(histogram, 256, 1);
    pvm_send(ptid, 1);

    pvm_exit();
    exit(0);
}

