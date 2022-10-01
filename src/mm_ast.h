#define MM_AST_BLOCK(I) ((I) << 4)

typedef MM_u32 MM_AST_Kind;
enum MM_AST_KIND
{
    MM_AST_Argument,
    MM_AST_Parameter,
    MM_AST_ReturnValue,
    MM_AST_StructMember,
    MM_AST_EnumMember,
    
    MM_AST__FirstExpression,
    MM_AST__FirstPrimary = MM_AST__FirstExpression,
    MM_AST_Identifier = MM_AST__FirstPrimary, // _identifier1, ident1fier
    MM_AST_String,                            // "aæ"
    MM_AST_Int,                               // 3, 0b11, 0o3, 0x3, 0y3, 0z3
    MM_AST_Float,                             // 3.14, 0h4247, 0h40490FDB, 0h400921FB54442D18
    MM_AST_Bool,                              // true, false
    MM_AST_ProcLit,                           // proc(a, ...) -> (a, ...) {}
    MM_AST_ProcType,                          // proc(a, ...) -> (a, ...)
    MM_AST_Struct,                            // struct {}
    MM_AST_Enum,                              // enum {}
    MM_AST_Compound,                          // (a)
    MM_AST_BuiltinCall,                       // builtin(a, ...)
    MM_AST__LastPrimary = MM_AST_BuiltinCall,
    
    MM_AST__FirstTypePrefix,
    MM_AST_PointerTo = MM_AST__FirstTypePrefix, // ^a
    MM_AST_SliceOf,                             // []a
    MM_AST_ArrayOf,                             // [b]a
    MM_AST__LastTypePrefix = MM_AST_ArrayOf,
    
    MM_AST__FirstPostfixUnary,
    MM_AST_Deref = MM_AST__FirstPostfixUnary,   // a^
    MM_AST_Subscript,                           // a[b]
    MM_AST_Slice,                               // a[b:c]
    MM_AST_Call,                                // a(b, ...)
    MM_AST_Member,                              // a.b
    MM_AST_StructLit,                           // a.{b, c, ...}  NOTE: parsed as primary for the inferred type version .{}
    MM_AST_ArrayLit,                            // a.[b, c, ...]  NOTE: parsed as primary for the inferred type version .[]
    MM_AST__LastPostfixUnary = MM_AST_ArrayLit,
    
    MM_AST__FirstPrefixUnary,
    MM_AST_Neg = MM_AST__FirstPrefixUnary,      // -a
    MM_AST_Not,                                 // !a
    MM_AST_BitNot,                              // ~a
    MM_AST_Ref,                                 // &a
    MM_AST__LastPrefixUnary = MM_AST_Ref,
    
    MM_AST__FirstBinary = MM_AST_BLOCK(4),
    MM_AST__FirstMulLevel = MM_AST__FirstBinary,
    MM_AST_Mul = MM_AST__FirstMulLevel,         // a * b
    MM_AST_Div,                                 // a / b
    MM_AST_Rem,                                 // a % b
    MM_AST_BitAnd,                              // a & b
    MM_AST_BitShl,                              // a << b
    MM_AST_BitShr,                              // a >> b
    MM_AST_BitSar,                              // a >>> b
    MM_AST__LastMulLevel = MM_AST_BitSar,
    
    MM_AST__FirstAddLevel = MM_AST_BLOCK(5),
    MM_AST_Add = MM_AST__FirstAddLevel,         // a + b
    MM_AST_Sub,                                 // a - b
    MM_AST_BitOr,                               // a | b
    MM_AST_BitXor,                              // a ~ b
    MM_AST__LastAddLevel = MM_AST_BitXor,
    
    MM_AST__FirstCmpLevel = MM_AST_BLOCK(6),
    MM_AST_CmpEqual = MM_AST__FirstCmpLevel,    // ==
    MM_AST_CmpNotEQ,                            // !=
    MM_AST_CmpLess,                             // <
    MM_AST_CmpLessEQ,                           // <=
    MM_AST_CmpGreater,                          // >
    MM_AST_CmpGreaterEQ,                        // >=
    MM_AST__LastCmpLevel = MM_AST_CmpGreaterEQ,
    
