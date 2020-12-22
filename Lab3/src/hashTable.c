#include "hashTable.h"

void initHashTable()
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        hashTable[i] = NULL;
    }
}

unsigned int getHashCode(char *name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~HASH_TABLE_SIZE)
            val = (val ^ (i >> 12)) & HASH_TABLE_SIZE;
    }
    return val;
}

Type *getType(char *name)
{
    int index = getHashCode(name);
    HashNode *hashNode = hashTable[index];
    while (hashNode != NULL)
    {
        if (strcmp(hashNode->name, name) == 0)
        {
            return hashNode->type;
        }
        hashNode = hashNode->next;
    }
    return NULL;
}

void addHashNode(Type *type, char *name)
{
    HashNode *newNode = (HashNode *)malloc(sizeof(HashNode));
    newNode->type = type;
    newNode->name = name;
    newNode->next = NULL;
    int index = getHashCode(name);
    if (hashTable[index] == NULL)
    {
        hashTable[index] = newNode;
        return;
    }
    HashNode *preNode = hashTable[index];
    while (preNode->next != NULL)
    {
        preNode = preNode->next;
    }
    preNode->next = newNode;
}

void deleteHashNode(char *name)
{
    int index = getHashCode(name);
    if (hashTable[index] == NULL)
    {
        return;
    }
    if (strcmp(name, hashTable[index]->name) == 0)
    {
        hashTable[index] = hashTable[index]->next;
        return;
    }
    HashNode *preNode = hashTable[index];
    while (preNode->next != NULL)
    {
        if (strcmp(name, preNode->next->name) == 0)
        {
            preNode->next = preNode->next->next;
            break;
        }
        preNode = preNode->next;
    }
}

int checkDuplicate(char *name)
{
    int index = getHashCode(name);
    HashNode *node = hashTable[index];
    while (node != NULL)
    {
        if (strcmp(node->name, name) == 0)
        {
            return 1;
        }
        node = node->next;
    }
    return 0;
}

