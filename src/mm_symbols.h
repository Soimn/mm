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