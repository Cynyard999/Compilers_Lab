%{
   #include "syntax.tab.h"
   int errorFlag = 0;
%}
%option    yylineno
WS         [ \t\n\r]+
LETTER     [A-Za-z]
DIGIT      [0-9]
INTEGER    0|([1-9]({DIGIT}*))
HEX        0[xX]([0-9a-fA-F]+)
OCTAL      0([0-7]+)
FLOAT      ({DIGIT}+)\.({DIGIT}+)
FLOAT_E    ((\.{DIGIT}+)|({DIGIT}+\.{DIGIT}*))([eE][+-]?{DIGIT}+)
ID         ({LETTER}|_)({LETTER}|{DIGIT}|_)*
RELOP      ">"|"<"|">="|"<="|"=="|"!="
TYPE       "int"|"float"
SIN_COMMENT \/\/.*
%%
";"      {fprintf(stdout,"SEMI %s at Line %d.\n", yytext, yylineno);}
","      {fprintf(stdout,"COMMA %s at Line %d.\n", yytext, yylineno);}
"="      {fprintf(stdout,"ASSIGNOP %s at Line %d.\n", yytext, yylineno);}
{RELOP}  {fprintf(stdout,"RELOP %s at Line %d.\n", yytext, yylineno);}
"+"      {fprintf(stdout,"PLUS %s at Line %d.\n", yytext, yylineno);}
"-"      {fprintf(stdout,"MINUS %s at Line %d.\n", yytext, yylineno);}
"*"      {fprintf(stdout,"STAR %s at Line %d.\n", yytext, yylineno);}
"/"      {fprintf(stdout,"DIV %s at Line %d.\n", yytext, yylineno);}
"&&"     {fprintf(stdout,"AND %s at Line %d.\n", yytext, yylineno);}
"||"     {fprintf(stdout,"OR %s at Line %d.\n", yytext, yylineno);}
"."      {fprintf(stdout,"DOT %s at Line %d.\n", yytext, yylineno);}
"!"      {fprintf(stdout,"NOT %s at Line %d.\n", yytext, yylineno);}
"("      {fprintf(stdout,"LP %s at Line %d.\n", yytext, yylineno);}
")"      {fprintf(stdout,"RP %s at Line %d.\n", yytext, yylineno);}
"["      {fprintf(stdout,"LB %s at Line %d.\n", yytext, yylineno);}
"]"      {fprintf(stdout,"RB %s at Line %d.\n", yytext, yylineno);}
"{"      {fprintf(stdout,"LC %s at Line %d.\n", yytext, yylineno);}
"}"      {fprintf(stdout,"RC %s at Line %d.\n", yytext, yylineno);}
"struct" {fprintf(stdout,"STRUCT %s at Line %d.\n", yytext, yylineno);}
"return" {fprintf(stdout,"RETURN %s at Line %d.\n", yytext, yylineno);}
"if"     {fprintf(stdout,"IF %s at Line %d.\n", yytext, yylineno);}
"else"   {fprintf(stdout,"ELSE %s at Line %d.\n", yytext, yylineno);}
"while"  {fprintf(stdout,"WHILE %s at Line %d.\n", yytext, yylineno);}
{TYPE}   {fprintf(stdout,"TYPE %s at Line %d.\n", yytext, yylineno);}
{FLOAT}  {fprintf(stdout,"FLOAT %f at Line %d.\n", atof(yytext), yylineno);}
{FLOAT_E}  {fprintf(stdout,"FLOAT %f at Line %d.\n", atof(yytext), yylineno);}
{INTEGER} {fprintf(stdout,"INT %u at Line %d.\n", atoi(yytext), yylineno);}
{HEX}    {
        char * pEnd;
        fprintf(stdout,"INT %u at Line %d.\n", strtol(yytext,&pEnd,16), yylineno);
}
{OCTAL}    {
        char * pEnd;
        fprintf(stdout,"INT %u at Line %d.\n", strtol(yytext,&pEnd,8), yylineno);
}
{ID}     {fprintf(stdout,"ID %s at Line %d.\n", yytext, yylineno);}
{SIN_COMMENT}    {}
"/*"    {
            char c;
            while (1) {
                c = input();
                if (c == '*') {
                    c = input();
                    if (c == '/')
                        break;
                    else
                        unput(c);
                } 
                else if (c == EOF) {
                    fprintf(stderr,"Error type A at Line %d: Unclosed comment, expect */\n",yylineno);
                    break;
                }
            }
        }
{WS}    {}
.       {
        errorFlag = 1;
        fprintf(stderr,"Error type A at Line %d: Mysterious character \"%s\".\n",yylineno,yytext);
}
%%