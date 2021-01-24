#ifndef _SEMANTIC
#define _SEMANTIC

#include "syntaxTree.h"
#include "enum.h"

int semanticErrorFlag;

typedef struct Type_ Type;
typedef struct Array_ Array;
typedef struct Structure_ Structure;
typedef struct Function_ Function;
typedef struct FieldList_ FieldList;


struct Type_
{
    Kind kind;
    union
    {
        BasicType basic;
        Array *array;
        Structure *structure;
        Function *function;
    } u;
};

struct Array_
{
    Type *elem;
    int size;
};

struct Structure_
{
    char *name;
    FieldList *domain;
};

struct Function_
{
    char *name;
    Type *returnType;
    FieldList *params;
};

struct FieldList_
{
    char *name;
    Type *type;
    FieldList *next;
};

// 创建新的符号，以便加入符号表
Type* newType(Kind kind,int num, ...);

// 检查类型函数
// *cmp()，在相同时返回0
// check*()，在相同时返回1
int typecmp(Type* first, Type* second);
int strtcmp(Structure *first, Structure *second);
int checkLeftExp(Node *leftNode);
int checkParams(FieldList* first, FieldList* second);

// 与TreeNode有关的操作
void analyze_tree();
void Program(Node *root);
void ExtDefList(Node *n);
void ExtDef(Node *n);
void ExtDecList(Node *n, Type* type);
Type *Specifier(Node *n);
Type *StructSpecifier(Node *n);
FieldList *VarDec(Node *n, Type *type);
Type *FunDec(Node *n, Type *returnType);
FieldList *VarList(Node *n);
FieldList *ParamDec(Node *n);
void CompSt(Node *n, Type *returnType);
void StmtList(Node *n, Type *returnType);
void Stmt(Node *n, Type *returnType);
FieldList *DefList(Node *n, int inStruct);
FieldList *Def(Node *n, int inStruct);
FieldList *DecList(Node *n, Type *specifier, int inStruct);
FieldList *Dec(Node *n, Type *specifier, int inStruct);
Type *Exp(Node *n);
FieldList *Args(Node *n);

// 报错
void print_error(ErrorType error, int lineno);


#endif