typedef enum AST_NODE_KIND
{
    AST_Invalid = 0,
    
    // NOTE: These nodes are neither expressions, statements nor declarations.
    //       They only exist to make some things easier to handle, e.g.
    //       return .a = 0, .b = 1;
    //       and
    //       int.[0..50 = 0, 51..63 = 1]
    AST_FirstSpecial,
    AST_NamedValue = AST_FirstSpecial,
    AST_Range,
    AST_PolyConstant,
    AST_LastSpecial = AST_Range,
    
    AST_FirstExpression,
    AST_Identifier = AST_FirstExpression,
    AST_String,
    AST_Int,
    AST_Float,
    AST_Boolean,
    AST_Proc,
    AST_ProcLiteral,
    AST_Struct,
    AST_Union,
    AST_Enum,
    AST_Compound,
    AST_Selector,
    AST_StructLiteral,
    AST_ArrayLiteral,
    AST_Cast,
    AST_Conditional,
    
    AST_FirstPostfixLevel,
    AST_Call = AST_FirstPostfixLevel,
    AST_ElementOf,
    AST_Subscript,
    AST_Slice,
    AST_LastPostfixLevel,
    
    AST_FirstTypeLevel,
    AST_SliceType = AST_FirstTypeLevel,
    AST_ArrayType,
    AST_Dereference,
    AST_Reference,
    AST_LastTypeLevel = AST_Reference,
    
    AST_FirstPrefixLevel,
    AST_Not = AST_FirstPrefixLevel,
    AST_BitNot,
    AST_Neg,
    AST_LastPrefixLevel,
    
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
    AST_Block,
    AST_If,
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
    AST_Include,
    AST_When,
    AST_LastDeclaration = AST_When,
    
    AST_NODE_KIND_MAX = 256,
} AST_NODE_KIND;

// NOTE: This is used for parsing of binary expressions
#define AST_EXPR_BLOCK_FROM_KIND(kind) ((kind) >> 4)

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
        
        struct
        {
            struct AST_Node* start;
            struct AST_Node* end;
            bool is_open;
        } range;
        
        struct
        {
            struct AST_Node* operand;
        } poly;
        
        struct
        {
            Interned_String name;
        } identifier;
        
        Interned_String string;
        Big_Int integer;
        Big_Float floating;
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
            struct AST_Node* element;
        } selector;
        
        struct
        {
            struct AST_Node* member_type;
            struct AST_Node* members;
        } enum_type;
        
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
            struct AST_Node* init;
            struct AST_Node* condition;
            struct AST_Node* true_body;
            struct AST_Node* false_body;
            Interned_String label;
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
            Interned_String label;
        } while_statement;
        
        struct
        {
            Interned_String label;
        } jmp_statement;
        
        struct AST_Node* defer_statement;
        
        struct
        {
            struct AST_Node* args;
        } return_statement;
        
        struct
        {
            struct AST_Node* expr;
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
        
        struct
        {
            Interned_String path;
        } include_decl;
    };
} AST_Node;