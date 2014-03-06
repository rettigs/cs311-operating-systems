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
#include "trienode.c"

char *name; // Name of program
int DEBUG = 0; // Whether we are in debug mode
int w = 1; // Number of workers to use
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; // Mutex for stdin
int done = 0; // Whether we are done reading from stdin (have we reached EOF?)
struct trienode *trie; // Our trie for storing ASNs

/* Do BGP stuff with a trie using multiple threads */
int main(int argc, char *argv[])
{
    name = argv[0];

    /* Parse flags */
    int opt;
    while((opt = getopt(argc, argv, "w:d")) != -1){
        switch(opt){
            case 'w':
                if(sscanf(optarg, "%d", &w) != 1) usage();
                break;
            case 'd':
                DEBUG++;
                break;
            default: // '?'
                usage();
        }
    }
    if(w < 1){
        printf("Must specify at least 1 worker\n");
        exit(EXIT_FAILURE);
    }

    /* Initialize trie */
    trie = init_trienode();

    /* Spin off w - 1 workers */
    if(DEBUG) printf("[Main] Creating %d workers\n", w);
    for(int i = 0; i < w - 1; i++){
        pthread_t id;
        pthread_create(&id, NULL, worker, NULL);
    }

    /* Become a worker */
    worker(NULL);
}

/* Read instructions from stdin, execute them, and exit on EOF */
void *worker(void *arg)
{
    if(DEBUG) printf("[Worker %d] Started\n", pthread_self());

    char line[1+3+1+3+1+3+1+3+1+2+1+10]; // Max length of command + CIDR address + space + 32-bit decimal int
    
    for(;;){
        /* Read a line from stdin, terminating if nothing is left */
        pthread_mutex_lock(&mtx);
        if(done){ // If a previous worker got EOF, there's nothing left for us; terminate
            if(DEBUG) printf("[Worker %d] Previous worker got EOF; terminating\n", pthread_self());
            pthread_mutex_unlock(&mtx);
            pthread_exit(NULL);
        }

        if(gets(line) == NULL){ // If we got EOF, notify other workers, then terminate
            done = 1;
            if(DEBUG) printf("[Worker %d] Got EOF, terminating\n", pthread_self());
            pthread_mutex_unlock(&mtx);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&mtx);

        /* Process that line */
        if(line[0] == '?'){ // Query
            printf("The ASN for %s is: %d\n", &line[1], query(&line[1]));
        }else if(line[0] == '!'){ // Entry
            char prefix[3+1+3+1+3+1+3+1+2]; // Max length of a CIDR address
            int ASN;
            sscanf(&line[1], "%s %d", (char *) &prefix, &ASN);
            entry((char *)&prefix, ASN);
        }
    }
}

/* Looks up an IP address in the trie and returns the corresponding ASN */
int query(char *ip)
{
    /* Convert ip from dotted decimal format to binary format */
    char cidrip[strlen(ip)+3];
    strcpy(cidrip, ip);
    strcat(cidrip, "/32"); // We need to add a /32 at the end of the ip for the converter to work

    char *binip = prefix_to_binary(cidrip);

    if(DEBUG) printf("[Worker %d] Query: IP is %s, CIDR is %s, binary is %s\n", pthread_self(), ip, cidrip, binip);

    return search(trie, binip);
}

/* Adds the given ASN to the trie for the given prefix */
void entry(char *prefix, int ASN)
{
    char *binprefix = prefix_to_binary(prefix);

    if(DEBUG) printf("[Worker %d] Entry: CIDR is %s, binary is %s\n", pthread_self(), prefix, binprefix);

    insert(trie, binprefix, ASN);
}

/* Converts an 8-bit integer to binary in string format */
char *dec2bin(int decimal)
{
	char* ret = malloc(sizeof(char) * 9); // Max length of 8-bit int plus null terminator
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

	char *prefix_portion_s = malloc(sizeof(char) * 16); // Max length of IPv4 address + null terminator
    char *prefix_portion = prefix_portion_s;
    strncpy(prefix_portion, prefix, slash_spot - prefix);

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

    free(prefix_portion_s);
	
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
