typedef enum MM_BUILTIN_KIND
{
    MM_Builtin_Cast,
    MM_Builtin_Transmute,
    MM_Builtin_Sizeof,
    MM_Builtin_Alignof,
    MM_Builtin_Offsetof,
    MM_Builtin_Typeof,
} MM_BUILTIN_KIND;

#define MM_AST_KIND_BLOCK_SIZE 16
#define MM_AST_BLOCK_INDEX_FROM_KIND(kind) ((kind) / MM_AST_KIND_BLOCK_SIZE)
#define MM_AST_BLOCK_OFFSET_FROM_KIND(kind) ((kind) / MM_AST_KIND_BLOCK_SIZE)

typedef enum MM_AST_KIND
{
    MM_AST_Invalid = 0,
    
    MM_AST_FirstExpression,
    MM_AST_FirstSpecialExpression = MM_AST_FirstExpression,
    MM_AST_Enum_Member = MM_AST_FirstSpecialExpression,
    MM_AST_Argument,
    MM_AST_Parameter,
    MM_AST_LastSpecialExpression = MM_AST_Parameter,
    
    MM_AST_FirstPrimaryExpression,
    MM_AST_Identifier = MM_AST_FirstPrimaryExpression,
    MM_AST_Int,
    MM_AST_Float,
    MM_AST_String,
    MM_AST_Codepoint,
    MM_AST_Bool,
    MM_AST_Proc,
    MM_AST_ProcLiteral,
    MM_AST_Struct,
    MM_AST_Union,
    MM_AST_Enum,
    MM_AST_Builtin,
    MM_AST_Compound,
    MM_AST_LastPrimaryExpression = MM_AST_Compound,
    
    MM_AST_FirstPostfix,
    MM_AST_Dereference = MM_AST_FirstPostfix,
    MM_AST_MemberAccess,
    MM_AST_Subscript,
    MM_AST_Slice,
    MM_AST_Call,
    MM_AST_StructLiteral,
    MM_AST_ArrayLiteral,
    MM_AST_LastPostfix = MM_AST_Call,
    
    MM_AST_FirstPrefix,
    MM_AST_Reference = MM_AST_FirstPrefix,
    MM_AST_ArrayType,
    MM_AST_SliceType,
    MM_AST_Neg,
    MM_AST_BitNot,
    MM_AST_Not,
    MM_AST_LastPrefix = MM_AST_Not,
    
    MM_AST_FirstBinary = 6*16,
    MM_AST_FirstMulLevel = MM_AST_FirstBinary,
    MM_AST_Mul = MM_AST_FirstMulLevel,
    MM_AST_Div,
    MM_AST_Rem,
    MM_AST_BitAnd,
    MM_AST_BitShr,
    MM_AST_BitShl,
    MM_AST_BitSar,
    MM_AST_LastMulLevel = MM_AST_BitSar,
    
    MM_AST_FirstAddLevel = 7*16,
    MM_AST_BitXor = MM_AST_FirstAddLevel,
    MM_AST_BitOr,
    MM_AST_Sub,
    MM_AST_Add,
    MM_AST_LastAddLevel = MM_AST_Add,
    
    MM_AST_FirstCmpLevel = 8*16,
    MM_AST_IsGreater = MM_AST_FirstCmpLevel,
    MM_AST_IsGreaterEquals,
    MM_AST_IsLess,
    MM_AST_IsLessEquals,
    MM_AST_IsEqual,
    MM_AST_IsNotEqual,
    MM_AST_LastCmpLevel = MM_AST_IsNotEqual,
    
    MM_AST_And = 9*16,
    
    MM_AST_Or = 10*16,
    MM_AST_LastBinary = MM_AST_Or,
    
    MM_AST_Conditional,
    MM_AST_LastExpression = MM_AST_Conditional,
    
    MM_AST_FirstDeclaration,
    MM_AST_Variable,
    MM_AST_Constant,
    MM_AST_Using,
    MM_AST_LastDeclaration = MM_AST_Using,
    
    MM_AST_FirstStatement,
    MM_AST_Block,
    MM_AST_If,
    MM_AST_While,
    MM_AST_Break,
    MM_AST_Continue,
    MM_AST_Return,
    MM_AST_Assignment,
    MM_AST_LastStatement = MM_AST_Assignment,
    
} MM_AST_KIND;

