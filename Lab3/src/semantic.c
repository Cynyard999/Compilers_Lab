#include "semantic.h"
#include "hashTable.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
Type *newType(Kind kind, int num, ...)
{
    Type *newType = (Type *)malloc(sizeof(Type));
    newType->kind = kind;
    va_list valist;
    switch (kind)
    {
    case BASIC:
        va_start(valist, num);
        newType->u.basic = va_arg(valist, BasicType);
        break;
    case ARRAY:
        va_start(valist, num);
        newType->u.array = va_arg(valist, Array *);
        break;
    case STRUCTURE:
        va_start(valist, num);
        newType->u.structure = va_arg(valist, Structure *);
        break;
    case FUNCTION:
        va_start(valist, num);
        newType->u.function = va_arg(valist, Function *);
        break;
    }
    va_end(valist);
    return newType;
}

// 类型比较，相同返回0 不同返回1,
int typecmp(Type *first, Type *second)
{
    if (first == NULL && second == NULL)
    {
        return 0;
    }
    if (first == NULL || second == NULL)
    {
        return 1;
    }
    if (first->kind != second->kind)
    {
        return 1;
    }
    switch (first->kind)
    {
    case BASIC:
        return first->u.basic != second->u.basic;
    case ARRAY:
        return typecmp(first->u.array->elem, second->u.array->elem);
    case FUNCTION:
        return 1;
    case STRUCTURE:
        return strtcmp(first->u.structure, second->u.structure);
    }
}

// 结构比较，相同返回0 不同返回1,
int strtcmp(Structure *first, Structure *second)
{
    // 个数相同，并且类型的顺序也需要相同
    FieldList *firstfield = first->domain;
    FieldList *secondfield = second->domain;
    while (firstfield != NULL && secondfield != NULL)
    {
        if (typecmp(firstfield->type, secondfield->type) != 0)
        {
            return 1;
        }
        else
        {
            firstfield = firstfield->next;
            secondfield = secondfield->next;
        }
    }
    // 如果while之后两个都是NULL
    if (firstfield == NULL && secondfield == NULL)
    {
        return 0;
    }
    // while之后只有一个NULL
    else
    {
        return 1;
    }
}

// leftNode就代表一个Exp节点
int checkLeftExp(Node *leftNode)
{
    // 只有这三类才能作为左值被赋值
    if (
        // ID
        ((strcmp(leftNode->firstChild->name, "ID") == 0) && (leftNode->firstChild->nextSibling == NULL)) ||
        // EXP DOT ID
        ((strcmp(leftNode->firstChild->name, "Exp") == 0) && (strcmp(leftNode->firstChild->nextSibling->name, "DOT") == 0)) ||
        // EXP LB
        ((strcmp(leftNode->firstChild->name, "Exp") == 0) && (strcmp(leftNode->firstChild->nextSibling->name, "LB") == 0)))
    {
        return 1;
    }
    else if (strcmp(leftNode->firstChild->name, "LP") == 0)
    {
        return checkLeftExp(leftNode->firstChild->nextSibling);
    }
    else
    {
        return 0;
    }
}

int checkParams(FieldList *first, FieldList *second)
{
    FieldList *left = first;
    FieldList *right = second;
    while ((left != NULL && right != NULL) && typecmp(left->type, right->type) == 0)
    {
        left = left->next;
        right = right->next;
    }
    // 如果跳出后是两个都为NULL
    if (left == NULL && right == NULL)
    {
        return 1;
    }
    return 0;
}
void analyze_tree()
{
    if (root == NULL)
    {
        return;
    }
    initHashTable();
    initHashTable_struct();
    Program(root);
}

void Program(Node *root)
{
    ExtDefList(root->firstChild);
}

void ExtDefList(Node *node)
{
    if (node == NULL)
    {
        return;
    }
    else
    {
        ExtDef(node->firstChild);
        ExtDefList(node->firstChild->nextSibling);
    }
}

void ExtDef(Node *node)
{
    Type *specifier = Specifier(node->firstChild);
    // 如果全局变量是一个结构体的定义，是有可能返回NULL的
    if (specifier == NULL)
    {
        return;
    }
    // Specifier ExtDecList SEMI
    if (strcmp(node->firstChild->nextSibling->name, "ExtDecList") == 0)
    {
        ExtDecList(node->firstChild->nextSibling, specifier);
    }
    // Specifier FunDec CompSt
    else if (strcmp(node->firstChild->nextSibling->name, "FunDec") == 0)
    {
        Type *func = FunDec(node->firstChild->nextSibling, specifier);
        // discard whole ExtDef
        if (func == NULL)
        {
            return;
        }
        CompSt(node->firstChild->nextSibling->nextSibling, specifier);
    }
    // Specifier SEMI
    else
    {
        // do nothing
    }
}

