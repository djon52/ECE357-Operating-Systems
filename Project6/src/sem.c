#include <unistd.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h> 
#include <ctype.h>
#include <stdbool.h>
#include <sched.h>

#include "sem.h"
#include "spinlock.h"

void sighandler(int signal)
{
  sig[my_procnum]++;
  return;
	/* wakeup signal */
}

void sem_init(struct sem *s, int count)
{
	s->count = count;
	s->lock = 0;
	for(int i = 0; i < N_PROC; i++)
	{
		s->waitproc[i] = 0;
		s->procstat[i] = 0;
	}
	
	if(signal(SIGUSR1,sighandler) == SIG_ERR)
	{
		fprintf(stderr," An error occured, failed to define SIGUSR1. Error code: %s. \n", strerror(errno));
		exit(-1);
	}			
}
/*
int sem_try(struct sem *s)
{
	spin_lock(&s->lock);
	if(s->count > 0)
	{
		s->count--;
		spin_unlock(&s->lock);
		s->procstat[my_procnum] = 0;
		return 1;
	}
	else
	{
		spin_unlock(&s->lock);	
		s->procstat[my_procnum] = 1;		
		return 0;
	}	
}

void sem_wait(struct sem *s)
{
	sigset_t newmask, oldmask;
	s->waitproc[my_procnum] = getpid();
	
	while(!sem_try(s))
	{
		spin_lock(&s->lock);
		if(sigemptyset(&newmask) == -1)
		{
			fprintf(stderr, "An error occured, failed to create empty signal mask. Error code: %s. \n", strerror(errno));
			exit(-1);
		}	
		if(sigemptyset(&oldmask) == -1)
		{
			fprintf(stderr,"An error occured, failed to create empty signal mask. Error code: %s. \n", strerror(errno));
			exit(-1);
		}	
		if(sigaddset(&newmask, SIGUSR1) == -1)
		{
			fprintf(stderr,"An error occured, failed to block SIGUSR1. Error code: %s. \n", strerror(errno));
			exit(-1);
		}	
		if(sigprocmask(SIG_BLOCK, &newmask, NULL) == -1)
		{
			fprintf(stderr," An error occured, failed to change signal mask. Error code: %s. \n", strerror(errno));
			exit(-1);
		}
		
		spin_unlock(&s->lock);
		sigsuspend(&oldmask);
		
	}
}
*/


int sem_try(struct sem *s)
{
	spin_lock(&s->lock);
	if(s->count > 0)
	{
		s->count--;
		spin_unlock(&s->lock);
		s->procstat[my_procnum] = 0;
		return 1;
	}
	else
	{
		spin_unlock(&s->lock);	
		s->procstat[my_procnum] = 1;		
		return 0;
	}	
}

void sem_wait(struct sem *s, int vcpu)
{
	sigset_t newmask, oldmask;
	s->waitproc[my_procnum] = getpid();
	if(sigemptyset(&newmask) == -1)
	{
		fprintf(stderr, "An error occured, failed to create empty signal mask. Error code: %s. \n", strerror(errno));
		exit(-1);
	}	
	if(sigemptyset(&oldmask) == -1)
	{
		fprintf(stderr,"An error occured, failed to create empty signal mask. Error code: %s. \n", strerror(errno));
		exit(-1);
	}	
	if(sigaddset(&newmask, SIGUSR1) == -1)
	{
		fprintf(stderr,"An error occured, failed to block SIGUSR1. Error code: %s. \n", strerror(errno));
		exit(-1);
	}	
	while(1)
	{
		spin_lock(&s->lock);		
		
		if(sigprocmask(SIG_BLOCK, &newmask, NULL) == -1)
		{
			fprintf(stderr," An error occured, failed to change signal mask. Error code: %s. \n", strerror(errno));
			exit(-1);
		}	
		if(s->count > 0)
		{
			s->count--;
			s->procstat[my_procnum] = 0;
			spin_unlock(&s->lock);
			return;
		}
		else // was blocked
		{
      s->sleeps[vcpu]++;
			spin_unlock(&s->lock);
			s->procstat[my_procnum] = 1;
			sigsuspend(&oldmask);
		}
	}
}

void sem_inc(struct sem *s, int vcpu)
{
	// increments the value of the semaphore and wakes up a blocked process waiting on the semaphore
	spin_lock(&s->lock);
	s->count++;
	for(int i = 0; i < N_PROC; i++)
	{
		if(s->procstat[i] == 1) //if was blocked and sleeping
		{
			s->procstat[i] = 0;
			if(kill(s->waitproc[i], SIGUSR1) == -1) //wakeup
			{
				fprintf(stderr," An error occured, failed to send signal to pid %d. Error code: %s. \n", s->waitproc[i],strerror(errno));
				exit(-1);
			}
      s->wakes[i]++;
		}
	}
	spin_unlock(&s->lock);
}

