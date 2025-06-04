#include "ir.h"
#include "semantic.h"
//中间代码优化！！！！！
//const2var貌似不必要，等上虚拟机试试水
//符号表去重
//DEBUG
//暂未完成一维数组的直接赋值（当前仅支持一维数组且基类型为基本类型）

//函数调用前准备的参数还没处理好(已解决)
//NOT 暂且认为一定是条件表达式，需添加int形 (已解决)
//嵌套结构体和多维数组中的地址我们可以另写一个函数（貌似已解决）

//没有高维数组的直接赋值，我们只需要记录好数组的基类型

//将所生成的中间代码先保存到内存中，等全部翻译完毕，再使用一个专门的打印函数把在内存中的中间代码打印出来。
//选择使用双向（循环）链表来存储中间代码，方便在中间代码生成的过程中进行插入和删除操作。
//暂不考虑进行优化...

/*
（1）假设 1：不会出现注释、八进制或十六进制整型常数、浮点型常数或者变量。
（2）假设 2：不会出现类型为结构体或高维数组（高于一维的数组）的变量。
（3）假设 3：任何函数参数都只能为简单变量，也就是说，结构体和数组都不会作为参数传入函数中。
    // 要求 4.1：修改前面对 C−−源代码的假设 2 和 3，使源代码中：
    //    1） 可以出现结构体类型的变量（但不会有结构体变量之间直接赋值）。
    //    2） 结构体类型的变量可以作为函数的参数（但函数不会返回结构体类型的值）。
    // 要求 4.2：修改前面对 C−−源代码的假设 2 和 3，使源代码中：
    //    1） 一维数组类型的变量可以作为函数参数（但函数不会返回一维数组类型的值）。
    //    2） 可以出现高维数组类型的变量（但高维数组类型的变量不会作为函数的参数或返回类值）。
（4）假设 4：没有全局变量的使用，并且所有变量均不重名。
（5）假设 5：函数不会返回结构体或数组类型的值。
（6）假设 6：函数只会进行一次定义（没有函数声明）。
（7）假设 7：输入文件中不包含任何词法、语法或语义错误（函数也必有 return 语句）。
*/



void initIR(){
    //初始化Codes链表 : guard
    
    codes_head=(InterCodes)malloc(sizeof(struct InterCodes_));
    codes_head->next=codes_head;
    codes_head->prev=codes_head;
    codes_tail=codes_head;

    //初始化变量计数器
    //v1,v2... t1,t2...
    //var_cnt=0;
    tmp_cnt=0;
    label_cnt=0;

}

void freeIR(){
    //释放中间代码链表
    InterCodes p=codes_head->next;
    while(p!=codes_head){
        InterCodes tmp=p;
        p=p->next;
        free(tmp);
    }
    free(codes_head);
}

Operand newOP(int kind,int isAddr,...){//kind isAddr name/value
    //创建一个新的操作数
    Operand op=(Operand)malloc(sizeof(struct Operand_));
    op->kind=kind;
    op->isAddr=isAddr;
    va_list args;
    va_start(args, isAddr);
    switch(kind){
        case OP_VARIABLE : //所有变量都不重名，直接用变量名字就好了...
            //op->u.no=var_cnt++;
            strcpy(op->u.name, va_arg(args, char*));
            break;
        case OP_TEMP : //t1,t2...
            op->u.no=tmp_cnt++;
            break;
        case OP_CONSTANT :
            op->u.val=va_arg(args, int);
            break;
        case OP_LABEL :
            op->u.no=label_cnt++;
            break;
        case OP_FUNCTION :
            strcpy(op->u.name, va_arg(args, char*));
            break;
        default :
            if(DEBUG){
                printf("Error: Unknown Operand kind %d\n", kind);
                assert(0);
            }
    }
    return op;
}


