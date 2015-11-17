#include <stdio.h>
#include "pvm3.h"

void main(int argc, char *argv[])
{
	printf("[master] My tid=%x\n", pvm_mytid());

	int n;
	int i, instnum, size;
	int tids[32];
	int M = 2;
	int data[10];
	int nproc;

	if (argc == 1)
	{
		printf("[master] Please supply number of slaves as argument.\n");
		exit(1);
	}	
	
	nproc = atoi(argv[1]);	

	printf("[master] Spawning %d slave processes\n", nproc);

	instnum = pvm_joingroup("my_group");	

	if (instnum == 0)
	{
		printf("[master] Successfully joined group. (instnum = %d)\n", instnum);

		/* Start up slave tasks */
		n = pvm_spawn("slave-groups", (char**)0, 0, "", nproc, tids);
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
		/* Wait for slaves to join the group */
		pvm_barrier("my_group", nproc + 1);
	}
	else
	{
		printf("[master] Failed to join group. (instnum = %d)\n", instnum);
	}	

	size = pvm_gsize("my_group");
	printf("[master] The size of the group is %d.\n", size);

	/* everyone broadcast their gids and tids */
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&M, 1, 1);
	pvm_bcast("my_group", 1);

	pvm_barrier("my_group", nproc + 1);
	pvm_reduce(PvmSum, data, 10, PVM_INT, 2, "my_group", instnum);
	pvm_barrier("my_group", nproc + 1);

	for(i = 0; i < 10; i ++)
	{	
		printf("%d ", data[i]);	
	}
	
	exit(0);
}

