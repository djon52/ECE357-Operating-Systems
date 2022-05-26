#include <stdio.h>
#include <fcntl.h> /* Defines O_XXX constants */
#include <errno.h> /* Defines error numbers */
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int concat(int fdInfile, int fdOutfile, char *infile);
char buf[4096];
int totalNumOfBytesWritten;
int totalNumOfReadSysCalls;
int totalNumOfWriteSysCalls;

int main(int argc, char *argv[]) 
{
	int catReturn, c;
	int fdInfile = STDIN;
	int fdOutfile = STDOUT;
	char *outfile;
	char *infile;
	int i = 1;

	// checks if argument contains -o, if so will open file provided by next argument
	if(argc!=1 && strcmp(argv[1],"-o") == 0){
		outfile = argv[2];
		fdOutfile = open(outfile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
		i+=2;
		
		if(fdOutfile < 0){
			fprintf(stderr, "An error occured, open output file was not successful. Error code: %s. File: %s.\n", strerror(errno), outfile);		
			return -1;
		}
	}

	if(argc==1 || (argc==3 && strcmp(argv[1],"-o") == 0 && argv[3] == NULL)){
		infile = "<standard input>";
		concat(fdInfile, fdOutfile, infile);

		//closing stuff
		c = close(fdOutfile);
		if(c<0){
			fprintf(stderr, "An error occured, close output file was not successful. Error code: %s. File: %s.\n", strerror(errno), outfile);
			return -1;
		}
		return 0;
	}

	//EOF denoted by CTRL+D
	while(i<argc){
		infile = argv[i];
		if(strcmp(infile,"-") == 0){
			fdInfile = STDIN;
			infile = "<standard input>";
		}
		else{
			fdInfile = open(infile, O_RDONLY, 0666);
			if(fdInfile < 0){
				fprintf(stderr, "An error occured, open input file was not successful. Error code: %s. File: %s.\n", strerror(errno), infile);
				return -1;
			}
		}
    
		catReturn = concat(fdInfile, fdOutfile, infile);
		if(fdInfile != STDIN){
			c = close(fdInfile);
			if(c<0){
				fprintf(stderr, "An error occured, close input file was not successful. Error code: %s. File: %s.\n", strerror(errno), infile);
				return -1;
			}
		}
		i++;
	}
	
	fprintf(stderr, "-------------------------\nTotal number of bytes transferred: %d.\nTotal number of read system calls: %d.\nTotal number of write system calls: %d.\n", totalNumOfBytesWritten, totalNumOfReadSysCalls, totalNumOfWriteSysCalls);
	
	c = close(fdOutfile);
	if(c<0){
		fprintf(stderr, "An error occured, close output file was not successful. Error code: %s. File: %s.\n", strerror(errno), outfile);
		return -1;
	}
	return 0;
}

int concat(int fdInfile, int fdOutfile, char *infile){
	int r, w, lim;
  char *bin = "";
	int numOfBytesWritten = 0;
	int numOfReadSysCalls = 0;
	int numOfWriteSysCalls = 0;
  bool isBinary = false;
	lim = sizeof(buf);
	while((r=read(fdInfile,buf,lim)) > 0){
		numOfReadSysCalls++;
		totalNumOfReadSysCalls++;
    for(int i =0;i<r;i++){
      if(!(isprint(buf[i])||isspace(buf[i]))){
        isBinary = true;
      }
    }
		w = write(fdOutfile,buf,r);
		numOfWriteSysCalls++;
		totalNumOfWriteSysCalls++;
		if(r==w || w==0){
			numOfBytesWritten += w;
			totalNumOfBytesWritten += w;		
		}
		else if (w<0){
			fprintf(stderr, "An error occured, the data was not written. Error code: %s. File: %s.\n", strerror(errno), infile);
			return -1;
		}
		else{
			fprintf(stderr, "Partial write occured, some bytes were not written. Error code: %s. File: %s.\n", strerror(errno), infile);
			return -1;
		}
	}	
	if (r<0){
		fprintf(stderr, "An error has occured, read was not successful. Error code: %s. File: %s.\n", strerror(errno), infile);
		return -1;
	}	  
	if(isBinary)
    bin = "WARNING: Input file is binary.\n";
	fprintf(stderr, "\nConcatentated from input file: %s.\nNumber of bytes transferred: %d.\nNumber of read system calls: %d.\nNumber of write system calls: %d.\n%s\n", infile, numOfBytesWritten, numOfReadSysCalls, numOfWriteSysCalls,bin);
	return 0;			
}