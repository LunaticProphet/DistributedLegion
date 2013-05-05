#include "def.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

#define M_REQ 1
#define M_ACC 2
#define M_REL 3

char buf[1024];
int nproc;
int tids[SLAVENUM];
int timestamp = 0;
int prev_in_cs = -1;
int accepts = 0;

int mytid;	// my TID
int myid;	// my position in TIDS
int mrtid;	// master TID

typedef struct{
	int type;
	int stmp;
	int from;
	int prev;
} msg;

void lg(){
	pvm_initsend(PvmDataDefault);	//initialize sending
	pvm_pkint(&myid,	1,1);
	pvm_pkstr(buf);
	pvm_send(mrtid, MSG_PRNT);
}

///////////////////////////////////////////
msg tqueue[SLAVENUM];
void tqueue_add(msg *m){
	int i;
	for(i=0;i<SLAVENUM;++i) if(tqueue[i].from == -1){
		tqueue[i] = *m;
		break;
	}
}
void tqueue_del(int from){
	int i;
	for(i=0;i<SLAVENUM;++i) if(tqueue[i].from == from){
		tqueue[i].from = -1;
		break;
	}
}
msg* tqueue_top(){
	int i,best = -1;
	for(i=0;i<SLAVENUM;++i){
		if(tqueue[i].from != -1){
			if(best == -1 || tqueue[i].stmp < tqueue[best].stmp || (tqueue[i].stmp == tqueue[best].stmp && tqueue[i].from < tqueue[best].from)){
				best = i;
			}
		}
	}
	if(best == -1) return 0;
	return &tqueue[best];
}
///////////////////////////////////////////
int hang[SLAVENUM];
void hang_add(int i){
	hang[i] = 1;	
}
void hang_del(int i){
	hang[i] = 0;
}
int hang_contains(int i){
	return hang[i];
}
int hang_count(){
	int i,c=0;
	for(i=0;i<SLAVENUM;++i) if(hang_contains(i)) ++c;
	return c;
}
///////////////////////////////////////////

int tid(int id){
	return tids[id];
}
int randn(int n){
	return rand()%n;
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
	for(i=0;i<SLAVENUM;++i) tqueue[i].from = -1;
}
void send_with_ts(int id, msg *m, int ts){
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&(m->type),	1,1);
	pvm_pkint(&(ts),	1,1);
	pvm_pkint(&myid,		1,1);
	pvm_pkint(&(m->prev),	1,1);
	pvm_send(tid(id), MSG_COMM);
	sprintf(buf,"SND%i type=%i stmp=%i from=%i",id,m->type,ts,myid);
	lg();
}
void send(int id, msg *m){
	send_with_ts(id, m, ++timestamp);
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
	pvm_upkint(&(m->prev), 	1,1);
	if(m->stmp > timestamp) timestamp = m->stmp;
	sprintf(buf,"RECV type=%i stmp=%i from=%i",m->type,m->stmp,m->from);
	lg();
	return 1;
}
void send_req_to(int id, int ts){
	msg t;
	t.type = M_REQ;
	t.prev = prev_in_cs;
	send_with_ts(id, &t, ts);
}
void send_rel_to(int id){
	msg t;
	t.type = M_REL;
	t.prev = myid;
	send(id, &t);
}
void send_acc_to(int id){
	msg t;
	t.type = M_ACC;
	t.prev = prev_in_cs;
	send(id, &t);
}
void handle(msg *m){
	msg *t;
	if(m->prev > prev_in_cs) prev_in_cs = m->prev;
	switch(m->type){
		case M_ACC:
				accepts++;
			break;
		case M_REQ:	
				tqueue_add(m);
				t = tqueue_top();
				if(t->from == m->from)
					send_acc_to(m->from);
				else
					hang_add(m->from);
			break;
		case M_REL:
				tqueue_del(m->from);
				t = tqueue_top();
				if(t != 0)
					if(hang_contains(t->from)){
						hang_del(t->from);
						send_acc_to(t->from);
					}
			break;

		default:
			sprintf(buf, "HOUSTON..."); 
			lg();
	}
	usleep(500000);
}


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

int main()
{
	int i;
	msg m;
	init();

	//chcę wejść do CS
	int ts = timestamp;
	for(i=0;i<nproc;++i)
		send_req_to(i,ts);

	while(1){
		if(accepts==nproc && tqueue_top()->from == myid) break;
		if(trecv(1000,&m))
			handle(&m);
	}

	sprintf(buf, "CS IN "); lg();

	sprintf(buf, "PREVIOUS IN CS: %i",prev_in_cs); lg();
	sleepForMax(1000);
	sprintf(buf, "CS OUT"); lg();
	
	for(i=0;i<nproc;++i)
		send_rel_to(i);

	sprintf(buf,"DONE --------------------");
	lg();

	while(hang_count() > 0)
		if(trecv(1000, &m))
			handle(&m);
		


	sprintf(buf, "finished"); lg();
	pvm_exit();
}
