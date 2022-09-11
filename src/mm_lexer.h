#define MM_TOKEN_BINARY_OP_BLOCK_INDEX(kind) ((kind) / 16 - 4)
#define MM_TOKEN_BINARY_OP_BLOCK_OFFSET(kind) ((kind) % 16)

typedef enum MM_TOKEN_KIND
{
    MM_Token_Invalid = 0,
    MM_Token_EndOfStream,
    
    MM_Token_Identifier,
    MM_Token_String,
    MM_Token_Codepoint,
    MM_Token_Int,
    MM_Token_Float,
    
    MM_Token_Bang,
    MM_Token_Backslash,
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
    
    MM_Token_FirstAssignment,
    MM_Token_Equals = MM_Token_FirstAssignment,
    MM_Token_StarEquals,
    MM_Token_SlashEquals,
    MM_Token_PercentEquals,
    MM_Token_AndEquals,
    MM_Token_LessLessEquals,
    MM_Token_TripleLessEquals,
    MM_Token_GreaterGreaterEquals,
    MM_Token_TripleGreaterEquals,
    MM_Token_TildeEquals,
    MM_Token_OrEquals,
    MM_Token_MinusEquals,
    MM_Token_PlusEquals,
    MM_Token_AndAndEquals,
    MM_Token_OrOrEquals,
    MM_Token_LastAssignment = MM_Token_OrOrEquals,
    
    MM_Token_FirstBinary = 4*16,
    MM_Token_FirstMulLevel = MM_Token_FirstBinary,
    MM_Token_Star = MM_Token_FirstMulLevel,
    MM_Token_Slash,
    MM_Token_Percent,
    MM_Token_And,
    MM_Token_LessLess,
    MM_Token_TripleLess,
    MM_Token_GreaterGreater,
    MM_Token_TripleGreater,
    MM_Token_LastMulLevel = MM_Token_TripleGreater,
    
    MM_Token_FirstAddLevel = 5*16,
    MM_Token_Plus  = MM_Token_FirstAddLevel,
    MM_Token_Minus,
    MM_Token_Or,
    MM_Token_Tilde,
    MM_Token_LastAddLevel = MM_Token_Tilde,
    
    MM_Token_FirstCmpLevel = 6*16,
    MM_Token_Greater = MM_Token_FirstCmpLevel,
    MM_Token_GreaterEquals,
    MM_Token_Less,
    MM_Token_LessEquals,
    MM_Token_EqualsEquals,
    MM_Token_BangEquals,
    MM_Token_LastCmpLevel = MM_Token_BangEquals,
    
    MM_Token_AndAnd = 7*16,
    
    MM_Token_OrOr = 8*16,
    MM_Token_LastBinary = MM_Token_OrOr,
    
    MM_Token_FirstReservedIdentifier,
    MM_Token_FirstKeyword = MM_Token_FirstReservedIdentifier,
    MM_Token_Include = MM_Token_FirstKeyword,
    MM_Token_Proc,
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
    MM_Token_Distinct,
    MM_Token_LastKeyword = MM_Token_Distinct,
    
    MM_Token_FirstBuiltin,
    MM_Token_Cast = MM_Token_FirstBuiltin,
    MM_Token_Transmute,
    MM_Token_Sizeof,
    MM_Token_Alignof,
    MM_Token_Offsetof,
    MM_Token_Typeof,
    MM_Token_Min,
    MM_Token_Max,
    MM_Token_Len,
    MM_Token_Memcopy,
    MM_Token_Memmove,
    MM_Token_Memset,
    MM_Token_Memzero,
    MM_Token_SourcePos,
    MM_Token_LastBuiltin = MM_Token_SourcePos,
    MM_Token_LastReservedIdentifier = MM_Token_LastBuiltin,
} MM_TOKEN_KIND;

typedef struct MM_Token
{
    MM_TOKEN_KIND kind;
    MM_Text text;
    
    union
    {
        MM_Soft_Int soft_int;
        MM_Soft_Float soft_float;
        MM_Identifier identifier;
        MM_String_Literal string_literal;
        MM_Codepoint codepoint;
    };
} MM_Token;

typedef struct MM_Lexer
{
    MM_u8* content;
    
    union
    {
        struct MM_Text_Pos;
        MM_Text_Pos pos;
    };
} MM_Lexer;

MM_Lexer
MM_Lexer_Init(MM_u8* content, MM_Text_Pos pos)
{
    return (MM_Lexer){
        .content = content,
        .pos     = pos,
    };
}

