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
    
    Type_String,
    Type_LastPrimitive = Type_String,
    
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
    
    u32 size;
    
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
            struct Symbol_Table* parameters;
            struct Symbol_Table* return_values;
            CALL_CONV_KIND call_conv;
        } proc;
        
        struct
        {
            struct Symbol_Table* members;
        } structure;
        
        struct
        {
            Type_ID underlying_type;
            struct Symbol_Table* members;
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

internal inline bool
Type_IsInteger(Type_ID type)
{
    return (type >= Type_FirstInteger && type <= Type_LastInteger);
}

internal inline bool
Type_IsSignedInteger(Type_ID type)
{
    return (type >= Type_FirstInt && type <= Type_LastInt);
}

internal inline bool
Type_IsUnsignedInteger(Type_ID type)
{
    return (type >= Type_FirstUint && type <= Type_LastUint);
}

internal inline bool
Type_IsFloat(Type_ID type)
{
    return (type >= Type_FirstFloat && type <= Type_LastFloat);
}

internal inline bool
Type_IsNumeric(Type_ID type)
{
    return (Type_IsInteger(type) || Type_IsFloat(type));
}

internal inline bool
Type_IsBoolean(Type_ID type)
{
    return (type >= Type_FirstBool && type <= Type_LastBool);
}

internal inline bool
Type_IsPointer(Type_ID type)
{
    Type_Info* type_info = Type_InfoOf(type);
    
    return (type_info != 0 && type_info->kind == Type_Pointer);
}

internal inline bool
Type_IsArray(Type_ID type)
{
    Type_Info* type_info = Type_InfoOf(type);
    
    return (type_info != 0 && type_info->kind == Type_Array);
}

internal inline bool
Type_IsSlice(Type_ID type)
{
    Type_Info* type_info = Type_InfoOf(type);
    
    return (type_info != 0 && type_info->kind == Type_Slice);
}

internal inline Type_ID
Type_PointerTo(Type_ID type)
{
    Type_ID result = Type_None;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_StripPointer(Type_ID type)
{
    Type_Info* type_info = Type_InfoOf(type);
    ASSERT(type_info != 0 && type_info->kind == Type_Pointer);
    
    return type_info->elem_type;
}

internal inline Type_ID
Type_SliceOf(Type_ID type)
{
    Type_ID result = Type_None;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_StripSlice(Type_ID type)
{
    Type_Info* type_info = Type_InfoOf(type);
    ASSERT(type_info != 0 && type_info->kind == Type_Slice);
    
    return type_info->elem_type;
}

internal inline Type_ID
Type_ArrayOf(Type_ID type, u64 size)
{
    Type_ID result = Type_None;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_StripArray(Type_ID type)
{
    Type_Info* type_info = Type_InfoOf(type);
    ASSERT(type_info != 0 && type_info->kind == Type_Array);
    
    return type_info->array.elem_type;
}

internal inline bool
Type_Exists(Type_ID type)
{
    bool result = false;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline bool
Type_IsSoft(Type_ID type)
{
    return (type == Type_SoftInt   ||
            type == Type_SoftFloat ||
            type == Type_SoftBool);
}

internal inline bool
Type_IsPrimitive(Type_ID type)
{
    return (type >= Type_FirstPrimitive && type <= Type_LastPrimitive);
}

internal inline imm
Type_Sizeof(Type_ID type)
{
    imm size = 0;
    
    if (!Type_IsPrimitive(type)) size = Type_InfoOf(type)->size;
    else switch (type)
    {
        case Type_Byte:
        case Type_I8:
        case Type_U8:
        case Type_Bool:
        size = 1;
        break;
        
        case Type_I16:
        case Type_U16:
        size = 2;
        break;
        
        case Type_I32:
        case Type_U32:
        case Type_F32:
        case Type_Typeid:
        size = 4;
        break;
        
        case Type_Int:
        case Type_I64:
        case Type_Uint:
        case Type_U64:
        case Type_Float:
        case Type_F64:
        size = 8;
        break;
        
        case Type_Any:
        case Type_String:
        size = 16;
        break;
        
        case Type_SoftInt:   size = -32; break;
        case Type_SoftFloat: size = -32; break;
        case Type_SoftBool:  size = -1;  break;
        
        INVALID_DEFAULT_CASE;
    }
    
    return size;
}