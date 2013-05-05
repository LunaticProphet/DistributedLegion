#include "common.h"
#include "stage2.h"
#include <signal.h>

typedef struct{
	int tks;
	int next_peer;
	int goal;
	int moved;
} token_pocket;

token_pocket state;

void send_ask_to(int id){
	msg t;
	t.type = M_ASK;
	send(id, &t);
}

void send_tks_to(int id, int count){
	msg t;
	t.type = M_TKS;
	t.data = count;
	send(id, &t);
}

void handle_ask_tks(msg *m){
	token_pocket *s = &state;
	switch(m->type){
		case M_ASK:
			if(s->next_peer == -1){
				s->next_peer = m->from;
			}else{
				sprintf(buf,"Critical error: duplicate ASK");
				lg();
			}
		break;

		case M_TKS:
			s->moved += m->data;
			if(s->goal){
				send_tks_to(s->next_peer, m->data);
			}else{
				s->tks += m->data;
			}
		break;

		default:
			sprintf(buf, "HOUSTON......");
			lg();

	};
}

void stage2_init(){
	state.next_peer = -1;
	state.tks = 0;
	state.goal = 0;
	state.moved = 0;
}

void stage2_enter_queue(int target){
	send_ask_to(target);
}

///////////////////////////
int keep_working;
void stopper(int signal){
	keep_working = 0;
}
///////////////////////////

void stage2_enter_cs(int road, int required, int overall, void (*event)(int), void (*msg_handler)(msg*)){
	token_pocket *s = &state;
	int i;
	msg m;
	while(s->tks < required){
		if(trecv(1000,&m))
			msg_handler(&m);
	}
	
	event(road);
	s->goal = 1;

	/*
	keep_working = 1;
	int child = fork();
	if(child){ //mam dziecko, jestem rodzicem
		event();
		s->goal = 1;					
		kill(getppid(), SIGUSR1);
	}else{ //nie mam dziecka, jestem dzieckiem
		signal(SIGUSR1, stopper);
		while(keep_working){
			if(trecv(100,&m))
				msg_handler(&m);
		}
		exit(0);	
	}		
	*/

	while(s->moved < overall){
		if(trecv(1000,&m))
			msg_handler(&m);
	}
	s->next_peer = -1;
	s->goal = 0;
	s->moved = 0;
	s->tks = 0; //?
}
