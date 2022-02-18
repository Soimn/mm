typedef enum SYMBOL_KIND
{
    Symbol_Var,
    Symbol_Const,
    Symbol_UsingLink,
    Symbol_Parameter,
    Symbol_Member,
} SYMBOL_KIND;

typedef struct Symbol
{
    SYMBOL_KIND kind;
    
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
            // link to symbol,
            // restriction within scope
        } using_link;
        
        struct
        {
            Interned_String name;
            Type_ID type;
            Const_Val default_value;
        } parameter;
        
        struct
        {
            Interned_String name;
            Type_ID type;
        } member;
    };
} Symbol;

// TODO: Should the symbol table have a "parent table" instead of using a scope chain?
typedef struct Symbol_Table
{
    bool _;
} Symbol_Table;