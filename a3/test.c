#define _BSD_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>

#define FP_SPECIAL 1

char *name; // Name of program

int main(int argc, char *argv[])
{
        name = argv[0];

        // Parse flags
        int opt, threads, infd, outfd;
        threads = 1;
        infd = 0;
        outfd = 1;
        while ((opt = getopt(argc, argv, "n:")) != -1) {
                switch (opt) {
                        case 'n':
                                if (sscanf(optarg, "%d", &threads) != 1) usage();
                                break;
                        default: // '?'
                                usage();
                }
        }
        if (threads < 1) {
                printf("Error: Must specify at least 1 scorer thread\n");
                exit(EXIT_FAILURE);
        }
        if (infd == -1) error("Error: Could not open input file");
        if (outfd == -1) error("Error: Could not open output file");

        int pipefd[2];
        if(pipe(&pipefd) == -1) error("Could not create pipe");

        pid_t pid;
        switch(pid = fork()){
            case -1: // error case
                error("Could not fork process");
            case 0: // child case
                close(pipefd[0]); // close read end
                FILE *wstream = fdopen(pipefd[1], "w");
                char *buf = "hello from child";
                fputs(buf, wstream);
                fclose(wstream);
                _exit(1);
            default: // parent case
                close(pipefd[1]); // close write end
                FILE *rstream = fdopen(pipefd[0], "r");
                char buf2[17];
                fgets(buf2, 17, rstream);
                fclose(rstream);
                printf("Message: %s\n", buf2);
        }

        exit(EXIT_SUCCESS);
}

/* Print usage info and exit */
void usage()
{
        printf("Usage: %s -t <#ofsortthreads> [-i infile] [-o outfile]\n",
        name);
        exit(EXIT_FAILURE);
}

/* Print given error message and exit */
void error(char *message)
{
        perror(message);
        exit(EXIT_FAILURE);
}
