#include"table.h"

struct SymbolTable_* newTable(){
    struct SymbolTable_* ret = (struct SymbolTable_*)malloc(sizeof(struct SymbolTable_));
    for(int i=0;i<HASH_SIZE;i++){
        ret->head[i]=NULL;
    }
    return ret;
}

void deleteTable(struct SymbolTable_* table){
    for(int i=0;i<HASH_SIZE;i++){
        struct SymbolItem_* p=table->head[i];
        while(p!=NULL){
            struct SymbolItem_* q=p;
            p=p->next;
            free(q);
        }
    }
    free(table);
}


int TypeEqual(Type type1, Type type2){

    assert(type1!=NULL&&type2!=NULL);
    
    Type p1=type1;
    Type p2=type2;

    if(type1->kind!=type2->kind){
        return 0; 
    }
    switch(type1->kind){
        case BASIC:
            return type1->u.basic==type2->u.basic;
        case ARRAY://基类型和维度相同就好了
            
            while(p1->kind==ARRAY&&p2->kind==ARRAY){
                p1=p1->u.array.elem;
                p2=p2->u.array.elem;
            }
            if(p1->kind!=p2->kind){
                return 0;
            }
        case STRUCTURE:
            return StructureEqual(type1->u.structure.sf,type2->u.structure.sf);
        case FUNCTION:
            if(type1->u.function.argc!=type2->u.function.argc){
                return 0;
            }
            if(!TypeEqual(type1->u.function.returnType,type2->u.function.returnType)){
                return 0;
            }
            FieldList p=type1->u.function.argv;
            FieldList q=type2->u.function.argv;
            while(p!=NULL&&q!=NULL){
                if(!TypeEqual(p->type,q->type)){
                    return 0;
                }
                p=p->tail;
                q=q->tail;
            }
            return p==NULL&&q==NULL;
        default:
            assert(0);
    }
}

struct Stack_* newStack(){
    struct Stack_* ret=(struct Stack_*)malloc(sizeof(struct Stack_));
    ret->head=NULL;
    ret->next=NULL;
    return ret;
}

void addDepth(){
    struct Stack_* p=newStack();
    p->next=stack;
    stack=p;
    cur_depth++;
}

void minusDepth(){
    struct SymbolItem_* p=stack->head;
    //printf("minusDepth\n");
    
    
    while(p!=NULL){
        struct SymbolItem_* q=p;
        //printf("%x\n",q);
        //从符号表中删除q
        //printf("delete %s\n",q->field->name);
        int index=hash_pjw(q->field->name);
        //printf("index:%d\n",index);
        
        struct SymbolItem_* pre=table->head[index];
        //printf("%x\n",pre);
        if(pre==q){
            
            table->head[index]=q->next;
            
            
        }
        else{
            
            while(pre->next!=NULL&&pre->next!=q){
                pre=pre->next;
            }
            if(pre->next==NULL){
                //error: not found
                return;
            }
            pre->next=q->next;
        }
        p=p->snext;
        
        free(q);
    }
    struct Stack_* temp=stack;
    
    stack=stack->next;

    free(temp);
    cur_depth--;
    assert(cur_depth>=0);
    
}

FieldList newField(){
    //名字和类型都还没写
    FieldList field=(FieldList)malloc(sizeof(struct FieldList_));
    field->tail=NULL;
    return field;
}

void structinsert(char* name,struct SymbolItem_* item){
    
    if(name[0]!='\0'){

        int index=hash_pjw(name);
    
        struct SymbolItem_* p=def_struct->head[index];
        while(p!=NULL){
            if(strcmp(p->field->name,name)==0){
                printf("Error type 16 at Line %d: Redefined structure \"%s\".\n",item->field->lineno,name);
                return;
            }
            p=p->next;
        }

        int check=insertcheck(table,name);//0:不存在，1:存在
        if(check==1){
            printf("Error type 16 at Line %d: Redefined structure \"%s\".\n",item->field->lineno,name);
            return;
        } 
    }
        
    
    

    unsigned int index=hash_pjw(name);
    //printf("index:%d\n",index);
    
    item->next=def_struct->head[index];
    
    def_struct->head[index]=item;

}

void decinsert(char* name,struct SymbolItem_* item){
    //插入声明表...
    //检查有没有冲突的声明.
    int index=hash_pjw(name);
    
    struct SymbolItem_* p=dec_func->head[index];
    while(p!=NULL){
        if(strcmp(p->field->name,name)==0){
            //找到了同名声明
            if(TypeEqual(p->field->type,item->field->type)==0){
                //出现冲突,错误类型19
                printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",item->field->lineno,name);
            }
            
            return;
        }
        p=p->next;
    }
    
    //检查有没有冲突的定义.
    p=def_func->head[index];
    while(p!=NULL){
        if(strcmp(p->field->name,name)==0){
            //找到了同名定义
            if(TypeEqual(p->field->type,item->field->type)==0){
                //出现冲突,错误类型19
                printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",item->field->lineno,name);
            }
            return;
        }
        p=p->next;
    }
    
    //插入声明表之中...

    item->next=table->head[index];
    dec_func->head[index]=item;
    

}