    MM_AST__FirstAndLevel = MM_AST_BLOCK(7),
    MM_AST_And = MM_AST__FirstAndLevel,         // &&
    MM_AST__LastAndLevel = MM_AST_And,
    
    MM_AST__FirstOrLevel = MM_AST_BLOCK(8),
    MM_AST_Or = MM_AST__FirstOrLevel,           // ||
    MM_AST__LastOrLevel = MM_AST_Or,
    MM_AST__LastBinary  = MM_AST_BLOCK(9) - 1,
    MM_AST__LastExpression = MM_AST__LastBinary,
    
    MM_AST__FirstDeclaration,
    MM_AST_Var = MM_AST__FirstDeclaration, // a : type = value, a := value, a: type, a : type = ---
    MM_AST_Const,                          // a : type : value, a :: value
    MM_AST__LastDeclaration = MM_AST_Const,
    
    MM_AST__FirstStatement,
    MM_AST_If = MM_AST__FirstStatement, // if (condition) statement, if (condition) statement else statement
    MM_AST_When,                        // when (condition) statement, when (condition) statement else statement
    MM_AST_While,                       // while (init; condition; step) statement, while (condition) statement
    MM_AST_Break,                       // break, break label
    MM_AST_Continue,                    // continue, continue label
    MM_AST_Return,                      // return, return expression, return expressions, expression, expression, ...
    
    MM_AST__FirstAssignmentStatement,
    MM_AST_Equals = MM_AST__FirstAssignmentStatement, // =
    MM_AST_MulEQ,                                     // *=
    MM_AST_DivEQ,                                     // /=
    MM_AST_RemEQ,                                     // %=
    MM_AST_BitAndEQ,                                  // &=
    MM_AST_BitShlEQ,                                  // <<=
    MM_AST_BitShrEQ,                                  // >>=
    MM_AST_BitSarEQ,                                  // >>>=
    MM_AST_AddEQ,                                     // +=
    MM_AST_SubEQ,                                     // -=
    MM_AST_BitOrEQ,                                   // |=
    MM_AST_BitXorEQ,                                  // ~=
    MM_AST_AndEQ,                                     // &&=
    MM_AST_OrEQ,                                      // ||=
    MM_AST__LastAssignmentStatement = MM_AST_OrEQ,
    MM_AST__LastStatement = MM_AST__LastAssignmentStatement,
};

MM_STATIC_ASSERT(MM_AST__LastPrefixUnary < MM_AST__FirstBinary);

#undef MM_AST_BLOCK

typedef enum MM_Builtin_Proc_ID
{
    MM_BuiltinProc_Cast,      // cast(T, e), cast(e)
    MM_BuiltinProc_Transmute, // transmute(T, e), transmute(e)
    MM_BuiltinProc_Sizeof,    // sizeof(T)
    MM_BuiltinProc_Alignof,   // alignof(T)
    MM_BuiltinProc_Offsetof,  // offsetof(T)
    MM_BuiltinProc_Typeof,    // typeof(e)
    MM_BuiltinProc_Min,       // min(e, e), min(T)
    MM_BuiltinProc_Max,       // max(e, e), max(T)
} MM_Builtin_Proc_ID;

typedef struct MM_Argument
{
    MM_AST_Kind kind;
    struct MM_Argument* next;
    struct MM_Expression* label;
    struct MM_Expression* value;
} MM_Argument;

typedef struct MM_Parameter
{
    MM_AST_Kind kind;
    struct MM_Parameter* next;
    struct MM_Expression* names;
    struct MM_Expression* type;
    struct MM_Expression* value;
} MM_Parameter;

typedef struct MM_Return_Value
{
    MM_AST_Kind kind;
    struct MM_Return_Value* next;
    struct MM_Expression* names;
    struct MM_Expression* type;
    struct MM_Expression* value;
} MM_Return_Value;

typedef struct MM_Struct_Member
{
    MM_AST_Kind kind;
    struct MM_Struct_Member* next;
    struct MM_Expression* names;
    struct MM_Expression* type;
    struct MM_Expression* const_value;
} MM_Struct_Member;

typedef struct MM_Enum_Member
{
    MM_AST_Kind kind;
    struct MM_Enum_Member* next;
    MM_Identifier name;
    struct MM_Expression* value;
} MM_Enum_Member;

typedef struct MM_Expression MM_Expression;

