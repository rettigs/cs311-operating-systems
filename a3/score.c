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
int DEBUG = 0; // Whether we are in debug mode

int main(int argc, char *argv[])
{
    name = argv[0];

    /* Parse flags */
    int opt, threads, outfd;
    threads = 1;
    outfd = 1;
    while((opt = getopt(argc, argv, "t:o:d")) != -1){
        char *outfile = "";
        switch(opt){
            case 't':
                if(sscanf(optarg, "%d", &threads) != 1) usage();
                break;
            case 'o':
                if(sscanf(optarg, "%s", outfile) != 1) usage();
                if((outfd = open(outfile, O_WRONLY | O_TRUNC | O_CREAT, 0644)) == -1) error("Could not open output file");
                break;
            case 'd':
                DEBUG = 1;
                break;
            default: // '?'
                usage();
        }
    }
    if(threads < 1) error("Must specify at least 1 scorer thread\n");

    /* Strip the flags from the arguments */
    int argc2 = argc - optind;
    char **argv2 = &argv[optind];

    /* Create reader -> scorers pipes */
    int rtospipe[threads][2];
    for(int i = 0; i < threads; i++) {
        if(pipe((int *) &rtospipe[i]) == -1) error("Error: Could not create reader -> scorer pipe");
    }

    /* Create scorers -> combiner pipe */
    int stocpipe[2];
    if(pipe((int *) &stocpipe) == -1) error("Error: Could not create scorer -> combiner pipe");

    /* Fork off reader process */
    pid_t rpid;
    switch(rpid = fork()){
        case -1: // Error case
            error("Could not fork off reader process");
        case 0: // Child case
            reader(threads, rtospipe, argc2, argv2);
            _exit(EXIT_SUCCESS);
    } // Parent continues

    /* Fork off scorer processes */
    pid_t spid[threads];
    for(int i = 0; i < threads; i++){
        switch(spid[i] = fork()){
            case -1: // Error case
                error("Could not fork off scorer process");
            case 0: // Child case
                scorer();
                _exit(EXIT_SUCCESS);
        } // Parent continues
    }

    /* Become the combiner process */
    combiner();

    exit(EXIT_SUCCESS);
}

/* Performs the task of the reader process */
void reader(int threads, int (*rtospipe)[2], int filec, char **filev)
{
    debug("Reader spawned");

    for(int i = 0; i < filec; i++){
        
    }
}

/* Performs the task of a scorer process */
void scorer()
{
    printf("Scorer spawned; closing\n");
}

/* Performs the task of the combiner process */
void combiner()
{
    printf("Combiner started; finishing\n");
}

/* Print usage info and exit */
void usage()
{
    printf("Usage: %s [-t scorer_threads] [-o outfile] [-d] FILE...\n", name);
    exit(EXIT_FAILURE);
}

/* Print given error message and exit */
void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

/* Print given message if debug flag is set */
void debug(char *message)
{
    if(DEBUG) printf("%s\n", message);
}
