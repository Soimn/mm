typedef enum AST_NODE_KIND
{
    AST_Nil = 0,
    
    AST_NamedValue,
    AST_Parameter,
    
    AST_FirstExpression,
    AST_Identifier = AST_FirstExpression,
    AST_String,
    AST_Char,
    AST_Int,
    AST_Float,
    AST_Bool,
    AST_Proc,
    AST_ProcLiteral,
    AST_Struct,
    AST_Union,
    AST_Enum,
    AST_Compound,
    AST_Selector,
    AST_PolymorphicName,
    AST_IntrinsicCall,
    
    AST_Dereference,
    AST_Member,
    AST_Subscript,
    AST_Slice,
    AST_Call,
    
    AST_Reference,
    AST_ArrayType,
    AST_SliceType,
    
    AST_StructLiteral,
    AST_ArrayLiteral,
    
    AST_Neg,
    AST_BitNot,
    AST_Not,
    
    AST_FirstMulLevel = 8*16,
    AST_Mul,
    AST_Div,
    AST_Rem,
    AST_BitAnd,
    AST_BitShl,
    AST_BitShr,
    AST_BitSar,
    AST_LastMulLevel = AST_BitSar,
    
    AST_FirstAddLevel = 9*16,
    AST_BitOr,
    AST_BitXor,
    AST_Sub,
    AST_Add,
    AST_LastAddLevel = AST_Add,
    
    AST_FirstCmpLevel = 10*16,
    AST_IsStrictlyGreater = AST_FirstCmpLevel,
    AST_IsGreater,
    AST_IsStrictlyLess,
    AST_IsLess,
    AST_IsEquals,
    AST_IsNotEquals,
    AST_LastCmpLevel = AST_IsNotEquals,
    
    AST_And = 11*16,
    
    AST_Or = 12*16,
    
    AST_Conditional = 13*16,
    AST_LastExpression = AST_Conditional,
    
    AST_FirstStatement,
    AST_Block,
    AST_If,
    AST_While,
    AST_Break,
    AST_Continue,
    AST_Defer,
    AST_Return,
    AST_Assignment,
    AST_LastStatement = AST_Assignment,
    
    AST_FirstDeclaration,
    AST_Variable = AST_FirstDeclaration,
    AST_Constant,
    AST_Using,
    AST_LastDeclaration = AST_Using,
} AST_NODE_KIND;

