#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

//
//	PROPER USAGE: ./prob4.out [# of children to fork] [# of times they send signal] ['real' or 'conventional']
//	'Real' will send a real-time signal: SIGRTMIN with a signal number of 34.
//	'Conventional' will send a conventional signal: SIGUSR1 with a signal number of 10.
//

int signo, numSendSig, sigReceiptCounter;
int mainPID;

void handler(int sig)
{	
	if (sig == signo)
		sigReceiptCounter++;
}

int sender()
{
	for(int i = 0; i < numSendSig; i++)
	{
		kill(mainPID,signo);
	}
}

int main(int argc, char **argv)
{
	if(argc != 4)
	{
		fprintf(stderr, "An error occured, proper usage is: ./prob4.out [# of children to fork] [# of times they send signal] ['real' or 'conventional']\n");
		return -1;
	}
	int numOfKillerChildren = atoi(argv[1]);
	numSendSig = atoi(argv[2]);	
	struct sigaction sa;
	int pid,status;
	
	if(strcmp(argv[3],"real") == 0)
		signo = SIGRTMIN; /* real-time signal (#34) */
	else if(strcmp(argv[3],"conventional") == 0)
		signo = SIGUSR1; /* conventional signal (#10) */
	else
	{
		fprintf(stderr, "An error occured, proper usage is: ./prob4.out [# of children to fork] [# of times they send signal] ['real' or 'conventional']\n");
		return -1;
	}
	
	mainPID = getpid();
	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	
	if (sigaction(signo,&sa,0) == -1)
	{
		fprintf(stderr, "An error occured with sigaction. Error code: %s. \n", strerror(errno));
		return -1;
	}
	
	for(int i = 0; i < numOfKillerChildren; i++)
	{		
		switch(pid = fork())
		{
			case -1:
				fprintf(stderr, "An error occured, failed to fork process. Error code: %s. \n", strerror(errno));
				return -1;
			case 0:
				sender();
				return 0;
		}
	}
		
	
	while (wait(&status) > 0 || errno == EINTR)
		;
	
	
	printf("For signal #%d: Sent a total of %d signals, received a total of %d signals. \n", signo, numOfKillerChildren*numSendSig, sigReceiptCounter);
}