typedef struct MM_Proc_Header
{
    // IMPORTANT HACK
    struct MM_Parameter* params;
    struct MM_Parameter* return_values;
} MM_Proc_Header;

typedef MM_Proc_Header MM_Proc_Expr;
typedef struct MM_Proc_Lit_Expr
{
    union
    {
        struct MM_Proc_Header;
        MM_Proc_Header header;
    };
    
    struct MM_Block_Statement* body;
} MM_Proc_Lit_Expr;

typedef struct MM_Struct_Expr
{
    struct MM_Block_Statement* body;
} MM_Struct_Expr;

typedef struct MM_Enum_Expr
{
    struct MM_Expression* member_type;
    struct MM_Enum_Member* members;
} MM_Enum_Expr;

typedef struct MM_Builtin_Expr
{
    MM_BUILTIN_KIND kind;
    struct MM_Argument* args;
} MM_Builtin_Expr;

typedef struct MM_Member_Access_Expr
{
    struct MM_Expression* symbol;
    MM_String member;
} MM_Member_Access_Expr;

typedef struct MM_Subscript_Expr
{
    struct MM_Expression* array;
    struct MM_Expression* index;
} MM_Subscript_Expr;

typedef struct MM_Slice_Expr
{
    struct MM_Expression* array;
    struct MM_Expression* start_index;
    struct MM_Expression* past_end_index;
} MM_Slice_Expr;

typedef struct MM_Call_Expr
{
    struct MM_Expression* proc;
    struct MM_Argument* args;
} MM_Call_Expr;

typedef struct MM_Struct_Lit_Expr
{
    struct MM_Expression* type;
    struct MM_Argument* args;
} MM_Struct_Lit_Expr;

typedef struct MM_Array_Lit_Expr
{
    struct MM_Expression* type;
    struct MM_Argument* args;
} MM_Array_Lit_Expr;

typedef struct MM_Array_Type_Expr
{
    struct MM_Expression* size;
    struct MM_Expression* elem_type;
} MM_Array_Type_Expr;

typedef struct MM_Binary_Expr
{
    struct MM_Expression* left;
    struct MM_Expression* right;
} MM_Binary_Expr;

typedef struct MM_Conditional_Expr
{
    struct MM_Expression* condition;
    struct MM_Expression* true_expr;
    struct MM_Expression* false_expr;
} MM_Conditional_Expr;

typedef union MM_Expression_Union
{
    MM_String identifier;
    MM_i128 integer;
    MM_f64 floating;
    MM_String string_literal;
    MM_u32 codepoint;
    MM_bool boolean;
    
    MM_Proc_Expr proc_expr;
    MM_Proc_Lit_Expr proc_lit_expr;
    MM_Struct_Expr struct_expr;
    MM_Struct_Expr union_expr;
    MM_Enum_Expr enum_expr;
    MM_Builtin_Expr builtin_expr;
    struct MM_Expression* compound_expr;
    struct MM_Expression* unary_expr;
    MM_Member_Access_Expr member_access_expr;
    MM_Subscript_Expr subscript_expr;
    MM_Slice_Expr slice_expr;
    MM_Call_Expr call_expr;
    MM_Struct_Lit_Expr struct_lit_expr;
    MM_Array_Lit_Expr array_lit_expr;
    MM_Array_Type_Expr array_type_expr;
    MM_Binary_Expr binary_expr;
    MM_Conditional_Expr conditional_expr;
} MM_Expression_Union;

typedef struct MM_Variable_Decl
{
    struct MM_Expression* names;
    struct MM_Expression* types;
    struct MM_Expression* values;
    MM_bool is_using;
    MM_bool is_uninitialized;
} MM_Variable_Decl;

typedef struct MM_Constant_Decl
{
    struct MM_Expression* names;
    struct MM_Expression* types;
    struct MM_Expression* values;
    MM_bool is_using;
} MM_Constant_Decl;

typedef struct MM_Using_Decl
{
    struct MM_Expression* symbols;
    struct MM_Expression* aliases;
} MM_Using_Decl;

typedef union MM_Declaration_Union
{
    MM_Variable_Decl variable_decl;
    MM_Constant_Decl constant_decl;
    MM_Using_Decl using_decl;
} MM_Declaration_Union;

typedef struct MM_Block_Statement
{
    MM_String label;
    struct MM_Statement* body;
} MM_Block_Statement;

