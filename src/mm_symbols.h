typedef enum SYMBOL_KIND
{
    Symbol_Var,
    Symbol_Const,
    
    Symbol_UsingLink,
    
    SYMBOL_KIND_COUNT,
} SYMBOL_KIND;

typedef struct Symbol
{
    struct Symbol* next;
    Interned_String name;
    u32 declaration_id;
    u16 depth;
    u16 index;
    
    union
    {
        struct
        {
            Type_ID type;
            Const_Val const_value;
        };
        
        struct
        {
            struct Symbol_Table* table;
        } using_link;
    };
} Symbol;

typedef struct Symbol_Table
{
    Arena* arena;
    u32 hash_table_size;
    Symbol* anonymous_links_head;
    Symbol** anonymous_links_tail;
    Symbol* hash_table[];
} Symbol_Table;

// IMPORTANT NOTE: The operates at the scope level, which means declaration order within the same scope is ignored
internal Symbol*
SymbolTable_GetSymbol(Symbol_Table* table, Interned_String name, u16 search_origin_depth, u16 search_origin_index)
{
    Symbol* symbol = table->hash_table[name % table->hash_table_size];
    
    for (; symbol != 0; symbol = symbol->next)
    {
        if      (symbol->name != name)                                                         continue;
        else if (symbol->depth < search_origin_depth)                                          break;
        else if (symbol->depth == search_origin_depth && symbol->index == search_origin_index) break;
        else                                                                                   continue;
    }
    
    for (Symbol* using_link = table->anonymous_links_head;
         symbol == 0 && using_link != 0;
         using_link = using_link->next)
    {
        symbol = SymbolTable_GetSymbol(using_link->table, 0, 0);
    }
    
    return symbol;
}