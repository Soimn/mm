typedef enum TYPE_KIND
{
    Type_None = 0,
    
    Type_FirstPrimitive,
    Type_Byte = Type_FirstPrimitive,
    Type_Any,
    Type_Typeid,
    
    Type_FirstInteger,
    Type_SoftInt = Type_FirstInteger,
    
    Type_FirstInt,
    Type_Int = Type_FirstInt,
    Type_I8,
    Type_I16,
    Type_I32,
    Type_I64,
    Type_I128,
    Type_LastInt = Type_I128,
    
    Type_FirstUint,
    Type_Uint = Type_FirstUint,
    Type_U8,
    Type_U16,
    Type_U32,
    Type_U64,
    Type_U128,
    Type_LastUint = Type_U128,
    Type_LastInteger = Type_LastUint,
    
    Type_FirstFloat,
    Type_SoftFloat = Type_FirstFloat,
    
    Type_Float,
    Type_F16,
    Type_F32,
    Type_F64,
    Type_LastFloat = Type_F64,
    
    Type_FirstBool,
    Type_SoftBool = Type_FirstBool,
    
    Type_Bool,
    Type_B8,
    Type_B16,
    Type_B32,
    Type_B64,
    Type_B128,
    Type_LastBool = Type_B128,
    Type_LastPrimitive = Type_LastBool,
    
    Type_String,
    
    Type_Array,
    Type_Slice,
    Type_Pointer,
    
    Type_Proc,
    Type_Struct,
    Type_Union,
    Type_Enum,
} TYPE_KIND;

typedef enum CALL_CONV_KIND
{
    CALL_CONV_KIND_COUNT
} CALL_CONV_KIND;

typedef struct Type_Info
{
    TYPE_KIND kind;
    
    union
    {
        struct
        {
            u64 size;
            Type_ID elem_type;
        } array;
        
        Type_ID elem_type;
        
        struct
        {
            // TODO: Polymorphism with type specialization
            // Type_Table* poly_type_table ?
            Symbol_Table* parameters;
            Symbol_Table* return_values;
            CALL_CONV_KIND call_conv;
        } proc;
        
        struct
        {
            Symbol_Table* members;
        } structure;
        
        struct
        {
            Type_ID underlying_type;
            Symbol_Table* members;
        } enumeration;
    };
} Type_Info;

typedef struct Type_Table
{
    bool _;
} Type_Table;

internal Type_Info*
Type_InfoOf(Type_ID type)
{
    Type_Info* result = 0;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal bool
Type_IsInteger(Type_ID type)
{
    return (type >= Type_FirstInteger && type <= Type_LastInteger);
}

internal bool
Type_IsSignedInteger(Type_ID type)
{
    return (type >= Type_FirstInt && type <= Type_LastInt);
}

internal bool
Type_IsUnsignedInteger(Type_ID type)
{
    return (type >= Type_FirstUint && type <= Type_LastUint);
}

internal bool
Type_IsFloat(Type_ID type)
{
    return (type >= Type_FirstFloat && type <= Type_LastFloat);
}

internal bool
Type_IsBool(Type_ID type)
{
    return (type >= Type_FirstBool && type <= Type_LastBool);
}

internal Type_ID
Type_PointerTo(Type_ID type)
{
    Type_ID result = Type_None;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Type_ID
Type_StripPointer(Type_ID type)
{
    Type_Info* type_info = Type_InfoOf(type);
    ASSERT(type_info != 0 && type_info->kind == Type_Pointer);
    
    return type_info->elem_type;
}

internal Type_ID
Type_SliceOf(Type_ID type)
{
    Type_ID result = Type_None;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Type_ID
Type_StripSlice(Type_ID type)
{
    Type_Info* type_info = Type_InfoOf(type);
    ASSERT(type_info != 0 && type_info->kind == Type_Slice);
    
    return type_info->elem_type;
}

internal Type_ID
Type_ArrayOf(Type_ID type, u64 size)
{
    Type_ID result = Type_None;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Type_ID
Type_StripArray(Type_ID type)
{
    Type_Info* type_info = Type_InfoOf(type);
    ASSERT(type_info != 0 && type_info->kind == Type_Array);
    
    return type_info->array.elem_type;
}

internal bool
Type_Exists(Type_ID type)
{
    bool result = false;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal bool
Type_IsSoft(Type_ID type)
{
    return (type == Type_SoftInt   ||
            type == Type_SoftFloat ||
            type == Type_SoftBool);
}