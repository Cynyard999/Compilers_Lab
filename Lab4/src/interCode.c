#include "interCode.h"

void init(){
    tempNum = 1;
    labelNum = 1;
}

void generateCode(char* fileName){
    init();
    InterCodeList* finalCode = translateExtDefList(root->firstChild);
    outputFile = fopen (fileName,"w");
    printCode(finalCode);
    fclose(outputFile);
}
void printCode(InterCodeList* finalCodeList){
    PrintCodeFunc funcTable[IC_WRITE + 1] = {
        labelPrint,
        functionPrint,
        assignPrint,
        addPrint,
        subPrint,
        mulPrint,
        divPrint,
        get_addrPrint,
        read_addrPrint,
        write_addrPrint,
        gotoPrint,
        if_gotoPrint,
        returnPrint,
        decPrint,
        argPrint,
        callPrint,
        paramPrint,
        readPrint,
        writePrint
    };
    InterCode* codeLine = finalCodeList->head;
    while (codeLine != NULL)
    {
        //fprintf(stderr,"%d \n",codeLine->kind);
        funcTable[codeLine->kind](codeLine);
        codeLine = codeLine->next;
        fprintf(outputFile,"\n");
    }    
}

void printOperand(Operand* op){
    switch (op->kind) {
        case OP_CONSTANT:
            fprintf(outputFile, "#%d", op->u.value);
            break;
        case OP_VARIABLE:
        case OP_ADDRESS:
        case OP_LABEL:
        case OP_FUNCTION:
            fprintf(outputFile, "%s", op->u.name);
            break;
    }
}

int getSize(Type* type){
    if (type == NULL)
    {
        return 0;
    }
    else if (type->kind == BASIC)
    {
        return 4;
    }
    else if (type->kind == ARRAY)
    {
        return type->u.array->size* getSize(type->u.array->elem);
    }
    else if (type->kind == STRUCTURE)
    {
        int size = 0;
        FieldList* fields = type->u.structure->domain;
        while (fields!= NULL)
        {
            size += getSize(fields->type);
            fields = fields->next;
        }
        return size;
    }
    return 0;
}


InterCodeList* join(InterCodeList* first, InterCodeList* second){
    if (first == NULL && second != NULL)
    {
        //fprintf(stderr,"NULL first\n ");
        return second;
    }
    if (second == NULL && first != NULL)
    {
        return first;
    }
    if (second == NULL && first == NULL)
    {
        return newInterCodeList();
    }
    if (first->head == NULL && second->head != NULL)
    {
        return second;
    }
    if (first->head != NULL && second->head == NULL)
    {
        return first;
    }
    if (first->head == NULL && second->head == NULL)
    {
        return first;
    }
    
    first->tail->next = second->head;
    first->tail = second->tail;
    return first;
}

Operand* newOperand(OperandKind kind){
     Operand* op = (Operand*)malloc(sizeof(Operand));
     op->kind = kind;
     return op;
}

Operand* newTemp(){
    Operand* op = newOperand(OP_VARIABLE);
    char *tempName = (char*)malloc(10*sizeof(char));;
    sprintf(tempName,"t%d",tempNum);
    op->u.name = tempName;
    tempNum++;
    return op;
}

Operand* newLabel(){
    Operand* op = newOperand(OP_LABEL);
    char *labelName = (char*)malloc(20*sizeof(char));;
    sprintf(labelName,"label%d",labelNum);
    op->u.name = labelName;
    labelNum++;
    return op;
}

Operand* newConstant(int val){
    Operand* op = newOperand(OP_CONSTANT);
    op->u.value = val;
    return op;
}

InterCodeList* newInterCodeList(){
    InterCodeList* codeList = (InterCodeList*)malloc(sizeof(InterCodeList));
    codeList->head = codeList->tail = NULL;
    return codeList;
}


InterCode* newInterCode(CodeKind kind){
    InterCode* code = (InterCode*)malloc(sizeof(InterCode));
    code->kind = kind;
    code->next = NULL;
    return code;
}

ArgList* newArgList(){
    ArgList* arglist = (ArgList*)malloc(sizeof(ArgList));
    arglist->head = NULL;
    arglist->tail = NULL;
    return arglist;
}
Arg* newArg(Operand* op){
    Arg* arg = (Arg*)malloc(sizeof(Arg));
    arg->operand = op;
    arg->next = NULL;
    return arg;
}
void addArg(ArgList* arglist,Arg* arg){
    if (arglist == NULL)
    {
        arglist = newArgList();
    }
    
    if (arglist->head == NULL)
    {
        arglist->head = arg;
        arglist->tail = arg;
        return;
    }
    if (arg == NULL)
    {
        return;
    }
    // 往前面插入
    Arg* temp  = arglist->head;
    arglist->head = arg;
    arg->next = temp;
}



void addInterCode(InterCodeList* list, InterCode* code){
    if (list == NULL)
    {
        // 其实下面这个代码是没用的
        list = newInterCodeList();
    }
    if (list->head == NULL)
    {
        list->head = code;
        list->tail = code;
        return;
    }
    if (code == NULL)
    {
        return;
    }
    list->tail->next = code;
    list->tail = code;
}

