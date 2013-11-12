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
#include "uniqify.h"

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
        while ((opt = getopt(argc, argv, "t:i:o:")) != -1) {
                switch (opt) {
                        case 't':
                                if (sscanf(optarg, "%d", &threads) != 1) usage();
                                break;
                        case 'i':
                                infd = open(optarg, O_RDONLY);
                                break;
                        case 'o':
                                outfd = open(optarg, O_WRONLY | O_CREAT, 0666);
                                break;
                        default: // '?'
                                usage();
                }
        }
        if (threads < 1) {
                printf("Error: Must specify at least 1 sort thread\n");
                exit(EXIT_FAILURE);
        }
        if (infd == -1) error("Error: Could not open input file");
        if (outfd == -1) error("Error: Could not open output file");

        // Create tosort pipes
        int * tosortpipes[threads];
        for (int i = 0; i < threads; i++) {
                int pipefds[2];
                tosortpipes[i] = pipefds;
                if (pipe(tosortpipes[i]) == -1) error("Error: Could not create input pipe");
        }

        // Create fromsort pipes
        int * fromsortpipes[threads];
        for (int i = 0; i < threads; i++) {
                int pipefds[2];
                fromsortpipes[i] = pipefds;
                if (pipe(fromsortpipes[i]) == -1) error("Error: Could not create output pipe");
        }

        // Close read pipes

        // Fork off children
        for (int i = 0; i < threads; i++) {
                switch(fork()) {
                        case -1: // Error case
                                error("Could not fork");
                        case 0: // Child case
                                close(tosortpipes[i][0]); //Close the input write end.
                                close(fromsortpipes[i][1]); //Close the output read end.
                                dup2(0, tosortpipes[i][1]); //Make the input read end the new stdin.
                                dup2(1, tosortpipes[i][0]); //Make the output write end the new stdout.
                                //exec
                }
        }

        // Close read ends of tosort pipes and write ends of fromsort pipes
        for (int i = 0; i < threads; i++) {
                close(tosortpipes[i][1]);
                close(fromsortpipes[i][0]);
        }

        // Read and distribute input

        // Close write pipes

        // Write output

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
