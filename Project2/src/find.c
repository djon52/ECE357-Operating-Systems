#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
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

int exploreDirectory(char *basePath);
int printOutput(struct dirent *dp, struct stat buf);
bool start = true; 

int main(int argc, char **argv)
{
	char *path;
	if(argc > 2)
	{
		printf("An error occured, proper usage is as follows: ./find [starting directory]\n");
		return -1;
	}

	if(argc == 2)
		path = argv[1];
	else
		path = ".";
	
	exploreDirectory(path);	
	return 0;	
}

int exploreDirectory(char *basePath)
{
	char path[1000];
	struct dirent *dp;
	DIR *dir = opendir(basePath);
	struct stat buf;
	
	if(dir == NULL)
	{
		fprintf(stderr,"An error occured when attempting to open directory. Error code: %s. File: %s.\n", strerror(errno), basePath);
		return -1;
	}
	
	while((dp = readdir(dir)) != NULL)
	{
					
		strcpy(path, basePath);
		strcat(path, "/");
		strcat(path, dp->d_name);

		if(lstat(path, &buf) < 0)
		{
			fprintf(stderr,"An error occured when attempting to get stats of file. Error code: %s. File: %s.\n", strerror(errno), path);
			return -1;
		}
				
				/* https://programmer.group/detailed-description-of-dir-dirent-stat-and-other-structures-under-linux.html*/
		if(S_ISREG(buf.st_mode))
		{			
			printOutput(dp, buf);
			printf("%s\n", path);
		}
		else if (S_ISDIR(buf.st_mode))
		{
			if((strcmp(dp->d_name, ".") == 0) && start)
			{
				printOutput(dp, buf);
				printf("%s\n", basePath);
				start = false;
				continue;
			}
			else if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
			{
				printOutput(dp, buf);
				printf("%s\n", path);
				exploreDirectory(path);        
			}
    }
		else if (S_ISLNK(buf.st_mode))
		{
			char linkBuf[1000];
			ssize_t bytesPlaced;
			bytesPlaced = readlink(path, linkBuf, sizeof(linkBuf));
			if(bytesPlaced == -1)
			{
				fprintf(stderr,"An error occured when attempting read link.  Error code: %s. File: %s.\n", strerror(errno), path);
				return -1;
			}
			else
				linkBuf[bytesPlaced] = '\0'; //since readlink does not append temerinating null
			printOutput(dp, buf);
			printf("%s -> %s\n", path, linkBuf);
		}
	}
	closedir(dir);
	return 0;
}

int printOutput(struct dirent *dp, struct stat buf){
  char m[11];
  int i =0;
  if(S_ISDIR(buf.st_mode))
    m[i]='d';
  else if(S_ISLNK(buf.st_mode))
    m[i]='l';
  else if(S_ISCHR(buf.st_mode))
    m[i]='c';
  else if(S_ISBLK(buf.st_mode))
    m[i]='b';
  else if(S_ISLNK(buf.st_mode))
    m[i]='l';
  else if(S_ISREG(buf.st_mode))
    m[i]='-';
  else if(S_ISFIFO(buf.st_mode))
    m[i]='p';
  else
    m[i]='?';
  
  i++;
	(buf.st_mode & S_IRUSR) ? (m[i] = 'r') : (m[i] = '-');
	i++;
	(buf.st_mode & S_IWUSR) ? (m[i] = 'w') : (m[i] = '-');
  i++;
	(buf.st_mode & S_IXUSR) ? (m[i] = 'x') : (m[i] = '-');
  i++;
	(buf.st_mode & S_IRGRP) ? (m[i] = 'r') : (m[i] = '-');
	i++;
	(buf.st_mode & S_IWGRP) ? (m[i] = 'w') : (m[i] = '-');
  i++;
	(buf.st_mode & S_IXGRP) ? (m[i] = 'x') : (m[i] = '-');
  i++;
	(buf.st_mode & S_IROTH) ? (m[i] = 'r') : (m[i] = '-');
	i++;
	(buf.st_mode & S_IWOTH) ? (m[i] = 'w') : (m[i] = '-');
  i++;
	(buf.st_mode & S_IXOTH) ? (m[i] = 'x') : (m[i] = '-');
  i++;

	printf("%9lu %6lu %9s %3lu ",buf.st_ino,buf.st_blocks/2, m, buf.st_nlink);

	struct passwd *pwu;
	struct group *grg;
	pwu = getpwuid(buf.st_uid);
	grg = getgrgid(buf.st_gid);
	if(pwu == NULL)
	{
		if(errno)
		{
			fprintf(stderr, "An error occured when attempting to get user id.  Error code: %s. File: %s.\n", strerror(errno), dp->d_name);
			return -1;
		}
		else
			printf("%9du",buf.st_uid);			
	}
	else
		printf("%9s",pwu->pw_name);
	
	if(grg == NULL)
	{
		if(errno)
		{
			fprintf(stderr, "An error occured when attempting to get group id.  Error code: %s. File: %s.\n", strerror(errno), dp->d_name);
			return -1;
		}
		else
			printf("%9du",buf.st_gid);			
	}
	else
		printf("%9s",grg->gr_name);
	
	time_t t = buf.st_mtime;
	struct tm lt;
	localtime_r(&t, &lt);
	char timbuf[80];
	strftime(timbuf, sizeof(timbuf), "%b %d %H:%M", &lt);

	printf("%10lu %10s ",buf.st_size, timbuf);
	return 0;
}