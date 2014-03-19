#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

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

/* Start an echo server */
int main()
{
    char buf[BUFSIZE];
	DWORD bytesread, byteswritten;
    SOCKET listens, socks;
    struct sockaddr_in srvaddr, cliaddr;
    int addrlen = sizeof(srvaddr);
	WSADATA wsaData;
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) error("Could not initialize Winsock");

    /* Initialize network address */
    memset(&srvaddr, '\0', addrlen);
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(PORT);
	srvaddr.sin_addr.s_addr = inet_addr(IP);

    /* Bind to address and start listening for a connection */
    listens = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(bind(listens, (struct sockaddr *) &srvaddr, addrlen) == INVALID_SOCKET) error("Could not bind to address");
    listen(listens, BACKLOG);

    /* Accept the connection */
    if((socks = accept(listens, (struct sockaddr *) &cliaddr, &addrlen)) == -1) error("Could not accept connection");

    /* Read what the client sends and send it back */
    memset(buf, '\0', BUFSIZE);
    if((bytesread = recv(socks, buf, BUFSIZE, 0)) == -1)
        error("Problem reading from socket");
    else{
        printf("Got message from client: %s\n", buf);
        if((byteswritten = send(socks, buf, bytesread, 0)) == -1)
            error("Could not write to socket");
        if(byteswritten != bytesread)
            error("Could not write whole buffer to socket");
    }

	closesocket(socks);
	closesocket(listens);

    exit(EXIT_SUCCESS);
}
