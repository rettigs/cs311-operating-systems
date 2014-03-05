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
#include <pthread.h>
#include "trouter.h"
//#include "trienode.h"

#define MAX_WORD_SIZE 64
#define MAX_PATH_SIZE 256

char *name; // Name of program
int DEBUG = 0; // Whether we are in debug mode
int n = 1; // Number of worker threads to use

/* Do BGP stuff with a trie using multiple threads */
int main(int argc, char *argv[])
{
    name = argv[0];

    /* Parse flags */
    int opt;
    while((opt = getopt(argc, argv, "n:d")) != -1){
        switch(opt){
            case 'n':
                if(sscanf(optarg, "%d", &n) != 1) usage();
                break;
            case 'd':
                DEBUG++;
                break;
            default: // '?'
                usage();
        }
    }
    if(n < 1){
        printf("Must specify at least 1 worker thread\n");
        exit(EXIT_FAILURE);
    }

    if(DEBUG) printf("Creating %d threads\n", n);

    /* Spin off worker threads */
    for(int i = 0; i < n - 1; i++){
        pthread_t id;
        pthread_create(&id, NULL, worker, NULL);
    }
    worker(NULL); // Become a worker
}

/* Read instructions from stdin, execute them, and exit on EOF */
void *worker(void *arg)
{
    printf("Hello\n");
    fflush(NULL);
    pthread_exit(NULL);
    return NULL;
}

/* Converts a decimal integer to a binary one in string format */
char *dec2bin(int decimal)
{
	char* ret = malloc(sizeof(char) * 33); // Max length of 32-bit int plus null terminator
	int d = decimal;
	
	for(int i = 128; i >= 1; i = i/2){
		if (d / i){ 
            strcat(ret, "1");
			d -= i;
		} else { 
            strcat(ret, "0");
		}
	}
	
	return ret;
}

/* Converts a CIDR address to a binary prefix in string format */
char *prefix_to_binary(char *prefix)
{
	char *slash_spot = strstr(prefix, "/");

	int prefix_length = atoi(slash_spot + 1);

	char *prefix_portion = malloc(sizeof(char) * 16); // Max length of IPv4 address + null terminator
    slash_spot[0] = '\0';
    strcpy(prefix_portion, prefix);

	char *binary_string = malloc(sizeof(char) * 33);
	
	// A prefix can have a maximum of 4 decimal sets:
	for (int i = 4; i > 0; --i) {
		char *dot_spot = strstr(prefix_portion, ".");
		
		if (dot_spot != NULL) {
			// We have a dot after this decimal portion. We
			// make sure we only consider the decimal portion 
			// before the dot:
			
            strcat(binary_string, dec2bin((int) strtol(prefix_portion, &dot_spot, 10)));

			prefix_portion = dot_spot + 1;
		} else {
			// We have no dot after this decimal portion
			// This means the entire portion is decimal
            strcat(binary_string, dec2bin(atoi(prefix_portion)));
			
			// We're done. Quit looping.
			break;
		}
	}

    free(prefix_portion);
	
	// Alright. We have a complete binary string but we only care about the first prefix_length number of binary digits:
    binary_string[prefix_length] = '\0';
	//cout << "   Binary prefix is : " << retval << endl;
	
	return binary_string;
}

/* Print usage info and exit */
void usage()
{
    printf("Usage: %s [-t scorer_threads] [-o outfile] [-d]... FILE...\n", name);
    exit(EXIT_FAILURE);
}

/* Print given error message and exit */
void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

/*
class ip_trie{
private:
	char *prefix_list;
	Trie<std::pair<int, std::string> > my_trie;
	
public:
	ffsn(char *prefix_list){pre
		this->prefix_list = prefix_list;
		std::cerr << "building trie" << std::endl;
		this->build_prefixes();
		std::cerr << "trie built" << std::endl;
	}
	
	void build_prefixes(){
		std::ifstream data(this->prefix_list.c_str());
		char *prefix;
		int AS_Number;
		
		if (data.fail()) {
			std::cerr << "Unable to open BGP RIB file. Please check that the BGP_RIB_FILE" 
			          << std::endl
			          << "variable is set to the correct location for the file." 
			          << std::endl
			          << "   Current value: " << this->prefix_list
			          << std::endl << std::endl;
			exit(-1); // Exit program.
		}
		
		std::cerr << "BGP Table: Reading BGP RIB..." << std::endl;
		
		while (data >> prefix) {
			char *resultPrefix;
			data >> AS_Number;
			
			// Convert AS prefix to binary string:
			resultPrefix = prefix_to_binary(prefix);
			
			this->my_trie.insert(resultPrefix, std::make_pair(AS_Number, prefix));
		}
		std::cerr << "BGP Table: BGP RIB loaded." << std::endl;
	}
	
	std::pair<int, std::string> get_ASN(char *ip){
		ip += "/32";
		std::pair<int, std::string> ASN = std::make_pair(-1, "");
		this->my_trie.search(prefix_to_binary(ip), ASN);
		return ASN;
	}
};
*/
