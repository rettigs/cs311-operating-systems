#define _POSIX_SOURCE || _POSIX_C_SOURCE >= 1
#define _XOPEN_SOURCE
#define _BSD_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include "ip2asn.h"
#include "uthash.h"

#define MAX_IP_LEN 32
#define MAX_PATH_LEN 256
#define MAX_WORKERS 2
#define BACKLOG 1024
#define DEFAULT_IP INADDR_ANY
#define DEFAULT_PORT 54321

struct hostnode{
    char ip[MAX_IP_LEN];
    UT_hash_handle hh;
};

char *name; // Name of program
int DEBUG = 0; // Whether we are in debug mode
int curw = 0; // Number of workers currently running
FILE *ins = NULL; // Stream to write trie to at end
FILE *outs = NULL; // Stream to read trie from at start
int done = 0; // Whether we are done reading from stdin (have we reached EOF?)
struct trienode *trie; // Our trie for storing ASNs
struct sockaddr_in address; // Our network address
char ipstring[MAX_IP_LEN]; // Our IP in string format
struct hostnode *hosthash = NULL; // Hashmap to store unique client hosts
int queries = 0; // Number of queries answered
int prefixes = 0; // Number of prefixes stored

/* Start an IP to ASN translation service server */
int main(int argc, char *argv[])
{
    name = argv[0];

    /* Register signal handler so we terminate cleanly */
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if(sigaction(SIGINT, &sa, NULL) == -1) error("Could not register signal handler");
    if(sigaction(SIGQUIT, &sa, NULL) == -1) error("Could not register signal handler");
    if(sigaction(SIGHUP, &sa, NULL) == -1) error("Could not register signal handler");

    /* Initialize network address */
    memset(&address, 0, sizeof(struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(DEFAULT_IP);
    address.sin_port = htons(DEFAULT_PORT);

    /* Parse flags */
    int opt;
    int port = -1;
    char infile[MAX_PATH_LEN];
    char outfile[MAX_PATH_LEN];
    while((opt = getopt(argc, argv, "da:i:o:h")) != -1){
        switch(opt){
            case 'd':
                DEBUG++;
                break;
            case 'a':
                if(sscanf(optarg, "%s:%d", (char *) &ipstring, &port) < 1) usage();
                inet_pton(AF_INET, ipstring, &address.sin_addr.s_addr);
                if(port > 0) address.sin_port = htons(port);
                break;
            case 'i':
                if(sscanf(optarg, "%s", infile) != 1) usage();
                if((ins = fopen(infile, "r")) == NULL) error("Could not open input file");
                break;
            case 'o':
                if(sscanf(optarg, "%s", outfile) != 1) usage();
                if((outs = fopen(outfile, "w")) == NULL) error("Could not open output file");
                break;
            case 'h':
            default: // '?'
                usage();
        }
    }

    /* Initialize trie */
    trie = init_trienode();

    /* Populate trie from database, if one is given */
    if(ins != NULL){
        if(DEBUG) printf("[Main] Importing database\n");
        int ASN;
        char prefix[32+1];
        while(fscanf(ins, "%d %s\n", &ASN, prefix) > 0){ // Read each line
            if(DEBUG > 1) printf("[Main] Importing entry with ASN: %d,\tprefix: %s\n", ASN, prefix);
            insert(-1, trie, prefix, ASN);
        }
    }

    /* Bind to address and start listening for connections */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(DEBUG) printf("[Main] Binding to address %s:%d\n", ipstring, port);
    if(bind(listenfd, (struct sockaddr *) &address, sizeof(address)) != 0) error("Could not bind to address");
    listen(listenfd, BACKLOG);

    /* Create array to store each worker's id and client socket fd */
    struct workerarg workers[MAX_WORKERS];

    /* Keep accepting connections forever, spinning off new worker threads to handle each one */
    for(;;){

        /* Accept the connection and get ready to spin off a worker thread */
        if(DEBUG) printf("[Main] Spinning off worker %d\n", curw);
        struct sockaddr_in clientaddress;
        socklen_t addresslen = sizeof(struct sockaddr_in);
        workers[curw].id = curw;
        workers[curw].fd = accept(listenfd, (struct sockaddr *) &clientaddress, &addresslen);
        if(workers[curw].fd == -1) error("Could not accept connection");

        /* Add the client's IP to the list of unique hosts if it's not already in it */
        char clientip[MAX_IP_LEN];
        inet_ntop(AF_INET, &clientaddress.sin_addr.s_addr, clientip, sizeof(*clientip));
        struct hostnode *hostentry;
        HASH_FIND_STR(hosthash, clientip, hostentry);
        if(hostentry == NULL){ // If it's not already in the hashmap, then add it
            hostentry = malloc(sizeof(*hostentry));
            strcpy(hostentry->ip, clientip);
            HASH_ADD_STR(hosthash, ip, hostentry);
        }

        /* Print debug information about the connection */
        if(DEBUG){
            struct sockaddr_in acceptaddress;
            getsockname(workers[curw].fd, (struct sockaddr *) &acceptaddress, &addresslen);
            printf("[Main] Worker %d on port %d will be servicing client at %s\n", curw, acceptaddress.sin_port, clientip);
        }
        
        /* Spin off the worker thread */
        pthread_t id;
        pthread_create(&id, NULL, worker, &workers[curw]);
        curw++;
    }

    exit(EXIT_SUCCESS);
}

/* Perform commands given by the client */
void *worker(void *workers)
{
    int wid = ((struct workerarg *) workers)->id;

    if(DEBUG) printf("[Worker %d] Started\n", wid);

    /* Open stream for socket */
    FILE *stream = fdopen(((struct workerarg *) workers)->fd, "r+");

    /* Initialize some variables to put scanned values into */
    char ip[MAX_IP_LEN];
    int asn;

    /* Keep reading XML forever */
    for(;;){
        /* Read an XML command and perform the requested operation */
        if      (fscanf(stream, "<query><ip>%s</ip></query>", ip) == 1) XMLquery(wid, stream, ip);
        else if (fscanf(stream, "<entry><cidr>%s</cidr><asn>%d</asn></entry>", ip, &asn) == 2) XMLentry(wid, ip, asn);
        else if (fscanf(stream, "<stats />") == 0) XMLstats(wid, stream);
        else if (fscanf(stream, "<terminate />") == 0){
            printf("[Worker %d] Got termination notice; sending termination signal\n", wid);
            kill(0, SIGINT);
            pthread_exit(NULL);
        }
    }
}

/* Handle an XML query */
void XMLquery(int wid, FILE *stream, char *ip)
{
    fprintf(stream, "<answer><asn>%d</asn></answer>\n", query(wid, ip));
}

/* Handle an XML entry */
void XMLentry(int wid, char *cidr, int asn)
{
    entry(wid, cidr, asn);
}

/* Handle an XML stats request */
void XMLstats(int wid, FILE *stream)
{
    if(DEBUG) printf("[Worker %d] Doing a stat lookup\n", wid);
    fprintf(stream, "<stats><hosts>%d</hosts><queries>%d</queries><prefixes>%d</prefixes></stats>\n", HASH_COUNT(hosthash), queries, prefixes);
}

/* Looks up an IP address in the trie and returns the corresponding ASN */
int query(int wid, char *ip)
{
    /* Convert ip from dotted decimal format to binary format */
    char cidrip[strlen(ip)+3];
    strcpy(cidrip, ip);
    strcat(cidrip, "/32"); // We need to add a /32 at the end of the ip for the converter to work

    char *binip = prefix_to_binary(cidrip);

    if(DEBUG) printf("[Worker %d] Query: IP is %s, CIDR is %s, binary is %s\n", wid, ip, cidrip, binip);

    return search(wid, trie, binip);
}

/* Adds the given ASN to the trie for the given prefix */
void entry(int wid, char *prefix, int ASN)
{
    char *binprefix = prefix_to_binary(prefix);

    if(DEBUG) printf("[Worker %d] Entry: CIDR is %s, binary is %s\n", wid, prefix, binprefix);

    insert(wid, trie, binprefix, ASN);
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

/* Cleanly handle termination signals */
void handler(int sig)
{
    printf("[Handler] Saving database\n");
    if(outs != NULL) print_trie(trie); // Save the database before terminating
    printf("[Handler] Terminating cleanly\n");
    exit(EXIT_SUCCESS);
}

/* Print usage info and exit */
void usage()
{
    printf("Usage: %s [-h] [-a ip[:port]] [-i infile] [-o outfile] [-d]...\n", name);
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
void insert(int wid, struct trienode *root, char *key, int value)
{
    if(strlen(key) > 0) __recurseInsert(wid, root, key, value);
    else{
        if(DEBUG > 1){
            if(wid == -1) printf("[Main] Insert: key length is 0, placing ASN %d at root\n", value);
            else printf("[Worker %d] Insert: key length is 0, placing ASN %d at root\n", wid, value);
        }
        root->ASN = value;
        root->populated = 1;
        prefixes++;
    }
}

/* Returns the ASN at the given location in the trie.
   Key is binary integer of no more than 32 bits, in string format */
int search(int wid, struct trienode *root, char *key)
{
    int valuebuf = root->ASN;
    queries++;
    return __recurseSearch(wid, root, key, &valuebuf);
}

void __recurseInsert(int wid, struct trienode *root, char *key, int value)
{
    // Root cannot be null
    char digit = key[0];

    if(DEBUG > 1) printf("[Worker %d] Recursive insert: started, digit is %c, key is %s\n", wid, digit, key);

    if(strlen(key) <= 0){
        // We have hit the bottom. We should shove our value at this node.
        if(!root->populated){
            root->ASN = value;
            root->populated = 1;
        }else if(DEBUG) printf("[Worker %d] Recursive insert: duplicate assignment; node already contains ASN %d; not adding %d\n", wid, root->ASN, value);
        return;
    }

    if(digit == '0'){
        if(root->zero == NULL){
            if(DEBUG > 1) printf("[Worker %d] Recursive insert: adding 'zero' node for ASN %d at %s\n", wid, value, key);
            root->zero = init_trienode();
        }
        __recurseInsert(wid, root->zero, &key[1], value);
    }else{
        if(root->one == NULL){
            if(DEBUG > 1) printf("[Worker %d] Recursive insert: adding 'one' node for ASN %d at %s\n", wid, value, key);
            root->one = init_trienode();
        }
        __recurseInsert(wid, root->one, &key[1], value);
    }
}

int __recurseSearch(int wid, struct trienode *root, char *key, int *valuebuf)
{
    char digit = key[0];

    if(DEBUG > 1) printf("[Worker %d] Recursive search: started, digit is %c, key is %s\n", wid, digit, key);

    if(root == NULL){
        if(DEBUG > 1) printf("[Worker %d] Recursive search: root is null at key %s\n", wid, key);
        return *valuebuf;
    }

    if(root->populated){
        if(DEBUG > 1) printf("[Worker %d] Recursive search: root is populated with ASN %d at key %s\n", wid, root->ASN, key);
        valuebuf = &root->ASN;
    }

    if(strlen(key) <= 0){
        // We're done!
        if(DEBUG > 1) printf("[Worker %d] Recursive search: key exhausted, returning ASN %d\n", wid, root->ASN);
        return *valuebuf;
    }

    if(digit == '0'){
        if(root->zero == NULL){
            if(DEBUG > 1) printf("[Worker %d] Recursive search: node at key %s has no 'zero' child, returning ASN %d\n", wid, key, root->ASN);
            return *valuebuf;
        }
        else return __recurseSearch(wid, root->zero, &key[1], valuebuf);
    }else{
        if(root->one == NULL){
            if(DEBUG > 1) printf("[Worker %d] Recursive search: node at key %s has no 'one' child, returning ASN %d\n", wid, key, root->ASN);
            return *valuebuf;
        }
        else return __recurseSearch(wid, root->one, &key[1], valuebuf);
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
    if(DEBUG > 1) printf("[Handler] Recursive print: checking node at prefix %s\n", prefix);

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
