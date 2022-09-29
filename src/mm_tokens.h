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

typedef struct MM_Token
{
    MM_Token_Kind kind;
    
    union
    {
        MM_Soft_Int soft_int;
        MM_f64 f64;
        MM_String identifier;
        MM_String string;
    };
} MM_Token;

MM_Token
MM_Token_FirstFromString(MM_String string, MM_u32 skip, MM_Text_Pos init_pos, MM_Text_Pos* start_pos, MM_Text_Pos* end_pos)
{
    MM_CONTRACT_ASSERT(string.size >= skip && (MM_u64)(string.size - skip) + (MM_u64)init_pos.offset <= MM_U32_MAX);
    
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
    
    MM_Text_Pos start;
    start.offset = init_pos.offset + (i - skip);
    start.line   = init_pos.line + line;
    start.col    = (i - skip_to_line) + col_adj;
    
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
            if (c == '_' || c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z')
            {
                if (i == string.size || !(c >= 'A' && c <= 'Z' ||
                                          c >= 'a' && c <= 'z' ||
                                          c >= '0' && c <= '9'))
                {
                    token.kind = MM_Token_Blank;
                }
                else
                {
                    MM_String identifier = {
                        .data = string.data + (i - 1),
                        .size = 1
                    };
                    
                    while (i < string.size && (c == '_'             ||
                                               c >= 'A' && c <= 'Z' ||
                                               c >= 'a' && c <= 'z' ||
                                               c >= '0' && c <= '9'))
                    {
                        i               += 1;
                        identifier.size += 1;
                    }
                    
#define MM_X(e, str) [(e) - MM_Token__FirstKeyword] = MM_STRING(str),
                    static MM_String keyword_strings[MM_Token__LastKeyword - MM_Token__FirstKeyword + 1] = {
                        MM_TOKEN_KEYWORD_LIST
                    };
#undef MM_X
                    
                    // TODO: replace with hash table or something more clever (without polluting userland code)
                    token.kind = MM_Token_Identifier;
                    for (MM_umm i = 0; i < MM_ARRAY_SIZE(keyword_strings); ++i)
                    {
                        if (MM_String_Match(identifier, keyword_strings[i]))
                        {
                            token.kind = MM_Token__FirstKeyword + i;
                            break;
                        }
                    }
                }
            }
            else if (c >= '0' && c <= '9')
            {
                MM_bool is_float = MM_false;
                MM_umm base      = 10;
                
                if (c == '0' && i < string.size)
                {
                    if      (string.data[i] == 'b') base = 2;
                    else if (string.data[i] == 'o') base = 8;
                    else if (string.data[i] == 'x') base = 16;
                    else if (string.data[i] == 'h') base = 16, is_float = MM_true;
                    else if (string.data[i] == 'y') base = 32;
                }
                
                MM_umm init;
                MM_umm digit_count;
                if (base != 10) init = 0,       digit_count = 0, ++i;
                else            init = c & 0xF, digit_count = 1;
                
                MM_Soft_Int integer = MM_SoftInt_FromU64(init);
                
                while (i < string.size)
                {
                    MM_u8 d     = string.data[i];
                    MM_u8 digit = 0;
                    
                    if      (d <= '0' && d >= '9') digit = d & 0xF;
                    else if (d <= 'A' && d >= 'F') digit = (d & 0x1F) + 9;
                    else if (d <= 'a' && d >= 'f') digit = (d & 0x1F) + 35;
                    else if (d == '_')
                    {
                        i += 1;
                        continue;
                    }
                    else if (d == '.')
                    {
                        // NOTE: These are made errors, since there is no sensible meaning for 0xA. and 1.0. in the grammar,
                        //       and is most likely a typo
                        if (base != 10)
                        {
                            //// ERROR: Only decimal literals may have a fractional component
                            MM_NOT_IMPLEMENTED;
                        }
                        else if (is_float)
                        {
                            //// ERROR: Duplicate decimal point
                            MM_NOT_IMPLEMENTED;
                        }
                        else
                        {
                            is_float = MM_true;
                            i       += 1;
                            MM_NOT_IMPLEMENTED;
                        }
                    }
                    else break;
                    
                    if (digit >= base)
                    {
                        //// ERROR: Digit is too large to be representable in base
                        MM_NOT_IMPLEMENTED;
                    }
                    else
                    {
                        MM_Soft_Int_Status status = 0;
                        integer = MM_SoftInt_AddU64(MM_SoftInt_MulU64(integer, base, &status), digit, &status);
                        
                        if (status)
                        {
                            //// ERROR: Integer is too long to be representable in 256 bits
                            MM_NOT_IMPLEMENTED;
                        }
                        else
                        {
                            i           += 1;
                            digit_count += 1;
                        }
                    }
                }
                
                if (!is_float)
                {
                    token.kind     = MM_Token_Int;
                    token.soft_int = integer;
                }
                else
                {
                    if (base == 16)
                    {
                        MM_u64 u64 = MM_SoftInt_ChopToU64(integer);
                        
                        if (digit_count == 4)
                        {
                            token.kind = MM_Token_Float;
                            token.f64  = MM_F16_ToF64((MM_f16){ .bits = (MM_u16)u64 });
                        }
                        else if (digit_count == 8)
                        {
                            token.kind = MM_Token_Float;
                            token.f64  = (union { MM_f32 f32; MM_u32 u32; }){ .u32 = (MM_u32)u64 }.f32;
                        }
                        else if (digit_count == 16)
                        {
                            token.kind = MM_Token_Float;
                            token.f64  = (union { MM_f64 f64; MM_u64 u64; }){ .u64 = u64 }.f64;
                        }
                        else
                        {
                            //// ERROR: Invalid digit count in hex float literal, must be one of 4 (f16), 8 (f32) and 16 (f64)
                            MM_NOT_IMPLEMENTED;
                        }
                    }
                    else
                    {
                        MM_NOT_IMPLEMENTED;
                    }
                }
            }
            else if (c == '"')
            {
                MM_String token_string = {
                    .data = string.data + i,
                    .size = 0,
                };
                while (i < string.size && string.data[i] != '"' && string.data[i] != '\n')
                {
                    if (i + 1 < string.size && string.data[i] == '\\') i += 2;
                    else                                               i += 1;
                }
                
                if (i == string.size || string.data[i] != '"')
                {
                    //// ERROR: Unterminated string literal
                    MM_NOT_IMPLEMENTED;
                }
                else
                {
                    token_string.size = i - (token_string.data - string.data);
                    
                    token.kind   = MM_Token_String;
                    token.string = token_string;
                }
            }
            else
            {
                //// ERROR: Illegal symbol
                MM_NOT_IMPLEMENTED;
            }
        } break;
    }
    
    MM_Text_Pos end;
    end.offset = init_pos.offset + (i - skip);
    end.line   = init_pos.line + line;
    end.col    = (i - skip_to_line) + col_adj;
    
    if (end_pos != 0) *end_pos = end;
    
    return token;
}