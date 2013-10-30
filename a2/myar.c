#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <ar.h>
#include "myar.h"

char *name; /* Name of the program */

int main(int argc, char *argv[])
{
        /* Parse the flags */
        int opt, q, x, t, d, A, w, v;
        q = x = t = d = A = w = v = 0;
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
        else if (t) ft(argc2, argv2, v);
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

/* Print given error message and exit */
void error(char *message)
{
        perror(message);
        exit(EXIT_FAILURE);
}

/* Verifies the header of a file to check if it's an archive file */
void isar(int fd)
{
      char buf[SARMAG];
      if(read(fd, buf, SARMAG) != SARMAG) error("Could not read archive");
      if(strcmp(buf, ARMAG) != 0) error("No archive specified");    
}

/* Removes '/'s from the given char array */
char* rslash(char name[], int len)
{
        for (int i = 0; i < len; i++) {
                if (name[i] == '/') name[i] = '\0';
        }
        return name;
}

/* Quickly append named files to archive */
void fq(int argc, char *argv[])
{
        if (argc < 2) usage();

        int arfd;
		

 = open(argv[0], O_WRONLY | O_CREAT, 00666);

        if (arfd == -1) error("Could not open archive");

        isar(arfd);
}

/* Extract named files */
void fx(int argc, char *argv[])
{
        
}

/* Print a concise table of contents of the archive */
void ft(int argc, char *argv[], int v)
{
        if (argc != 1) usage();
        
        int arfd = open(argv[0], O_RDONLY);

        if (arfd == -1) error("Could not open archive");

        isar(arfd);
        
        struct ar_hdr *hdrbuf = malloc(sizeof(struct ar_hdr));
        while (1){
                int bytes_read = read(arfd, hdrbuf, sizeof(struct ar_hdr));
                if (bytes_read == -1) error("Error reading archive");
                if (bytes_read != sizeof(struct ar_hdr)) break; //We have hit the end of the file.
                if (v){
                        
                } else {
                        printf("%.*s\n", 16, rslash(hdrbuf->ar_name, 16));
                }
                char *arname = hdrbuf->ar_name;
                int filesize = (int) strtol(hdrbuf->ar_size, hdrbuf->ar_size + 9, 10);
                lseek(arfd, filesize + filesize % 2, SEEK_CUR); //Skip to the next file header, taking odd-length file newline padding into account.
        }

        close(arfd);
}

/* Delete named files from archive */
void fd(int argc, char *argv[])
{
        
}

/* Quickly append all regular files in the current directory (except the archive itself) */
void fA(int argc, char *argv[])
{
        
}

/* For a given timeout, add all modified files to the archive (except the archive itself) */
void fw(int argc, char *argv[])
{
        
}
