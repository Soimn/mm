enum
{
    Type_NoType = 0,
    
    Type_FirstUntypedType,
    Type_UntypedInt = Type_FirstUntypedType,
    Type_UntypedFloat,
    Type_UntypedBool,
    Type_UntypedString,
    Type_UntypedChar,
    Type_LastUntypedType = Type_UntypedString,
    
    Type_String,
    Type_Cstring,
    Type_Char,
    Type_Any,
    Type_Typeid,
    
    Type_Byte,
    
    Type_FirstBoolean,
    Type_Bool = Type_FirstBoolean,
    Type_B8,
    Type_B16,
    Type_B32,
    Type_B64,
    Type_LastBoolean = Type_B64,
    
    Type_FirstIntegral,
    Type_FirstSigned = Type_FirstIntegral,
    Type_Int = Type_FirstSigned,
    Type_I8,
    Type_I16,
    Type_I32,
    Type_I64,
    Type_LastSigned = Type_I64,
    Type_Uint,
    Type_U8,
    Type_U16,
    Type_U32,
    Type_U64,
    Type_LastIntegral = Type_U64,
    
    Type_FirstFloating,
    Type_Float = Type_FirstFloating,
    Type_F32,
    Type_F64,
    Type_LastFloating = Type_F64,
};

enum TYPE_INFO_KIND
{
    TypeInfo_Primitive = 0,
    
    TypeInfo_Pointer,
    TypeInfo_Array,
    TypeInfo_DynArray,
    TypeInfo_Slice,
    TypeInfo_VarArgSlice,
    
    TypeInfo_Struct,
    TypeInfo_Union,
    TypeInfo_Enum,
    TypeInfo_Proc,
    
    TypeInfo_Range,
};

typedef struct Type_Info
{
    Enum8(TYPE_INFO_KIND) kind;
    
    union
    {
        Type_ID elem_type;
        Type_ID ptr_type;
        
        struct
        {
            u64 size;
            Type_ID elem_type;
        } array;
        
        struct
        {
            Interned_String name;
            Symbol_Table member_table;
            u64 size;
        } structure;
        
        struct
        {
            Interned_String name;
            Symbol_Table member_table;
        } enumeration;
        
        struct
        {
            Interned_String name;
            Symbol_Table param_table;
            Symbol_Table return_table;
        } procedure;
    };
} Type_Info;

internal inline bool
Type_IsTyped(Type_ID type)
{
    return (type < Type_FirstUntypedType || type > Type_LastUntypedType);
}

internal inline bool
Type_IsBoolean(Type_ID type)
{
    return (type == Type_UntypedBool || type >= Type_FirstBoolean && type <= Type_LastBoolean);
}

internal inline bool
Type_IsInteger(Type_ID type)
{
    return (type == Type_UntypedInt || type >= Type_FirstIntegral && type <= Type_LastIntegral);
}

internal inline bool
Type_IsSignedInteger(Type_ID type)
{
    return (type >= Type_FirstSigned && type <= Type_LastSigned);
}

internal inline bool
Type_IsChar(Type_ID type)
{
    return (type == Type_UntypedChar || type == Type_Char);
}

internal inline bool
Type_IsIntegral(Type_ID type)
{
    return (Type_IsInteger(type) || Type_IsChar(type));
}

internal inline bool
Type_IsFloating(Type_ID type)
{
    return (type == Type_UntypedFloat || type >= Type_FirstFloating && type <= Type_LastFloating);
}

internal inline bool
Type_IsNumeric(Type_ID type)
{
    return (Type_IsIntegral(type) || Type_IsFloating(type));
}

internal inline Type_ID
Type_ToDefTyped(Type_ID type)
{
    Type_ID result = type;
    
    if (type == Type_UntypedInt)    result = Type_Int;
    if (type == Type_UntypedFloat)  result = Type_Float;
    if (type == Type_UntypedBool)   result = Type_Bool;
    if (type == Type_UntypedString) result = Type_String;
    if (type == Type_UntypedChar)   result = Type_Char;
    
    return result;
}

internal Type_Info*
MM_TypeInfoOf(Type_ID type)
{
    Type_Info* info = 0;
    
    NOT_IMPLEMENTED;
    
    return info;
}

