#define MM_TOKEN_BLOCK(I) ((I) << 4)
#define MM_TOKEN_BLOCK_INDEX(K) ((K) >> 4)
#define MM_TOKEN_BLOCK_OFFSET(K) ((K) & 0xF)

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
    MM_Token_StarEQ,                               // *=
    MM_Token_SlashEQ,                              // /=
    MM_Token_PercentEQ,                            // %=
    MM_Token_AndEQ,                                // &=
    MM_Token_ShlEQ,                                // <<=
    MM_Token_ShrEQ,                                // >>=
    MM_Token_SarEQ,                                // >>>=
    MM_Token_PlusEQ,                               // +=
    MM_Token_MinusEQ,                              // -=
    MM_Token_OrEQ,                                 // |=
    MM_Token_TildeEQ,                              // ~=
    MM_Token_AndAndEQ,                             // &&=
    MM_Token_OrOrEQ,                               // ||=
    MM_Token__LastAssignment = MM_Token_OrOrEQ,
    
    MM_Token__FirstBinary = MM_TOKEN_BLOCK(4),
    MM_Token_Star = MM_Token__FirstBinary,         // *
    MM_Token_Slash,                                // /
    MM_Token_Percent,                              // %
    MM_Token_And,                                  // &
    MM_Token_Shl,                                  // <<
    MM_Token_Shr,                                  // >>
    MM_Token_Sar,                                  // >>>
    
    MM_Token_Plus = MM_TOKEN_BLOCK(5),             // +
    MM_Token_Minus,                                // -
    MM_Token_Or,                                   // |
    MM_Token_Tilde,                                // ~
    
    MM_Token_EqualEQ = MM_TOKEN_BLOCK(6),          // ==
    MM_Token_BangEQ,                               // !=
    MM_Token_Less,                                 // <
    MM_Token_LessEQ,                               // <=
    MM_Token_Greater,                              // >
    MM_Token_GreaterEQ,                            // >=
    
    MM_Token_AndAnd = MM_TOKEN_BLOCK(7),           // &&
    
    MM_Token_OrOr = MM_TOKEN_BLOCK(8),             // ||
    MM_Token__LastBinary = MM_TOKEN_BLOCK(9) - 1,
    
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
    MM_Token_Enum,                                 // enum
    MM_Token_True,                                 // true
    MM_Token_False,                                // false
    MM_Token_Nil,                                  // nil
    
    MM_Token__FirstBuiltinKeyword,
    MM_Token_Cast = MM_Token__FirstBuiltinKeyword, // cast
    MM_Token_Transmute,                            // transmute
    MM_Token_Sizeof,                               // sizeof
    MM_Token_Alignof,                              // alignof
    MM_Token_Offsetof,                             // offsetof
    MM_Token_Typeof,                               // typeof
    MM_Token_Min,                                  // min
    MM_Token_Max,                                  // max
    MM_Token__LastBuiltinKeyword = MM_Token_Max,
    MM_Token__LastKeyword = MM_Token__LastBuiltinKeyword,
    
};

#undef MM_TOKEN_BLOCK

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
MM_X(MM_Token_Enum,      "enum")      \
MM_X(MM_Token_True,      "true")      \
MM_X(MM_Token_False,     "false")     \
MM_X(MM_Token_Nil,       "nil")       \
MM_X(MM_Token_Cast,      "cast")      \
MM_X(MM_Token_Transmute, "transmute") \
MM_X(MM_Token_Sizeof,    "sizeof")    \
MM_X(MM_Token_Alignof,   "alignof")   \
MM_X(MM_Token_Offsetof,  "offsetof")  \
MM_X(MM_Token_Typeof,    "typeof")    \
MM_X(MM_Token_Min,       "min")       \
MM_X(MM_Token_Max,       "max")       \

// NOTE: Testing for overlap between token blocks and surrounding tokens
MM_STATIC_ASSERT(MM_TOKEN_BLOCK_INDEX(MM_Token__LastAssignment) < MM_TOKEN_BLOCK_INDEX(MM_Token__FirstBinary));
MM_STATIC_ASSERT(MM_TOKEN_BLOCK_INDEX(MM_Token__LastBinary) < MM_TOKEN_BLOCK_INDEX(MM_Token__FirstKeyword));

typedef struct MM_Text_Pos
{
    MM_u32 offset;
    MM_u32 line;
    MM_u32 col;
} MM_Text_Pos;

typedef struct MM_Text_Range
{
    MM_Text_Pos start;
    MM_Text_Pos past_end;
} MM_Text_Range;

typedef struct MM_Token
{
    MM_Token_Kind kind;
    MM_Text_Range text;
    
    union
    {
        MM_Soft_Int soft_int;
        MM_Soft_Float soft_float;
        MM_String identifier;
        MM_String string;
    };
} MM_Token;