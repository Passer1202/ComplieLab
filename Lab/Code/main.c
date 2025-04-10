#include "common.h"
#include "syntax.tab.h"
#include "semantic.h"

extern int yylineno;
extern int yyparse();
extern void yyrestart(FILE*);

int inStruct=0; // 0: not in struct, 1: in struct

//extern int yydebug;

struct node* root=NULL;

struct Stack_* stack=NULL;

int cur_depth=0;

struct SymbolTable_* table;//变量符号表
struct SymbolTable_* def_func;//函数定义表
struct SymbolTable_* dec_func;//函数声明表
struct SymbolTable_* def_struct;//结构体定义表

Type retType;

int lex_err=0;
int syn_err=0; 

int line_err=0;

int main(int argc,char *argv[])
{
    if(argc<=1){
        return 1;
    }
    FILE *file=fopen(argv[1],"r");
    if(file==NULL){
        perror(argv[1]);
        return 1;
    }
    
    yyrestart(file);
    //yydebug=1;
    yyparse();

    if(!lex_err&&!syn_err){
        //printf("Syntax analysis success!\n");
        //PrintTree(root,0);

        table=newTable();
        def_func=newTable();
        dec_func=newTable();
        def_struct=newTable();
        retType=(Type)malloc(sizeof(struct Type_));
        retType->kind=ERR;
        stack=newStack();
        walkTree(root);
        check_defed();
        deleteTable(table);
        deleteTable(def_func);
        deleteTable(dec_func);
        deleteTable(def_struct);
    }
    
    fclose(file);
    FreeTree(root);

    return 0;
}