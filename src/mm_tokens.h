#define MM_TOKEN_BLOCK(I) ((I) << 4)
#define MM_TOKEN_BLOCK_INDEX(K) ((K) >> 4)
#define MM_TOKEN_BLOCK_OFFSET(K) ((K) & 0xF)

typedef MM_u32 MM_Token_Kind;
enum MM_TOKEN_KIND
{
    MM_Token_Invalid = 0,
    MM_Token_EOF,
    
    MM_Token_Identifier,                         // _identifier1, ident1fier,
    MM_Token_String,                             // "aæ"
    MM_Token_Int,                                // 3, 0b11, 0o3, 0d3, 0x3, 0y3, 0s3
    MM_Token_Float,                              // 3.14, 0h4247, 0h40490FDB, 0h400921FB54442D18
    
    MM_Token_OpenParen,                          // (
    MM_Token_CloseParen,                         // )
    MM_Token_OpenBracket,                        // [
    MM_Token_CloseBracket,                       // ]
    MM_Token_OpenBrace,                          // {
    MM_Token_CloseBrace,                         // }
    MM_Token_Hat,                                // ^
    MM_Token_Comma,                              // ,
    MM_Token_Colon,                              // :
    MM_Token_Semicolon,                          // ;
    
    MM_Token_Period,                             // .
    MM_Token_PeriodParen,                        // .(
    MM_Token_PeriodBracket,                      // .[
    MM_Token_PeriodBrace,                        // .{
    MM_Token_Bang,                               // !
    MM_Token_Arrow,                              // ->
    MM_Token_Blank,                              // _
    
    MM_Token__FirstAssignment,
    MM_Token_Equals = MM_Token__FirstAssignment,
    
    MM_Token_StarEQ = MM_TOKEN_BLOCK(2),         // *=
    MM_Token_SlashEQ,                            // /=
    MM_Token_PercentEQ,                          // %=
    MM_Token_AndEQ,                              // &=
    MM_Token_ShlEQ,                              // <<=
    MM_Token_ShrEQ,                              // >>=
    MM_Token_SarEQ,                              // >>>=
    
    MM_Token_PlusEQ = MM_TOKEN_BLOCK(3),         // +=
    MM_Token_MinusEQ,                            // -=
    MM_Token_OrEQ,                               // |=
    MM_Token_TildeEQ,                            // ~=
    
    MM_Token_AndAndEQ = MM_TOKEN_BLOCK(4),       // &&=
    
    MM_Token_OrOrEQ = MM_TOKEN_BLOCK(5),         // ||=
    MM_Token__LastAssignment = MM_TOKEN_BLOCK(6) - 1,
    
    MM_Token__FirstBinary = MM_TOKEN_BLOCK(6),
    MM_Token_Star = MM_Token__FirstBinary,       // *
    MM_Token_Slash,                              // /
    MM_Token_Percent,                            // %
    MM_Token_And,                                // &
    MM_Token_Shl,                                // <<
    MM_Token_Shr,                                // >>
    MM_Token_Sar,                                // >>>
    
    MM_Token_Plus = MM_TOKEN_BLOCK(7),           // +
    MM_Token_Minus,                              // -
    MM_Token_Or,                                 // |
    MM_Token_Tilde,                              // ~
    
    MM_Token_EqualEQ = MM_TOKEN_BLOCK(8),        // ==
    MM_Token_BangEQ,                             // !=
    MM_Token_Less,                               // <
    MM_Token_LessEQ,                             // <=
    MM_Token_Greater,                            // >
    MM_Token_GreaterEQ,                          // >=
    
    MM_Token_AndAnd = MM_TOKEN_BLOCK(9),         // &&
    
    MM_Token_OrOr = MM_TOKEN_BLOCK(10),          // ||
    MM_Token__LastBinary = MM_TOKEN_BLOCK(11) - 1,
    
    MM_Token_If,                                 // if
    MM_Token_When,                               // when
    MM_Token_Else,                               // else
    MM_Token_While,                              // while
    MM_Token_Break,                              // break
    MM_Token_Continue,                           // continue
    MM_Token_Return,                             // return
    MM_Token_Proc,                               // proc
    MM_Token_Struct,                             // struct
    MM_Token_Enum,                               // enum
    
    MM_Token_Cast,                               // cast
    MM_Token_Transmute,                          // transmute
    MM_Token_Sizeof,                             // sizeof
    MM_Token_Alignof,                            // alignof
    MM_Token_Offsetof,                           // offsetof
    MM_Token_Min,                                // min
    MM_Token_Max,                                // max
    
};

