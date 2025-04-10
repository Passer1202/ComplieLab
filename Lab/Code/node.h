#ifndef NODE_H
#define NODE_H
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_CHILD_NUM 8
#define MAX_NAME_LEN 32
#define MAX_VAL_LEN 32

typedef struct node
{
    char name[MAX_NAME_LEN];
    char val[MAX_VAL_LEN];
    int lineno;
    int child_num;//孩子节点个数
    struct node* child[MAX_CHILD_NUM];//孩子节点

}node;


static void FreeTree(struct node* root){
    if(root==NULL){
        return;
    }
    for(int i=0;i<root->child_num;i++){
        FreeTree(root->child[i]);
    }
    free(root);
    root=NULL;
}


static void AddChild(int child_num,struct node* parent,...){
    va_list valist;
    va_start(valist,parent);
    
    for(int i=0;i<child_num;i++){
        //判断是否为NULL
        struct node* child = va_arg(valist, struct node*);
        if(child==NULL){
            i--;
            child_num--;
            //printf("NULL\n");
            continue;
            
        }
        parent->child[i]=child;
    }
    parent->child_num=child_num;
    //printf("name:%s ",parent->name);
    //printf("child_num:%d\n",parent->child_num);
    va_end(valist);
}

/*
static void AddChild(int child_num,struct node* parent,...){
    va_list valist;
    va_start(valist,parent);
    int cnt=0;

    for(int i=0;i<child_num;i++){
        //查非NULL的个数
        struct node* tmp=va_arg(valist,struct node*);
        if(tmp!=NULL){
            cnt++;
        }
    }
    parent->child_num=child_num;
    va_end(valist);
}
*/
static struct node* NewNode(char* name,char* val,int lineno){
    struct node* p=(node*)malloc(sizeof(node));
    strcpy(p->name,name);
    strcpy(p->val,val);
    p->lineno=lineno;
    p->child_num=0;
    return p;
}

static void PrintTree(struct node* root,int level){
    if(root==NULL){
        return;
    }
    for(int i=0;i<level;i++){
        printf("  ");
    }
    printf("%s",root->name);
    if(root->child_num!=0){
        printf(" (%d)\n",root->lineno);
        for(int i=0;i<root->child_num;i++){
            PrintTree(root->child[i],level+1);
        }
    }
    else{
        //name为ID FLOAT INT时打印val
        if(strcmp(root->name,"ID")==0||strcmp(root->name,"FLOAT")==0||strcmp(root->name,"INT")==0||strcmp(root->name,"TYPE")==0){
            printf(": %s\n",root->val);
        }
        else{
            printf("\n");
        }
        
    }
}

#endif // NODE_H