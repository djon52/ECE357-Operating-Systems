#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dirent.h>
#include <fcntl.h> /* Defines O_XXX constants */
#include <errno.h> /* Defines error numbers */
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#define STDIN 0
#define STDOUT 1
#define STDERR 2

int myCd();
int myPwd();
int shell(FILE *input);
struct tms t;
int main(int argc, char **argv)
{
	if(argc > 2)
	{
		fprintf(stderr,"An error occured, proper usage is as follows: command {argument {argument...} } {redirection_operation {redirection_operation...}}\n");
		return -1;
	}

	FILE *input;
	if(argc == 2)
	{	
    
		if((input = fopen(argv[1], "r")) == NULL)
		{
			fprintf(stderr,"An error occured when opening input file. Error code: %s. File: %s.\n",argv[1], strerror(errno));
			return -1;
		}
	}
	else
		input = stdin;
	
  shell(input);
	return 0;	
}

int shell(FILE *input)
{
	size_t bufsize = 4096;
	ssize_t characters;	
	
	char* line;
	line = (char *)malloc(bufsize * sizeof(char));	
	if(line == NULL)
	{
		fprintf(stderr,"An error occured when allocating memory for line. Error code: %s. \n", strerror(errno));
		return -1;
	}
	
	char* token;
	token = (char *)malloc(bufsize * sizeof(char));
	if(token == NULL)
	{
		fprintf(stderr,"An error occured when allocating memory for token. Error code: %s. \n", strerror(errno));
		return -1;
	}
	
	char** tokenArr;
	tokenArr = (char **)malloc(bufsize * sizeof(char*));
	if(tokenArr == NULL)
	{
		fprintf(stderr,"An error occured when allocating memory for tokenArr. Error code: %s. \n", strerror(errno));
		return -1;
	}
	
	int redirectFd = -1;
	int flags = 0;
	int i = 0;
	int exitStatus = 0;
	char* redirectPath;
	redirectPath = (char *)malloc(bufsize * sizeof(char));	
	if(redirectPath == NULL)
	{
		fprintf(stderr,"An error occured when allocating memory for redirected path. Error code: %s. \n", strerror(errno));
		return -1;
	}
	
	for(;;)
	{
		if((characters = getline(&line, &bufsize, input)) < 0)
		{
			fprintf(stderr,"end of file read, exiting shell with exit code %d. \n", exitStatus);
			return -1;
		}
		if(line[0] == '#')
			continue;
		else
		{
			token = strtok(line, " \r\n\t"); // discard new line that's left over source: https://stackoverflow.com/questions/16677800/strtok-not-discarding-the-newline-character
			//fprintf(stderr,token);
			
			while (token != NULL)
			{
				//fprintf(stderr,"%s\n",&token[0]);
				//fprintf(stderr,"%s\n",&token[1]);
				//fprintf(stderr,"%s\n",&token[2]);
				//fprintf(stderr,"%s\n",&token[3]);
				if(token[0] == '<') //redirect stdin
				{
					redirectFd = STDIN;
					flags = O_RDONLY;
					strcpy(redirectPath, (token+1));
				}
				else if (token[0] == '>' && token[1] == '>')
				{
					redirectFd = STDOUT;
					flags = O_WRONLY | O_APPEND | O_CREAT;
					strcpy(redirectPath, (token+2));
				}	
				else if (token[0] == '>') //open/create/truncate and redirect stdout
				{
					redirectFd = STDOUT;
					flags = O_WRONLY | O_TRUNC | O_CREAT;
					strcpy(redirectPath, (token+1));
				}	
				else if (token[0] == '2' && token[1] == '>' && token[2] == '>')
				{
					redirectFd = STDERR;
					flags = O_WRONLY | O_APPEND | O_CREAT;
					strcpy(redirectPath, (token+3));
				}				
				else if (token[0] == '2' && token[1] == '>')
				{
					redirectFd = STDERR;
					flags = O_WRONLY | O_TRUNC | O_CREAT;
					strcpy(redirectPath, (token+2));
				}
				else
				{
					tokenArr[i] = token;
					i++;
				}
				
				token = strtok(NULL, " \r\n\t");
			}
			tokenArr[i] = NULL; //execvp requires NULL terminated string...
			
			
			if(strcmp(tokenArr[0], "cd") == 0)
			{
				char *dir;
				if(tokenArr[1]==NULL)
				{
					dir = getenv("HOME");
				}
				else
				{
					dir = tokenArr[1];
				}
				exitStatus = myCd(dir);
			}
			else if(strcmp(tokenArr[0], "pwd") == 0)
				exitStatus = myPwd();
			else if(strcmp(tokenArr[0], "exit") == 0){
				
				if(tokenArr[1]==NULL){
          exit(exitStatus);
				}
				else
				{
					exit(atoi(tokenArr[1]));
				}
			}
			else
			{
				int fd, pid, wstatus;
        struct rusage stats;
        struct timeval start, end;
        gettimeofday(&start, NULL);
				switch(pid = fork())
				{
					case -1:
						fprintf(stderr, "An error occured, failed to fork process. Error code: %s. \n", strerror(errno));
						return -1;
					case 0:
						/*ref for execvp
						char *cmd = "ls";
						char *argv[3];
						argv[0] = "ls";
						argv[1] = "-la";
						argv[2] = NULL;

						execvp(cmd, argv); //This will run "ls -la" as if it were a command*/
						
						if(redirectFd > -1) // redirect has been triggered
						{
							if((fd = open(redirectPath, flags, 0666)) >= 0)
							{
								if(dup2(fd,redirectFd) == -1)
								{
									fprintf(stderr, "An error occured, failed to dup2. Error code: %s. \n", strerror(errno));
									return -1;
								}
								else if(close(fd) == -1)
								{
									fprintf(stderr, "An error occured, failed to close redirect file. Error code: %s. \n", strerror(errno));
									return -1;
								}
							}
							else
							{
								fprintf(stderr, "An error occured, failed to open redirect file. Error code: %s. File: %s\n", strerror(errno), redirectPath);
								return -1;
							}							
						}
						if((execvp(tokenArr[0], tokenArr) == -1))
						{
							exit(127);
							fprintf(stderr, "An error occured, failed to execute command. Error code: %s. Command: %s\n", strerror(errno), tokenArr[0]);
							return -1;
						}
						
							
					default:
						if (wait3(&wstatus,WUNTRACED,&stats) == -1)
						{
							fprintf(stderr, "An error occured, failed to wait for child process completion. Error code: %s. Command: %s\n", strerror(errno), tokenArr[0]);
							return -1;
						}
            exitStatus = WEXITSTATUS(wstatus);
            if(WEXITSTATUS(wstatus)==0){
              fprintf(stderr, "Child process %d exited normally\n",pid);
            }
            else{
              fprintf(stderr, "Child process %d exited with return code %d\n",pid, WEXITSTATUS(wstatus));
            }
            gettimeofday(&end, NULL);
            fprintf(stderr, "Real: %ld.%03ld sec ", (end.tv_sec-start.tv_sec), (end.tv_usec-start.tv_usec));
            fprintf(stderr, "User: %ld.%03ld sec ", stats.ru_utime.tv_sec, stats.ru_utime.tv_usec);
            fprintf(stderr, "Sys: %ld.%03ld sec\n", stats.ru_stime.tv_sec, stats.ru_stime.tv_usec);				
					
				}
				
			}
			redirectFd = -1;
			flags = 0;
			i = 0;
		}
		
	}

}

int myCd(const char *path){
  if(chdir(path)==-1){
    fprintf(stderr,"An error occured when changing the current directory to %s. Error code: %s. \n", path, strerror(errno));
		return -1;
  }
  return 0;
}

int myPwd(){
  long size;
  char *buf;
  char *dir;
  size = pathconf(".", _PC_PATH_MAX);
  if ((buf = (char *)malloc((size_t)size)) != NULL)
    dir = getcwd(buf, (size_t)size);
  if(dir == NULL){
		fprintf(stderr,"An error occured when getting the current working directory. Error code: %s. \n", strerror(errno));
		return -1;
	}
  printf("%s\n",dir);
  return 0;
}
