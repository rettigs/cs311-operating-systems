#define _BSD_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include "uthash.h"

#define MAX_WORD_SIZE 64

struct node{
    char word[MAX_WORD_SIZE];
    int count;
    UT_hash_handle hh;
};

///* Get a pointer to the count of the given word, adding the word to the list first if necessary */
//int *list_get(struct node *base, char *word)
//{
//    for(;;){ // For every node...
//        if(base != NULL){ // If the node exists...
//            if(strcmp(word, base->word) == 0){ // And if it's a match...
//                return &base->count; // Return the count
//            }else if(base->next != NULL){ // Otherwise...
//                base = base->next; // Move to the next node
//            }else{ // If there is no next node, make it!
//                struct node *newnode = (struct node *) malloc(sizeof(struct node));
//                printf("Added new node to end\n");
//                base->next = newnode;
//                newnode->word = (char *) malloc(sizeof(char) * strlen(word));
//                strcpy(newnode->word, word);
//                newnode->count = 0;
//                return &newnode->count;
//            }
//        }else{ // If the node doesn't exist, make it! (used for initializing a blank list)
//            struct node *newnode = (struct node *) malloc(sizeof(struct node));
//            printf("Created base\n");
//            base = newnode;
//            newnode->word = (char *) malloc(sizeof(char) * strlen(word));
//            strcpy(newnode->word, word);
//            newnode->count = 0;
//            return &newnode->count;
//        }
//    }
//}
//
///* Prints the list */
//void list_print(struct node *base)
//{
//    while(base != NULL){ // For every node that exists...
//        printf("%d %s\n", base->count, base->word); // Print it
//        base = base->next; // Move to the next node
//    }
//}

int main(int argc, char *argv[])
{
    struct node *words = NULL;
    struct node *wordentry;

    wordentry = (struct node *) malloc(sizeof(struct node));
    strcpy(wordentry->word, "apples");
    wordentry->count = 1;
    HASH_ADD_STR(words, word, wordentry);

    wordentry = (struct node *) malloc(sizeof(struct node));
    strcpy(wordentry->word, "bananas");
    wordentry->count = 5;
    HASH_ADD_STR(words, word, wordentry);

    wordentry = (struct node *) malloc(sizeof(struct node));
    strcpy(wordentry->word, "peaches");
    wordentry->count = 4;
    HASH_ADD_STR(words, word, wordentry);

    struct node *s = (struct node *) malloc(sizeof(struct node));
    for(s = words; s != NULL; s = s->hh.next){
        printf("Count: %d, Word: %s\n", s->count, s->word);
    }

    exit(EXIT_SUCCESS);
}
