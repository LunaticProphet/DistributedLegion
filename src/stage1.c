#include "common.h"
#include "stage1.h"

int timestamp = 0;
int prev_in_cs[ROADS];
int accepts = 0;

int get_timestamp(){
	return timestamp;
}
int get_accept_count(){
	return accepts;
}
int get_prev_in_cs(int road){
	return prev_in_cs[road];
}

////////////////////////////////////////

msg tqueue[ROADS][SLAVENUM];

void tqueue_add(int road, msg *m){
	int i;
	for(i=0;i<SLAVENUM;++i) if(tqueue[road][i].from == -1){
		tqueue[road][i] = *m;
		break;
	}
}
void tqueue_del(int road, int from){
	int i;
	for(i=0;i<SLAVENUM;++i) if(tqueue[road][i].from == from){
		tqueue[road][i].from = -1;
		break;
	}
}
msg* tqueue_top(int road){
	int i,best = -1;
	for(i=0;i<SLAVENUM;++i){
		if(tqueue[road][i].from != -1){
			if(best == -1 || tqueue[road][i].stmp < tqueue[road][best].stmp || (tqueue[road][i].stmp == tqueue[road][best].stmp && tqueue[road][i].from < tqueue[road][best].from)){
				best = i;
			}
		}
	}
	if(best == -1) return 0;
	return &tqueue[road][best];
}

////////////////////////////////

int hang[ROADS][SLAVENUM];
void hang_add(int road, int i){
	hang[road][i] = 1;	
}
void hang_del(int road, int i){
	hang[road][i] = 0;
}
int hang_contains(int road, int i){
	return hang[road][i];
}
int hang_count(int road){
	int i,c=0;
	for(i=0;i<SLAVENUM;++i) if(hang_contains(road,i)) ++c;
	return c;
}
////////////////////////////////

void stage1_init(){
	int i,j;
	for(j=0;j<ROADS;++j){
		for(i=0;i<SLAVENUM;++i) tqueue[j][i].from = -1;
		for(i=0;i<SLAVENUM;++i) hang[j][i] = 0;
		prev_in_cs[j] = -1;
	}
}
void send_with_ts(int id, msg *m, int ts){
	int tmp = m->stmp;
	m->stmp = ts;
	send(id,m);
	m->stmp = tmp;
}
int trecv_with_ts(int msec, msg *m){
	if(trecv(msec,m)){
		if(m->stmp > timestamp) timestamp = m->stmp;
		return 1;
	} else return 0;
}


////////////////////////////////

void send_req_to(int road, int id, int ts){
	msg t;
	t.type = M_REQ;
	t.data = get_prev_in_cs(road);
	t.road = road;
	send_with_ts(id, &t, ts);
}
void send_rel_to(int road, int id){
	msg t;
	t.type = M_REL;
	t.data = my_id();
	t.road = road;
	send_with_ts(id, &t, ++timestamp);
}
void send_acc_to(int road, int id){
	msg t;
	t.type = M_ACC;
	t.data = get_prev_in_cs(road);
	t.road = road;
	send_with_ts(id, &t, ++timestamp);
}
void handle_req_acc_rel(msg *m){
	msg *t;
	if(m->data > get_prev_in_cs(m->road)) prev_in_cs[m->road] = m->data;
	switch(m->type){
		case M_ACC:
				accepts++;
			break;
		case M_REQ:	
				tqueue_add(m->road, m);
				t = tqueue_top(m->road);
				if(t->from == m->from)
					send_acc_to(m->road, m->from);
				else
					hang_add(m->road, m->from);
			break;
		case M_REL:
				tqueue_del(m->road, m->from);
				t = tqueue_top(m->road);
				if(t != 0)
					if(hang_contains(m->road, t->from)){
						hang_del(m->road, t->from);
						send_acc_to(m->road, t->from);
					}
			break;

		default:
			sprintf(buf, "HOUSTON..."); 
			lg();
	}
	//usleep(500000);
}
void stage1_enter_cs(int road, void (*event)(int,int), void (*msg_handler)(msg*)){
	accepts = 0;
	int i,ts = get_timestamp();
	msg m;
	for(i=0;i<peer_count();++i)
		send_req_to(road, i,ts);

	while(get_accept_count() != peer_count() || tqueue_top(road)->from != my_id())
		if(trecv_with_ts(1000,&m))
			msg_handler(&m);

	event(get_prev_in_cs(road),road);

	for(i=0;i<peer_count();++i)
		send_rel_to(road, i);

	while(hang_count(road) > 0)
		if(trecv_with_ts(1000, &m))
			msg_handler(&m);
}
