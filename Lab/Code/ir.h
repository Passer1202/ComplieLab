#ifndef IR_H

#define IR_H

#include "table.h"



typedef struct Operand_ * Operand;//操作数
struct Operand_{
    enum {
        OP_VARIABLE,    //v1,v2...
        OP_TEMP,        //t1,t2...
        OP_CONSTANT,
        OP_LABEL,
        OP_FUNCTION,
    } kind;
    union {
        char name[MAX_NAME_LEN];
        int val;
        int no;
    } u;
    int isAddr; //是否是地址 0:不是地址 1:是地址
};

struct InterCode_{
    enum{
        //单目
        IR_LABEL,
        IR_FUNCTION,
        IR_GOTO,
        IR_RETURN,
        IR_ARG,
        IR_PARAM,
        IR_READ,
        IR_WRITE,
        //双目
        IR_ASSIGN,
        IR_DEC,
        IR_CALL,
        IR_GETADDR,
        IR_READADDR,
        IR_WRITEADDR,
        //三目
        IR_ADD,
        IR_SUB,
        IR_MUL,
        IR_DIV,
        //四目
        IR_IF,

    } kind;
    union{
        struct{
            Operand op;
        } one;
        struct{
            Operand left;
            Operand right;
        } two;
        struct{
            Operand result;
            Operand op1;
            Operand op2;
        } three;
        struct{
            Operand op1;
            char *relop;
            Operand op2;
            Operand label;
        } four;
    } u;
};

struct OPList_{
    Operand op;
    struct OPList_ *next;
};
typedef struct OPList_ * OPList;

typedef struct InterCodes_ * InterCodes;

struct InterCodes_ {
    struct InterCode_ code;
    struct InterCodes_ *prev;
    struct InterCodes_ *next;
};

InterCodes codes_head; //中间代码链表头指针
InterCodes codes_tail; //中间代码链表尾指针



//int var_cnt;//v1,v2... //
int tmp_cnt;//t1,t2...
int label_cnt;//L1,L2...

void initIR();//一些准备工作
void freeIR();//释放中间代码链表
void makeIR(struct node* root,FILE* fp);
void addCode(int kind,...);//创建中间代码并添加到链表尾部
void printOP(FILE * fp,Operand op);//打印操作数
void printCode(FILE* fp);
Operand addr2var(Operand op);
Operand const2var(Operand op);
int  getSize(FieldList field);

//具体语法单元的翻译函数
void tProgram(struct node* root);
void tExtDef(struct node* root);
void tFunDec(struct node* root);
void tCompSt(struct node* root);
void tDefList(struct node* root);
void tDef(struct node* root);
void tDecList(struct node* root);
void tDec(struct node* root);
Operand tVarDec(struct node* root);
void tStmtList(struct node* root);
void tStmt(struct node* root);
Operand tExp(struct node* root);
void tCondExp(struct node* root,Operand label_true,Operand label_false);
OPList tArgs(struct node* root);

#endif // IR_H