MM_Token
MM_Lexer_NextToken(MM_Lexer* lexer)
{
    MM_Token token = { .kind = MM_Token_Invalid };
    
    for (;;)
    {
        char c = lexer->content[lexer->offset];
        
        if (c == '\n' || c == '\r' ||
            c == '\f' || c == 'v'  ||
            c == '\t' || c == ' ')
        {
            lexer->offset += 1;
        }
        else if (c == '/' && lexer->content[lexer->offset + 1] == '*')
        {
            lexer->offset += 2;
            
            MM_u32 level = 1;
            
            while (lexer->content[lexer->offset] != 0 && level != 0)
            {
                char c0 = lexer->content[lexer->offset];
                char c1 = lexer->content[lexer->offset + 1];
                
                if (c0 == '/' && c1 == '*')
                {
                    lexer->offset += 2;
                    level         += 1;
                }
                else if (c0 == '*' && c1 == '/')
                {
                    lexer->offset += 2;
                    level         -= 1;
                }
                else lexer->offset += 1;
            }
            
            if (level != 0)
            {
                //// ERROR: Unterminated block comment
                MM_NOT_IMPLEMENTED;
            }
        }
        else break;
    }
    
    MM_Text_Pos start_pos = lexer->pos;
    
    char c[4] = {};
    for (MM_umm i = 0; i < MM_ARRAY_SIZE(c) && lexer->content[lexer->offset + i]; ++i)
    {
        c[i] = lexer->content[lexer->offset + i];
    }
    
    lexer->offset += (c[0] != 0);
    
    switch (c[0])
    {
        case '\0': token.kind = MM_Token_EndOfStream;  break;
        case '\\': token.kind = MM_Token_Backslash;    break;
        case '(':  token.kind = MM_Token_OpenParen;    break;
        case ')':  token.kind = MM_Token_CloseParen;   break;
        case '[':  token.kind = MM_Token_OpenBracket;  break;
        case ']':  token.kind = MM_Token_CloseBracket; break;
        case '{':  token.kind = MM_Token_OpenBrace;    break;
        case '}':  token.kind = MM_Token_CloseBrace;   break;
        case ':':  token.kind = MM_Token_Colon;        break;
        case ',':  token.kind = MM_Token_Comma;        break;
        case ';':  token.kind = MM_Token_Semicolon;    break;
        case '?':  token.kind = MM_Token_QuestionMark; break;
        case '^':  token.kind = MM_Token_Hat;          break;
        
        case '*': token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_StarEquals    : MM_Token_Star);    break;
        case '/': token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_SlashEquals   : MM_Token_Slash);   break;
        case '%': token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_PercentEquals : MM_Token_Percent); break;
        case '+': token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_PlusEquals    : MM_Token_Plus);    break;
        case '=': token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_EqualsEquals  : MM_Token_Equals);  break;
        case '!': token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_BangEquals    : MM_Token_Bang);    break;
        case '~': token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_TildeEquals   : MM_Token_Tilde);   break;
        case '-':
        {
            token.kind = MM_Token_Minus;
            if (c[1] == '=')                lexer->offset += 1, token.kind = MM_Token_MinusEquals;
            if (c[1] == '>')                lexer->offset += 1, token.kind = MM_Token_Arrow;
            if (c[1] == '-' && c[2] == '-') lexer->offset += 2, token.kind = MM_Token_TripleMinus;
        } break;
        
        case '.':
        {
            token.kind = MM_Token_Period;
            if (c[1] == '{') ++lexer->offset, token.kind = MM_Token_PeriodOpenBrace;
            if (c[1] == '[') ++lexer->offset, token.kind = MM_Token_PeriodOpenBracket;
        }
        
        case '>':
        {
            token.kind = MM_Token_Greater;
            if (c[1] == '=')                               lexer->offset += 1, token.kind = MM_Token_GreaterEquals;
            if (c[1] == '>' && c[2] != '=')                lexer->offset += 1, token.kind = MM_Token_GreaterGreater;
            if (c[1] == '>' && c[2] == '=')                lexer->offset += 2, token.kind = MM_Token_GreaterGreaterEquals;
            if (c[1] == '>' && c[2] == '>' && c[3] != '=') lexer->offset += 2, token.kind = MM_Token_TripleGreater;
            if (c[1] == '>' && c[2] == '>' && c[3] == '=') lexer->offset += 3, token.kind = MM_Token_TripleGreaterEquals;
        } break;
        
        case '<':
        {
            token.kind = MM_Token_Less;
            if (c[1] == '=')                               lexer->offset += 1, token.kind = MM_Token_LessEquals;
            if (c[1] == '<' && c[2] != '=')                lexer->offset += 1, token.kind = MM_Token_LessLess;
            if (c[1] == '<' && c[2] == '=')                lexer->offset += 2, token.kind = MM_Token_LessLessEquals;
            if (c[1] == '<' && c[2] == '<' && c[3] != '=') lexer->offset += 2, token.kind = MM_Token_TripleLess;
            if (c[1] == '<' && c[2] == '<' && c[3] == '=') lexer->offset += 3, token.kind = MM_Token_TripleLessEquals;
        } break;
        
        case '&':
        {
            token.kind = MM_Token_And;
            if (c[1] == '=')                lexer->offset += 1, token.kind = MM_Token_AndEquals;
            if (c[1] == '&' && c[2] != '=') lexer->offset += 1, token.kind = MM_Token_AndAnd;
            if (c[1] == '&' && c[2] == '=') lexer->offset += 2, token.kind = MM_Token_AndAndEquals;
        } break;
        
        case '|':
        {
            token.kind = MM_Token_Or;
            if (c[1] == '=')                lexer->offset += 1, token.kind = MM_Token_OrEquals;
            if (c[1] == '|' && c[2] != '=') lexer->offset += 1, token.kind = MM_Token_OrOr;
            if (c[1] == '|' && c[2] == '=') lexer->offset += 2, token.kind = MM_Token_OrOrEquals;
        } break;
        
        default:
        {
            if (c[0] == '_' || c[0] >= 'a' && c[0] <= 'z' || c[0] >= 'A' && c[0] <= 'Z')
            {
                MM_umm ident_offset = lexer->offset - 1;
                
                while (c[0] == '_'                ||
                       c[0] >= 'a' && c[0] <= 'z' ||
                       c[0] >= 'A' && c[0] <= 'Z' ||
                       c[0] >= '0' && c[0] <= '9')
                {
                    lexer->offset += 1;
                }
                
                MM_String identifier = {
                    .data = lexer->content + ident_offset,
                    .size = lexer->offset  - ident_offset,
                };
                
                MM_NOT_IMPLEMENTED;
            }
            else if (c[0] >= '0' && c[0] <= '9')
            {
                MM_umm base      = 10;
                MM_bool is_float = MM_false;
                
                if (c[0] == 0)
                {
                    if      (c[1] == 'b') base = 2;
                    else if (c[1] == 'o') base = 8;
                    else if (c[1] == 'z') base = 12;
                    else if (c[1] == 'x') base = 16;
                    else if (c[1] == 'h') base = 16, is_float = MM_true;
                    else if (c[1] == 'y') base = 32; // base32hex
                    else if (c[1] == 's') base = 60; // sexagesimalhex
                }
                
                MM_Soft_Int integer;
                if (base == 10) integer = MM_SoftInt_FromU64(c[0] & 0xF);
                else
                {
                    integer        = MM_SoftInt_FromU64(0);
                    lexer->offset += 2;
                }
                
                for (;; lexer->offset += 1)
                {
                    char d = lexer->content[lexer->offset];
                    
                    MM_umm digit;
                    if      (d >= '0' && d <= '9') digit = d & 0xF;
                    else if (d >= 'A' && d <= 'Z') digit = (d & 0x1F) + 9;
                    else if (d >= 'a' && d <= 'z') digit = (d & 0x1F) + 36;
                    else if (d == '_') continue;
                    else if (d == '.')
                    {
                        if (base != 10)
                        {
                            //// ERROR: Only base 10 literals may have a fractional part
                            MM_NOT_IMPLEMENTED;
                        }
                        else
                        {
                            is_float = MM_true;
                            break;
                        }
                    }
                    else break;
                    
                    if (digit >= base)
                    {
                        //// ERROR: Digit too large for specified base
                        MM_NOT_IMPLEMENTED;
                    }
                    else
                    {
                        MM_Soft_Int_Status status = MM_SoftIntStatus_None;
                        
                        integer = MM_SoftInt_AddU64(MM_SoftInt_MulU64(integer, base, &status), digit, &status);
                        
                        if (status != MM_SoftIntStatus_None)
                        {
                            //// ERROR: Integer part of numeric literal exceeds 256 bits
                            MM_NOT_IMPLEMENTED;
                        }
                    }
                }
                
                if (!is_float)
                {
                    MM_NOT_IMPLEMENTED;
                }
                else
                {
                    if (base == 16)
                    {
                        if (digit_count == 4)
                        {
                            MM_NOT_IMPLEMENTED;
                        }
                        else if (digit_count == 8)
                        {
                            MM_NOT_IMPLEMENTED;
                        }
                        else if (digit_count == 16)
                        {
                            MM_NOT_IMPLEMENTED;
                        }
                        else
                        {
                            //// ERROR: Invalid digit count for hex float literal. Must be either 4, 8 or 16, corresponding to
                            ////        16-, 32- and 64-bit IEEE754 float little endian bit representation, respectively.
                            MM_NOT_IMPLEMENTED;
                        }
                    }
                    else
                    {
                        MM_ASSERT(lexer->content[lexer->offset] == '.');
                        lexer->offset += 1;
                        
                        MM_NOT_IMPLEMENTED;
                    }
                }
            }
            else if (c[0] == '"' || c[0] == '\'')
            {
                MM_NOT_IMPLEMENTED;
            }
            else
            {
                //// ERROR: Unknown token
                MM_NOT_IMPLEMENTED;
            }
        } break;
    }
    
    return token;
}