InterCodeList* translateExtDefList(Node* node){
    // ExtDefList -> ExtDef ExtDefList
    //             | e
    if (node == NULL)
    {
        return NULL;
    }
    
    InterCodeList* extdef = translateExtDef(node->firstChild);
    InterCodeList* extdeflist = translateExtDefList(node->firstChild->nextSibling);
    return join(extdef,extdeflist);
}
InterCodeList* translateExtDef(Node* node){
    // ExtDef -> Specifier ExtDecList SEMI
    //         | Specifier SEMI
    //         | Specifier FunDec CompSt
    InterCodeList* fundec = translateFunDec(node->firstChild->nextSibling);
    InterCodeList* compSt = translateCompSt(node->firstChild->nextSibling->nextSibling);
    return join(fundec,compSt);
}
InterCodeList* translateVarDec(Node* node, Operand* place){
    // VarDec -> ID
    //         | VarDec LB INT RB
    if (node == NULL)
    {
        return NULL;
    }
    if (strcmp(node->firstChild->name,"ID") == 0)
    {
        Type* type = getType(node->firstChild->val.s);
        if (type->kind == BASIC)
        {
            if (place)
            {
                tempNum--;
                // 把place变成variable而不是临时变量
                place->kind = OP_VARIABLE;
                // 名字变成原先的名字
                place->u.name = node->firstChild->val.s;
                return NULL;
            }
        }
        else if (type->kind == ARRAY)
        {
            // 一维数组,需要dec语句
            if (type->u.array->elem->kind == BASIC)
            {
                Operand* op = newOperand(OP_VARIABLE);
                op->u.name = node->firstChild->val.s;
                InterCode* decCode = newInterCode(IC_DEC);
                decCode->u.dec.op = op;
                decCode->u.dec.size = getSize(type);
                InterCodeList* res = newInterCodeList();
                addInterCode(res,decCode);
                return res;
            }
            // 高维数组
            else
            {
                fprintf(stderr,"Cannot translate: Code containsvariables of "
                    "multi-dimensional array type or parameters of array "
                    "type.\n");
                return NULL;
            } 
        }
        else
        {
            // ID的种类不是基本变量
        }  
        return NULL;
    }
    else
    {
        return translateVarDec(node->firstChild,place);
    }
}
InterCodeList* translateFunDec(Node* node){
    // FunDec -> ID LP VarList RP
    //         | ID LP RP
    if (node == NULL)
    {
        return NULL;
    }
    Operand* op = newOperand(OP_FUNCTION);
    op->u.name = node->firstChild->val.s;
    InterCode* code = newInterCode(IC_FUNCTION);
    code->u.monoOp.op = op;
    InterCodeList* list = newInterCodeList();
    addInterCode(list,code);
    //fprintf(stderr,"%s, \n",node->firstChild->name);
    if (strcmp(node->firstChild->nextSibling->nextSibling->name,"VarList")==0)
    {
        Function* func = getType(node->firstChild->val.s)->u.function;
        FieldList* param = func->params;
        while (param!=NULL)
        {
            Operand* paramOp = newOperand(OP_VARIABLE);
            paramOp->u.name = param->name;
            InterCode* paramIc = newInterCode(IC_PARAM);
            paramIc->u.monoOp.op = paramOp;
            addInterCode(list,paramIc);
            param = param->next;
        }     
    }
    return list;
}
InterCodeList* translateStmtList(Node* node){
    // StmtList -> Stmt StmtList
    //           | e
    if (node == NULL)
    {
        return NULL;
    }
    InterCodeList* stmt = translateStmt(node->firstChild);
    InterCodeList* stmtlist = translateStmtList(node->firstChild->nextSibling);
    return join(stmt,stmtlist);
}
InterCodeList* translateCompSt(Node* node){
    // CompSt -> LC DefList StmtList RC
    if (node == NULL)
    {
        return NULL;
    }
    Node* temp = node->firstChild->nextSibling;
    InterCodeList* code1 = NULL;
    InterCodeList* code2 = NULL;
    if (strcmp(temp->name,"DefList")== 0)
    {
        code1 =  translateDefList(temp);
        temp = temp->nextSibling;
    }
    if (strcmp(temp->name,"StmtList")== 0)
    {
        code2 = translateStmtList(temp);
    }
    return join(code1,code2);
}
InterCodeList* translateStmt(Node* node){
    // Stmt -> Exp SEMI
    //       | CompSt
    //       | RETURN Exp SEMI
    //       | IF LP Exp RP Stmt
    //       | IF LP Exp RP Stmt ELSE Stmt
    //       | WHILE LP Exp RP Stmt
    //fprintf(stderr,"in Stmt %d %s \n",node->lineNo,node->name);
    if (node == NULL || node->firstChild == NULL)
    {
        return NULL;
    }
    
    if (strcmp(node->firstChild->name,"Exp") == 0)
    {
        return translateExp(node->firstChild, NULL);
    }
    if (strcmp(node->firstChild->name,"CompSt")==0)
    {
        return translateCompSt(node->firstChild);
    }
    if (strcmp(node->firstChild->name,"RETURN")==0)
    {
        Operand* temp = newTemp();
        // 生成一条语句，将exp的计算值放入temp中； temp = xxxx
        InterCodeList* exp =  translateExp(node->firstChild->nextSibling,temp);
        // 再生成一条语句，return temp
        InterCodeList* returnExp = newInterCodeList();
        InterCode* returncode = newInterCode(IC_RETURN);
        returncode->u.monoOp.op = temp;
        addInterCode(returnExp,returncode);
        // 连接这两条语句
        return join(exp,returnExp);
    }
    if (strcmp(node->firstChild->name,"IF")==0)
    {
        Node* exp = node->firstChild->nextSibling->nextSibling;
        Node* stmt = exp->nextSibling->nextSibling;
        Operand* label1 = newLabel();
        Operand* label2 = newLabel();
        // if xxx goto label1
        // goto label2
        InterCodeList* condition = translateCond(exp,label1,label2);
        // Label label1: 
        InterCode* label1Code = newInterCode(IC_LABEL);
        label1Code->u.monoOp.op = label1;
        addInterCode(condition,label1Code);
        // 得到stmt的语句集
        InterCodeList* stmtCodeList1 = translateStmt(stmt);
        // 再加在Label1的后面
        condition = join(condition,stmtCodeList1);
        if (stmt->nextSibling==NULL)
        {
            // Label label2:
            InterCode* label2Code = newInterCode(IC_LABEL);
            label2Code->u.monoOp.op = label2;
            addInterCode(condition,label2Code);
        }
        else
        {
            // GOTO label3;
            // Label label2;
            // xxx
            // Label label3;
            Operand* label3 = newLabel();
            InterCode* gotoLabel3Code = newInterCode(IC_GOTO);
            gotoLabel3Code->u.monoOp.op = label3;
            addInterCode(condition,gotoLabel3Code);
            InterCode* label2Code = newInterCode(IC_LABEL);
            label2Code->u.monoOp.op = label2;
            addInterCode(condition,label2Code);
            // 得到stmt的语句集
            InterCodeList* stmtCodeList2 = translateStmt(stmt->nextSibling->nextSibling);
            condition =  join(condition,stmtCodeList2);
            InterCode* label3Code = newInterCode(IC_LABEL);
            label3Code->u.monoOp.op = label3;
            addInterCode(condition,label3Code);
        }
        return condition;
    }
    if (strcmp(node->firstChild->name,"WHILE")==0)
    {
        Operand* label1 = newLabel();
        Operand* label2 = newLabel();
        Operand* label3 = newLabel();

        InterCode* label1Code = newInterCode(IC_LABEL);
        label1Code->u.monoOp.op = label1;
        InterCode* gotoLabel1Code = newInterCode(IC_GOTO);
        gotoLabel1Code->u.monoOp.op = label1;
        InterCode* label2Code = newInterCode(IC_LABEL);
        label2Code->u.monoOp.op = label2;
        InterCode* label3Code = newInterCode(IC_LABEL);
        label3Code->u.monoOp.op = label3;

        InterCodeList* whileCodeList = newInterCodeList();
        addInterCode(whileCodeList,label1Code);

        whileCodeList =  join(whileCodeList,translateCond(node->firstChild->nextSibling->nextSibling,label2,label3));
        addInterCode(whileCodeList,label2Code);
        whileCodeList =  join(whileCodeList,translateStmt(node->firstChild->nextSibling->nextSibling->nextSibling->nextSibling));
        addInterCode(whileCodeList,gotoLabel1Code);
        addInterCode(whileCodeList,label3Code);
        return whileCodeList;
    }
    return NULL;
}
InterCodeList* translateDefList(Node* node){
    // DefList -> Def DefList
    //          | e
    if (node == NULL)
    {
        return NULL;
    }
    InterCodeList* code1 = translateDef(node->firstChild);
    InterCodeList* code2 = translateDefList(node->firstChild->nextSibling);
    return join(code1,code2);
}
InterCodeList* translateDef(Node* node){
    if (node == NULL)
    {
        return NULL;
    }
    // Def -> Specifier DecList SEMI
    return translateDecList(node->firstChild->nextSibling);
}
InterCodeList* translateDecList(Node* node){
    // DecList -> Dec
    //          | Dec COMMA DecList
    if (node == NULL)
    {
        return NULL;
    }
    InterCodeList* code1 = translateDec(node->firstChild);
    if (node->firstChild->nextSibling!=NULL)
    {
        InterCodeList* code2 = translateDecList(node->firstChild->nextSibling->nextSibling);
        code1 = join(code1,code2);
    }
    return code1;
    
    
}
InterCodeList* translateDec(Node* node){
    // Dec -> VarDec
    //      | VarDec ASSIGNOP Exp
    if (node == NULL)
    {
        return NULL;
    }
    if (node->firstChild->nextSibling ==NULL)
    {
        // Dec -> VarDec
        return translateVarDec(node->firstChild,NULL);
    }
    else
    {
        // 
        Operand* t1 = newTemp();
        // code1可能是NULL
        InterCodeList* code1 =  translateVarDec(node->firstChild,t1);
        Operand* t2 = newTemp();
        InterCodeList* code2 =  translateExp(node->firstChild->nextSibling->nextSibling,t2);
        InterCode* assignCode = newInterCode(IC_ASSIGN);
        assignCode->u.assign.left = t1;
        assignCode->u.assign.right = t2;
        code1 = join(code1,code2);    
        addInterCode(code1,assignCode);
        return code1;
    }
    
    
}
InterCodeList* translateExp(Node* node, Operand* place){
    // Exp -> Exp ASSIGNOP Exp
    //      | Exp AND Exp
    //      | Exp OR Exp
    //      | Exp RELOP Exp

    //      | Exp PLUS Exp
    //      | Exp MINUS Exp
    //      | Exp STAR Exp
    //      | Exp DIV Exp

    //      | LP Exp RP

    //      | MINUS Exp

    //      | NOT Exp

    //      | ID LP Args RP
    //      | ID LP RP

    //      | Exp LB Exp RB
    //      | Exp DOT ID

    //      | ID
    //      | INT
    //      | FLOAT
    if (node == NULL)
    {
        return NULL;
    }
    // ID
    //fprintf(stderr,"%s in %d\n",node->name,node->lineNo);
    if (strcmp(node->firstChild->name,"ID") == 0 && node->firstChild->nextSibling == NULL){
        Type* type = getType(node->firstChild->val.s);
        if (type->kind == STRUCTURE||type->kind == ARRAY)
        {
            
            InterCode* code = newInterCode(IC_GET_ADDR);
            code->u.assign.left = place;
            Operand* op = newOperand(OP_VARIABLE);
            op->u.name = node->firstChild->name;
            code->u.assign.right = op;
            InterCodeList* res = newInterCodeList();
            addInterCode(res,code);
            return res;
        }
        else
        // ID
        {
            if (place!=NULL)
            {
                place->u.name = node->firstChild->val.s;
                tempNum--;
                return NULL;
            }
        }   
    }
    // INT
    else if (strcmp(node->firstChild->name,"INT") == 0){
        int val = node->firstChild->val.i;
        Operand* op = newConstant(val);
        InterCode* intCode = newInterCode(IC_ASSIGN);
        intCode->u.assign.left = place;
        intCode->u.assign.right = op;
        InterCodeList* res = newInterCodeList();
        addInterCode(res,intCode);
        return res;
    }
    // LP Exp RP
    else if (strcmp(node->firstChild->name,"LP") == 0)
    {
        return translateExp(node->firstChild->nextSibling,place);
    }
    // MINUS Exp
    else if (strcmp(node->firstChild->name,"MINUS") == 0){
        Operand* t1 = newTemp();
        // t1 = xxxx
        // t2 = 0-t1
        InterCodeList* code1 = translateExp(node->firstChild->nextSibling,t1);
        Operand* zero = newOperand(OP_CONSTANT);
        zero->u.value = 0;
        InterCode* minusCode = newInterCode(IC_SUB);
        minusCode->u.binOp.result = place;
        minusCode->u.binOp.op1 = zero;
        minusCode->u.binOp.op2 = t1;
        addInterCode(code1,minusCode);
        return code1;
    }
    // NOT Exp, AND, OR, RELOP
    else if (strcmp(node->firstChild->name,"NOT") == 0
    ||strcmp(node->firstChild->nextSibling->name,"AND") == 0
    ||strcmp(node->firstChild->nextSibling->name,"OR") == 0
    ||strcmp(node->firstChild->nextSibling->name,"RELOP") == 0)
    {
            Operand* label1 = newLabel();
            Operand* label2 = newLabel();

            // false
            InterCode* code1 = newInterCode(IC_ASSIGN);
            code1->u.assign.left = place;
            code1->u.assign.right = newConstant(0);
            InterCodeList* codeList1 = newInterCodeList();
            addInterCode(codeList1,code1);


            InterCodeList* codeList2 = translateCond(node,label1,label2);

            InterCode* code2 = newInterCode(IC_LABEL);
            code2->u.monoOp.op = label1;
            addInterCode(codeList2,code2);

            // true
            InterCode* code3 = newInterCode(IC_ASSIGN);
            code3->u.assign.left = place;
            code3->u.assign.right = newConstant(1);
            addInterCode(codeList2,code3);


            InterCode* code4 = newInterCode(IC_LABEL);
            code4->u.monoOp.op = label2;
            addInterCode(codeList2,code4);

            return join(codeList1,codeList2);
    }
    // Exp Assign Exp
    else if (strcmp(node->firstChild->nextSibling->name,"ASSIGNOP") == 0)
    {
        Operand* t1 = newTemp();
        InterCodeList* code1 = translateExp(node->firstChild,t1);
        Operand* t2 = newTemp();
        InterCodeList* code2 = translateExp(node->firstChild->nextSibling->nextSibling,t2);
        code1 = join(code1,code2);
        // t1 = t2
        InterCode* assignCode = newInterCode(IC_ASSIGN);
        if (t1->kind == OP_ADDRESS&&t2->kind!= OP_ADDRESS)
        {
            //*t1 = t2
            assignCode->kind = IC_WRITE_ADDR;
        }
        else if (t1->kind != OP_ADDRESS&&t2->kind == OP_ADDRESS)
        {
            // t1 = *t2
            assignCode->kind = IC_READ_ADDR;
        }
        else if (t1->kind == OP_ADDRESS&&t2->kind == OP_ADDRESS)
        {
            assignCode->kind = IC_WRITE_ADDR;
            Operand* t3 = newTemp();
            InterCode* readAddr = newInterCode(IC_READ_ADDR);
            readAddr->u.assign.left = t3;
            readAddr->u.assign.right = t2;
            addInterCode(code1,readAddr);
            t2 = t3;
        }
        
        assignCode->u.assign.left = t1;
        assignCode->u.assign.right = t2;
        addInterCode(code1,assignCode);
        if (place!= NULL)
        {
            InterCode* assignCode2 = newInterCode(IC_ASSIGN);
            assignCode2->u.assign.left = place;
            assignCode2->u.assign.right = t1;
            addInterCode(code1,assignCode2);
        }
        return code1;
    }
    // 加减乘除
    else if (strcmp(node->firstChild->nextSibling->name,"PLUS") == 0
    || strcmp(node->firstChild->nextSibling->name,"MINUS") == 0
    || strcmp(node->firstChild->nextSibling->name,"STAR") == 0
    || strcmp(node->firstChild->nextSibling->name,"DIV") == 0
    )
    {
       Operand* t1 = newTemp();
       InterCodeList* exp1 = translateExp(node->firstChild,t1);
       if (exp1 == NULL)
       {
           exp1 = newInterCodeList();
       }

        if (t1->kind == OP_ADDRESS)
        {
            Operand* temp3 = newTemp();
            InterCode* code1  = newInterCode(IC_READ_ADDR);
            code1->u.assign.left = temp3;
            code1->u.assign.right = t1;
            addInterCode(exp1,code1);
            t1 = temp3;
        }
       
       Operand* t2 = newTemp();
       InterCodeList* exp2 = translateExp(node->firstChild->nextSibling->nextSibling,t2);
       exp1 = join(exp1,exp2);
        if (t2->kind == OP_ADDRESS)
        {
            Operand* temp4 = newTemp();
            InterCode* code2  = newInterCode(IC_READ_ADDR);
            code2->u.assign.left = temp4;
            code2->u.assign.right = t2;
            addInterCode(exp1,code2);
            t2 = temp4;
        }
       if (strcmp(node->firstChild->nextSibling->name,"PLUS") == 0)
       {
           InterCode* code = newInterCode(IC_ADD);
           code->u.binOp.result = place;
           code->u.binOp.op1 = t1;
           code->u.binOp.op2 = t2;
           addInterCode(exp1,code);
       }
       else if (strcmp(node->firstChild->nextSibling->name,"MINUS") == 0)
       {
           InterCode* code = newInterCode(IC_SUB);
           code->u.binOp.result = place;
           code->u.binOp.op1 = t1;
           code->u.binOp.op2 = t2;
           addInterCode(exp1,code);
       }
       else if (strcmp(node->firstChild->nextSibling->name,"STAR") == 0)
       {
           InterCode* code = newInterCode(IC_MUL);
           code->u.binOp.result = place;
           code->u.binOp.op1 = t1;
           code->u.binOp.op2 = t2;
           addInterCode(exp1,code);
       }
       else if (strcmp(node->firstChild->nextSibling->name,"DIV") == 0)
       {
           InterCode* code = newInterCode(IC_DIV);
           code->u.binOp.result = place;
           code->u.binOp.op1 = t1;
           code->u.binOp.op2 = t2;
           addInterCode(exp1,code);
       }
       return exp1;
    }
    // Exp DOT ID
    else if (strcmp(node->firstChild->nextSibling->name,"DOT") == 0)
    {
        // TODO
        // 结构体访问
    }
    // Exp LB Exp RB
    // ID LB Exp RB
    else if (strcmp(node->firstChild->nextSibling->name,"LB") == 0)
    {
        // 一维数组
        // op[j]
        // t18 := j * #4
        // t19 := &op
        // t20 := t19 + t18

        // code1计算基址
        Operand* baseAddr = newTemp();
        baseAddr->kind = OP_ADDRESS;
        InterCode* getAddrCode = newInterCode(IC_GET_ADDR);
        Operand* target = newOperand(OP_VARIABLE);
        target->u.name = node->firstChild->firstChild->val.s;
        getAddrCode->u.assign.left = baseAddr;
        getAddrCode->u.assign.right = target;
        InterCodeList* code1 = newInterCodeList();
        addInterCode(code1,getAddrCode);

        Operand* offset = newTemp();
        Operand* index = newTemp();

        // code2计算偏移量
        InterCodeList* code2 = translateExp(node->firstChild->nextSibling->nextSibling,index);
        code1 = join(code1,code2);


        InterCode* getOffset = newInterCode(IC_MUL);
        getOffset->u.binOp.result = offset;
        getOffset->u.binOp.op1 = index;
        getOffset->u.binOp.op2 = newConstant(4);
        addInterCode(code1,getOffset);
        // code3通过偏移量和基址得到实际地址
        Operand* realAddr = newTemp();
        realAddr->kind = OP_ADDRESS;
        if (place != NULL)
        {
            InterCode* getRealAddr = newInterCode(IC_ADD);
            getRealAddr->u.binOp.result = realAddr;
            getRealAddr->u.binOp.op1 = baseAddr;
            getRealAddr->u.binOp.op2 = offset;
            
            InterCode* transferRealAddr = newInterCode(IC_ASSIGN);
            transferRealAddr->u.assign.left = place;
            transferRealAddr->u.assign.right = realAddr;
            addInterCode(code1,getRealAddr);
            addInterCode(code1, transferRealAddr);
            // 上面那一段assign都可以不要 但直接更改place可能会有问题
            place->kind = OP_ADDRESS;
        }
        return code1;
    }
    // ID LP Args RP
    // ID LP RP
    else if (strcmp(node->firstChild->nextSibling->name,"LP")==0 && strcmp(node->firstChild->name,"ID")==0)
    {
        Operand* func = newOperand(OP_FUNCTION);
        func->u.name = node->firstChild->val.s;
        // ID LP RP
        if (strcmp(node->firstChild->nextSibling->nextSibling->name,"RP")==0)
        {
            // read肯定有place
            if (strcmp(node->firstChild->val.s,"read")==0)
            {
                InterCode* readFunc = newInterCode(IC_READ);
                readFunc->u.monoOp.op = place;
                InterCodeList* readCode = newInterCodeList();
                addInterCode(readCode,readFunc);
                return readCode;
            }
            else
            {
                InterCode* callFunc = newInterCode(IC_CALL);
                if (place)
                {
                    callFunc->u.assign.left = place;
                    callFunc->u.assign.right = func;
                }
                else
                { 
                    Operand* temp = newTemp();
                    callFunc->u.assign.left = temp;
                    callFunc->u.assign.right = func;
                }
                InterCodeList* callCode = newInterCodeList();
                addInterCode(callCode,callFunc);
                return callCode;
            }
        }
        // ID LP Args RP
        else
        {
            ArgList* arglist = newArgList();
            // Arg xxx
            InterCodeList* codeList1 = translateArgs(node->firstChild->nextSibling->nextSibling,arglist);
            
            if (strcmp(node->firstChild->val.s,"write") == 0)
            {
                if (codeList1 == NULL)
                {
                    codeList1 = newInterCodeList();
                }
                InterCode* writeCode = newInterCode(IC_WRITE);
                writeCode->u.monoOp.op = arglist->head->operand;
                addInterCode(codeList1,writeCode);
                return codeList1;
            }
            else
            {
                InterCodeList* codeList2 = newInterCodeList();
                Arg* arg = arglist->head;
                while (arg != NULL)
                {
                    InterCode* argcode = newInterCode(IC_ARG);
                    argcode->u.monoOp.op = arg->operand;
                    arg = arg->next;
                    addInterCode(codeList2,argcode);
                }
                InterCode* callFunc = newInterCode(IC_CALL);
                Operand* func = newOperand(OP_FUNCTION);
                func->u.name = node->firstChild->val.s;
                callFunc->u.assign.right = func;
                if (place)
                {
                    callFunc->u.assign.left = place;
                }
                else
                {
                    Operand* temp = newTemp();
                    callFunc->u.assign.left = temp;
                }
                addInterCode(codeList2,callFunc);
                codeList1 = join(codeList1,codeList2);
                return codeList1;
            }
        } 
    }
    else
    {
        return NULL;
    }  
}
// Args -> Exp COMMA Args
//       | Exp
InterCodeList* translateArgs(Node* node, ArgList* argList){
    if (node == NULL)
    {
        return NULL;
    }
    
    Operand* temp = newTemp();
    // temp = xxx
    InterCodeList* exp = translateExp(node->firstChild,temp);
    // 参数是数组
    if (temp->kind == OP_ADDRESS)
    {
        // temp2 = *temp1
        Operand* temp2 = newTemp();
        InterCode* readAddr = newInterCode(IC_READ_ADDR);
        readAddr->u.assign.left = temp2;
        readAddr->u.assign.right = temp;
        addInterCode(exp,readAddr);
        Arg* arg = newArg(temp2);
        addArg(argList,arg);
    }
    // 参数是简单变量
    else
    {
        Arg* arg = newArg(temp);
        addArg(argList,arg);
    }
    // Exp COMMA Args
    if (node->firstChild->nextSibling!=NULL)
    {
        InterCodeList* args = translateArgs(node->firstChild->nextSibling->nextSibling,argList);
        exp = join(exp,args);
    }
    return exp;
}
InterCodeList* translateCond(Node* node, Operand* labelTrue, Operand* labelFalse){
    // Exp -> Exp AND Exp
    //      | Exp OR Exp
    //      | Exp RELOP Exp
    //      | NOT Exp
    //      | ID INT FLOAT

    // Exp -> NOT Exp
    if (strcmp(node->firstChild->name,"NOT") == 0)
    {
        return translateCond(node->firstChild->nextSibling,labelFalse,labelTrue);
    }
    else if (strcmp(node->firstChild->name,"ID") == 0 && node->firstChild->nextSibling == NULL
            || strcmp(node->firstChild->name,"INT") == 0
            || strcmp(node->firstChild->name,"FLOAT") == 0)
    {
        Operand* temp1 = newTemp();
        InterCodeList* codeList1;
        codeList1 = translateExp(node,temp1);
        if (codeList1 == NULL)
        {
            codeList1 = newInterCodeList();
        }
        Operand* temp2 = newConstant(0);
        char* relop = "!=";
        if (temp1->kind == OP_ADDRESS)
        {
            Operand* temp3 = newTemp();
            InterCode* code1  = newInterCode(IC_READ_ADDR);
            code1->u.assign.left = temp3;
            code1->u.assign.right = temp1;
            addInterCode(codeList1,code1);
            temp1 = temp3;
        }
        InterCode* ifgoto = newInterCode(IC_IF_GOTO);
        ifgoto->u.if_goto.relop = relop;
        ifgoto->u.if_goto.x = temp1;
        ifgoto->u.if_goto.y = temp2;
        ifgoto->u.if_goto.z = labelTrue;
        InterCode* gotocode = newInterCode(IC_GOTO);
        gotocode->u.monoOp.op = labelFalse;
        addInterCode(codeList1,ifgoto);
        addInterCode(codeList1,gotocode);
        return codeList1;
    }
    
    else if (strcmp(node->firstChild->nextSibling->name,"RELOP") == 0)
    {
        Operand* temp1 = newTemp();
        InterCodeList* codeList1 = translateExp(node->firstChild,temp1);
        Operand* temp2 = newTemp();
        InterCodeList* codeList2 = translateExp(node->firstChild->nextSibling->nextSibling,temp2);
        codeList1 = join(codeList1,codeList2);
        if (temp1->kind == OP_ADDRESS)
        {
            Operand* temp3 = newTemp();
            InterCode* code1  = newInterCode(IC_READ_ADDR);
            code1->u.assign.left = temp3;
            code1->u.assign.right = temp1;
            addInterCode(codeList1,code1);
            temp1 = temp3;
        }
        if (temp2->kind == OP_ADDRESS)
        {
            Operand* temp4 = newTemp();
            InterCode* code2  = newInterCode(IC_READ_ADDR);
            code2->u.assign.left = temp4;
            code2->u.assign.right = temp2;
            addInterCode(codeList1,code2);
            temp2 = temp4;
        }
        InterCode* ifgoto = newInterCode(IC_IF_GOTO);
        ifgoto->u.if_goto.relop = node->firstChild->nextSibling->val.s;
        //fprintf(stderr,"%s\n",node->firstChild->nextSibling->val.s);
        ifgoto->u.if_goto.x = temp1;
        ifgoto->u.if_goto.y = temp2;
        ifgoto->u.if_goto.z = labelTrue;
        InterCode* gotocode = newInterCode(IC_GOTO);
        gotocode->u.monoOp.op = labelFalse;
        addInterCode(codeList1,ifgoto);
        addInterCode(codeList1,gotocode);
        return codeList1;
    }
    else if (strcmp(node->firstChild->nextSibling->name,"AND") == 0){
        Operand* label1 = newLabel();
        InterCodeList* codelist1 = translateCond(node->firstChild,label1,labelFalse);
        InterCode* labelCode = newInterCode(IC_LABEL);
        labelCode->u.monoOp.op = label1;
        addInterCode(codelist1,labelCode);
        InterCodeList* codelist2 = translateCond(node->firstChild->nextSibling->nextSibling,labelTrue,labelFalse);
        codelist1 =  join(codelist1,codelist2);
        return codelist1;
    }
    else if (strcmp(node->firstChild->nextSibling->name,"OR") == 0){
        Operand* label1 = newLabel();
        InterCodeList* codelist1 = translateCond(node->firstChild,labelTrue,label1);
        InterCode* labelCode = newInterCode(IC_LABEL);
        labelCode->u.monoOp.op = label1;
        addInterCode(codelist1,labelCode);
        InterCodeList* codelist2 = translateCond(node->firstChild->nextSibling->nextSibling,labelTrue,labelFalse);
        codelist1 =  join(codelist1,codelist2);
        return codelist1;
    }
    else
    {

        Operand* temp1 = newTemp();
        InterCodeList* codeList1;
        // (Exp)
        if (strcmp(node->firstChild->name,"LP")==0)
        {
            codeList1 = translateExp(node->firstChild->nextSibling,temp1);
        }
        // Exp -> ID ()
        else
        {
            codeList1 = translateExp(node,temp1);
        }
        Operand* temp2 = newConstant(0);
        char* relop = "!=";
        if (temp1->kind == OP_ADDRESS)
        {
            Operand* temp3 = newTemp();
            InterCode* code1  = newInterCode(IC_READ_ADDR);
            code1->u.assign.left = temp3;
            code1->u.assign.right = temp1;
            addInterCode(codeList1,code1);
            temp1 = temp3;
        }
        InterCode* ifgoto = newInterCode(IC_IF_GOTO);
        ifgoto->u.if_goto.relop = relop;
        ifgoto->u.if_goto.x = temp1;
        ifgoto->u.if_goto.y = temp2;
        ifgoto->u.if_goto.z = labelTrue;
        InterCode* gotocode = newInterCode(IC_GOTO);
        gotocode->u.monoOp.op = labelFalse;
        addInterCode(codeList1,ifgoto);
        addInterCode(codeList1,gotocode);
        return codeList1;
    }
}


