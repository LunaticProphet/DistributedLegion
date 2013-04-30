#include "def.h"
#include <stdio.h>

int main()
{
	char s[10];
	int ptid = 0;
	int tid = pvm_mytid();
	printf("Slave: Czekam na wiadomosci\n");
	pvm_recv(-1,1);
	pvm_upkbyte( s, 6, 1);
	printf("Slave: Nazwa grupy: %s\n", s );
	pvm_joingroup( s );
	if (pvm_barrier(s, SLAVENUM+1) <0)
		printf("Slave: blad podczas bariery\n");
	printf("Slave: Jestem za bariera\n");
	pvm_recv( pvm_parent(), 2 );
	pvm_upkint( &ptid, 1, 1);
	pvm_initsend( PvmDataRaw );
	pvm_pkint( &tid, 1, 1);
	pvm_send( pvm_parent(), 3);
	pvm_exit();	
}