void definsert(char* name,struct SymbolItem_* item){
    //插入定义表...
    
    int index=hash_pjw(name);
    struct SymbolItem_* p=def_func->head[index];

     //检查有没有冲突的定义.
     
    while(p!=NULL){
         if(strcmp(p->field->name,name)==0){
             //找到了同名定义
             printf("Error type 4 at Line %d: Redefined function \"%s\".\n",item->field->lineno,name);
             return;
         }
         p=p->next;
    }

    p=dec_func->head[index];
    while(p!=NULL){
        if(strcmp(p->field->name,name)==0){
            //找到了同名声明
            if(TypeEqual(p->field->type,item->field->type)==0){
                //出现冲突,错误类型19
                printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",item->field->lineno,name);
            }
        }
        p=p->next;
    }
    
   
    
    //插入声明表之中...
    //定义如果和声明冲突就定义优先好了...

    item->next=table->head[index];
    def_func->head[index]=item;

}


void insert(struct SymbolTable_* table,char* name,struct SymbolItem_* item){

    unsigned int index=hash_pjw(name);
    
    struct SymbolItem_* p=def_struct->head[index];
    while(p!=NULL){
        if(strcmp(p->field->name,name)==0){
            printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",item->field->lineno,name);
            return;
        }
        p=p->next;
    }

    item->depth=cur_depth;
    
    if(name[0]!='\0'){
        int check=insertcheck(table,name);//0:不存在，1:存在
        if(check==1){
                //error: variable or structure or function redefined
            if(item->kind==VARIABLE){
                    //printf("%s\n",item->field->lineno);
                printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",item->field->lineno,name);
            }
            else if(item->kind==STRUCTURE){
                printf("Error type 16 at Line %d: Redefined structure \"%s\".\n",item->field->lineno,name);
            }
            else if(item->kind==FUNCTION){
                printf("Error type 4 at Line %d: Redefined function \"%s\".\n",item->field->lineno,name);
            }
            else{
                assert(0);
            }
            return;
        } 
    }
        
    
    

    //unsigned int index=hash_pjw(name);
    //printf("index:%d\n",index);
    
    item->next=table->head[index];
    
    table->head[index]=item;
    
    item->snext=stack->head;
    stack->head=item;
    //printf("%x\n",stack->head);
    //printf("table->head[index]\n",item);
    if(DEBUG){
        printf("insert %s\n",name);
        printTable(table);
    }
}


void sfinsert(struct SymbolTable_* table,char* name,struct SymbolItem_* item){
    
    item->depth=cur_depth;
    if(item->kind!=FUNCTION ||(item->kind==FUNCTION&&item->flag==1)){
        if(name[0]!='\0'){
            int check=insertcheck(table,name);//0:不存在，1:存在
            if(check==1){
                //error: variable or structure or function redefined
                //Error type 15 at Line 4: Redefined field "x".
                printf("Error type 15 at Line %d: Redefined field \"%s\".\n",item->field->lineno,name);
                return;
            } 
        }
        
    }
    

    unsigned int index=hash_pjw(name);
    //printf("index:%d\n",index);
    
    item->next=table->head[index];
    
    table->head[index]=item;
    
    item->snext=stack->head;
    stack->head=item;
    //printf("%x\n",stack->head);
    //printf("table->head[index]\n",item);
    if(DEBUG){
        printf("insert %s\n",name);
        printTable(table);
    }
}

int insertcheck(struct SymbolTable_* table, char* name){

    int index=hash_pjw(name);
    struct SymbolItem_* p=table->head[index];
    while(p!=NULL){
        if(strcmp(p->field->name,name)==0){
            if(p->depth==cur_depth){
                return 1;
            }
            else{
                return 0;
            }
        }
        p=p->next;
    }
    return 0;
    //检查当前域符号表中有无同名表项
    //直接找栈顶就是了.
}


struct SymbolItem_* isDefined(struct SymbolTable_* table, char* name){
    //所有域中的第一个表项
    int index=hash_pjw(name);
    struct SymbolItem_* p=table->head[index];
    while(p!=NULL){
        if(strcmp(p->field->name,name)==0){
            return p;
        }
        p=p->next;
    }
    //没有找到，返回NULL
    return NULL;
}

int StructureEqual(FieldList structure1, FieldList structure2){
    //依次比较两个结构体的sf
    FieldList p1=structure1;
    FieldList p2=structure2;

    while(p1!=NULL&&p2!=NULL){
        if(!TypeEqual(p1->type,p2->type)){
            return 0;
        }
        p1=p1->tail;
        p2=p2->tail;
    }
    if(p1!=NULL||p2!=NULL){
        return 0;
    }
    return 1;
}

unsigned int hash_pjw(char* name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~HASH_SIZE) val = (val ^ (i >> 12)) & HASH_SIZE;
    }
    return val;
}

void printTable(struct SymbolTable_* table){
    printf("=====================\n");
    for(int i=0;i<HASH_SIZE;i++){
        if(table->head[i]==NULL) continue;
        struct SymbolItem_* p=table->head[i];
        while(p!=NULL){
            printf("%s ",p->field->name);
            p=p->next;
        }
        printf("\n");
    }
}

void check_defed(){
    //检查有没有未定义的声明
    for(int i=0;i<HASH_SIZE;i++){
        struct SymbolItem_* p=dec_func->head[i];
        while(p!=NULL){
            //定义表也在head[i]
            struct SymbolItem_* q=def_func->head[i];
            while(q!=NULL){
                if(strcmp(p->field->name,q->field->name)==0){
                    //找到了同名定义
                    break;
                }
                q=q->next;
            }
            if(q==NULL){
                //没有找到同名定义
                printf("Error type 18 at Line %d: Undefined function \"%s\".\n",p->field->lineno,p->field->name);
            }
            p=p->next;
        }
    }
}