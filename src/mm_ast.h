#define MM_AST_FIRST_KIND(type, group, sub_group) (((type) << 24) | ((group) << 16) | ((sub_group) << 8) | 1)

enum MM_AST_TYPE
{
    MM_ASTType_Special = 1,
    MM_ASTType_Expression,
    MM_ASTType_Declaration,
    MM_ASTType_Statement,
};

enum MM_AST_GROUP
{
    // NOTE: Groups under MM_AST_Expression
    MM_ASTGroup_PrimaryExpression = 1,
    MM_ASTGroup_UnaryExpression,
    MM_ASTGroup_BinaryExpression,
    MM_ASTGroup_ConditionalExpression,
    
    // NOTE: Groups under MM_AST_Statement
    MM_ASTGroup_Jump = 1,
    MM_ASTGroup_Assignment,
};

enum MM_AST_SUB_GROUP
{
    // NOTE: Sub groups under MM_ASTType_Expression, MM_ASTGroup_PrimaryExpression
    MM_ASTSubGroup_Builtin = 1,
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
};

enum MM_AST_KIND
{
    MM_AST_None = 0,
    
    MM_AST_Argument = MM_AST_FIRST_KIND(MM_ASTType_Special, MM_AST_None, MM_AST_None),
    MM_AST_Parameter,
    MM_AST_ReturnValue,
    MM_AST_EnumMember,
    
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
    MM_AST_Struct,
    MM_AST_Union,
    MM_AST_Enum,
    
    MM_AST_ProcLiteralFwdDecl = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_PrimaryExpression, MM_ASTSubGroup_FwdDecl),
    MM_AST_StructFwdDecl,
    MM_AST_UnionFwdDecl,
    MM_AST_EnumFwdDecl,
    
    MM_AST_BuiltinCast = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_PrimaryExpression, MM_ASTSubGroup_Builtin),
    MM_AST_BuiltinTransmute,
    MM_AST_BuiltinSizeof,
    MM_AST_BuiltinAlignof,
    MM_AST_BuiltinOffsetof,
    MM_AST_BuiltinTypeof,
    MM_AST_BuiltinMin,
    MM_AST_BuiltinMax,
    MM_AST_BuiltinLen,
    MM_AST_BuiltinMemcopy,
    MM_AST_BuiltinMemmove,
    MM_AST_BuiltinMemset,
    MM_AST_BuiltinMemzero,
    MM_AST_BuiltinSourcePos,
    
    MM_AST_Dereference = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_UnaryExpression, MM_ASTSubGroup_PostfixUnary),
    MM_AST_MemberAccess,
    MM_AST_Subscript,
    MM_AST_Slice,
    MM_AST_Call,
    MM_AST_StructLiteral,
    MM_AST_ArrayLiteral,
    
    MM_AST_BackScope = MM_AST_FIRST_KIND(MM_ASTType_Expression, MM_ASTGroup_UnaryExpression, MM_ASTSubGroup_PrefixUnary),
    MM_AST_Reference,
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
    MM_AST_BitSpl,
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
    MM_AST_ConstantFwdDecl,
    MM_AST_Using,
    MM_AST_When,
    MM_AST_Include,
    
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
    MM_AST_BitSplAssignment,
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
    union
    {
        MM_u32 kind;
        struct
        {
            MM_u8 kind_index;
            MM_u8 kind_sub_group;
            MM_u8 kind_group;
            MM_u8 kind_type;
        };
    };
} MM_AST_Kind;

typedef struct MM_Special MM_Special;

typedef struct MM_Special_Header
{
    struct MM_AST_Kind;
} MM_Special_Header;

#define MM_SPECIAL_HEADER() union { struct MM_Special_Header; MM_Special_Header header; }

typedef struct MM_Argument
{
    MM_SPECIAL_HEADER();
    struct MM_Argument* next;
    struct MM_Expression* name;
    struct MM_Expression* value;
} MM_Argument;

