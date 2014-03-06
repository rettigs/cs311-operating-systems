#include <string.h>
#include "trienode.h"

#define TDEBUG 0

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
        if(TDEBUG > 1) printf("Insert: key length is 0, placing ASN %d at root\n", value);
        root->ASN = value;
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

    if(TDEBUG > 1) printf("Recursive insert: started, digit is %c, key is %s\n", digit, key);

    if(strlen(key) <= 0){
        // We have hit the bottom. We should shove our value at this node.
        if(!root->populated){
            root->ASN = value;
            root->populated = 1;
        }else if(TDEBUG) printf("WARNING: Duplicate assignment: node already contains ASN %d; not adding %d\n", root->ASN, value);
        return;
    }

    if(digit == '0'){
        if(root->zero == NULL){
            if(TDEBUG > 1) printf("Recursive insert: adding 'zero' node for ASN %d at %s\n", value, key);
            root->zero = init_trienode();
        }
        __recurseInsert(root->zero, &key[1], value);
    }else{
        if(root->one == NULL){
            if(TDEBUG > 1) printf("Recursive insert: adding 'one' node for ASN %d at %s\n", value, key);
            root->one = init_trienode();
        }
        __recurseInsert(root->one, &key[1], value);
    }
}

int __recurseSearch(struct trienode *root, char *key, int *valuebuf)
{
    char digit = key[0];

    if(TDEBUG > 1) printf("Recursive search: started, digit is %c, key is %s\n", digit, key);

    if(root == NULL){
        if(TDEBUG > 1) printf("Recursive search: root is null at key %s\n", key);
        return *valuebuf;
    }

    if(root->populated){
        if(TDEBUG > 1) printf("Recursive search: root is populated with ASN %d at key %s\n", root->ASN, key);
        valuebuf = &root->ASN;
    }

    if(strlen(key) <= 0){
        // We're done!
        if(TDEBUG > 1) printf("Recursive search: key exhausted, returning ASN %d\n", root->ASN);
        return *valuebuf;
    }

    if(digit == '0'){
        if(root->zero == NULL){
            if(TDEBUG > 1) printf("Recursive search: node at key %s has no 'zero' child, returning ASN %d\n", key, root->ASN);
            return *valuebuf;
        }
        else return __recurseSearch(root->zero, &key[1], valuebuf);
    }else{
        if(root->one == NULL){
            if(TDEBUG > 1) printf("Recursive search: node at key %s has no 'one' child, returning ASN %d\n", key, root->ASN);
            return *valuebuf;
        }
        else return __recurseSearch(root->one, &key[1], valuebuf);
    }
}