typedef struct MM_Expression_Header
{
    MM_AST_Kind kind;
    MM_Expression* next;
} MM_Expression_Header;

#define MM_EXPRESSION_HEADER() union { MM_Expression_Header header; struct { struct MM_Expression_Header; }; }

typedef struct MM_Expression_Identifier
{
    MM_EXPRESSION_HEADER();
    MM_Identifier identifier;
} MM_Expression_Identifier;

typedef struct MM_Expression_String
{
    MM_EXPRESSION_HEADER();
    MM_String_Literal string;
} MM_Expression_String;

typedef struct MM_Expression_Int
{
    MM_EXPRESSION_HEADER();
    MM_Soft_Int soft_int;
} MM_Expression_Int;

typedef struct MM_Expression_Float
{
    MM_EXPRESSION_HEADER();
    MM_f64 f64;
} MM_Expression_Float;

typedef struct MM_Expression_Bool
{
    MM_EXPRESSION_HEADER();
    MM_bool boolean;
} MM_Expression_Bool;

typedef struct MM_Expression_ProcLit
{
    MM_EXPRESSION_HEADER();
    MM_Parameter* params;
    MM_Return_Value* ret_vals;
    struct MM_Statement_Block* body;
} MM_Expression_ProcLit;

typedef struct MM_Expression_ProcType
{
    MM_EXPRESSION_HEADER();
    MM_Parameter* params;
    MM_Return_Value* ret_vals;
} MM_Expression_ProcType;

typedef struct MM_Expression_Struct
{
    MM_EXPRESSION_HEADER();
    MM_Struct_Member* members;
} MM_Expression_Struct;

typedef struct MM_Expression_Enum
{
    MM_EXPRESSION_HEADER();
    MM_Expression* member_type;
    MM_Enum_Member* members;
} MM_Expression_Enum;

typedef struct MM_Expression_Compound
{
    MM_EXPRESSION_HEADER();
    MM_Expression* expr;
} MM_Expression_Compound;

typedef struct MM_Expression_BuiltinCall
{
    MM_EXPRESSION_HEADER();
    MM_Builtin_Proc_ID proc_id;
    MM_Argument* args;
} MM_Expression_BuiltinCall;

typedef struct MM_Expression_Unary
{
    MM_EXPRESSION_HEADER();
    MM_Expression* operand;
} MM_Expression_Unary;

typedef struct MM_Expression_Binary
{
    MM_EXPRESSION_HEADER();
    MM_Expression* left;
    MM_Expression* right;
} MM_Expression_Binary;

typedef struct MM_Expression_ArrayOf
{
    MM_EXPRESSION_HEADER();
    MM_Expression* type;
    MM_Expression* size;
} MM_Expression_ArrayOf;

typedef struct MM_Expression_Subscript
{
    MM_EXPRESSION_HEADER();
    MM_Expression* array;
    MM_Expression* index;
} MM_Expression_Subscript;

typedef struct MM_Expression_Slice
{
    MM_EXPRESSION_HEADER();
    MM_Expression* array;
    MM_Expression* start;
    MM_Expression* one_past_end;
} MM_Expression_Slice;

typedef struct MM_Expression_Call
{
    MM_EXPRESSION_HEADER();
    MM_Expression* proc;
    MM_Argument* args;
} MM_Expression_Call;

typedef struct MM_Expression_Member
{
    MM_EXPRESSION_HEADER();
    MM_Expression* symbol;
    MM_Identifier member;
} MM_Expression_Member;

typedef struct MM_Expression_StructLit
{
    MM_EXPRESSION_HEADER();
    MM_Expression* type;
    MM_Argument* args;
} MM_Expression_StructLit;

typedef struct MM_Expression_ArrayLit
{
    MM_EXPRESSION_HEADER();
    MM_Expression* type;
    MM_Argument* args;
} MM_Expression_ArrayLit;