void ExtDecList(Node *node, Type *type)
{
    // VarDec
    FieldList *vardec = VarDec(node->firstChild, type);
    // vardec never be NULL
    if (checkDuplicate(vardec->name))
    {
        print_error(REDEF_VAR, node->firstChild->lineNo);
    }
    else
    {
        addHashNode(vardec->type, vardec->name);
    }
    // VarDec COMMA ExtDecList
    if (node->firstChild->nextSibling != NULL)
    {
        ExtDecList(node->firstChild->nextSibling->nextSibling, type);
    }
}

Type *Specifier(Node *node)
{
    // TYPE
    if (strcmp("TYPE", node->firstChild->name) == 0)
    {
        if (strcmp("float", node->firstChild->val.s) == 0)
        {
            return newType(BASIC, 1, TYPE_FLOAT);
        }
        else
        {
            return newType(BASIC, 1, TYPE_INT);
        }
    }
    // StructSpecifier
    else
    {
        Type *specifier = StructSpecifier(node->firstChild);
        return specifier;
    }
}

//
Type *StructSpecifier(Node *node)
{
    // STRUCT OptTag LC DefList RC
    if (node->firstChild->nextSibling->nextSibling != NULL)
    {
        Structure *structure = (Structure *)malloc(sizeof(Structure));
        // STRUCT OptTag LC DefList RC
        if (strcmp("OptTag", node->firstChild->nextSibling->name) == 0)
        {
            // OptTag : ID
            structure->name = node->firstChild->nextSibling->firstChild->val.s;
            structure->domain = DefList(node->firstChild->nextSibling->nextSibling->nextSibling, 1);
        }
        // STRUCT LC DefList RC
        else
        {
            structure->name == NULL;
            structure->domain = DefList(node->firstChild->nextSibling->nextSibling, 1);
        }
        Type *structspecifier = newType(STRUCTURE, 1, structure);
        if (structure->name != NULL)
        {
            if (checkDuplicate(structure->name))
            {
                print_error(DUPLIC_STRUCT, node->firstChild->nextSibling->lineNo);
                return NULL;
            }
            else
            {
                addHashNode(structspecifier, structure->name);
            }
        }
        return structspecifier;
    }
    // STRUCT Tag
    // 这里相当于用一个已经声明的结构体来定义一个变量的类型
    else
    {
        // 遍历符号表，判断是否声明过这个结构体
        // Tag: ID
        Type *type = getType(node->firstChild->nextSibling->firstChild->val.s);
        // 如果之前没有声明过
        if (type == NULL)
        {
            print_error(UNDEF_STRUCT, node->firstChild->nextSibling->lineNo);
        }
        return type;
    }
}

// Varder不做重复性的判断 留给调用者做
FieldList *VarDec(Node *node, Type *type)
{
    // ID
    if (strcmp("ID", node->firstChild->name) == 0)
    {
        FieldList *fieldlist = (FieldList *)malloc(sizeof(FieldList));
        fieldlist->name = node->firstChild->val.s;
        fieldlist->type = type;
        fieldlist->next = NULL;
        return fieldlist;
    }
    // VarDec LB INT RB
    else
    {
        Array *array = (Array *)malloc(sizeof(Array));
        array->size = node->firstChild->nextSibling->nextSibling->val.i;
        array->elem = type;
        Type *newtype = newType(ARRAY, 1, array);
        // 这个递归有点复杂，建议画个图 a[10][3]
        return VarDec(node->firstChild, newtype);
    }
}

