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
#include "uthash.h"

#define MAX_WORD_SIZE 64

struct wordnode{
    char word[MAX_WORD_SIZE];
    int count;
    UT_hash_handle hh;
};

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
                DEBUG++;
                break;
            default: // '?'
                usage();
        }
    }
    if(threads < 1){
        printf("Must specify at least 1 scorer thread\n");
        exit(EXIT_FAILURE);
    }

    /* Strip the flags from the arguments */
    int argc2 = argc - optind;
    char **argv2 = &argv[optind];

    /* Create reader -> scorers pipes */
    int *rtospipe = (int *) malloc(sizeof(int) * 2 * threads);
    for(int i = 0; i < threads; i++) {
        int pipefd[2];
        if(pipe(pipefd) == -1) error("Error: Could not create reader -> scorer pipe");
        /* Stagger the values because 2D arrays are hard :( */
        rtospipe[i*2] = pipefd[0];
        rtospipe[i*2+1] = pipefd[1];
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
    } // Parent continues

    if(DEBUG) printf("[Main] Forked off reader process\n");

    /* Fork off scorer processes */
    pid_t spid[threads];
    int threadnumber = 0;
    for(int i = 0; i < threads; i++){
        switch(spid[i] = fork()){
            case -1: // Error case
                error("Could not fork off scorer process");
            case 0: // Child case
                scorer(threadnumber, rtospipe, stocpipe);
        } // Parent continues
        threadnumber++;
    }

    /* Become the combiner process */
    combiner(threads, stocpipe, outfd);

    exit(EXIT_SUCCESS);
}

/* Performs the task of the reader process */
void reader(int threads, int *rtospipe, int filec, char **filev)
{
    if(DEBUG) printf("[Reader] Starting\n");

    /* Set up pipes/streams for writing */
    FILE *rtosstreamw[threads];
    for(int i = 0; i < threads; i++){
        if(DEBUG) printf("[Reader] Closing read end of rtos pipe %d (fd %d)\n", i, rtospipe[i*2]);
        close(rtospipe[i*2]); // Close pipe read end
        rtosstreamw[i] = fdopen(rtospipe[i*2+1], "w"); // Open stream for pipe write end
    }

    /* Read the files, parse the words, and send them to the scorer processes */
    int robin = 0;
    for(int i = 0; i < filec; i++){ // For every file...
        int filefd = open(filev[i], O_RDONLY);
        if(filefd == -1) error("Could not open input file");
        FILE *filestream = fdopen(filefd, "r");
        if(filestream == NULL) error("Could not open input file stream");
        if(DEBUG) printf("[Reader] Opened file %s\n", filev[i]);

        /* Find words in the file by reading each char and storing it in a word buffer until we hit a non-letter, non-hyphen, non-apostrophe character */
        char word[MAX_WORD_SIZE];
        int wordlen = 0;
        char c;
        int isword = 0;
        while((c = fgetc(filestream))){
            if(DEBUG > 1) printf("[Reader] Got char: %c (%d)\n", c, c);
            if(c == EOF) break;
            else if(c == '\'' || c == '-' || isalpha(c)){ // If the char is a letter, apostrophe, or hyphen, add it to the word buffer
                word[wordlen] = tolower(c);
                wordlen++;
                isword = 1;
            }else if(isword){ // Otherwise, the word is over, so terminate it and hand it off
                word[wordlen] = '\0';
                wordlen = 0;
                isword = 0;
                if(DEBUG > 1) printf("[Reader] Next word (going to scorer %d) is: %s\n", robin % threads, word);
                fputs(word, rtosstreamw[robin % threads]); // Hand the word to one of the scorer processes, round-robin style
                fputs("\n", rtosstreamw[robin % threads]); // Also send a newline to delimit them.
                robin++;
            }
        }
    }

    /* Close streams */
    for(int i = 0; i < threads; i++){
        fclose(rtosstreamw[i]);
    }

    if(DEBUG) printf("[Reader] Terminating\n");
    _exit(EXIT_SUCCESS);
}

