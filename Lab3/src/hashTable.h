#ifndef _HASH_TABLE
#define _HASH_TABLE

#include "semantic.h"
#include <string.h>
#define HASH_TABLE_SIZE 0x3fff

typedef  struct HashNode_  HashNode;

HashNode* hashTable[HASH_TABLE_SIZE];

HashNode* hashTable_struct[HASH_TABLE_SIZE];

struct HashNode_{
    char *name;
    Type *type;
    HashNode *next;
};

void initHashTable();

void initHashTable_struct();

unsigned int getHashCode(char* name);

Type* getType(char* name);

void addHashNode(Type* type, char* name);

void deleteHashNode(char* name);

int checkDuplicate(char* name);



Type* getType_struct(char* name);

void addHashNode_struct(Type* type, char* name);

void deleteHashNode_struct(char* name);

int checkDuplicate_struct(char* name);

#endif