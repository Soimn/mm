enum SYMBOL_KIND
{
    Symbol_Var,
    Symbol_Const,
    Symbol_UsingLink,
    // TODO: const parameters, using parameters
    //Symbol_Parameter,
    //Symbol_ReturnVal,
};

typedef i32 Scope_Index;
#define SCOPE_INDEX_UNORDERED -1
#define SCOPE_INDEX_FIRST_ORDERED 0

typedef struct Symbol
{
    Enum8(SYMBOL_KIND) kind;
    
    Scope_Index index;
    
    union
    {
        struct
        {
            Interned_String name;
            Type_ID type;
        } var;
        
        struct
        {
            Interned_String name;
            Type_ID type;
            Const_Val value;
        } constant;
        
        struct
        {
            struct Symbol* symbol;
            Scope_Index valid_threshold;
        } using_link;
    };
} Symbol;