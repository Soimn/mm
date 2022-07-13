#define MM_AST_FIRST_KIND(type, group, sub_group) (((type) << 24) | ((group) << 16) | ((sub_group) << 8) | 1)

enum MM_AST
{
    MM_AST_None = 0,
    
    // NOTE: Types
    MM_ASTType_Argument = 1,
    MM_ASTType_Parameter,
    MM_ASTType_ReturnValue,
    MM_ASTType_StructMember,
    MM_ASTType_UnionMember,
    MM_ASTType_EnumMember,
    MM_ASTType_Expression,
    MM_ASTType_Declaration,
    MM_ASTType_Statement,
    
    // NOTE: Groups under MM_AST_Expression
    MM_ASTGroup_PrimaryExpression = 1,
    MM_ASTGroup_UnaryExpression,
    MM_ASTGroup_BinaryExpression,
    MM_ASTGroup_ConditionalExpression,
    
    // NOTE: Sub groups under MM_ASTType_Expression, MM_ASTGroup_PrimaryExpression
    MM_ASTSubGroup_Builtin,
    MM_ASTSubGroup_FwdDecl,
    
    // NOTE: Sub groups under MM_ASTType_Expression, MM_ASTGroup_UnaryExpression
    MM_ASTSubGroup_PostfixUnary = 1,
    MM_ASTSubGroup_PrefixUnary,
    
    // NOTE: Sub groups under MM_ASTType_Expression, MM_ASTGroup_BinaryExpression
    MM_ASTSubGroup_MulLevel = 1,
    MM_ASTSubGroup_AddLevel,
    MM_ASTSubGroup_CmpLevel,
    MM_ASTSubGroup_AndLevel,
    MM_ASTSubGroup_OrLevel,
    
    // NOTE: Groups under MM_AST_Statement
    MM_ASTGroup_Jump,
    MM_ASTGroup_Assignment,
    
