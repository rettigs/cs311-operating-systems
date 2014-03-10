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

#define LISTENQ 1024
#define MAXLINE 4096
#define MAXSOCKADDR 128
#define BUFFSIZE 4096

#define SERV_PORT 9879
#define SERV_PORT_STR "9879"


int main(int argc, char **argv)
{
    int i;
    int sockfd;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE];
    char recvline[MAXLINE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr.s_addr);

    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
    while(fgets(sendline, MAXLINE, stdin) != NULL){
        
        memset(recvline, 0, MAXLINE);

        write(sockfd, sendline, strlen(sendline) + 1);

        if(read(sockfd, recvline, MAXLINE) == 0){
            perror("Something broke");
            exit(-1);
        }
        
        fputs(recvline, stdout);

    }

    return 0;
}