typedef struct MM_Assignment_Statement
{
    struct MM_Expression* lhs;
    struct MM_Expression* rhs;
    MM_AST_KIND op;
} MM_Assignment_Statement;

typedef struct MM_If_Statement
{
    MM_String label;
    struct MM_DeclAssignmentOrExpression* init;
    struct MM_Expression* condition;
    struct MM_Statement* true_body;
    struct MM_Statement* false_body;
} MM_If_Statement;

typedef struct MM_While_Statement
{
    MM_String label;
    struct MM_DeclAssignmentOrExpression* init;
    struct MM_Expression* condition;
    struct MM_AssignmentOrExpression* step;
    struct MM_Statement* body;
} MM_While_Statement;

typedef struct MM_Jump_Statement
{
    MM_String label;
} MM_Jump_Statement;

typedef MM_Jump_Statement MM_Break_Statement;
typedef MM_Jump_Statement MM_Continue_Statement;

typedef struct MM_Return_Statement
{
    struct MM_Argument* args;
} MM_Return_Statement;

typedef union MM_Statement_Union
{
    MM_Block_Statement block_statement;
    MM_If_Statement if_statement;
    MM_While_Statement while_statement;
    MM_Jump_Statement jump_statement;
    MM_Break_Statement break_statement;
    MM_Continue_Statement continue_statement;
    MM_Return_Statement return_statement;
    MM_Assignment_Statement assignment_statement;
} MM_Statement_Union;

typedef struct MM_Expression
{
    MM_AST_KIND kind;
    struct MM_Expression* next;
    
    union MM_Expression_Union;
} MM_Expression;

typedef struct MM_Declaration
{
    MM_AST_KIND kind;
    struct MM_Declaration* next;
    
    union MM_Declaration_Union;
} MM_Declaration;

typedef struct MM_Statement
{
    MM_AST_KIND kind;
    struct MM_Statement* next;
    
    union
    {
        union MM_Expression_Union;
        union MM_Declaration_Union;
        union MM_Statement_Union;
    };
} MM_Statement;

typedef struct  MM_AssignmentOrExpression
{
    MM_AST_KIND kind;
    
    union
    {
        MM_Assignment_Statement assignment_statement;
        union MM_Expression_Union;
    };
} MM_AssignmentOrExpression;

typedef struct MM_DeclAssignmentOrExpression
{
    MM_AST_KIND kind;
    
    union
    {
        union MM_Declaration_Union;
        MM_Assignment_Statement assignment_statement;
        union MM_Expression_Union;
    };
} MM_DeclAssignmentOrExpression;

typedef union MM_Enum_Member_Union
{
    struct
    {
        MM_String name;
        struct MM_Expression* value;
    };
} MM_Enum_Member_Union;

typedef struct MM_Enum_Member
{
    MM_AST_KIND kind;
    struct MM_Enum_Member* next;
    
    union MM_Enum_Member_Union;
} MM_Enum_Member;

typedef union MM_Argument_Union
{
    struct
    {
        struct MM_Expression* name;
        struct MM_Expression* value;
    };
} MM_Argument_Union;

typedef struct MM_Argument
{
    MM_AST_KIND kind;
    struct MM_Argument* next;
    
    union MM_Argument_Union;
} MM_Argument;

typedef union MM_Parameter_Union
{
    struct
    {
        struct MM_Expression* names;
        struct MM_Expression* type;
        struct MM_Expression* value;
        MM_bool is_using;
    };
} MM_Parameter_Union;

typedef struct MM_Parameter
{
    MM_AST_KIND kind;
    struct MM_Parameter* next;
    
    union MM_Parameter_Union;
} MM_Parameter;

typedef union MM_AST
{
    struct
    {
        MM_AST_KIND kind;
        
        union
        {
            union MM_Expression_Union;
            union MM_Declaration_Union;
            union MM_Statement_Union;
            
            struct
            {
                union MM_Enum_Member_Union;
            } enum_member;
            
            struct
            {
                union MM_Argument_Union;
            } argument;
            
            struct
            {
                union MM_Parameter_Union;
            } parameter;
        };
    };
    
    MM_Expression expression;
    MM_Declaration declaration;
    MM_Statement statement;
    MM_Enum_Member enum_member_wrapper;
    MM_Argument argument_wrapper;
    MM_Parameter parameter_wrapper;
} MM_AST;