#include <stdio.h>
#include "pvm3.h"

void main()
{
	int ptid = pvm_parent();
	int i = 1;

	pvm_initsend(PvmDataDefault);
	pvm_pkint(&i, 1, 1);	
    	pvm_send(ptid, 1);

    	pvm_exit();
	exit(0);
}