void addCode(int kind,...){
    //创建中间代码
    InterCodes code=(InterCodes)malloc(sizeof(struct InterCodes_));
    code->code.kind=kind;
    va_list args;
    va_start(args, kind);
    
    //根据中间代码的类型，设置相应的操作数
    switch(kind){
        case IR_LABEL:
        case IR_FUNCTION:
        case IR_GOTO:
        case IR_RETURN:
        case IR_ARG:
        case IR_PARAM:
        case IR_READ:
        case IR_WRITE:
            code->code.u.one.op=va_arg(args, Operand);
            break;
        case IR_ASSIGN:
        case IR_DEC:
        case IR_CALL:
        case IR_GETADDR:
        case IR_READADDR:
        case IR_WRITEADDR:
            code->code.u.two.left=va_arg(args, Operand);
            code->code.u.two.right=va_arg(args, Operand);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            code->code.u.three.result=va_arg(args, Operand);
            code->code.u.three.op1=va_arg(args, Operand);
            code->code.u.three.op2=va_arg(args, Operand);
            break;
        case IR_IF:
            code->code.u.four.op1=va_arg(args, Operand);
            code->code.u.four.relop=va_arg(args, char*);
            code->code.u.four.op2=va_arg(args, Operand);
            code->code.u.four.label=va_arg(args, Operand);
            break;
        default:
            if(DEBUG){
                printf("Error: Unknown IR code kind %d\n", kind);
                assert(0);
            }

    }
    //if(kind==IR_READADDR)
    //    printf("%d\n",code->code.u.two.right->u.no);
    va_end(args);
    //将中间代码添加到链表尾部
    code->next=codes_head;
    code->prev=codes_tail;
    codes_tail->next=code;
    codes_head->prev=code;
    codes_tail=code;
}

void printOP(FILE* fp,Operand op){
    //打印操作数
    if(op->kind==OP_VARIABLE){
        fprintf(fp,"%s",op->u.name);
    }
    else if(op->kind==OP_TEMP){
        fprintf(fp,"t%d",op->u.no);
    }
    else if(op->kind==OP_CONSTANT){
        fprintf(fp,"#%d",op->u.val);
    }
    else if(op->kind==OP_LABEL){
        fprintf(fp,"label%d",op->u.no);
    }
    else if(op->kind==OP_FUNCTION){
        fprintf(fp,"%s",op->u.name);
    }
}

void printCode(FILE * fp){
    //TODO : 将中间代码打印到文件
    //printf("printCode\n");
    //遍历InterCodes链表
    
    InterCodes p=codes_head->next;
    while(p!=codes_head){
        //根据中间代码的类型，打印相应的操作数
        switch(p->code.kind){
            //单目
            case IR_LABEL:
                fprintf(fp,"LABEL ");
                printOP(fp,p->code.u.one.op);
                fprintf(fp," :\n");
                break;
            case IR_FUNCTION:
                fprintf(fp,"FUNCTION ");
                printOP(fp,p->code.u.one.op);
                fprintf(fp," :\n");
                break;
            case IR_GOTO:
                fprintf(fp,"GOTO ");
                printOP(fp,p->code.u.one.op);
                fprintf(fp,"\n");
                break;
            case IR_RETURN:
                fprintf(fp,"RETURN ");
                printOP(fp,p->code.u.one.op);
                fprintf(fp,"\n");
                break;
            case IR_ARG:
                fprintf(fp,"ARG ");
                printOP(fp,p->code.u.one.op);
                fprintf(fp,"\n");
                break;
            case IR_PARAM:
                fprintf(fp,"PARAM ");
                printOP(fp,p->code.u.one.op);
                fprintf(fp,"\n");
                break;
            case IR_READ:
                fprintf(fp,"READ ");
                printOP(fp,p->code.u.one.op);
                fprintf(fp,"\n");
                break;
            case IR_WRITE:
                fprintf(fp,"WRITE ");
                printOP(fp,p->code.u.one.op);
                fprintf(fp,"\n");
                break;
            //双目
            case IR_ASSIGN:
                printOP(fp,p->code.u.two.left);
                fprintf(fp," := ");
                printOP(fp,p->code.u.two.right);
                fprintf(fp,"\n");
                break;
            case IR_DEC:
                fprintf(fp,"DEC ");
                printOP(fp,p->code.u.two.left);
                fprintf(fp," %d\n",p->code.u.two.right->u.val);
                break;
            case IR_CALL:
                printOP(fp,p->code.u.two.left);
                fprintf(fp," := CALL ");
                printOP(fp,p->code.u.two.right);
                fprintf(fp,"\n");
                break;
            case IR_GETADDR:
                printOP(fp,p->code.u.two.left);
                fprintf(fp," := &");
                printOP(fp,p->code.u.two.right);
                fprintf(fp,"\n");
                break;
            case IR_READADDR:
                printOP(fp,p->code.u.two.left);
                fprintf(fp," := *");
                printOP(fp,p->code.u.two.right);
                fprintf(fp,"\n");
                break;
            case IR_WRITEADDR:
                fprintf(fp,"*");
                printOP(fp,p->code.u.two.left);
                fprintf(fp," := ");
                printOP(fp,p->code.u.two.right);
                fprintf(fp,"\n");
                break;
            //三目
            case IR_ADD:
                printOP(fp,p->code.u.three.result);
                fprintf(fp," := ");
                printOP(fp,p->code.u.three.op1);
                fprintf(fp," + ");
                printOP(fp,p->code.u.three.op2);
                fprintf(fp,"\n");
                break;
            case IR_SUB:
                printOP(fp,p->code.u.three.result);
                fprintf(fp," := ");
                printOP(fp,p->code.u.three.op1);
                fprintf(fp," - ");
                printOP(fp,p->code.u.three.op2);
                fprintf(fp,"\n");
                break;
            case IR_MUL:
                printOP(fp,p->code.u.three.result);
                fprintf(fp," := ");
                printOP(fp,p->code.u.three.op1);
                fprintf(fp," * ");
                printOP(fp,p->code.u.three.op2);
                fprintf(fp,"\n");
                break;               
            case IR_DIV:
                printOP(fp,p->code.u.three.result);
                fprintf(fp," := ");
                printOP(fp,p->code.u.three.op1);
                fprintf(fp," / ");
                printOP(fp,p->code.u.three.op2);
                fprintf(fp,"\n");
                break;
            //四目
            case IR_IF:
                fprintf(fp,"IF ");
                printOP(fp,p->code.u.four.op1);
                fprintf(fp," %s ",p->code.u.four.relop);
                printOP(fp,p->code.u.four.op2);
                fprintf(fp," GOTO ");
                printOP(fp,p->code.u.four.label);
                fprintf(fp,"\n");
                break;
        }
        p=p->next;
    }
        
}

