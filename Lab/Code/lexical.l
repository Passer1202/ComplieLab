%{
    /*This part will be copyed to .c program...*/
    #include "common.h"
    #include "syntax.tab.h"

    int yycolumn = 1;
    extern int lex_err;
    extern int line_err;

    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
%}

%option yylineno

ws          [ \t]
digit       [0-9]
letter      [_a-zA-Z]
SEMI        ";"
COMMA       ","
ASSIGNOP    "="
RELOP       >|<|>=|<=|==|!=
PLUS        "+"
MINUS       "-"
STAR        "*"
DIV         "/"
AND         "&&"
OR          "||"
DOT         "."
NOT         "!"
TYPE        int|float
LP          "("
RP          ")"
LB          "["
RB          "]"
LC          "{"
RC          "}"
STRUCT      struct
RETURN      return
IF          if
ELSE        else
WHILE       while
dec         0|[1-9][0-9]*
oct         0[0-7]+
hex         0[xX][0-9a-fA-F]+
INT         {dec}|{oct}|{hex}
iee         {digit}+"."{digit}+
car         ({digit}*"."{digit}+)|({digit}+".")
fac         [eE][+-]?{digit}+
sci         {car}{fac}
FLOAT       {iee}|{sci}
ID          {letter}({letter}|{digit})*
COMMENT     "//".*|"/*"([^/]|"/*"|[^*]"/")*"*/"
IDERR       {digit}+{ID}
FLOATERR    "."{digit}+|{digit}+"."|{car}[eE]|{digit}+{fac}
OCTERR      0{digit}*[89]{digit}*
HEXERR      0[xX][0-9a-fA-F]*[G-Zg-z][0-9a-zA-Z]*

%%
{ws}        {/*no action and no return*/}
\r|\n       {yycolumn = 1;line_err=0;}
{SEMI}      {
    yylval.type_node = NewNode("SEMI",yytext,yylineno);
    return SEMI;
    }
{COMMA}     {
    yylval.type_node = NewNode("COMMA",yytext,yylineno);
    return COMMA;
    }
{ASSIGNOP}  {
    yylval.type_node = NewNode("ASSIGNOP",yytext,yylineno);
    return ASSIGNOP;
    }
{RELOP}     {
    yylval.type_node = NewNode("RELOP",yytext,yylineno);
    return RELOP;
    }
{PLUS}      {
    yylval.type_node = NewNode("PLUS",yytext,yylineno);
    return PLUS;
    }
{MINUS}     {
    yylval.type_node = NewNode("MINUS",yytext,yylineno);
    return MINUS;
    }
{STAR}      {
    yylval.type_node = NewNode("STAR",yytext,yylineno);
    return STAR;
    }
{DIV}       {
    yylval.type_node = NewNode("DIV",yytext,yylineno);
    return DIV;
    }
{AND}       {
    yylval.type_node = NewNode("AND",yytext,yylineno);
    return AND;
    }
{OR}        {
    yylval.type_node = NewNode("OR",yytext,yylineno);
    return OR;
    }
{DOT}       {
    yylval.type_node = NewNode("DOT",yytext,yylineno);
    return DOT;
    }
{NOT}       {
    yylval.type_node = NewNode("NOT",yytext,yylineno);
    return NOT;
    }
{TYPE}      {
    yylval.type_node = NewNode("TYPE",yytext,yylineno);
    return TYPE;
    }
{LP}        {
    yylval.type_node = NewNode("LP",yytext,yylineno);
    return LP;
    }
{RP}        {
    yylval.type_node = NewNode("RP",yytext,yylineno);
    return RP;
    }
{LB}        {
    yylval.type_node = NewNode("LB",yytext,yylineno);
    return LB;
    }
{RB}        {
    yylval.type_node = NewNode("RB",yytext,yylineno);
    return RB;
    }
{LC}        {
    yylval.type_node = NewNode("LC",yytext,yylineno);
    return LC;
    }
{RC}        {
    yylval.type_node = NewNode("RC",yytext,yylineno);
    return RC;
    }
{STRUCT}    {
    yylval.type_node = NewNode("STRUCT",yytext,yylineno);
    return STRUCT;
    }
{RETURN}    {
    yylval.type_node = NewNode("RETURN",yytext,yylineno);
    return RETURN;
    }
{IF}        {
    yylval.type_node = NewNode("IF",yytext,yylineno);
    return IF;
    }
{ELSE}      {
    yylval.type_node = NewNode("ELSE",yytext,yylineno);
    return ELSE;
    }
{WHILE}     {
    yylval.type_node = NewNode("WHILE",yytext,yylineno);
    return WHILE;
    }
{oct}       {
    unsigned int decimal_value = (unsigned int) strtoul(yytext, NULL, 8);
    char dec_val[32];
    snprintf(dec_val, 32,"%u",decimal_value);
    yylval.type_node = NewNode("INT",dec_val,yylineno);
    return INT;
    }
{hex}       {
    unsigned int decimal_value = (unsigned int) strtoul(yytext, NULL, 16);
    char dec_val[32];
    snprintf(dec_val, 32,"%u",decimal_value);
    yylval.type_node = NewNode("INT",dec_val,yylineno);
    return INT;
    }
{INT}       {
    yylval.type_node = NewNode("INT",yytext,yylineno);
    return INT;
    }
{ID}        {
    yylval.type_node = NewNode("ID",yytext,yylineno);
    return ID;
    }
{FLOAT}     {
    float float_value = strtod(yytext, NULL);
    char iee_val[32];
    snprintf(iee_val, 32, "%f", float_value);
    yylval.type_node = NewNode("FLOAT",iee_val,yylineno);
    return FLOAT;
    }
{COMMENT}   {/*no action and no return*/}
{FLOATERR}  {
    lex_err=1;
    if(!line_err)printf("Error type A at Line %d: Illegal float number \'%s\'.\n",yylineno, yytext);
    yylval.type_node = NewNode("FLOAT",yytext,yylineno);
    line_err=1;
    return FLOAT;
    }
{OCTERR}    {
    lex_err=1;
    if(!line_err)printf("Error type A at Line %d: Illegal octal number \'%s\'.\n",yylineno, yytext);
    yylval.type_node = NewNode("INT",yytext,yylineno);
    line_err=1;
    return INT;
    }
{HEXERR}    {
    lex_err=1;
    if(!line_err)printf("Error type A at Line %d: Illegal hexadecimal number \'%s\'.\n",yylineno, yytext);
    yylval.type_node = NewNode("INT",yytext,yylineno);
    line_err=1;
    return INT;
    }
{IDERR}     {
    lex_err=1;
    if(!line_err)printf("Error type A at Line %d: Illegal ID \'%s\'.\n",yylineno, yytext);
    yylval.type_node = NewNode("ID",yytext,yylineno);
    line_err=1;
    return ID;
    }
.           {
    lex_err=1;
    //printf("%d\n",line_err);
    if(!line_err)printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno, yytext);
    line_err=1;
    }

%%
