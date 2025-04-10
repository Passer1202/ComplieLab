%{
    #include "lex.yy.c"
    #include "common.h"
    extern struct node* root;
    extern int syn_err;
    extern int line_err;
    //void syntax_error(char* msg,int lineno);
    //#define YYDEBUG 1
%}

%union{
    struct node* type_node;
}

%locations

//tokens

//terminals
%token <type_node> INT FLOAT
%token <type_node> ID TYPE
%token <type_node> SEMI COMMA 
%token <type_node> ASSIGNOP RELOP PLUS MINUS STAR DIV
%token <type_node> AND OR NOT
%token <type_node> LP RP LB RB LC RC
%token <type_node> STRUCT DOT
%token <type_node> RETURN WHILE
%token <type_node> IF ELSE

//non-terminals
%type <type_node> Program ExtDefList ExtDef ExtDecList
%type <type_node> Specifier StructSpecifier OptTag Tag
%type <type_node> VarDec FunDec VarList ParamDec
%type <type_node> CompSt StmtList Stmt 
%type <type_node> DefList Def DecList Dec 
%type <type_node> Exp Args

//precedence and associativity
%left   COMMA
%right  ASSIGNOP
%left   OR
%left   AND
%left   RELOP
%left   PLUS MINUS
%left   STAR DIV
%right  NOT
%left   LB RB LP RP DOT



%nonassoc LOWER_THAN_ELSE 
%nonassoc ELSE




%%
//开始改
//High-level Definitions

Program : ExtDefList           {$$=NewNode("Program","",@$.first_line);AddChild(1,$$,$1);root=$$;}
    ;
ExtDefList : ExtDef ExtDefList {$$=NewNode("ExtDefList","",@$.first_line);AddChild(2,$$,$1,$2);}
    |                          {$$=NULL;}
    ;