Operand addr2var(Operand op){
    if(op->isAddr==1){
        //如果是地址，先取值
        Operand tmp1=newOP(OP_TEMP,0);
        addCode(IR_READADDR,tmp1,op);
        op=tmp1;
    }
    return op;
}

Operand const2var(Operand op){
    if(op->kind==OP_CONSTANT){
        //如果是常量，先取值
        Operand tmp1=newOP(OP_TEMP,0);
        addCode(IR_ASSIGN,tmp1,op);
        op=tmp1;
    }
    return op;
}

int getSize(FieldList field){
    if(field->type->kind==ARRAY){
        
        Type elem=field->type->u.array.elem;
        int size=field->type->u.array.size;
        
        while(elem->kind==ARRAY){
            size*=elem->u.array.size;
            elem=elem->u.array.elem;
        }
        
        if(elem->kind==BASIC){
            size*=4;
        }
        else{
            //结构体
            FieldList p=elem->u.structure.sf;
            int structSize=0;
            while(p!=NULL){
                //如果sf对应的变量不在符号表，假如符号表Var，并且将offset设置成structSize
                //否则已经插入符号表，不再更新...
                struct SymbolItem_* item=isDefined(Var,p->name);
                item->field->offset=structSize;
                structSize+=getSize(p);
                p=p->tail;
            }
            size*=structSize;
        }
        int tmp=size;
        elem=field->type;
        while(elem->kind==ARRAY&&elem->u.array.elem->kind==ARRAY){
            
            tmp/=elem->u.array.size;
            //printf("line 366 %d\n",tmp);
            elem=elem->u.array.elem;
            elem->u.array.width=tmp;
            
        }
        return size;
    }
    else if(field->type->kind==STRUCTURE){
        //结构体
        FieldList p=field->type->u.structure.sf;
        int size=0;
        while(p!=NULL){
            //assert(0);
            struct SymbolItem_* item=isDefined(Var,p->name);
            item->field->offset=size;
            //printTable(Var);
            size+=getSize(p);
            p=p->tail;
        }
        return size;
    }
    else{
        //基本类型
        return 4;
    }
}

void makeIR(struct node* root,FILE* fp){ 
    //准备工作
    initIR();

    //遍历语法树，生成中间代码
    tProgram(root);
    
    //将中间代码打印到文件
    //printCode(fp);
    //freeIR();
}

void tProgram(struct node* root){
    if(root==NULL) return;
    if(strcmp(root->name,"ExtDef")==0){
        tExtDef(root);
    }
    for(int i=0;i<root->child_num;i++){
        tProgram(root->child[i]);
    }
}

