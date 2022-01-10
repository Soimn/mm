enum
{
    Type_Nil = 0,
    
    Type_FirstUntypedType,
    Type_UntypedInt = Type_FirstUntypedType,
    Type_UntypedFloat,
    Type_UntypedBool,
    Type_UntypedString,
    Type_UntypedChar,
    Type_LastUntypedType = Type_UntypedString,
    
    Type_Bool,
    
    Type_String,
    Type_Cstring,
    Type_Char,
    Type_Any,
    Type_Typeid,
    
    Type_Byte,
    
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
    Type_F16,
    Type_F32,
    Type_F64,
    Type_LastFloating = Type_F64,
};

enum TYPE_INFO_KIND
{
    TypeInfo_Primitive = 0,
    
    TypeInfo_Array,
    TypeInfo_Pointer,
    TypeInfo_DynArray,
    TypeInfo_Slice,
    
    TypeInfo_Struct,
    TypeInfo_Union,
    TypeInfo_Enum,
    TypeInfo_Proc,
    
    TypeInfo_Alias,
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
        
        struct
        {
            Interned_String name;
            Type_ID aliased_type;
            Type_ID type;
        } alias;
    };
} Type_Info;

typedef union Range
{
    struct
    {
        u64 unsigned_start;
        u64 unsigned_end;
    };
    
    struct
    {
        i64 signed_start;
        i64 signed_end;
    };
} Range;

typedef struct Const_Val
{
    Type_ID type;
    Enum8(TYPE_INFO_KIND) type_kind; // NOTE: chached from Type_Info lookup
    
    union
    {
        Big_Int untyped_int;
        Big_Float untyped_float;
        bool boolean;
        Interned_String string;
        Character character;
        Type_ID type_id;
        i64 signed_int;
        u64 unsigned_int;
        f64 floating;
        
        Range range;
        
        struct Const_Val* any;
        struct Const_Val* enum_value;
        struct Const_Val* pointer;
        
        struct Const_Val* array;
        struct Const_Val* members;
    };
} Const_Val;

enum SYMBOL_KIND
{
    Symbol_Var,
    Symbol_Const,
    Symbol_UsingLink,
};

enum SYMBOL_STATE
{
    SymbolState_Unresolved = 0,
    SymbolState_Resolving,
    SymbolState_Resolved,
};

typedef struct Symbol
{
    struct Symbol* next;
    
    Interned_String name;
    Enum8(SYMBOL_KIND) kind;
    Enum8(SYMBOL_STATE) state;
    
    union
    {
        struct
        {
            Type_ID type;
            Const_Val val;
            AST_Node* ast;
        };
        
        struct
        {
            struct Symbol* used_symbol;
            //Scope_Index active_threshold;
        } using_link;
    };
} Symbol;


internal Symbol*
Symbol_Get(Symbol_Table table, Interned_String name)
{
    Symbol* result = 0;
    
    for (Symbol* scan = table; scan != 0; scan = scan->next)
    {
        if (scan->name == name)
        {
            result = scan;
            break;
        }
    }
    
    return result;
}

internal Symbol*
Symbol_Add(Memory_Arena* arena, Symbol_Table* table, Interned_String name, Enum8(SYMBOL_KIND) kind)
{
    Symbol* sym = Arena_PushSize(arena, sizeof(Symbol), ALIGNOF(Symbol));
    ZeroStruct(sym);
    sym->name = name;
    sym->kind = kind;
    
    Symbol** next = table;
    for (; *next != 0; next = &(*next)->next);
    
    *next = sym;
    
    return sym;
}

internal Symbol*
MM_GetSymbol(Interned_String name)
{
    u32 hash = String_HashOfInternedString(name);
    
    Symbol* entry = MM.global_symbol_table[hash % ARRAY_SIZE(MM.global_symbol_table)];
    
    for (; entry != 0; entry = entry->next)
    {
        if (entry->name == name) break;
    }
    
    return entry;
}

internal Symbol*
MM_AddSymbol(Interned_String name, Enum8(SYMBOL_KIND) kind)
{
    Symbol* sym = 0;
    
    if (!MM_GetSymbol(name))
    {
        u32 hash = String_HashOfInternedString(name);
        
        Symbol** next = &MM.global_symbol_table[hash % ARRAY_SIZE(MM.global_symbol_table)];
        
        for (; *next != 0; next = &(*next)->next);
        
        sym = Arena_PushSize(&MM.misc_arena, sizeof(Symbol), ALIGNOF(Symbol));
        ZeroStruct(sym);
        sym->name = name;
        sym->kind = kind;
        
        *next = sym;
    }
    
    return sym;
}