// NOTE: Testing for overlap between token blocks and preceeding tokens
MM_STATIC_ASSERT(MM_TOKEN_BLOCK_INDEX(MM_Token_Equals) < MM_TOKEN_BLOCK_INDEX(MM_Token_StarEQ));
MM_STATIC_ASSERT(MM_TOKEN_BLOCK_INDEX(MM_Token__LastAssignment) < MM_TOKEN_BLOCK_INDEX(MM_Token__FirstBinary));

typedef struct MM_Text_Pos
{
    MM_u32 offset;
    MM_u32 line;
    MM_u32 col;
} MM_Text_Pos;

typedef struct MM_Token
{
    MM_Token_Kind kind;
    
    union
    {
        MM_Soft_Int soft_int;
        MM_Soft_Float soft_float;
        MM_String identifier;
        MM_String string;
    };
} MM_Token;

MM_Token
MM_Token_FirstFromString(MM_String string, MM_u32 skip, MM_Text_Pos init_pos, MM_Text_Pos* start_pos, MM_Text_Pos* end_pos)
{
    MM_Token token = { .kind = MM_Token_Invalid };
    
    MM_u32 i = skip;
    
    MM_u32 skip_to_line  = 0;
    MM_u32 col_adj       = init_pos.col;
    MM_u32 line          = 0;
    MM_u32 comment_depth = 0;
    while (i < string.size)
    {
        MM_u8 c = *string.data;
        
        if (c == '\n')
        {
            skip_to_line = i;
            col_adj      = 0;
            line        += 1;
        }
        else if (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r')
        {
            line += 1;
        }
        else if (i + 1 < string.size && string.data[i] == '/' && string.data[i + 1] == '*')
        {
            while (i < string.size && string.data[i] != '\n') i += 1;
        }
        else if (i + 1 < string.size && string.data[i] == '/' && string.data[i + 1] == '*')
        {
            i             += 2;
            comment_depth += 1;
        }
        else if (i + 1 < string.size && string.data[i] == '*' && string.data[i + 1] == '/' &&
                 comment_depth > 0)
        {
            i             += 2;
            comment_depth -= 1;
        }
        else
        {
            if (comment_depth > 0) i += 1;
            else                   break;
        }
    }
    
    // NOTE: (string.size - skip) + init_pos.offset + init_pos.col must be < MM_U32_MAX to avoid overflow
    MM_Text_Pos start;
    start.offset = init_pos.offset + (i - skip); // TODO: overflow
    start.line   = init_pos.line + line; // TODO: overflow
    start.col    = (i - skip_to_line) + col_adj; // TODO: overflow
    
    if (start_pos != 0) *start_pos = start;
    
    MM_u8 c;
    if (i < string.size) c = string.data[i++];
    else                 c = 0;
    
    switch (c)
    {
        case 0: token.kind = MM_Token_EOF; break;
        
        case '(': token.kind = MM_Token_OpenParen;    break;
        case ')': token.kind = MM_Token_CloseParen;   break;
        case '[': token.kind = MM_Token_OpenBracket;  break;
        case ']': token.kind = MM_Token_CloseBracket; break;
        case '{': token.kind = MM_Token_OpenBrace;    break;
        case '}': token.kind = MM_Token_CloseBrace;   break;
        case '^': token.kind = MM_Token_Hat;          break;
        case ',': token.kind = MM_Token_Comma;        break;
        case ':': token.kind = MM_Token_Colon;        break;
        case ';': token.kind = MM_Token_Semicolon;    break;
        
#define MM_TOKEN_SINGLE_OR_EQ(ch, single, eq)                                 \
case (ch):                                                                \
{                                                                         \
if (i < string.size && string.data[i] == '=') token.kind = (eq), ++i; \
else                                          token.kind = (single);  \
} break
        
        MM_TOKEN_SINGLE_OR_EQ('+', MM_Token_Plus,  MM_Token_PlusEQ);
        MM_TOKEN_SINGLE_OR_EQ('~', MM_Token_Tilde, MM_Token_TildeEQ);
        MM_TOKEN_SINGLE_OR_EQ('*', MM_Token_Star,  MM_Token_StarEQ);
        MM_TOKEN_SINGLE_OR_EQ('/', MM_Token_Slash, MM_Token_SlashEQ);
        MM_TOKEN_SINGLE_OR_EQ('!', MM_Token_Bang,  MM_Token_BangEQ);
        
#undef MM_TOKEN_SINGLE_OR_EQ
        
#define MM_TOKEN_SINGLE_DOUBLE_OR_EQ(ch, single, singleeq, dub, dubeq)                   \
case (ch):                                                                           \
{                                                                                    \
if      (i < string.size && string.data[i] == '=') token.kind = (singleeq), ++i; \
else if (i < string.size && string.data[i] == (ch))                              \
{                                                                                \
i += 1;                                                                      \
if (i < string.size && string.data[i] == '=') token.kind = (dubeq), ++i;     \
else                                          token.kind = (dub);            \
}                                                                                \
else token.kind = (single);                                                      \
} break
        
        MM_TOKEN_SINGLE_DOUBLE_OR_EQ('&', MM_Token_And, MM_Token_AndEQ, MM_Token_AndAnd, MM_Token_AndAndEQ);
        MM_TOKEN_SINGLE_DOUBLE_OR_EQ('|', MM_Token_Or, MM_Token_OrEQ, MM_Token_OrOr, MM_Token_OrOrEQ);
        
#undef MM_TOKEN_SINGLE_DOUBLE_OR_EQ
        
        case '<':
        {
            if      (i < string.size && string.data[i] == '=') token.kind = MM_Token_LessEQ, ++i;
            else if (i < string.size && string.data[i] == '<')
            {
                i += 1;
                
                if (i < string.size && string.data[i] == '=') token.kind = MM_Token_ShlEQ, ++i;
                else                                          token.kind = MM_Token_Shl;
            }
            else token.kind = MM_Token_Less;
        } break;
        
        case '>':
        {
            if      (i < string.size && string.data[i] == '=') token.kind = MM_Token_GreaterEQ, ++i;
            else if (i < string.size && string.data[i] == '>')
            {
                i += 1;
                
                if      (i < string.size && string.data[i] == '=') token.kind = MM_Token_ShrEQ, ++i;
                else if (i < string.size && string.data[i] == '>')
                {
                    i += 1;
                    
                    if (i < string.size && string.data[i] == '=') token.kind = MM_Token_SarEQ, ++i;
                    else                                          token.kind = MM_Token_Sar;
                }
                else token.kind = MM_Token_Shr;
            }
            else token.kind = MM_Token_Greater;
        } break;
        
        case '-':
        {
            if (i < string.size && string.data[i] == '>') token.kind = MM_Token_Arrow, ++i;
            else                                          token.kind = MM_Token_Minus;
        } break;
        
        case '.':
        {
            if      (i < string.size && string.data[i] == '(') token.kind = MM_Token_PeriodParen,   ++i;
            else if (i < string.size && string.data[i] == '[') token.kind = MM_Token_PeriodBracket, ++i;
            else if (i < string.size && string.data[i] == '{') token.kind = MM_Token_PeriodBrace,   ++i;
            else                                               token.kind = MM_Token_Period;
        } break;
        
        default:
        {
            // NOTE: This is just for fun, probably not a good idea (although I did spend some time fiddling in godbolt)
#define MM_IS_ALPHA(c) (((c) >= 0x40) & (((c) & 0x1F) - 1 < 26))
#define MM_IS_ALPHANUM(c) (((c) >= 0x30) & (((c) & 0x1F) - 1 < 10 + 16*(c >= 0x40)))
            
            // TODO: REMOVE
#define MM_NOT_IMPLEMENTED
            if ((c == '_') | MM_IS_ALPHA(c))
            {
                if (i == string.size || !MM_IS_ALPHA(string.data[i])) token.kind = MM_Token_Blank;
                else
                {
                    MM_String identifier = {
                        .data = string.data + (i - 1),
                        .size = 1
                    };
                    
                    while (i < string.size && ((c == '_') | MM_IS_ALPHANUM(string.data[i]))) ++i, ++identifier.size;
                    
                    // TODO: Hash lookup
                    MM_NOT_IMPLEMENTED;
                }
            }
            else if (c >= '0' && c <= '9')
            {
                MM_NOT_IMPLEMENTED;
            }
            else if (c == '"' || c == '\'')
            {
                MM_NOT_IMPLEMENTED;
            }
            else
            {
                //// ERROR: Illegal symbol
                MM_NOT_IMPLEMENTED;
            }
            
#undef MM_IS_ALPHA
#undef MM_IS_ALPHANUM
        } break;
    }
    
    // NOTE: same as start
    MM_Text_Pos end;
    end.offset = init_pos.offset + (i - skip); // TODO: overflow
    end.line   = init_pos.line + line; // TODO: overflow
    end.col    = (i - skip_to_line) + col_adj; // TODO: overflow
    
    if (end_pos != 0) *end_pos = end;
    
    return token;
}