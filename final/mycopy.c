#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 4096

/* Print usage info and exit */
void usage()
{
    printf("Usage: mycopy SOURCE DEST\n");
    exit(EXIT_FAILURE);
}

/* Print given error message and exit */
void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if(argc != 3) usage();
    
    int infd = open(argv[1], O_RDONLY);
    int outfd = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0664);

    if(infd == -1) error("Could not open input file");
    if(outfd == -1) error("Could not open output file");

    char buf[BUFSIZE];

    int bytesread;
    int byteswritten;
    
    for(;;){
        bytesread = read(infd, buf, BUFSIZE);
        if(bytesread == -1) error("Problem reading from input file");
        else if(bytesread == 0) break;
        else{
            byteswritten = write(outfd, buf, bytesread);
            if(byteswritten != bytesread) error("Problem writing to output file");
        }
    }

    close(infd);
    close(outfd);

    exit(EXIT_SUCCESS);
}