internal Type_Info*
MM_TypeInfoOf(Type_ID type)
{
    Type_Info* info = 0;
    
    NOT_IMPLEMENTED;
    
    return info;
}

internal Type_ID
Type_PointerTo(Type_ID ptr_type)
{
    Type_ID type = Type_Nil;
    
    NOT_IMPLEMENTED;
    
    return type;
}

internal Type_ID
Type_SliceOf(Type_ID elem_type)
{
    Type_ID type = Type_Nil;
    
    NOT_IMPLEMENTED;
    
    return type;
}

internal Type_ID
Type_DynArrayOf(Type_ID elem_type)
{
    Type_ID type = Type_Nil;
    
    NOT_IMPLEMENTED;
    
    return type;
}

internal Type_ID
Type_ArrayOf(Type_ID elem_type, umm size)
{
    Type_ID type = Type_Nil;
    
    NOT_IMPLEMENTED;
    
    return type;
}

internal Type_ID
Type_AliasTo(Type_ID aliased_type, Interned_String name)
{
    Type_ID type = Type_Nil;
    
    NOT_IMPLEMENTED;
    
    return type;
}

internal Type_ID
Type_RangeOf(Type_ID elem_type, bool is_half_open)
{
    Type_ID type = Type_Nil;
    
    NOT_IMPLEMENTED;
    
    return type;
}

internal inline bool
Type_IsTyped(Type_ID type)
{
    return (type < Type_FirstUntypedType || type > Type_LastUntypedType);
}

internal inline bool
Type_IsBoolean(Type_ID type)
{
    return (type == Type_UntypedBool || type == Type_Bool);
}

internal inline bool
Type_IsIntegral(Type_ID type)
{
    return (type == Type_UntypedInt || type >= Type_FirstIntegral && type <= Type_LastIntegral);
}

