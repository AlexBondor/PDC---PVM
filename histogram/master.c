#include <stdio.h>
#include "pvm3.h"

#define SIZE 500

void main(int argc, char *argv[])
{
    printf("[master] My tid=%x\n", pvm_mytid());

    int n, nhost, narch, nproc;
    int histogram[256], temp[256], i, j, data[SIZE];
    int tids[32];
    int length = 0, offset, value;
    struct pvmhostinfo *hostp;
    
    /* Set number of slaves to start */
    pvm_config(&nhost, &narch, &hostp);
    nproc = atoi(argv[2]);
    if (nproc > 32) 
    {
        nproc = 32;
    }

    printf("[master] Spawning %d slave processes\n", nproc);

    /* Start up slave tasks */
    n = pvm_spawn("slave", (char**)0, 0, "", nproc, tids);
    if (n < nproc)
    {
       printf("[master] Trouble spawning slaves. Aborting. Error codes are:\n");
       for (i = n; i < nproc; i++ ) 
       {
          printf("[master] process: %d tid: %d\n", i, tids[i]);
       }
       for(i = 0; i < n; i++)
       {
          pvm_kill(tids[i]);
       }
       pvm_exit();
       exit(1);
    }

    printf("[master] Successfully started %d processes\n", n);

    /* Read input file */
    FILE *image;
    image = fopen(argv[1], "r");

    if (image == NULL)
    {
        fprintf(stderr, "[master] Can't open input file %s\n", argv[1]);
	exit(1);
    }
    else
    {
	printf("[master] Successfully read image\n");

        while(fscanf(image, "%d", &data[length]) > 0)
        {
	    length ++;
        }
    }   

    printf("[master] Image has %d pixels\n", length);

    offset = length / n;

    for (i = 0; i < 256; i++)
    {
        histogram[i] = temp[i] = 0;
    }

    int left, right;
    for (i = 0; i < n; i++)
    {
	left = i * offset;
	right = i == n - 1 ? length : left + offset;
	data[SIZE - 2] = left;
	data[SIZE - 1] = right;

        printf("[master] Sending [from %d to %d] to slave %d\n", left, right, i);
	for (j = left; j < right; j++)
	{
	    printf("\t data[%d] = %d\n", j, data[j]);
	}
        pvm_initsend(PvmDataDefault);
        pvm_pkint(data, SIZE, 1);
        pvm_send(tids[i], 1);

        printf("[master] Waiting for slave %d to do my work\n", i);
    }
 
    for (i = 0; i < n; i++)
    {
	pvm_recv(tids[i], 1);
        pvm_upkint(temp, 256, 1);
       
        printf("[master] Got histogram from %d:\n", i);
	
	for (j = 0; j < 256; j++)
	{
	    histogram[j] += temp[j];
	}
    }

    printf("[master] Histogram values:\n");

    for (j = 0; j < 256; j++)
    {
        printf("%d ", histogram[j]);
    }
	
    pvm_exit();
    exit(0);
}