    MM_AST_Identifier = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_PrimaryExpression, MM_AST_None),
    MM_AST_Int,
    MM_AST_Float,
    MM_AST_String,
    MM_AST_Codepoint,
    MM_AST_Bool,
    MM_AST_Compound,
    MM_AST_This,
    MM_AST_Proc,
    MM_AST_ProcLiteral,
    MM_AST_ProcSet,
    MM_AST_Struct,
    MM_AST_Union,
    MM_AST_Enum,
    
    MM_AST_ProcLiteralFwdDecl = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_PrimaryExpression, MM_ASTSubGroup_FwdDecl),
    MM_AST_ProcSetFwdDecl,
    MM_AST_StructFwdDecl,
    MM_AST_UnionFwdDecl,
    MM_AST_EnumFwdDecl,
    
    MM_AST_BuiltinCast = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_PrimaryExpression, MM_ASTSubGroup_Builtin),
    MM_AST_BuiltinTransmute,
    MM_AST_BuiltinSizeof,
    MM_AST_BuiltinAlignof,
    MM_AST_BuiltinOffsetof,
    MM_AST_BuiltinTypeof,
    MM_AST_BuiltinShadowed,
    MM_AST_BuiltinMin,
    MM_AST_BuiltinMax,
    MM_AST_BuiltinLen,
    
    MM_AST_Dereference = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_UnaryExpression, MM_ASTSubGroup_PostfixUnary),
    MM_AST_MemberAccess,
    MM_AST_Subscript,
    MM_AST_Slice,
    MM_AST_Call,
    MM_AST_StructLiteral,
    MM_AST_ArrayLiteral,
    
    MM_AST_Reference = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_UnaryExpression, MM_ASTSubGroup_PrefixUnary),
    MM_AST_ArrayType,
    MM_AST_SliceType,
    MM_AST_Neg,
    MM_AST_BitNot,
    MM_AST_Not,
    
    MM_AST_Mul = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_BinaryExpression, MM_ASTSubGroup_MulLevel),
    MM_AST_Div,
    MM_AST_Rem,
    MM_AST_BitAnd,
    MM_AST_BitShl,
    MM_AST_BitShr,
    MM_AST_BitSar,
    
    MM_AST_Add = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_BinaryExpression, MM_ASTSubGroup_AddLevel),
    MM_AST_Sub,
    MM_AST_BitOr,
    MM_AST_BitXor,
    
    MM_AST_CmpGreater = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_BinaryExpression, MM_ASTSubGroup_CmpLevel),
    MM_AST_CmpGreaterEquals,
    MM_AST_CmpLess,
    MM_AST_CmpLessEquals,
    MM_AST_CmpEquals,
    MM_AST_CmpNotEquals,
    
    MM_AST_And = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_BinaryExpression, MM_ASTSubGroup_AndLevel),
    
    MM_AST_Or = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_BinaryExpression, MM_ASTSubGroup_OrLevel),
    
    MM_AST_Conditional = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_ConditionalExpression, MM_AST_None),
    
    MM_AST_Variable = MM_AST_FIRST_KIND(MM_ASTType_Declaration, MM_AST_None, MM_AST_None),
    MM_AST_Constant,
    MM_AST_Using,
    MM_AST_When,
    
    MM_AST_Block = MM_AST_FIRST_KIND(MM_ASTType_Statement, MM_AST_None, MM_AST_None),
    MM_AST_If,
    MM_AST_While,
    MM_AST_Return,
    MM_AST_Defer,
    
    MM_AST_Continue = MM_AST_FIRST_KIND(MM_ASTType_Statement, MM_ASTGroup_Jump, MM_AST_None),
    MM_AST_Break,
    
    MM_AST_Assignment = MM_AST_FIRST_KIND(MM_ASTType_Statement, MM_ASTGroup_Assignment, MM_AST_None),
    MM_AST_MulAssignment,
    MM_AST_DivAssignment,
    MM_AST_RemAssignment,
    MM_AST_BitAndAssignment,
    MM_AST_BitShlAssignment,
    MM_AST_BitShrAssignment,
    MM_AST_BitSarAssignment,
    MM_AST_AddAssignment,
    MM_AST_SubAssignment,
    MM_AST_BitOrAssignment,
    MM_AST_BitXorAssignment,
    MM_AST_AndAssignment,
    MM_AST_OrAssignment,
};

#undef MM_AST_FIRST_KIND

typedef struct MM_AST_Kind
{
    MM_u32 kind;
    struct
    {
        MM_u8 kind_index;
        MM_u8 kind_sub_group;
        MM_u8 kind_group;
        MM_u8 kind_type;
    };
} MM_AST_Kind;

// TODO:
typedef MM_String MM_Identifier;
typedef MM_String MM_String_Literal;
typedef MM_i128 MM_Soft_Int;
typedef MM_f64 MM_Soft_Float;
//

typedef struct MM_Argument
{
    struct MM_AST_Kind;
    struct MM_Argument* next;
    struct MM_Expression* name;
    struct MM_Expression* value;
} MM_Argument;

typedef struct MM_Parameter
{
    struct MM_AST_Kind;
    struct MM_Parameter* next;
    struct MM_Expression* names;
    struct MM_Expression* type;
    struct MM_Expression* values;
} MM_Parameter;

typedef struct MM_Return_Value
{
    struct MM_AST_Kind;
    struct MM_Return_Value* next;
    struct MM_Expression* names;
    struct MM_Expression* type;
    struct MM_Expression* values;
} MM_Return_Value;

typedef struct MM_Struct_Member
{
    struct MM_AST_Kind;
    struct MM_Struct_Member* next;
    struct MM_Expression* names;
    struct MM_Expression* type;
} MM_Struct_Member;

typedef struct MM_Union_Member
{
    struct MM_AST_Kind;
    struct MM_Union_Member* next;
    struct MM_Expression* names;
    struct MM_Expression* type;
} MM_Union_Member;