typedef struct MM_Parameter
{
    MM_SPECIAL_HEADER();
    struct MM_Parameter* next;
    struct MM_Expression* names;
    struct MM_Expression* type;
    struct MM_Expression* value;
} MM_Parameter;

typedef struct MM_Return_Value
{
    MM_SPECIAL_HEADER();
    struct MM_Return_Value* next;
    struct MM_Expression* names;
    struct MM_Expression* type;
    struct MM_Expression* value;
} MM_Return_Value;

typedef struct MM_Enum_Member
{
    MM_SPECIAL_HEADER();
    struct MM_Enum_Member* next;
    struct MM_Expression* name;
    struct MM_Expression* value;
} MM_Enum_Member;

typedef struct MM_Special
{
    union
    {
        struct
        {
            MM_SPECIAL_HEADER();
            struct MM_Special* next;
        };
        
        MM_Argument argument;
        MM_Parameter parameter;
        MM_Return_Value return_val;
        MM_Enum_Member enum_member;
    };
} MM_Special;

typedef struct MM_Expression MM_Expression;

typedef struct MM_Expression_Header
{
    struct MM_AST_Kind;
    MM_Expression* next;
} MM_Expression_Header;

#define MM_EXPRESSION_HEADER() union { struct MM_Expression_Header; MM_Expression_Header header; }

typedef struct MM_Identifier_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Identifier ident;
} MM_Identifier_Expression;

typedef struct MM_Integer_Literal_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Soft_Int value;
    MM_u8 explicit_base; // NOTE: 0 means no explicit base, non 0 corresponds to the bases of 0x0 (16), 0d0 (10) and 0b0 (2)
} MM_Integer_Literal_Expression;

typedef struct MM_Float_Literal_Expression
{
    MM_EXPRESSION_HEADER();
    MM_Soft_Float value;
    MM_u8 explicit_size; // NOTE: 0 means no explicit size, non 0 corresponds to the size of HexFloat16 - 64 in bytes
} MM_Float_Literal_Expression;

typedef struct MM_String_Literal_Expression
{
    MM_EXPRESSION_HEADER();
    MM_String_Literal str;
} MM_String_Literal_Expression;

typedef struct MM_Codepoint_Expression
{
    MM_EXPRESSION_HEADER();
    MM_u32 value;
} MM_Codepoint_Expression;

typedef struct MM_Boolean_Expression
{
    MM_EXPRESSION_HEADER();
    MM_bool value;
} MM_Boolean_Expression;

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
    struct MM_Block_Statement* body;
} MM_Proc_Literal_Expression;

typedef struct MM_Struct_Expression
{
    MM_EXPRESSION_HEADER();
    struct MM_Declaration* body;
} MM_Struct_Expression;

typedef struct MM_Union_Expression
{
    MM_EXPRESSION_HEADER();
    struct MM_Declaration* body;
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
    MM_EXPRESSION_HEADER();
    MM_Expression* operand;
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
        
        MM_Identifier_Expression ident_expr;
        MM_Integer_Literal_Expression int_expr;
        MM_Float_Literal_Expression float_expr;
        MM_String_Literal_Expression string_expr;
        MM_Codepoint_Expression codepoint_expr;
        MM_Boolean_Expression bool_expr;
        MM_Compound_Expression compound_expr;
        MM_Proc_Expression proc_expr;
        MM_Proc_Literal_Expression proc_lit_expr;
        MM_Struct_Expression struct_expr;
        MM_Union_Expression union_expr;
        MM_Enum_Expression enum_expr;
        MM_Proc_Literal_Fwd_Decl_Expression proc_lit_fwd_decl_expr;
        MM_Enum_Fwd_Decl_Expression enum_fwd_decl_expr;
        MM_Builtin_Expression builtin_expr;
        MM_Unary_Expression unary_expr;
        MM_Array_Type_Expression array_type_expr;
        MM_Array_Litteral_Expression array_lit_expr;
        MM_Struct_Litteral_Expression struct_lit_expr;
        MM_Call_Expression call_expr;
        MM_Slice_Expression slice_expr;
        MM_Subscript_Expression subscript_expr;
        MM_Member_Access_Expression member_access_expr;
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
    MM_Expression* type;
    MM_Expression* values;
    MM_bool is_uninitialized;
    MM_bool is_using;
} MM_Variable_Declaration;

