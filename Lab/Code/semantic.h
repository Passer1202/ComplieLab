#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "table.h"



void walkTree(struct node* root); // walk the syntax tree

void ExtDef(struct node* root); // process ExtDef node

Type FunDec(struct node* root,Type spec); // process FunDec node
void CompSt(struct node* root); // process CompSt node
FieldList ExtDecList(struct node* root,Type spec); // process ExtDecList node
FieldList VarList(struct node* root,Type type); // process VarList node
FieldList VarDec(struct node* root,Type spec); // process VarDec node
Type Specifier(struct node* root); // process Specifier node
Type StructSpecifier(struct node* root); // process StructSpecifier node
FieldList DefList(struct node* root); // process DefList node
FieldList Def(struct node* root);
FieldList ParamDec(struct node* root);
FieldList DecList(struct node* root,Type type);
FieldList Dec(struct node* root,Type type);
Type Exp(struct node* root); // process Exp node
void StmtList(struct node* root); // process StmtList node
void Stmt(struct node* root); // process Stmt node
FieldList Args(struct node* root,Type type); // process Args node


#endif // SEMANTIC_H