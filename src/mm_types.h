typedef enum TYPE_KIND
{
    Type_None = 0,
    
    Type_FirstPrimitive,
    Type_Byte = Type_FirstPrimitive,
    Type_Typeid,
    Type_Any,
    
    Type_FirstInteger,
    Type_SoftInt = Type_FirstInteger,
    
    Type_FirstInt,
    Type_Int = Type_FirstInt,
    Type_I8,
    Type_I16,
    Type_I32,
    Type_I64,
    Type_LastInt = Type_I64,
    
    Type_FirstUint,
    Type_Uint = Type_FirstUint,
    Type_U8,
    Type_U16,
    Type_U32,
    Type_U64,
    Type_LastUint = Type_U64,
    Type_LastInteger = Type_LastUint,
    
    Type_FirstFloat,
    Type_SoftFloat = Type_FirstFloat,
    
    Type_Float,
    Type_F32,
    Type_F64,
    Type_LastFloat = Type_F64,
    
    Type_String,
    
    Type_Bool,
    Type_LastPrimitive = Type_Bool,
    
    Type_Array,
    Type_Slice,
    Type_Pointer,
    
    Type_Proc,
    Type_Struct,
    Type_Union,
    Type_Enum,
} TYPE_KIND;

typedef enum CALLING_CONVENTION_KIND
{
    CallingConvention_None = 0,
    
    CALLING_CONVENTION_KIND_COUNT
} CALLING_CONVENTION_KIND;

typedef struct Type_Info
{
    TYPE_KIND kind;
    u32 size;
    
    union
    {
        struct
        {
            Type_ID elem_type;
            u64 size;
        } array;
        
        Type_ID elem_type;
        
        struct
        {
            Symbol_Table parameters;
            Symbol_Table return_types;
            CALLING_CONVENTION_KIND calling_convention;
        } proc;
        
        struct
        {
            Symbol_Table members;
        } structure;
        
        struct
        {
            Type_ID backing_type;
            Symbol_Table members;
        } enumeration;
    };
} Type_Info;

internal inline Type_ID
Type_ArrayOf(Type_ID type, umm size)
{
    Type_ID result = Type_None;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_SliceOf(Type_ID type)
{
    Type_ID result = Type_None;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_PointerTo(Type_ID type)
{
    Type_ID result = Type_None;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline bool
Type_IsInteger(Type_ID type)
{
    return (type >= Type_FirstInteger && type <= Type_LastInteger);
}

internal inline bool
Type_IsUnsignedInteger(Type_ID type)
{
    return (type >= Type_FirstUint && type <= Type_LastUint);
}

internal inline bool
Type_IsSignedInteger(Type_ID type)
{
    return (type >= Type_FirstInt && type <= Type_LastInt);
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
Type_IsString(Type_ID type)
{
    return (type == Type_SoftString || type == Type_String);
}

internal inline bool
Type_IsSoft(Type_ID type)
{
    return (type == Type_SoftInt || type == Type_SoftFloat || type == Type_SoftString);
}

internal inline Type_ID
Type_Harden(Type_ID type)
{
    Type_ID result = Type_None;
    if      (type == Type_SoftInt)    result = Type_Int;
    else if (type == Type_SoftFloat)  result = Type_Float;
    else if (type == Type_SoftString) result = Type_String;
    else INVALID_CODE_PATH;
    
    return result;
}

internal Type_ID
Type_HardenTo(Type_ID src, Type_ID dst, Const_Val val)
{
    if (type == Type_SoftInt)
    {
        NOT_IMPLEMENTED;
    }
    else if (type == Type_SoftFloat)
    {
        NOT_IMPLEMENTED;
    }
    else if (type == Type_SoftString)
    {
        NOT_IMPLEMENTED;
    }
    else INVALID_CODE_PATH;
}

internal inline Type_Info*
Type_InfoOf(Type_ID type)
{
    Type_Info* result = 0;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline bool
Type_IsComposite(Type_ID type)
{
    return (type > Type_LastPrimitive);
}

// NOTE: Type_None is ignored, asking if something, that is
//       not a type, is a primitive type makes no sense
internal inline bool
Type_IsPrimitive(Type_ID type)
{
    return (type <= Type_LastPrimitive);
}

internal inline umm
Type_SizeOf(Type_ID type)
{
    umm size;
    if (Type_IsComposite(type)) size = Type_InfoOf(type)->size;
    else
    {
        ASSERT(!Type_IsSoft(type));
        
        switch (type)
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
        }
        
        return size;
    }