#include "semantic.h"

/*
typedef struct node
{
    char name[MAX_NAME_LEN];
    char val[MAX_VAL_LEN];
    int lineno;
    int child_num;//孩子节点个数
    struct node* child[MAX_CHILD_NUM];//孩子节点

}node;
*/

//完善插入检查...
void walkTree(struct node* root){
    
    if(root==NULL) return;

    if(strcmp(root->name,"ExtDef")==0){
        
        ExtDef(root);
    }

    for(int i=0;i<root->child_num;i++){
        walkTree(root->child[i]);
    }
}

/*
ExtDef : Specifier ExtDecList SEMI  
    | Specifier SEMI                
    | Specifier FunDec CompSt
    ;       
*/
void ExtDef(struct node* root){
    assert(root!=NULL);

    if(root->child_num==2){
        Specifier(root->child[0]);
    }
    else if(root->child_num==3){
        Type spec = Specifier(root->child[0]);
        if(strcmp(root->child[1]->name,"ExtDecList")==0){
            FieldList field = ExtDecList(root->child[1],spec);
            //将field中的东西加入到符号表中
            //遍历fieldlist
            FieldList p = field;
            while(p!=NULL){
                struct SymbolItem_* item = (struct SymbolItem_*)malloc(sizeof(struct SymbolItem_));
                item->kind = VARIABLE;
                item->field = (FieldList)malloc(sizeof(struct FieldList_));
                item->field->tail = NULL;
                assert(p->name!=NULL);
                strcpy(item->field->name,p->name);
                item->field->type = p->type;
                item->field->lineno = p->lineno;
                item->flag = 0;
                insert(table, item->field->name, item);
                p=p->tail;
            }
        }
        else if(strcmp(root->child[1]->name,"FunDec")==0){
            //将函数加入到符号表中
            //考虑一下函数不往符号表里放？我们好像允许函数和变量重名欸...
            //直接来一个全局的哈希好像就可了
            //如果是定义
            if(strcmp(root->child[2]->name,"CompSt")==0){
                struct SymbolItem_* item = (struct SymbolItem_*)malloc(sizeof(struct SymbolItem_));
                item->kind = FUNCTION;
                item->field = (FieldList)malloc(sizeof(struct FieldList_));
                item->field->tail = NULL;
                assert(root->child[1]->child[0]->val!=NULL);
                strcpy(item->field->name,root->child[1]->child[0]->val);
                item->field->type = FunDec(root->child[1],spec);
                item->flag = 1;
                item->field->lineno = root->lineno;
                definsert(item->field->name, item);
                addDepth();

                //实现函数
                //将argv加入到符号表中
                FieldList p = item->field->type->u.function.argv;
                while(p!=NULL){
                    struct SymbolItem_* item1 = (struct SymbolItem_*)malloc(sizeof(struct SymbolItem_));
                    item1->kind = VARIABLE;
                    item1->field = (FieldList)malloc(sizeof(struct FieldList_));
                    item1->field->tail = NULL;
                    assert(p->name!=NULL);
                    strcpy(item1->field->name,p->name);
                    item1->field->type = p->type;
                    item1->flag = 0;
                    item1->field->lineno = p->lineno;
                    insert(table,item1->field->name, item1);
                    p=p->tail;
                }
                CompSt(root->child[2]);//进行检查和符号表维护.
                
                minusDepth();
            }
            else if(strcmp(root->child[2]->name,"SEMI")==0){
                //函数声明
                //加入声明表，检查有没有冲突声明，检查有没有冲突定义//Error type 19 at Line 8: Inconsistent declaration of function "func".
                //main函数最后检查有没有声明未定义//Error type 18 at Line 6: Undefined function "func". 
                struct SymbolItem_* item = (struct SymbolItem_*)malloc(sizeof(struct SymbolItem_));
                item->kind = FUNCTION;
                item->field = (FieldList)malloc(sizeof(struct FieldList_));
                item->field->tail = NULL;
                assert(root->child[1]->child[0]->val!=NULL);
                strcpy(item->field->name,root->child[1]->child[0]->val);
                item->field->type = FunDec(root->child[1],spec);
                item->flag = 0;
                item->field->lineno = root->lineno;

                //检查有没有冲突的声明或定义
                decinsert(item->field->name, item);
                
            }
            
        }
        else{
            assert(0);
        }
    }
    else{
        assert(0);
    }
    
}


