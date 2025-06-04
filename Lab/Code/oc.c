#include "oc.h"
//先实现一个朴素寄存器分配算法，暂时不管函数调用（5.25）
//先不区分临时变量和变量5.28

//寄存器别名
//1.指令选择
//2.寄存器分配
//3.栈管理

/*
另外，实验四对作为输入的C−−源代码有如下的假设：
1) 假设1：输入文件中不包含任何词法、语法或语义错误（函数也必有return语句）。
2) 假设2：不会出现注释、八进制或十六进制整型常数、浮点型常数或者变量。
3) 假设3：整型常数都在16bits位的整数范围内，也就是说你不必考虑如果某个整型常数
无法在addi等包含立即数的指令中表示时该怎么办。
4) 假设4：不会出现类型为结构体或高维数组（高于1维的数组）的变量。
5) 假设5：没有全局变量的使用，并且所有变量均不重名，变量的存储空间都放到该变量
所在的函数的活动记录中。
6) 假设6：任何函数参数都只能是简单变量，也就是说数组和结构体不会作为参数传入某
个函数中。
7) 假设7：函数不会返回结构体或数组类型的值。
8) 假设8：函数只会进行一次定义（没有函数声明）。
*/

const char* reg_name[32]={"$zero","$at","$v0","$v1", 
                    "$a0","$a1","$a2","$a3",         
                    "$t0","$t1","$t2","$t3",        
                    "$t4","$t5","$t6","$t7",
                    "$s0","$s1","$s2","$s3",
                    "$s4","$s5","$s6","$s7",
                    "$t8","$t9",
                    "$k0","$k1",
                    "$gp","$sp",
                    "$fp","$ra"};

//初始化寄存器分配
void initReg(){
    //初始化寄存器
    for(int i=0;i<32;i++){
        regs[i].flag=0;
    }
}



//初始化代码段
//.data
//_prompt: .asciiz "Enter an integer:"
//_ret: .asciiz "\n"
//.globl main
void initData(FILE *fp){
    fprintf(fp,".data\n");
	fprintf(fp, "_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n");
}
//预先定义read和write函数
//.text
//read:
// li $v0, 4
// la $a0, _prompt
// syscall
// li $v0, 5
// syscall
// jr $ra
//
//write:
// li $v0, 1
// syscall
// li $v0, 4
// la $a0, _ret
// syscall
// move $v0, $0
// jr $ra
void initText(FILE *fp){

    
    fprintf(fp,".text\n");
    fprintf(fp,"\nread:\n");
    fprintf(fp," li $v0, 4\n");
    fprintf(fp," la $a0, _prompt\n");
    fprintf(fp," syscall\n");
    fprintf(fp," li $v0, 5\n");
    fprintf(fp," syscall\n");
    fprintf(fp," jr $ra\n");

    fprintf(fp,"\nwrite:\n");
    fprintf(fp," li $v0, 1\n");
    fprintf(fp," syscall\n");
    fprintf(fp," li $v0, 4\n");
    fprintf(fp," la $a0, _ret\n");
    fprintf(fp," syscall\n");
    fprintf(fp," move $v0, $0\n");
    fprintf(fp," jr $ra\n");

    stack_guard=(struct Stack_Item_*)malloc(sizeof(struct Stack_Item_));
    stack_guard->next=NULL;
    //stack_fp=(struct Stack_Item_*)malloc(sizeof(struct Stack_Item_));
    stack_fp=stack_guard;//有待商榷
    stack_fp->offset=0; //栈底指针的偏移量为0
    stack_sp=stack_fp;
}

int pushOP(FILE* fp,Operand op){
    //将操作数op压入栈中
    //分配一个新的栈帧
    struct Stack_Item_ *item=(struct Stack_Item_*)malloc(sizeof(struct Stack_Item_));
    item->op=op;
    assert(item->op!=NULL);
    item->offset=stack_sp->offset-4; //相对于栈底的偏移量
    item->next=stack_sp;
    stack_sp=item;
    fprintf(fp," addi $sp, $sp, -4\n"); //栈顶指针下移
    return item->offset;
}

