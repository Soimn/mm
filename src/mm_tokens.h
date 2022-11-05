#define MM_TOKEN_BLOCK(I) ((I) << 4)
#define MM_TOKEN_BLOCK_INDEX(K) ((K) >> 4)
#define MM_TOKEN_BLOCK_OFFSET(K) ((K) & 0xF)
#define MM_TOKEN_FIRST_ASSIGNMENT_BLOCK 4
#define MM_TOKEN_FIRST_BINARY_BLOCK 9

#define MM_TOKEN_BINARY_TO_ASSIGNMENT(kind) ((kind) - MM_TOKEN_BLOCK(MM_TOKEN_FIRST_BINARY_BLOCK - MM_TOKEN_FIRST_ASSIGNMENT_BLOCK))
#define MM_TOKEN_ASSIGNMENT_TO_BINARY(kind) ((kind) + MM_TOKEN_BLOCK(MM_TOKEN_FIRST_BINARY_BLOCK - MM_TOKEN_FIRST_ASSIGNMENT_BLOCK))

typedef MM_u32 MM_Token_Kind;
enum MM_TOKEN_KIND
{
    MM_Token_Invalid = 0,
    MM_Token_EOF,
    
    MM_Token_Identifier,                           // _identifier1, ident1fier,
    MM_Token_String,                               // "aæ"
    MM_Token_Int,                                  // 3, 0b11, 0o3, 0x3, 0y3, 0z3
    MM_Token_Float,                                // 3.14, 0h4247, 0h40490FDB, 0h400921FB54442D18
    
    MM_Token_OpenParen,                            // (
    MM_Token_CloseParen,                           // )
    MM_Token_OpenBracket,                          // [
    MM_Token_CloseBracket,                         // ]
    MM_Token_OpenBrace,                            // {
    MM_Token_CloseBrace,                           // }
    MM_Token_Hat,                                  // ^
    MM_Token_Comma,                                // ,
    MM_Token_Colon,                                // :
    MM_Token_Semicolon,                            // ;
    
    MM_Token_TpMinus,                              // ---
    MM_Token_Period,                               // .
    MM_Token_PeriodParen,                          // .(
    MM_Token_PeriodBracket,                        // .[
    MM_Token_PeriodBrace,                          // .{
    MM_Token_Bang,                                 // !
    MM_Token_Arrow,                                // ->
    MM_Token_Blank,                                // _
    
    MM_Token__FirstAssignment,
    MM_Token_Equals = MM_Token__FirstAssignment,   // =
    
    MM_Token_StarEQ = MM_TOKEN_BLOCK(4),           // *=
    MM_Token_SlashEQ,                              // /=
    MM_Token_PercentEQ,                            // %=
    MM_Token_AndEQ,                                // &=
    MM_Token_ShlEQ,                                // <<=
    MM_Token_ShrEQ,                                // >>=
    MM_Token_SarEQ,                                // >>>=
    
    MM_Token_PlusEQ = MM_TOKEN_BLOCK(5),           // +=
    MM_Token_MinusEQ,                              // -=
    MM_Token_OrEQ,                                 // |=
    MM_Token_TildeEQ,                              // ~=
    
    MM_Token_AndAndEQ = MM_TOKEN_BLOCK(7),         // &&=
    
    MM_Token_OrOrEQ = MM_TOKEN_BLOCK(8),           // ||=
    MM_Token__LastAssignment = MM_TOKEN_BLOCK(9) - 1,
    
    MM_Token__FirstBinary = MM_TOKEN_BLOCK(9),
    MM_Token_Star = MM_Token__FirstBinary,         // *
    MM_Token_Slash,                                // /
    MM_Token_Percent,                              // %
    MM_Token_And,                                  // &
    MM_Token_Shl,                                  // <<
    MM_Token_Shr,                                  // >>
    MM_Token_Sar,                                  // >>>
    
    MM_Token_Plus = MM_TOKEN_BLOCK(10),            // +
    MM_Token_Minus,                                // -
    MM_Token_Or,                                   // |
    MM_Token_Tilde,                                // ~
    
    MM_Token_EqualEQ = MM_TOKEN_BLOCK(11),         // ==
    MM_Token_BangEQ,                               // !=
    MM_Token_Less,                                 // <
    MM_Token_LessEQ,                               // <=
    MM_Token_Greater,                              // >
    MM_Token_GreaterEQ,                            // >=
    
    MM_Token_AndAnd = MM_TOKEN_BLOCK(12),          // &&
    
