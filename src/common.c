#include "common.h"
int tids[SLAVENUM];
int nproc;
int mytid;	// my TID
int myid;	// my position in TIDS
int mrtid;	// master TID


void lg(){
	pvm_initsend(PvmDataDefault);	//initialize sending
	pvm_pkint(&myid,	1,1);
	pvm_pkstr(buf);
	pvm_send(mrtid, MSG_PRNT);
}
int randn(int n){
	return rand()%n;
}
int tid(int id){
	return tids[id];
}
void sleepForMax(int n){
	usleep(randn(n*1000));
}
void init(){
    srand(myid);
	mytid = pvm_mytid();
	pvm_recv(-1, MSG_TIDS);
	pvm_upkint(&mrtid, 1,1);
	pvm_upkint(&myid, 1,1);
	pvm_upkint(&nproc, 1,1);
	int i;
	for(i=0;i<nproc;++i) pvm_upkint(&tids[i], 1,1);
}
int my_id(){
	return myid;
}
int peer_count(){
	return nproc;
}