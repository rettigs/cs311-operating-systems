#define _POSIX_SOURCE || _POSIX_C_SOURCE >= 1
#define _XOPEN_SOURCE
#define _BSD_SOURCE

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "score.h"

#define MAX_WORD_LENGTH 4096

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
    int rtospipe = malloc([threads][2];
    for(int i = 0; i < threads; i++) {
        if(pipe((int *) &rtospipe[i]) == -1) error("Error: Could not create reader -> scorer pipe");
    }

    /* Create scorers -> combiner pipe */
    int stocpipe[2];
    if(pipe((int *) &stocpipe) == -1) error("Error: Could not create scorer -> combiner pipe");

//    /* Fork off reader process */
//    pid_t rpid;
//    switch(rpid = fork()){
//        case -1: // Error case
//            error("Could not fork off reader process");
//        case 0: // Child case
//            reader(threads, (int **) rtospipe, argc2, argv2);
//    } // Parent continues
//
//    printf("[Main] Forked off reader process\n");
//
//    /* Fork off scorer processes */
//    pid_t spid[threads];
//    int threadnumber = 0;
//    for(int i = 0; i < threads; i++){
//        switch(spid[i] = fork()){
//            case -1: // Error case
//                error("Could not fork off scorer process");
//            case 0: // Child case
//                scorer(threadnumber, (int **) rtospipe, (int *) stocpipe);
//        } // Parent continues
//        threadnumber++;
//    }
//
//    printf("[Main] Forked off %d scorer processes\n", threads);
//
//    int status;
//    wait(&status);

    reader(threads, (int **) rtospipe, argc2, argv2);

    /* Become the combiner process */
    combiner();

    exit(EXIT_SUCCESS);
}

/* Performs the task of the reader process */
void reader(int threads, int **rtospipe, int filec, char **filev)
{
    printf("Reader DERPEDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
    printf("Reader DERPEDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");

    printf("Reader DERPEDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
    /* Set up pipes/streams for writing */
    printf("Reader DERPEDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
    FILE *rtosstreamw[threads];
    printf("Reader DERPEDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
    for(int i = 0; i < threads; i++){
    printf("Reader DERPEDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
        printf("Reader closing read end of rtos pipe %d (fd %d)\n", i, rtospipe[i][0]);
        close(rtospipe[i][0]); // Close pipe read end
    printf("Reader DERPEDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
        rtosstreamw[i] = fdopen(rtospipe[i][1], "w"); // Open stream for pipe write end
    printf("Reader DERPEDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
    }
    printf("Reader DERPEDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");

    /* Read the files, parse the words, and send them to the scorer processes */
    int robin = 0;
    for(int i = 0; i < filec; i++){ // For every file...
        int filefd = open(filev[i], O_RDONLY);
        if(filefd == -1) error("Could not open input file");
        FILE *filestream = fdopen(filefd, "r");
        if(filestream == NULL) error("Could not open input file stream");
        debug("Opened file");

        /* Find words in the file by reading each char and storing it in a word buffer until we hit a non-letter, non-hyphen, non-apostrophe character */
        char word[MAX_WORD_LENGTH];
        int wordlen = 0;
        char c;
        while((c = fgetc(filestream)) != EOF){
            if(c == '\'' || c == '-' || isalpha(c)){ // If the char is a letter, apostrophe, or hyphen, add it to the word buffer
                word[wordlen] = tolower(c);
                wordlen++;
            }else{ // Otherwise, the word is over, so terminate it and hand it off
                word[wordlen] = '\n';
                wordlen = 0;
                fputs(word, rtosstreamw[robin++ % threads]); // Hand the word to one of the scorer processes, round-robin style
            }
        }
    }

    /* Close streams */
    for(int i = 0; i < threads; i++){
        fclose(rtosstreamw[i]);
    }

    debug("Reader terminating");
    _exit(EXIT_SUCCESS);
}

/* Performs the task of a scorer process */
void scorer(int threadnumber, int **rtospipe, int *stocpipe)
{
    printf("Scorer %d spawned\n", threadnumber);

    /* Set up pipe/stream for reading */
    FILE *rtosstreamr;
        printf("Scorer %d closing write end of rtos pipe %d\n", threadnumber, threadnumber);
    close(rtospipe[threadnumber][1]); // Close pipe write end
        printf("derp\n");
    rtosstreamr = fdopen(rtospipe[threadnumber][0], "r"); // Open stream for pipe read end
        printf("derp\n");

    /* Set up pipe/stream for writing */
    FILE *stocstreamw;
    close(stocpipe[0]); // Close pipe read end
    stocstreamw = fdopen(stocpipe[1], "w"); // Open stream for pipe write end
        printf("derp\n");

    char word[MAX_WORD_LENGTH];
    for(;;){
        printf("derp\n");
        if(fgets(word, MAX_WORD_LENGTH, rtosstreamr) == NULL) break;
        printf("%s\n", word);
    }

    /* Close streams */
    fclose(rtosstreamr);
    fclose(stocstreamw);

    debug("Scorer terminating");
    _exit(EXIT_SUCCESS);
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
