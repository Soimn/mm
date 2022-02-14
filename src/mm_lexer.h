internal inline bool
IsAlpha(u8 c)
{
    return (c >= 'a' && c <= 'z' ||
            c >= 'A' && c <= 'Z');
}

internal inline bool
IsNumeric(u8 c)
{
    return (c >= '0' && c <= '9');
}

internal inline bool
IsAlphaNumericOrUnderscore(u8 c)
{
    return (IsAlpha(c) || IsNumeric(c) || c == '_');
}

typedef enum TOKEN_KIND
{
    Token_Error = 0,
    Token_EndOfStream,
    
    Token_TripleMinus,                                   // ---
    Token_Arrow,                                         // ->
    Token_CloseParen,                                    // )
    Token_CloseBracket,                                  // ]
    Token_OpenBrace,                                     // {
    Token_CloseBrace,                                    // }
    Token_Comma,                                         // ,
    Token_Colon,                                         // :
    Token_Semicolon,                                     // ;
    Token_Underscore,                                    // _
    Token_QuestionMark,                                  // ?
    Token_Identifier,
    Token_String,
    Token_Character,
    Token_Int,
    Token_Float,
    Token_Not,                                           // !
    Token_Complement,                                    // ~
    
    Token_FirstPostfixLevel,
    Token_OpenParen,                                     // (
    Token_Period,                                        // .
    Token_OpenPeriodBrace,                               // .{
    Token_OpenPeriodBracket,                             // .[
    Token_OpenPeriodParen,                               // .(
    Token_OpenBracket,                                   // [
    Token_LastPostfixLevel,
    
    Token_FirstAssignment,
    Token_Equals = Token_FirstAssignment,                // =
    
    Token_FirstMulLevelAssignment = 2*16,
    Token_StarEquals = Token_FirstMulLevelAssignment,    // *=
    Token_SlashEquals,                                   // /=
    Token_RemEquals,                                     // %=
    Token_AndEquals,                                     // &=
    Token_ArithmeticRightShiftEquals,                    // >>>=
    Token_RightShiftEquals,                              // >>=
    Token_SplatLeftShiftEquals,                          // <<<=
    Token_LeftShiftEquals,                               // <<=
    Token_LastMulLevelAssignment = 3*16 - 1,
    
    Token_FirstAddLevelAssignment = 3*16,
    Token_PlusEquals = Token_FirstAddLevelAssignment,    // +=
    Token_MinusEquals,                                   // -=
    Token_OrEquals,                                      // |=
    Token_HatEquals,                                     // ^=
    Token_LastAddLevelAssignment = 4*16 - 1,
    
    Token_AndAndEquals = 5*16,                           // &&=
    
    Token_OrOrEquals = 6*16,                             // ||=
    
    Token_LastAssignment = 7*16 - 1,
    
    Token_FirstMulLevel = 8*16,
    Token_Star = Token_FirstMulLevel,                    // *
    Token_Slash,                                         // /
    Token_Rem,                                           // %
    Token_And,                                           // &
    Token_ArithmeticRightShift,                          // >>>
    Token_RightShift,                                    // >>
    Token_SplatLeftShift,                                // <<<
    Token_LeftShift,                                     // <<
    Token_LastMulLevel = 9*16 - 1,
    
    Token_FirstAddLevel = 9*16,
    Token_Plus = Token_FirstAddLevel,                    // +
    Token_Minus,                                         // -
    Token_Or,                                            // |
    Token_Hat,                                           // ^
    Token_LastAddLevel = 10*16 - 1,
    
    Token_FirstComparative = 10*16,
    Token_EqualEquals = Token_FirstComparative,          // ==
    Token_NotEquals,                                     // !=
    Token_Less,                                          // <
    Token_Greater,                                       // >
    Token_LessEquals,                                    // <=
    Token_GreaterEquals,                                 // >=
    Token_LastComparative = 11*16 - 1,
    
    Token_AndAnd = 11*16,                                // &&
    
    Token_OrOr = 12*16,                                  // ||
} TOKEN_KIND;

typedef struct Token
{
    TOKEN_KIND kind;
    u32 offset;
    u32 line;
    u32 column;
    u32 size;
    
    union
    {
        u8 character;
        u64 integer;
        f64 floating;
        Interned_String string;
        Interned_String identifier;
    };
} Token;

