#ifndef _HASH_TABLE
#define _HASH_TABLE

#include "semantic.h"
#include <string.h>
#define HASH_TABLE_SIZE 0x3fff

typedef struct HashNode_ HashNode;

HashNode *hashTable[HASH_TABLE_SIZE];

struct HashNode_
{
    char *name;
    Type *type;
    HashNode *next;
};

void initHashTable();

unsigned int getHashCode(char *name);

Type *getType(char *name);

void addHashNode(Type *type, char *name);

void deleteHashNode(char *name);

int checkDuplicate(char *name);

#endif