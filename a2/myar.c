#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ar.h>
#include "myar.h"

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
                                q = 1;
                                break;
                        case 'x':
                                x = 1;
                                break;
                        case 't':
                                t = 1;
                                break;
                        case 'd':
                                d = 1;
                                break;
                        case 'A':
                                A = 1;
                                break;
                        case 'w':
                                w = 1;
                                break;
                        case 'v':
                                v = 1;
                                break;
                        default: /* '?' */
                                usage(argv[0]);
                }
        }

        /* Make sure they specified exactly one non-v flag */
        if (q + x + t + d + A + w != 1) usage(argv[0]);

        /* Parse the arguments and create file descriptors for each file */
        int fd = 

        /* Perform the action indicated by the given flag. */
        if (q) q(fd, fds);

        exit(EXIT_SUCCESS);
}

/* Print usage info and exit */
void usage(char *name)
{
        printf("Usage: %s -<q|x|t|d|A|w>[v] <archive> [file ...]\n",
        name);
        exit(EXIT_FAILURE);
}

/* Print table of contents */
void q(int fd, int fds[])
{
        
}

/* Print table of contents */
void x(int fd)
{
        
}

/* Print table of contents */
void t(int fd)
{
        
}

/* Print table of contents */
void d(int fd)
{
        
}

/* Print table of contents */
void A(int fd)
{
        
}

/* Print table of contents */
void w(int fd)
{
        
}