typedef struct Lexer
{
    String string;
    u32 offset;
    u32 line;
    u32 column;
} Lexer;

Lexer
Lexer_Init(u8* string)
{
    return (Lexer){
        .string = string,
        .offset = 0,
        .line   = 1,
        .column = 1,
    };
}

Token
Lexer_NextToken(Lexer* lexer)
{
    Token token = { .kind = Token_Error };
    
    /// Skip comments and whitespace
    while (lexer->offset < lexer->string.size)
    {
        u8 c = lexer->string.data[lexer->offset];
        
        if (c == ' '  || c == '\t' ||
            c == '\v' || c == '\r')
        {
            lexer->offset += 1;
            lexer->column += 1;
        }
        else if (c == '\n')
        {
            lexer->offset += 1;
            lexer->line   += 1;
            lexer->column  = 1;
        }
        else if (c == '/' && lexer->string.data[lexer->offset + 1] == '/')
        {
            lexer->offset += 2;
            
            while (lexer->offset < lexer->string.size && lexer->string.data[lexer->offset] != '\n') lexer->offset += 1;
        }
        else if (c == '/' && lexer->offset + 1 < lexer->string.size && lexer->string.data[lexer->offset + 1] == '*')
        {
            lexer->offset += 2;
            
            u32 nest_level = 1;
            while (lexer->offset < lexer->string.size && nest_level != 0)
            {
                if (lexer->string.data[lexer->offset] == '\n')
                {
                    lexer->offset += 1;
                    lexer->line   += 1;
                    lexer->column  = 1;
                }
                else
                {
                    u8 c0 = lexer->string.data[lexer->offset];
                    u8 c1 = (lexer->offset + 1 < lexer->string.size ? lexer->string.data[lexer->offset + 1] : 0);
                    
                    if (c0 == '/' && c1 == '*')
                    {
                        nest_level += 1;
                        lexer->offset     += 2;
                    }
                    else if (c0 == '*' && c1 == '/')
                    {
                        nest_level -= 1;
                        lexer->offset     += 2;
                    }
                    else lexer->offset += 1;
                }
            }
        }
        else break;
    }
    /// 
    
    token.offset = lexer->offset;
    token.line   = lexer->line;
    token.column = lexer->column;
    
    u8 c[3] = {
        lexer->offset + 0 < lexer->string.size ? lexer->string.data[lexer->offset + 0] : 0,
        lexer->offset + 1 < lexer->string.size ? lexer->string.data[lexer->offset + 1] : 0,
        lexer->offset + 2 < lexer->string.size ? lexer->string.data[lexer->offset + 2] : 0,
    };
    
    if (c[0] == 0) token.kind = Token_EndOfStream;
    else
    {
        lexer->offset += 1;
        
        switch (c[0])
        {
            case '(': token.kind = Token_OpenParen;    break;
            case ')': token.kind = Token_CloseParen;   break;
            case '[': token.kind = Token_OpenBracket;  break;
            case ']': token.kind = Token_CloseBracket; break;
            case '{': token.kind = Token_OpenBrace;    break;
            case '}': token.kind = Token_CloseBrace;   break;
            case ':': token.kind = Token_Colon;        break;
            case ',': token.kind = Token_Comma;        break;
            case ';': token.kind = Token_Semicolon;    break;
            case '?': token.kind = Token_QuestionMark; break;
            case '~': token.kind = Token_Complement;   break;
            
#define SINGLE_OR_EQ(single_c, single, eq) \
case single_c:                         \
{                                      \
token.kind = single;              \
if (c[1] == '=')                   \
{                                  \
token.kind = eq;              \
lexer->offset += 1;                   \
}                                  \
} break
            
            SINGLE_OR_EQ('!', Token_Not, Token_NotEquals);
            SINGLE_OR_EQ('+', Token_Plus, Token_PlusEquals);
            SINGLE_OR_EQ('*', Token_Star, Token_StarEquals);
            SINGLE_OR_EQ('=', Token_Equals, Token_EqualEquals);
            SINGLE_OR_EQ('/', Token_Slash, Token_SlashEquals);
            SINGLE_OR_EQ('%', Token_Rem, Token_RemEquals);
            SINGLE_OR_EQ('^', Token_Hat, Token_HatEquals);
            
#undef SINGLE_OR_EQ
            
            case '|':
            {
                token.kind = Token_Or;
                
                if (c[1] == '=')
                {
                    token.kind = Token_OrEquals;
                    lexer->offset     += 1;
                }
                
                if (c[1] == '|')
                {
                    token.kind = Token_OrOr;
                    lexer->offset     += 1;
                    
                    if (c[2] == '=')
                    {
                        token.kind = Token_OrOrEquals;
                        lexer->offset     += 1;
                    }
                }
            } break;
            
            case '&':
            {
                token.kind = Token_And;
                
                if (c[1] == '=')
                {
                    token.kind = Token_AndEquals;
                    lexer->offset     += 1;
                }
                
                if (c[1] == '&')
                {
                    token.kind = Token_AndAnd;
                    lexer->offset     += 1;
                    
                    if (c[2] == '=')
                    {
                        token.kind = Token_AndAndEquals;
                        lexer->offset     += 1;
                    }
                }
            } break;
            
            case '<':
            {
                token.kind = Token_Less;
                
                if (c[1] == '=')
                {
                    token.kind = Token_LessEquals;
                    lexer->offset     += 1;
                }
                
                if (c[1] == '<')
                {
                    token.kind = Token_LeftShift;
                    lexer->offset     += 1;
                    
                    if (c[2] == '=')
                    {
                        token.kind = Token_LeftShiftEquals;
                        lexer->offset     += 1;
                    }
                    
                    if (c[2] == '<')
                    {
                        token.kind = Token_SplatLeftShift;
                        lexer->offset     += 1;
                        
                        if (c[3] == '=')
                        {
                            token.kind = Token_SplatLeftShiftEquals;
                            lexer->offset     += 1;
                        }
                    }
                }
            } break;
            
            case '>':
            {
                token.kind = Token_Greater;
                
                if (c[1] == '=')
                {
                    token.kind = Token_GreaterEquals;
                    lexer->offset     += 1;
                }
                
                if (c[1] == '>')
                {
                    token.kind = Token_RightShift;
                    lexer->offset     += 1;
                    
                    if (c[2] == '=')
                    {
                        token.kind = Token_RightShiftEquals;
                        lexer->offset     += 1;
                    }
                    
                    if (c[2] == '>')
                    {
                        token.kind = Token_ArithmeticRightShift;
                        lexer->offset     += 1;
                        
                        if (c[3] == '=')
                        {
                            token.kind = Token_ArithmeticRightShiftEquals;
                            lexer->offset     += 1;
                        }
                    }
                }
            } break;
            
            case '.':
            {
                token.kind = Token_Period;
                
                if (c[1] == '{')
                {
                    token.kind = Token_OpenPeriodBrace;
                    lexer->offset     += 1;
                }
                
                if (c[1] == '[')
                {
                    token.kind = Token_OpenPeriodBracket;
                    lexer->offset     += 1;
                }
                
                if (c[1] == '(')
                {
                    token.kind = Token_OpenPeriodParen;
                    lexer->offset     += 1;
                }
            } break;
            
            case '-':
            {
                token.kind = Token_Minus;
                
                if (c[1] == '=')
                {
                    token.kind = Token_MinusEquals;
                    lexer->offset     += 1;
                }
                
                if (c[1] == '>')
                {
                    token.kind = Token_Arrow;
                    lexer->offset     += 1;
                }
                
                if (c[1] == '-' && c[2] == '-')
                {
                    token.kind = Token_TripleMinus;
                    lexer->offset     += 2;
                }
            } break;
            
            default:
            {
                if (c[0] == '_' && !IsAlphaNumericOrUnderscore(c[1]))
                {
                    token.kind = Token_Underscore;
                }
                
                else if (c[0] == '_' || IsAlpha(c[0]))
                {
                    String identifier = {
                        .data = lexer->string.data + lexer->offset - 1,
                        .size = 1,
                    };
                    
                    while (lexer->offset < lexer->string.size && IsAlphaNumericOrUnderscore(lexer->string.data[lexer->offset]))
                    {
                        identifier.size += 1;
                        lexer->offset          += 1;
                    }
                    
                    token.identifier = MM_InternString(identifier);
                }
                
                else if (IsNumeric(c[0]))
                {
                    bool encountered_errors = false;
                    
                    umm base      = 10;
                    bool is_float = false;
                    
                    u64 integer              = 0;
                    umm integer_digit_count  = 0;
                    u64 fraction             = 0;
                    umm fraction_digit_count = 0;
                    i64 exponent             = 0;
                    
                    bool integer_overflow = false;
                    
                    if (c[0] == '0')
                    {
                        if      (c[1] == 'y') base = 32;
                        else if (c[1] == 'x') base = 16;
                        else if (c[1] == 'h') base = 16, is_float = true;
                        else if (c[1] == 'b') base = 2;
                    }
                    
                    if (base != 10) lexer->offset += 1;
                    else            integer = c[0] - '0', integer_digit_count += 1;
                    
                    while (lexer->offset < lexer->string.size)
                    {
                        i8 digit = 0;
                        
                        u8 ch = lexer->string.data[lexer->offset];
                        if      (ch >= '0' && ch <= '9') digit = ch - '0';
                        else if (ch >= 'a' && ch <= 'v') digit = (ch - 'a') + 10;
                        else if (ch >= 'A' && ch <= 'V') digit = (ch - 'A') + 10;
                        else if (ch == '_') continue;
                        else                break;
                        
                        if (digit >= base)
                        {
                            //// ERROR: Invalid digit in base % constant
                            encountered_errors = true;
                            break;
                        }
                        else
                        {
                            u64 old_integer = integer;
                            integer = integer*10 + digit;
                            
                            integer_overflow = (integer_overflow || old_integer > integer);
                            integer_digit_count += 1;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        if (integer_digit_count == 0)
                        {
                            //// ERROR: Missing digits after base % literal prefix
                            encountered_errors = true;
                        }
                        else if (integer_overflow)
                        {
                            //// ERROR: Internal error. Literal is too large to be parsed by the lexer
                            encountered_errors = true;
                        }
                        else
                        {
                            if (lexer->offset < lexer->string.size && lexer->string.data[lexer->offset] == '.')
                            {
                                if (is_float)
                                {
                                    //// ERROR: Hexfloats must be separated from periods
                                    encountered_errors = true;
                                }
                                else
                                {
                                    lexer->offset += 1;
                                    
                                    integer_overflow = false;
                                    while (lexer->offset < lexer->string.size)
                                    {
                                        i8 digit = 0;
                                        
                                        u8 ch = lexer->string.data[lexer->offset];
                                        if      (ch >= '0' && ch <= '9') digit = ch - '0';
                                        else if (ch == '_') continue;
                                        else                break;
                                        
                                        u64 old_fraction = fraction;
                                        
                                        fraction = fraction*10 + digit;
                                        
                                        integer_overflow = (integer_overflow || old_fraction > fraction);
                                        fraction_digit_count += 1;
                                    }
                                    
                                    if (fraction_digit_count == 0)
                                    {
                                        //// ERROR: Missing digits after decimal point
                                        encountered_errors = true;
                                    }
                                    else if (integer_overflow)
                                    {
                                        //// ERROR: Internal Error. Floating point literal has too many decimals to be parsed by the lexer
                                        encountered_errors = true;
                                    }
                                    else
                                    {
                                        if (lexer->offset < lexer->string.size && (lexer->string.data[lexer->offset] == 'e' || lexer->string.data[lexer->offset] == 'E'))
                                        {
                                            if (base != 10)
                                            {
                                                //// ERROR: scientific notation is only allowed for base 10
                                                encountered_errors = true;
                                            }
                                            else
                                            {
                                                lexer->offset += 1;
                                                
                                                umm exponent_digit_count = 0;
                                                
                                                imm sign = 1;
                                                if (lexer->offset < lexer->string.size && (lexer->string.data[lexer->offset] == '+' || lexer->string.data[lexer->offset] == '-'))
                                                {
                                                    lexer->offset += 1;
                                                    sign = (lexer->string.data[lexer->offset] == '+' ? 1 : -1);
                                                }
                                                
                                                integer_overflow = false;
                                                while (lexer->offset < lexer->string.size)
                                                {
                                                    i8 digit = 0;
                                                    
                                                    u8 ch = lexer->string.data[lexer->offset];
                                                    if      (ch >= '0' && ch <= '9') digit = ch - '0';
                                                    else if (ch == '_') continue;
                                                    else                break;
                                                    
                                                    i64 old_exponent = exponent;
                                                    
                                                    exponent = exponent*10 + digit;
                                                    
                                                    integer_overflow = (integer_overflow || old_exponent > exponent);
                                                    exponent_digit_count += 1;
                                                }
                                                
                                                exponent *= sign;
                                                
                                                if (exponent_digit_count == 0)
                                                {
                                                    //// ERROR: Missing digits of exponent after scientific notation suffix
                                                    encountered_errors = true;
                                                }
                                                else if (integer_overflow)
                                                {
                                                    //// ERROR: Internal Error. Exponent is long to be parsed by the lexer
                                                    encountered_errors = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            
                            if (!encountered_errors)
                            {
                                if (lexer->offset < lexer->string.size && IsAlpha(lexer->string.data[lexer->offset]))
                                {
                                    //// ERROR: Numeric literals must be separated from identifiers be at least on non alphabetical character
                                    encountered_errors = true;
                                }
                            }
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        if (is_float && base == 16)
                        {
                            if (integer_digit_count == 8)
                            {
                                f32 f;
                                Copy((u32*)&integer, &f, sizeof(u32));
                                
                                token.kind     = Token_Float;
                                token.floating = (f64)f;
                            }
                            else if (integer_digit_count == 16)
                            {
                                f64 f;
                                Copy(&integer, &f, sizeof(u64));
                                
                                token.kind     = Token_Float;
                                token.floating = f;
                            }
                            else
                            {
                                //// ERROR: Invalid digit count for hex float. Hex floats must be either 4, 8 or 16 digits long
                                encountered_errors = true;
                            }
                        }
                        else if (is_float)
                        {
                            token.kind = Token_Float;
                            NOT_IMPLEMENTED;
                        }
                        else
                        {
                            token.kind    = Token_Int;
                            token.integer = integer;
                        }
                    }
                }
                
                else if (c[0] == '"')
                {
                    umm start = lexer->offset;
                    
                    while (lexer->offset < lexer->string.size && lexer->string.data[lexer->offset] != '"')
                    {
                        if (lexer->string.data[lexer->offset] == '\\') lexer->offset += 1;
                        lexer->offset += 1;
                    }
                    
                    if (lexer->offset == lexer->string.size)
                    {
                        //// ERROR: Unterminated lexer->string literal
                    }
                    else
                    {
                        lexer->offset += 1;
                        
                        String string = {
                            .data = lexer->string.data + start,
                            .size = lexer->offset - start,
                        };
                        
                        (void)string;
                        NOT_IMPLEMENTED;
                    }
                }
                
                else if (c[0] == '\'')
                {
                    bool encountered_errors = false;
                    
                    u8 character = c[1];
                    
                    umm terminator_index = 2;
                    
                    if (c[1] == '\\')
                    {
                        switch (c[2])
                        {
                            case 'a':  character = 0x07; break;
                            case 'b':  character = 0x08; break;
                            case 't':  character = 0x09; break;
                            case 'n':  character = 0x0A; break;
                            case 'v':  character = 0x0B; break;
                            case 'f':  character = 0x0C; break;
                            case 'r':  character = 0x0D; break;
                            case 'e':  character = 0x1B; break;
                            case '"':  character = 0x22; break;
                            case '\'': character = 0x27; break;
                            case '\\': character = 0x5C; break;
                            
                            default:
                            {
                                //// ERROR: Invalid character escape sequence
                                encountered_errors = true;
                            } break;
                        }
                        
                        terminator_index = 3;
                    }
                    else if (c[1] == '\'')
                    {
                        //// ERROR: Missing character in character litteral
                        encountered_errors = true;
                    }
                    
                    if (c[terminator_index] != '\'')
                    {
                        //// ERROR: Missing terminating ' character in character litteral
                    }
                    else
                    {
                        token.kind      = Token_Character;
                        token.character = character;
                    }
                }
                else
                {
                    //// ERROR: Unknown symbol
                }
            } break;
        }
        
        token.size = lexer->offset - token.offset;
    }
    
    return token;
}