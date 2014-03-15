struct trienode{
    int populated;
    int ASN;
    struct trienode *zero;
    struct trienode *one;
};

struct workerarg{
    int id;
    int fd;
};

void *worker(void *);
void XMLquery(int, FILE *, char *);
void XMLentry(int, char *, int);
void XMLstats(int, FILE *);
int query(int wid, char *);
void entry(int wid, char *, int);
char *dec2bin(int);
char *prefix_to_binary(char *);
int bin2dec(char *);
char *binary_to_prefix(char *);
void handler(int);
void usage();
void error(char *);
struct trienode *init_trienode();
void insert(int wid, struct trienode *, char *, int);
int search(int wid, struct trienode *, char *);
void __recurseInsert(int wid, struct trienode *, char *, int);
int __recurseSearch(int wid, struct trienode *, char *, int *);
void print_trie(FILE *, struct trienode *);
void __recursePrint_trie(FILE *, struct trienode *, char *);
