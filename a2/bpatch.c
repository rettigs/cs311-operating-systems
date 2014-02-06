#define _BSD_SOURCE
#define _GNU_SOURCE
#define _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE

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
#include <math.h>
#include <sys/param.h>
#include "bpatch.h"

int BUFSIZE = 4096;
char *name; /* Name of the program */

int recursive;

int main(int argc, char *argv[])
{
        name = argv[0];

        /* Parse the flags */
        recursive = 0;
        int opt;
        while ((opt = getopt(argc, argv, "r")) != -1) {
                switch (opt) {
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

        /* Patch the files. */
        exit(patch(argv2[0]));
}

/* Print usage info and exit */
void usage()
{
        printf("Usage: <patch> | %s [-r] FILE\n",
        name);
        exit(2);
}

/* Print given error message and exit */
void error(char *message)
{
        perror(message);
        exit(2);
}

/* Patches the given file with the patch info from stdin. */
int patch(char *file)
{
        int fd;
        if((fd = open(file, O_WRONLY)) == -1) error("Could not open file");

        char *line = NULL;
        size_t len = 0;
        while(getline(&line, &len, stdin) != -1){
                int index;
                char origbyte;
                sscanf(line, " %d %o %*o ", &index, &origbyte);
                if(origbyte == 0){ // The original file was shorter, so truncate the file we're patching.
                        if(ftruncate(fd, index - 1) == -1) error("Could not truncate file");
                        break;
                }else{
                        lseek(fd, index - 1, SEEK_SET);
                        if(write(fd, &origbyte, 1) < 1) error("Could not write to file");
                }
        }

        free(line);
        close(fd);

        return EXIT_SUCCESS;
}