typedef struct MM_Constant_Declaration
{
    MM_DECLARATION_HEADER();
    MM_Expression* names;
    MM_Expression* type;
    MM_Expression* values;
    MM_bool is_using;
    MM_bool is_distinct;
} MM_Constant_Declaration;

typedef struct MM_Constant_Fwd_Declaration
{
    MM_DECLARATION_HEADER();
    MM_Expression* names;
    MM_Expression* type;
} MM_Constant_Fwd_Declaration;

typedef struct MM_Using_Declaration
{
    MM_DECLARATION_HEADER();
    MM_Expression* symbol;
    MM_Identifier alias;
} MM_Using_Declaration;

typedef struct MM_When_Declaration
{
    MM_DECLARATION_HEADER();
    MM_Expression* condition;
    struct MM_Statement* true_body;
    struct MM_Statement* false_body;
} MM_When_Declaration;

typedef struct MM_Include_Declaration
{
    MM_DECLARATION_HEADER();
    MM_String_Literal path;
} MM_Include_Declaration;

typedef struct MM_Declaration
{
    union
    {
        MM_DECLARATION_HEADER();
        MM_Variable_Declaration var_decl;
        MM_Constant_Declaration const_decl;
        MM_Constant_Fwd_Declaration const_fwd_decl;
        MM_Using_Declaration using_decl;
        MM_When_Declaration when_decl;
        MM_Include_Declaration include_decl;
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
    MM_Identifier label;
} MM_Block_Statement;

typedef struct MM_If_Statement
{
    MM_STATEMENT_HEADER();
    MM_Statement* init;
    MM_Expression* condition;
    MM_Statement* true_body;
    MM_Statement* false_body;
    MM_Identifier label;
} MM_If_Statement;

typedef struct MM_While_Statement
{
    MM_STATEMENT_HEADER();
    MM_Statement* init;
    MM_Expression* condition;
    MM_Statement* step;
    MM_Statement* body;
    MM_Identifier label;
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
    MM_Identifier label;
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

typedef struct MM_AST
{
    union
    {
        struct
        {
            struct MM_AST_Kind;
            struct MM_AST* next;
        };
        
        MM_Special special;
        MM_Statement statement;
        MM_Declaration declaration;
        MM_Expression expression;
    };
} MM_AST;

inline MM_umm
MM_AST_SizeFromKind(MM_AST_Kind kind)
{
    MM_umm size = 0;
    if      (kind.kind == MM_AST_Argument)           size = sizeof(MM_Argument);
    else if (kind.kind == MM_AST_Parameter)          size = sizeof(MM_Parameter);
    else if (kind.kind == MM_AST_ReturnValue)        size = sizeof(MM_Return_Value);
    else if (kind.kind == MM_AST_EnumMember)         size = sizeof(MM_Enum_Member);
    else if (kind.kind == MM_AST_Identifier)         size = sizeof(MM_Identifier_Expression);
    else if (kind.kind == MM_AST_Int)                size = sizeof(MM_Integer_Literal_Expression);
    else if (kind.kind == MM_AST_Float)              size = sizeof(MM_Float_Literal_Expression);
    else if (kind.kind == MM_AST_String)             size = sizeof(MM_String_Literal_Expression);
    else if (kind.kind == MM_AST_Codepoint)          size = sizeof(MM_Codepoint_Expression);
    else if (kind.kind == MM_AST_Bool)               size = sizeof(MM_Boolean_Expression);
    else if (kind.kind == MM_AST_Compound)           size = sizeof(MM_Compound_Expression);
    else if (kind.kind == MM_AST_This)               size = sizeof(MM_Expression_Header);
    else if (kind.kind == MM_AST_Proc)               size = sizeof(MM_Proc_Expression);
    else if (kind.kind == MM_AST_ProcLiteral)        size = sizeof(MM_Proc_Literal_Expression);
    else if (kind.kind == MM_AST_Struct)             size = sizeof(MM_Struct_Expression);
    else if (kind.kind == MM_AST_Union)              size = sizeof(MM_Union_Expression);
    else if (kind.kind == MM_AST_Enum)               size = sizeof(MM_Enum_Expression);
    else if (kind.kind == MM_AST_ProcLiteralFwdDecl) size = sizeof(MM_Proc_Literal_Fwd_Decl_Expression);
    else if (kind.kind == MM_AST_StructFwdDecl)      size = sizeof(MM_Expression_Header);
    else if (kind.kind == MM_AST_UnionFwdDecl)       size = sizeof(MM_Expression_Header);
    else if (kind.kind == MM_AST_EnumFwdDecl)        size = sizeof(MM_Enum_Fwd_Decl_Expression);
    else if (kind.kind == MM_AST_MemberAccess)       size = sizeof(MM_Member_Access_Expression);
    else if (kind.kind == MM_AST_Subscript)          size = sizeof(MM_Subscript_Expression);
    else if (kind.kind == MM_AST_Slice)              size = sizeof(MM_Slice_Expression);
    else if (kind.kind == MM_AST_Call)               size = sizeof(MM_Call_Expression);
    else if (kind.kind == MM_AST_StructLiteral)      size = sizeof(MM_Struct_Litteral_Expression);
    else if (kind.kind == MM_AST_ArrayLiteral)       size = sizeof(MM_Array_Litteral_Expression);
    else if (kind.kind == MM_AST_ArrayType)          size = sizeof(MM_Array_Type_Expression);
    else if (kind.kind == MM_AST_Conditional)        size = sizeof(MM_Conditional_Expression);
    else if (kind.kind == MM_AST_Variable)           size = sizeof(MM_Variable_Declaration);
    else if (kind.kind == MM_AST_Constant)           size = sizeof(MM_Constant_Declaration);
    else if (kind.kind == MM_AST_ConstantFwdDecl)    size = sizeof(MM_Constant_Fwd_Declaration);
    else if (kind.kind == MM_AST_Using)              size = sizeof(MM_Using_Declaration);
    else if (kind.kind == MM_AST_When)               size = sizeof(MM_When_Declaration);
    else if (kind.kind == MM_AST_Include)            size = sizeof(MM_Include_Declaration);
    else if (kind.kind == MM_AST_Block)              size = sizeof(MM_Block_Statement);
    else if (kind.kind == MM_AST_If)                 size = sizeof(MM_If_Statement);
    else if (kind.kind == MM_AST_While)              size = sizeof(MM_While_Statement);
    else if (kind.kind == MM_AST_Return)             size = sizeof(MM_Return_Statement);
    else if (kind.kind == MM_AST_Defer)              size = sizeof(MM_Defer_Statement);
    else if (kind.kind_type == MM_ASTType_Expression && kind.kind_group == MM_ASTGroup_UnaryExpression)
    {
        size = sizeof(MM_Unary_Expression);
    }
    else if (kind.kind_type == MM_ASTType_Expression && kind.kind_group == MM_ASTGroup_BinaryExpression)
    {
        size = sizeof(MM_Binary_Expression);
    }
    else if (kind.kind_type == MM_ASTType_Expression && kind.kind_group == MM_ASTGroup_PrimaryExpression && kind.kind_sub_group == MM_ASTSubGroup_Builtin)
    {
        size = sizeof(MM_Builtin_Expression);
    }
    else if (kind.kind_group == MM_ASTGroup_Jump)
    {
        MM_ASSERT(kind.kind_type == MM_ASTType_Statement);
        size = sizeof(MM_Jump_Statement);
    }
    else if (kind.kind_group == MM_ASTGroup_Assignment)
    {
        MM_ASSERT(kind.kind_type == MM_ASTType_Statement);
        size = sizeof(MM_Assignment_Statement);
    }
    else MM_INVALID_CODE_PATH;
    
    return size;
}