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
    struct Symbol* next;
    
    Enum8(SYMBOL_KIND) kind;
    Identifier name;
    
    Type_ID type;
    Package_ID package;
    u64 constant_value;
} Symbol;

enum TYPE_KIND
{
    Type_Unresolved = 0,
    Type_Incomplete,
    //Type_Completing,
    Type_ResolvedThreshold,
    
    Type_FirstBaseType = Type_ResolvedThreshold,
    Type_FirstUntyped  = Type_FirstBaseType,
    Type_UntypedString = Type_FirstUntyped,
    Type_UntypedChar,
    Type_UntypedBool,
    Type_UntypedInt,
    Type_UntypedUint,
    Type_UntypedFloat,
    Type_LastUntyped = Type_UntypedFloat,
    
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