void popOP(){
    //清空栈
    struct Stack_Item_ *p = stack_sp;
    while(p != stack_guard){
        struct Stack_Item_ *tmp = p;
        p = p->next;
        free(tmp);
    }
    stack_sp = stack_guard; //将栈顶指针指向栈保护指针
    stack_fp = stack_guard; //将栈底指针指向栈保护指针
}

//分配寄存器
//5.28 朴素寄存器分配算法
//只使用t0-t2寄存器
int getReg(FILE *fp,Operand op){
    //我们先不实现这一步，先打印reg(x)的形式
    if(op->kind==OP_VARIABLE){
        fprintf(fp,"reg(%s)",op->u.name);
    }
    else if(op->kind==OP_TEMP){
        fprintf(fp,"reg(t%d)",op->u.no);
    }
    else if(op->kind==OP_CONSTANT){
        fprintf(fp,"reg(#%d)",op->u.val);
    }
    else {
        assert(0 && "Unknown operand kind in getReg");
    }
}

void loadReg(FILE *fp,int offset ,int reg_no){
    //将寄存器reg_no的值从栈中加载到寄存器中
    //offset是相对于栈底也就是$fp的偏移量
    fprintf(fp," lw %s, %d($fp)\n", reg_name[reg_no], offset);
}

void storeReg(FILE *fp,int offset ,int reg_no){
    //将寄存器reg_no的值存储到栈中
    //offset是相对于栈底也就是$fp的偏移量
    fprintf(fp," sw %s, %d($fp)\n", reg_name[reg_no], offset);
}

int getOffset(Operand op){
    int res=-1;
    
    struct Stack_Item_ *p = stack_sp;
    while(p != stack_guard){
        
        assert(p->op!=NULL);

        if(p->op->kind == op->kind ){
            assert(p->op!=NULL);
            if(p->op->kind == OP_VARIABLE){
                assert(p->op->u.name!=NULL);
                if(strcmp(p->op->u.name, op->u.name) == 0){
                    res = p->offset;
                    break;
                }
            }
            else if(p->op->kind == OP_TEMP){
                assert(p->op->u.no >= 0);
                if(p->op->u.no == op->u.no){
                    
                    res = p->offset;
                    break;
                }
            }
            else {
                assert(0);
            }
        }
        p = p->next;
        
        
    }
    //assert(0);
    
    return res;
}

int isVar(Operand op){
    //判断操作数是否是变量
    return (op->kind == OP_VARIABLE || op->kind == OP_TEMP);
}


