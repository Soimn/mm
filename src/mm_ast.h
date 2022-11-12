// NOTE: This is mainly to simplify parsing (it matches up with MM_TOKEN_BLOCK in mm_tokens.h)
#define MM_AST_BLOCK(I) ((I) << 4)
#define MM_AST_BLOCK_INDEX(K) ((K) >> 4)
#define MM_AST_BLOCK_OFFSET(K) ((K) & 0xF)
#define MM_AST_CMP_BLOCK_INDEX 11

typedef enum MM_AST_Kind
{
    MM_AST_Invalid = 0,
    
    MM_AST_Argument,
    MM_AST_Parameter,
    MM_AST_ReturnValue,
    
    MM_AST__FirstExpression,
    MM_AST__FirstPrimary = MM_AST__FirstExpression,
    MM_AST_Identifier = MM_AST__FirstPrimary,
    MM_AST_Int,
    MM_AST_Float,
    MM_AST_String,
    MM_AST_Bool,
    MM_AST_ProcType,
    MM_AST_ProcLit,
    MM_AST_StructType,
    MM_AST_Compound,
    MM_AST_BuiltinCall,
    MM_AST__LastPrimary = MM_AST_BuiltinCall,
    
    MM_AST__FirstTypePrefix,
    MM_AST_ArrayType = MM_AST__FirstTypePrefix,
    MM_AST_SliceType,
    MM_AST_PointerType,
    MM_AST__LastTypePrefix = MM_AST_PointerType,
    
    MM_AST__FirstPostfixUnary,
    MM_AST_Dereference = MM_AST__FirstPostfixUnary,
    MM_AST_Subscript,
    MM_AST_Slice,
    MM_AST_Call,
    MM_AST_Member,
    MM_AST_StructLit,
    MM_AST_ArrayLit,
    MM_AST__LastPostfixUnary = MM_AST_ArrayLit,
    
    MM_AST__FirstPrefixUnary,
    MM_AST_Pos = MM_AST__FirstPrefixUnary,
    MM_AST_Neg,
    MM_AST_Not,
    MM_AST_BitNot,
    MM_AST_Reference,
    MM_AST__LastPrefixUnary = MM_AST_Reference,
    
    MM_AST__FirstBinary = MM_AST_BLOCK(9),
    MM_AST_Mul = MM_AST__FirstBinary,
    MM_AST_Div,
    MM_AST_Rem,
    MM_AST_BitAnd,
    MM_AST_BitShl,
    MM_AST_BitShr,
    MM_AST_BitSar,
    
    MM_AST_Add = MM_AST_BLOCK(10),
    MM_AST_Sub,
    MM_AST_BitOr,
    MM_AST_BitXor,
    
    MM_AST_CmpEqual = MM_AST_BLOCK(11),
    MM_AST_CmpNotEQ,
    MM_AST_CmpLess,
    MM_AST_CmpLessEQ,
    MM_AST_CmpGreater,
    MM_AST_CmpGreaterEQ,
    
    MM_AST_And = MM_AST_BLOCK(12),
    
    MM_AST_Or = MM_AST_BLOCK(13),
    MM_AST__LastBinary = MM_AST_Or,
    MM_AST__LastExpression = MM_AST__LastBinary,
    
    MM_AST__FirstDeclaration,
    MM_AST_Var = MM_AST__FirstDeclaration,
    MM_AST_Const,
    MM_AST__LastDeclaration = MM_AST_Const,
    
    MM_AST__FirstStatement,
    MM_AST_Block = MM_AST__FirstStatement,
    MM_AST_If,
    MM_AST_When,
    MM_AST_While,
    MM_AST_Break,
    MM_AST_Continue,
    MM_AST_Return,
    MM_AST_Assignment,
    MM_AST__LastStatement = MM_AST_Assignment,
} MM_AST_Kind;

typedef enum MM_BuiltinCall_Kind
{
    MM_BuiltinCall_Cast = 0,
    MM_BuiltinCall_Transmute,
    MM_BuiltinCall_Sizeof,
    MM_BuiltinCall_Alignof,
    MM_BuiltinCall_Offsetof,
} MM_BuiltinCall_Kind;

