%{
    #include "lex.yy.c"
    #include "syntaxTree.h"
    void yyerror(const char* msg);
    void printError(int lineNo, int columnNo, int errorType, const char* msg);
    int lastWrongLine = -1;
    int syntaxErrorFlag = 0;
%}

/* 让Bison定义yylloc并且在头文件中声明yylloc */
%locations

/*  declared tokens */
%token IF ELSE WHILE STRUCT RETURN
%token RELOP ASSIGNOP
%token SEMI COMMA
%token PLUS MINUS STAR DIV AND OR NOT
%token DOT
%token LP RP LB RB LC RC
%token INT
%token FLOAT
%token TYPE ID


/* declared non-terminals */
%type Program ExtDefList ExtDef ExtDecList 
%type Specifier StructSpecifier OptTag Tag
%type VarDec FunDec VarList ParamDec
%type CompSt StmtList Stmt
%type DefList Def DecList Dec 
%type Exp Args

/*  accociativity & priority */
%nonassoc LOWER_THAN_ERROR
%nonassoc error
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right UMINUS NOT
%left LP RP LB RB DOT

%%

/*  High-level Definitions */
Program : 
    ExtDefList{
        if($1 == NULL){
            $$ = create_node(yylineno, "Program", 0, NULL);
        }
        else{
            $$ = create_node(@$.first_line, "Program", 0, NULL);
            add_child($$,$1);
        }
        root = $$;
    };


