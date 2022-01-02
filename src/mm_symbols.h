typedef union Const_Val
{
    u64 integer;
    f64 floating;
    Interned_String string;
    Character character;
} Const_Val;

internal inline Const_Val
ConstVal_FromBool(bool val)
{
    return (Const_Val){ .integer = val };
}

internal inline bool
ConstVal_ToBool(Const_Val val)
{
    return !!val.integer;
}

internal inline Const_Val
ConstVal_FromU64(u64 val)
{
    return (Const_Val){ .integer = val };
}

internal inline u64
ConstVal_ToU64(Const_Val val)
{
    return val.integer;
}

internal inline Const_Val
ConstVal_FromI64(i64 val)
{
    return (Const_Val){ .integer = (u64)val };
}

internal inline i64
ConstVal_ToI64(Const_Val val)
{
    return (i64)val.integer;
}

internal inline Const_Val
ConstVal_FromF64(f64 val)
{
    return (Const_Val){ .floating = val };
}

internal inline f64
ConstVal_ToF64(Const_Val val)
{
    return val.floating;
}

internal inline Const_Val
ConstVal_FromChar(Character val)
{
    return (Const_Val){ .character = val };
}

internal inline Character
ConstVal_ToChar(Const_Val val)
{
    return val.character;
}

internal inline Const_Val
ConstVal_FromString(Interned_String val)
{
    return (Const_Val){ .string = val };
}

internal inline Interned_String
ConstVal_ToString(Const_Val val)
{
    return val.string;
}

internal inline Const_Val
ConstVal_FromTypeid(Type_ID val)
{
    return (Const_Val){ .integer = val };
}

internal inline Type_ID
ConstVal_ToTypeid(Const_Val val)
{
    return (Type_ID)val.integer;
}

enum SYMBOL_KIND
{
    Symbol_Var,
    Symbol_Const,
    Symbol_UsingLink,
    //Symbol_ProcGroup
};

enum SYMBOL_STATE
{
    Symbol_Unresolved = 0,
    Symbol_Resolving,
    Symbol_Resolved
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
    TypeInfo_Int,
    TypeInfo_Float,
    
    TypeInfo_Array,
    TypeInfo_Pointer,
    TypeInfo_DynArray,
    TypeInfo_Slice,
    
    TypeInfo_Struct,
    TypeInfo_Union,
    TypeInfo_Enum,
    TypeInfo_Proc,
    
    TypeInfo_Alias,
};

typedef struct Type_Info
{
    Enum8(TYPE_INFO_KIND) kind;
    
    union
    {
        struct
        {
            u64 width;
            bool is_signed;
        } integer;
        
        struct
        {
            u64 width;
        } floating;
        
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
    u32 hash = String_HashOfInternedString(name);
    
    Symbol** next = &MM.global_symbol_table[hash % ARRAY_SIZE(MM.global_symbol_table)];
    
    for (; *next != 0; next = &(*next)->next);
    
    Symbol* sym = Arena_PushSize(&MM.misc_arena, sizeof(Symbol), ALIGNOF(Symbol));
    ZeroStruct(sym);
    sym->name = name;
    sym->kind = kind;
    
    *next = sym;
    
    return sym;
}

internal inline Type_Info*
MM_TypeInfo(Type_ID type)
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
Type_IsImplicitlyCastableTo(Type_ID type, Type_ID target)
{
    bool result = false;
    
    type   = Type_StripAlias(type);
    target = Type_StripAlias(target);
    
    if      (type == target)                         result = true;
    else if (type == Type_Any || target == Type_Any) result = true;
    else if (Type_IsTyped(target))
    {
        if ((type == Type_UntypedInt || type == Type_UntypedBool || type == Type_UntypedChar) && 
            (Type_IsNumeric(target) || Type_IsBoolean(target) || target == Type_Char || target == Type_Byte))
        {
            result = true;
        }
        
        else if (type == Type_Byte          && (target == Type_U8 || target == Type_I8))          result = true;
        else if (type == Type_UntypedFloat  && Type_IsFloating(target))                           result = true;
        else if (type == Type_UntypedString && (target == Type_String || target == Type_Cstring)) result = true;
    }
    
    return result;
}

internal bool
Type_IsCastableTo(Type_ID type, Type_ID target, Const_Val type_val)
{
    bool result = false;
    
    type   = Type_StripAlias(type);
    target = Type_StripAlias(target);
    
    if (Type_IsImplicitlyCastableTo(type, target)) result = true;
    else if (Type_IsTyped(target))
    {
        Type_ID def_type = Type_ToDefTyped(type);
        
        if ((def_type == Type_Typeid ||Type_IsNumeric(def_type) || Type_IsBoolean(def_type) || def_type == Type_Char || def_type == Type_Byte) &&
            (Type_IsNumeric(target) || Type_IsBoolean(target) || target == Type_Char || target == Type_Byte))
        {
            result = true;
        }
        
        else
        {
            Type_Info* type_info   = MM_TypeInfo(type);
            Type_Info* target_info = MM_TypeInfo(target);
            
            if (type_info->kind == TypeInfo_Pointer && target_info->kind == TypeInfo_Pointer) result = true;
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
            
            else NOT_IMPLEMENTED;
        }
    }
    
    return result;
}

internal Type_ID
Type_CommonType(Type_ID t0, Type_ID t1)
{
    Type_ID common = Type_Nil;
    
    if (t0 == t1) common = t0;
    else if ( Type_IsTyped(t0) && !Type_IsTyped(t1) && Type_IsImplicitlyCastableTo(t1, t0)) common = t0;
    else if (!Type_IsTyped(t0) &&  Type_IsTyped(t1) && Type_IsImplicitlyCastableTo(t0, t1)) common = t1;
    
    return common;
}

internal inline f64
ConstVal_ConvertToF64(Const_Val val, Type_ID val_type)
{
    f64 result = 0;
    
    if (Type_IsFloating(val_type)) result     = ConstVal_ToF64(val);
    else if (Type_IsBoolean(val_type)) result = (f64)ConstVal_ToBool(val);
    else if (Type_IsChar(val_type)) result    = (f64)ConstVal_ToChar(val).word;
    else
    {
        ASSERT(Type_IsIntegral(val_type));
        
        if (Type_IsSigned(val_type)) result = (f64)ConstVal_ToI64(val);
        else                         result = (f64)ConstVal_ToU64(val);
    }
    
    return result;
}