/*
ExtDef : Specifier ExtDecList SEMI  
    | Specifier SEMI                
    | Specifier FunDec CompSt
    ;       
*/
void tExtDef(struct node* root){
    //翻译外部定义
    //更准确的,只需要处理函数定义
    assert(root!=NULL);
    if(root->child_num==3){
        if(strcmp(root->child[1]->name,"FunDec")==0){
            //函数定义
            tFunDec(root->child[1]);
            tCompSt(root->child[2]);
        }
    }
}
/*
FunDec : ID LP VarList RP   
   | ID LP RP
   ;
*/
void tFunDec(struct node* root){
    //FUNCTION f :
    //PARAM v1
    //...
    assert(root!=NULL);
    //提取ID
    assert(strcmp(root->child[0]->name,"ID")==0);
    char* name=root->child[0]->val;
    struct SymbolItem_* item=isDefined(def_func,name);
    //将argv加入到符号表中
    //遍历参数列表

    //创建函数操作数
    Operand funcOP=newOP(OP_FUNCTION,0,name);
    addCode(IR_FUNCTION,funcOP);
    //创建参数操作数
    FieldList p = item->field->type->u.function.argv;
    while(p!=NULL){
        //如果类型是数组或结构体，传址
        //否则传值
        int isAddr=0;
        if(p->type->kind==ARRAY||p->type->kind==STRUCTURE){
            isAddr=1;
            getSize(p);
            item=isDefined(Var,p->name);
            item->isAddr=1;
        }
        Operand paramOP=newOP(OP_VARIABLE,isAddr,p->name);
        addCode(IR_PARAM,paramOP);
        p=p->tail;
    }
}

/* CompSt : LC DefList StmtList RC
    |  : LC StmtList RC 
    |  : LC DefList RC
    |  : LC RC
*/
void tCompSt(struct node* root){
    //对于DefList，我们只关心数组和结构体变量.
    assert(root!=NULL);
    if(root->child_num==4){
        //CompSt : LC DefList StmtList RC
        tDefList(root->child[1]);
        tStmtList(root->child[2]);
    }
    else if(root->child_num==3){
        //CompSt : LC StmtList RC
        //CompSt : LC DefList RC
        if(strcmp(root->child[1]->name,"StmtList")==0){
            //CompSt : LC StmtList RC
            tStmtList(root->child[1]);
        }
        else{
            //CompSt : LC DefList RC
            tDefList(root->child[1]);
        }
    }
    else if(root->child_num==2){
        //CompSt : LC RC
        //do nothing
    }
}

/*
DefList : Def DefList
    ;
*/
void tDefList(struct node* root){
    //翻译定义列表
    //只关心数组和结构体变量
    assert(root!=NULL);
    tDef(root->child[0]);
    if(root->child_num==2){
        tDefList(root->child[1]);
    }
}

/*
Def : Specifier DecList SEMI 
    ;  
*/
void tDef(struct node* root){
    //翻译定义
    //不需要再管Specifier了
    assert(root!=NULL);
    tDecList(root->child[1]);
}

/*
DecList : Dec
    | Dec COMMA DecList 
    ;
*/
void tDecList(struct node* root){
    assert(root!=NULL);
    if(root->child_num==1){
        //DecList : Dec
        tDec(root->child[0]);
    }
    else if(root->child_num==3){
        //DecList : Dec COMMA DecList
        tDec(root->child[0]);
        tDecList(root->child[2]);
        
    }
}

/*
Dec : VarDec                
    | VarDec ASSIGNOP Exp
    ;
;
*/
void tDec(struct node* root){
    //翻译变量定义
    assert(root!=NULL);
    if(root->child_num==1){
        tVarDec(root->child[0]);
    }
    else{
        Operand op1=tVarDec(root->child[0]);
        Operand op2=tExp(root->child[2]);
        op2=addr2var(op2);
        addCode(IR_ASSIGN,op1,op2);
    }
}

/*
VarDec : ID             
    | VarDec LB INT RB 
    ;
*/
Operand tVarDec(struct node* root){
    //如果是数组或结构体，申请空间
    //否则直接返回变量操作数
    assert(root!=NULL);
    //一直展开找到ID
    if(root->child_num==1){
        //ID!
        //查表
        struct SymbolItem_* item=isDefined(Var,root->child[0]->val);
        //判断类型
        //如果是数组或结构体，申请空间
        if(item->field->type->kind==ARRAY||item->field->type->kind==STRUCTURE){
            //数组or结构题
            //申请空间
            //TODO
            int size=getSize(item->field);
            Operand op1=newOP(OP_VARIABLE,0,root->child[0]->val);
            Operand op2=newOP(OP_CONSTANT,0,size);
            //printf("line 340 size:%d\n",size);
            addCode(IR_DEC,op1,op2);
            return op1;
        }
        
        Operand op=newOP(OP_VARIABLE,0,root->child[0]->val);
        return op;
        
    }
    else{
        //找下去
        return tVarDec(root->child[0]);
    }

}

/*
StmtList : Stmt StmtList
    | Stmt
    ;
*/
void tStmtList(struct node* root){
    //翻译语句列表
    assert(root!=NULL);
    tStmt(root->child[0]);
    if(root->child_num==2){
        tStmtList(root->child[1]);
    }
}

