#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BACKLOG 1
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
    char buf[BUFSIZE];
    int listenfd, sockfd, bytesread, byteswritten;
    struct sockaddr_in srvaddr, cliaddr;
    socklen_t addrlen = sizeof(srvaddr);

    /* Initialize network address */
    memset(&srvaddr, '\0', addrlen);
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &srvaddr.sin_addr.s_addr);

    /* Bind to address and start listening for a connection */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(bind(listenfd, (struct sockaddr *) &srvaddr, addrlen) != 0) error("Could not bind to address");
    listen(listenfd, BACKLOG);

    /* Accept the connection */
    if((sockfd = accept(listenfd, (struct sockaddr *) &cliaddr, &addrlen)) == -1) error("Could not accept connection");

    /* Read what the client sends and send it back */
    memset(buf, '\0', BUFSIZE);
    if((bytesread = read(sockfd, buf, BUFSIZE)) == -1)
        error("Problem reading from socket");
    else{
        printf("Got message from client: %s\n", buf);
        if((byteswritten = write(sockfd, buf, bytesread)) == -1)
            error("Could not write to socket");
        if(byteswritten != bytesread)
            error("Could not write whole buffer to socket");
    }

    close(listenfd);
    close(sockfd);

    exit(EXIT_SUCCESS);
}