typedef struct MM_Enum_Member
{
    struct MM_AST_Kind;
    struct MM_Enum_Member* next;
    struct MM_Expression* name;
    struct MM_Expression* value;
} MM_Enum_Member;

typedef struct MM_Expression MM_Expression;

typedef struct MM_Expression_Header
{
    struct MM_AST_Kind;
    MM_Expression* next;
} MM_Expression_Header;

#define MM_EXPRESSION_HEADER() union { struct MM_Expression_Header; MM_Expression_Header header; }

typedef struct MM_Compound_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* body;
} MM_Compound_Expression;

typedef struct MM_Proc_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Parameter* params;
    MM_Return_Value* return_vals;
} MM_Proc_Expression;

typedef struct MM_Proc_Literal_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Parameter* params;
    MM_Return_Value* return_vals;
    struct MM_Statement* body;
} MM_Proc_Literal_Expression;

typedef struct MM_Proc_Set_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* members;
} MM_Proc_Set_Expression;

typedef struct MM_Struct_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Struct_Member* members;
} MM_Struct_Expression;

typedef struct MM_Union_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Union_Member* members;
} MM_Union_Expression;

typedef struct MM_Enum_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* member_type;
    MM_Enum_Member* members;
} MM_Enum_Expression;

typedef struct MM_Proc_Literal_Fwd_Decl_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Parameter* params;
    MM_Return_Value* return_vals;
} MM_Proc_Literal_Fwd_Decl_Expression;

typedef struct MM_Enum_Fwd_Decl_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* member_type;
} MM_Enum_Fwd_Decl_Expression;

typedef struct MM_Builtin_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Argument* args;
} MM_Builtin_Expression;

typedef struct MM_Member_Access_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* symbol;
    MM_Identifier member;
} MM_Member_Access_Expression;

typedef struct MM_Subscript_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* array;
    MM_Expression* index;
} MM_Subscript_Expression;

typedef struct MM_Slice_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* array;
    MM_Expression* start;
    MM_Expression* past_end;
} MM_Slice_Expression;

typedef struct MM_Call_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* proc;
    MM_Argument* args;
} MM_Call_Expression;

typedef struct MM_Struct_Litteral_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* type;
    MM_Argument* args;
} MM_Struct_Litteral_Expression;

typedef struct MM_Array_Litteral_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* elem_type;
    MM_Argument* args;
} MM_Array_Litteral_Expression;

typedef struct MM_Array_Type_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* size;
    MM_Expression* elem_type;
} MM_Array_Type_Expression;

typedef struct MM_Unary_Expression
{
    union
    {
        struct
        {
            MM_EXPRESSION_HEADER();
            MM_Expression* operand;
        };
        
        MM_Array_Type_Expression array_type_expr;
        MM_Array_Litteral_Expression array_lit_expr;
        MM_Struct_Litteral_Expression struct_lit_expr;
        MM_Call_Expression call_expr;
        MM_Slice_Expression slice_expr;
        MM_Subscript_Expression subscript_expr;
        MM_Member_Access_Expression member_access_expr;
    };
} MM_Unary_Expression;

typedef struct MM_Binary_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* left;
    MM_Expression* right;
} MM_Binary_Expression;

typedef struct MM_Conditional_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Expression* condition;
    MM_Expression* true_expr;
    MM_Expression* false_expr;
} MM_Conditional_Expression;

typedef struct MM_Expression
{
    union
    {
        MM_EXPRESSION_HEADER();
        
        MM_Identifier identifier;
        MM_Soft_Int integer;
        MM_Soft_Float floating;
        MM_String_Literal string;
        MM_bool boolean;
        MM_Compound_Expression compound_expr;
        MM_Proc_Expression proc_expr;
        MM_Proc_Literal_Expression proc_lit_expr;
        MM_Proc_Set_Expression proc_set_expr;
        MM_Struct_Expression struct_expr;
        MM_Union_Expression union_expr;
        MM_Enum_Expression enum_expr;
        MM_Proc_Literal_Fwd_Decl_Expression proc_lit_fwd_decl_expr;
        MM_Enum_Fwd_Decl_Expression enum_fwd_decl_expr;
        MM_Builtin_Expression builtin_expr;
        MM_Unary_Expression unary_expr;
        MM_Binary_Expression binary_expr;
        MM_Conditional_Expression conditional_expr;
    };
} MM_Expression;

