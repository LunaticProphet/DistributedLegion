#include "def.h"

int main(int argc, char* argv[])
{
	int mytid;
	int tids[SLAVENUM];		/* slave task ids */
	int nproc, i, j, who;

	if(argc<2){
		printf("Usage: ./master SLAVE_PATH\n");
		return 1;
	}

	mytid=pvm_mytid();
	printf("Master TID is %i \n", mytid);

	nproc=pvm_spawn(argv[1], NULL, PvmTaskDefault, "", SLAVENUM, tids);
	printf("Spawned %i slaves \n", nproc);




	for( i=0 ; i<nproc ; i++ ) //send to each slave
	{
		pvm_initsend(PvmDataDefault);	//initialize sending
		pvm_pkint(&mytid,	1,1);
		pvm_pkint(&i, 1,1);
		pvm_pkint(&nproc, 	1,1);
		for(j=0;j<nproc;++j)
			pvm_pkint(&tids[j], 1,1);
	   	pvm_send(tids[i], MSG_TIDS);
		printf("P%i has tid=%i \n", i, tids[i]);
	}

	char buf[1024];
	int finishes = 0;

	while(1){
		pvm_recv(-1, MSG_PRNT);
		pvm_upkint(&who, 1,1);
		pvm_upkstr(buf);
		printf("P%i: %s\n", who,buf);
		if(!strcmp(buf, "finished")){
			if(++finishes==nproc){
				printf("Work done!\n");
				pvm_exit();
				return 0;
			}
		}
	}
	return 0;
}

