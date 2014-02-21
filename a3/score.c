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
#include "score.h"

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

        /* Create reader -> scorers pipes */
        int rtospipe[threads][2];
        for(int i = 0; i < threads; i++) {
            if(pipe(&rtospipe[i]) == -1) error("Error: Could not create reader -> scorer pipe");
        }

        /* Create scorers -> combiner pipe */
        int stocpipe[2];
        if(pipe(&stocpipe) == -1) error("Error: Could not create scorer -> combiner pipe");

        /* Fork off reader process */
        pid_t rpid;
        switch(rpid = fork()){
            case -1: // Error case
                error("Could not fork off reader process");
            case 0: // Child case
                reader();
        } // Parent continues

        /* Fork off scorer processes */
        pid_t spid[threads];
        for(int i = 0; i < threads; i++){
            switch(spid[i] = fork()){
                case -1: // Error case
                    error("Could not fork off scorer process");
                case 0: // Child case
                    scorer();
            } // Parent continues
        }

        /* Become the combiner process */
        combiner();

        exit(EXIT_SUCCESS);
}

/* Performs the task of the reader process */
void reader()
{
    printf("Reader spawned; closing\n");
    _exit(0);
}

/* Performs the task of a scorer process */
void scorer()
{
    printf("Scorer spawned; closing\n");
    _exit(0);
}

/* Performs the task of the combiner process */
void combiner()
{
    printf("Combiner started; finishing\n");
    _exit(0);
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
