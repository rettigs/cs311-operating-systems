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

#define MAX_PATH_LEN 256

char *name; // Name of program
int DEBUG = 0; // Whether we are in debug mode
int w = 1; // Number of workers to use
int curw = 1; // Number of workers currently running
FILE *ins = NULL; // Stream to write trie to at end
FILE *outs = NULL; // Stream to read trie from at start
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; // Mutex for stdin
int done = 0; // Whether we are done reading from stdin (have we reached EOF?)
struct trienode *trie; // Our trie for storing ASNs

/* Do BGP stuff with a trie using multiple threads */
int main(int argc, char *argv[])
{
    name = argv[0];

    /* Parse flags */
    int opt;
    char infile[MAX_PATH_LEN];
    char outfile[MAX_PATH_LEN];
    while((opt = getopt(argc, argv, "w:di:o:")) != -1){
        switch(opt){
            case 'w':
                if(sscanf(optarg, "%d", &w) != 1) usage();
                break;
            case 'd':
                DEBUG++;
                break;
            case 'i':
                if(sscanf(optarg, "%s", infile) != 1) usage();
                if((ins = fopen(infile, "r")) == NULL) error("Could not open input file");
                break;
            case 'o':
                if(sscanf(optarg, "%s", outfile) != 1) usage();
                if((outs = fopen(outfile, "w")) == NULL) error("Could not open output file");
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

    /* Populate tree from database, if one is given */
    if(ins != NULL){
        int ASN;
        char prefix[32+1];
        while(fscanf(ins, "%d %s\n", &ASN, prefix) > 0){ // Read each line
            if(DEBUG) printf("[Main] Adding entry to trie with ASN %d and prefix %s\n", ASN, prefix);
            insert(trie, prefix, ASN);
        }
    }

    /* Spin off w - 1 workers */
    if(DEBUG) printf("[Main] Creating %d workers\n", w);
    for(int i = 0; i < w - 1; i++){
        curw++;
        pthread_t id;
        pthread_create(&id, NULL, worker, NULL);
    }

    /* Become a worker */
    worker(NULL);
}

/* Read instructions from stdin, execute them, and exit on EOF */
void *worker(void *arg)
{
    if(DEBUG) printf("[Worker %d] Started\n", (int) pthread_self());

    char line[1+3+1+3+1+3+1+3+1+2+1+10]; // Max length of command + CIDR address + space + 32-bit decimal int
    
    for(;;){
        /* Read a line from stdin, terminating if nothing is left */
        pthread_mutex_lock(&mtx);
        if(done){ // If a previous worker got EOF, there's nothing left for us; terminate
            pthread_mutex_unlock(&mtx);
            if(curw == 1 && outs != NULL) print_trie(trie); // Print the trie if we are the last worker
            curw--;
            if(DEBUG) printf("[Worker %d] Previous worker got EOF; terminating\n", (int) pthread_self());
            pthread_exit(NULL);
        }

        if(gets(line) == NULL){ // If we got EOF, notify other workers, then terminate
            pthread_mutex_unlock(&mtx);
            done = 1;
            if(curw == 1 && outs != NULL) print_trie(trie); // Print the trie if we are the last worker
            curw--;
            if(DEBUG) printf("[Worker %d] Got EOF, terminating\n", (int) pthread_self());
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

    if(DEBUG) printf("[Worker %d] Query: IP is %s, CIDR is %s, binary is %s\n", (int) pthread_self(), ip, cidrip, binip);

    return search(trie, binip);
}

/* Adds the given ASN to the trie for the given prefix */
void entry(char *prefix, int ASN)
{
    char *binprefix = prefix_to_binary(prefix);

    if(DEBUG) printf("[Worker %d] Entry: CIDR is %s, binary is %s\n", (int) pthread_self(), prefix, binprefix);

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
    printf("Usage: %s [-w workers] [-i infile] [-o outfile] [-d]...\n", name);
    exit(EXIT_FAILURE);
}

/* Print given error message and exit */
void error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

/* Initializes a new trie node */
struct trienode *init_trienode()
{
    struct trienode *trienode = malloc(sizeof(struct trienode));

    trienode->populated = 0;
    trienode->ASN = -1;
    trienode->zero = NULL;
    trienode->one = NULL;

    return trienode;
}

/* Inserts the given value into the trie at the specified location.
   Key is binary integer of no more than 32 bits, in string format */
void insert(struct trienode *root, char *key, int value)
{
    if(strlen(key) > 0) __recurseInsert(root, key, value);
    else{
        if(DEBUG > 1) printf("[Worker %d] Insert: key length is 0, placing ASN %d at root\n", (int) pthread_self(), value);
        root->ASN = value;
        root->populated = 1;
    }
}

/* Returns the ASN at the given location in the trie.
   Key is binary integer of no more than 32 bits, in string format */
int search(struct trienode *root, char *key)
{
    int valuebuf = root->ASN;
    return __recurseSearch(root, key, &valuebuf);
}

void __recurseInsert(struct trienode *root, char *key, int value)
{
    // Root cannot be null
    char digit = key[0];

    if(DEBUG > 1) printf("[Worker %d] Recursive insert: started, digit is %c, key is %s\n", (int) pthread_self(), digit, key);

    if(strlen(key) <= 0){
        // We have hit the bottom. We should shove our value at this node.
        if(!root->populated){
            root->ASN = value;
            root->populated = 1;
        }else if(DEBUG) printf("[Worker %d] Recursive insert: duplicate assignment; node already contains ASN %d; not adding %d\n", (int) pthread_self(), root->ASN, value);
        return;
    }

    if(digit == '0'){
        if(root->zero == NULL){
            if(DEBUG > 1) printf("[Worker %d] Recursive insert: adding 'zero' node for ASN %d at %s\n", (int) pthread_self(), value, key);
            root->zero = init_trienode();
        }
        __recurseInsert(root->zero, &key[1], value);
    }else{
        if(root->one == NULL){
            if(DEBUG > 1) printf("[Worker %d] Recursive insert: adding 'one' node for ASN %d at %s\n", (int) pthread_self(), value, key);
            root->one = init_trienode();
        }
        __recurseInsert(root->one, &key[1], value);
    }
}

int __recurseSearch(struct trienode *root, char *key, int *valuebuf)
{
    char digit = key[0];

    if(DEBUG > 1) printf("[Worker %d] Recursive search: started, digit is %c, key is %s\n", (int) pthread_self(), digit, key);

    if(root == NULL){
        if(DEBUG > 1) printf("[Worker %d] Recursive search: root is null at key %s\n", (int) pthread_self(), key);
        return *valuebuf;
    }

    if(root->populated){
        if(DEBUG > 1) printf("[Worker %d] Recursive search: root is populated with ASN %d at key %s\n", (int) pthread_self(), root->ASN, key);
        valuebuf = &root->ASN;
    }

    if(strlen(key) <= 0){
        // We're done!
        if(DEBUG > 1) printf("[Worker %d] Recursive search: key exhausted, returning ASN %d\n", (int) pthread_self(), root->ASN);
        return *valuebuf;
    }

    if(digit == '0'){
        if(root->zero == NULL){
            if(DEBUG > 1) printf("[Worker %d] Recursive search: node at key %s has no 'zero' child, returning ASN %d\n", (int) pthread_self(), key, root->ASN);
            return *valuebuf;
        }
        else return __recurseSearch(root->zero, &key[1], valuebuf);
    }else{
        if(root->one == NULL){
            if(DEBUG > 1) printf("[Worker %d] Recursive search: node at key %s has no 'one' child, returning ASN %d\n", (int) pthread_self(), key, root->ASN);
            return *valuebuf;
        }
        else return __recurseSearch(root->one, &key[1], valuebuf);
    }
}

/* Prints the trie */
void print_trie(struct trienode *root)
{
    char *prefix = ""; //Max length of 32-bit int in binary plus null terminator
    __recursePrint_trie(root, prefix);
}

void __recursePrint_trie(struct trienode *root, char *prefix)
{
    if(DEBUG > 1) printf("[Worker %d] Recursive print: checking node at prefix %s\n", (int) pthread_self(), prefix);

    /* If we have an ASN, print it with our path so far (the prefix) */
    if(root->populated){
        char *printprefix = prefix;
        if(strcmp(prefix, "") == 0) printprefix = "-";
        fprintf(outs, "%d\t%s\n", root->ASN, printprefix); // It's backwards because it prints prettier
    }

    if(root->zero != NULL){
        char prefix0[33];
        strcpy(prefix0, prefix);
        strcat(prefix0, "0");
        __recursePrint_trie(root->zero, prefix0);
    }

    if(root->one != NULL){
        char prefix1[33];
        strcpy(prefix1, prefix);
        strcat(prefix1, "1");
        __recursePrint_trie(root->one, prefix1);
    }
}
