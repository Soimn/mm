typedef enum MM_SYMBOL_KIND
{
    MM_Symbol_Variable,
    MM_Symbol_Constant,
    MM_Symbol_UsingLink,
    MM_Symbol_Parameter,
    MM_Symbol_ReturnValue,
    MM_Symbol_Label,
} MM_SYMBOL_KIND;

typedef struct MM_Symbol
{
    void* next; // NOTE: MM_Global_Symbol also uses this, hence void* instead of MM_Symbol*
    struct MM_AST* ast;
    MM_SYMBOL_KIND kind;
    
    union
    {
        MM_String name;
        
        struct
        {
            MM_String name;
            MM_Type_Info* type;
        } variable;
        
        struct
        {
            MM_String name;
            MM_Type_Info* type;
            MM_Const_Val value; // NOTE: MM_Type_None can signify a fwd decl (see MM_Symbol_IsComplete)
        } constant;
        
        struct
        {
            MM_String name;
            MM_Type_Info* type;
            // NOTE: This is the base symbol, the type specified above is the type one gets by following the access path,
            //       specified by the ast, from this symbol.
            struct MM_Symbol* symbol;
        } using_link;
        
        struct
        {
            MM_String name;
            MM_Type_Info* type;
            MM_Const_Val value;
        } parameter;
        
        struct
        {
            MM_String name;
            MM_Type_Info* type;
            MM_Const_Val value;
        } return_value;
        
        struct
        {
            MM_String name;
            struct MM_Symbol* symbol;
        } label;
    };
} MM_Symbol;

typedef struct MM_Global_Symbol
{
    struct MM_Symbol;
    // TODO: dependencies (symbols, types)
} MM_Global_Symbol;

typedef struct MM_Symbol_Table
{
    struct MM_Symbol_Table* parent;
    MM_Symbol* symbols;
} MM_Symbol_Table;

typedef struct MM_Global_Symbol_Table
{
    struct MM_Symbol_Table;
    MM_Global_Symbol** map;
    MM_u32 map_size;
    void** free_list;
} MM_Global_Symbol_Table;

MM_Internal MM_Symbol*
MM_Symbol_Get(MM_Symbol_Table* table, MM_String name)
{
    if (table->parent == 0)
    {
        MM_Global_Symbol_Table* global_table = (MM_Global_Symbol_Table*)table;
        
        MM_Global_Symbol* scan = global_table->map[MM_String_Hash(name) % global_table->map_size];
        
        for (; scan != 0; scan = scan->next)
        {
            if (MM_String_Match(name, scan->name)) break;
        }
        
        return (MM_Symbol*)scan;
    }
    else
    {
        MM_Symbol* scan = table->symbols;
        
        for (; scan != 0; scan = scan->next)
        {
            if (MM_String_Match(name, scan->name)) break;
        }
        
        return scan;
    }
}

// NOTE: Symbols that are only forward declared are incomplete.
//       Symbols become complete when they are properly declared (i.e. everything is specified, no :: --- or struct ---).
//       The type along with the value specifies the kind of forward declaration.
MM_Internal MM_bool
MM_Symbol_IsComplete(MM_Symbol* symbol)
{
    if (symbol->kind == MM_Symbol_Constant)
    {
        if (symbol->constant.value.type == MM_Type_None)
        {
            // :type: ---
            // :: proc ---
            return MM_false;
        }
        else if (symbol->constant.type->kind == MM_TypeInfo_BaseType && symbol->constant.type->type_id == MM_Type_Typeid)
        {
            // :: struct ---
            // :: union ---
            // :: enum ---
            // :: proc_set ---
            MM_ASSERT(symbol->constant.type->kind == MM_TypeInfo_Struct ||
                      symbol->constant.type->kind == MM_TypeInfo_Union  ||
                      symbol->constant.type->kind == MM_TypeInfo_Enum   ||
                      symbol->constant.type->kind == MM_TypeInfo_ProcSet);
            
            // HACK: This is ugly and should by all means be inferred by the optimizer,
            //       but I am paranoid that it is too dumb.
            MM_bool* is_complete = (MM_bool*)(&symbol->constant.type->header + 1);
            
            // HACK: This might invoke undefined behaviour, the MM_umm cast is an attempt at mitigating this, but the generated
            //       code should be checked regardless.
            MM_ASSERT((MM_umm)&((MM_Type_Info_Struct*)  symbol->constant.type)->is_complete == (MM_umm)is_complete &&
                      (MM_umm)&((MM_Type_Info_Union*)   symbol->constant.type)->is_complete == (MM_umm)is_complete &&
                      (MM_umm)&((MM_Type_Info_Enum*)    symbol->constant.type)->is_complete == (MM_umm)is_complete &&
                      (MM_umm)&((MM_Type_Info_Proc_Set*)symbol->constant.type)->is_complete == (MM_umm)is_complete);
            
            return *is_complete;
        }
    }
    
    return MM_true;
}

// IMPORTANT NOTE: Types depend on symbols, they should be part of the dependency graph
// IMPORTANT NOTE: dependencies on symbols can go through the type table