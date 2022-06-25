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