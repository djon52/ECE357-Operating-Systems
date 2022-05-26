#include <unistd.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char **argv) {
  int fd[2];
  if (pipe(fd) < 0) {
    fprintf(stderr, "Can't create pipe: %s\n", strerror(errno));
    return -1;
  }
  printf("Pipe successfully made\n");
  fcntl(fd[1],F_SETFL, O_NONBLOCK);
  int bytes=0, totalBytes=0;
  for(;;){
    if((bytes = write(fd[1],"test",4))<=0){
      printf("Total Bytes Written: %d\n",totalBytes-totalBytes%256);
      fprintf(stderr, "Pipe full: %s\n", strerror(errno));
      /*Error message should be "Resource temporarily unavailable" corresponding to error EAGAIN*/
      return -1;
    }
    totalBytes+=bytes;
  }
  return 0;
}