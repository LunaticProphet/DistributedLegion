#include "common.h"
#include "stage1.h"
#include "stage2.h"

const int roadsize[] = {10,10}; //values of an extern int[] defined in def.h


void handle(msg *m){
	switch(m->type){
		case M_REQ: case M_ACC: case M_REL:
			handle_req_acc_rel(m);
			break;
		case M_ASK: case M_TKS:
			handle_ask_tks(m);
			break;
	};	
}
void enter_road(int road, int required, void (*event)(int,int)){
	stage1_enter_cs(road, stage2_enter_queue, handle);
	stage2_enter_cs(road, required, event, handle);
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

void walk_on_the_road(int road, int required){
	sprintf(buf, "enters road %i -------------",road); lg();
	sleepForMax(1000);
	sprintf(buf, "leaves road %i --------------",road); lg();
}


int main()
{
	init_slave();
	stage1_init();
	stage2_init();
	
	

	enter_road(0, 4, walk_on_the_road);	


	sprintf(buf, "finished"); lg();
	quit_slave();
}
