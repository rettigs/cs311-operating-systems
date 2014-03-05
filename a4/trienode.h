struct trienode{
    int populated;
    int ASN;
    struct trienode *zero;
    struct trienode *one;
};

struct trienode *init_trienode();
void insert(struct trienode *, char *, int);
int search(struct trienode *, char *);
void __recurseInsert(struct trienode *, char *, int);
int __recurseSearch(struct trienode *, char *, int *);
