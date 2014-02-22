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
#include <math.h>
#include <signal.h>
#include "search.h"
#include "uthash.h"

#define MAX_WORD_SIZE 64
#define MAX_PATH_SIZE 256

struct wordnode{
    char word[MAX_WORD_SIZE];
    int count;
    UT_hash_handle hh;
};

struct filenode{
    char path[MAX_PATH_SIZE];
    double relevance;
    struct wordnode *wordhash;
    UT_hash_handle hh;
};

// tf-idf of a term in a document = termcount * log(totaldocs / founddocs)

char *name; // Name of program
int DEBUG = 0; // Whether we are in debug mode
int infd = 1; // Where to read data

int main(int argc, char *argv[])
{
    name = argv[0];

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if(sigaction(SIGINT, &sa, NULL) == -1) error("Could not register signal handler");
    if(sigaction(SIGQUIT, &sa, NULL) == -1) error("Could not register signal handler");
    if(sigaction(SIGHUP, &sa, NULL) == -1) error("Could not register signal handler");

    /* Parse flags */
    int opt;
    while((opt = getopt(argc, argv, "i:d")) != -1){
        char outfile[MAX_PATH_SIZE];
        switch(opt){
            case 'i':
                if(sscanf(optarg, "%s", outfile) != 1) usage();
                if((infd = open(outfile, O_RDONLY)) == -1) error("Could not open input file");
                break;
            case 'd':
                DEBUG++;
                break;
            default: // '?'
                usage();
        }
    }

    /* Strip the flags from the arguments */
    int argc2 = argc - optind;
    char **argv2 = &argv[optind];

    //read input into hashmaps for each file
    //for each term...
        //check each hashmap for the term to get founddocs
        //spin off tfidf processes for each file to get tfidf score for each document
        //add tfidf score for term to the document's relevance score
    //print documents in order of highest relevance score to lowest

    /* Set up stream for reading */
    FILE *instream;
    instream = fdopen(infd, "r"); // Open stream for input

    /* Set up hashmap for storing file paths and their corresponding word hashmaps */
    struct filenode *filehash = NULL;

    /* Loop through all segments in input data */
    char newpath[MAX_PATH_SIZE];
    while(fscanf(instream, ": %s ", newpath) > 0){ // For every segment in the file...

        /* Set up hashmap for storing words and their counts */
        struct wordnode *newwordhash = NULL;

        /* Get words and their counts */
        int newcount;
        char newword[MAX_WORD_SIZE];
        while(fscanf(instream, "%d %s ", &newcount, newword) > 1){ // For every word count line...
            /* Update word count in hashmap */
            struct wordnode *wordentry = (struct wordnode *) malloc(sizeof(struct wordnode));
            HASH_FIND_STR(newwordhash, newword, wordentry);
            if(wordentry == NULL){ // If it's not already in the hashmap, then add it
                wordentry = (struct wordnode *) malloc(sizeof(struct wordnode));
                strcpy(wordentry->word, newword);
                wordentry->count = newcount;
                HASH_ADD_STR(newwordhash, word, wordentry);
            }else{ // If it is already in the hashmap, then sum the counts
                HASH_DEL(newwordhash, wordentry);
                wordentry->count += newcount;
                HASH_ADD_STR(newwordhash, word, wordentry);
            }
        }

        /* Add the file and its hashmap to the file hashmap */
        struct filenode *fileentry = (struct filenode *) malloc(sizeof(struct filenode));
        strcpy(fileentry->path, newpath);
        fileentry->relevance = 0.0;
        fileentry->wordhash = newwordhash;
        HASH_ADD_STR(filehash, path, fileentry);
    }

    int totaldocs, founddocs;
    for(int i = 0; i < argc2; i++){ // For every search term...
        char *term = argv2[i];
        totaldocs = founddocs = 0;

        /* First pass through files to update totaldocs and founddocs */
        struct filenode *fileentry = (struct filenode *) malloc(sizeof(struct filenode));
        for(fileentry = filehash; fileentry != NULL; fileentry = fileentry->hh.next){ // For every file...
            struct wordnode *wordentry = (struct wordnode *) malloc(sizeof(struct wordnode));
            HASH_FIND_STR(fileentry->wordhash, term, wordentry);
            if(wordentry != NULL){ // If the search term is in the file...
                founddocs++; // Increment founddocs
            }
            totaldocs++;
        }

        if(DEBUG) printf("founddocs for term '%s': %d\n", term, founddocs);

        /* Second pass through files to calculate tfidf for each one */
        for(fileentry = filehash; fileentry != NULL; fileentry = fileentry->hh.next){ // For every file...
            struct wordnode *wordentry = (struct wordnode *) malloc(sizeof(struct wordnode));
            HASH_FIND_STR(fileentry->wordhash, term, wordentry);
            if(wordentry != NULL){ // If the search term is in the file...
                fileentry->relevance += tfidf(wordentry->count, totaldocs, founddocs);
            }
        }
    }

    /* Print relevance of each document */
    struct filenode *fileentry = (struct filenode *) malloc(sizeof(struct filenode));
    for(fileentry = filehash; fileentry != NULL; fileentry = fileentry->hh.next){ // For every file...
        /*if(fileentry->relevance > 0.0) */printf("File: %s\tRelevance: %f\n", fileentry->path, fileentry->relevance);
    }

    exit(EXIT_SUCCESS);
}

/* Why the hell does this need its own process */
double tfidf(int termcount, int totaldocs, int founddocs){
    if(founddocs < 1) return 0;
    else return ((double) termcount) * log(((double) totaldocs) / ((double) founddocs));
}

/* Handles termination signals */
void handler(int sig)
{
    printf("Terminating cleanly\n");
    fflush(NULL);
    _exit(EXIT_FAILURE);
}

/* Print usage info and exit */
void usage()
{
    printf("Usage: %s [-i infile] [-d]... TERM...\n", name);
    exit(EXIT_FAILURE);
}

/* Print given error message and exit */
void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}
