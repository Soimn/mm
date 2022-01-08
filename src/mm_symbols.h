typedef union Const_Val_
{
    u64 uinteger;
    i64 sinteger;
    bool boolean;
    Type_ID type;
    f64 floating;
    Interned_String string;
    Character character;
} Const_Val_;

typedef struct Any
{
    Const_Val_ data;
    Type_ID type;
} Any;

typedef struct Const_Val
{
    Type_ID type;
    union
    {
        struct Const_Val_ data;
        Any any;
    };
} Const_Val;

internal inline Const_Val
ConstVal_StripAny(Const_Val val)
{
    return (val.type != Type_Any ? val : (Const_Val){ .type = val.any.type, .data = val.any.data });
}

internal inline Const_Val
ConstVal_FromBool(bool val)
{
    return (Const_Val){ .data.boolean = val };
}

internal inline bool
ConstVal_ToBool(Const_Val val)
{
    ASSERT(Type_IsBoolean(val.type));
    
    return val.data.boolean;
}

internal inline Const_Val
ConstVal_FromU64(u64 val)
{
    return (Const_Val){ .data.integer = val };
}

internal inline u64
ConstVal_ToU64(Const_Val val)
{
    ASSERT(Type_IsIntegral(val.type));
    
    return val.data.integer;
}

internal inline Const_Val
ConstVal_FromF64(f64 val)
{
    return (Const_Val){ .data.floating = val };
}

internal inline f64
ConstVal_ToF64(Const_Val val)
{
    ASSERT(Type_IsFloating(val.type));
    
    return val.data.floating;
}

internal inline Const_Val
ConstVal_FromChar(Character val)
{
    return (Const_Val){ .data.character = val };
}

internal inline Character
ConstVal_ToChar(Const_Val val)
{
    ASSERT(Type_IsChar(val.type));
    
    return val.data.character;
}

internal inline Const_Val
ConstVal_FromString(Interned_String val)
{
    val = ConstVal_StripAny(val);
    
    return (Const_Val){ .data.string = val };
}

internal inline Interned_String
ConstVal_ToString(Const_Val val)
{
    ASSERT(val.type == Type_String || val.type == Type_Cstring);
    
    return val.data.string;
}

internal inline Const_Val
ConstVal_FromTypeid(Type_ID val)
{
    return (Const_Val){ .data.type = val };
}

internal inline Type_ID
ConstVal_ToTypeid(Const_Val val)
{
    ASSERT(val.type == Type_Typeid);
    
    return val.data.type;
}

enum SYMBOL_KIND
{
    Symbol_Var,
    Symbol_Const,
    Symbol_UsingLink,
};

typedef struct Symbol
{
    struct Symbol* next;
    
    Interned_String name;
    Enum8(SYMBOL_KIND) kind;
    
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
    
    Type_Info* info = MM_TypeInfo(type);
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
        if      (val.type == Type_UntypedBool) result = 1;
        else if (val.type == Type_UntypedInt || val.type == Type_UntypedChar)
        {
            if ((i64)val.data.integer >= I8_MIN && (i64)val.data.integer <= U8_MAX)
            {
                result = 1;
            }
            
            else if ((i64)val.data.integer >= I16_MIN && (i64)val.data.integer <= U16_MAX)
            {
                result = 2;
            }
            
            else if ((i64)val.data.integer >= I32_MIN && (i64)val.data.integer <= U32_MAX)
            {
                result = 4;
            }
            
            else result = 8;
        }
        
        else if (val.type == Type_UntypedFloat)
        {
            // TODO: verify this
            u64 bits = val.data.floating;
            
            i64 exponent = (i64)(bits &  ((u64)0x7FF << (63 - 12))) - 1023;
            u64 mantissa =       bits & ~((u64)0xFFF << (63 - 12));
            
            umm mantissa_length = 0;
            for (umm scan = mantissa; mantissa != 0; mantissa <<= 1) ++mantissa_length;
            
            // NOTE: subnormal numbers have one bit less precision
            if      (ABS(exponent) <= 15  && mantissa_length <= 10 - (exponent == -15))  result = 2;
            else if (ABS(exponent) <= 127 && mantissa_length <= 23 - (exponent == -127)) result = 4;
            else                                                                         result = 8;
        }
        
        else if (val.type == Type_UntypedString)
        {
            result = StringLiteral_Of(val.data.string).size;
        }
        
        else INVALID_CODE_PATH;
    }
    
    return result;
}

internal bool
ConstVal_IsImplicitlyConvertibleTo(Const_Val src_val, Type_ID dst_type)
{
    bool result = false;
    
    if (Type_IsImplicitlyCastableTo(src_val.type, dst_type)) result = true;
    else if (Type_IsTyped(dst_type))
    {
        else if ((src_val.type == Type_UntypedInt || src_val.type == Type_UntypedChar || Type_IsBoolean(src_val.type)) && Type_IsNumeric(dst_type))
        {
            if (Type_IsFloating(dst_type))
            {
                if (Type_IsFloating(dst_type) && ConstVal_Sizeof(src_val) <= Type_Sizeof(dst_type))
                {
                    result = true;
                }
                
                else if ((u64)src_val.data.floating == src_val.data.floating &&
                         ConstVal_Sizeof(ConstVal_FromU64((u64)src_val.data.floating)) <= Type_Sizeof(dst_type))
                {
                    result = true;
                }
            }
            
            else if (ConstVal_Sizeof(src_val) <= Type_Sizeof(dst_type)) result = true;
        }
        
        else if (type == Type_UntypedString && target_info->kind == TypeInfo_Array)
        {
            if (Type_StripAlias(target_info->elem_type) == Type_Byte && target_info->array.size == StringLiteral_Of(ConstVal_ToString(type_val)).size)
            {
                result = true;
            }
            
            else if (Type_StripAlias(target_info->elem_type) == Type_Char && target_info->array.size == String_CountChars(StringLiteral_Of(ConstVal_ToString(type_val))))
            {
                result = true;
            }
        }
    }
    
    return result;
}

