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

int signo;

void handler(int sig)
{	
	fprintf(stderr, "Signal %s received\n",strsignal(sig));
	exit(sig);
}

int createFile(size_t len)
{
	char buf[1];
	int fd = open("testFile.txt", O_RDWR|O_CREAT|O_TRUNC, 0666);
	for(int i = 0; i < len; i++)
	{
		buf[0] = 'A';
		write(fd,buf,1);
	}
	
	return fd;
}

void mtest1()
{
	printf("Executing Test #1 (write to r/o mmap):\n");
	char *map;
	
	size_t len = 256;	
	int fd = createFile(len);
			
	struct sigaction sa;
	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	
	for(int i = 1; i < 32; i++)
		sigaction(i,&sa,0);
	
	if((map=mmap(NULL,len,PROT_READ,MAP_SHARED,fd,0)) == MAP_FAILED)
		fprintf(stderr, "An error occured, failed to mmap file\n");
		
	printf("map[3]==\'%c\'\n",map[3]);
	printf("Writing a \'B\'\n");
	
	if(munmap(map,len)<0)
		fprintf(stderr, "An error occured, failed to unmap file\n");
	
	if((map[3] = 'B') != 'B')
		exit(255);
	
	if(close(fd)<0)
		fprintf(stderr, "An error occured, failed to close file\n");
	
	exit(0);	
}


void mtest23(int test, int mapPerm)
{
	printf("Executing Test #%d (write to MAP_SHARED/MAP_PRIVATE):\n",test);
	char *map;
	char buf[1];
	int updateVisible;
	size_t len = 256;	
	int fd = createFile(len);	
	
	if((map = mmap(NULL,len,PROT_READ|PROT_WRITE,mapPerm,fd,0)) == MAP_FAILED)
		fprintf(stderr, "An error occured, failed to mmap file\n");
	
	printf("map[3]==\'%c\'\n",map[3]);
	printf("Writing a \'B\'\n");
	
	map[3] = 'B';
	
	printf("new map[3]==\'%c\'\n",map[3]);
	
	if(lseek(fd,3,SEEK_SET)<0)
		fprintf(stderr, "An error occured, failed to lseek test file\n");
	
	if(read(fd,buf,1)<0)
		fprintf(stderr, "An error occured, failed to read byte from test file\n");
	
	printf("read == \'%c\'\n",buf[0]);
	if(buf[0] == 'B')
	{
		updateVisible = 0;
		printf("Update is visible\n");
	}
	else
	{
		updateVisible = 1;
		printf("Update not visible \n");
	}
	
	if(munmap(map,len)<0)
		fprintf(stderr, "An error occured, failed to unmap file\n");
	
	if(close(fd)<0)
		fprintf(stderr, "An error occured, failed to close file\n");
	
	exit(updateVisible);
	
}

void mtest4()
{
	printf("Executing Test #4 (writing into a hole):\n");
	char *map;
	int fd, offset, ans, buf[1];
	size_t len = 4000;
	fd = createFile(len);

	if ((map = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		fprintf(stderr, "An error occured, failed to mmap file\n");
		exit(255);
	}
	
	offset = len+1;
	printf("map[4001]==\'%c\'\n",map[offset]);
	printf("Writing a \'X\'\n");
	map[offset] = 'X';
	printf("New map[4001]==\'%c\'\n",map[offset]);
	
	if(lseek(fd, (len+16), SEEK_SET) == -1) {
		fprintf(stderr, "An error occured, failed to increase file size\n");
		exit(255);
	}
	buf[0] = 0;
	if(write(fd, buf, 1) == -1) {
		fprintf(stderr, "An error occured, failed to write at end of file\n");
		exit(255);
	}
	if(lseek(fd, len+1, SEEK_SET) == -1) {
		fprintf(stderr, "An error occured, failed to set offset\n");
		exit(255);
	}
	if(read(fd, buf, 1) == -1) {
		fprintf(stderr, "An error occured, failed to read file\n");
		exit(255);
	}
	printf("Read from file returns: \'%c\'\n", buf[0]);
	if (buf[0] == 'X') {
		ans = 0;
	}
	else{
		ans = 1;
	}
	
	if (munmap(map, len) == -1) {
		fprintf(stderr, "An error occured, failed to unmap file\n");
		exit(255);
	}
	if (close(fd) == -1) {
		fprintf(stderr, "An error occured, failed to close file\n");
		exit(255);
	}
	if(ans){
		printf("Writing to hole in mapped memory is not visible.\n");
		exit(1);
	}
	else{
		printf("Writing to hole in mapped memory is visible.\n");
		exit(0);
	}	
	
}


int main(int argc, char **argv)
{
	
	if(argc != 2)
	{
		fprintf(stderr, "An error occured, proper usage is: ./mtest [test # 1,2,3,or 4]\n");
		return -1;
	}
	switch(atoi(argv[1]))
	{
		case 1:
			mtest1();
			break;
		case 2:
			mtest23(atoi(argv[1]),MAP_SHARED);
			break;
		case 3:
			mtest23(atoi(argv[1]),MAP_PRIVATE);
			break;
		case 4:
			mtest4();
			break;
	}
}