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
    // TODO: link to ast
    // TODO: forward decls
    
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
            MM_Const_Val value;
        } constant;
        
        struct
        {
            MM_String name;
            MM_Type_Info* type;
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

// IMPORTANT NOTE: Types depend on symbols, they should be part of the dependency graph