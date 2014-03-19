#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PORT 54321
#define BUFSIZE 4096

/* Print given error message and exit */
void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

/* Start an IP to ASN translation service server */
int main(int argc, char *argv[])
{
    char *message = "Hello world";
    char buf[BUFSIZE];
    int sockfd, bytesread, byteswritten;
    struct sockaddr_in srvaddr;
    socklen_t addrlen = sizeof(srvaddr);

    /* Initialize network address */
    memset(&srvaddr, '\0', addrlen);
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &srvaddr.sin_addr.s_addr);

    /* Connect to the server */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(sockfd, (struct sockaddr *) &srvaddr, addrlen) == -1) error("Could not connect to address");

    /* Send message to server and print the reply */
    memset(buf, '\0', BUFSIZE);
    if((byteswritten = write(sockfd, message, strlen(message))) == -1)
        error("Could not write to socket");
    else if(byteswritten != strlen(message))
        error("Could not write whole buffer to socket");
    else{
        if((bytesread = read(sockfd, buf, BUFSIZE)) == -1)
            error("Problem reading from socket");
        if(bytesread != byteswritten)
            printf("Did not get whole message back from server\n");
        printf("Got reply from server: %s\n", buf);
    }

    close(sockfd);

    exit(EXIT_SUCCESS);
}
