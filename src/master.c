#include "def.h"

main()
{
	int mytid;
	int tids[SLAVENUM];		/* slave task ids */
	char slave_name[NAMESIZE];
	int nproc, i, who;

	mytid=pvm_mytid();
	printf("Master TID is %i \n", mytid);

	nproc=pvm_spawn(SLAVENAME, NULL, PvmTaskDefault, "", SLAVENUM, tids);
	printf("Spawned %i slaves \n", nproc);

	for( i=0 ; i<nproc ; i++ ) //send to each slave
	{
		pvm_initsend(PvmDataDefault);	//initialize sending
		pvm_pkint(&mytid, 	1, 1);		//insert 
		pvm_pkint(&i, 		1, 1);
	   	pvm_send(tids[i], MSG_MSTR);	//send
		printf("Sent message to tid=%i \n", tids[i]);
	}

	for( i=0 ; i<nproc ; i++ )
	{
		pvm_recv( -1, MSG_SLV );
		pvm_upkint(&who, 1, 1 );
		printf("%d: responded\n",who);
	}

	pvm_exit();
}
