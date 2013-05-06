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
	sprintf(buf, "SND%i type=%i from=%i data=%i", id, t.type, t.from, t.data); lg();
	send(id, &t);
}

void send_tks_to(int id, int count){
	msg t;
	t.type = M_TKS;
	t.data = count;
	sprintf(buf, "SND%i type=%i from=%i data=%i", id, t.type, t.from, t.data); lg();
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
			if(s->goal){
				s->moved += m->data;
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

void stage2_enter_queue(int target, int overall){
	if(target == -1){
		token_pocket *s = &state;
		s->tks = overall;
	}
	else
		send_ask_to(target);
}

///////////////////////////
token_pocket *tohalt;
void stopper(int signal){
	tohalt->goal = 1;
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


	int child = fork();
	if(child){ //parent
		tohalt = s;
		signal(SIGUSR1, stopper);
		while(s->goal==0 || s->moved < overall){
			if(trecv(1000,&m))
			msg_handler(&m);
			if(
				s->next_peer != -1 && (
					(s->goal==0 && s->tks>required) ||
					(s->goal==1 && s->tks>0)
				)
			){
				send_tks_to(s->next_peer, s->tks);
				s->moved += s->tks;
				s->tks = 0;
			}
		}
	}else{ //child
		event(road);
		s->goal = 1;
		kill(getppid(), SIGUSR1);
		exit(0);
	}

	s->next_peer = -1;
	s->goal = 0;
	s->moved = 0;
	s->tks = 0; //?
}
