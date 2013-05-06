#include "def.h"


typedef struct{
	int type;
	int stmp;
	int from;
	int data;
	int road;
} msg;

char buf[1024];
void lg();
int randn(int n);
int tid(int id);
int my_id();
int peer_count();
void sleepForMax(int n);
void init_slave();
void quit_slave();
void send(int id, msg *m);
int trecv(int usec, msg *m);