Type *FunDec(Node *node, Type *returnType)
{
    Function *func = (Function *)malloc(sizeof(Function));
    func->name = node->firstChild->val.s;
    func->returnType = returnType;
    Type *funcType = newType(FUNCTION, 1, func);
    // 先检验函数名是否重复 或者让函数名早于参数名被添加进符号表
    if (checkDuplicate(funcType->u.function->name))
    {
        // 函数名重复，函数无效
        print_error(REDEF_FUNC, node->firstChild->nextSibling->lineNo);
        return NULL;
    }
    else
    {
        addHashNode(funcType, funcType->u.function->name);
    }
    // ID LP VarList RP
    if (strcmp(node->firstChild->nextSibling->nextSibling->name, "VarList") == 0)
    {
        // VarList中会添加参数名进符号表
        func->params = VarList(node->firstChild->nextSibling->nextSibling);
        // 如果Varlist出错，函数无效
        if (func->params == NULL)
        {
            return NULL;
        }
    }
    // ID LP RP
    else
    {
        func->params = NULL;
    }
    return funcType;
}
// 只要有一个参数有错误，报错，返回NULL
FieldList *VarList(Node *node)
{
    // ParamDec
    if (node->firstChild->nextSibling == NULL)
    {
        return ParamDec(node->firstChild);
    }
    // ParamDec COMMA VarList
    else
    {
        FieldList *firstParam = ParamDec(node->firstChild);
        if (firstParam == NULL)
        {
            return NULL;
        }
        firstParam->next = VarList(node->firstChild->nextSibling->nextSibling);
        if (firstParam->next == NULL)
        {
            return NULL;
        }
        return firstParam;
    }
}

// 先看specifier是不是重复的，才看vardec是不是重复的
FieldList *ParamDec(Node *node)
{
    // Specifier VarDec
    Type *specifier = Specifier(node->firstChild);
    if (specifier == NULL)
    {
        return NULL;
    }
    FieldList *vardec = VarDec(node->firstChild->nextSibling, specifier);
    if (checkDuplicate(vardec->name))
    {
        print_error(REDEF_VAR, node->firstChild->nextSibling->lineNo);
        return NULL;
    }
    else
    {
        addHashNode(vardec->type, vardec->name);
    }
    return vardec;
}

// 只有func才会用到Compst有错误不会直接舍弃全部，只舍弃这一条，即函数还是有用的
void CompSt(Node *node, Type *returnType)
{
    // 因为DefList和StmtList是可能为NULL的
    // LC RC
    if (node->firstChild->nextSibling->nextSibling == NULL)
    {
        return;
    }
    // LC DefList StmtList RC || LC DefList RC
    if (strcmp(node->firstChild->nextSibling->name, "DefList") == 0)
    {
        DefList(node->firstChild->nextSibling, 0);
        if (strcmp(node->firstChild->nextSibling->nextSibling->name, "StmtList") == 0)
        {
            StmtList(node->firstChild->nextSibling->nextSibling, returnType);
        }
        return;
    }
    // LC StmtList RC
    if (strcmp(node->firstChild->nextSibling->name, "StmtList") == 0)
    {
        StmtList(node->firstChild->nextSibling, returnType);
    }
}
void StmtList(Node *node, Type *returnType)
{
    // empty
    if (node == NULL)
    {
        return;
    }
    // Stmt StmtList
    Stmt(node->firstChild, returnType);
    StmtList(node->firstChild->nextSibling, returnType);
}
void Stmt(Node *node, Type *returnType)
{
    // Exp SEMI
    if (strcmp(node->firstChild->name, "Exp") == 0)
    {
        Type *exp = Exp(node->firstChild);
        if (exp == NULL)
        {
            return;
        }
    }
    // CompSt
    else if (strcmp(node->firstChild->name, "CompSt") == 0)
    {
        CompSt(node->firstChild, returnType);
    }
    // RETURN Exp SEMI
    else if (strcmp(node->firstChild->name, "RETURN") == 0)
    {
        Type *returnExp = Exp(node->firstChild->nextSibling);
        if (typecmp(returnType, returnExp) == 0)
        {
            // do nothing
        }
        else
        {
            print_error(MISMATCH_RETURN, node->firstChild->nextSibling->lineNo);
        }
    }
    // IF LP Exp RP Stmt | IF LP Exp RP Stmt ELSE Stmt
    else if (strcmp(node->firstChild->name, "IF") == 0)
    {
        Type *exp = Exp(node->firstChild->nextSibling->nextSibling);
        // 后面的stmt不被抛弃
        if (exp == NULL)
        {
            // do nothing
        }
        else if (exp->kind == BASIC && exp->u.basic == TYPE_INT)
        {
            // do nothing
        }
        else
        {
            print_error(MISMATCH_OPRAND, node->firstChild->nextSibling->nextSibling->lineNo);
        }

        Node *stmt = node->firstChild->nextSibling->nextSibling->nextSibling->nextSibling;
        Stmt(stmt, returnType);
        if (stmt->nextSibling != NULL)
        {
            Stmt(stmt->nextSibling->nextSibling, returnType);
        }
    }
    // WHILE LP Exp RP Stmt
    else if (strcmp(node->firstChild->name, "WHILE") == 0)
    {
        Type *exp = Exp(node->firstChild->nextSibling->nextSibling);
        if (exp == NULL)
        {
            // do nothing
        }
        else if (exp->kind == BASIC && exp->u.basic == TYPE_INT)
        {
            // do nothing
        }
        else
        {
            print_error(MISMATCH_OPRAND, node->firstChild->nextSibling->nextSibling->lineNo);
        }
        Node *stmt = node->firstChild->nextSibling->nextSibling->nextSibling->nextSibling;
        Stmt(stmt, returnType);
    }
}

