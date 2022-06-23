// depdendency tracking
// type table
// symbol tracking
// checking
// global vs local symbols?
// isolation and api

typedef enum MM_TYPE_KIND
{
    MM_Type_None = 0,
    
    MM_Type_FirstSoft,
    MM_Type_SoftInt = MM_Type_FirstSoft,
    MM_Type_SoftFloat,
    MM_Type_SoftBool,
    MM_Type_LastSoft = MM_Type_SoftBool,
    
    MM_Type_FirstBase,
    MM_Type_FirstInteger = MM_Type_FirstBase,
    MM_Type_FirstInt = MM_Type_FirstInteger,
    MM_Type_Int = MM_Type_FirstInt, // NOTE: register sized
    MM_Type_I8,
    MM_Type_I16,
    MM_Type_I32,
    MM_Type_I64,
    MM_Type_I128,
    MM_Type_LastInt = MM_Type_I128,
    
    MM_TypeFirstUint,
    MM_Type_Uint = MM_TypeFirstUint, // NOTE: register sized
    MM_Type_U8,
    MM_Type_U16,
    MM_Type_U32,
    MM_Type_U64,
    MM_Type_U128,
    MM_Type_LastUint = MM_Type_U128,
    MM_Type_LastInteger = MM_Type_LastUint,
    
    MM_Type_FirstBool,
    MM_Type_Bool = MM_Type_FirstBool, // NOTE: 8-bit
    MM_Type_B8,
    MM_Type_B16,
    MM_Type_B32,
    MM_Type_B64,
    MM_Type_B128,
    MM_Type_LastBool = MM_Type_B128,
    
    MM_Type_FirstFloat,
    MM_Type_Float = MM_Type_FirstFloat, // NOTE: 32-bit
    MM_Type_F16,
    MM_Type_F32,
    MM_Type_F64,
    MM_Type_LastFloat = MM_Type_F64,
    
    MM_Type_String, // NOTE: {size, pointer}, therefore 2x register size
    
    MM_Type_Any,    // NOTE: {type_id, pointer}, therefore 2x register size
    
    MM_Type_Typeid, // NOTE: register sized
    
    MM_Type_FirstWord,
    MM_Type_Word = MM_Type_FirstWord, // NOTE: register sized
    MM_Type_W8,
    MM_Type_W16,
    MM_Type_W32,
    MM_Type_W64,
    MM_Type_W128,
    MM_Type_LastWord = MM_Type_W128,
    MM_Type_LastBase = MM_Type_LastWord,
} MM_TYPE_KIND;

typedef void* MM_Type_ID;

typedef enum MM_SYMBOL_KIND
{
    MM_Symbol_Invalid = 0,
    
    MM_Symbol_Variable,
    MM_Symbol_Constant,
    MM_Symbol_UsingLink,
    MM_Symbol_Parameter,
    MM_Symbol_ReturnValue,
    MM_Symbol_BlockLabel,
    MM_Symbol_StructMember,
    MM_Symbol_UnionMember,
    MM_Symbol_EnumMember,
} MM_SYMBOL_KIND;

typedef struct MM_Symbol
{
    MM_SYMBOL_KIND kind;
    struct MM_Symbol* prev;
    struct MM_Symbol* next;
    
    union
    {
        MM_String name;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
        } variable;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: value
        } constant;
        
        struct
        {
            MM_String name;
            // symbol
        } using_link;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value?
        } parameter;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value?
        } return_value;
        
        struct
        {
            MM_String name;
            // TODO: Reference to the block it is applied to
        } block_label;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value
            // TODO: offset?
        } struct_member;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value
        } union_member;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: value
        } enum_member;
    };
    
    MM_AST* ast;
} MM_Symbol;

typedef struct MM_Symbol_Table
{
    MM_Symbol* symbols;
    MM_Symbol** map;
    MM_u32 map_size;
    MM_u32 symbol_count;
} MM_Symbol_Table;

typedef enum MM_TYPE_INFO_KIND
{
    MM_TypeInfo_Invalid = 0,
    
    MM_TypeInfo_Pointer,
    MM_TypeInfo_Slice,
    MM_TypeInfo_Array,
    
    MM_TypeInfo_Struct,
    MM_TypeInfo_Union,
    MM_TypeInfo_Enum,
    MM_TypeInfo_Proc,
} MM_TYPE_INFO_KIND;

typedef struct MM_Type_Info
{
    MM_TYPE_INFO_KIND kind;
    
    union
    {
        struct MM_Type_Info* elem_type;
        MM_Symbol_Table symbols;
        
        struct
        {
            struct MM_Type_Info* elem_type;
            MM_u64 size;
        } array;
        
        struct
        {
            MM_Symbol_Table symbols;
        } struct_type;
        
        struct
        {
            MM_Symbol_Table symbols;
        } union_type;
        
        struct
        {
            MM_Symbol_Table symbols;
        } enum_type;
        
        struct
        {
            MM_Symbol_Table symbols;
        } proc_type;
    };
} MM_Type_Info;