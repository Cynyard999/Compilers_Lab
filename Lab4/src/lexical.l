%{
    #include "syntaxTree.h"
    #define YYSTYPE Node*
    #include "syntax.tab.h"
    int lexicalErrorFlag = 0;
    int yycolumn = 1; 
    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \ 
    yylloc.first_column = yycolumn; \ 
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}
%option    yylineno
WS         [ \t\r]+
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
";"      {yylval = create_node(yylineno,"SEMI",SEMI,NULL); return SEMI;}
","      {yylval = create_node(yylineno,"COMMA",COMMA,NULL);return COMMA;}
"="      {yylval = create_node(yylineno,"ASSIGNOP",ASSIGNOP,NULL);return ASSIGNOP;}
{RELOP}  {yylval = create_node(yylineno,"RELOP",RELOP,yytext);return RELOP;}
"+"      {yylval = create_node(yylineno,"PLUS",PLUS,NULL);return PLUS;}
"-"      {yylval = create_node(yylineno,"MINUS",MINUS,NULL);return MINUS;}
"*"      {yylval = create_node(yylineno,"STAR",STAR,NULL);return STAR;}
"/"      {yylval = create_node(yylineno,"DIV",DIV,NULL);return DIV;}
"&&"     {yylval = create_node(yylineno,"AND",AND,NULL);return AND;}
"||"     {yylval = create_node(yylineno,"OR",OR,NULL);return OR;}
"."      {yylval = create_node(yylineno,"DOT",DOT,NULL);return DOT;}
"!"      {yylval = create_node(yylineno,"NOT",NOT,NULL);return NOT;}
"("      {yylval = create_node(yylineno,"LP",LP,NULL);return LP;}
")"      {yylval = create_node(yylineno,"RP",RP,NULL);return RP;}
"["      {yylval = create_node(yylineno,"LB",LB,NULL);return LB;}
"]"      {yylval = create_node(yylineno,"RB",RB,NULL);return RB;}
"{"      {yylval = create_node(yylineno,"LC",LC,NULL);return LC;}
"}"      {yylval = create_node(yylineno,"RC",RC,NULL);return RC;}
"struct" {yylval = create_node(yylineno,"STRUCT",STRUCT,NULL);return STRUCT;}
"return" {yylval = create_node(yylineno,"RETURN",RETURN,NULL);return RETURN;}
"if"     {yylval = create_node(yylineno,"IF",IF,NULL);return IF;}
"else"   {yylval = create_node(yylineno,"ELSE",ELSE,NULL);return ELSE;}
"while"  {yylval = create_node(yylineno,"WHILE",WHILE,NULL);return WHILE;}
{TYPE}   {yylval = create_node(yylineno,"TYPE",TYPE,yytext);return TYPE; }
{FLOAT}  {double f = atof(yytext); yylval = create_node(yylineno,"FLOAT",FLOAT, &f);return FLOAT;/*char buffer[64]; sprintf(buffer,"%f",atof(yytext));yylval = create_node(yylineno,"FLOAT",FLOAT, buffer);return FLOAT;*/}
{FLOAT_E}  {double f = atof(yytext); yylval = create_node(yylineno,"FLOAT",FLOAT, &f);return FLOAT;}
{INTEGER} {unsigned int i = atoi(yytext); yylval = create_node(yylineno,"INT",INT,&i);return INT;}
{HEX}    {
        char * pEnd;
        unsigned int val = strtol(yytext,&pEnd,16);
        yylval = create_node(yylineno,"INT",INT,&val);
        return INT;
}
{OCTAL}    {
        char * pEnd;
        unsigned int val = strtol(yytext,&pEnd,8);
        yylval = create_node(yylineno,"INT",INT,&val);
        return INT;
}
{ID}     {yylval = create_node(yylineno,"ID",ID,yytext);return ID;}
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
\n      { 
            // 回车单独判定，方便column更新
            yycolumn = 1; 
        }
.       {
        lexicalErrorFlag = 1;
        fprintf(stderr,"Error type A at Line %d: Mysterious character \"%s\".\n",yylineno,yytext);
}
%%