    MM_Token_OrOr = MM_TOKEN_BLOCK(13),            // ||
    MM_Token__LastBinary = MM_TOKEN_BLOCK(14) - 1,
    
    MM_Token__FirstKeyword,
    MM_Token_If = MM_Token__FirstKeyword,          // if
    MM_Token_When,                                 // when
    MM_Token_Else,                                 // else
    MM_Token_While,                                // while
    MM_Token_Break,                                // break
    MM_Token_Continue,                             // continue
    MM_Token_Return,                               // return
    MM_Token_Proc,                                 // proc
    MM_Token_Struct,                               // struct
    MM_Token_True,                                 // true
    MM_Token_False,                                // false
    MM_Token_Nil,                                  // nil
    
    MM_Token__FirstBuiltin,
    MM_Token_Cast = MM_Token__FirstBuiltin,        // cast
    MM_Token_Transmute,                            // transmute
    MM_Token_Sizeof,                               // sizeof
    MM_Token_Alignof,                              // alignof
    MM_Token_Offsetof,                             // offsetof
    MM_Token__LastBuiltin = MM_Token_Offsetof,
    MM_Token__LastKeyword = MM_Token__LastBuiltin,
    
};

MM_STATIC_ASSERT(MM_Token__LastAssignment < MM_Token__FirstBinary);
MM_STATIC_ASSERT(MM_Token__FirstBinary == MM_TOKEN_BLOCK(MM_TOKEN_FIRST_BINARY_BLOCK));

#define MM_TOKEN_KEYWORD_LIST             \
MM_X(MM_Token_If,        "if")        \
MM_X(MM_Token_When,      "when")      \
MM_X(MM_Token_Else,      "else")      \
MM_X(MM_Token_While,     "while")     \
MM_X(MM_Token_Break,     "break")     \
MM_X(MM_Token_Continue,  "continue")  \
MM_X(MM_Token_Return,    "return")    \
MM_X(MM_Token_Proc,      "proc")      \
MM_X(MM_Token_Struct,    "struct")    \
MM_X(MM_Token_True,      "true")      \
MM_X(MM_Token_False,     "false")     \
MM_X(MM_Token_Nil,       "nil")       \

MM_String MM_Token_KeywordList[] = {
#define MM_X(e, s) [(e) - MM_Token__FirstKeyword].data = (MM_u8*)(s), [(e) - MM_Token__FirstKeyword].size = sizeof(s) - 1,
    MM_TOKEN_KEYWORD_LIST
#undef MM_X
};

#define MM_TOKEN_BUILTIN_LIST             \
MM_X(MM_Token_Cast,      "cast")      \
MM_X(MM_Token_Transmute, "transmute") \
MM_X(MM_Token_Sizeof,    "sizeof")    \
MM_X(MM_Token_Alignof,   "alignof")   \
MM_X(MM_Token_Offsetof,  "offsetof")  \

MM_String MM_Token_BuiltinList[] = {
#define MM_X(e, s) [(e) - MM_Token__FirstBuiltin].data = (MM_u8*)(s), [(e) - MM_Token__FirstBuiltin].size = sizeof(s) - 1,
    MM_TOKEN_BUILTIN_LIST
#undef MM_X
};