ExtDef : Specifier ExtDecList SEMI  {$$=NewNode("ExtDef","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Specifier SEMI                {$$=NewNode("ExtDef","",@$.first_line);AddChild(2,$$,$1,$2);}
    | Specifier FunDec CompSt       {$$=NewNode("ExtDef","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Specifier FunDec SEMI         {$$=NewNode("ExtDef","",@$.first_line);AddChild(3,$$,$1,$2,$3);}  
    | Specifier error ExtDef         {yyerrok;}
    //| Specifier error  CompSt       {}
    | Specifier ExtDecList error    {yyerrok;}
    | error SEMI                    {yyerrok;}
    ;
ExtDecList : VarDec             {$$=NewNode("ExtDecList","",@$.first_line);AddChild(1,$$,$1);}
    | VarDec COMMA ExtDecList   {$$=NewNode("ExtDecList","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | error  COMMA ExtDecList   {yyerrok;}
    ;

//Specifiers

Specifier : TYPE         {$$=NewNode("Specifier","",@$.first_line);AddChild(1,$$,$1);}
    | StructSpecifier    {$$=NewNode("Specifier","",@$.first_line);AddChild(1,$$,$1);}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC   {$$=NewNode("StructSpecifier","",@$.first_line);AddChild(5,$$,$1,$2,$3,$4,$5);}
    | STRUCT Tag                                {$$=NewNode("StructSpecifier","",@$.first_line);AddChild(2,$$,$1,$2);}
    //| error Tag                                 {}
    //| error OptTag LC DefList RC                {}
    //| error                                     {}
    ;
OptTag : ID {$$=NewNode("OptTag","",@$.first_line);AddChild(1,$$,$1);}
    |       {$$=NULL;}
    ;
Tag : ID    {$$=NewNode("Tag","",@$.first_line);AddChild(1,$$,$1);}

//Declarators

VarDec : ID             {$$=NewNode("VarDec","",@$.first_line);AddChild(1,$$,$1);}
    | VarDec LB INT RB  {$$=NewNode("VarDec","",@$.first_line);AddChild(4,$$,$1,$2,$3,$4);}
    | error RB          {yyerrok;}
    ;
FunDec : ID LP VarList RP   {$$=NewNode("FunDec","",@$.first_line);AddChild(4,$$,$1,$2,$3,$4);}
    | ID LP RP              {$$=NewNode("FunDec","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | error RP              {yyerrok;}
    | error LP VarList RP   {yyerrok;}
    | error LP RP           {yyerrok;}
    //| ID error VarList RP   {if(!line_err)syntax_error("Missing '('",@2.first_line);line_err=1;syn_err=1;}
    //| error LP RP           {if(!line_err)syntax_error("FunDec error",@1.first_line);line_err=1;syn_err=1;}
    ;
VarList : ParamDec COMMA VarList    {$$=NewNode("VarList","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | ParamDec                      {$$=NewNode("VarList","",@$.first_line);AddChild(1,$$,$1);}
    //| error COMMA VarList           {if(!line_err)syntax_error("Missing ParamDec",@1.first_line);line_err=1;syn_err=1;}
    ;
ParamDec : Specifier VarDec   {$$=NewNode("ParamDec","",@$.first_line);AddChild(2,$$,$1,$2);}
    ;

//Statements

CompSt : LC DefList StmtList RC  {$$=NewNode("CompSt","",@$.first_line);AddChild(4,$$,$1,$2,$3,$4);}
    | error RC                   {yyerrok;}
    //| LC error RC               {if(!line_err)syntax_error("CompSt error",@2.first_line);line_err=1;syn_err=1;}
    ;
StmtList : Stmt StmtList    {$$=NewNode("StmtList","",@$.first_line);AddChild(2,$$,$1,$2);}
    |                       {$$=NULL;}
    ;
Stmt : Exp SEMI                                 {$$=NewNode("Stmt","",@$.first_line);AddChild(2,$$,$1,$2);}
    | CompSt                                    {$$=NewNode("Stmt","",@$.first_line);AddChild(1,$$,$1);}
    | RETURN Exp SEMI                           {$$=NewNode("Stmt","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE   {$$=NewNode("Stmt","",@$.first_line);AddChild(5,$$,$1,$2,$3,$4,$5);}
    | IF LP Exp RP Stmt ELSE Stmt               {$$=NewNode("Stmt","",@$.first_line);AddChild(7,$$,$1,$2,$3,$4,$5,$6,$7);}
    | IF error RP Stmt %prec LOWER_THAN_ELSE    {yyerrok;}
    | IF error RP Stmt ELSE Stmt                {yyerrok;}
    | WHILE LP Exp RP Stmt                      {$$=NewNode("Stmt","",@$.first_line);AddChild(5,$$,$1,$2,$3,$4,$5);}
    | WHILE error RP  Stmt                      {yyerrok;}
    | error SEMI                                {yyerrok;}
    //| RETURN error                              {if(!line_err)syntax_error("return error",@2.first_line);line_err=1;syn_err=1;}
    //| RETURN error SEMI                         {if(!line_err)syntax_error("Miss Exp",@2.first_line);line_err=1;syn_err=1;}
    | RETURN Exp error                          {yyerrok;}
    | Exp error                                 {yyerrok;}
    ;

//Local Definitions

DefList : Def DefList           {$$=NewNode("DefList","",@$.first_line);AddChild(2,$$,$1,$2);}
    //| error SEMI DefList     {yyerrok;}
    |                           {$$=NULL;}
    ;
Def : Specifier DecList SEMI    {$$=NewNode("Def","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Specifier DecList error   {yyerrok;}
    //| ID DecList SEMI           {yyerrok;yyerror("Miss Type");}
    ;
DecList : Dec               {$$=NewNode("DecList","",@$.first_line);AddChild(1,$$,$1);}
    | Dec COMMA DecList     {$$=NewNode("DecList","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | error COMMA DecList   {}
    ;
Dec : VarDec                {$$=NewNode("Dec","",@$.first_line);AddChild(1,$$,$1);}
    | VarDec ASSIGNOP Exp   {$$=NewNode("Dec","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    ;

//Expressions

Exp : Exp ASSIGNOP Exp  {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Exp AND Exp       {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Exp OR Exp        {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Exp RELOP Exp     {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Exp PLUS Exp      {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Exp MINUS Exp     {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Exp STAR Exp      {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Exp DIV Exp       {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | LP Exp RP         {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | MINUS Exp         {$$=NewNode("Exp","",@$.first_line);AddChild(2,$$,$1,$2);}
    | NOT Exp           {$$=NewNode("Exp","",@$.first_line);AddChild(2,$$,$1,$2);}
    | ID LP Args RP     {$$=NewNode("Exp","",@$.first_line);AddChild(4,$$,$1,$2,$3,$4);}
    | ID LP RP          {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | Exp LB Exp RB     {$$=NewNode("Exp","",@$.first_line);AddChild(4,$$,$1,$2,$3,$4);}
    | Exp DOT ID        {$$=NewNode("Exp","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    | ID                {$$=NewNode("Exp","",@$.first_line);AddChild(1,$$,$1);}
    | INT               {$$=NewNode("Exp","",@$.first_line);AddChild(1,$$,$1);}
    | FLOAT             {$$=NewNode("Exp","",@$.first_line);AddChild(1,$$,$1);}
    //| Exp LB error RB   {if(!line_err)syntax_error("Exp error",@1.first_line);line_err=1;syn_err=1;}
    //| error RP          {}
    //| error STAR        {if(!line_err)syntax_error("Exp error",@1.first_line);line_err=1;syn_err=1;}
    //| error DIV         {if(!line_err)syntax_error("Exp error",@1.first_line);line_err=1;syn_err=1;}
    //| error RB          {if(!line_err)syntax_error("Exp error",@1.first_line);line_err=1;syn_err=1;}
    ;
Args : Exp COMMA Args   {$$=NewNode("Args","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
    //|  COMMA Args  {if(!line_err)syntax_error("Missing Exp",@1.first_line);line_err=1;syn_err=1;}
    | Exp               {$$=NewNode("Args","",@$.first_line);AddChild(1,$$,$1);}
    ;

%%

int yyerror(char* msg){
    //printf("%d\n",line_err);

    if(!line_err) printf("Error type B at line %d: %s.\n", yylineno, msg);
    line_err=1;
    syn_err=1;
}

//void syntax_error(char* msg,int lineno){
    //printf("Error type B at line %d: %s.\n",lineno,msg);
//}