// NOTE: Every member that can legally be 0 is marked with the value means when it is
typedef struct AST_Node
{
    AST_NODE_KIND kind;
    
    struct AST_Node* next; // NOTE: 0 means no next
    
    union
    {
        struct
        {
            struct AST_Node* name;  // NOTE: 0 means no name, this is not valid for enum members
            struct AST_Node* value;
        } named_value;
        
        struct
        {
            struct AST_Node* name;
            struct AST_Node* type;  // NOTE: 0 means infered type
            struct AST_Node* value; // NOTE: 0 means no value
            bool is_using;
        } parameter;
        
        Interned_String string;  // NOTE: 0 means ""
        u32 character;           // NOTE: 0 means '\x00'
        Big_Int integer;         // NOTE: 0 means 0
        Big_Float floating;      // NOTE: 0 means 0
        bool boolean;            // NOTE: 0 means false
        
        struct
        {
            struct AST_Node* params;       // NOTE: 0 means no parameters
            struct AST_Node* return_types; // NOTE: 0 means no return values
            struct AST_Node* where_clause; // NOTE: 0 means no clause
        } proc_type;
        
        struct
        {
            struct AST_Node* params;       // NOTE: 0 means no parameters
            struct AST_Node* return_types; // NOTE: 0 means no return value
            struct AST_Node* where_clause; // NOTE: 0 means no clause
            struct AST_Node* body;         // NOTE: 0 means declaration, will otherwise point to a block_statement
        } proc_literal;
        
        struct
        {
            struct AST_Node* params; // NOTE: 0 means no parameters
            struct AST_Node* body;   // NOTE: 0 means empty body
        } struct_type;
        
        struct
        {
            struct AST_Node* params; // NOTE: 0 means no parameters
            struct AST_Node* body;   // NOTE: 0 means empty body
        } union_type;
        
        struct
        {
            struct AST_Node* type; // NOTE: 0 means default enum backing type
            struct AST_Node* body; // NOTE: 0 means empty body
        } enum_type;
        
        struct AST_Node* compound_expr;
        
        Interned_String selector_expr;
        
        Interned_String poly_name;
        
        struct
        {
            Interned_String proc;
            struct AST_Node* args; // NOTE: 0 means no arguments
        } intrinsic_call_expr;
        
        struct
        {
            struct AST_Node* expr;
            Interned_String member;
        } member_expr;
        
        struct
        {
            struct AST_Node* array;
            struct AST_Node* index;
        } subscript_expr;
        
        struct
        {
            struct AST_Node* array;
            struct AST_Node* start;    // NOTE: 0 means the start of the array
            struct AST_Node* past_end; // NOTE: 0 means one past the the of the array
        } slice_expr;
        
        struct
        {
            struct AST_Node* proc;
            struct AST_Node* args; // NOTE: 0 means no arguments
        } call_expr;
        
        struct
        {
            struct AST_Node* size;
            struct AST_Node* elem_type;
        } array_type;
        
        struct
        {
            struct AST_Node* elem_type;
        } slice_type;
        
        struct
        {
            struct AST_Node* type; // NOTE: 0 means infered type
            struct AST_Node* args; // NOTE: 0 means all default values
        } struct_literal;
        
        struct
        {
            struct AST_Node* type; // NOTE: 0 means infered type
            struct AST_Node* args; // NOTE: 0 means all default values
        } array_literal;
        
        struct AST_Node* unary_expr;
        
        struct
        {
            struct AST_Node* left;
            struct AST_Node* right;
        } binary_expr;
        
        struct
        {
            struct AST_Node* condition;
            struct AST_Node* true_expr;
            struct AST_Node* false_expr;
        } conditional_expr;
        
        struct
        {
            struct AST_Node* body; // NOTE: 0 means empty block
            Interned_String label; // NOTE: 0 means no label
        } block_statement;
        
        struct
        {
            struct AST_Node* init;       // NOTE: 0 means no init, can be either an expression with side effects, an assignment or a declaration
            struct AST_Node* condition;  // NOTE: can only be an expression with a boolean type
            struct AST_Node* true_body;  // NOTE: 0 means no body, can otherwise be either a block_statement or statement, but not a declaration
            struct AST_Node* false_body; // NOTE: 0 means no body, can otherwise be either a block_statement or statement, but not a declaration
            Interned_String label;       // NOTE: 0 means no label
        } if_statement;
        
        struct
        {
            struct AST_Node* init;      // NOTE: 0 means no init, can be either an expression with side effects, an assignment or a declaration
            struct AST_Node* condition; // NOTE: can only be an expression with a boolean type
            struct AST_Node* step;      // NOTE: 0 means no step
            struct AST_Node* body;      // NOTE: 0 means no body, can otherwise be either a block_statement or statement, but not a declaration
            Interned_String label;      // NOTE: 0 means no label
        } while_statement;
        
        Interned_String jump_label;    // NOTE: 0 means no label
        
        struct AST_Node* defer_statement; // NOTE: can only be a statement, or an expression with side effects
        
        struct AST_Node* return_values; // NOTE: 0 means no return values
        
        struct
        {
            struct AST_Node* lhs;
            struct AST_Node* rhs;
            AST_NODE_KIND op; // NOTE: 0 means regular assignment and not e.g. +=
        } assignment_statement;
        
        struct
        {
            struct AST_Node* names;
            struct AST_Node* type;   // NOTE: 0 means infered type
            struct AST_Node* values; // NOTE: 0 means default initialized
            bool is_using;
            bool is_uninitialized;
        } variable_decl;
        
        struct
        {
            struct AST_Node* names;
            struct AST_Node* type;  // NOTE: 0 means infered type
            struct AST_Node* values;
            bool is_using;
        } constant_decl;
        
        struct
        {
            struct AST_Node* symbol_paths;
            Interned_String alias; // NOTE: 0 means no alias
        } using_decl;
    };
} AST_Node;