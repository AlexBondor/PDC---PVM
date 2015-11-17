#include <stdio.h>
#include "pvm3.h"

#define TAG 100

static int foundit = 0;
static int my_match(mid, tid, code)
int mid;
int tid;
int code;
{
	int t, c, cc;
	if ((cc = pvm_bufinfo(mid, (int*)0, &c, &t)) < 0)
	{
		return cc;
	}

	if ((tid == -1 || tid == t) && (code == -1 || code == c))
	{
		foundit = 1;
	}

	return 0;
}

int probe(src, code)
{
	int (*omatch)();
	int cc;

	omatch = pvm_recvf(my_match);

	foundit = 0;

	if ((cc = pvm_nrecv(src, code)) < 0)
	{
		return cc;
	}

	pvm_recvf(omatch);
	return foundit;
}

void main()
{
	int infos[2];
	int info, i;
	int tids[2];
	int tid;

	printf("[master] Successfuly started\n");

	info = pvm_spawn("slave-message-criterion", (char**)0, 0, "", 2, tids);

	printf("[master] Spawned %d slaves\n", info);

	printf("[master] Tid 0: %d\n", tids[0]);
	printf("[master] Tid 1: %d\n", tids[1]);
	
	pvm_recvf(my_match);
	
	pvm_recv(tids[0], 1);
    	pvm_upkint(&i, 1, 1);
	
	printf("[master] Got %d from %d\n", i, tids[0]);

	printf("[master] Execution finished\n");

	pvm_exit();
	exit(0);
}