ExtDefList : 
    ExtDef ExtDefList{
        $$ = create_node(@$.first_line, "ExtDefList", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
    }
    | /* empty */  {
        $$ = NULL;
    }; 

ExtDef : 
    Specifier ExtDecList SEMI{
        $$ = create_node(@$.first_line, "ExtDef", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | Specifier SEMI{
        $$ = create_node(@$.first_line, "ExtDef", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
    }
    | Specifier FunDec CompSt{
        $$ = create_node(@$.first_line, "ExtDef", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | Specifier error SEMI{}
    | error SEMI{};

ExtDecList : 
    VarDec{
        $$ = create_node(@$.first_line, "ExtDecList", 0, NULL);
        add_child($$,$1);
    }
    | VarDec COMMA ExtDecList{
        $$ = create_node(@$.first_line, "ExtDecList", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    } ;

/*  Specifiers */
Specifier : 
    TYPE{
        $$ = create_node(@$.first_line, "Specifier", 0, NULL);
        add_child($$,$1);
    }
    | StructSpecifier{
        $$ = create_node(@$.first_line, "Specifier", 0, NULL);
        add_child($$,$1);
    } ;

StructSpecifier : 
    STRUCT OptTag LC DefList RC{
        $$ = create_node(@$.first_line, "StructSpecifier", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
        add_child($$,$4);
        add_child($$,$5);
    }
    | STRUCT OptTag LC error RC{}
    | STRUCT Tag{
        $$ = create_node(@$.first_line, "StructSpecifier", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
    };

OptTag : 
    ID{
        $$ = create_node(@$.first_line, "OptTag", 0, NULL);
        add_child($$,$1);
    }
    | /* empty */ {
        $$ = NULL;
    };

Tag : 
    ID{
        $$ = create_node(@$.first_line, "Tag", 0, NULL);
        add_child($$,$1);
    };

/*  Declarators */
VarDec : 
    ID{
        $$ = create_node(@$.first_line, "VarDec", 0, NULL);
        add_child($$,$1);
    }
    | VarDec LB INT RB{
        $$ = create_node(@$.first_line, "VarDec", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
        add_child($$,$4);
    }
    | VarDec LB error RB{}
    ;

FunDec : 
    ID LP VarList RP{
        $$ = create_node(@$.first_line, "FunDec", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
        add_child($$,$4);
    }
    | ID LP RP{
        $$ = create_node(@$.first_line, "FunDec", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | ID LP error RP{};


VarList : 
    ParamDec COMMA VarList{
        $$ = create_node(@$.first_line, "VarList", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | ParamDec{
        $$ = create_node(@$.first_line, "VarList", 0, NULL);
        add_child($$,$1);
    } ;

ParamDec : 
    Specifier VarDec {
        $$ = create_node(@$.first_line, "ParamDec", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
    };

/*  Statements */
CompSt : 
    LC DefList StmtList RC{
        $$ = create_node(@$.first_line, "CompSt", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
        add_child($$,$4);
    }
    | LC DefList error RC{};

StmtList : 
    Stmt StmtList{
        $$ = create_node(@$.first_line, "StmtList", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
    }
    | /* empty */{
        $$ = NULL;
    };

Stmt : 
    Exp SEMI{
        $$ = create_node(@$.first_line, "Stmt", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
    }
    | CompSt{
        $$ = create_node(@$.first_line, "Stmt", 0, NULL);
        add_child($$,$1);
    }
    | RETURN Exp SEMI{
        $$ = create_node(@$.first_line, "Stmt", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
        $$ = create_node(@$.first_line, "Stmt", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
        add_child($$,$4);
        add_child($$,$5);
    }
    | IF LP Exp RP Stmt ELSE Stmt{
        $$ = create_node(@$.first_line, "Stmt", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
        add_child($$,$4);
        add_child($$,$5);
        add_child($$,$6);
        add_child($$,$7);
    }
    | WHILE LP Exp RP Stmt{
        $$ = create_node(@$.first_line, "Stmt", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
        add_child($$,$4);
        add_child($$,$5);
    }
    | RETURN error SEMI{}
    | IF LP error RP Stmt{}
    | WHILE LP error RP Stmt{}
    | error SEMI{};

/*  Local Definitions */

DefList : 
    Def DefList{
        $$ = create_node(@$.first_line, "DefList", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
    } 
/* 可以选择将这个产生式的优先级设置为比error还要低 那么在遇到error的时候 就不会选择用这条产生式进行规约 而会选择将error移入 */
    | /* empty */ %prec LOWER_THAN_ERROR{
        $$ = NULL;
    };

Def : 
    Specifier DecList SEMI{
        $$ = create_node(@$.first_line, "Def", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | error SEMI{}
;

DecList : 
    Dec {
        $$ = create_node(@$.first_line, "DecList", 0, NULL);
        add_child($$,$1);
    }
    | Dec COMMA DecList{
        $$ = create_node(@$.first_line, "DecList", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    };

Dec : 
    VarDec{
        $$ = create_node(@$.first_line, "Dec", 0, NULL);
        add_child($$,$1);
    }
    | VarDec ASSIGNOP Exp{
        $$ = create_node(@$.first_line, "Dec", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    };

/*  Expressions */
Exp : 
    Exp ASSIGNOP Exp{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | Exp AND Exp{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);        
    }
    | Exp OR Exp{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);  
    }
    | Exp RELOP Exp{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | Exp PLUS Exp{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | Exp MINUS Exp{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | Exp STAR Exp{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | Exp DIV Exp{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | LP Exp RP{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | MINUS Exp %prec UMINUS{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
    }
    | NOT Exp{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
    }
    | ID LP Args RP{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
        add_child($$,$4);
    }
    | ID LP RP{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | Exp LB Exp RB{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
        add_child($$,$4);
    }
    | Exp DOT ID{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | ID{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
    }
    | INT{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
    }
    | FLOAT{
        $$ = create_node(@$.first_line, "Exp", 0, NULL);
        add_child($$,$1);
    }
    | Exp LB error RB{}
    | LP error RP{}
    | ID LP error RP{}
    ;

Args : 
    Exp COMMA Args{
        $$ = create_node(@$.first_line, "Args", 0, NULL);
        add_child($$,$1);
        add_child($$,$2);
        add_child($$,$3);
    }
    | Exp{
        $$ = create_node(@$.first_line, "Args", 0, NULL);
        add_child($$,$1);
    } ;


%%
void yyerror(const char* msg){
    if(yylineno != lastWrongLine){
        syntaxErrorFlag = 1;
        fprintf(stderr, "Error type B at Line %d: %s.\n", yylineno, msg);
        lastWrongLine = yylineno;
    }
}

void printError(int lineNo, int columnNo, int errorType, const char* msg){
    if(errorType == 0){
        fprintf(stderr, "Error type A at Line %d: %s at column %d.\n", lineNo, msg, columnNo);
    }
    else{
        fprintf(stderr, "Error type B at Line %d: %s at column %d.\n", lineNo, msg, columnNo);
    }
}