MM_STATIC_ASSERT(MM_AST_BLOCK(MM_AST_CMP_BLOCK_INDEX) == MM_AST_CmpEqual);

typedef struct MM_Expression MM_Expression;
typedef struct MM_Declaration MM_Declaration;
typedef struct MM_Statement MM_Statement;

#define MM_AST_HEADER(next_t) struct { MM_AST_Kind kind; struct next_t* next; }
#define MM_EXPRESSION_HEADER MM_AST_HEADER(MM_Expression)
#define MM_DECLARATION_HEADER MM_AST_HEADER(MM_Declaration)
#define MM_STATEMENT_HEADER MM_AST_HEADER(MM_Statement)

typedef struct MM_Argument
{
    MM_AST_HEADER(MM_Argument);
    MM_Expression* name;
    MM_Expression* value;
} MM_Argument;

typedef struct MM_Parameter
{
    MM_AST_HEADER(MM_Parameter);
    MM_Expression* names;
    MM_Expression* type;
    MM_Expression* value;
} MM_Parameter;

typedef struct MM_Return_Value
{
    MM_AST_HEADER(MM_Return_Value);
    MM_Expression* names;
    MM_Expression* type;
    MM_Expression* value;
} MM_Return_Value;


typedef struct MM_Identifier_Expression
{
    MM_EXPRESSION_HEADER;
    MM_String value;
} MM_Identifier_Expression;

typedef struct MM_Int_Expression
{
    MM_EXPRESSION_HEADER;
    MM_i128 value;
} MM_Int_Expression;

typedef struct MM_Float_Expression
{
    MM_EXPRESSION_HEADER;
    MM_f64 value;
} MM_Float_Expression;

typedef struct MM_String_Expression
{
    MM_EXPRESSION_HEADER;
    MM_String value;
} MM_String_Expression;

typedef struct MM_Bool_Expression
{
    MM_EXPRESSION_HEADER;
    MM_bool value;
} MM_Bool_Expression;

typedef struct MM_Proc_Type_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Parameter* params;
    MM_Return_Value* return_vals;
} MM_Proc_Type_Expression;

typedef struct MM_Proc_Lit_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Parameter* params;
    MM_Return_Value* return_vals;
    struct MM_Block_Statement* body;
} MM_Proc_Lit_Expression;

typedef struct MM_Struct_Type_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Declaration* members;
} MM_Struct_Type_Expression;

typedef struct MM_Compound_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Expression* expr;
} MM_Compound_Expression;

typedef struct MM_Builtin_Call_Expression
{
    MM_EXPRESSION_HEADER;
    MM_BuiltinCall_Kind builtin_kind;
    MM_Argument* args;
} MM_Builtin_Call_Expression;

typedef struct MM_Binary_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Expression* left;
    MM_Expression* right;
} MM_Binary_Expression;

typedef struct MM_Unary_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Expression* operand;
} MM_Unary_Expression;

typedef struct MM_Array_Type_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Expression* size;
    MM_Expression* type;
} MM_Array_Type_Expression;

typedef struct MM_Subscript_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Expression* array;
    MM_Expression* index;
} MM_Subscript_Expression;

typedef struct MM_Slice_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Expression* array;
    MM_Expression* start_index;
    MM_Expression* past_end_index;
} MM_Slice_Expression;

typedef struct MM_Call_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Expression* proc;
    MM_Argument* args;
} MM_Call_Expression;

typedef struct MM_Member_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Expression* symbol;
    MM_String member;
} MM_Member_Expression;

typedef struct MM_Struct_Lit_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Expression* type;
    MM_Argument* args;
} MM_Struct_Lit_Expression;

typedef struct MM_Array_Lit_Expression
{
    MM_EXPRESSION_HEADER;
    MM_Expression* type;
    MM_Argument* args;
} MM_Array_Lit_Expression;