/*
Stmt : CompSt                       1
    | Exp SEMI                      2                                               
    | RETURN Exp SEMI               3             
    | IF LP Exp RP Stmt             5     
    | WHILE LP Exp RP Stmt          5
    | IF LP Exp RP Stmt ELSE Stmt   7                    
    ;
*/
void tStmt(struct node* root){
    assert(root!=NULL);
    if(root->child_num==1){
        tCompSt(root->child[0]);
    }
    else if(root->child_num==2){
        tExp(root->child[0]);
    }
    else if(root->child_num==3){
        Operand op=tExp(root->child[1]);
        op=addr2var(op);
        addCode(IR_RETURN,op);
    }
    else if(root->child_num==5){
        if(strcmp(root->child[0]->name,"IF")==0){
            //IF LP Exp RP Stmt
            Operand label_true=newOP(OP_LABEL,0);
            Operand label_false=newOP(OP_LABEL,0);
            tCondExp(root->child[2],label_true,label_false);
            //true
            addCode(IR_LABEL,label_true);
            tStmt(root->child[4]);
            //false
            addCode(IR_LABEL,label_false);
        }
        else if(strcmp(root->child[0]->name,"WHILE")==0){
            //WHILE LP Exp RP Stmt
            Operand label_begin=newOP(OP_LABEL,0);
            Operand label_true=newOP(OP_LABEL,0);
            Operand label_false=newOP(OP_LABEL,0);
            addCode(IR_LABEL,label_begin);
            tCondExp(root->child[2],label_true,label_false);
            //true
            addCode(IR_LABEL,label_true);
            tStmt(root->child[4]);
            addCode(IR_GOTO,label_begin);
            //false
            addCode(IR_LABEL,label_false);
        }
    }
    else {
        //IF LP Exp RP Stmt ELSE Stmt
        
        Operand label_true=newOP(OP_LABEL,0);
        Operand label_false=newOP(OP_LABEL,0);
        Operand label_next=newOP(OP_LABEL,0);
        tCondExp(root->child[2],label_true,label_false);
        //true
        addCode(IR_LABEL,label_true);
        tStmt(root->child[4]);
        addCode(IR_GOTO,label_next);
        //false
        
        addCode(IR_LABEL,label_false);
        tStmt(root->child[6]);
        addCode(IR_LABEL,label_next);
    }
}

