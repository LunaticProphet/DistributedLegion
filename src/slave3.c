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
int legit_accepts = 0;

int mytid;	// my TID
int myid;	// my position in TIDS
int mrtid;	// master TID
int accepts;

typedef struct{
	int type;
	int stmp;
	int from;
	int val1;
	int val2;
} msg;

void lg(){
	pvm_initsend(PvmDataDefault);	//initialize sending
	pvm_pkint(&myid,	1,1);
	pvm_pkstr(buf);
	pvm_send(mrtid, MSG_PRNT);
}


///////////////////////////////////////////
msg reqs[SLAVENUM];

int req_contains(int from){
	int i;
	for(i=0;i<SLAVENUM;++i){
		if(reqs[i].from == from) return 1;
	}
	return 0;
}

void req_add(msg *m){	
	int i;
	for(i=0;i<SLAVENUM;++i){
		if(reqs[i].from == -1){
			reqs[i].type = m->type;
			reqs[i].stmp = m->stmp;
			reqs[i].from = m->from;
			reqs[i].val1 = m->val1;
			reqs[i].val2 = m->val2;
			break;
		}
	}
}
void req_del(int from){
	int i;
	for(i=0;i<SLAVENUM;++i){
		if(reqs[i].from == from){
			reqs[i].from = -1;
			//break;
		}
	}
}
msg* req_top(){
	int i;
	int best = -1;
	for(i=0;i<SLAVENUM;++i){
		if(reqs[i].from != -1){
			best = i;
			break;
		}	
	}
	if(best == -1){
		return 0;
	}	
	for(i=best;i<SLAVENUM;++i){
		if(reqs[i].from != -1){
			if(reqs[i].stmp==reqs[best].stmp){
				if(reqs[i].from<reqs[best].from) best=i;
			}
			else if(reqs[i].stmp<reqs[best].stmp){
				best=i;	
			}
		}
	}
	return &reqs[best];
}
int req_count(){
	int i,c=0;
	for(i=0;i<SLAVENUM;++i){
		if(reqs[i].from != -1) ++c;
	}
	return c;
} 
void req_print(){
	sprintf(buf, "STACK: %i %i %i %i", reqs[0].from, reqs[1].from, reqs[2].from, reqs[3].from);
	lg();
}
///////////////////////////////////////////

int tid(int id){
	return tids[id];
}

int randn(int n){
	return rand()%n;
}

void sleepForMax(int n){
	usleep(randn(n));
}


void init(){
	accepts = 0;
	mytid = pvm_mytid();
	pvm_recv(-1, MSG_TIDS);
	pvm_upkint(&mrtid, 1,1);
	pvm_upkint(&myid, 1,1);
	pvm_upkint(&nproc, 1,1);
	int i;
	for(i=0;i<nproc;++i) pvm_upkint(&tids[i], 1,1);
    srand(myid);

	
	for(i=0;i<SLAVENUM;++i){
		reqs[i].from = -1;
	}
}

void sendt(int id, msg *m, int ts){
	pvm_initsend(PvmDataDefault);	//initialize sending
	pvm_pkint(&(m->type),	1,1);
	pvm_pkint(&(ts),	1,1);
	pvm_pkint(&myid,		1,1);
	pvm_pkint(&(m->val1),	1,1);
	pvm_pkint(&(m->val2),	1,1);
	pvm_send(tid(id), MSG_COMM);
}

void send(int id, msg *m){
	sendt(id, m, ++timestamp);
}

int recv(int usec, msg *m){
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = usec;
	int e = pvm_trecv(-1, MSG_COMM, &t);
	if(e <= 0) return 0;
	pvm_upkint(&(m->type), 	1,1);
	pvm_upkint(&(m->stmp),	1,1);
	pvm_upkint(&(m->from), 	1,1);
	pvm_upkint(&(m->val1), 	1,1);
	pvm_upkint(&(m->val2), 	1,1);
	if(m->stmp > timestamp) timestamp = m->stmp;
	sprintf(buf, "Received message type=%i stmp=%i from=%i val1=%i val2=%i", m->type, m->stmp, m->from, m->val1, m->val2);
	lg();
	return 1;
}
void send_req_to(int id, int ts){
	msg t;
	t.type = M_REQ;
	t.val1 = 0;
	t.val2 = 0;
	sendt(id, &t, ts);
}
void send_rel_to(int id){
	msg t;
	t.type = M_REL;
	t.val1 = 0;
	t.val2 = 0;
	send(id, &t);
}
void send_acc_to(int id, int val1){
	msg t;
	t.type = M_ACC;
	t.val1 = val1;
	t.val2 = 0;
	send(id, &t);
}
void handle(msg *m){
	msg* t;
	switch(m->type){

		case M_ACC:
			accepts++;
			break;
		case M_REQ:

			req_add(m);
			t = req_top();
			if(t->from == m->from){
				send_acc_to(m->from,0);
			}
		
			break;
		case M_REL:
			t = req_top();
			if(t == 0){
				sprintf(buf, "SCHEISSE...");
				lg();
			}else{
				if(t->from == m->from){
					req_del(m->from);
					t = req_top();
					if(t != 0){
						send_acc_to(t->from,0);
					}
				}else 
					req_del(m->from);
			}
			break;

		default:
			sprintf(buf, "HOUSTON...");
			lg();
	}
//	usleep(500000);
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
	sleepForMax(1000000);
	//chcemy wejść do sekcji krytycznej
	int ts = timestamp;
	for(i=0;i<nproc;++i){
		send_req_to(i, ts);
	}

	while(accepts<nproc){
		if(recv(1000000,&m))
			handle(&m);
	}

	sprintf(buf,"CS IN -------");
	lg();

	sleepForMax(1000000);

	sprintf(buf,"CS OUT ------");
	lg();

	for(i=0;i<nproc;++i){
		send_rel_to(i);
	}

	while(1){ 
		if(recv(1000000,&m))
			handle(&m);
	}
	



	sprintf(buf, "finished");
	lg();
	pvm_exit();
}
