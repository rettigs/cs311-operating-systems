#define _GNU_SOURCE
#include <signal.h>

static void sigHandler(int sig){
    _exit();
}

int main(int argc, char **argv)
{
    int child[3];

    for(int i = 0; i < 2; i++){
        int pid;
        switch(pid = fork()){
            case -1: //error
                perror("Could not fork");
                exit(1);
            case 0: //child
                signal(SIGUSR1, sigHandler);
                break;
            default: //parent
                child[i] = pid;
                if(i == 2){
                    for(int j = 0; j < 2; j++){
                        kill(child[j], SIGUSR1);
                    }

                    int status;
                    waitpid(child[1], &status, 0);
                    waitpid(child[0], &status, 0);
                    waitpid(child[2], &status, 0);

                    exit(1);
                }
        }
    }
}
