#include "def.h"
#include <stdio.h>

int main()
{
	int tid = pvm_mytid();
	int tids[16];
	char name[] = "grupa";
	int i, res=0;

	pvm_spawn("slave",0,0,".",SLAVENUM,tids);

	pvm_initsend( PvmDataRaw );
	pvm_pkbyte(name,6, 1);
	pvm_mcast(tids,SLAVENUM,1);

	pvm_joingroup( name );	
	printf("Master: Czekam na barierze\n");

	if (pvm_barrier(name, SLAVENUM+1) <0)
		printf("Nastapil blad podczas czekania na barierze\n");
	printf("Master: Jestem za Bariera \n");

	pvm_initsend( PvmDataRaw );
	pvm_pkint( &tid, 1,1);
	pvm_bcast(name, 2);

	for (i=0;i<SLAVENUM;i++)
	{
		pvm_recv( -1, 3 );
		pvm_upkint( &res, 1, 1);
		printf("Master: %d-ty komunikat. Nadawca to t%x\n",i,res);
	}

	printf("Master: Odebralem wiadomosc od wszystkich\n");


	pvm_exit();

}
