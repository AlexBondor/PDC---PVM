#include <stdio.h>
#include <stdlib.h>
#include "pvm3.h"

#define MAX_BLOCK_SIZE 4

void main(int argc, char *argv[])
{
	/* Check usage */
	if (argc != 4)
	{
		printf("Usage: ./master-cannon (int)matrix_size (path)matrix-a (path)matrix-b\n");
		exit(1);
	}

	FILE *inputA, *inputB;
	int **matrixA, **matrixB;
	int ***matrixA_blocks, ***matrixB_blocks;

	int matrix_size, n, nproc;
	int block_size;
	int i, j;
	int *tids;

	/* Successfully started master */
	printf("[master] My tid=%x\n", pvm_mytid());

	matrix_size = atoi(argv[1]);
	printf("[master] Matrix size is: %d x %d\n", matrix_size, matrix_size);
	
	/* Set the block size */
	if (matrix_size > MAX_BLOCK_SIZE)
	{
		if (matrix_size % MAX_BLOCK_SIZE == 0)
		{
			block_size = MAX_BLOCK_SIZE;
		}
		else
		{
			int divisor = matrix_size / 2;
			while (matrix_size % divisor != 0)
			{
				divisor--;
			}
			block_size = divisor == 1 ? matrix_size : divisor;
		}
	}
	else
	{
		block_size = matrix_size;
	}
	printf("[master] Computed block size is: %d\n", block_size);

	/* Square root of number of slaves */
	int temp = matrix_size / block_size;
	printf("[master] Size of slave processes ids matrix: %d\n", temp);	
	tids = (int*)malloc(temp * temp * sizeof(int));

	/* The matrix to store the slaves ids */
	int **my_slaves = (int**)malloc(temp * sizeof(int*));
	for (i = 0; i < temp; i++)
	{
		my_slaves[i] = (int*)malloc(temp * sizeof(int));
	}
	matrixA_blocks = (int***)malloc(temp * temp * sizeof(int**));
	matrixB_blocks = (int***)malloc(temp * temp * sizeof(int**));
	for (i = 0; i < temp * temp; i++)
	{
		matrixA_blocks[i] = (int**)malloc(block_size * sizeof(int*));
		matrixB_blocks[i] = (int**)malloc(block_size * sizeof(int*));
		for(j = 0; j < block_size; j++)
		{
			matrixA_blocks[i][j] = (int*)malloc(block_size * sizeof(int));
			matrixB_blocks[i][j] = (int*)malloc(block_size * sizeof(int));
		}
	}

	matrixA = (int**)malloc(matrix_size * sizeof(int*)); 
	matrixB = (int**)malloc(matrix_size * sizeof(int*));

	for (i = 0; i < matrix_size; i++)
	{
		matrixA[i] = (int*)malloc(matrix_size * sizeof(int));
		matrixB[i] = (int*)malloc(matrix_size * sizeof(int));
	}

	/* Read the input matrices */

	/* Read matrix A */
	inputA = fopen(argv[2], "r");
	if (inputA == NULL)
	{
		fprintf(stderr, "[master] Can't open input file %s\n", argv[2]);
		exit(1);
	}
	else
	{
		i = 0;
		j = 0;
		while(fscanf(inputA, "%d", &matrixA[i][j]) > 0)
		{
		    j ++;
		    if (j % matrix_size == 0)
		    {
		    	i ++;
		    	j = 0;
		    }
		}
	} 
	printf("[master] Successfully read %s\n", argv[2]);  

	/* Read matrix B */
	inputB = fopen(argv[3], "r");
	if (inputB == NULL)
	{
		fprintf(stderr, "[master] Can't open input file %s\n", argv[3]);
		exit(1);
	}
	else
	{
		i = 0;
		j = 0;
		while(fscanf(inputB, "%d", &matrixB[i][j]) > 0)
		{
		    j ++;
		    if (j % matrix_size == 0)
		    {
		    	i ++;
		    	j = 0;
		    }
		}
	}   
	printf("[master] Successfully read %s\n", argv[3]);

	/* Start up slave tasks */
	nproc = matrix_size / block_size * matrix_size / block_size;

	printf("[master] Spawning %d slave processes\n", nproc);

	n = pvm_spawn("slave-cannon", (char**)0, 0, "", nproc, tids);
	if (n < nproc)
	{
		printf("[master] Failed to spawn slave processes\n");
	}
	printf("[master] Successfully spawned %d slave processes\n", n);

	/* Initialize the matrix of slave ids */
	int proc_index = 0;
	for (i = 0; i < temp; i++)
	{				
		for (j = 0; j < temp; j++)
		{
			my_slaves[i][j] = tids[proc_index++];
		}
	}

	/* Assign the neighbours ids for each process */
	for (i = 0; i < temp; i++)
	{
		for (j = 0; j < temp; j++)
		{
			pvm_initsend(PvmDataDefault);		

			/* Left neighbour */
			int left = j == 0 ? my_slaves[i][temp - 1] : my_slaves[i][j - 1];
			pvm_pkint(&left, 1, 1);
			/* Right neighbour */
			int right = j == temp - 1 ? my_slaves[i][0] : my_slaves[i][j + 1];
			pvm_pkint(&right, 1, 1);
			/* Top neighbour */
			int top = i == 0 ? my_slaves[temp - 1][j] : my_slaves[i - 1][j];
			pvm_pkint(&top, 1, 1);
			/* Bottom neighbour */
			int bottom = i == temp - 1 ? my_slaves[0][j] : my_slaves[i + 1][j];
			pvm_pkint(&bottom, 1, 1);

			pvm_send(my_slaves[i][j], 0);
		}
	}
	printf("[master] Assigned and send neighbour ids\n");

	int k;
	int block_index = 0;
	int **block_tempA;
	int **block_tempB;	
		

	for (i = 0; i < matrix_size; i++)
	{
		for (j = 0; j < matrix_size; j++)
		{
			block_index = (i / block_size) * temp + j / block_size;
			matrixA_blocks[block_index][i % block_size][j % block_size] = matrixA[i][j];
			matrixB_blocks[block_index][i % block_size][j % block_size] = matrixB[i][j];
		}
	}

	int test[5];
	test[0] = -1;
	test[1] = -2;

	for (i = 0; i < temp; i++)
	{
		for (j = 0; j < temp; j++)
		{
			/* Do the initial left shift */
			block_index = i * temp + j;
			block_index = block_index + i < temp * (i + 1) ? block_index + i : block_index + i - temp;
			block_tempA = matrixA_blocks[block_index];

			pvm_initsend(PvmDataDefault);
			pvm_pkint(&block_size, 1, 1);
			pvm_pkint(&temp, 1, 1);	

			//for (k = 0; k < block_size; k ++)
			{
				pvm_pkint(&block_size, 1, 1);
			}
			pvm_send(my_slaves[i][j], 1);

			/* Do the initial top shift */
			block_index = i * temp + j;
			block_index =  (block_index + (temp * j)) % (temp * temp);
			block_tempB = matrixB_blocks[block_index];
	
			pvm_initsend(PvmDataDefault);
			for (k = 0; k < block_size; k ++)
			{
				pvm_pkint(block_tempB[k], block_size, 1);
			}
			pvm_send(my_slaves[i][j], 2);

		}
	}

	printf("[master] Waiting for slaves\n");

	int t;
	for (i = 0; i < temp; i++)
	{
		for (j = 0; j < temp; j++)
		{
			pvm_recv(-1, -1);
			pvm_upkint(&t, 1, 1);
			printf("Got %d from %d\n", t, i * temp + j);
		}
	}


	exit(0);	
}
