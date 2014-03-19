#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#define BUFSIZE 4096

/* Print given error message and exit */
void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

/* Send data to self using pipes */
int main()
{
    char *message = "Hello world";
    char buf[BUFSIZE];
    HANDLE pipefd_r = INVALID_HANDLE_VALUE;
    HANDLE pipefd_w = INVALID_HANDLE_VALUE;
	HANDLE npipe = INVALID_HANDLE_VALUE;
	HANDLE npipe_rw = INVALID_HANDLE_VALUE;
	DWORD bytesread, byteswritten;
	LPTSTR npipe_path = TEXT("\\\\.\\pipe\\mynamedpipe");

    /* Send a message through a pipe and read it back */
    if(CreatePipe(&pipefd_r, &pipefd_w, NULL, BUFSIZE) == 0) error("Could not create pipe");
    memset(buf, '\0', BUFSIZE);
    printf("Sending message through pipe: %s\n", message);
    if(WriteFile((HANDLE) pipefd_w, message, strlen(message), &byteswritten, NULL) == FALSE)
        error("Could not write to pipe");
    else if(byteswritten != strlen(message))
        error("Could not write whole buffer to pipe");
    else{
        if(ReadFile(pipefd_r, buf, BUFSIZE, &bytesread, NULL) == FALSE)
            error("Problem reading from pipe");
        if(bytesread != byteswritten)
            printf("Did not get whole message back from pipe\n");
        printf("Got message back from pipe: %s\n", buf);
    }
    CloseHandle(pipefd_r);
    CloseHandle(pipefd_w);

	/* Send a message through a named pipe and read it back */
    npipe = CreateNamedPipe(npipe_path, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT, PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 0, NULL);
	if(npipe == INVALID_HANDLE_VALUE) error("Could not create named pipe");

    memset(buf, '\0', BUFSIZE);

	npipe_rw = CreateFile(npipe_path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if(npipe_rw == INVALID_HANDLE_VALUE) error("Could not open named pipe");

    printf("Sending message through named pipe: %s\n", message);
    if(WriteFile(npipe_rw, message, strlen(message), &byteswritten, NULL) == FALSE)
        error("Could not write to named pipe");
    else if(byteswritten != strlen(message))
        error("Could not write whole buffer to named pipe");
    else{
        if(ReadFile(npipe_rw, buf, BUFSIZE, &bytesread, NULL) == FALSE)
            error("Problem reading from named pipe");
        if(bytesread != byteswritten)
            printf("Did not get whole message back from named pipe\n");
        printf("Got message back from named pipe: %s\n", buf);
    }
    CloseHandle(npipe_rw);
    CloseHandle(npipe);

    exit(EXIT_SUCCESS);
}