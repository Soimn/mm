typedef enum SYMBOL_KIND
{
    Symbol_Var,
    Symbol_Const,
    Symbol_UsingLink,
    Symbol_Parameter,
    Symbol_StructMember,
    Symbol_EnumMember,
} SYMBOL_KIND;

typedef struct Symbol
{
    SYMBOL_KIND kind;
    bool is_complete;
    
    union
    {
        struct
        {
            Interned_String name;
            Type_ID type;
        } variable;
        
        struct
        {
            Interned_String name;
            Type_ID type;
            Const_Val value;
        } constant;
        
        struct
        {
            Interned_String accessor; // NOTE: BLANK_IDENTIFIER counts as having no accessor
            // link to symbol, this may not be in the same symbol table e.g. a: Struct; { using a; }
            // restriction within scope
        } using_link;
        
        struct
        {
            Interned_String name;
            Type_ID type;
            bool is_const;
            Const_Val value;
        } parameter;
        
        struct
        {
            Interned_String name;
            Type_ID type;
            //bool is_using;
        } struct_member;
        
        struct
        {
            Interned_String name;
            Type_ID underlying_type;
            Type_ID enum_type;
            Const_Val value;
        } enum_member;
    };
} Symbol;

// TODO: Should the symbol table have a "parent table" instead of using a scope chain?
typedef struct Symbol_Table
{
    bool _;
} Symbol_Table;

internal Symbol*
SymbolTable_GetSymbol(Symbol_Table* table, Interned_String name)
{
    Symbol* result = 0;
    
    if (name != BLANK_IDENTIFIER)
    {
        NOT_IMPLEMENTED;
    }
    
    return result;
}