typedef struct MM_Expression
{
    union
    {
        MM_EXPRESSION_HEADER;
        
        MM_Identifier_Expression identifier_expr;
        MM_Int_Expression int_expr;
        MM_Float_Expression float_expr;
        MM_String_Expression string_expr;
        MM_Bool_Expression bool_expr;
        MM_Proc_Type_Expression proc_type_expr;
        MM_Proc_Lit_Expression proc_lit_expr;
        MM_Struct_Type_Expression struct_type_expr;
        MM_Compound_Expression compound_expr;
        MM_Builtin_Call_Expression builtin_call_expr;
        MM_Binary_Expression binary_expr;
        MM_Unary_Expression unary_expr;
        MM_Array_Type_Expression array_type_expr;
        MM_Subscript_Expression subscript_expr;
        MM_Slice_Expression slice_expr;
        MM_Call_Expression call_expr;
        MM_Member_Expression member_expr;
        MM_Struct_Lit_Expression struct_lit_expr;
        MM_Array_Lit_Expression array_lit_expr;
    };
} MM_Expression;



typedef struct MM_Variable_Declaration
{
    MM_DECLARATION_HEADER;
    MM_Expression* names;
    MM_Expression* type;
    MM_Expression* values;
    MM_bool is_uninitialized;
} MM_Variable_Declaration;

typedef struct MM_Constant_Declaration
{
    MM_DECLARATION_HEADER;
    MM_Expression* names;
    MM_Expression* type;
    MM_Expression* values;
} MM_Constant_Declaration;

typedef struct MM_When_Declaration
{
    MM_DECLARATION_HEADER;
    MM_Expression* condition;
    MM_Statement* true_body;
    MM_Statement* false_body;
} MM_When_Declaration;

typedef struct MM_Declaration
{
    union
    {
        MM_DECLARATION_HEADER;
        
        MM_Variable_Declaration var_decl;
        MM_Constant_Declaration const_decl;
        MM_When_Declaration when_decl;
    };
} MM_Declaration;



typedef struct MM_Block_Statement
{
    MM_STATEMENT_HEADER;
    MM_String label;
    MM_Statement* body;
} MM_Block_Statement;

typedef struct MM_If_Statement
{
    MM_STATEMENT_HEADER;
    MM_String label;
    MM_Statement* init; // var, const, assignment, expression
    MM_Expression* condition;
    MM_Statement* true_body;
    MM_Statement* false_body;
} MM_If_Statement;

typedef struct MM_While_Statement
{
    MM_STATEMENT_HEADER;
    MM_String label;
    MM_Statement* init; // var, const, assignment, expression
    MM_Expression* condition;
    MM_Statement* step; // assignment, expression
    MM_Statement* body;
} MM_While_Statement;

typedef struct MM_Jump_Statement
{
    MM_STATEMENT_HEADER;
    MM_String label;
} MM_Jump_Statement;

typedef struct MM_Return_Statement
{
    MM_STATEMENT_HEADER;
    MM_Argument* args;
} MM_Return_Statement;

typedef struct MM_Assignment_Statement
{
    MM_STATEMENT_HEADER;
    MM_AST_Kind assign_op;
    MM_Expression* lhs;
    MM_Expression* rhs;
} MM_Assignment_Statement;

typedef struct MM_Statement
{
    union
    {
        MM_STATEMENT_HEADER;
        MM_Expression expression;
        MM_Declaration declaration;
        
        MM_Block_Statement block_stmnt;
        MM_If_Statement if_stmnt;
        MM_While_Statement while_stmnt;
        MM_Jump_Statement jump_stmnt;
        MM_Return_Statement return_stmnt;
        MM_Assignment_Statement assignment_stmnt;
    };
} MM_Statement;




typedef struct MM_AST
{
    union
    {
        MM_AST_HEADER(MM_AST);
        
        MM_Argument argument;
        MM_Parameter parameter;
        MM_Return_Value return_value;
        MM_Expression expression;
        MM_Declaration declaration;
        MM_Statement statement;
    };
} MM_AST;