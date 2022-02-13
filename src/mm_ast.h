typedef enum AST_NODE_KIND
{
    AST_Invalid = 0,
    
    AST_NamedValue,
    
    AST_FirstExpression,
    AST_Identifier = AST_FirstExpression,
    AST_String,
    AST_Char,
    AST_Int,
    AST_Float,
    AST_Boolean,
    AST_Proc,
    AST_ProcLiteral,
    AST_Struct,
    AST_Union,
    AST_Enum,
    AST_Compound,
    AST_Conditional,
    
    AST_FirstTypeLevel = 5*16,
    AST_PointerType = AST_FirstTypeLevel,
    AST_SliceType,
    AST_ArrayType,
    AST_LastTypeLevel = 6*16 - 1,
    
    AST_FirstPostfixLevel = 6*16,
    AST_Call = AST_FirstPostfixLevel,
    AST_ElementOf,
    AST_StructLiteral,
    AST_ArrayLiteral,
    AST_Cast,
    AST_Subscript,
    AST_Slice,
    AST_LastPostfixLevel = 7*16 - 1,
    
    AST_FirstPrefixLevel = 7*16,
    AST_Not = AST_FirstPrefixLevel,
    AST_BitNot,
    AST_Dereference,
    AST_Reference,
    AST_Neg,
    AST_LastPrefixLevel = 8*16 - 1,
    
    AST_FirstMulLevel = 8*16,
    AST_FirstBinary = AST_FirstMulLevel, 
    AST_Mul = AST_FirstBinary,
    AST_Div,
    AST_Rem,
    AST_BitAnd,
    AST_BitSar,
    AST_BitShr,
    AST_BitSplatShl,
    AST_BitShl,
    AST_LastMulLevel = 9*16 - 1,
    
    AST_FirstAddLevel = 9*16,
    AST_Add = AST_FirstAddLevel,
    AST_Sub,
    AST_BitOr,
    AST_BitXor,
    AST_LastAddLevel = 10*16 - 1,
    
    AST_FirstComparative = 10*16,
    AST_IsEqual = AST_FirstComparative,
    AST_IsNotEqual,
    AST_IsLess,
    AST_IsGreater,
    AST_IsLessEqual,
    AST_IsGreaterEqual,
    AST_LastComparative = 11*16 - 1,
    
    AST_And = 11*16,
    
    AST_Or = 12*16,
    AST_LastBinary = AST_Or,
    AST_LastExpression = AST_LastBinary,
    
    AST_FirstStatement = 13*16,
    AST_Scope,
    AST_If,
    AST_When,
    AST_While,
    AST_Break,
    AST_Continue,
    AST_Defer,
    AST_Return,
    AST_Using,
    AST_Assignment,
    AST_LastStatement = AST_Assignment,
    
    AST_FirstDeclaration,
    AST_Variable = AST_FirstDeclaration,
    AST_Constant,
    AST_LastDeclaration = AST_Constant,
    
    AST_NODE_KIND_MAX = 256,
} AST_NODE_KIND;

typedef struct AST_Node
{
    AST_NODE_KIND kind;
    struct AST_Node* next;
    
    union
    {
        struct
        {
            struct AST_Node* name;
            struct AST_Node* value;
        } named_value;
        
        Interned_String identifier;
        Interned_String string;
        u8 character;
        u64 integer;
        f64 floating;
        bool boolean;
        
        struct
        {
            struct AST_Node* condition;
            struct AST_Node* true_clause;
            struct AST_Node* false_clause;
        } conditional_expr;
        
        struct
        {
            struct AST_Node* params;
            struct AST_Node* return_values;
        } proc_type;
        
        struct
        {
            struct AST_Node* params;
            struct AST_Node* return_values;
            struct AST_Node* body;
        } proc_literal;
        
        struct
        {
            struct AST_Node* members;
        } struct_type;
        
        struct
        {
            struct AST_Node* member_type;
            struct AST_Node* members;
        } enum_type;
        
        struct AST_Node* compound_expr;
        struct AST_Node* unary_expr;
        
        struct
        {
            struct AST_Node* type;
            struct AST_Node* size;
        } array_type;
        
        struct
        {
            struct AST_Node* array;
            struct AST_Node* index;
        } subscript_expr;
        
        struct
        {
            struct AST_Node* array;
            struct AST_Node* start;
            struct AST_Node* past_end;
        } slice_expr;
        
        struct
        {
            struct AST_Node* func;
            struct AST_Node* args;
        } call_expr;
        
        struct
        {
            struct AST_Node* structure;
            Interned_String element;
        } element_of;
        
        struct
        {
            struct AST_Node* type;
            struct AST_Node* args;
        } struct_literal;
        
        struct
        {
            struct AST_Node* type;
            struct AST_Node* args;
        } array_literal;
        
        struct
        {
            struct AST_Node* type;
            struct AST_Node* expr;
        } cast_expr;
        
        struct
        {
            struct AST_Node* left;
            struct AST_Node* right;
        } binary_expr;
        
        struct
        {
            struct AST_Node* body;
            Interned_String label;
            bool is_braced;
        } block_statement;
        
        struct
        {
            Interned_String label;
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
            struct AST_Node* args;
        } return_statement;
        
        struct
        {
            struct AST_Node* symbol;
            Interned_String alias;
        } using_statement;
        
        struct
        {
            struct AST_Node* lhs;
            struct AST_Node* rhs;
            AST_NODE_KIND op;
        } assignment_statement;
        
        struct
        {
            struct AST_Node* names;
            struct AST_Node* type;
            struct AST_Node* values;
            bool is_uninitialized;
            bool is_using;
        } var_decl;
        
        struct
        {
            struct AST_Node* names;
            struct AST_Node* type;
            struct AST_Node* values;
            bool is_using;
        } const_decl;
    };
} AST_Node;