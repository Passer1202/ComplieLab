#ifndef TABLE_H
#define TABLE_H

#include"common.h"

enum KIND { BASIC, ARRAY, STRUCTURE, FUNCTION, VARIABLE, ERR} ; // kind of the symbol

extern int inStruct; // 0: not in struct, 1: in struct

/*Part A: symbol table*/
#define HASH_SIZE 0x3fff // hash table size
typedef struct Type_ *Type;
typedef struct FieldList_ *FieldList;


typedef struct ARRAY{Type elem; int size;}ARRAYTYPE; // array type

typedef struct STRUCTURE_
{
    char name[MAX_NAME_LEN]; // name of the structure
    FieldList sf; // field list of the structure
}STRUCTYPE;

typedef struct FUNCTION_
{
    char name[MAX_NAME_LEN]; // name of the function
    Type returnType; // return type of the function
    int argc; // number of arguments
    FieldList argv; // field list of the arguments
}FUNCTYPE;

struct Type_
{
    enum KIND kind;
    union{
            int basic; // 0:int, 1:float
            ARRAYTYPE array; // size is the number of elements in the array
            STRUCTYPE structure; // field list of the structure
            FUNCTYPE function; // returnType is the type of the function, argc is the number of arguments, argv is the field list of the arguments
    }u;
};

struct FieldList_
{
    int lineno; // line number of the field
    char name[MAX_NAME_LEN]; // name of the field
    Type type; // type of the field
    FieldList tail; // next field in the list
};

// symbol table entry
struct SymbolItem_
{
    int depth; // current depth of the symbol table
    enum KIND kind; // kind of the symbol
    struct SymbolItem_* next; //next symbol in the same bucket
    struct SymbolItem_* snext; //next symbol in the same stack
    FieldList field; // field list of the symbol
    int flag;       // 0: not defined, 1: defined //函数定义时flag=1，函数声明时flag=0
};

struct SymbolTable_
{
    struct SymbolItem_* head[HASH_SIZE]; // hash table of the symbol table
};

struct Stack_
{
    struct SymbolItem_* head; // hash table of the symbol table
    struct Stack_* next; // next stack in the list
};

extern int cur_depth;

extern struct Stack_* stack;

extern struct SymbolTable_* table; // var symbol table

extern struct SymbolTable_* def_func;//函数定义表
extern struct SymbolTable_* dec_func;//函数声明表
extern struct SymbolTable_* def_struct;//结构体定义表

extern Type retType;

// 创建一个新的fieldlist
FieldList newField(); 

//创建一个栈,用于构建十字链表
struct Stack_* newStack();
//进入一个新的作用域,即增加一个深度
void addDepth();
//离开一个作用域,即减少一个深度
void minusDepth(); 

//检查两个类型是否相同;0:不相同,1:相同
int TypeEqual(Type type1, Type type2); 
//判断结构体是否结构等价;0:不等价,1:等价
int StructureEqual(FieldList structure1, FieldList structure2); 

//创建一个空哈希表
struct SymbolTable_* newTable(); 
//删除哈希表
void deleteTable(struct SymbolTable_* table); 

//往符号表中插入一个表项
void insert(struct SymbolTable_* table, char* name, struct SymbolItem_* item); 
void sfinsert(struct SymbolTable_* table, char* name, struct SymbolItem_* item); 
void decinsert(char* name,struct SymbolItem_* item);
void definsert(char* name,struct SymbolItem_* item);
void structinsert(char* name,struct SymbolItem_* item);
//检查当前域中有没有同名表项
int insertcheck(struct SymbolTable_* table, char* name); 

//从符号表中查找是否有对应表项，没有返回NULL
struct SymbolItem_* isDefined(struct SymbolTable_* table, char* name); 

//计算哈希表索引
unsigned int hash_pjw(char* name);


//DEBUG:打印符号表
void printTable(struct SymbolTable_* table); 

void check_defed();




#endif // TABLE_H