#define M_REQ 1
#define M_ACC 2
#define M_REL 3

void stage1_init();
void handle_req_acc_rel(msg *m);
void stage1_enter_cs(int road, void (*event)(int,int), void (*msg_handler)(msg*));
