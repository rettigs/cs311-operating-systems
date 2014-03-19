#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FIFO_PATH "my.fifo"
#define BUFSIZE 4096

/* Print given error message and exit */
void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

/* Send data to self using pipes */
int main(int argc, char *argv[])
{
    char *message = "Hello world";
    char buf[BUFSIZE];
    int pipefd[2];
    int fifofd_r, fifofd_w, bytesread, byteswritten, flags;

    /* Send a message through a pipe and read it back */
    if(pipe(pipefd) == -1) error("Could not create pipe");
    memset(buf, '\0', BUFSIZE);
    printf("Sending message through pipe: %s\n", message);
    if((byteswritten = write(pipefd[1], message, strlen(message))) == -1)
        error("Could not write to pipe");
    else if(byteswritten != strlen(message))
        error("Could not write whole buffer to pipe");
    else{
        if((bytesread = read(pipefd[0], buf, BUFSIZE)) == -1)
            error("Problem reading from pipe");
        if(bytesread != byteswritten)
            printf("Did not get whole message back from pipe\n");
        printf("Got message back from pipe: %s\n", buf);
    }
    close(pipefd[0]);
    close(pipefd[1]);

    /* Send a message through a FIFO and read it back */
    if(mkfifo(FIFO_PATH, 0660) == -1) error("Could not create FIFO");
    if((fifofd_r = open(FIFO_PATH, O_RDONLY | O_NONBLOCK)) == -1) error("Could not open FIFO for reading");
    if((fifofd_w = open(FIFO_PATH, O_WRONLY)) == -1) error("Could not open FIFO for writing");
    flags = fcntl(fifofd_r, F_GETFL);
    flags &= ~O_NONBLOCK;
    fcntl(fifofd_r, F_SETFL, flags);
    memset(buf, '\0', BUFSIZE);
    printf("Sending message through FIFO: %s\n", message);
    if((byteswritten = write(fifofd_w, message, strlen(message))) == -1)
        error("Could not write to FIFO");
    else if(byteswritten != strlen(message))
        error("Could not write whole buffer to FIFO");
    else{
        if((bytesread = read(fifofd_r, buf, BUFSIZE)) == -1)
            error("Problem reading from FIFO");
        if(bytesread != byteswritten)
            printf("Did not get whole message back from FIFO\n");
        printf("Got message back from FIFO: %s\n", buf);
    }
    close(fifofd_r);
    close(fifofd_w);
    if(remove(FIFO_PATH) == -1) error("Could not remove FIFO");

    exit(EXIT_SUCCESS);
}
