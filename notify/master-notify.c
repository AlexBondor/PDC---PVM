#include <stdio.h>
#include "pvm3.h"

#define TAG 100

void main()
{
	int infos[2];
	int info, i;
	int tids[2];
	int tid;

	printf("[master] Successfuly started\n");

	info = pvm_spawn("slave-notify", (char**)0, 0, "", 2, tids);

	printf("[master] Spawned %d slaves\n", info);

	printf("[master] Tid 0: %d\n", tids[0]);
	printf("[master] Tid 1: %d\n", tids[1]);

	pvm_notify(PvmTaskExit, TAG, 2, tids);
	
	pvm_recv(-1, TAG);

	printf("[master] Execution finished\n");

	pvm_exit();
	exit(0);
}