void labelPrint(InterCode* code){
    fprintf(outputFile,"LABEL ");
    printOperand(code->u.monoOp.op);
    fprintf(outputFile," :");
}
void functionPrint(InterCode* code){
    fprintf(outputFile,"FUNCTION ");
    printOperand(code->u.monoOp.op);
    fprintf(outputFile," :");
}
void assignPrint(InterCode* code){
    // 可能使用place对left赋值，place可能为NULL
    if (code->u.assign.left == NULL || code->u.assign.right == NULL)
    {
        return;
    }
    printOperand(code->u.assign.left);
    fprintf(outputFile," := ");
    printOperand(code->u.assign.right);
}
void addPrint(InterCode* code){
    if (code->u.binOp.result == NULL)
    {
        return;
    }
    printOperand(code->u.binOp.result);
    fprintf(outputFile," := ");
    printOperand(code->u.binOp.op1);
    fprintf(outputFile," + ");
    printOperand(code->u.binOp.op2);
}
void subPrint(InterCode* code){
    if (code->u.binOp.result == NULL)
    {
        return;
    }
    printOperand(code->u.binOp.result);
    fprintf(outputFile," := ");
    printOperand(code->u.binOp.op1);
    fprintf(outputFile," - ");
    printOperand(code->u.binOp.op2);
}
void mulPrint(InterCode* code){
    if (code->u.binOp.result == NULL)
    {
        return;
    }
    printOperand(code->u.binOp.result);
    fprintf(outputFile," := ");
    printOperand(code->u.binOp.op1);
    fprintf(outputFile," * ");
    printOperand(code->u.binOp.op2);
}
void divPrint(InterCode* code){
    if (code->u.binOp.result == NULL)
    {
        return;
    }
    printOperand(code->u.binOp.result);
    fprintf(outputFile," := ");
    printOperand(code->u.binOp.op1);
    fprintf(outputFile," / ");
    printOperand(code->u.binOp.op2);
}
void get_addrPrint(InterCode* code){
    printOperand(code->u.assign.left);
    fprintf(outputFile," := &");
    printOperand(code->u.assign.right);
}
void read_addrPrint(InterCode* code){
    printOperand(code->u.assign.left);
    fprintf(outputFile," := *");
    printOperand(code->u.assign.right);
}
void write_addrPrint(InterCode* code){
    fprintf(outputFile,"*");
    printOperand(code->u.assign.left);
    fprintf(outputFile," := ");
    printOperand(code->u.assign.right);
}
void gotoPrint(InterCode* code){
    fprintf(outputFile,"GOTO ");
    printOperand(code->u.monoOp.op);
}
void if_gotoPrint(InterCode* code){
    fprintf(outputFile,"IF ");
    printOperand(code->u.if_goto.x);
    fprintf(outputFile," %s ",code->u.if_goto.relop);
    printOperand(code->u.if_goto.y);
    fprintf(outputFile," GOTO ");
    printOperand(code->u.if_goto.z);
}
void returnPrint(InterCode* code){
    fprintf(outputFile,"RETURN ");
    printOperand(code->u.monoOp.op);
}
void decPrint(InterCode* code){
    fprintf(outputFile,"DEC ");
    printOperand(code->u.dec.op);
    fprintf(outputFile," %d",code->u.dec.size);
}

void argPrint(InterCode* code){
    fprintf(outputFile,"ARG ");
    printOperand(code->u.monoOp.op);
}
void callPrint(InterCode* code){
    printOperand(code->u.assign.left);
    fprintf(outputFile," := CALL ");
    printOperand(code->u.assign.right);
}
void paramPrint(InterCode* code){
    fprintf(outputFile,"PARAM ");
    printOperand(code->u.monoOp.op);
}
void readPrint(InterCode* code){
    fprintf(outputFile,"READ ");
    printOperand(code->u.monoOp.op);
}
void writePrint(InterCode* code){
    fprintf(outputFile,"WRITE ");
    printOperand(code->u.monoOp.op);
}