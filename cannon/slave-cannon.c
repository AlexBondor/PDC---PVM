#include <stdio.h>
#include <stdlib.h>
#include "pvm3.h"

void main()
{
	int left, right, top, bottom, temp, block_size;
	int ptid;
	int i, j;
	int **matrixA;
	int **matrixB;
	int **result;

	ptid = pvm_parent();

	pvm_recv(ptid, 0);
	pvm_upkint(&left, 1, 1);
	pvm_upkint(&right, 1, 1);
	pvm_upkint(&top, 1, 1);
	pvm_upkint(&bottom, 1, 1);

	/* Allocate space */ 
	matrixA = (int**)malloc(block_size * sizeof(int*));
	matrixB = (int**)malloc(block_size * sizeof(int*));;
	result = (int**)malloc(block_size * sizeof(int*));

	for (i = 0; i < block_size; i++)
	{
		matrixA[i] = (int *)malloc(block_size * sizeof(int));
		matrixB[i] = (int *)malloc(block_size * sizeof(int));
		result[i] = (int *)malloc(block_size * sizeof(int));

	}

	pvm_recv(ptid, 1);
	pvm_upkint(&block_size, 1, 1);
	pvm_upkint(&temp, 1, 1);
		
	for (i = 0; i < block_size; i++)
	{
		pvm_upkint(&j, 1, 1);
	}

	pvm_recv(ptid, 2);
	for (i = 0; i < block_size; i++)
	{
		pvm_upkint(matrixB[i], block_size, 1);
	}
	sleep(1);

	pvm_initsend(PvmDataDefault);
	pvm_pkint(&matrixA[0][0], 1, 1);
	pvm_send(ptid, 1);

	exit(0);
}
