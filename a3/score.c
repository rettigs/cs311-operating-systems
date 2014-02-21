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

        /* Create reader -> scorer pipes */
        int rtos[threads][2];
        for (int i = 0; i < threads; i++) {
                rtos[i] = pipefds;
                if (pipe(&rtos[i]) == -1) error("Error: Could not create reader -> scorer pipe");

        /* Spin off reader process */
        pid_t rpid;
        switch(rpid = fork()){
            case -1: // error case
                error("Could not fork reader process");
            case 0: // child case
                reader();
        } // parent continues

        /* Spin off scorer processes */
        pid_t spid[thread];
        for(int i = 0; i < threads; i++){
            switch(spid[i] = fork()){
                case -1: // error case
                    error("Could not fork scorer process");
                case 0: // child case
                    scorer();
            } // parent continues
        }
        switch(rpid = fork()){
            case -1: // error case
                error("could not fork reader process");
            case 0: // child case
                reader();
        } // parent continues


        // Create tosort pipes
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