#define MM_TOKEN_NAME_LIST                                     \
MM_X(MM_Token_Invalid,       "MM_Token_Invalid")           \
MM_X(MM_Token_EOF,           "MM_Token_EOF")               \
MM_X(MM_Token_Identifier,    "MM_Token_Identifier")        \
MM_X(MM_Token_String,        "MM_Token_String")            \
MM_X(MM_Token_Int,           "MM_Token_Int")               \
MM_X(MM_Token_Float,         "MM_Token_Float")             \
MM_X(MM_Token_OpenParen,     "MM_Token_OpenParen")         \
MM_X(MM_Token_CloseParen,    "MM_Token_CloseParen")        \
MM_X(MM_Token_OpenBracket,   "MM_Token_OpenBracket")       \
MM_X(MM_Token_CloseBracket,  "MM_Token_CloseBracket")      \
MM_X(MM_Token_OpenBrace,     "MM_Token_OpenBrace")         \
MM_X(MM_Token_CloseBrace,    "MM_Token_CloseBrace")        \
MM_X(MM_Token_Hat,           "MM_Token_Hat")               \
MM_X(MM_Token_Comma,         "MM_Token_Comma")             \
MM_X(MM_Token_Colon,         "MM_Token_Colon")             \
MM_X(MM_Token_Semicolon,     "MM_Token_Semicolon")         \
MM_X(MM_Token_TpMinus,       "MM_Token_TpMinus")           \
MM_X(MM_Token_Period,        "MM_Token_Period")            \
MM_X(MM_Token_PeriodParen,   "MM_Token_PeriodParen")       \
MM_X(MM_Token_PeriodBracket, "MM_Token_PeriodBracket")     \
MM_X(MM_Token_PeriodBrace,   "MM_Token_PeriodBrace")       \
MM_X(MM_Token_Bang,          "MM_Token_Bang")              \
MM_X(MM_Token_Arrow,         "MM_Token_Arrow")             \
MM_X(MM_Token_Blank,         "MM_Token_Blank")             \
MM_X(MM_Token_Equals,        "MM_Token_Equals")            \
MM_X(MM_Token_StarEQ,        "MM_Token_StarEQ")            \
MM_X(MM_Token_SlashEQ,       "MM_Token_SlashEQ")           \
MM_X(MM_Token_PercentEQ,     "MM_Token_PercentEQ")         \
MM_X(MM_Token_AndEQ,         "MM_Token_AndEQ")             \
MM_X(MM_Token_ShlEQ,         "MM_Token_ShlEQ")             \
MM_X(MM_Token_ShrEQ,         "MM_Token_ShrEQ")             \
MM_X(MM_Token_SarEQ,         "MM_Token_SarEQ")             \
MM_X(MM_Token_PlusEQ,        "MM_Token_PlusEQ")            \
MM_X(MM_Token_MinusEQ,       "MM_Token_MinusEQ")           \
MM_X(MM_Token_OrEQ,          "MM_Token_OrEQ")              \
MM_X(MM_Token_TildeEQ,       "MM_Token_TildeEQ")           \
MM_X(MM_Token_AndAndEQ,      "MM_Token_AndAndEQ")          \
MM_X(MM_Token_OrOrEQ,        "MM_Token_OrOrEQ")            \
MM_X(MM_Token_Star,          "MM_Token_Star")              \
MM_X(MM_Token_Slash,         "MM_Token_Slash")             \
MM_X(MM_Token_Percent,       "MM_Token_Percent")           \
MM_X(MM_Token_And,           "MM_Token_And")               \
MM_X(MM_Token_Shl,           "MM_Token_Shl")               \
MM_X(MM_Token_Shr,           "MM_Token_Shr")               \
MM_X(MM_Token_Sar,           "MM_Token_Sar")               \
MM_X(MM_Token_Plus,          "MM_Token_Plus")              \
MM_X(MM_Token_Minus,         "MM_Token_Minus")             \
MM_X(MM_Token_Or,            "MM_Token_Or")                \
MM_X(MM_Token_Tilde,         "MM_Token_Tilde")             \
MM_X(MM_Token_EqualEQ,       "MM_Token_EqualEQ")           \
MM_X(MM_Token_BangEQ,        "MM_Token_BangEQ")            \
MM_X(MM_Token_Less,          "MM_Token_Less")              \
MM_X(MM_Token_LessEQ,        "MM_Token_LessEQ")            \
MM_X(MM_Token_Greater,       "MM_Token_Greater")           \
MM_X(MM_Token_GreaterEQ,     "MM_Token_GreaterEQ")         \
MM_X(MM_Token_AndAnd,        "MM_Token_AndAnd")            \
MM_X(MM_Token_OrOr,          "MM_Token_OrOr")              \
MM_X(MM_Token_If,            "MM_Token_If")                \
MM_X(MM_Token_When,          "MM_Token_When")              \
MM_X(MM_Token_Else,          "MM_Token_Else")              \
MM_X(MM_Token_While,         "MM_Token_While")             \
MM_X(MM_Token_Break,         "MM_Token_Break")             \
MM_X(MM_Token_Continue,      "MM_Token_Continue")          \
MM_X(MM_Token_Return,        "MM_Token_Return")            \
MM_X(MM_Token_Proc,          "MM_Token_Proc")              \
MM_X(MM_Token_Struct,        "MM_Token_Struct")            \
MM_X(MM_Token_True,          "MM_Token_True")              \
MM_X(MM_Token_False,         "MM_Token_False")             \
MM_X(MM_Token_Nil,           "MM_Token_Nil")               \
MM_X(MM_Token_Cast,          "MM_Token_Cast")              \
MM_X(MM_Token_Transmute,     "MM_Token_Transmute")         \
MM_X(MM_Token_Sizeof,        "MM_Token_Sizeof")            \
MM_X(MM_Token_Alignof,       "MM_Token_Alignof")           \
MM_X(MM_Token_Offsetof,      "MM_Token_Offsetof")

