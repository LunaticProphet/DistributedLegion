#define M_ASK 4 // prosba o wejscie na koniec kolejki
#define M_TKS 5 // tokeny kolejkowe


void stage2_init();
void handle_ask_tks(msg *m);
void stage2_enter_queue(int target, int road);
void stage2_enter_cs(int road, int required, void (*event)(int,int), void (*msg_handler)(msg*));
