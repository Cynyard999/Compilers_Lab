#include <iostream>
#include <stdio.h>
extern FILE* yyin;
extern "C" int yylex();
extern "C" int yyparse();
extern "C" int lexicalErrorFlag;
extern "C" int syntaxErrorFlag;
extern "C" int semanticErrorFlag;
extern "C" {
    #include "syntaxTree.h"
    #include "semantic.h"
    #include "interCode.h"
}
using namespace std;

int main(int argc, char **argv){
    if(argc>1){
        if (!(yyin = fopen(argv[1],"r"))){
            perror(argv[1]);
            return 1;
        }
    }
    yyparse();
    if (syntaxErrorFlag == 0&&lexicalErrorFlag == 0)
    {
        //print_tree();
        analyze_tree();
        if (semanticErrorFlag == 0)
        {
            generateCode(argv[2]);
        }
    }
    return 0;
}