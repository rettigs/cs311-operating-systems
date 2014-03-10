#include    <sys/types.h>   /* basic system data types */
#include    <sys/socket.h>  /* basic socket definitions */
#include    <sys/time.h>    /* timeval{} for select() */
#include    <time.h>        /* timespec{} for pselect() */
#include    <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include    <arpa/inet.h>   /* inet(3) functions */
#include    <errno.h>
#include    <fcntl.h>       /* for nonblocking */
#include    <netdb.h>
#include    <signal.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <strings.h>     /* for bzero */
#include    <sys/stat.h>    /* for S_xxx file mode constants */
#include    <sys/uio.h>     /* for iovec{} and readv/writev */
#include    <unistd.h>
#include    <sys/wait.h>
#include    <sys/select.h>

#define LISTENQ     1024    /* 2nd argument to listen() */

/* Miscellaneous constants */
#define MAXLINE     4096    /* max text line length */
#define MAXSOCKADDR  128    /* max socket address structure size */
#define BUFFSIZE    8192    /* buffer size for reads and writes */

/* Define some port number that can be used for client-servers */
#define SERV_PORT        9877           /* TCP and UDP client-servers */
#define SERV_PORT_STR   "9877"          /* TCP and UDP client-servers */


int main(int argc, char **argv) {
    int i;
    int maxi;
    int maxfd;
    int listenfd;
    int connfd;
    int sockfd;
    int nready;
    int client[FD_SETSIZE];

    ssize_t n;
    fd_set rset;
    fd_set allset;
    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr;
    struct sockaddr_in servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SERV_PORT);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);

    maxfd = listenfd;           /* initialize */
    maxi = -1;                  /* index into client[] array */
    for (i = 0; i < FD_SETSIZE; i++){
        client[i] = -1;         /* -1 indicates available entry */
    }

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for ( ; ; ) {
        rset = allset;      /* structure assignment */
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);

        if(nready == -1){
            perror("Something bad happened");
            exit(-1);
        }

        if (FD_ISSET(listenfd, &rset)) {    /* new client connection */
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

            for (i = 0; i < FD_SETSIZE; i++){
                if (client[i] < 0) {
                    client[i] = connfd; /* save descriptor */
                    break;
                }
            }

            if (i == FD_SETSIZE){
                perror("too many clients");
                exit(-1);
            }

            FD_SET(connfd, &allset);    /* add new descriptor to set */
            if (connfd > maxfd)
                maxfd = connfd;         /* for select */
            if (i > maxi)
                maxi = i;               /* max index in client[] array */

            if (--nready <= 0)
                continue;               /* no more readable descriptors */
        }

        for (i = 0; i <= maxi; i++) {   /* check all clients for data */
            if ( (sockfd = client[i]) < 0){
                continue;
            }

            if (FD_ISSET(sockfd, &rset)) {
                if ( (n = read(sockfd, buf, MAXLINE)) == 0) {
                    /* connection closed by client */
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else{
                    write(sockfd, buf, n);
                }

                if (--nready <= 0)
                    break;              /* no more readable descriptors */
            }
        }
    }
}
