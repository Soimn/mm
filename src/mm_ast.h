enum AST_NODE_KIND
{
    // precedence 0: 0 - 19
    AST_Invalid = 0,
    
    AST_FirstExpression,
    AST_Identifier = AST_FirstExpression,
    AST_String,
    AST_Char,
    AST_Number,
    AST_Boolean,
    AST_StructLiteral,
    AST_ArrayLiteral,
    AST_Proc,
    AST_ProcType,
    AST_Struct,
    AST_Union,
    AST_Enum,
    AST_Directive,
    AST_Compound,
    
    // precedence 1: 20 - 39
    AST_FirstTypeLevel = 20,
    AST_PointerType = AST_FirstTypeLevel,
    AST_SliceType,
    AST_ArrayType,
    AST_DynamicArrayType,
    AST_LastTypeLevel = AST_DynamicArrayType,
    
    // precedence 2: 40 - 59
    AST_FirstPostfixLevel = 40,
    AST_Subscript = AST_FirstPostfixLevel,
    AST_Slice,
    AST_Call,
    AST_ElementOf,
    AST_UfcsOf,
    AST_LastPostfixLevel = AST_UfcsOf,
    
    // precedence 3: 60 - 79
    AST_FirstPrefixLevel = 60,
    AST_Negation = AST_FirstPrefixLevel,
    AST_Complement,
    AST_Not,
    AST_Reference,
    AST_Dereference,
    AST_Spread,
    AST_LastPrefixLevel = AST_Spread,
    
    // precedence 4: 80 - 99
    AST_FirstRangeLevel = 80,
    AST_ClosedRange = AST_FirstRangeLevel,
    AST_HalfOpenRange,
    AST_LastRangeLevel = AST_HalfOpenRange,
    
    // precedence 5: 100 - 119
    AST_FirstMulLevel = 100,
    AST_Mul = AST_FirstMulLevel,
    AST_Div,
    AST_Rem,
    AST_BitwiseAnd,
    AST_ArithmeticRightShift,
    AST_RightShift,
    AST_LeftShift,
    AST_LastMulLevel = AST_LeftShift,
    
    // precedence 6: 120 - 139
    AST_FirstAddLevel = 120,
    AST_Add = AST_FirstAddLevel,
    AST_Sub,
    AST_BitwiseOr,
    AST_BitwiseXor,
    AST_LastAddLevel = AST_BitwiseOr,
    
    // precedence 7: 140 - 159
    AST_FirstComparative = 140,
    AST_IsEqual = AST_FirstComparative,
    AST_IsNotEqual,
    AST_IsStrictlyLess,
    AST_IsStrictlyGreater,
    AST_IsLess,
    AST_IsGreater,
    AST_LastComparative = AST_IsGreater,
    
    // precedence 8: 160 - 179
    AST_And = 160,
    
    // precedence 9: 180 - 199
    AST_Or = 180,
    
    // precedence 10: 200 - 219
    AST_Conditional,
    AST_LastExpression = AST_Conditional,
    
    AST_FirstStatement = 220,
    AST_Scope = AST_FirstStatement,
    
    AST_If,
    AST_When,
    AST_While,
    AST_Break,
    AST_Continue,
    AST_Defer,
    AST_Return,
    AST_Using,
    AST_Assignment,
    
    AST_VariableDecl,
    AST_ConstantDecl,
    AST_IncludeDecl,
    AST_LastStatement = AST_IncludeDecl,
    
    AST_NODE_KIND_END
};

#define PRECEDENCE_FROM_KIND(kind) (kind / 20)
#define IS_EXPRESSION(kind) (kind >= AST_FirstExpression && kind <= AST_LastExpression)

typedef struct AST_Node
{
    Enum8(AST_NODE_KIND) kind;
    
    File_ID file_id;
    u32 file_offset;
    
    struct AST_Node* next;
    
    union
    {
        Interned_String identifier;
        Interned_String string;
        Character character;
        Number number;
        bool boolean;
        
        struct
        {
            struct AST_Node* type;
            struct AST_Node* params;
        } struct_literal;
        
        struct
        {
            struct AST_Node* type;
            struct AST_Node* params;
        } array_literal;
        
        struct
        {
            struct AST_Node* params;
            struct AST_Node* return_values;
            struct AST_Node* body;
        } proc_literal;
        
        struct
        {
            struct AST_Node* params;
            struct AST_Node* return_values;
        } proc_type;
        
        struct
        {
            struct AST_Node* members;
        } struct_type, union_type;
        
        struct
        {
            struct AST_Node* elem_type;
            struct AST_Node* members;
        } enum_type;
        
        struct
        {
            struct AST_Node* elem_type;
            struct AST_Node* size;
        } array_type;
        
        struct
        {
            Interned_String name;
            struct AST_Node* params;
        } directive;
        
        struct AST_Node* compound_expr;
        
        struct
        {
            struct AST_Node* left;
            struct AST_Node* right;
        } binary_expr;
        
        struct AST_Node* unary_expr;
        
        struct
        {
            struct AST_Node* array;
            struct AST_Node* index;
        } subscript_expr;
        
        struct
        {
            struct AST_Node* array;
            struct AST_Node* start;
            struct AST_Node* one_after_end;
        } slice_expr;
        
        struct
        {
            struct AST_Node* func;
            struct AST_Node* params;
        } call_expr;
        
        struct
        {
            struct AST_Node* condition;
            struct AST_Node* true_clause;
            struct AST_Node* false_clause;
        } conditional_expr;
        
        struct
        {
            struct AST_Node* body;
            Interned_String label;
            bool is_do;
        } scope_statement;
        
        struct
        {
            struct AST_Node* init;
            struct AST_Node* condition;
            struct AST_Node* true_body;
            struct AST_Node* false_body;
        } if_statement;
        
        struct
        {
            struct AST_Node* condition;
            struct AST_Node* true_body;
            struct AST_Node* false_body;
        } when_statement;
        
        struct
        {
            struct AST_Node* init;
            struct AST_Node* condition;
            struct AST_Node* step;
            struct AST_Node* body;
        } while_statement;
        
        struct
        {
            Interned_String label;
        } break_statement, continue_statement;
        
        struct AST_Node* defer_statement;
        
        struct 
        {
            struct AST_Node* expression;
            struct Symbol* symbol;
        } using_statement;
        
        struct
        {
            struct AST_Node* values;
        } return_statement;
        
        struct
        {
            struct AST_Node* left;
            struct AST_Node* right;
            u8 kind; // NOTE: AST_Invalid is used to signify regular =
        } assignment_statement;
        
        struct
        {
            struct AST_Node* names;
            struct AST_Node* type;
            struct AST_Node* values;
            struct Symbol* symbol;
            bool is_uninitialized;
            bool is_using;
        } var_decl, const_decl;
        
        struct
        {
            Interned_String path;
            File_ID file_id;
        } include;
    };
} AST_Node;