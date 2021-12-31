typedef u32 Type_ID;

typedef union Const_Val
{
    u64 integer;
    f64 floating;
    Interned_String string;
} Const_Val;

internal inline bool
ConstVal_ToBool(Const_Val val)
{
    return !!val.integer;
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

typedef Symbol* Symbol_Table;

enum
{
    Type_Nil = 0,
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
    Type_Bool,
    Type_String,
    Type_Any,
    Type_Typeid,
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

internal inline bool
Type_IsImplicitlyConvertibleTo(Type_ID type, Type_ID target)
{
    bool result = false;
    
    NOT_IMPLEMENTED;
    
    return result;
}