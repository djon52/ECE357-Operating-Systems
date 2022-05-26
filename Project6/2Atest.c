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
#define N_PROC 64

//
//	Number of 'processors': 64.  Number of Iterations: 1000000
//

int main() 
{
	int status;
	volatile char *lock;
	char* mapProt;
	char* mapUnprot;
	int* mapProt1;
	int* mapUnprot1;
	pid_t pid;

	mapProt = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	mapUnprot = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	
	if(mapProt == MAP_FAILED || mapUnprot == MAP_FAILED) 
	{
		fprintf(stderr, "An error occured, failed to create mmap. Error code: %s. \n", strerror(errno));
		return -1;
	}
	
	mapProt1 = (int*)mapProt;
	mapUnprot1 = (int*)mapUnprot;

	mapProt1[0] = 0;
	mapUnprot1[0] = 0;
		
	lock = mapProt + sizeof(mapProt1);
	*lock = 0;

	for(int i = 0; i < N_PROC; i++) 
	{
		switch(pid = fork()) 
		{
			case -1:
				fprintf(stderr, "An error occured, failed to fork process. Error code: %s. \n", strerror(errno));
				return -1;
			case 0:
				for(int j = 0; j < 1000000; j++) 
				{
					mapUnprot1[0]++;
				}
				for(int k = 0; k < 1000000; k++) 
				{
					spin_lock(lock);
					mapProt1[0]++;
					spin_unlock(lock);
				}
				return 0;
		}
	}

	for(int i = 0; i < N_PROC; i++) 
	{
		if((pid = wait(&status)) == -1) 
		{
			fprintf(stderr, "An error occured, failed to wait for child. Error code: %s. \n", strerror(errno));
			return -1;
		}
	}

	printf("Expected: %d\nWith spinlock: %d\nWihout spinlock: %d\n", N_PROC*1000000, mapProt1[0], mapUnprot1[0]);
	return 0;
}