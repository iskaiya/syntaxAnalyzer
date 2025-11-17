#ifndef WORDHASH_H
#define WORDHASH_H

#include <stdbool.h>
#include "tokens.h"

//HASH TABLE STUFF
#define TABLE_SIZE 100
#define MAX 100

typedef struct {
    char key[MAX];
    TokenCategory category;
    int tokenValue;
} HashEntry;

extern HashEntry hash_table[TABLE_SIZE]; //declare hash table

//function prototypes
unsigned int hash(const char *key);
void hashhashInsert(const char *key, TokenCategory category, int token_value);
HashEntry* hashLookUp(const char *key);
void initialize_table(void);


// new function for parser to use
int hashLookup(const char *lexeme, int *category, int *value);

#endif