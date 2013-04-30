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
int accepts = 0;

int mytid;	// my TID
int myid;	// my position in TIDS
int mrtid;	// master TID

typedef struct{
	int type;
	int from;
	int val1;
	int val2;
} msg;

int tid(int id){
	return tids[id];
}

int randn(int n){
	return rand()%n;
}

void sleepForMax(int n){
	usleep(randn(n));
}

void lg(){
	pvm_initsend(PvmDataDefault);	//initialize sending
	pvm_pkint(&myid,	1,1);
	pvm_pkstr(buf);
	pvm_send(mrtid, MSG_PRNT);
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
//	if(myid==0){
		//sprintf(buf,"GRUP: %d A: %d B: %d",groups,a,b);
		//lg(); 
	//}
	int t;
	for(i=0;i<groups;++i){
		t = a*groups + i;
		if(t<nproc && t!=myid)	friends[nfriends++] = t;
	}
	for(i=0;i<groups;++i){
		t = i*groups + b;
		if(t<nproc && t!=myid)	friends[nfriends++] = t;
	}
}

void send(int id, msg *m){
	pvm_initsend(PvmDataDefault);	//initialize sending
	pvm_pkint(&(m->type),	1,1);
	pvm_pkint(&myid,		1,1);
	pvm_pkint(&(m->val1),	1,1);
	pvm_pkint(&(m->val2),	1,1);
	pvm_send(tid(id), MSG_COMM);
}

int recv(int usec, msg *m){
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = usec;
	int e = pvm_trecv(-1, MSG_COMM, &t);
	if(e <= 0) return 0;
	pvm_upkint(&(m->type), 	1,1);
	pvm_upkint(&(m->from), 	1,1);
	pvm_upkint(&(m->val1), 	1,1);
	pvm_upkint(&(m->val2), 	1,1);
	return 1;
}
void handle(msg *m){
	switch(m->type){

		case M_ACC:
			if(m->val1 == myid){
				accepts++;
			}else{
				msg t;
				t.type = M_REQ;
				t.val1 = 0;
				t.val2 = m->val2;
				send(m->val1, &t);
			}
			break;
		case M_REQ:
			
			break;
		case M_REL:


			break;


		default:
			sprintf(buf, "HOUSTON...");
			lg();
	}
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
	
	int i;
	msg m;


	/*
	for(i=0;i<nfriends;++i){
		m.type = M_REQ;
		m.val1 = 0; //puste
		m.val2 = 0; //nr trasy
		send(friends[i],&m);
	}



	while(accepts<nfriends){
		recv(1000000, &m);
		handle(&m);
	}

	*/
	





	sprintf(buf, "finished");
	lg();
	pvm_exit();
}
