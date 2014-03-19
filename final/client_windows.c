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

/* Start an IP to ASN translation service server */
int main()
{
    char *message = "Hello world";
    char buf[BUFSIZE];
	DWORD bytesread, byteswritten;
    SOCKET socks;
    struct sockaddr_in srvaddr;
    int addrlen = sizeof(srvaddr);
	WSADATA wsaData;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) error("Could not initialize Winsock");

    /* Initialize network address */
    memset(&srvaddr, '\0', addrlen);
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(PORT);
	srvaddr.sin_addr.s_addr = inet_addr(IP);

    /* Connect to the server */
    socks = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(connect(socks, (struct sockaddr *) &srvaddr, addrlen) == SOCKET_ERROR) error("Could not connect to address");

    /* Send message to server and print the reply */
    memset(buf, '\0', BUFSIZE);
    if((byteswritten = send(socks, message, strlen(message), 0)) == SOCKET_ERROR)
        error("Could not write to socket");
    else if(byteswritten != strlen(message))
        error("Could not write whole buffer to socket");
    else{
        if((bytesread = recv(socks, buf, BUFSIZE, 0)) == SOCKET_ERROR)
            error("Problem reading from socket");
        if(bytesread != byteswritten)
            printf("Did not get whole message back from server\n");
        printf("Got reply from server: %s\n", buf);
    }

    closesocket(socks);

    exit(EXIT_SUCCESS);
}
