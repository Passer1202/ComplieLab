#ifndef OC_H
#define OC_H

#include "table.h"
#include "ir.h"

extern const char* reg_name[32];

struct Register_{
    int flag;       //寄存器是否被使用 0:未使用 1:使用
    char var[16];   //存在里面的变量的值
}regs[32];

struct Stack_Item_{
    Operand op;     //操作数
    int offset;     //相对于栈底偏移
    struct Stack_Item_ *next; //下一个栈帧
};

struct Stack_Item_ *stack_fp; //栈底指针
struct Stack_Item_ *stack_sp; //栈顶指针
struct Stack_Item_ *stack_guard; //栈保护指针

void makeOC(FILE* fp);//生成目标代码到out1.s

int para_cnt;
#endif