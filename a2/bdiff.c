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
#include <math.h>
#include <sys/param.h>
#include "bdiff.h"

int BUFSIZE = 4096;
char *name; /* Name of the program */

int verbose, offset1, offset2, limit, quiet, recursive;

int main(int argc, char *argv[])
{
        name = argv[0];

        /* Parse the flags */
        verbose = offset1 = offset2 = limit = quiet = recursive = 0;
        int opt;
        while ((opt = getopt(argc, argv, "bi:n:sr")) != -1) {
                switch (opt) {
                        case 'b':
                                verbose = 1;
                                break;
                        case 'i':
                                sscanf(optarg, "%d:%d", &offset1, &offset2);
                                break;
                        case 'n':
                                limit = atoi(optarg);
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
        int status;
        if(recursive) status = rdiff(argv2[0], argv2[1]);
        else status = diff(argv2[0], argv2[1]);

        exit(status);
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

/* Calls diff() recursively on the given directories. */
int rdiff(char *file1, char *file2)
{
        FILE * dir1;
        if((dir1 = opendir(file1)) == NULL) error("Could not open directory");
        struct dirent *entry;
        while((entry = readdir(dir1)) != NULL){
                char *name = &entry->d_name;
                if(strcmp(name, ".") != 0 && strcmp(name, "..") != 0){
                        char path1[512];
                        strcpy(path1, file1);
                        strcat(path1, "/");
                        strcat(path1, name);

                        char path2[512];
                        strcpy(path2, file2);
                        strcat(path2, "/");
                        strcat(path2, name);

                        struct stat statbuf;
                        if(stat(path1, &statbuf) == -1) error(path1);

                        if(S_ISDIR(statbuf.st_mode)){
                                rdiff(path1, path2);
                        }else{
                                printf("%s\n", path2);
                                if(diff(path1, path2) == 0){
                                        printf(" -1 -1 -1 \n");
                                }
                        }
                }
        }
        return 1;
}

/* Diff two files. Returns 0 if same, 1 if different, 2 if error. */
int diff(char *file1, char *file2)
{
        int fd1 = open(file1, O_RDONLY);
        int fd2 = open(file2, O_RDONLY);

        if(fd1 == -1) error("Could not open file");
        if(fd2 == -1){
                printf(" 0 0 0 \n");
                return EXIT_SUCCESS;
        }

        char buf1[BUFSIZE];
        char buf2[BUFSIZE];

        /* Get the number of spaces to pad output with */
        struct stat statbuf1;
        struct stat statbuf2;
        if(fstat(fd1, &statbuf1) == -1 || fstat(fd2, &statbuf2) == -1) error("Could not stat file");
        int pad = ceil(log10(MAX(statbuf1.st_size, statbuf2.st_size)));

        /* Lookup table so we can print special characters */
        char *chars[256] = { "^@", "^A", "^B", "^C", "^D", "^E", "^F", "^G", "^H", "^I", "^J", "^K", "^L", "^M", "^N", 
        "^O", "^P", "^Q", "^R", "^S", "^T", "^U", "^V", "^W", "^X", "^Y", "^Z", "^[", "^\\", "^]", 
        "^^", "^_", " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", 
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", "@", 
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", 
        "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_", "`", 
        "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", 
        "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "^?", 
        "M-^@", "M-^A", "M-^B", "M-^C", "M-^D", "M-^E", "M-^F", "M-^G", "M-^H", "M-^I", "M-^J", 
        "M-^K", "M-^L", "M-^M", "M-^N", "M-^O", "M-^P", "M-^Q", "M-^R", "M-^S", "M-^T", "M-^U", 
        "M-^V", "M-^W", "M-^X", "M-^Y", "M-^Z", "M-^[", "M-^\\", "M-^]", "M-^^", "M-^_", "M- ", 
        "M-!", "M-\"", "M-#", "M-$", "M-%", "M-&", "M-'", "M-(", "M-)", "M-*", "M-+", "M-,", "M--", 
        "M-.", "M-/", "M-0", "M-1", "M-2", "M-3", "M-4", "M-5", "M-6", "M-7", "M-8", "M-9", "M-:", 
        "M-;", "M-<", "M-=", "M->", "M-?", "M-@", "M-A", "M-B", "M-C", "M-D", "M-E", "M-F", "M-G", 
        "M-H", "M-I", "M-J", "M-K", "M-L", "M-M", "M-N", "M-O", "M-P", "M-Q", "M-R", "M-S", "M-T", 
        "M-U", "M-V", "M-W", "M-X", "M-Y", "M-Z", "M-[", "M-\\", "M-]", "M-^", "M-_", "M-`", "M-a", 
        "M-b", "M-c", "M-d", "M-e", "M-f", "M-g", "M-h", "M-i", "M-j", "M-k", "M-l", "M-m", "M-n", 
        "M-o", "M-p", "M-q", "M-r", "M-s", "M-t", "M-u", "M-v", "M-w", "M-x", "M-y", "M-z", "M-{", 
        "M-|", "M-}", "M-~", "M-^?" };

        int compared = 0;
        int read1 = 0;
        int read2 = 0;
        int byteindex = 0;
        int status = 0;

        lseek(fd1, offset1, SEEK_SET);
        lseek(fd2, offset2, SEEK_SET);

        int done = 0;
        while(!done){
                read1 = read(fd1, buf1, BUFSIZE);
                read2 = read(fd2, buf2, BUFSIZE);

                if (read1 > 0 && read2 > 0){
                        for(int i = 0; i <= MAX(read1, read2); i++){
                                byteindex++;
                                if(buf1[i] != buf2[i]){
                                        status = 1;
                                        if(!quiet){
                                                if(verbose) printf("%*d %3o %-4s %3o %s\n", pad, byteindex, buf1[i], chars[buf1[i]], buf2[i], chars[buf2[i]]);
                                                else printf("%*d %3o %3o\n", pad, byteindex, buf1[i], buf2[i]);
                                        }
                                }
                                if(++compared >= limit && limit > 0){
                                        done = 1;
                                        break;
                                }
                        }
                }else{
                        if(read1 == -1 || read2 == -1) error("Could not read file");
                        break;
                }
        }

        close(fd1);
        close(fd2);

        return status;
}
