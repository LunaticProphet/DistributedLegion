#include "def.h"
#include <stdio.h>

int main()
{
	int mytid = pvm_mytid();	//my tid
	int mrtid = 0;				//master tid
	int mymid = 0;				//my id from master

		
	pvm_recv( -1, MSG_MSTR );
	pvm_upkint(&mrtid, 1, 1 );
	pvm_upkint(&mymid, 1, 1 );

	pvm_initsend(PvmDataDefault);
	pvm_pkint(&mymid, 1, 1);
	pvm_send(mrtid, MSG_SLV);

	pvm_exit();
}