internal inline umm
Type_Sizeof(Type_ID type)
{
    umm result = 0;
    
    Type_Info* info = MM_TypeInfoOf(type);
    
    if (info == 0)
    {
        switch (type)
        {
            case Type_Byte:
            case Type_Bool:
            case Type_B8:
            case Type_I8:
            case Type_U8:
            result = 1;
            break;
            
            case Type_B16:
            case Type_I16:
            case Type_U16:
            result = 2;
            break;
            
            case Type_Char:
            case Type_Typeid:
            case Type_B32:
            case Type_I32:
            case Type_U32:
            case Type_F32:
            result = 4;
            break;
            
            case Type_Cstring:
            case Type_B64:
            case Type_Int:
            case Type_I64:
            case Type_Uint:
            case Type_U64:
            case Type_Float:
            case Type_F64:
            result = 8;
            break;
            
            case Type_String:
            case Type_Any:
            result = 16;
            break;
            
            INVALID_DEFAULT_CASE;
        }
    }
    
    else
    {
        switch (info->kind)
        {
            case TypeInfo_Pointer:
            case TypeInfo_Proc:
            result = 8;
            break;
            
            case TypeInfo_Slice:
            result = 16;
            break;
            
            case TypeInfo_DynArray:
            result = 24;
            break;
            
            case TypeInfo_Array:
            result = info->array.size * Type_Sizeof(info->array.elem_type);
            break;
            
            case TypeInfo_Enum:
            result = Type_Sizeof(info->elem_type);
            break;
            
            case TypeInfo_Struct:
            case TypeInfo_Union:
            result = info->structure.size;
            break;
            
            INVALID_DEFAULT_CASE;
        }
    }
    
    return result;
}

internal bool
Type_IsImplicitlyCastableTo(Type_ID src, Type_ID dst)
{
    bool result = false;
    
    if      (src == dst)                                                                      result = true;
    else if (dst == Type_Any)                                                                 result = true;
    else if (Type_ToDefTyped(src) == dst || src == Type_UntypedString && dst == Type_Cstring) result = true;
    else if (src == Type_UntypedBool && (Type_IsBoolean(dst) || Type_IsNumeric(dst)))         result = true;
    else
    {
        Type_Info* src_info = MM_TypeInfoOf(src);
        Type_Info* dst_info = MM_TypeInfoOf(dst);
        
        if (src_info != 0 && dst_info != 0)
        {
            if (src_info->kind == TypeInfo_Pointer && dst_info->kind == TypeInfo_Pointer &&
                (src_info->ptr_type == Type_Byte || dst_info->ptr_type == Type_Byte))
            {
                result = true;
            }
        }
        
        else if (src == Type_UntypedString && dst_info != 0 && dst_info->kind == TypeInfo_Slice &&
                 (dst_info->elem_type == Type_Char || dst_info->elem_type == Type_Byte))
        {
            result = true;
        }
        
    }
    
    return result;
}

internal bool
Type_IsCastableTo(Type_ID src, Type_ID dst)
{
    bool result = false;
    
    if (Type_IsImplicitlyCastableTo(src, dst)) result = true;
    else if ((Type_IsNumeric(src) || Type_IsChar(src) || Type_IsBoolean(src) || src == Type_Typeid || src == Type_Byte) &&
             (Type_IsNumeric(dst) || Type_IsChar(dst) || Type_IsBoolean(dst) || dst == Type_Byte))
    {
        result = true;
    }
    
    else
    {
        Type_Info* src_info = MM_TypeInfoOf(src);
        Type_Info* dst_info = MM_TypeInfoOf(dst);
        
        if (src_info != 0 && dst_info != 0)
        {
            if (src_info->kind == TypeInfo_Pointer && dst_info->kind == TypeInfo_Pointer) result = true;
        }
    }
    
    return result;
}

internal inline Type_ID
Type_ClosedRangeOf(Type_ID type)
{
    Type_ID result = Type_NoType;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_HalfOpenRangeOf(Type_ID type)
{
    Type_ID result = Type_NoType;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_ArrayOf(Type_ID elem_type, u64 size)
{
    Type_ID result = Type_NoType;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_PointerTo(Type_ID type)
{
    Type_ID result = Type_NoType;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_SliceOf(Type_ID type)
{
    Type_ID result = Type_NoType;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_DynArrayOf(Type_ID type)
{
    Type_ID result = Type_NoType;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal inline Type_ID
Type_VarArgSliceOf(Type_ID type)
{
    Type_ID result = Type_NoType;
    
    NOT_IMPLEMENTED;
    
    return result;
}