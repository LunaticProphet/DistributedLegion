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
int friends[SLAVENUM];
int nfriends = 0;
int timestamp = 0;
int legit_accepts = 0;

int mytid;	// my TID
int myid;	// my position in TIDS
int mrtid;	// master TID
int accepted[SLAVENUM];

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
int acc_contains(int id){
	int i;
	for(i=0;i<SLAVENUM;++i) 
		if(accepted[i]==id) 
			return 1;
	return 0;
}

void acc_add(int id){
	int i;
	for(i=0;i<SLAVENUM;++i) 
		if(accepted[i]==-1){
			accepted[i]=id;
			break;
		}
}

void acc_del(int id){
	int i;
	for(i=0;i<SLAVENUM;++i) 
		if(accepted[i]==id){
			accepted[i]=-1;
			break;
		}
}
void acc_del_all(){
	int i;
	for(i=0;i<SLAVENUM;++i) accepted[i] = -1;
}

int acc_count(){
	int i,r = 0;
	for(i=0;i<SLAVENUM;++i) 
		if(accepted[i]!=-1) ++r;
	return r;
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
	mytid = pvm_mytid();
	pvm_recv(-1, MSG_TIDS);
	pvm_upkint(&mrtid, 1,1);
	pvm_upkint(&myid, 1,1);
	pvm_upkint(&nproc, 1,1);
	int i;
	for(i=0;i<nproc;++i) pvm_upkint(&tids[i], 1,1);
    srand(myid);

	int groups = ceil(sqrt(nproc));
	int a = myid / groups;
	int b = myid % groups;
	int t;
	for(i=0;i<groups;++i){
		t = a*groups + i;
		if(t<nproc && t!=myid)	friends[nfriends++] = t;
	}
	for(i=0;i<groups;++i){
		t = i*groups + b;
		if(t<nproc && t!=myid)	friends[nfriends++] = t;
	}
	for(i=0;i<SLAVENUM;++i){
		reqs[i].from = -1;
	}
	for(i=0;i<SLAVENUM;++i){
		accepted[i] = -1;
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
	if(myid==3){
		sprintf(buf, "DEBUG3 to=%i type=%i stmp=%i from=%i val1=%i val2=%i", id, m->type, ts, myid, m->val1, m->val2);
		lg();
	}
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
void handle(msg *m){
	int asdasd1, asdasd2;
	switch(m->type){

		case M_ACC:
			acc_add(m->from);
			if(m->val1 != myid){
				msg t;
				t.type = M_REQ;
				t.val1 = 0;
				t.val2 = m->val2;
				send(m->val1, &t);
			}else{
				legit_accepts++;
				int asdasd = req_count();
				sprintf(buf, "LEGIT ACCEPT (%i) FROM %i, ON STACK: %i", legit_accepts, m->from, asdasd);
				lg();
			}
			
			break;
		case M_REQ:

			req_add(m);
			msg* t = req_top();
			msg resp;
			resp.type = M_ACC;
			resp.val1 = t->from;
			resp.val2 = 0;
			send(m->from, &resp);
			

			/*

			jeśli nie chcę wejść do sekcji, nikt mnie nie poprosił o wejście, to daję ACCEPT
			jeśli komuś dałem zgodę, a koleś który się pyta ma lepszy priorytet, daję mu accept warunkowy
			jeśli sam chcę wejść, 

			*/
			break;
		case M_REL:
			asdasd1 = req_count();
			req_del(m->from);
			asdasd2 = req_count();
			sprintf(buf, "RELEASE FROM %i, ON STACK:%i WAS:%i", m->from, asdasd2, asdasd1);
			lg();
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

	init();
	sleepForMax(1000000);
	//chcemy wejść do sekcji krytycznej

	int init_ts = timestamp;

	int i;
	msg m;
	send_req_to(myid, init_ts);
	while(acc_count() == 0){
		if(recv(1000000, &m)){
			handle(&m);
		}
	}
	//Kiedy otrzymam accepta od samego siebie

	for(i=0;i<nfriends;++i){
		send_req_to(friends[i], init_ts);
	}

	msg* tmp;
	while(1){
		tmp = req_top();
		if(legit_accepts==nfriends+1 && tmp->from == myid)
			break;

		if(recv(1000000, &m)){
			handle(&m);
		}
	}

	//zakładam że jestem na req_top

	//wchodzę do sekcji krytycznej
	sprintf(buf, "CS IN -------");
	lg();
	sleepForMax(1000000);
	sprintf(buf, "CS OUT ------");
	lg();

	for(i=0;i<SLAVENUM;++i){
		if(accepted[i] != -1){
			send_rel_to(i);
		}	
	}
	acc_del_all();
	legit_accepts = 0;

	while(1){
		if(recv(1000000, &m)){
			handle(&m);
		}
	}


	sprintf(buf, "finished");
	lg();
	pvm_exit();
}
