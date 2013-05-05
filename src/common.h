#include "def.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

#define M_REQ 1
#define M_ACC 2
#define M_REL 3

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
void init();