/*
Exp :  ID LP Args RP    4
    | Exp LB Exp RB     4
    | Exp ASSIGNOP Exp  3
    | Exp AND Exp       3
    | Exp OR Exp        3
    | Exp RELOP Exp     3
    | Exp PLUS Exp      3
    | Exp MINUS Exp     3
    | Exp STAR Exp      3
    | Exp DIV Exp       3
    | LP Exp RP         3
    | ID LP RP          3     
    | Exp DOT ID        3
    | MINUS Exp         2
    | NOT Exp           2         
    | ID                1 //不要求支持高维数组的直接赋值，如果ID是数组，那么直接传地址
    | INT               1
    | FLOAT             1
    ;

*/
Operand tExp(struct node* root){//可传入tmp
    assert(root!=NULL);
    if(root->child_num==4){
        if(strcmp(root->child[0]->name,"ID")==0){
            //带参函数调用
            //TODO
            //ID LP Args RP
            
            OPList list=tArgs(root->child[2]);
            //头插法
            //所以是倒过来的
            //创建函数操作数
            if(strcmp(root->child[0]->val,"write")==0){
                addCode(IR_WRITE,list->op);
            }
            else{
                while(list!=NULL){
                    addCode(IR_ARG,list->op);
                    list=list->next;
                }
                Operand func=newOP(OP_FUNCTION,0,root->child[0]->val);
                Operand tmp=newOP(OP_TEMP,0);
                addCode(IR_CALL,tmp,func);
                return tmp;
            }
        }
        else{
            //数组下标
            //Exp LB Exp RB
            //另写一个函数计算地址...
            //TBD
            Operand base=tExp(root->child[0]);
            //结构体数组...
            Operand width_num=tExp(root->child[2]);
            Type curtype=Exp(root);
            Operand width=NULL;
            if(curtype->kind==ARRAY){
                int w=curtype->u.array.width;
                //printf("line 744 %d\n",w);
                width=newOP(OP_CONSTANT,1,w);
            }
            else if(curtype->kind==STRUCTURE){
                FieldList p=curtype->u.structure.sf;
                int w=0;
                while(p!=NULL){
                    w+=getSize(p);
                    p=p->tail;
                }
                width=newOP(OP_CONSTANT,1,w);
            }
            else{
                //其他类型
                width=newOP(OP_CONSTANT,1,4);
            }
            Operand tmp1=newOP(OP_TEMP,1);
            //计算地址
            Operand tmp=newOP(OP_TEMP,1);
            addCode(IR_MUL,tmp1,width,width_num);
            addCode(IR_ADD,tmp,base,tmp1);
            return tmp;
        }
    }
    else if(root->child_num==3){
        if(strcmp(root->child[1]->name,"ASSIGNOP")==0){
            //Exp ASSIGNOP Exp
            //赋值语句
            Operand op1=tExp(root->child[0]);
            Operand op2=tExp(root->child[2]);
            //特判一下
            //检查op1和op2是不是都是数组（不要求多维数组赋值，因此此处为一维数组）
            //如果是数组，按相对地址赋值下去
            if(root->child[0]->child_num==1&&root->child[2]->child_num==1){
                //ID
                struct node* id1=root->child[0]->child[0];
                struct node* id2=root->child[2]->child[0];
                if(strcmp(id1->name,"ID")==0&&strcmp(id2->name,"ID")==0){
                    //ID ID
                    //查表
                    struct SymbolItem_* item1=isDefined(Var,id1->val);
                    struct SymbolItem_* item2=isDefined(Var,id2->val);
                
                    assert(item1!=NULL);
                    assert(item2!=NULL);
                    if(item1->field->type->kind==ARRAY&&item2->field->type->kind==ARRAY){
                        //一维数组
                        //逐个赋值
                        //假定item1和item2的基类型都为BASIC
                        if(item1->field->type->u.array.elem->kind==BASIC&&item2->field->type->u.array.elem->kind==BASIC){
                            //不支持高维数组的直接赋值和结构体的直接赋值
                            //先偷个小懒
                            int size1=item1->field->type->u.array.size;
                            int size2=item2->field->type->u.array.size;
                            int size=size1<size2?size1:size2;
                            Operand base1=op1;
                            Operand base2=op2;
                            for(int i=0;i<size;i++){
                                //逐个赋值
                                //计算offset+base
                                Operand offset=newOP(OP_CONSTANT,1,i*4);
                                Operand tmp1=newOP(OP_TEMP,1);
                                Operand tmp2=newOP(OP_TEMP,1);
                                addCode(IR_ADD,tmp1,base1,offset);
                                addCode(IR_ADD,tmp2,base2,offset);
                                //tmp1=base1+offset
                                //tmp2=base2+offset
                                tmp2=addr2var(tmp2);
                                addCode(IR_WRITEADDR,tmp1,tmp2);
                                //赋值
                            }

                            op1=addr2var(op1);
                            return op1;
                            
                        }
                    }
                }
                
            }
            
            
            
            op2=addr2var(op2);
            if(op1->isAddr==1){
                    //如果是地址，先取值;
                addCode(IR_WRITEADDR,op1,op2);
            }
            else addCode(IR_ASSIGN,op1,op2);
            op1=addr2var(op1);
            return op1;

            
        }
        else if(strcmp(root->child[1]->name,"OR")==0){
            //逻辑或
            //a||b
            //t=1;
            //...
            //label2:
            //label3:
            //t=0;
            //label1:
            Operand tmp=newOP(OP_TEMP,0);
            Operand zero=newOP(OP_CONSTANT,0,0);
            Operand one=newOP(OP_CONSTANT,0,1);
            Operand label_true=newOP(OP_LABEL,0);
            Operand label_false1=newOP(OP_LABEL,0);
            Operand label_false2=newOP(OP_LABEL,0);
            addCode(IR_ASSIGN,tmp,one);
            tCondExp(root->child[0],label_true,label_false1);
            addCode(IR_LABEL,label_false1);
            tCondExp(root->child[2],label_true,label_false2);
            addCode(IR_LABEL,label_false2);
            addCode(IR_ASSIGN,tmp,zero);
            addCode(IR_LABEL,label_true);
            return tmp;

        }
        else if(strcmp(root->child[1]->name,"AND")==0){
            //逻辑与
            Operand tmp=newOP(OP_TEMP,0);
            Operand zero=newOP(OP_CONSTANT,0,0);
            Operand one=newOP(OP_CONSTANT,0,1);
            Operand label_true1=newOP(OP_LABEL,0);
            Operand label_true2=newOP(OP_LABEL,0);
            Operand label_false=newOP(OP_LABEL,0);
            addCode(IR_ASSIGN,tmp,zero);
            tCondExp(root->child[0],label_true1,label_false);
            addCode(IR_LABEL,label_true1);
            tCondExp(root->child[2],label_true2,label_false);
            addCode(IR_LABEL,label_true2);
            addCode(IR_ASSIGN,tmp,one);
            addCode(IR_LABEL,label_false);
            return tmp;
        }
        else if(strcmp(root->child[1]->name,"RELOP")==0){
            //关系运算符
            Operand tmp=newOP(OP_TEMP,0);
            Operand zero=newOP(OP_CONSTANT,0,0);
            Operand one=newOP(OP_CONSTANT,0,1);
            Operand label_true=newOP(OP_LABEL,0);
            Operand label_false=newOP(OP_LABEL,0);
            addCode(IR_ASSIGN,tmp,zero);
            tCondExp(root,label_true,label_false);
            addCode(IR_LABEL,label_true);
            addCode(IR_ASSIGN,tmp,one);
            addCode(IR_LABEL,label_false);
            return tmp;

        }
        else if(strcmp(root->child[1]->name,"PLUS")==0){
            //加法
            Operand op1=tExp(root->child[0]);
            Operand op2=tExp(root->child[2]);
            op1=addr2var(op1);
            op2=addr2var(op2);
            Operand tmp=newOP(OP_TEMP,0);
            addCode(IR_ADD,tmp,op1,op2);
            return tmp;
        }
        else if(strcmp(root->child[1]->name,"MINUS")==0){
            //减法
            Operand op1=tExp(root->child[0]);
            Operand op2=tExp(root->child[2]);
            op1=addr2var(op1);
            op2=addr2var(op2);
            Operand tmp=newOP(OP_TEMP,0);
            addCode(IR_SUB,tmp,op1,op2);
            return tmp;
        }
        else if(strcmp(root->child[1]->name,"STAR")==0){
            //乘法
            Operand op1=tExp(root->child[0]);
            Operand op2=tExp(root->child[2]);
            op1=addr2var(op1);
            op2=addr2var(op2);
            Operand tmp=newOP(OP_TEMP,0);
            addCode(IR_MUL,tmp,op1,op2);
            return tmp;
        }
        else if(strcmp(root->child[1]->name,"DIV")==0){
            //除法

            Operand op1=tExp(root->child[0]);
            Operand op2=tExp(root->child[2]);
            op1=addr2var(op1);
            op2=addr2var(op2);
            Operand tmp=newOP(OP_TEMP,0);
            addCode(IR_DIV,tmp,op1,op2);
            return tmp;
        }
        else if(strcmp(root->child[1]->name,"Exp")==0){
            //括号
            return tExp(root->child[1]);
        }
        else if(strcmp(root->child[1]->name,"DOT")==0){
            //结构体成员访问
            //Exp DOT ID
            //TODO
            //一路展下去找到id，然后算size
            //对于数组结构体，假定已经返回了当前维的地址，也即是base
            //只需要base+offset就可以了
            //而计算offset
            Operand base=tExp(root->child[0]);
            struct SymbolItem_* item=isDefined(Var,root->child[2]->val);
            //查表
            //计算偏移量
            //printf("line 408 %s\n",root->child[2]->val);
            assert(item!=NULL);

            int off=item->field->offset;
            
            Operand offset=newOP(OP_CONSTANT,1,off);
            Operand tmp=newOP(OP_TEMP,1);
            addCode(IR_ADD,tmp,base,offset);
            return tmp;
        }
        else if(strcmp(root->child[0]->name,"ID")==0){
            //函数调用
            //ID LP RP
            //TODO
            //函数调用
            
            Operand tmp=newOP(OP_TEMP,0);
            if(strcmp(root->child[0]->val,"read")==0){
                //read函数
                addCode(IR_READ,tmp);
            }
            else{
                //其他函数调用
                //TODO
                Operand func=newOP(OP_FUNCTION,0,root->child[0]->val);
                addCode(IR_CALL,tmp,func);
            }
            return tmp;
        }
    }
    else if(root->child_num==2){
        if(strcmp(root->child[0]->name,"MINUS")==0){
            //改成 t=#0-Exp
            Operand tmp=newOP(OP_TEMP,0);
            Operand zero= newOP(OP_CONSTANT,0,0);
            Operand op=tExp(root->child[1]);
            op=addr2var(op);
            addCode(IR_SUB,tmp,zero,op);
            return tmp;
        }
        else if(strcmp(root->child[0]->name,"NOT")==0){
            //真为1，假为0
            //t=0;
            //if istrue goto label_true:
            //goto laber_false:
            //label_true:
            //t=1;
            //laber_false:
            Operand tmp=newOP(OP_TEMP,0);
            Operand zero=newOP(OP_CONSTANT,0,0);
            Operand one=newOP(OP_CONSTANT,0,1);
            Operand label_true=newOP(OP_LABEL,0);
            Operand label_false=newOP(OP_LABEL,0);
            addCode(IR_ASSIGN,tmp,zero);
            tCondExp(root->child[1],label_false,label_true);//not,反转一下
            addCode(IR_LABEL,label_true);
            addCode(IR_ASSIGN,tmp,one);
            addCode(IR_LABEL,label_false);
            return tmp;
        }
    }
    else{
        if(strcmp(root->child[0]->name,"ID")==0){
            //先查符号表看看是不是数组
            
            struct SymbolItem_* item=isDefined(Var,root->child[0]->val);
            if(item->field->type->kind==ARRAY||item->field->type->kind==STRUCTURE){
                //是数组或结构体，返回地址
                //To be done!!!!!一维数组的直接赋值
                
                if(item->isAddr==1){
                    //传进来的就是地址
                    Operand op=newOP(OP_VARIABLE,1,root->child[0]->val);
                    return op;
                }
                //否则，申请一个临时变量
                Operand op=newOP(OP_VARIABLE,0,root->child[0]->val);
                Operand tmp=newOP(OP_TEMP,1);
                addCode(IR_GETADDR,tmp,op);
                return tmp;
            }
            else{
                //不是数组or结构体，直接返回变量操作数
                Operand op=newOP(OP_VARIABLE,0,root->child[0]->val);
                return op;
            }
        }
        else if(strcmp(root->child[0]->name,"INT")==0){
            //返回常量操作数
            Operand op=newOP(OP_CONSTANT,0,atoi(root->child[0]->val));
            //op=const2var(op);
            return op;
        }
        else if(strcmp(root->child[0]->name,"FLOAT")==0){
            //不会出现，直接返回常数0得了
            Operand op=newOP(OP_CONSTANT,0,0);
            //op=const2var(op);
            return op;
        }
    }
}

