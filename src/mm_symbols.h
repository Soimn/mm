typedef union Constant_Value
{
    u64 uint64;
    f64 float64;
    f32 float32;
    bool boolean;
    Character character;
    String string;
} Constant_Value;

enum SYMBOL_KIND
{
    Symbol_Var,
    Symbol_Const,
    Symbol_Package,
    Symbol_Parameter,
    Symbol_ReturnValue,
    //Symbol_UsingRef,
};

typedef struct Symbol
{
    struct Symbol* next;
    AST_Node* ast;
    
    Enum8(SYMBOL_KIND) kind;
    Identifier name;
    
    Type_ID type;
    Package_ID package;
    Constant_Value const_val;
} Symbol;

enum TYPE_KIND
{
    Type_Unresolved = 0,
    Type_Erroneous,
    
    Type_FirstBaseType,
    Type_String        = Type_FirstBaseType,
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
    Type_LastBaseType = Type_F64,
    
    Type_Array,
    Type_DynamicArray,
    Type_Slice,
    Type_Pointer,
    Type_Range,
    
    Type_Struct,
    Type_Union,
    Type_Enum,
    
    Type_Proc,
};

typedef struct Type_Info
{
    u32 size;
    u8 alignment;
    u8 kind;
    
    Identifier name;
    Type_ID sub_type;
    Symbol_Table symbol_stable;
    u32 array_size;
} Type_Info;