//FunDec : ID LP VarList RP   
//   | ID LP RP  
/*
VarList : ParamDec COMMA VarList    
    | ParamDec                      
    ;
ParamDec : Specifier VarDec   
    ;
*/           //int a,int b,int a[][] 
Type FunDec(struct node* root,Type spec){
    //返回“函数”类型 
    //struct {Type returnType; int argc; FieldList argv; } function;
    //FunDec : ID LP VarList RP   
    //   | ID LP RP
    assert(root!=NULL);
    assert(root->child_num==4||root->child_num==3);
    //待完成：对函数声明的处理，当前假设，函数只支持定义
    
    Type ret = (Type)malloc(sizeof(struct Type_));

    ret->kind=FUNCTION;
    ret->u.function.returnType = spec;
    retType=spec;
    ret->u.function.argc=0;

    if(root->child_num==4){
        //FunDec : ID LP VarList RP
        
        ret->u.function.argv=VarList(root->child[2],ret);
        
        
    }
    else if(root->child_num==3){
        //FunDec : ID LP RP
        ret->u.function.argv=NULL;
    }
    else{
        
    }
    
    return ret;

}
FieldList VarList(struct node* root,Type type){
    //在这里面更新argc
    assert(root!=NULL);
    assert(root->child_num==3||root->child_num==1);
    if(root->child_num==1){
        //VarList : ParamDec
        type->u.function.argc++;
        return ParamDec(root->child[0]);
    }
    else if(root->child_num==3){
        type->u.function.argc++;
        FieldList tmp1 = ParamDec(root->child[0]);
        FieldList tmp2 = VarList(root->child[2],type);
        tmp1->tail = tmp2;
        return tmp1;
    }else{
        assert(0);
    }
}

//ParamDec : Specifier VarDec
//    ;
FieldList ParamDec(struct node* root){
    assert(root!=NULL);
    assert(root->child_num==2);
    Type spec = Specifier(root->child[0]);
    FieldList ret = VarDec(root->child[1],spec);
    return ret;
}

