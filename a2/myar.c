#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <ar.h>
#include "myar.h"

char *name; /* Name of the program */

int main(int argc, char *argv[])
{
        /* Parse the flags */
        int opt;
        int q = 0;
        int x = 0;
        int t = 0;
        int d = 0;
        int A = 0;
        int w = 0;
        int v = 0;
        while ((opt = getopt(argc, argv, "qxtdAwv")) != -1) {
                switch (opt) {
                        case 'q':
                                q += 1;
                                break;
                        case 'x':
                                x += 1;
                                break;
                        case 't':
                                t += 1;
                                break;
                        case 'd':
                                d += 1;
                                break;
                        case 'A':
                                A += 1;
                                break;
                        case 'w':
                                w += 1;
                                break;
                        case 'v':
                                v += 1;
                                break;
                        default: /* '?' */
                                usage();
                }
        }

        /* Make sure they specified exactly one non-v flag */
        if (q + x + t + d + A + w != 1) usage();

        /* Make sure they didn't specify the v flag multiple times */
        if (v > 1) usage();

        /* Strip the flags from the arguments */
        int argc2 = argc - optind;
        char **argv2 = &argv[optind];

        /* Perform the action indicated by the given flag. */
        if (q) fq(argc2, argv2);
        else if (x) fx(argc2, argv2);
        else if (t) ft(argc2, argv2);
        else if (d) fd(argc2, argv2);
        else if (A) fA(argc2, argv2);
        else if (w) fw(argc2, argv2);

        exit(EXIT_SUCCESS);
}

/* Print usage info and exit */
void usage()
{
        printf("Usage: %s -<q|x|t|d|A|w>[v] <archive> [file ...]\n",
        name);
        exit(EXIT_FAILURE);
}

/* Print table of contents */
void fq(int argc, char *argv[])
{
        if (argc < 2) usage();
}

/* Print table of contents */
void fx(int argc, char *argv[])
{
        
}

/* Print table of contents */
void ft(int argc, char *argv[])
{
        if (argc != 1) usage();

        for(int i = 0; i < argc; i++){
                printf("%s\n", argv[i]);
        }
}

/* Print table of contents */
void fd(int argc, char *argv[])
{
        
}

/* Print table of contents */
void fA(int argc, char *argv[])
{
        
}

/* Print table of contents */
void fw(int argc, char *argv[])
{
        
}