void printOC(FILE *fp,InterCodes p){
    //根据表11,将中间代码一一转换为目标代码,待实现寄存器分配
    //5.28: 考虑动态维护函数栈，遇到一个新的变量就放进去，
    int offset1, offset2, offset3;
    int offset;
    switch(p->code.kind){
        case IR_LABEL:
            //LABEL x: -> x:
            fprintf(fp,"label%d:\n",p->code.u.one.op->u.no);
            break;
        case IR_FUNCTION:
            //TODO
            para_cnt=0;
            popOP();
            fprintf(fp,"\n%s:\n",p->code.u.one.op->u.name);
            fprintf(fp," move $fp, $sp\n"); //将栈顶指针$sp赋值给栈底指针$fp
            break;
        case IR_GOTO:
            //TODO
            //GOTO x -> j x
            fprintf(fp," j label%d\n",p->code.u.one.op->u.no);
            break;
        case IR_RETURN:
            //TODO
            //RETURN x -> move $v0, reg(x)
            //            jr $ra
            if(p->code.u.one.op->kind == OP_CONSTANT){
                //如果是常量，直接将其值放到$v0中
                fprintf(fp," li $v0, %d\n", p->code.u.one.op->u.val);
                fprintf(fp," jr $ra\n");
                break;
            }
            
            offset = getOffset(p->code.u.one.op);
            if(offset==-1){
                //没有找到偏移量
                offset=pushOP(fp,p->code.u.one.op);
            }

            //从栈中加载寄存器
            loadReg(fp, offset, 8); //t0
            fprintf(fp," move $v0, %s",reg_name[8]);//t0
            //getReg(fp,p->code.u.one.op);
            fprintf(fp,"\n");
            fprintf(fp," jr $ra\n");
            break;
        case IR_ARG:
            //TODO
            //突然意识到，我们可以不用寄存器来传参数嘛，懒死了
            //直接将参数压入栈中
            if(p->code.u.one.op->kind == OP_CONSTANT){
                //如果是常量，直接将其值放到栈中
                pushOP(fp,p->code.u.one.op);
                fprintf(fp," li $t0, %d\n", p->code.u.one.op->u.val); //t0
                fprintf(fp," sw $t0, 0($sp)\n"); //将$t0的值存储到栈中
            }
            else {
                offset = getOffset(p->code.u.one.op);
                if(offset==-1){
                    //没有找到偏移量
                    offset=pushOP(fp,p->code.u.one.op);
                }
                else{
                    pushOP(fp,p->code.u.one.op);
                }
                loadReg(fp, offset, 8); //t0
                fprintf(fp," sw %s, 0($sp)\n", reg_name[8]); //将$t0的值存储到栈中
            }
            break;
        case IR_PARAM:
            //TODO
            //将fp上面的参数赋值回来
            offset=pushOP(fp,p->code.u.one.op);
            loadReg(fp, 12+4*para_cnt, 8); //t0
            fprintf(fp," sw %s, %d($fp)\n", reg_name[8], offset); //将$t0的值存储到栈中
            para_cnt++;
            break;
        case IR_READ:
            //TODO
            offset = getOffset(p->code.u.one.op);
            if(offset==-1){
                //没有找到偏移量
                offset=pushOP(fp,p->code.u.one.op);
            }

            fprintf(fp," addi $sp, $sp, -4\n"); //栈顶指针下移
            fprintf(fp," sw $ra, 0($sp)\n"); //将$ra寄存器的值存储到栈中
            fprintf(fp," jal read\n"); //调用write函数
            fprintf(fp," lw $ra, 0($sp)\n"); //从栈中加载$ra寄存器的值
            fprintf(fp," addi $sp, $sp, 4\n"); //栈顶指针上移
            //返回值在$v0中
            //将$v0的值存储到内存
            fprintf(fp," sw $v0, %d($fp)\n", offset); //将$v0的值存储到栈中
            break;
        case IR_WRITE:
            //TODO
            //WRITE x -> move $a0, reg(x)
            if(p->code.u.one.op->kind == OP_CONSTANT){
                //如果是常量，直接将其值放到$a0中
                fprintf(fp," li $a0, %d\n", p->code.u.one.op->u.val);
            }
            else {
                offset = getOffset(p->code.u.one.op);
                if(offset==-1){
                    //没有找到偏移量
                    offset=pushOP(fp,p->code.u.one.op);
                }
                loadReg(fp, offset, 4); //a0
            }
            //存ra寄存器
            fprintf(fp," addi $sp, $sp, -4\n"); //栈顶指针下移
            fprintf(fp," sw $ra, 0($sp)\n"); //将$ra寄存器的值存储到栈中
            fprintf(fp," jal write\n"); //调用write函数
            fprintf(fp," lw $ra, 0($sp)\n"); //从栈中加载$ra寄存器的值
            fprintf(fp," addi $sp, $sp, 4\n"); //栈顶指针上移
            break;
        case IR_ASSIGN:
            if(p->code.u.two.right->kind == OP_CONSTANT){
                //x:=#k
                //li reg(x), k
                //t0,1,2,3;
                offset=getOffset(p->code.u.two.left);
                if(offset ==-1){
                    offset = pushOP(fp,p->code.u.two.left);
                }
                //loadReg(fp, offset, 8); //t0
                fprintf(fp," li %s, %d\n",reg_name[8],p->code.u.two.right->u.val);//t0
                storeReg(fp, offset, 8); //将寄存器的值存储到栈中
                //getReg(fp,p->code.u.two.left);

            }
            else if(p->code.u.two.right->kind == OP_VARIABLE || p->code.u.two.right->kind == OP_TEMP){
                //x:=y
                //move reg(x), reg(y)
                offset1=getOffset(p->code.u.two.left);
                if(offset1 ==-1){
                    offset1 = pushOP(fp,p->code.u.two.left);
                }
                offset2=getOffset(p->code.u.two.right);
                if(offset2 ==-1){
                    offset2 = pushOP(fp,p->code.u.two.right);
                }
                loadReg(fp, offset2, 8); //t0
                storeReg(fp, offset1, 8); //将寄存器的值存储到栈中
                //getReg(fp,p->code.u.two.left);
                //fprintf(fp,", ");
                //getReg(fp,p->code.u.two.right);
                //fprintf(fp,"\n");
            }
            else {
                fprintf(stderr,"Error: Unknown operand kind in IR_ASSIGN\n");
            }     
            break;
        case IR_DEC:
            //TODO
            //分配空间
            //DEC x size
            offset=p->code.u.two.right->u.val;
            offset/=4; //转换为字的单位
            while(offset--){
                pushOP(fp,p->code.u.two.left);
            }
            break;
        case IR_CALL:
            //TODO
            //x := CALL f -> jal f
            //            move reg(x), $v0
            //保存$ra寄存器的值到栈中
            fprintf(fp," addi $sp, $sp, -12\n"); //栈顶指针下移
            fprintf(fp," sw $ra, 0($sp)\n"); //将$ra寄存器的值存储到栈中
            fprintf(fp," sw $fp, 4($sp)\n"); //将$fp寄存器的值存储到栈中
            fprintf(fp," sw $sp, 8($sp)\n");//将$sp寄存器的值保存到栈中

            fprintf(fp," jal %s\n",p->code.u.two.right->u.name);
            fprintf(fp," lw $sp, 8($fp)\n"); //从栈中加载$sp寄存器的值
            fprintf(fp," lw $fp, 4($sp)\n"); //从栈中加载$fp寄存器的值
            fprintf(fp," lw $ra, 0($sp)\n"); //从栈中加载$ra寄存器的值
            fprintf(fp," addi $sp, $sp, 12\n"); //栈顶指针上移

            if(isVar(p->code.u.two.left)){
                
                offset=getOffset(p->code.u.two.left);
                if(offset == -1){
                    offset = pushOP(fp,p->code.u.two.left);
                }
                /*
                fprintf(fp," move ");
                getReg(fp,p->code.u.two.left);
                fprintf(fp,", $v0\n");
                */
                //将$v0的值存储到栈中
                fprintf(fp," sw $v0, %d($fp)\n", offset); //将$v0的值存储到栈中
            }
            else {
                fprintf(stderr,"Error: Unknown operand kind in IR_CALL\n");
            }

            break;
        case IR_GETADDR:
            //TODO
            //x := &y -> la reg(x), y
            offset1=getOffset(p->code.u.two.left);
            if(offset1 ==-1){
                offset1 = pushOP(fp,p->code.u.two.left);
            }
            offset2=getOffset(p->code.u.two.right);
            if(offset2 ==-1){
                offset2 = pushOP(fp,p->code.u.two.right);
            }
            //计算$fp+offset的值，赋给x
            fprintf(fp," addi $t0, $fp, %d\n", offset2); //t0 = $fp + offset2
            fprintf(fp," sw $t0, %d($fp)\n", offset1); //将$t0的值存储到栈中
            break;
        case IR_READADDR:
            //TODO
            //x := *y -> lw reg(x), 0(reg(y))
            offset1=getOffset(p->code.u.two.left);
            if(offset1 ==-1){
                offset1 = pushOP(fp,p->code.u.two.left);
            }
            offset2=getOffset(p->code.u.two.right);
            if(offset2 ==-1){
                offset2 = pushOP(fp,p->code.u.two.right);
            }

            loadReg(fp, offset2, 8); //t0
            fprintf(fp," lw %s, 0(%s)\n", reg_name[8], reg_name[8]); //t0
            storeReg(fp, offset1, 8); //将寄存器的值存储到栈中
            break;
        case IR_WRITEADDR:
            //TODO
            //*x := y -> sw reg(y), 0(reg(x))
            //y是常数
            offset1=getOffset(p->code.u.two.left);
            if(offset1 ==-1){
                offset1 = pushOP(fp,p->code.u.two.left);
            }
            loadReg(fp, offset1, 8); //t0

            if(p->code.u.two.right->kind == OP_CONSTANT){
                //如果是常量，直接将其值放到$t0中
                fprintf(fp," li $t1, %d\n", p->code.u.two.right->u.val); //t0
                
            }
            else{
                offset2=getOffset(p->code.u.two.right);
                if(offset2 ==-1){
                    offset2 = pushOP(fp,p->code.u.two.right);
                }
                loadReg(fp, offset2, 9); //t1
            }
            //将t1的值存储到t0指向的地址中
            fprintf(fp," sw %s, 0(%s)\n", reg_name[9], reg_name[8]); //t1, t0
            break;
        case IR_ADD:
            //TODO
            if(isVar(p->code.u.three.op1) && isVar(p->code.u.three.op2)){
                //操作数都是（临时）变量
                //x := y + z -> add reg(x), reg(y), reg(z)
                offset1=getOffset(p->code.u.three.result);
                if(offset1 ==-1){
                    offset1 = pushOP(fp,p->code.u.three.result);
                }
                offset2=getOffset(p->code.u.three.op1);
                if(offset2 ==-1){
                    offset2 = pushOP(fp,p->code.u.three.op1);
                }
                offset3=getOffset(p->code.u.three.op2);
                if(offset3 ==-1){
                    offset3 = pushOP(fp,p->code.u.three.op2);
                }
                loadReg(fp, offset2, 8); //t0
                loadReg(fp, offset3, 9); //t1
                //将t0和t1相加，结果存储到t0
                fprintf(fp," add %s, %s, %s\n", reg_name[8], reg_name[8], reg_name[9]); //t0 = t0 + t1
                storeReg(fp, offset1, 8); //将寄存器的值存储到栈中
                /*
                fprintf(fp," add ");
                getReg(fp,p->code.u.three.result);
                fprintf(fp,", ");
                getReg(fp,p->code.u.three.op1);
                fprintf(fp,", ");
                getReg(fp,p->code.u.three.op2);
                fprintf(fp,"\n");
                */
            }
            else if(p->code.u.three.op1->kind == OP_CONSTANT && p->code.u.three.op2->kind == OP_CONSTANT){
                //操作数都是常量
                //x := #k1 + #k2 -> li reg(x), k1 + k2
                offset=getOffset(p->code.u.three.result);
                if(offset ==-1){
                    offset = pushOP(fp,p->code.u.three.result);
                }
                fprintf(fp," li %s, %d\n", reg_name[8], p->code.u.three.op1->u.val + p->code.u.three.op2->u.val); //t0
                storeReg(fp, offset, 8); //将寄存器的值存储到栈中
                /*
                fprintf(fp," li ");
                getReg(fp,p->code.u.three.result);
                fprintf(fp,", %d\n",p->code.u.three.op1->u.val + p->code.u.three.op2->u.val);
                */
            }
            else {
                //操作数有变量有常量
                //x := y + #k -> addi reg(x), reg(y), k
                if(p->code.u.three.op1->kind == OP_CONSTANT){
                    //交换操作数
                    Operand tmp;
                    tmp = p->code.u.three.op1;
                    p->code.u.three.op1 = p->code.u.three.op2;
                    p->code.u.three.op2 = tmp;
                }
                assert(p->code.u.three.op1->kind == OP_VARIABLE || p->code.u.three.op1->kind == OP_TEMP);
                /*
                fprintf(fp," addi ");
                getReg(fp,p->code.u.three.result);
                fprintf(fp,", ");
                getReg(fp,p->code.u.three.op1);
                fprintf(fp,", %d\n",p->code.u.three.op2->u.val);
                */
                offset1=getOffset(p->code.u.three.result);
                if(offset1 ==-1){
                    offset1 = pushOP(fp,p->code.u.three.result);
                }
                offset2=getOffset(p->code.u.three.op1);
                if(offset2 ==-1){
                    offset2 = pushOP(fp,p->code.u.three.op1);
                }
                loadReg(fp, offset2, 8); //t0
                fprintf(fp," addi %s, %s, %d\n", reg_name[8], reg_name[8], p->code.u.three.op2->u.val); //t0 = t0 + k
                storeReg(fp, offset1, 8); //将寄存器的值存储到栈中
            }
            break;
        case IR_SUB:
            //TODO
            if(isVar(p->code.u.three.op1) && isVar(p->code.u.three.op2)){
                //操作数都是（临时）变量
                //x := y – z -> sub reg(x), reg(y), reg(z)
                /*
                fprintf(fp," sub ");
                getReg(fp,p->code.u.three.result);
                fprintf(fp,", ");
                getReg(fp,p->code.u.three.op1);
                fprintf(fp,", ");
                getReg(fp,p->code.u.three.op2);
                fprintf(fp,"\n");
                */
                offset1=getOffset(p->code.u.three.result);
                if(offset1 ==-1){
                    offset1 = pushOP(fp,p->code.u.three.result);
                }
                offset2=getOffset(p->code.u.three.op1);
                if(offset2 ==-1){
                    offset2 = pushOP(fp,p->code.u.three.op1);
                }
                offset3=getOffset(p->code.u.three.op2);
                if(offset3 ==-1){
                    offset3 = pushOP(fp,p->code.u.three.op2);
                }
                loadReg(fp, offset2, 8); //t0
                loadReg(fp, offset3, 9); //t1
                //将t0和t1相减，结果存储到t0
                fprintf(fp," sub %s, %s, %s\n", reg_name[8], reg_name[8], reg_name[9]); //t0 = t0 - t1
                storeReg(fp, offset1, 8); //将寄存器的值存储到栈中
            }
            else if(p->code.u.three.op1->kind == OP_CONSTANT && p->code.u.three.op2->kind == OP_CONSTANT){
                //操作数都是常量
                /*
                fprintf(fp," li ");
                getReg(fp,p->code.u.three.result);
                fprintf(fp,", %d\n",p->code.u.three.op1->u.val - p->code.u.three.op2->u.val);
                */
                offset=getOffset(p->code.u.three.result);
                if(offset ==-1){
                    offset = pushOP(fp,p->code.u.three.result);
                }
                fprintf(fp," li %s, %d\n", reg_name[8], p->code.u.three.op1->u.val - p->code.u.three.op2->u.val); //t0
                storeReg(fp, offset, 8); //将寄存器的值存储到栈中
            }
            else {
                //操作数有变量有常量
                //x := y – #k -> addi reg(x), reg(y), -k
                if(p->code.u.three.op1->kind == OP_CONSTANT){
                    //交换操作数
                    Operand tmp;
                    tmp = p->code.u.three.op1;
                    p->code.u.three.op1 = p->code.u.three.op2;
                    p->code.u.three.op2 = tmp;
                }
                assert(p->code.u.three.op1->kind == OP_VARIABLE || p->code.u.three.op1->kind == OP_TEMP);
                /*
                fprintf(fp," addi ");
                getReg(fp,p->code.u.three.result);
                fprintf(fp,", ");
                getReg(fp,p->code.u.three.op1);
                fprintf(fp,", %d\n",-p->code.u.three.op2->u.val);
                */
                offset1=getOffset(p->code.u.three.result);
                if(offset1 ==-1){
                    offset1 = pushOP(fp,p->code.u.three.result);
                }
                offset2=getOffset(p->code.u.three.op1);
                if(offset2 ==-1){
                    offset2 = pushOP(fp,p->code.u.three.op1);
                }
                loadReg(fp, offset2, 8); //t0
                fprintf(fp," addi %s, %s, %d\n", reg_name[8], reg_name[8], -p->code.u.three.op2->u.val); //t0 = t0 - k
                storeReg(fp, offset1, 8); //将寄存器的值存储到栈中
            }
            break;
        case IR_MUL:
            //TODO
            if(p->code.u.three.op1->kind == OP_CONSTANT && p->code.u.three.op2->kind == OP_CONSTANT){
                //操作数都是常量
                /*
                fprintf(fp," li ");
                getReg(fp,p->code.u.three.result);
                fprintf(fp,", %d\n",p->code.u.three.op1->u.val * p->code.u.three.op2->u.val);
                */
                offset=getOffset(p->code.u.three.result);
                if(offset ==-1){
                    offset = pushOP(fp,p->code.u.three.result);
                }
                fprintf(fp," li %s, %d\n", reg_name[8], p->code.u.three.op1->u.val * p->code.u.three.op2->u.val); //t0
                storeReg(fp, offset, 8); //将寄存器的值存储到栈中
            }
            else {
                //操作数有变量
                //x := y * z -> mul reg(x), reg(y), reg(z)
                offset1=getOffset(p->code.u.three.result);
                if(offset1 ==-1){
                    offset1 = pushOP(fp,p->code.u.three.result);
                }
                if(p->code.u.three.op1->kind == OP_CONSTANT){
                    //直接放到寄存器中
                    fprintf(fp," li %s, %d\n", reg_name[8], p->code.u.three.op1->u.val); //t0
                }
                else {
                    offset2=getOffset(p->code.u.three.op1);
                    if(offset2 ==-1){
                        offset2 = pushOP(fp,p->code.u.three.op1);
                    }
                    loadReg(fp, offset2, 8); //t0
                }
                if(p->code.u.three.op2->kind == OP_CONSTANT){
                    //直接放到寄存器中
                    fprintf(fp," li %s, %d\n", reg_name[9], p->code.u.three.op2->u.val); //t1
                }
                else {
                    offset3=getOffset(p->code.u.three.op2);
                    if(offset3 ==-1){
                        offset3 = pushOP(fp,p->code.u.three.op2);
                    }
                    loadReg(fp, offset3, 9); //t1
                }
                //将t0和t1相乘，结果存储到t0
                fprintf(fp," mul %s, %s, %s\n", reg_name[8], reg_name[8], reg_name[9]); //t0 = t0 * t1
                storeReg(fp, offset1, 8); //将寄存器的值存储到栈中
                /*
                fprintf(fp," mul ");
                getReg(fp,p->code.u.three.result);
                fprintf(fp,", ");
                getReg(fp,p->code.u.three.op1);
                fprintf(fp,", ");
                getReg(fp,p->code.u.three.op2);
                fprintf(fp,"\n");
                */
            }
            break;
        case IR_DIV:
            //TODO
            if(p->code.u.three.op1->kind == OP_CONSTANT && p->code.u.three.op2->kind == OP_CONSTANT){
                //操作数都是常量
                /*
                fprintf(fp," li ");
                getReg(fp,p->code.u.three.result);
                fprintf(fp,", %d\n",p->code.u.three.op1->u.val / p->code.u.three.op2->u.val);
                */
                offset=getOffset(p->code.u.three.result);
                if(offset ==-1){
                    offset = pushOP(fp,p->code.u.three.result);
                }
                fprintf(fp," li %s, %d\n", reg_name[8], p->code.u.three.op1->u.val / p->code.u.three.op2->u.val); //t0
                storeReg(fp, offset, 8); //将寄存器的值存储到栈中

            }
            else {
                //操作数有变量
                //x := y / z
                //div reg(y), reg(z)
                //mflo reg(x)
                /*
                fprintf(fp," div ");
                getReg(fp,p->code.u.three.op1);
                fprintf(fp,", ");
                getReg(fp,p->code.u.three.op2);
                fprintf(fp,"\n");
                fprintf(fp," mflo ");
                getReg(fp,p->code.u.three.result);
                fprintf(fp,"\n");
                */
                offset1=getOffset(p->code.u.three.result);
                if(offset1 ==-1){
                    offset1 = pushOP(fp,p->code.u.three.result);
                }
                if(p->code.u.three.op1->kind == OP_CONSTANT){
                    //直接放到寄存器中
                    fprintf(fp," li %s, %d\n", reg_name[8], p->code.u.three.op1->u.val); //t0
                }
                else {
                    offset2=getOffset(p->code.u.three.op1);
                    if(offset2 ==-1){
                        offset2 = pushOP(fp,p->code.u.three.op1);
                    }
                    loadReg(fp, offset2, 8); //t0
                }
                if(p->code.u.three.op2->kind == OP_CONSTANT){
                    //直接放到寄存器中
                    fprintf(fp," li %s, %d\n", reg_name[9], p->code.u.three.op2->u.val); //t1
                }
                else {
                    offset3=getOffset(p->code.u.three.op2);
                    if(offset3 ==-1){
                        offset3 = pushOP(fp,p->code.u.three.op2);
                    }
                    loadReg(fp, offset3, 9); //t1
                }
                //将t0和t1相除，结果存储到t0
                fprintf(fp," div %s, %s\n", reg_name[8], reg_name[9]); //t0 = t0 / t1
                fprintf(fp," mflo %s\n", reg_name[8]); //将商存储到t0
                storeReg(fp, offset1, 8); //将寄存器的值存储到栈中
            }
            break;
        case IR_IF:
            //TODO
            //检查relop的值
            if(isVar(p->code.u.four.op1)){
                offset1=getOffset(p->code.u.four.op1);
                if(offset1 ==-1){
                    offset1 = pushOP(fp,p->code.u.four.op1);
                }
                loadReg(fp, offset1, 8); //t0
            }
            else{
                //如果是常量，直接将其值放到寄存器中
                fprintf(fp," li %s, %d\n", reg_name[8], p->code.u.four.op1->u.val); //t0
            }
            if(isVar(p->code.u.four.op2)){
                //如果是变量
                offset2=getOffset(p->code.u.four.op2);
                if(offset2 ==-1){
                    offset2 = pushOP(fp,p->code.u.four.op2);
                }
                loadReg(fp, offset2, 9); //t1
            }
            else {
                //如果是常量，直接将其值放到寄存器中
                fprintf(fp," li %s, %d\n", reg_name[9], p->code.u.four.op2->u.val); //t1
            }

            if(strcmp(p->code.u.four.relop,"==") == 0){
                //IF x == y GOTO z -> beq reg(x), reg(y), z
                fprintf(fp," beq ");
            }
            else if(strcmp(p->code.u.four.relop,"!=") == 0){
                //IF x != y GOTO z -> bne reg(x), reg(y), z
                fprintf(fp," bne ");
            }
            else if(strcmp(p->code.u.four.relop,">") == 0){
                //IF x > y GOTO z  -> bgt reg(x), reg(y), z
                fprintf(fp," bgt ");
            }
            else if(strcmp(p->code.u.four.relop,"<") == 0){
                //IF x < y GOTO z  -> blt reg(x), reg(y), z
                fprintf(fp," blt ");
            }
            else if(strcmp(p->code.u.four.relop,">=") == 0){
                //IF x >= y GOTO z -> bge reg(x), reg(y), z
                fprintf(fp," bge ");
            }
            else if(strcmp(p->code.u.four.relop,"<=") == 0){
                //IF x <= y GOTO z -> ble reg(x), reg(y), z
                fprintf(fp," ble ");
            }
            else {
                fprintf(stderr,"Error: Unknown relop %s\n",p->code.u.four.relop);
                return;
            }
            /*
            getReg(fp,p->code.u.four.op1);
            fprintf(fp,", ");
            getReg(fp,p->code.u.four.op2);
            fprintf(fp,", label%d\n",p->code.u.four.label->u.no);
            */
            fprintf(fp," %s, %s, label%d\n", reg_name[8], reg_name[9], p->code.u.four.label->u.no); //t0, t1, label
            break;
        default:
            fprintf(stderr,"Error: Unknown IR code kind %d\n",p->code.kind);
            break;
    }
}

void makeOC(FILE* fp){
    //根据中间代码链表生成目标代码并输出到文件

    assert(fp!=NULL);
    initReg();
    initData(fp);
    initText(fp);
    InterCodes p=codes_head->next;
    while(p!=codes_head){
        assert(p!=NULL);
        printOC(fp,p);
        p=p->next;
    }
}