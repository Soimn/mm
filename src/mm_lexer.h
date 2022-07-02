// NOTE: Token kinds are organized into blocks of max 16 in size. This is to get checking for binary operators and their
//       precedence for free, as well as mapping binary assignment tokens to their operators

#define MM_TOKEN_KIND_BLOCK_SIZE 16
#define MM_TOKEN_BLOCK_INDEX_FROM_KIND(kind) ((kind) / MM_TOKEN_KIND_BLOCK_SIZE)
#define MM_TOKEN_BLOCK_OFFSET_FROM_KIND(kind) ((kind) / MM_TOKEN_KIND_BLOCK_SIZE)
#define MM_TOKEN_BINARY_ASSIGNMENT_TO_BINARY_KIND(kind) ((MM_TOKEN_BLOCK_INDEX_FROM_KIND(kind) + 5)*MM_TOKEN_KIND_BLOCK_SIZE + MM_TOKEN_BLOCK_OFFSET_FROM_KIND(kind))
#define MM_TOKEN_BINARY_TO_BINARY_ASSIGNMENT_KIND(kind) ((MM_TOKEN_BLOCK_INDEX_FROM_KIND(kind) - 5)*MM_TOKEN_KIND_BLOCK_SIZE + MM_TOKEN_BLOCK_OFFSET_FROM_KIND(kind))

typedef enum MM_TOKEN_KIND
{
    MM_Token_Invalid = 0,
    MM_Token_EndOfStream,
    
    MM_Token_Identifier,
    MM_Token_BlankIdentifier,
    MM_Token_Int,
    MM_Token_Float,
    MM_Token_String,
    MM_Token_Codepoint,
    
    MM_Token_Comment,
    MM_Token_BlockComment,
    
    MM_Token_FirstAssignment,
    MM_Token_Equals = MM_Token_FirstAssignment,
    
    MM_Token_FirstBinaryAssignment = 1*16,
    MM_Token_FirstMulLevelAssignment = MM_Token_FirstAssignment,
    MM_Token_StarEquals = MM_Token_FirstMulLevelAssignment,
    MM_Token_SlashEquals,
    MM_Token_PercentEquals,
    MM_Token_AndEquals,
    MM_Token_LessLessEquals,
    MM_Token_GreaterGreaterEquals,
    MM_Token_TripleGreaterEquals,
    MM_Token_LastMulLevelAssignment = MM_Token_GreaterGreaterEquals,
    
    MM_Token_FirstAddLevelAssignment = 2*16,
    MM_Token_TildeEquals = MM_Token_FirstAddLevelAssignment,
    MM_Token_OrEquals,
    MM_Token_MinusEquals,
    MM_Token_PlusEquals,
    MM_Token_LastAddLevelAssignment = MM_Token_PlusEquals,
    
    MM_Token_AndAndEquals = 4*16,
    
    MM_Token_OrOrEquals = 5*16,
    MM_Token_LastBinaryAssignment = MM_Token_OrOrEquals,
    MM_Token_LastAssignment = MM_Token_LastBinaryAssignment,
    
    MM_Token_FirstBinary = 6*16,
    MM_Token_FirstMulLevel = MM_Token_FirstBinary,
    MM_Token_Star = MM_Token_FirstMulLevel,
    MM_Token_Slash,
    MM_Token_Percent,
    MM_Token_And,
    MM_Token_LessLess,
    MM_Token_GreaterGreater,
    MM_Token_TripleGreater,
    MM_Token_LastMulLevel = MM_Token_TripleGreater,
    
    MM_Token_FirstAddLevel = 7*16,
    MM_Token_Tilde = MM_Token_FirstAddLevel,
    MM_Token_Or,
    MM_Token_Minus,
    MM_Token_Plus,
    MM_Token_LastAddLevel = MM_Token_Plus,
    
    MM_Token_FirstCmpLevel = 8*16,
    MM_Token_Greater = MM_Token_FirstCmpLevel,
    MM_Token_GreaterEquals,
    MM_Token_Less,
    MM_Token_LessEquals,
    MM_Token_EqualsEquals,
    MM_Token_BangEquals,
    MM_Token_LastCmpLevel = MM_Token_BangEquals,
    
    MM_Token_AndAnd = 9*16,
    
    MM_Token_OrOr = 10*16,
    MM_Token_LastBinary = MM_Token_OrOr,
    
    MM_Token_Bang,
    MM_Token_OpenParen,
    MM_Token_CloseParen,
    MM_Token_OpenBracket,
    MM_Token_CloseBracket,
    MM_Token_OpenBrace,
    MM_Token_CloseBrace,
    MM_Token_Colon,
    MM_Token_Comma,
    MM_Token_Semicolon,
    MM_Token_QuestionMark,
    MM_Token_Hat,
    MM_Token_Arrow,
    MM_Token_TripleMinus,
    MM_Token_Period,
    MM_Token_PeriodOpenBrace,
    MM_Token_PeriodOpenBracket,
    
    MM_Token_FirstKeyword,
    MM_Token_Include = MM_Token_FirstKeyword,
    MM_Token_Proc,
    MM_Token_ProcSet,
    MM_Token_Struct,
    MM_Token_Union,
    MM_Token_Enum,
    MM_Token_True,
    MM_Token_False,
    MM_Token_As,
    MM_Token_If,
    MM_Token_When,
    MM_Token_Else,
    MM_Token_While,
    MM_Token_Break,
    MM_Token_Continue,
    MM_Token_Using,
    MM_Token_Defer,
    MM_Token_Return,
    MM_Token_This,
    MM_Token_Inf,
    MM_Token_Qnan,
    MM_Token_Snan,
    MM_Token_LastKeyword = MM_Token_Return,
    
    MM_Token_FirstBuiltin,
    MM_Token_Cast = MM_Token_FirstBuiltin,
    MM_Token_Transmute,
    MM_Token_Sizeof,
    MM_Token_Alignof,
    MM_Token_Offsetof,
    MM_Token_Typeof,
    MM_Token_Shadowed,
    MM_Token_Min,
    MM_Token_Max,
    MM_Token_Len,
    MM_Token_LastBuiltin = MM_Token_Len,
    
    
} MM_TOKEN_KIND;

typedef struct MM_Text_Pos
{
    MM_u32 offset;
    MM_u32 line;
    MM_u32 column;
} MM_Text_Pos;

typedef struct MM_Text
{
    union
    {
        struct MM_Text_Pos;
        MM_Text_Pos pos;
    };
    
    MM_u32 length;
} MM_Text;

typedef struct MM_Token
{
    MM_TOKEN_KIND kind;
    MM_u32 preceding_spacing;
    
    union
    {
        struct MM_Text;
        MM_Text text;
    };
} MM_Token;

typedef struct MM_Token_Array
{
    MM_u8* string_base;
    MM_Token* tokens;
    MM_umm count;
} MM_Token_Array;

typedef struct MM_Lexer
{
    MM_String string;
    MM_u64 offset;
    MM_u32 offset_to_line;
    MM_u32 line;
    
    MM_Token last_token;
} MM_Lexer;