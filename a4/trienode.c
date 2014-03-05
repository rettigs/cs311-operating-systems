#include <string.h>
#include "trienode.h"

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
}

/* Returns the ASN at the given location in the trie.
   Key is binary integer of no more than 32 bits, in string format */
int search(struct trienode *root, char *key)
{
    int valuebuf = -1;
    return __recurseSearch(root, key, &valuebuf);
}

void __recurseInsert(struct trienode *root, char *key, int value)
{
    // Root cannot be null
    char digit = key[0];

    //cout << "Key is: " << key << " and digit is: " << digit << endl;

    if(strlen(key) <= 0){
        // We have hit the bottom. We should shove our 
        // value at this node.
        if(!root->populated){
            root->ASN = value;
            root->populated = 1;
        }else printf("WARNING: Duplicate assignment: node already contains ASN %d; not adding %d\n", root->ASN, value);
        return;
    }

    if(digit == '0'){
        if(root->zero == NULL) root->zero = init_trienode();
        __recurseInsert(root->zero, &key[1], value);
    }else{
        if(root->one == NULL) root->one = init_trienode();
        __recurseInsert(root->one, &key[1], value);
    }
}

int __recurseSearch(struct trienode *root, char *key, int *valuebuf)
{
    if(root == NULL){
        return *valuebuf;
    }

    if(root->populated){
        valuebuf = &root->ASN;
    }

    if(strlen(key) <= 0){
        // We're done!
        return *valuebuf;
    }

    char digit = key[0];

    if(digit == '0'){
        if(root->zero == NULL) return *valuebuf;
        else return __recurseSearch(root->zero, &key[1], valuebuf);
    }else{
        if(root->one == NULL) return *valuebuf;
        else return __recurseSearch(root->one, &key[1], valuebuf);
    }
}