/*
Exp : Exp AND Exp       3
    | Exp OR Exp        3
    | Exp RELOP Exp     3
    | NOT Exp           2 
    ...
*/
void tCondExp(struct node* root,Operand label_true,Operand label_false){//传入tmp
    assert(root!=NULL);
    if(strcmp(root->child[0]->name,"NOT")==0){
        tCondExp(root->child[1],label_false,label_true);
        return;
    }
    if(root->child_num==3){
        if(strcmp(root->child[1]->name,"AND")==0){
            //逻辑与
            Operand label=newOP(OP_LABEL,0);
            tCondExp(root->child[0],label,label_false);
            addCode(IR_LABEL,label);
            tCondExp(root->child[2],label_true,label_false);
            return;
        }
        else if(strcmp(root->child[1]->name,"OR")==0){
            //逻辑或
            Operand label=newOP(OP_LABEL,0);
            tCondExp(root->child[0],label_true,label);
            addCode(IR_LABEL,label);
            tCondExp(root->child[2],label_true,label_false);
            return;
        }
        else if(strcmp(root->child[1]->name,"RELOP")==0){
            //关系运算符
            
            Operand op1=tExp(root->child[0]);
            Operand op2=tExp(root->child[2]);
            op1=addr2var(op1);
            op2=addr2var(op2);
            addCode(IR_IF,op1,root->child[1]->val,op2,label_true);
            addCode(IR_GOTO,label_false);
            return;
        }
    }
    //other
    Operand op=tExp(root);
    op=addr2var(op);
    addCode(IR_IF,op,"!=",newOP(OP_CONSTANT,0,0),label_true);
    addCode(IR_GOTO,label_false);
    return;
}
/*
Args : Exp COMMA Args   
    | Exp
*/

OPList tArgs(struct node* root){
    //翻译参数列表
    assert(root!=NULL);
    OPList list=(OPList)malloc(sizeof(struct OPList_));
    list->next=NULL;
    list->op=tExp(root->child[0]);
    Type type=Exp(root->child[0]);
    //判断类型
    //如果是数组或结构体，传址
    //否则传值
    if(type->kind!=ARRAY&&type->kind!=STRUCTURE){
        list->op=addr2var(list->op);
    }

    if(root->child_num==3){
        //Args : Exp COMMA Args
        //将当前exp插到最后
        OPList h=tArgs(root->child[2]);
        OPList p=h;
        while(p->next!=NULL){
            p=p->next;
        }
        p->next=list;
        list=h;
    }
    return list;
}