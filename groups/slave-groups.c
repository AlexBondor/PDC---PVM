#include <stdio.h>
#include "pvm3.h"

#define NPROC 10
#define K 10

void main()
{
	printf("[slave] My tid=%x\n", pvm_mytid());

	int ptid = pvm_parent();
	int instnum;
	int M = 0;
	int data[K];
	int i;

	instnum = pvm_joingroup("my_group");

	printf("[slave] Successfully joined group. (instnum = %d)\n", instnum);
	
	pvm_barrier("my_group", NPROC + 1);
	
	pvm_recv(-1, 2);
	pvm_upkint(&M, 1, 1);

	for(i = 0; i < K; i++)
	{
		data[i] = i + M * instnum; 
	}
	
	pvm_barrier("my_group", NPROC + 1);
	pvm_reduce(PvmSum, data, 10, PVM_INT, 2, "my_group", pvm_getinst("my_group", ptid));
	pvm_barrier("my_group", NPROC + 1);

	exit(0);
}
