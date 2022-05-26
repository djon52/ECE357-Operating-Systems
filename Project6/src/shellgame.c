extern int my_procnum; // note: num processors is a global variable (sem)
#include <unistd.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "spinlock.h"
#include "sem.h"

struct shared {
  struct sem A;
  struct sem B;
  struct sem C; 
} shared;
int main(int argc, char **argv)
{ 
	if(argc != 3)
	{
		fprintf(stderr, "An error occured, proper usage is: shellgame [initial semaphore value] [number of moves per task]\n");
		return -1;
	}
  if(atoi(argv[1])<1){
    fprintf(stderr, "An error occured, must be positive initial semaphore value.\n");
		return -1;
  }
  
  pid_t pid;
  int status;
  int sem_initial = atoi(argv[1]);
  int moves = atoi(argv[2]);
  struct shared * mapShared;
  mapShared = (struct shared *)mmap(NULL, sizeof(struct shared), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
  sem_init(&mapShared->A,sem_initial);
  sem_init(&mapShared->B,sem_initial);
  sem_init(&mapShared->C,sem_initial);

  for(int i =0;i<6;i++){
    switch(pid = fork()) 
		{
			case -1:
				fprintf(stderr, "An error occured, failed to fork process. Error code: %s. \n", strerror(errno));
				return -1;
			case 0:
        if(i==0){
          printf("VCPU 0 starting, pid %d\n",getpid());
          my_procnum = 0;
          for(int j = 0;j<moves;j++){
            sem_wait(&mapShared->A,my_procnum);
            sem_inc(&mapShared->B,my_procnum);
          }
          printf("VCPU 0 done\n");
        }
        else if(i==1){
          printf("VCPU 1 starting, pid %d\n",getpid());
          my_procnum = 1;
          for(int j = 0;j<moves;j++){
            sem_wait(&mapShared->B,my_procnum);
            sem_inc(&mapShared->A,my_procnum);
          }
          printf("VCPU 1 done\n");
        }
        else if(i==2){
          printf("VCPU 2 starting, pid %d\n",getpid());
          my_procnum = 2;
          for(int j = 0;j<moves;j++){
            sem_wait(&mapShared->A,my_procnum);
            sem_inc(&mapShared->C,my_procnum);
          }
          printf("VCPU 2 done\n");
        }
        else if(i==3){
          printf("VCPU 3 starting, pid %d\n",getpid());
          my_procnum = 3;
          for(int j = 0;j<moves;j++){
            sem_wait(&mapShared->C,my_procnum);
            sem_inc(&mapShared->A,my_procnum);
          }
          printf("VCPU 3 done\n");
        }
        else if(i==4){
          printf("VCPU 4 starting, pid %d\n",getpid());
          my_procnum = 4;
          for(int j = 0;j<moves;j++){
            sem_wait(&mapShared->B,my_procnum);
            sem_inc(&mapShared->C,my_procnum);
          }
          printf("VCPU 4 done\n");
        }
        else{
          printf("VCPU 5 starting, pid %d\n",getpid());
          my_procnum = 5;
          for(int j = 0;j<moves;j++){
            sem_wait(&mapShared->C,my_procnum);
            sem_inc(&mapShared->B,my_procnum);
          }
          printf("VCPU 5 done\n");
        }
        printf("Child %d (pid %d) done, sig handler was invoked %d times\n",my_procnum,getpid(),sig[my_procnum]);
				return 0;
		}
  }
  printf("Main process spawned all children, waiting\n");
  for(int i = 0; i < 6; i++) 
	{
		if((pid = wait(&status)) == -1) 
		{
			fprintf(stderr, "An error occured, failed to wait for child. Error code: %s. \n", strerror(errno));
			return -1;
		}
    printf("Child pid %d exited w/ 0\n", pid);
	}

  printf("Sem#  \tval \t    sleeps\t     wakes\n");
  printf("0    \t  %d\t      \t\n", mapShared->A.count);
  for(int i =0;i<6;i++){
    printf(" VCPU %1d\t    \t%10d\t%10d\n",i,mapShared->A.sleeps[i],mapShared->A.wakes[i]);
  }
  printf("1    \t  %d\t      \t\n", mapShared->B.count);
  for(int i =0;i<6;i++){
    printf(" VCPU %1d\t    \t%10d\t%10d\n",i,mapShared->B.sleeps[i],mapShared->B.wakes[i]);
  }
  printf("2    \t  %d\t      \t\n", mapShared->C.count);
  for(int i =0;i<6;i++){
    printf(" VCPU %1d\t    \t%10d\t%10d\n",i,mapShared->C.sleeps[i],mapShared->C.wakes[i]);
  }
	return 0;
}