struct trienode{
    int populated;
    int ASN;
    struct trienode *zero;
    struct trienode *one;
};

void *worker(void *);
int query(char *);
void entry(char *, int);
char *dec2bin(int);
char *prefix_to_binary(char *);
void usage();
void error(char *);
struct trienode *init_trienode();
void insert(struct trienode *, char *, int);
int search(struct trienode *, char *);
void __recurseInsert(struct trienode *, char *, int);
int __recurseSearch(struct trienode *, char *, int *);
