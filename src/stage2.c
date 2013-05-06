#include "common.h"
#include "stage2.h"
#include <signal.h>

typedef struct{
	int tks;
	int next_peer;
	int goal;
	int moved;
} token_pocket;

token_pocket state[ROADS];

void send_ask_to(int road, int id){
	msg t;
	t.type = M_ASK;
	t.road = road;
	//sprintf(buf, "SND%i type=%i from=%i data=%i", id, t.type, t.from, t.data); lg();
	send(id, &t);
}

void send_tks_to(int road, int id, int count){
	msg t;
	t.type = M_TKS;
	t.data = count;
	t.road = road;
	//sprintf(buf, "SND%i type=%i from=%i data=%i", id, t.type, t.from, t.data); lg();
	send(id, &t);
}

void handle_ask_tks(msg *m){
	token_pocket *s = &(state[m->road]);
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
			if(s->goal){
				s->moved += m->data;
				send_tks_to(m->road, s->next_peer, m->data);
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
	int i;
	for(i=0;i<ROADS;++i){
		state[i].next_peer = -1;
		state[i].tks = 0;
		state[i].goal = 0;
		state[i].moved = 0;
	}
}

void stage2_enter_queue(int target, int road, int overall){
	if(target == -1){
		state[road].tks = overall;
	}
	else
		send_ask_to(road, target);
}

///////////////////////////
token_pocket *tohalt;
void stopper(int signal){
	tohalt->goal = 1;
}
///////////////////////////

void stage2_enter_cs(int road, int required, int overall, void (*event)(int), void (*msg_handler)(msg*)){
	token_pocket *s = &(state[road]);
	int i;
	msg m;
	while(s->tks < required){
		if(trecv(1000,&m))
			msg_handler(&m);
	}
	int child = fork();
	if(child){ //parent
		tohalt = s;
		signal(SIGUSR1, stopper);
		while(s->goal==0 || s->moved < overall){
			if(trecv(1000,&m))
			msg_handler(&m);
			
			if(s->next_peer != -1){
				if(s->goal == 1 && s->tks>0){
					send_tks_to(road, s->next_peer, s->tks);
					s->moved += s->tks;
					s->tks = 0;
				}
				if(s->goal == 0 && s->tks > required){
					send_tks_to(road, s->next_peer, s->tks-required);
					s->moved += s->tks-required;
					s->tks = required;
				}

			}
			
		}
	}else{ //child
		event(road);
		kill(getppid(), SIGUSR1);
		exit(0);
	}

	s->next_peer = -1;
	s->goal = 0;
	s->moved = 0;
	s->tks = 0; //?
}