typedef struct MM_Declaration MM_Declaration;

typedef struct MM_Declaration_Header
{
    struct MM_AST_Kind;
    MM_Declaration* next;
} MM_Declaration_Header;

#define MM_DECLARATION_HEADER() union { struct MM_Declaration_Header; MM_Declaration_Header header; }

typedef struct MM_Variable_Declaration
{
    MM_DECLARATION_HEADER();
    MM_Expression* names;
    MM_Expression* types;
    MM_Expression* values;
    MM_bool is_uninitialized;
} MM_Variable_Declaration;

typedef struct MM_Constant_Declaration
{
    MM_DECLARATION_HEADER();
    MM_Expression* names;
    MM_Expression* types;
    MM_Expression* values;
} MM_Constant_Declaration;

typedef struct MM_Using_Declaration
{
    MM_DECLARATION_HEADER();
    MM_Expression* symbols;
    MM_Expression* aliases;
} MM_Using_Declaration;

typedef struct MM_When_Declaration
{
    MM_DECLARATION_HEADER();
    MM_Expression* condition;
    struct MM_Statement* true_body;
    struct MM_Statement* false_body;
} MM_When_Declaration;

typedef struct MM_Declaration
{
    union
    {
        MM_DECLARATION_HEADER();
        MM_Variable_Declaration var_decl;
        MM_Constant_Declaration const_decl;
        MM_Using_Declaration using_decl;
        MM_When_Declaration when_decl;
    };
} MM_Declaration;

typedef struct MM_Statement MM_Statement;

typedef struct MM_Statement_Header
{
    struct MM_AST_Kind;
    MM_Statement* next;
} MM_Statement_Header;

#define MM_STATEMENT_HEADER() union { struct MM_Statement_Header; MM_Statement_Header header; }

typedef struct MM_Block_Statement
{
    MM_STATEMENT_HEADER();
    MM_Statement* body;
} MM_Block_Statement;

typedef struct MM_If_Statement
{
    MM_STATEMENT_HEADER();
    MM_Expression* label;
    MM_Statement* init;
    MM_Expression* condition;
    MM_Statement* true_body;
    MM_Statement* false_body;
} MM_If_Statement;

typedef struct MM_While_Statement
{
    MM_STATEMENT_HEADER();
    MM_Expression* label;
    MM_Statement* init;
    MM_Expression* condition;
    MM_Statement* step;
    MM_Statement* body;
} MM_While_Statement;

typedef struct MM_Return_Statement
{
    MM_STATEMENT_HEADER();
    MM_Argument* args;
} MM_Return_Statement;

typedef struct MM_Defer_Statement
{
    MM_STATEMENT_HEADER();
    MM_Statement* body;
} MM_Defer_Statement;

typedef struct MM_Jump_Statement
{
    MM_STATEMENT_HEADER();
    MM_Expression* label;
} MM_Jump_Statement;

typedef struct MM_Assignment_Statement
{
    MM_STATEMENT_HEADER();
    MM_Expression* lhs;
    MM_Expression* rhs;
} MM_Assignment_Statement;

typedef struct MM_Statement
{
    union
    {
        MM_STATEMENT_HEADER();
        
        MM_Block_Statement block_statement;
        MM_If_Statement if_statement;
        MM_While_Statement while_statement;
        MM_Return_Statement return_statement;
        MM_Defer_Statement defer_statement;
        MM_Jump_Statement jump_statement;
        MM_Assignment_Statement assignment_statement;
    };
} MM_Statement;