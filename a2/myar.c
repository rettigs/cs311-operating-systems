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
#include <ar.h>
#include "myar.h"

#define FP_SPECIAL 1
#define STR_SIZE sizeof("rwxrwxrwx")

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
        if (q) fq(argc2, argv2, v);
        else if (x) fx(argc2, argv2, v);
        else if (t) ft(argc2, argv2, v);
        else if (d) fd(argc2, argv2, v);
        else if (A) fA(argv2[0], v);
        else if (w) fw(argc2, argv2, v);

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

/* Return ls(1)-style string for file permissions mask */
char * file_perm_string(mode_t perm, int flags)
{
        static char str[STR_SIZE];
        snprintf(str, STR_SIZE, "%c%c%c%c%c%c%c%c%c",
                (perm & S_IRUSR) ? 'r' : '-', (perm & S_IWUSR) ? 'w' : '-',
                (perm & S_IXUSR) ?
                        (((perm & S_ISUID) && (flags & FP_SPECIAL)) ? 's' : 'x') :
                        (((perm & S_ISUID) && (flags & FP_SPECIAL)) ? 'S' : '-'),
                (perm & S_IRGRP) ? 'r' : '-', (perm & S_IWGRP) ? 'w' : '-',
                (perm & S_IXGRP) ?
                        (((perm & S_ISGID) && (flags & FP_SPECIAL)) ? 's' : 'x') :
                        (((perm & S_ISGID) && (flags & FP_SPECIAL)) ? 'S' : '-'),
                (perm & S_IROTH) ? 'r' : '-', (perm & S_IWOTH) ? 'w' : '-',
                (perm & S_IXOTH) ?
                        (((perm & S_ISVTX) && (flags & FP_SPECIAL)) ? 't' : 'x') :
                        (((perm & S_ISVTX) && (flags & FP_SPECIAL)) ? 'T' : '-'));
        return str;
}

/* Quickly append named files to archive */
void fq(int argc, char *argv[], int v)
{
        if (argc < 2) usage();

        int arfd = open(argv[0], O_RDWR | O_CREAT, 0666);
        if (arfd == -1) error("Could not open or create archive");
        struct stat arstatbuf;
        if (stat(argv[0], &arstatbuf) == -1) error("Could not stat archive");
        if (arstatbuf.st_size == 0) { //The archive file is empty; fill in the header.
                if (write(arfd, ARMAG, SARMAG) != SARMAG) error("Error writing to archive");
        } else {
                isar(arfd); //Make sure it's an archive file.
        }
        
        //Write the given files to the archive.
        for (int i = 1; i < argc; i++) {
                lseek(arfd, 0, SEEK_END);
                struct stat statbuf;
                if (stat(argv[i], &statbuf) == -1) {
                        printf("Could not stat file %s", argv[i]);
                        perror(NULL);
                        break; //Skip to the next file.
                }
                int fd = open(argv[i], O_RDONLY);
                if (fd == -1) {
                        printf("Could not open file %s", argv[i]);
                        perror(NULL);
                        break; //Skip to the next file.
                }

                //Write the file header to the archive.
                struct ar_hdr *hdr = malloc(sizeof(struct ar_hdr));
                strncpy(hdr->ar_name, argv[i], 16);
                snprintf(hdr->ar_date, 12, "%d", statbuf.st_mtime);
                snprintf(hdr->ar_uid, 6, "%d", statbuf.st_uid);
                snprintf(hdr->ar_gid, 6, "%d", statbuf.st_gid);
                snprintf(hdr->ar_mode, 8, "%d", statbuf.st_mode);
                snprintf(hdr->ar_size, 10, "%d", statbuf.st_size);
                strncpy(hdr->ar_fmag, ARFMAG, 2);
                for (int j = 0; j < sizeof(struct ar_hdr); j++) {
                        if (hdr->ar_name[j] == '\0') hdr->ar_name[j] = ' ';
                }
                if (write(arfd, hdr, sizeof(struct ar_hdr)) != sizeof(struct ar_hdr)) error("Error writing to archive");

                //Write the file contents to the archive.
                for (int j = 0; j < (int) strtol(hdr->ar_size, hdr->ar_size + 9, 10); j++) {
                        char byte;
                        int bytes_read = read(fd, &byte, 1);
                        if (bytes_read == 0) { //We hit the end of the file.
                                break; //Skip to the next file.
                        } else if (bytes_read != 1) {
                                printf("Error reading file %s\n", argv[i]);
                                perror(NULL);
                                break; //Skip to the next file.
                        }
                        int bytes_written = write(arfd, &byte, 1);
                        if (bytes_written != 1) error("Error writing to archive");
                }

                //Write an extra newline if the file length is odd.
                if ((int) strtol(hdr->ar_size, hdr->ar_size + 9, 10) % 2) {
                        char newline = '\n';
                        int bytes_written = write(arfd, &newline, 1);
                        if (bytes_written != 1) error("Error writing to archive");
                }
        }
}

/* Extract named files */
void fx(int argc, char *argv[], int v)
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
                        
                }
                printf("%.*s\n", 16, rslash(hdrbuf->ar_name, 16));

                char *arname = hdrbuf->ar_name;
                int filesize = (int) strtol(hdrbuf->ar_size, hdrbuf->ar_size + 9, 10);
                lseek(arfd, filesize + filesize % 2, SEEK_CUR); //Skip to the next file header, taking odd-length file newline padding into account.
        }

        close(arfd);
}

/* Delete named files from archive */
void fd(int argc, char *argv[], int v)
{
        
}

/* Quickly append all regular files in the current directory (except the archive itself) */
void fA(char *arch, int v)
{
        DIR *curdir;
        struct dirent *entry;
        curdir = opendir(".");
        if (curdir == NULL) error("Could not open current directory");
        if (v) printf("Adding all regular files in current directory to archive...\n");
        while ((entry = readdir(curdir)) != NULL) {
                if (entry->d_type == DT_REG){
                        char *argv[2];
                        argv[0] = arch;
                        argv[1] = entry->d_name;
                        fq(2, argv, v);
                }
        }
        if (v) printf("Done adding all regular files in current directory to archive.\n");
}

/* For a given timeout, add all modified files to the archive (except the archive itself) */
void fw(int argc, char *argv[], int v)
{
        
}
