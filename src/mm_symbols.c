typedef enum MM_SYMBOL_KIND
{
    MM_Symbol_Invalid = 0,
    
    MM_Symbol_Variable,
    MM_Symbol_Constant,
    MM_Symbol_UsingLink,
    MM_Symbol_Parameter,
    MM_Symbol_ReturnValue,
    MM_Symbol_BlockLabel,
    MM_Symbol_StructMember,
    MM_Symbol_UnionMember,
    MM_Symbol_EnumMember,
} MM_SYMBOL_KIND;

typedef struct MM_Symbol
{
    MM_SYMBOL_KIND kind;
    struct MM_Symbol* prev;
    struct MM_Symbol* next;
    
    union
    {
        MM_String name;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
        } variable;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: value
        } constant;
        
        struct
        {
            MM_String name;
            // symbol
        } using_link;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value?
        } parameter;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value?
        } return_value;
        
        struct
        {
            MM_String name;
            // TODO: Reference to the block it is applied to
        } block_label;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value
            // TODO: offset?
        } struct_member;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value
        } union_member;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: value
        } enum_member;
    };
    
    MM_AST* ast;
} MM_Symbol;

typedef struct MM_Symbol_Table
{
    MM_Symbol* symbols;
    MM_Symbol** map;
    MM_u32 map_size;
    MM_u32 symbol_count;
} MM_Symbol_Table;