internal Const_Val
ConstVal_UnaryOp(Const_Val val, AST_NODE_KIND op)
{
    Const_Val result = { .type == Type_Nil };
    
    switch (op)
    {
        case AST_Negation:
        {
            if (Type_IsFloating(type)) val.data.floating = -val.data.floating;
            else                       val.data.integer  = -val.data.integer;
        } break;
        
        case AST_Complement:
        {
            val.data.integer = ~val.data.integer;
        } break;
        
        case AST_Not:
        {
            val.data.boolean = !val.data.boolean;
        } break;
        
        INVALID_DEFAULT_CASE;
    }
    
    return result;
}

internal Const_Val
ConstVal_BinaryOp(Const_Val lhs, Const_Val rhs, AST_NODE_KIND op)
{
    Const_Val result = { .type == Type_Nil };
    
    switch (op)
    {
        case AST_Cast:
        {
            ASSERT(lhs.type == Type_Typeid);
            
            result = (Const_Val){ .type = dst };
            
            Type_ID src = rhs.type;
            Type_ID dst = lhs.data.type;
            Type_ID stripped_src = Type_StripAlias(src);
            Type_ID stripped_dst = Type_StripAlias(dst);
            
            if (dst == Type_Any)
            {
                if (src == Type_Any) result.any = lhs.any;
                else
                {
                    result.any = (Any){ .type = lhs.type, .data = lhs.data };
                }
            }
            
            else if (stripped_src == stripped_dst) result.data = rhs.data;
            
            else if (Type_IsNumeric(stripped_dst) || Type_IsChar(stripped_dst) ||
                     Type_IsBoolean(stripped_dst) || stripped_dst == Type_Byte)
            {
                
                if (Type_IsFloating(stripped_src))
                {
                    // HACK: figure out what should happen on integer overflow and underflow
                    // TODO: verify this
                    u64 mask = ~(u64)0 >> (64 - 8*Type_Sizeof(stripped_dst));
                    if (Type_IsSigned(stripped_dst)) result.data.sinteger = (i64)rhs.data.floating & mask;
                    else                             result.data.uinteger = (u64)rhs.data.floating & mask;
                    //
                }
                
                else
                {
                    ASSERT(Type_IsIntegral(stripped_src) || Type_IsChar(stripped_src)   ||
                           Type_IsBoolean(stripped_src)  || stripped_src == Type_Typeid ||
                           stripped_src == Type_Byte);
                    
                    if (Type_IsFloating(stripped_dst))
                    {
                        if (Type_IsSigned(stripped_src))
                        {
                            NOT_IMPLEMENTED;
                        }
                        
                        else
                        {
                            NOT_IMPLEMENTED;
                        }
                    }
                    
                    else
                    {
                        u64 mask = ~(u64)0 >> (64 - 8*Type_Sizeof(stripped_dst));
                        
                        if (!Type_IsSigned(stripped_src)) result.data.uinteger = rhs.data.uinteger & mask;
                        else
                        {
                            // TODO: verify this
                            u64 sign_extender = (~(u64)0 << 8*Type_Sizeof(stripped_src)) & mask;
                            u64 sign          = (u64)1 << (8*Type_Sizeof(stripped_src) - 1);
                            
                            result.data.uinteger = rhs.data.uinteger & mask;
                            
                            if (sign != 0) result.data.uinteger |= sign_extender;
                            //
                        }
                    }
                }
            }
            NOT_IMPLEMENTED;NOT_IMPLEMENTED;NOT_IMPLEMENTED;NOT_IMPLEMENTED;NOT_IMPLEMENTED;NOT_IMPLEMENTED;NOT_IMPLEMENTED;NOT_IMPLEMENTED;NOT_IMPLEMENTED;NOT_IMPLEMENTED;
            else if (src == Type_UntypedString && dst == Type_Cstring)              result = true;
            
            else
            {
                Type_Info* src_info = MM_TypeInfoOf(src);
                Type_Info* dst_info = MM_TypeInfoOf(dst);
                
                if (src_info != 0 && dst_info != 0)
                {
                    if (src_info->kind == TypeInfo_Pointer && dst_info->kind == TypeInfo_Pointer) result = true;
                }
            }
        } break;
        
        case AST_Mul:
        case AST_Div:
        case AST_Add:
        case AST_Sub:
        NOT_IMPLEMENTED;
        break;
        
        case AST_Rem:
        case AST_BitwiseAnd:
        case AST_ArithmeticRightShift:
        case AST_RightShift:
        case AST_LeftShift:
        case AST_BitwiseOr:
        case AST_BitwiseXor:
        NOT_IMPLEMENTED;
        break;
        
        case AST_IsEqual:
        case AST_IsNotEqual:
        NOT_IMPLEMENTED;
        break;
        
        case AST_IsStrictlyLess:
        case AST_IsStrictlyGreater:
        case AST_IsLess:
        case AST_IsGreater:
        NOT_IMPLEMENTED;
        break;
        
        case AST_And:
        case AST_Or:
        NOT_IMPLEMENTED;
        break;
        
        INVALID_DEFAULT_CASE;
    }
    
    return result;
}

internal Const_Val
ConstVal_TernaryOp(Const_Val val0, Const_Val val1, Const_Val val2, AST_NODE_KIND op)
{
    ASSERT(op == AST_Conditional);
    
    Const_Val result = { .type == Type_Nil };
    
    NOT_IMPLEMENTED;
    
    //AST_Conditional,
    
    return result;
}