internal inline bool
Type_IsSigned(Type_ID type)
{
    return (type >= Type_FirstSigned && type <= Type_LastSigned);
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

internal inline bool
Type_IsChar(Type_ID type)
{
    return (type == Type_UntypedChar || type == Type_Char);
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

internal inline umm
Type_Sizeof(Type_ID type)
{
    umm result = 0;
    
    Type_Info* info = MM_TypeInfoOf(type);
    
    if (info == 0)
    {
        switch (type)
        {
            case Type_Bool:
            case Type_Byte:
            case Type_I8:
            case Type_U8:
            result = 1;
            break;
            
            case Type_I16:
            case Type_U16:
            case Type_F16:
            result = 2;
            break;
            
            case Type_Char:
            case Type_Typeid:
            case Type_I32:
            case Type_U32:
            case Type_F32:
            result = 4;
            break;
            
            case Type_Cstring:
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
            
            case TypeInfo_Alias:
            result = Type_Sizeof(info->alias.type);
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

internal inline Type_ID
Type_StripAlias(Type_ID type)
{
    Type_ID result = type;
    
    Type_Info* info = MM_TypeInfoOf(type);
    if (info != 0 && info->kind == TypeInfo_Alias)
    {
        result = info->alias.type;
    }
    
    return result;
}

internal bool
Type_IsImplicitlyCastableTo(Type_ID src, Type_ID dst)
{
    bool result = false;
    
    if      (src == dst)                                                    result = true;
    else if (dst == Type_Any)                                               result = true;
    else if (Type_ToDefTyped(src) == dst)                                   result = true;
    else if (src == Type_UntypedString && dst == Type_Cstring)              result = true;
    else if (src == Type_Byte && (dst == Type_I8 || dst == Type_U8))        result = true;
    else if (!Type_IsTyped(src) && Type_IsNumeric(src) && dst == Type_Bool) result = true;
    else if (src == Type_UntypedChar)
    {
        if (Type_IsTyped(dst) && (Type_IsFloating(dst) || Type_IsIntegral(dst) && Type_Sizeof(dst) >= 4))
        {
            result = true;
        }
    }
    
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
    }
    
    return result;
}

internal bool
Type_IsCastableTo(Type_ID src, Type_ID dst)
{
    bool result = false;
    
    Type_ID original_src = src;
    Type_ID original_dst = dst;
    src = Type_StripAlias(src);
    dst = Type_StripAlias(dst);
    
    if (Type_IsImplicitlyCastableTo(original_src, original_dst)) result = true;
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

internal umm
ConstVal_Sizeof(Const_Val val)
{
    umm result = 0;
    
    if (Type_IsTyped(val.type)) result = Type_Sizeof(val.type);
    else
    {
        if      (val.type == Type_UntypedBool)   result = 1;
        else if (val.type == Type_UntypedChar)   result = U64_EffectiveByteSize(val.character);
        else if (val.type == Type_UntypedInt)    result = BigInt_EffectiveByteSize(val.untyped_int);
        else if (val.type == Type_UntypedFloat)  result = BigFloat_EffectiveByteSize(val.untyped_float);
        else if (val.type == Type_UntypedString) result = StringLiteral_Of(val.string).size;
        else INVALID_CODE_PATH;
    }
    
    return result;
}

internal bool
ConstVal_IsImplicitlyConvertibleTo(Const_Val src_val, Type_ID dst_type)
{
    bool result = false;
    
    if (Type_IsImplicitlyCastableTo(src_val.type, dst_type)) result = true;
    else if (!Type_IsTyped(src_val.type) && Type_IsTyped(dst_type))
    {
        if      (src_val.type == Type_Bool && Type_IsNumeric(dst_type)) result = true;
        else if ((src_val.type == Type_UntypedInt || src_val.type == Type_UntypedChar) && Type_IsNumeric(dst_type))
        {
            if (Type_IsFloating(dst_type))
            {
                Big_Float f;
                if (src_val.type == Type_UntypedChar) f = BigFloat_FromF64(src_val.character);
                else                                  f = BigFloat_FromBigInt(src_val.untyped_int);
                
                result = (BigFloat_EffectiveByteSize(f) <= Type_Sizeof(dst_type));
            }
            
            else if (ConstVal_Sizeof(src_val) <= Type_Sizeof(dst_type)) result = true;
        }
        
        else if (src_val.type == Type_UntypedString)
        {
            Type_Info* dst_info = MM_TypeInfoOf(dst_type);
            
            if (dst_info != 0 && dst_info->kind == TypeInfo_Array)
            {
                if (dst_info->elem_type == Type_Byte && dst_info->array.size == StringLiteral_Of(src_val.string).size)
                {
                    result = true;
                }
                
                else if (dst_info->elem_type == Type_Char && dst_info->array.size == String_CountChars(StringLiteral_Of(src_val.string)))
                {
                    result = true;
                }
            }
        }
    }
    
    return result;
}

internal Const_Val
ConstVal_CastTo(Const_Val val, Type_ID dst)
{
    ASSERT(Type_IsCastableTo(val.type, dst));
    
    Const_Val result = { .type = dst };
    
    Type_ID stripped_type = Type_StripAlias(val.type);
    Type_ID stripped_dst  = Type_StripAlias(dst);
    
    if (Type_IsIntegral(stripped_type) || Type_IsChar(stripped_type)   ||
        Type_IsBoolean(stripped_type)  || stripped_type == Type_Typeid ||
        stripped_type == Type_Byte)
    {
        if (Type_IsFloating(stripped_dst))
        {
            NOT_IMPLEMENTED;
        }
        
        else
        {
            ASSERT(Type_IsIntegral(stripped_dst) || Type_IsChar(stripped_dst) ||
                   Type_IsBoolean(stripped_dst)  || stripped_dst == Type_Byte);
            
            u64 unsigned_val;
            if      (Type_IsChar(stripped_type))       unsigned_val = val.character;
            else if (Type_IsBoolean(stripped_type))    unsigned_val = val.boolean;
            else if (stripped_type == Type_UntypedInt) unsigned_val = BigInt_ToU64(val.untyped_int);
            else                                       unsigned_val = val.unsigned_int;
            
            // NOTE: sneaky way of doing sign extension by leveraging movsx generated by uint to int cast
            if (Type_IsSigned(stripped_dst))
            {
                i64 signed_val = 0;
                
                switch (Type_Sizeof(stripped_type))
                {
                    case 1: signed_val = (i64)(u8)unsigned_val;  break;
                    case 2: signed_val = (i64)(u16)unsigned_val; break;
                    case 4: signed_val = (i64)(u32)unsigned_val; break;
                    case 8: signed_val = (i64)(u64)unsigned_val; break;
                    INVALID_DEFAULT_CASE;
                }
                
                unsigned_val = (u64)signed_val;
            }
            
            u64 keep_mask = (~(u64)0 >> (64 - 8*Type_Sizeof(stripped_dst)));
            
            result.unsigned_int = unsigned_val & keep_mask;
        }
    }
    
    else if (Type_IsFloating(stripped_type))
    {
        NOT_IMPLEMENTED;
    }
    
    else if (stripped_type == Type_Any)
    {
        result = ConstVal_CastTo(*val.any, dst);
    }
    
    else NOT_IMPLEMENTED;
    /*
    Type_String,
    Type_Cstring,
    
    Type_UntypedString,
*/
    
    return result;
}

internal bool
ConstVal_ToBool(Const_Val val)
{
    return val.boolean;
}

internal Const_Val
ConstVal_FromBool(bool val)
{
    return (Const_Val){ .boolean = val };
}