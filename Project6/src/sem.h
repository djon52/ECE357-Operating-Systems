#ifndef __SEM_H
#include <sys/types.h>
#include "spinlock.h"
#define N_PROC 64

int my_procnum;
int sig[6];


struct sem
{
	int count;
	char lock;
	unsigned int waitproc[N_PROC];
	int procstat[N_PROC];
  int sleeps[N_PROC];
  int wakes[N_PROC];
  int sig;
};

void sem_init(struct sem *s, int count);
int sem_try(struct sem *s);
void sem_wait(struct sem *s, int vcpu);
void sem_inc(struct sem *s, int vcpu);

#define __SEM_H
#endif