FieldList *DefList(Node *node, int inStruct)
{
    // empty
    if (node == NULL)
    {
        return NULL;
    }
    if (node->firstChild == NULL)
    {
        return NULL;
    }
    // Def DefList
    FieldList *def = Def(node->firstChild, inStruct);
    if (def == NULL)
    {
        return DefList(node->firstChild->nextSibling, inStruct);
    }
    // 由于def也是一个list，所以需要先获取到尾部
    FieldList *def_tail = def;
    while (def_tail->next != NULL)
    {
        def_tail = def_tail->next;
    }
    def_tail->next = DefList(node->firstChild->nextSibling, inStruct);
    return def;
}
FieldList *Def(Node *node, int inStruct)
{
    if (node == NULL)
    {
        return NULL;
    }
    // Specifier DecList SEMI
    Type *specifier = Specifier(node->firstChild);
    if (specifier == NULL)
    {
        return NULL;
    }
    return DecList(node->firstChild->nextSibling, specifier, inStruct);
}
FieldList *DecList(Node *node, Type *specifier, int inStruct)
{
    // Dec COMMA DecList
    if (node->firstChild->nextSibling != NULL)
    {
        // 有一个dec错了，后面的dec还能用
        FieldList *dec = Dec(node->firstChild, specifier, inStruct);
        if (dec == NULL)
        {
            return DecList(node->firstChild->nextSibling->nextSibling, specifier, inStruct);
        }
        dec->next = DecList(node->firstChild->nextSibling->nextSibling, specifier, inStruct);
        return dec;
    }
    // Dec
    else
    {
        return Dec(node->firstChild, specifier, inStruct);
    }
}
FieldList *Dec(Node *node, Type *specifier, int inStruct)
{
    FieldList *vardec = VarDec(node->firstChild, specifier);
    // VarDec
    if (node->firstChild->nextSibling == NULL)
    {
        if (checkDuplicate(vardec->name))
        {
            if (inStruct)
            {
                print_error(REDEF_FEILD, node->firstChild->lineNo);
            }
            else
            {
                print_error(REDEF_VAR, node->firstChild->lineNo);
            }
            return NULL;
        }
        else
        {
            addHashNode(vardec->type, vardec->name);
        }
        return vardec;
    }
    // VarDec ASSIGNOP Exp
    else
    {
        if (inStruct)
        {
            print_error(REDEF_FEILD, node->firstChild->nextSibling->lineNo);
            return NULL;
        }
        else
        {
            if (checkDuplicate(vardec->name))
            {
                print_error(REDEF_VAR, node->firstChild->nextSibling->lineNo);
                return NULL;
            }
            else
            {
                addHashNode(vardec->type, vardec->name);
            }
            Type *exp = Exp(node->firstChild->nextSibling->nextSibling);
            if (typecmp(exp, vardec->type) == 0)
            {
                return vardec;
            }
            else
            {
                print_error(MISMATCH_ASSIGN, node->firstChild->nextSibling->lineNo);
                return NULL;
            }
        }
    }
}
Type *Exp(Node *node)
{
    // 必须按照节点个数为顺序！不然NULL POINT EXCEPTION
    // 先做一个子节点的
    // ID
    if (strcmp(node->firstChild->name, "ID") == 0 && node->firstChild->nextSibling == NULL)
    {
        Type *id = getType(node->firstChild->val.s);
        // 没找到
        if (id == NULL)
        {
            print_error(UNDEF_VAR, node->firstChild->lineNo);
            return NULL;
        }
        return id;
    }
    // INT
    else if (strcmp(node->firstChild->name, "INT") == 0)
    {
        return newType(BASIC, 1, TYPE_INT);
    }
    // FLOAT
    else if (strcmp(node->firstChild->name, "FLOAT") == 0)
    {
        return newType(BASIC, 1, TYPE_FLOAT);
    }
    // 再做两个子节点的
    // NOT Exp
    else if (strcmp(node->firstChild->name, "NOT") == 0)
    {
        Type *exp = Exp(node->firstChild->nextSibling);
        if (exp != NULL && exp->kind == BASIC && exp->u.basic == TYPE_INT)
        {
            return exp;
        }
        else if (exp == NULL)
        {
            return exp;
        }
        else if (exp->kind != BASIC)
        {
            print_error(MISMATCH_OPRAND, node->firstChild->nextSibling->lineNo);
            return NULL;
        }
        // not 后面不能跟float
        else if (exp->u.basic != TYPE_INT)
        {
            print_error(MISMATCH_OPRAND, node->firstChild->nextSibling->lineNo);
            return NULL;
        }
        else
        {
            return NULL;
        }
    }
    // MINUS Exp
    else if (strcmp(node->firstChild->name, "MINUS") == 0)
    {
        Type *exp = Exp(node->firstChild->nextSibling);
        if (exp != NULL && exp->kind == BASIC)
        {
            return exp;
        }
        else if (exp == NULL)
        {
            return exp;
        }
        else if (exp->kind != BASIC)
        {
            print_error(MISMATCH_OPRAND, node->firstChild->nextSibling->lineNo);
            return NULL;
        }
        else
        {
            return NULL;
        }
    }
    // 再做三个子节点的
    // Exp ASSIGNOP Exp
    else if (strcmp(node->firstChild->nextSibling->name, "ASSIGNOP") == 0)
    {
        Type *first = Exp(node->firstChild);
        Type *second = Exp(node->firstChild->nextSibling->nextSibling);
        // 如果左右两个exp有一个是NULL了，那么导致其NULL的错误已经报过，就不用报错
        // 例如 a = (3>=1.5); a[1.5] = 10;
        if (first == NULL || second == NULL)
        {
            return NULL;
        }
        // 检查左值
        Node *leftNode = node->firstChild;
        if (checkLeftExp(leftNode))
        {
            // do nothing是左值
        }
        else
        {
            print_error(EXP_ASSIGN, leftNode->lineNo);
            return NULL;
        }
        if (typecmp(first, second) == 0)
        {
            return first;
        }
        else
        {
            if (first != NULL && second != NULL)
            {
                print_error(MISMATCH_ASSIGN, node->firstChild->nextSibling->lineNo);
            }
            return NULL;
        }
    }
    // Exp AND Exp, Exp OR Exp
    else if (strcmp(node->firstChild->nextSibling->name, "AND") == 0 ||
             strcmp(node->firstChild->nextSibling->name, "OR") == 0)
    {
        Type *first = Exp(node->firstChild);
        Type *second = Exp(node->firstChild->nextSibling->nextSibling);
        if (first == NULL || second == NULL)
        {
            return NULL;
        }
        // 只有int能逻辑运算
        if (first->kind == BASIC && second->kind == BASIC && first->u.basic == TYPE_INT && second->u.basic == TYPE_INT)
        {
            return first;
        }
        else
        {
            print_error(MISMATCH_OPRAND, node->firstChild->nextSibling->lineNo);
            return NULL;
        }
    }
    // Exp RELOP Exp
    else if (strcmp(node->firstChild->nextSibling->name, "RELOP") == 0)
    {
        Type *first = Exp(node->firstChild);
        Type *second = Exp(node->firstChild->nextSibling->nextSibling);
        // only Exp of INT or FLOAT, and int as return type
        if (first == NULL || second == NULL)
        {
            return NULL;
        }
        if (typecmp(first, second) == 0 && first->kind == BASIC)
        {
            return newType(BASIC, 1, TYPE_INT);
        }
        else
        {
            print_error(MISMATCH_OPRAND, node->firstChild->nextSibling->lineNo);
            return NULL;
        }
    }
    // Exp PLUS/MINUS/DIV/STAR Exp
    else if (strcmp(node->firstChild->nextSibling->name, "PLUS") == 0 ||
             strcmp(node->firstChild->nextSibling->name, "MINUS") == 0 ||
             strcmp(node->firstChild->nextSibling->name, "DIV") == 0 ||
             strcmp(node->firstChild->nextSibling->name, "STAR") == 0)
    {
        Type *first = Exp(node->firstChild);
        Type *second = Exp(node->firstChild->nextSibling->nextSibling);
        if (first == NULL || second == NULL)
        {
            return NULL;
        }
        // only Exp of INT or FLOAT
        if (typecmp(first, second) == 0 && first->kind == BASIC)
        {
            return first;
        }
        else
        {
            print_error(MISMATCH_OPRAND, node->firstChild->nextSibling->lineNo);
            return NULL;
        }
    }
    // LP Exp LP
    else if (strcmp(node->firstChild->name, "LP") == 0)
    {
        return Exp(node->firstChild->nextSibling);
    }
    // ID LP RP
    else if (strcmp(node->firstChild->nextSibling->nextSibling->name, "RP") == 0)
    {
        Type *func = getType(node->firstChild->val.s);
        if (func == NULL)
        {
            print_error(UNDEF_FUNC, node->firstChild->lineNo);
            return NULL;
        }
        if (func->kind != FUNCTION)
        {
            print_error(NON_FUNC, node->firstChild->lineNo);
            return NULL;
        }
        // if this is paramitric func
        if (func->u.function->params != NULL)
        {
            print_error(MISMATCH_PARAM, node->firstChild->nextSibling->nextSibling->lineNo);
            return NULL;
        }
        return func->u.function->returnType;
    }
    // Exp DOT ID
    else if (strcmp(node->firstChild->nextSibling->name, "DOT") == 0)
    {
        Type *exp = Exp(node->firstChild);
        if (exp == NULL)
        {
            return NULL;
        }
        if (exp->kind != STRUCTURE)
        {
            print_error(ILLEGAL_DOT, node->firstChild->nextSibling->lineNo);
            return NULL;
        }
        FieldList *domain = exp->u.structure->domain;
        // whether the struct has the field of the same name
        while (domain != NULL)
        {
            if (strcmp(domain->name, node->firstChild->nextSibling->nextSibling->val.s) == 0)
            {
                return domain->type;
            }
            domain = domain->next;
        }
        print_error(UNDEF_FIEED, node->firstChild->nextSibling->nextSibling->lineNo);
        return NULL;
    }
    // 再做四个子节点的
    // ID LP Args RP
    else if (strcmp(node->firstChild->nextSibling->nextSibling->name, "Args") == 0)
    {
        Type *func = getType(node->firstChild->val.s);
        if (func == NULL)
        {
            print_error(UNDEF_FUNC, node->firstChild->lineNo);
            return NULL;
        }
        if (func->kind != FUNCTION)
        {
            print_error(NON_FUNC, node->firstChild->lineNo);
            return NULL;
        }
        FieldList *args = Args(node->firstChild->nextSibling->nextSibling);
        if (checkParams(args, func->u.function->params))
        {
            // do nothing even if both are NULL
        }
        else
        {
            print_error(MISMATCH_PARAM, node->firstChild->nextSibling->nextSibling->lineNo);
            return NULL;
        }
        return func->u.function->returnType;
    }
    // Exp LB Exp RB
    else if (strcmp(node->firstChild->nextSibling->name, "LB") == 0)
    {
        Type *left = Exp(node->firstChild);
        Type *right = Exp(node->firstChild->nextSibling->nextSibling);
        if (left == NULL || right == NULL)
        {
            return NULL;
        }
        if (left->kind != ARRAY)
        {
            print_error(NON_ARRAY, node->firstChild->lineNo);
            return NULL;
        }
        if (right->kind == BASIC && right->u.basic == TYPE_INT)
        {
            // do nothing
        }
        else
        {
            print_error(NON_INT, node->firstChild->nextSibling->nextSibling->lineNo);
            return NULL;
        }
        return left->u.array->elem;
    }
}
FieldList *Args(Node *node)
{
    FieldList *args = (FieldList *)malloc(sizeof(FieldList));
    Type *exp = Exp(node->firstChild);
    if (exp == NULL)
    {
        return NULL;
    }
    args->type = exp;
    // Lack of name doesn't matter since we compare params only by Type
    args->name = NULL;
    // Exp COMMA Args
    if (node->firstChild->nextSibling != NULL)
    {
        args->next = Args(node->firstChild->nextSibling->nextSibling);
    }
    // Exp
    else
    {
        /* code */
    }
    return args;
}
void print_error(ErrorType error, int lineno)
{
    fprintf(stderr, "Error type %d at Line %d: check by yourself plz.\n", error, lineno);
}