/* Performs the task of a scorer process */
void scorer(int threadnumber, int *rtospipe, int *stocpipe)
{
    if(DEBUG) printf("[Scorer %d] Started\n", threadnumber);

    /* Set up pipe/stream for reading */
    FILE *rtosstreamr;
    if(DEBUG) printf("[Scorer %d] Closing write end of rtos pipe %d (fd %d)\n", threadnumber, threadnumber, rtospipe[threadnumber*2+1]);
    close(rtospipe[threadnumber*2+1]); // Close pipe write end
    rtosstreamr = fdopen(rtospipe[threadnumber*2], "r"); // Open stream for pipe read end

    /* Set up pipe/stream for writing */
    FILE *stocstreamw;
    if(DEBUG) printf("[Scorer %d] Closing read end of stoc pipe (fd %d)\n", threadnumber, stocpipe[0]);
    close(stocpipe[0]); // Close pipe read end
    stocstreamw = fdopen(stocpipe[1], "w"); // Open stream for pipe write end

//    char word[MAX_WORD_SIZE];
//    for(;;){
//        int result;stocpipe[0]
//        result = fscanf(rtosstreamr, "%s", &word);
//        printf("result: %d\n", result);
//        fflush(NULL);
//        if(result == -1) break;
//        if(DEBUG > 1) printf("[Scorer %d] Got word of len %d: %s\n", threadnumber, (int) strlen(word), word);
//    }

    /* Set up hashmap for storing words and their counts */
    struct wordnode *wordhash = NULL;

    /* Get words */
    char newword[MAX_WORD_SIZE];
    while(fgets(newword, MAX_WORD_SIZE, rtosstreamr) != NULL){
        newword[strlen(newword)-1] = '\0';
        if(DEBUG > 1) printf("[Scorer %d] Got word of len %d: %s\n", threadnumber, (int) strlen(newword), newword);

        /* Update word count in hashmap */
        struct wordnode *wordentry = (struct wordnode *) malloc(sizeof(struct wordnode));
        HASH_FIND_STR(wordhash, newword, wordentry);
        if(wordentry == NULL){ // If it's not already in the hashmap, then add it
            wordentry = (struct wordnode *) malloc(sizeof(struct wordnode));
            strcpy(wordentry->word, newword);
            wordentry->count = 1;
            HASH_ADD_STR(wordhash, word, wordentry);
        }else{ // If it is already in the hashmap, then increment the count
            HASH_DEL(wordhash, wordentry);
            wordentry->count++;
            HASH_ADD_STR(wordhash, word, wordentry);
        }
    }

    /* Send the score info to the combiner process */
    struct wordnode *s = (struct wordnode *) malloc(sizeof(struct wordnode));
    for(s = wordhash; s != NULL; s = s->hh.next){
        char line[10+1+MAX_WORD_SIZE]; // For a count of type int (max 10 digits), a space, and a word 
        sprintf(line, "%d %s\n", s->count, s->word);
        if(DEBUG > 1) printf("[Scorer %d] Sending to combiner: %s", threadnumber, line);
        fputs(line, stocstreamw);
    }

    /* Close streams */
    if(DEBUG) printf("[Scorer %d] Closing reader -> scorer stream\n", threadnumber);
    fclose(rtosstreamr);
    if(DEBUG) printf("[Scorer %d] Closing scorer -> combiner stream\n", threadnumber);
    fclose(stocstreamw);

    if(DEBUG) printf("[Scorer %d] Terminating\n", threadnumber);
    fflush(NULL);
    _exit(EXIT_SUCCESS);
}

/* Performs the task of the combiner process */
void combiner(int threads, int *stocpipe, int outfd)
{
    if(DEBUG) printf("[Combiner] Started\n");

    /* Set up pipe/stream for reading */
    FILE *stocstreamr;
    if(DEBUG) printf("[Combiner] Closing write end of stoc pipe (fd %d)\n", stocpipe[1]);
    close(stocpipe[1]); // Close pipe read end
    stocstreamr = fdopen(stocpipe[0], "r"); // Open stream for pipe read end

    /* Set up stream for writing */
    FILE *ctoostreamw;
    ctoostreamw = fdopen(outfd, "w"); // Open stream for output

    /* Set up hashmap for storing words and their counts */
    struct wordnode *wordhash = NULL;

    /* Get words and their counts */
    int newcount;
    char newword[MAX_WORD_SIZE];
    char newline[10+1+MAX_WORD_SIZE];
    while(fgets(newline, 10+1+MAX_WORD_SIZE, stocstreamr) != NULL){
        newline[strlen(newline)-1] = '\0';

        if(sscanf(newline, "%d %s ", &newcount, newword) < 1) error("Error reading from scorer -> combiner stream");

        /* Update word count in hashmap */
        struct wordnode *wordentry = (struct wordnode *) malloc(sizeof(struct wordnode));
        HASH_FIND_STR(wordhash, newword, wordentry);
        if(wordentry == NULL){ // If it's not already in the hashmap, then add it
            wordentry = (struct wordnode *) malloc(sizeof(struct wordnode));
            strcpy(wordentry->word, newword);
            wordentry->count = 1;
            HASH_ADD_STR(wordhash, word, wordentry);
        }else{ // If it is already in the hashmap, then sum the counts
            HASH_DEL(wordhash, wordentry);
            wordentry->count += newcount;
            HASH_ADD_STR(wordhash, word, wordentry);
        }
    }

    /* Close streams */
    if(DEBUG) printf("[Combiner] Closing scorer -> combiner stream\n");
    fclose(stocstreamr);
    if(DEBUG) printf("[Combiner] Closing combiner -> output stream\n");
    fclose(ctoostreamw);

    if(DEBUG) printf("[Combiner] Terminating\n");
}

/* Print usage info and exit */
void usage()
{
    printf("Usage: %s [-t scorer_threads] [-o outfile] [-d]... FILE...\n", name);
    exit(EXIT_FAILURE);
}

/* Print given error message and exit */
void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}