MM_String MM_Token_NameList[] = {
#define MM_X(e, s) [e].data = (MM_u8*)(s), [e].size = sizeof(s) - 1,
    MM_TOKEN_NAME_LIST
#undef MM_X
};

#define MM_TOKEN_SYMBOL_STRING_LIST      \
MM_X(MM_Token_OpenParen,     "(")    \
MM_X(MM_Token_CloseParen,    ")")    \
MM_X(MM_Token_OpenBracket,   "[")    \
MM_X(MM_Token_CloseBracket,  "]")    \
MM_X(MM_Token_OpenBrace,     "{")    \
MM_X(MM_Token_CloseBrace,    "}")    \
MM_X(MM_Token_Hat,           "^")    \
MM_X(MM_Token_Comma,         ",")    \
MM_X(MM_Token_Colon,         ":")    \
MM_X(MM_Token_Semicolon,     ";")    \
MM_X(MM_Token_TpMinus,       "---")  \
MM_X(MM_Token_Period,        ".")    \
MM_X(MM_Token_PeriodParen,   ".(")   \
MM_X(MM_Token_PeriodBracket, ".[")   \
MM_X(MM_Token_PeriodBrace,   ".{")   \
MM_X(MM_Token_Bang,          "!")    \
MM_X(MM_Token_Arrow,         "->")   \
MM_X(MM_Token_Blank,         "_")    \
MM_X(MM_Token_Equals,        "=")    \
MM_X(MM_Token_StarEQ,        "*=")   \
MM_X(MM_Token_SlashEQ,       "/=")   \
MM_X(MM_Token_PercentEQ,     "%=")   \
MM_X(MM_Token_AndEQ,         "&=")   \
MM_X(MM_Token_ShlEQ,         "<<=")  \
MM_X(MM_Token_ShrEQ,         ">>=")  \
MM_X(MM_Token_SarEQ,         ">>>=") \
MM_X(MM_Token_PlusEQ,        "+=")   \
MM_X(MM_Token_MinusEQ,       "-=")   \
MM_X(MM_Token_OrEQ,          "|=")   \
MM_X(MM_Token_TildeEQ,       "~=")   \
MM_X(MM_Token_AndAndEQ,      "&&=")  \
MM_X(MM_Token_OrOrEQ,        "||=")  \
MM_X(MM_Token_Star,          "*")    \
MM_X(MM_Token_Slash,         "/")    \
MM_X(MM_Token_Percent,       "%")    \
MM_X(MM_Token_And,           "&")    \
MM_X(MM_Token_Shl,           "<<")   \
MM_X(MM_Token_Shr,           ">>")   \
MM_X(MM_Token_Sar,           ">>>")  \
MM_X(MM_Token_Plus,          "+")    \
MM_X(MM_Token_Minus,         "-")    \
MM_X(MM_Token_Or,            "|")    \
MM_X(MM_Token_Tilde,         "~")    \
MM_X(MM_Token_EqualEQ,       "==")   \
MM_X(MM_Token_BangEQ,        "!=")   \
MM_X(MM_Token_Less,          "<")    \
MM_X(MM_Token_LessEQ,        "<=")   \
MM_X(MM_Token_Greater,       ">")    \
MM_X(MM_Token_GreaterEQ,     ">=")   \
MM_X(MM_Token_AndAnd,        "&&")   \
MM_X(MM_Token_OrOr,          "||")   \


MM_String MM_Token_SymbolStringList[] = {
#define MM_X(e, s) [e].data = (MM_u8*)(s), [e].size = sizeof(s) - 1,
    MM_TOKEN_SYMBOL_STRING_LIST
#undef MM_X
};

typedef struct MM_Text_Pos
{
    MM_u32 offset;
    MM_u32 line;
    MM_u32 col;
} MM_Text_Pos;

typedef struct MM_Token
{
    union { struct MM_Text_Pos; MM_Text_Pos text_pos; };
    MM_u32 size;
    MM_Token_Kind kind;
    
    union
    {
        MM_i128 i128;
        MM_f64 f64;
        MM_String identifier;
        MM_String string;
    };
} MM_Token;