typedef struct MM_Expression
{
    union
    {
        MM_EXPRESSION_HEADER();
        
        MM_Expression_Identifier identifier_expr;
        MM_Expression_String string_expr;
        MM_Expression_Int int_expr;
        MM_Expression_Float float_expr;
        MM_Expression_Bool bool_expr;
        MM_Expression_ProcLit proc_lit_expr;
        MM_Expression_ProcType proc_type_expr;
        MM_Expression_Struct struct_expr;
        MM_Expression_Enum enum_expr;
        MM_Expression_Compound compound_expr;
        MM_Expression_BuiltinCall builtin_call_expr;
        MM_Expression_Unary unary_expr;
        MM_Expression_Binary binary_expr;
        MM_Expression_ArrayOf array_of_expr;
        MM_Expression_Subscript subscript_expr;
        MM_Expression_Slice slice_expr;
        MM_Expression_Call call_expr;
        MM_Expression_Member member_expr;
        MM_Expression_StructLit struct_lit_expr;
        MM_Expression_ArrayLit array_lit_expr;
    };
} MM_Expression;

typedef struct MM_Declaration MM_Declaration;

typedef struct MM_Declaration_Header
{
    MM_AST_Kind kind;
    MM_Declaration* next;
} MM_Declaration_Header;

#define MM_DECLARATION_HEADER() union { MM_Declaration_Header header; struct { struct MM_Declaration_Header; }; }

typedef struct MM_Declaration_Var
{
    MM_DECLARATION_HEADER();
    MM_Expression* names;
    MM_Expression* type;
    MM_Expression* values;
    MM_bool is_uninitialized;
} MM_Declaration_Var;

typedef struct MM_Declaration_Const
{
    MM_DECLARATION_HEADER();
    MM_Expression* names;
    MM_Expression* type;
    MM_Expression* values;
} MM_Declaration_Const;

typedef struct MM_Declaration
{
    union
    {
        MM_DECLARATION_HEADER();
        MM_Declaration_Var var_decl;
        MM_Declaration_Const const_decl;
    };
} MM_Declaration;

typedef struct MM_Statement MM_Statement;

typedef struct MM_Statement_Header
{
    MM_AST_Kind kind;
    MM_Statement* next;
} MM_Statement_Header;

#define MM_STATEMENT_HEADER() union { MM_Statement_Header header; struct { struct MM_Statement_Header; }; }

typedef struct MM_Statement_Block
{
    MM_STATEMENT_HEADER();
    MM_Identifier label;
    MM_Statement* body;
} MM_Statement_Block;

typedef struct MM_Statement_If
{
    MM_STATEMENT_HEADER();
    MM_Identifier label;
    MM_Expression* condition;
    MM_Statement* true_body;
    MM_Statement* false_body;
} MM_Statement_If;

typedef struct MM_Statement_When
{
    MM_STATEMENT_HEADER();
    MM_Expression* condition;
    MM_Statement* true_body;
    MM_Statement* false_body;
} MM_Statement_When;

typedef struct MM_Statement_While
{
    MM_STATEMENT_HEADER();
    MM_Statement* init;
    MM_Expression* condition;
    MM_Statement* step;
    MM_Statement* body;
} MM_Statement_While;

typedef struct MM_Statement_Jump
{
    MM_STATEMENT_HEADER();
    MM_Identifier label;
} MM_Statement_Jump;

typedef struct MM_Statement_Return
{
    MM_STATEMENT_HEADER();
    MM_Argument* args;
} MM_Statement_Return;

typedef struct MM_Statement_Assignment
{
    MM_STATEMENT_HEADER();
    MM_Expression* lhs;
    MM_Expression* rhs;
} MM_Statement_Assignment;

typedef struct MM_Statement
{
    union
    {
        MM_STATEMENT_HEADER();
        MM_Expression expression;
        MM_Declaration declaration;
        
        MM_Statement_Block block_stmnt;
        MM_Statement_If if_stmnt;
        MM_Statement_When when_stmnt;
        MM_Statement_While while_stmnt;
        MM_Statement_Jump jump_stmnt;
        MM_Statement_Return return_stmnt;
        MM_Statement_Assignment assignment_stmnt;
    };
} MM_Statement;

typedef struct MM_AST
{
    union
    {
        struct
        {
            MM_AST_Kind kind;
            struct MM_AST* next;
        };
        
        MM_Argument argument;
        MM_Parameter parameter;
        MM_Return_Value return_value;
        MM_Struct_Member struct_member;
        MM_Enum_Member enum_member;
        MM_Expression expression;
        MM_Declaration declaration;
        MM_Statement statement;
    };
} MM_AST;