//CompSt : LC DefList StmtList RC
//    |  : LC StmtList RC 
//    |  : LC DefList RC
//    |  : LC RC
void CompSt(struct node* root){
    //函数体内部的CompSt传入type时给NULL
    assert(root!=NULL);
    assert(root->child_num==4||root->child_num==3||root->child_num==2);
    if(root->child_num==3){
        //检查就是了
        if(strcmp(root->child[1]->name,"StmtList")==0){
            StmtList(root->child[1]);
        }
        else if(strcmp(root->child[1]->name,"DefList")==0){
            //CompSt : LC DefList RC
            DefList(root->child[1]);
            //将field中的东西加入到符号表中
            //遍历fieldlist
            /*//这部分在Dec中实现...
            FieldList p = field;
            while(p!=NULL){
                struct SymbolItem_* item = (struct SymbolItem_*)malloc(sizeof(struct SymbolItem_));
                item->kind = VARIABLE;
                item->field = (FieldList)malloc(sizeof(struct FieldList_));
                item->field->tail = NULL;
                assert(p->name!=NULL);
                strcpy(item->field->name,p->name);
                item->field->type = p->type;
                item->flag = 0;
                item->field->lineno = p->lineno;
                insert(table, item->field->name, item);
                p=p->tail;
            }
            */
           
        }
        else{
            assert(0);
        }
        
    }
    else if(root->child_num==4){
        //检查之前先更新符号表，DefList！！！！
        DefList(root->child[1]);
        //将field中的东西加入到符号表中
        //遍历fieldlist
        /*
        FieldList p = field;
        while(p!=NULL){
            struct SymbolItem_* item = (struct SymbolItem_*)malloc(sizeof(struct SymbolItem_));
            item->kind = VARIABLE;
            item->field = (FieldList)malloc(sizeof(struct FieldList_));
            item->field->tail = NULL;
            assert(p->name!=NULL);
            strcpy(item->field->name,p->name);
            item->field->type = p->type;
            item->flag = 0;
            item->field->lineno = p->lineno;
            insert(table, item->field->name, item);
            p=p->tail;
        }
        */
        StmtList(root->child[2]);
    }
    else{
        //CompSt : LC RC
        //什么都不做
        
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
    | ID                1
    | INT               1
    | FLOAT             1
    ;
*/
Type Exp(struct node* root){
    //生成Exp的类型并进行一系列检查...
    //仅有int 型变量才能进行逻辑运算或者作为if 和while 语句的条件；
    //仅有int 型和float 型变量才能参与算术运算。
    assert(root!=NULL);
    assert(root->child_num==1||root->child_num==2||root->child_num==3||root->child_num==4);
    Type ret = NULL;
    if(root->child_num==1){
        // Exp : ID | INT | FLOAT
        //直接可提取类型
        if(strcmp(root->child[0]->name,"ID")==0){
            //查找符号表，得到其返回类型
            struct SymbolItem_* item = isDefined(table, root->child[0]->val);
            if(item==NULL){
                //未定义变量
                printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",root->lineno,root->child[0]->val);
                ret = (Type)malloc(sizeof(struct Type_));
                ret->kind=ERR;
            }
            else{
                ret = item->field->type;
                //printf("%s\n",item->field->name);
            }
        }
        else if(strcmp(root->child[0]->name,"INT")==0){
            ret = (Type)malloc(sizeof(struct Type_));
            ret->kind = BASIC;
            ret->u.basic = 0;
        }
        else if(strcmp(root->child[0]->name,"FLOAT")==0){
            ret = (Type)malloc(sizeof(struct Type_));
            ret->kind = BASIC;
            ret->u.basic = 1;
        }
        else{
            assert(0);
        }
    }
    else if(root->child_num==2){
        //Exp :  MINUS Exp         2
        //  | NOT Exp           2
        if(strcmp(root->child[0]->name,"MINUS")==0){
            //Exp : MINUS Exp
            assert(strcmp(root->child[1]->name,"Exp")==0);
            ret = Exp(root->child[1]);
            if(ret->kind!=BASIC){
                //类型不匹配
                //只有int和float可以进行算术运算
                printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
            }
        }
        else if(strcmp(root->child[0]->name,"NOT")==0){
            //Exp : NOT Exp
            assert(strcmp(root->child[1]->name,"Exp")==0);
            ret = Exp(root->child[1]);
            if(ret->kind!=BASIC||ret->u.basic!=0){
                //类型不匹配
                //只有int可以进行逻辑运算
                printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
            }
        }
        else{
            assert(0);
        }
    }
    else if(root->child_num==3){
        //Exp :  Exp ASSIGNOP Exp  3
        //  | Exp AND Exp       3  int型
        //  | Exp OR Exp        3  int型
        //  | Exp RELOP Exp     3  int或float型
        //  | Exp PLUS Exp      3  int或float型
        //  | Exp MINUS Exp     3  int或float型
        //  | Exp STAR Exp      3  int或float型
        //  | Exp DIV Exp       3  int或float型
        //  | LP Exp RP         3  直接扔掉括号
        //  | ID LP RP          3  函数调用   
        //  | Exp DOT ID        3  结构体成员

        //简单起见，可以只从语法层面来检查左值错误：
        //赋值号左边能出现的只有ID、Exp LB Exp RB 以及Exp DOT ID，而不能是其它形式的语法单元组合
        if(strcmp(root->child[1]->name,"ASSIGNOP")==0){
            //检查左值
            struct node* child = root->child[0];
            //只能展开成ID、Exp LB Exp RB 以及Exp DOT ID
            assert(child!=NULL);
            if(child->child_num!=1&&child->child_num!=4&&child->child_num!=3){
                //左值错误
                printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",root->lineno);
            }
            else if(child->child_num==1){
                //只能是ID
               if(strcmp(child->child[0]->name,"ID")!=0){
                    //左值错误
                    printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",root->lineno);
                }
            }
            else if(child->child_num==4){
                //只能是Exp LB Exp RB
                if(strcmp(child->child[0]->name,"Exp")!=0||strcmp(child->child[1]->name,"LB")!=0||strcmp(child->child[2]->name,"Exp")!=0||strcmp(child->child[3]->name,"RB")!=0){
                    //左值错误
                    printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",root->lineno);
                }
            }
            else if(child->child_num==3){
                //只能是Exp DOT ID
                if(strcmp(child->child[0]->name,"Exp")!=0||strcmp(child->child[1]->name,"DOT")!=0||strcmp(child->child[2]->name,"ID")!=0){
                    //左值错误
                    printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",root->lineno);
                }
            }
            else{
                assert(0);
            }

            Type type1 = Exp(root->child[0]);
            Type type2 = Exp(root->child[2]);
            assert(type1!=NULL&&type2!=NULL);
            if(TypeEqual(type1,type2)==0){
                //类型不匹配
                printf("Error type 5 at Line %d: Type mismatched for assignment.\n",root->lineno);
            }
            ret = type1;
        }
        else if(strcmp(root->child[1]->name,"Exp")==0){
            ret= Exp(root->child[1]);
        }
        else if(strcmp(root->child[1]->name,"AND")==0||strcmp(root->child[1]->name,"OR")==0){
            //  | Exp AND Exp       3  int型
            //  | Exp OR Exp        3  int型
            Type type1 = Exp(root->child[0]);
            Type type2 = Exp(root->child[2]);
            if(type1->kind!=BASIC||type1->u.basic!=0){
                //类型不匹配
                //只有int可以进行逻辑运算
                printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
            }
            else if(type2->kind!=BASIC||type2->u.basic!=0){
                //类型不匹配
                //只有int可以进行逻辑运算
                printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
            }
            
            ret = type1;

        }
        else if(strcmp(root->child[1]->name,"RELOP")==0||
                strcmp(root->child[1]->name,"PLUS")==0||
                strcmp(root->child[1]->name,"MINUS")==0||
                strcmp(root->child[1]->name,"STAR")==0||
                strcmp(root->child[1]->name,"DIV")==0){
                    

                    //  | Exp RELOP Exp     3  int或float型
                    //  | Exp PLUS Exp      3  int或float型
                    //  | Exp MINUS Exp     3  int或float型
                    //  | Exp STAR Exp      3  int或float型
                    //  | Exp DIV Exp       3  int或float型
                    
                    Type type1 = Exp(root->child[0]);
                    Type type2 = Exp(root->child[2]);
                    //BASIC类型才可以运算.
                    if(type1->kind!=BASIC||type2->kind!=BASIC){
                        printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
                    }
                    else if(TypeEqual(type1,type2)==0){
                        //类型不匹配
                        //只有int和float可以进行算术运算
                        printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
                    }
                    ret = type1;
            
        }
        else if(strcmp(root->child[1]->name,"DOT")==0){
            //  | Exp DOT ID        3  结构体成员
            Type type = Exp(root->child[0]);
            if(type->kind!=STRUCTURE){
                //类型不匹配
                //printf("%s %d\n",type->u.structure.name,root->lineno);
                printf("Error type 13 at Line %d: Not a structure.\n",root->lineno);
            }
            else{
                //检查有没有这个成员
                FieldList p = type->u.structure.sf;
                while(p!=NULL){
                    if(strcmp(p->name,root->child[2]->val)==0){
                        break;
                    }
                    p = p->tail;
                }
                if(p==NULL){
                    //没有这个成员
                    printf("Error type 14 at Line %d: Nonexistent field \"%s\".\n",root->lineno,root->child[2]->val);
                }
                else{
                    ret = p->type;
                }

            }
            if(ret==NULL){
                ret = (Type)malloc(sizeof(struct Type_));
                ret->kind=ERR;
            }
            
        }
        else if(strcmp(root->child[0]->name,"ID")==0){
            //  | ID LP RP          3  函数调用
            struct SymbolItem_* item = isDefined(def_func, root->child[0]->val);
            if(item==NULL){
                item = isDefined(dec_func, root->child[0]->val);
                if(item==NULL){
                    //未定义函数
                    item = isDefined(table, root->child[0]->val);
                    if(item==NULL){
                        printf("Error type 2 at Line %d: Undefined function \"%s\".\n",root->lineno,root->child[0]->val);
                        ret = (Type)malloc(sizeof(struct Type_));
                        ret->kind=ERR;
                    }
                    else{
                        //对普通变量使用“(…)”或“()”（函数调用）操作符
                        printf("Error type 11 at Line %d: \"%s\" is not a function.\n",root->lineno,root->child[0]->val);
                    }
                    
                }
                else{
                    //函数声明
                    ret = item->field->type->u.function.returnType;
                    //检查参数个数和类型
                    if(item->field->type->u.function.argc!=0){
                        //函数声明时参数个数不匹配
                        printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",root->lineno,root->child[0]->val);
                    }
                }
            }
            else{
                //函数定义
                ret = item->field->type->u.function.returnType;
                //检查参数个数和类型
                if(item->field->type->u.function.argc!=0){
                    //类型不匹配
                    printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",root->lineno,root->child[0]->val);
                }
                //assert(0);
            }
        }
        else{
            assert(0);
        }
       

        

    }
    else if(root->child_num==4){
        //Exp :  ID LP Args RP    4 //函数
        //  | Exp LB Exp RB     4   //数组
        if(strcmp(root->child[0]->name,"ID")==0){
            //Exp : ID LP Args RP
            
            assert(strcmp(root->child[1]->name,"LP")==0);
            assert(strcmp(root->child[3]->name,"RP")==0);
            struct SymbolItem_* item = isDefined(def_func, root->child[0]->val);
            if(item==NULL){
                item = isDefined(dec_func, root->child[0]->val);
                if(item==NULL){
                    //未定义函数
                    item = isDefined(table, root->child[0]->val);
                    if(item==NULL){
                        printf("Error type 2 at Line %d: Undefined function \"%s\".\n",root->lineno,root->child[0]->val);
                        ret = (Type)malloc(sizeof(struct Type_));
                        ret->kind=ERR;
                    }
                    else{
                        //对普通变量使用“(…)”或“()”（函数调用）操作符
                        printf("Error type 11 at Line %d: \"%s\" is not a function.\n",root->lineno,root->child[0]->val);
                    }
                    
                }
                else{
                    //函数声明
                    ret = item->field->type->u.function.returnType;
                    //检查参数个数和类型
                    Type argType = (Type)malloc(sizeof(struct Type_));
                    strcpy(argType->u.function.name,item->field->name);
                    argType->kind = FUNCTION;
                    argType->u.function.argc = 0;
                    argType->u.function.argv = Args(root->child[2],argType);
                    if(TypeEqual(item->field->type,argType)==0){
                        //类型不匹配
                        printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",root->lineno,root->child[0]->val);
                    }
                }
            }
            else{
                //函数定义
                ret = item->field->type->u.function.returnType;
                //检查参数个数和类型
                Type argType = (Type)malloc(sizeof(struct Type_));
                strcpy(argType->u.function.name,item->field->name);
                argType->kind = FUNCTION;
                argType->u.function.argc = 0;
                argType->u.function.argv = Args(root->child[2],argType);
                
                if(argType->u.function.argc!=item->field->type->u.function.argc){
                    //类型不匹配
                    
                    printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",root->lineno,root->child[0]->val);
                }
                else{
                    FieldList p1 = argType->u.function.argv;
                    FieldList p2 = item->field->type->u.function.argv;
                    while(p1!=NULL&&p2!=NULL){
                        if(TypeEqual(p1->type,p2->type)==0){
                            //类型不匹配
                            //printf("%d\n",argType->u.function.argv->type->kind);
                            printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",root->lineno,root->child[0]->val);
                            break;
                        }
                        p1 = p1->tail;
                        p2 = p2->tail;
                    }
                    if(p1!=NULL||p2!=NULL){
                        //参数个数不匹配
                        printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",root->lineno,root->child[0]->val);
                    }
                }
                //assert(0);
            }
            
            
        }
        else if(strcmp(root->child[0]->name,"Exp")==0){
            //Exp : Exp LB Exp RB
            //数组实现起来有点复杂啊感觉.
            //此处偷懒了，数组越界我们不管
            //TODO: 构造出数组，至于存在与否，我们放到使用的时候再检查吧
            
            Type type = Exp(root->child[0]);
            if(type->kind!=ARRAY){
                //类型不匹配
                
                printf("Error type 10 at Line %d: Not an array.\n",root->lineno);
                ret= (Type)malloc(sizeof(struct Type_));
                ret->kind=ERR;
                //assert(type->kind==ARRAY);
                
            }
            else if(strcmp(root->child[2]->child[0]->name,"INT")!=0){
                //检查下标是否是int型
                Type type1 = Exp(root->child[2]);
                if(type1->kind!=BASIC||type1->u.basic!=0){
                    //类型不匹配
                    //只有int可以进行逻辑运算
                    printf("Error type 12 at Line %d: \"%s\" is not an integer.\n",root->lineno,root->child[2]->child[0]->val);
                    ret= (Type)malloc(sizeof(struct Type_));
                    ret->kind=type1->kind;
                }
                else{
                    //printf("%s\n",root->child[2]->child[0]->val);
                    ret=type->u.array.elem;
                }
                
            }
            else{
                ret=type->u.array.elem;
            }
            
        }
        else{
            assert(0);
        }

    }
    else{
        assert(0);
    }

    if(ret==NULL){
        ret = (Type)malloc(sizeof(struct Type_));
        ret->kind=ERR;
    }
    return ret;

}


/*
if(root->child_num==1){
        //VarList : ParamDec
        type->u.function.argc++;
        return ParamDec(root->child[0]);
    }
    else if(root->child_num==3){
        type->u.function.argc++;
        FieldList tmp1 = ParamDec(root->child[0]);
        FieldList tmp2 = VarList(root->child[2],type);
        tmp1->tail = tmp2;
        return tmp1;
    }else{
        assert(0);
    }
*/

//Args : Exp COMMA Args   
//    | Exp
FieldList Args(struct node* root,Type type){
    //检查参数个数和类型
    assert(root!=NULL);
    //assert(root->child_num==1||root->child_num==3);
    FieldList ret = (FieldList)malloc(sizeof(struct FieldList_));
    type->u.function.argc++;
    ret->lineno = root->lineno;
    ret->tail = NULL;
    ret->type = Exp(root->child[0]);

    strcpy(ret->name,root->child[0]->child[0]->val);

    if(root->child_num==3){
        //Args : Exp COMMA Args
        FieldList tmp = Args(root->child[2],type);
        ret->tail = tmp;
    }
    else{
        assert(root->child_num==1);
    }
    
    return ret;

}


//StmtList : Stmt StmtList
//    | Stmt
//                       
void StmtList(struct node* root){
    assert(root!=NULL);
    //开始进行各种检查...
    assert(root->child_num==1||root->child_num==2);
    if(root->child_num==1){
        //StmtList : Stmt
        Stmt(root->child[0]);
    }
    else if(root->child_num==2){
        //StmtList : Stmt StmtList
        Stmt(root->child[0]);
        StmtList(root->child[1]);
    }
    else{
        assert(0);
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

//VARIABLE
void Stmt(struct node* root){
    assert(root!=NULL);
    assert(root->child_num==1||root->child_num==2||root->child_num==3||root->child_num==5||root->child_num==7);
    if(root->child_num==1){
        //Stmt : CompSt
        assert(strcmp(root->child[0]->name,"CompSt")==0);
        CompSt(root->child[0]);
    }
    else if(root->child_num==2){
        //Stmt : Exp SEMI
        assert(strcmp(root->child[1]->name,"SEMI")==0);
        assert(strcmp(root->child[0]->name,"Exp")==0);
        Exp(root->child[0]);
    }
    else if(root->child_num==3){
        //Stmt : RETURN Exp SEMI
        assert(strcmp(root->child[0]->name,"RETURN")==0);
        assert(strcmp(root->child[1]->name,"Exp")==0);
        assert(strcmp(root->child[2]->name,"SEMI")==0);
        //printf("%d\n",retType->kind);
        //assert(retType->kind==BASIC);
        //printf("ExpKind:%d\n",Exp(root->child[1])->kind);
        if(TypeEqual(Exp(root->child[1]),retType)==0){
            //返回值类型不匹配
            printf("Error type 8 at Line %d: Type mismatched for return.\n",root->lineno);
        }
    }
    else if(root->child_num==5){
        //Stmt : IF LP Exp RP Stmt
        //  | WHILE LP Exp RP Stmt
        //检查Exp是否是int型
        assert(strcmp(root->child[0]->name,"IF")==0||strcmp(root->child[0]->name,"WHILE")==0);
        assert(strcmp(root->child[1]->name,"LP")==0);
        assert(strcmp(root->child[2]->name,"Exp")==0);
        assert(strcmp(root->child[3]->name,"RP")==0);
        assert(strcmp(root->child[4]->name,"Stmt")==0);
        Type type=Exp(root->child[2]);
        if(type->kind!=BASIC||type->u.basic!=0){
            //类型不匹配
            printf("Error type 7 at Line %d: Error type 7 at Line 4: Type mismatched for operands.\n",root->lineno);
        }
        Stmt(root->child[4]);
    }
    else if(root->child_num==7){
        //Stmt : IF LP Exp RP Stmt ELSE Stmt
        assert(strcmp(root->child[0]->name,"IF")==0);
        assert(strcmp(root->child[1]->name,"LP")==0);
        assert(strcmp(root->child[2]->name,"Exp")==0);
        assert(strcmp(root->child[3]->name,"RP")==0);
        assert(strcmp(root->child[4]->name,"Stmt")==0);
        assert(strcmp(root->child[5]->name,"ELSE")==0);
        assert(strcmp(root->child[6]->name,"Stmt")==0);
        Type type=Exp(root->child[2]);
        if(type->kind!=BASIC||type->u.basic!=0){
            //类型不匹配
            printf("Error type 7 at Line %d: Error type 7 at Line 4: Type mismatched for operands.\n",root->lineno);
        }
        Stmt(root->child[4]);
        Stmt(root->child[6]);
        
    }
    else{
        assert(0);
    }

}

//ExtDecList : VarDec             
//    | VarDec COMMA ExtDecList  
FieldList ExtDecList(struct node* root,Type spec){
    assert(root!=NULL);
    assert(root->child_num==1||root->child_num==3);
    FieldList ret = NULL;
    if(root->child_num==1){
        //ExtDecList : VarDec
        ret = VarDec(root->child[0],spec);
    }
    else if(root->child_num==3){
        //ExtDecList : VarDec COMMA ExtDecList
        ret = VarDec(root->child[0],spec);
        FieldList tmp = ExtDecList(root->child[2],spec);
        FieldList p = ret;
        while(p->tail!=NULL){
            p = p->tail;
        }
        p->tail = tmp;
    }
    else{
        assert(0);
    }
    return ret;
}

/*
VarDec : ID             
    | VarDec LB INT RB  
*/

FieldList VarDec(struct node* root,Type spec){
    assert(root!=NULL);
    assert(root->child_num==1||root->child_num==4);
    FieldList ret = (FieldList)malloc(sizeof(struct FieldList_));
    if(root->child_num==1){
        //只是
        strcpy(ret->name,root->child[0]->val);
        ret->lineno = root->lineno;
        //printf("lineno:%d\n",ret->lineno);
        ret->type = spec;
        ret->tail = NULL;
        
    }
    else if(root->child_num==4){
        //VarDec : VarDec LB INT RB
        //数组
        Type type = (Type)malloc(sizeof(struct Type_));
        type->kind = ARRAY;
        type->u.array.elem = spec;
        type->u.array.size = atoi(root->child[2]->val);
        ret = VarDec(root->child[0],type);
    }
    else{
        assert(0);
    }
    return ret;
}



/*
Specifier : TYPE         
    | StructSpecifier    
    ;
*/
Type Specifier(struct node* root){
    
    assert(root!=NULL);
    assert(root->child_num==1);

    Type ret ;
    if(strcmp(root->child[0]->name,"TYPE")==0){
        ret= (Type)malloc(sizeof(struct Type_));
        ret->kind = BASIC;
        if(strcmp(root->child[0]->val,"int")==0){
            ret->u.basic = 0;
        }
        else if(strcmp(root->child[0]->val,"float")==0){
            ret->u.basic = 1;
        }
        else{
            assert(0);
        }
    }
    else if(strcmp(root->child[0]->name,"StructSpecifier")==0){
        ret = StructSpecifier(root->child[0]);

    }
    else{
        assert(0);
    }
    return ret;
}

/*
StructSpecifier : STRUCT OptTag LC DefList RC  
    | STRUCT Tag                                
    ;
OptTag -> ID | e ;
Tag -> ID;
*/
Type StructSpecifier(struct node* root){
    assert(root!=NULL);
    assert(root->child_num==5||root->child_num==2||root->child_num==4);
    Type ret = (Type)malloc(sizeof(struct Type_));
    ret->kind = STRUCTURE;
    
    if(root->child_num==2){
        //StructSpecifier->STRUCT Tag
        //查找符号表，判断是否定义过
        assert(strcmp(root->child[1]->name,"Tag")==0);

        strcpy(ret->u.structure.name,root->child[1]->child[0]->val);
        struct SymbolItem_* item = isDefined(def_struct, ret->u.structure.name);
        if(item==NULL){
            //没有定义过，报错
            printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",root->lineno,ret->u.structure.name);
            ret->u.structure.sf = NULL;
           
        }
        else{
            //已经定义过，返回类型
            ret->u.structure.sf = item->field->type->u.structure.sf;//或许会有问题
            
        }
        return ret;

    }
    else if(root->child_num==5){
        //StructSpecifier -> STRUCT OptTag(ID) LC DefList RC  

        //name为ID
        strcpy(ret->u.structure.name,root->child[1]->child[0]->val);
        //fieldlist
        assert(strcmp(root->child[3]->name,"DefList")==0);
        
        addDepth();
        inStruct ++;
        ret->u.structure.sf = DefList(root->child[3]);
        inStruct --;
        assert(inStruct>=0);
        minusDepth();
        //遍历fieldlist
        
        
    }
    else if(root->child_num==4){
        //StructSpecifier -> STRUCT LC DefList RC  //没有名字的结构体
        if(strcmp(root->child[1]->name,"LC")==0){
            ret->u.structure.name[0]='\0';//没有名字
            assert(strcmp(root->child[2]->name,"DefList")==0);
            addDepth();
            inStruct ++;
            ret->u.structure.sf = DefList(root->child[2]);
            inStruct --;
            assert(inStruct>=0);
            minusDepth();
        }
        else if(strcmp(root->child[1]->name,"OptTag")==0){
            //StructSpecifier -> STRUCT OptTag LC RC  
            //name为ID
            strcpy(ret->u.structure.name,root->child[1]->child[0]->val);
            ret->u.structure.sf = NULL;
        }
        else{
            assert(0);
        }

    }
    else if(root->child_num==3){
        //StructSpecifier -> STRUCT LC RC  
        ret->u.structure.name[0]='\0';//没有名字
        ret->u.structure.sf = NULL;
    }
    else{
        assert(0);
    }
    /*
    
    */
    //将各个变量加入到符号表中
    //目的在于检查结构体成员是否重复定义
    /*这一步也挪到了Dec之中
        FieldList p = ret->u.structure.sf;
        while(p!=NULL){
            struct SymbolItem_* item = (struct SymbolItem_*)malloc(sizeof(struct SymbolItem_));
            item->kind = VARIABLE;
            item->field = (FieldList)malloc(sizeof(struct FieldList_));
            item->field->tail = NULL;
            assert(p->name!=NULL);
            strcpy(item->field->name,p->name);
            item->field->type = p->type;
            item->flag = 0;
            item->field->lineno = p->lineno;
            sfinsert(table, item->field->name, item);
            p=p->tail;
        }
    */
    Type spec = ret;
    
    //将structure加入符号表
    struct SymbolItem_* item = (struct SymbolItem_*)malloc(sizeof(struct SymbolItem_));
    item->kind = STRUCTURE;
    item->field = (FieldList)malloc(sizeof(struct FieldList_));
    item->field->type = spec;
    item->field->tail = NULL;
    item->flag = 0;
    item->field->lineno = root->lineno;
        
    strcpy(item->field->name,spec->u.structure.name);
        
    structinsert(item->field->name, item);
    
    return ret;
}

/*DefList -> Def DefList*/
FieldList DefList(struct node* root){
    assert(root!=NULL);
    assert(root->child_num==1||root->child_num==2);
    assert(strcmp(root->name,"DefList")==0);
    FieldList ret = NULL;
    if(root->child_num==1){
        //DefList -> Def
        ret = Def(root->child[0]);
    }
    else if(root->child_num==2){
        //DefList -> Def DefList
        ret = Def(root->child[0]);
        FieldList temp = DefList(root->child[1]);
        FieldList p = ret;
        while(p->tail!=NULL){
            p = p->tail;
        }
        p->tail = temp;
        
    }
    else{
        assert(0);
    }

    
    return ret;
}


//Def : Specifier DecList SEMI    
FieldList Def(struct node* root){
    //提取定义的类型和变量列表
    assert(root!=NULL);
    assert(root->child_num==3);
    Type spec = Specifier(root->child[0]);
    assert(strcmp(root->child[1]->name,"DecList")==0);
    return DecList(root->child[1],spec);
}

//DecList : Dec
    //  | Dec COMMA DecList 
FieldList DecList(struct node* root,Type type){
    assert(root!=NULL);
    assert(root->child_num==1||root->child_num==3);
    if(root->child_num==1){
        //DecList : Dec
        return Dec(root->child[0],type);
    }
    else if(root->child_num==3){
        //DecList : Dec COMMA DecList
        FieldList tmp1 = Dec(root->child[0],type);
        FieldList tmp2 = DecList(root->child[2],type);
        FieldList p = tmp1;
        while(p->tail!=NULL){
            p = p->tail;
        }
        p->tail = tmp2;
        return tmp1;
    }
    else{
        assert(0);
    }
}

//Dec : VarDec                {$$=NewNode("Dec","",@$.first_line);AddChild(1,$$,$1);}
//      | VarDec ASSIGNOP Exp   {$$=NewNode("Dec","",@$.first_line);AddChild(3,$$,$1,$2,$3);}
//;

FieldList Dec(struct node* root,Type type){
    assert(root!=NULL);
    assert(root->child_num==1||root->child_num==3);
    FieldList ret = VarDec(root->child[0],type);
    if(root->child_num==3){
        assert(strcmp(root->child[1]->name,"ASSIGNOP")==0);
        //检查赋值语句的合法性
        if(inStruct!=0){
            printf("Error type 15 at Line %d: Illegal use of assignment.\n",root->lineno);
        }
        if(TypeEqual(Exp(root->child[2]),type)==0){
            //类型不匹配
            printf("Error type 5 at Line %d: Type mismatched for assignment.\n",root->lineno);
        }
    }

    //改为在此处插入符号表之中
    struct SymbolItem_* item = (struct SymbolItem_*)malloc(sizeof(struct SymbolItem_));
    item->kind = VARIABLE;
    item->field = (FieldList)malloc(sizeof(struct FieldList_));
    item->field->tail = NULL;
    assert(ret->name!=NULL);
    strcpy(item->field->name,ret->name);
    item->field->type = ret->type;
    item->flag = 0;
    item->field->lineno = ret->lineno;
    if(inStruct!=0){
        //在结构体中定义的变量
        sfinsert(table, item->field->name, item);
    }
    else{
        insert(table, item->field->name, item);
    }
    


    return ret;
}



