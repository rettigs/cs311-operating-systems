#include <stdio.h>
#if defined _WIN32 || defined WIN32 || WIN64 /* Support Windows */
#   include <windows.h>
#   include <tchar.h>
#   include <strsafe.h>

#   ifndef WIN
#       define WIN /* Convenience variable */
#   endif
#   ifndef BCOUNT
#       define BCOUNT DWORD /* How to store byte count */
#   endif
#   ifndef FTYPE
#       define FTYPE HANDLE /* How to store open file references */
#   endif
#elif defined __unix__ /* Support UNIX/Linux */
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <stdlib.h>
#   include <unistd.h>

#   ifndef BCOUNT
#       define BCOUNT ssize_t /* How to store byte count */
#   endif
#   ifndef FTYPE
#       define FTYPE int /* How to store open file references */
#   endif
#else /* Nothing else is supported */
#   error "Unsupported platform"
#endif
#ifndef BUFSIZE
#   define BUFSIZE 4096
#endif

/* Print usage info and exit */
void usage()
{
#ifdef WIN
    printf("Usage: mycopy.exe SOURCE DEST\n");
#else
    printf("Usage: ./mycopy SOURCE DEST\n");
#endif
    exit(EXIT_FAILURE);
}

/* Print given error message and exit */
void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

/* Copy the file at argv[1] to the file at argv[2] in increments of BUFSIZE */
#ifdef WIN
int __cdecl _tmain(int argc, TCHAR *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    FTYPE infile, outfile;
    BCOUNT bytesread, byteswritten;
    char buf[BUFSIZE];

    if(argc != 3) usage();

#ifdef WIN
    infile = CreateFile (argv[1], GENERIC_READ, FILE_SHARE_READ,
                         NULL, OPEN_EXISTING, 0, NULL);
    outfile = CreateFile(argv[2], GENERIC_WRITE, 0,
                         NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    infile = open(argv[1], O_RDONLY);
    outfile = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0664);
#endif
    
#ifndef NOFILE
#   ifdef WIN
#       define NOFILE INVALID_HANDLE_VALUE
#   else
#       define NOFILE -1
#   endif
#endif

    if(infile == NOFILE) error("Could not open input file");
    if(outfile == NOFILE) error("Could not open output file");

    for(;;){
#ifdef WIN
        if(ReadFile(infile, buf, BUFSIZE, &bytesread, NULL) == FALSE)
#else
        if((bytesread = read(infile, buf, BUFSIZE)) == -1)
#endif
            error("Problem reading from input file");
        else if(bytesread == 0)
            break;
        else{
#ifdef WIN
            if(WriteFile(outfile, buf, bytesread, &byteswritten, NULL) == FALSE)
#else
            if((byteswritten = write(outfile, buf, bytesread)) == -1)
#endif
                error("Could not write to output file");
            if(byteswritten != bytesread)
                error("Could not write whole buffer to output file");
        }
    }

#ifdef WIN
    CloseHandle(infile);
    CloseHandle(outfile);
#else
    close(infile);
    close(outfile);
#endif

    exit(EXIT_SUCCESS);
}
