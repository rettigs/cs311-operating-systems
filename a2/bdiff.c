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

#define BUFSIZE=4096

char *name; /* Name of the program */

int verbose, offset1, offset2, limit, quiet, recursive;

int main(int argc, char *argv[])
{
        name = argv[0];

        /* Parse the flags */
        print = offset1 = offset2 = limit = quiet = recursive = 0;
        int opt;
        while ((opt = getopt(argc, argv, "binsr")) != -1) {
                switch (opt) {
                        case 'b':
                                verbose = 1;
                                break;
                        case 'i':
                                offset1 = 0; //TODO
                                offset2 = 0; //TODO
                                break;
                        case 'n':
                                limit = 0; //TODO
                                break;
                        case 's':
                                quiet = 1;
                                break;
                        case 'r':
                                recursive = 1;
                                break;
                        default: /* '?' */
                                usage();
                }
        }

        /* Strip the flags from the arguments */
        int argc2 = argc - optind;
        char **argv2 = &argv[optind];

        /* Diff the files. */
        exit(diff(argv[1], argv[2]));
}

/* Print usage info and exit */
void usage()
{
        printf("Usage: %s [-b] [-i] [-n] [-s] [-r] FILE1 FILE2\n",
        name);
        exit(2);
}

/* Print given error message and exit */
void error(char *message)
{
        perror(message);
        exit(2);
}

/* Removes '/'s from the given char array */
char * rslash(char name[], int len)
{
        for (int i = 0; i < len; i++) {
                if (name[i] == '/') name[i] = '\0';
        }
        return name;
}

/* Converts a char array to a string by replacing whitespace at the end with null terminators */
char * cats(char array[], int len)
{
        char * string[len];
        for (int i = len - 1; i >= 0; i--) {
printf("array[%d]: %c\n", i, array[i]);
                if (array[i] = ' ') {
                        string[i] = '\0';
                } else {
                        string[i] = array[i];
                }
        }
        return string;
}

/* Diff two files. Returns 0 if same, 1 if different, 2 if error. */
int diff(char *file1, char *file2)
{
        int fd1 = open(file1, O_RDONLY)
        int fd2 = open(file2, O_RDONLY)

        if(fd1 == -1 || fd2 == -1) error("Could not open file");

        int buf1[BUFSIZE];
        int buf2[BUFSIZE];

        int read1 = 0;
        int read2 = 0;

        int status = 0;

        for(;;){
                read1 = read(fd1, &buf1, BUFSIZE);
                read2 = read(fd2, &buf2, BUFSIZE);

                if (read1 > 0 && read2 > 0){
                        for(int i = 0; i < max(read1, read2); i++){
                                if(buf1[i] != buf2[i]){
                                        status = 1;
                                        if(verbose) printf("%d\t%o\t%c\t%o\t%c", i, buf1[i], buf1[i], buf2[i], buf2[i]);
                                        else printf("%d\t%o\t%o", i, buf1[i], buf2[i]);
                                }
                        }
                }else{
                        if(read1 == -1 || read2 == -1) status = 2;
                        break;
                }
        }

        close(fd1);
        close(fd2);

        return status;
}
