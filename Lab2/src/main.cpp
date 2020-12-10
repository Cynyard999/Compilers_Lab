#include <iostream>
#include <stdio.h>
extern FILE* yyin;
extern "C" int yylex();
extern "C" int yyparse();
extern "C" int lexicalErrorFlag;
extern "C" int syntaxErrorFlag;
extern "C" {
    #include "syntaxTree.h"
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
        print_tree();
    }
    return 0;
}