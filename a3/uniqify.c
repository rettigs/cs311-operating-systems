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
#include <ar.h>
#include "myar.h"

#define FP_SPECIAL 1

char *name; /* Name of the program */

int main(int argc, char *argv[])
{
        name = argv[0];

        /* Parse the flags */
        int opt, threads, infd, outfd;
        threads = 0;
        infd = 0;
        outfd = 1;
        while ((opt = getopt(argc, argv, "t:i:o:")) != -1) {
                switch (opt) {
                        case 't':
                                threads = sscanf(optarg, "%d");
                                break;
                        case 'i':
                                infd = open(optarg, O_RDONLY);
                                break;
                        case 't':
                                outfd = open(optarg, O_WRONLY);
                                break;
                        default: /* '?' */
                                usage();
                }
        }

        if (threads < 1) {
                printf("Error: Must have at least 1 sort thread");
                exit(EXIT_FAILURE);
        }

        if (infd == -1) {
                printf("Error: Could not open input file");
                exit(EXIT_FAILURE);
        }
        
        if (outfd == -1) {
                printf("Error: Could not open output file");
                exit(EXIT_FAILURE);
        }

        //derp

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

