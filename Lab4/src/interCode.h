#ifndef _INTERMEDIATE_CODE
#define _INTERMEDIATE_CODE
#include "hashTable.h"
#include "syntaxTree.h"
#include <stdio.h>
#include <stdlib.h>

FILE *outputFile;
int tempNum;
int labelNum;

// 先集中声明
typedef struct Operand_ Operand;
typedef struct InterCode_ InterCode;
typedef struct InterCodeList_ InterCodeList;
typedef struct Arg_ Arg;
typedef struct ArgList_ ArgList;
typedef void (*PrintCodeFunc)(InterCode *code);

struct Operand_
{
    OperandKind kind;
    union
    {
        int value;
        char *name;
    } u;
};

struct InterCode_
{
    CodeKind kind;
    union
    {
        struct
        {
            Operand *op; // label funciton goto return arg param read write
        } monoOp;
        struct
        {
            Operand *left, *right;
        } assign;
        struct
        {
            Operand *result,*op1, *op2;
        } binOp;
        struct
        {
            Operand *x, *y, *z;
            char* relop;
        } if_goto;
        struct
        {
            Operand *op;
            int size;
        } dec;
    } u;
    InterCode *next;
};

struct InterCodeList_
{
    InterCode *head;
    InterCode *tail;
};

struct Arg_
{
    Operand *operand;
    Arg *next;
};

struct ArgList_
{
    Arg *head;
    // 记录当前链表指针的位置
    Arg *tail;
};

void init();
void generateCode(char *fileName);
void printCode(InterCodeList *);
void printOperand(Operand *op);
Operand* newOperand(OperandKind kind);
Operand* newTemp();
Operand* newLabel();
InterCodeList* newInterCodeList();
InterCode* newInterCode(CodeKind kind);
ArgList* newArgList();
Arg* newArg(Operand* op);
void addInterCode(InterCodeList* list, InterCode* code);
int getSize(Type* type);

InterCodeList *join(InterCodeList *first, InterCodeList *second);
InterCodeList *translateExtDefList(Node *node);
InterCodeList *translateExtDef(Node *node);
InterCodeList *translateVarDec(Node *node, Operand *place);
InterCodeList *translateFunDec(Node *node);
InterCodeList *translateStmtList(Node *node);
InterCodeList *translateCompSt(Node *node);
InterCodeList *translateStmt(Node *node);
InterCodeList *translateDefList(Node *node);
InterCodeList *translateDef(Node *node);
InterCodeList *translateDecList(Node *node);
InterCodeList *translateDec(Node *node);
InterCodeList *translateExp(Node *node, Operand *place);
InterCodeList *translateArgs(Node *node, ArgList *argList);
InterCodeList *translateCond(Node *node, Operand *labelTrue, Operand *labelFalse);

// 表驱动+函数指针
void labelPrint(InterCode *code);
void functionPrint(InterCode *code);
void assignPrint(InterCode *code);
void addPrint(InterCode *code);
void subPrint(InterCode *code);
void mulPrint(InterCode *code);
void divPrint(InterCode *code);
void get_addrPrint(InterCode *code);
void read_addrPrint(InterCode *code);
void write_addrPrint(InterCode *code);
void gotoPrint(InterCode *code);
void if_gotoPrint(InterCode *code);
void returnPrint(InterCode *code);
void decPrint(InterCode *code);
void argPrint(InterCode *code);
void callPrint(InterCode *code);
void paramPrint(InterCode *code);
void readPrint(InterCode *code);
void writePrint(InterCode *code);

#endif