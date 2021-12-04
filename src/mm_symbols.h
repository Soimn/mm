typedef u64 Type_ID;
typedef u64 Symbol_Table;

enum TYPE_KIND
{
    Type_Unresolved = 0,
    Type_Incomplete,
    //Type_Completing,
    
    Type_Array,
    Type_DynamicArray,
    Type_Slice,
    Type_Pointer,
    Type_Range,
    
    Type_Struct,
    Type_Union,
    Type_Enum,
    
    Type_Proc,
    
    Type_String,
    Type_Char,
    Type_Bool,
    Type_Int,
    Type_I8,
    Type_I16,
    Type_I32,
    Type_I64,
    Type_Uint,
    Type_U8,
    Type_U16,
    Type_U32,
    Type_U64,
    Type_Float,
    Type_F32,
    Type_F64,
};

typedef struct Type_Info
{
    Identifier name;
    u32 size;
    u8 alignment;
    
    Type_ID sub_type;
    u32 array_size;
    Symbol_Table symbol_table;
} Type_Info;

enum SYMBOL_KIND
{
    Symbol_Var,
    Symbol_Const,
    Symbol_Package,
    Symbol_Parameter,
    Symbol_ReturnValue,
};

typedef struct Symbol
{
    Symbol* next;
    
    Enum8(SYMBOL_KIND) kind;
    Identifier name;
    
    Type_ID type;
    Package_ID package;
    // Value
} Symbol;