#include "common.h"
#include <pvm3.h>
#include <time.h>
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
void init_slave(){
    srand(myid);
	mytid = pvm_mytid();
	pvm_recv(-1, MSG_TIDS);
	pvm_upkint(&mrtid, 1,1);
	pvm_upkint(&myid, 1,1);
	pvm_upkint(&nproc, 1,1);
	int i;
	for(i=0;i<nproc;++i) pvm_upkint(&tids[i], 1,1);
}
void quit_slave(){
	pvm_exit();
}
int my_id(){
	return myid;
}
int peer_count(){
	return nproc;
}
void send(int id, msg *m){
	int tmp = my_id();
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&(m->type),	1,1);
	pvm_pkint(&(m->stmp),	1,1);
	pvm_pkint(&tmp,			1,1);
	pvm_pkint(&(m->data),	1,1);
	pvm_pkint(&(m->road),	1,1);
	pvm_send(tid(id), MSG_COMM);
}
int trecv(int usec, msg *m){
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = usec*1000;
	int e = pvm_trecv(-1, MSG_COMM, &t);
	if(e <= 0) return 0;
	pvm_upkint(&(m->type), 	1,1);
	pvm_upkint(&(m->stmp),	1,1);
	pvm_upkint(&(m->from), 	1,1);
	pvm_upkint(&(m->data), 	1,1);
	pvm_upkint(&(m->road), 	1,1);
	return 1;
}
