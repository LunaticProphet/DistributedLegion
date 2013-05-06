#include "common.h"
#include "stage1.h"
#include "stage2.h"

int _overall = 0;

void handle(msg *m){
	switch(m->type){
		case M_REQ:
		case M_ACC:
		case M_REL:
			handle_req_acc_rel(m);
		break;
		
		case M_ASK:
		case M_TKS:
			handle_ask_tks(m);
		break;
	};	
}
void talk_to_last_in_queue(int road, int prev){
	stage2_enter_queue(prev, road, _overall);
}

void enter_road(int road, int required, int overall, void (*event)(int)){
	_overall = overall;
	stage1_enter_cs(road, talk_to_last_in_queue, handle);
	stage2_enter_cs(road, required, overall, event, handle);
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

void walk_on_the_road(int road){
	sprintf(buf,"Imma walkin' on da road %i", road); lg();
	sleepForMax(1000);
	sprintf(buf,"Imma out of da road %i", road); lg();
}


int main()
{
	init_slave();
	stage1_init();
	stage2_init();

	
	enter_road(0, 3, 10, walk_on_the_road);	


	sprintf(buf, "finished"); lg();
	quit_slave();
}
