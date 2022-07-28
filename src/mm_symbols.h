typedef enum MM_SYMBOL_KIND
{
    MM_Symbol_Var,
    MM_Symbol_Const,
    MM_Symbol_Param,
    MM_Symbol_RetVal,
    MM_Symbol_JMPLabel,
    MM_Symbol_UsingLink,
} MM_SYMBOL_KIND;

typedef struct MM_Symbol_Header
{
    MM_SYMBOL_KIND kind;
    MM_Identifier name;
    MM_AST* ast;
} MM_Symbol_Header;

#define MM_SYMBOL_HEADER() union { MM_Symbol_Header header; struct MM_Symbol_Header; }

typedef struct MM_Symbol_Variable
{
    MM_SYMBOL_HEADER();
    // type ref: type
} MM_Symbol_Variable;

typedef struct MM_Symbol_Constant
{
    MM_SYMBOL_HEADER();
    // type ref: type
    // value ref: value
} MM_Symbol_Constant;

typedef struct MM_Symbol_Parameter
{
    MM_SYMBOL_HEADER();
    // type ref: type
    // value ref: default value
} MM_Symbol_Parameter;

typedef struct MM_Symbol_Return_Value
{
    MM_SYMBOL_HEADER();
    // type ref: type
    // value ref: default value
} MM_Symbol_Return_Value;

typedef struct MM_Symbol_Jump_Label
{
    MM_SYMBOL_HEADER();
    // ast/scope ref: labeled block
} MM_Symbol_Jump_Label;

typedef struct MM_Symbol_Using_Link
{
    MM_SYMBOL_HEADER();
    // symbol ref: base symbol
    // : access path
    // type ref: type
    // TODO: is constant, is writable
} MM_Symbol_Using_Link;

typedef struct MM_Symbol
{
    union
    {
        MM_SYMBOL_HEADER();
        MM_Symbol_Variable var_sym;
        MM_Symbol_Constant const_sym;
        MM_Symbol_Parameter param_sym;
        MM_Symbol_Return_Value ret_val_sym;
        MM_Symbol_Jump_Label jmp_label_sym;
        MM_Symbol_Using_Link using_link_sym;
    };
} MM_Symbol;