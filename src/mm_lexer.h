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
    Token_Elipsis,                                       // ..
    Token_ElipsisLess,                                   // ..<
    Token_Cash,                                          // $
    
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
        Big_Int integer;
        Big_Float floating;
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
Lexer_Init(String string)
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
            case '$': token.kind = Token_Cash;         break;
            
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
                    lexer->offset += 1;
                }
                
                if (c[1] == '|')
                {
                    token.kind = Token_OrOr;
                    lexer->offset += 1;
                    
                    if (c[2] == '=')
                    {
                        token.kind = Token_OrOrEquals;
                        lexer->offset += 1;
                    }
                }
            } break;
            
            case '&':
            {
                token.kind = Token_And;
                
                if (c[1] == '=')
                {
                    token.kind = Token_AndEquals;
                    lexer->offset += 1;
                }
                
                if (c[1] == '&')
                {
                    token.kind = Token_AndAnd;
                    lexer->offset += 1;
                    
                    if (c[2] == '=')
                    {
                        token.kind = Token_AndAndEquals;
                        lexer->offset += 1;
                    }
                }
            } break;
            
            case '<':
            {
                token.kind = Token_Less;
                
                if (c[1] == '=')
                {
                    token.kind = Token_LessEquals;
                    lexer->offset += 1;
                }
                
                if (c[1] == '<')
                {
                    token.kind = Token_LeftShift;
                    lexer->offset += 1;
                    
                    if (c[2] == '=')
                    {
                        token.kind = Token_LeftShiftEquals;
                        lexer->offset += 1;
                    }
                    
                    if (c[2] == '<')
                    {
                        token.kind = Token_SplatLeftShift;
                        lexer->offset += 1;
                        
                        if (c[3] == '=')
                        {
                            token.kind = Token_SplatLeftShiftEquals;
                            lexer->offset += 1;
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
                    lexer->offset += 1;
                }
                
                if (c[1] == '>')
                {
                    token.kind = Token_RightShift;
                    lexer->offset += 1;
                    
                    if (c[2] == '=')
                    {
                        token.kind = Token_RightShiftEquals;
                        lexer->offset += 1;
                    }
                    
                    if (c[2] == '>')
                    {
                        token.kind = Token_ArithmeticRightShift;
                        lexer->offset += 1;
                        
                        if (c[3] == '=')
                        {
                            token.kind = Token_ArithmeticRightShiftEquals;
                            lexer->offset += 1;
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
                    lexer->offset += 1;
                }
                
                if (c[1] == '[')
                {
                    token.kind = Token_OpenPeriodBracket;
                    lexer->offset += 1;
                }
                
                if (c[1] == '(')
                {
                    token.kind = Token_OpenPeriodParen;
                    lexer->offset += 1;
                }
                
                if (c[1] == '.')
                {
                    token.kind = Token_Elipsis;
                    lexer->offset += 1;
                    
                    if (c[2] == '<')
                    {
                        token.kind = Token_ElipsisLess;
                        lexer->offset += 1;
                    }
                }
            } break;
            
            case '-':
            {
                token.kind = Token_Minus;
                
                if (c[1] == '=')
                {
                    token.kind = Token_MinusEquals;
                    lexer->offset += 1;
                }
                
                if (c[1] == '>')
                {
                    token.kind = Token_Arrow;
                    lexer->offset += 1;
                }
                
                if (c[1] == '-' && c[2] == '-')
                {
                    token.kind = Token_TripleMinus;
                    lexer->offset += 2;
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
                    token.kind = Token_Identifier;
                    
                    String identifier = {
                        .data = lexer->string.data + lexer->offset - 1,
                        .size = 1,
                    };
                    
                    while (lexer->offset < lexer->string.size && IsAlphaNumericOrUnderscore(lexer->string.data[lexer->offset]))
                    {
                        identifier.size += 1;
                        lexer->offset   += 1;
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
                            lexer->offset += 1;
                            
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
                                        
                                        lexer->offset += 1;
                                        
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
                                                    
                                                    lexer->offset += 1;
                                                    
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
                                token.floating = BigFloat_FromF64(f);
                            }
                            else if (integer_digit_count == 16)
                            {
                                f64 f;
                                Copy(&integer, &f, sizeof(u64));
                                
                                token.kind     = Token_Float;
                                token.floating = BigFloat_FromF64(f);
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
                            
                            // HACK
                            f64 f = (f64)integer;
                            
                            f64 adj = 1;
                            for (umm i = 0; i < fraction_digit_count; ++i) adj *= 10;
                            
                            f += fraction / adj;
                            
                            adj = 1;
                            if (exponent < 0) for (umm i = 0; i < (umm)-exponent; ++i) adj /= 10;
                            else              for (umm i = 0; i < (umm) exponent; ++i) adj *= 10;
                            
                            f *= adj;
                            
                            token.floating = BigFloat_FromF64(f);
                            //
                        }
                        else
                        {
                            token.kind    = Token_Int;
                            token.integer = BigInt_FromU64(integer);
                        }
                    }
                }
                
                else if (c[0] == '"' || c[0] == '\'')
                {
                    umm start = lexer->offset;
                    
                    while (lexer->offset < lexer->string.size && lexer->string.data[lexer->offset] != c[0])
                    {
                        if (lexer->string.data[lexer->offset] == '\\') lexer->offset += 1;
                        lexer->offset += 1;
                    }
                    
                    if (lexer->offset == lexer->string.size)
                    {
                        //// ERROR: Unterminated string/char literal
                    }
                    else
                    {
                        bool encountered_errors = false;
                        
                        lexer->offset += 1;
                        
                        String raw_string = {
                            .data = lexer->string.data + start,
                            .size = lexer->offset - start,
                        };
                        
                        Arena_Marker marker = Arena_BeginTemp(MM.string_arena);
                        Interned_String_Entry* entry = Arena_PushSize(MM.string_arena,
                                                                      sizeof(Interned_String_Entry) + raw_string.size,
                                                                      ALIGNOF(Interned_String_Entry));
                        
                        String string = {
                            .data = (u8*)(entry + 1),
                            .size = 0
                        };
                        
                        for (umm i = 0; !encountered_errors;)
                        {
                            for (; i < raw_string.size && raw_string.data[i] != '\\'; ++i, ++string.size)
                            {
                                string.data[string.size] = raw_string.data[i];
                            }
                            
                            if (i < raw_string.size)
                            {
                                ++i;
                                ASSERT(i < raw_string.size);
                                
                                u8 esc = raw_string.data[i];
                                
                                if (esc == 'U')
                                {
                                    ++i;
                                    
                                    if (i + 5 >= raw_string.size)
                                    {
                                        //// ERROR: Missing digits in codepoint escape sequence
                                        encountered_errors = true;
                                    }
                                    else
                                    {
                                        u32 codepoint = 0;
                                        
                                        for (umm j = i; j < i + 2 && !encountered_errors; ++j)
                                        {
                                            u8 d = raw_string.data[j];
                                            u8 n = 0;
                                            
                                            if      (IsNumeric(d))          n = d - '0';
                                            else if (d >= 'a' && d <= 'z') n = (d - 'a') + 10;
                                            else if (d >= 'A' && d <= 'Z') n = (d - 'a') + 10;
                                            else
                                            {
                                                //// ERROR: Invalid digit in byte value escape sequence
                                                encountered_errors = true;
                                            }
                                            
                                            codepoint = codepoint << 4 | n;
                                        }
                                        
                                        if (!encountered_errors)
                                        {
                                            if (codepoint <= 0x7F)
                                            {
                                                string.data[string.size++] = (u8)codepoint;
                                            }
                                            else if (codepoint <= 0x7FF)
                                            {
                                                string.data[string.size++] = (u8)(0xC0 | (codepoint & 0x7C0) >> 6);
                                                string.data[string.size++] = (u8)(0x80 | (codepoint & 0x03F) >> 0);
                                            }
                                            else if (codepoint <= 0xFFFF)
                                            {
                                                string.data[string.size++] = (u8)(0xE0 | (codepoint & 0xF000) >> 12);
                                                string.data[string.size++] = (u8)(0x80 | (codepoint & 0x0FC0) >> 6);
                                                string.data[string.size++] = (u8)(0x80 | (codepoint & 0x003F) >> 0);
                                            }
                                            else if (codepoint <= 0x10FFFF)
                                            {
                                                string.data[string.size++] = (u8)(0xF0 | (codepoint & 0x1C0000) >> 18);
                                                string.data[string.size++] = (u8)(0x80 | (codepoint & 0x03F000) >> 12);
                                                string.data[string.size++] = (u8)(0x80 | (codepoint & 0x000FC0) >> 6);
                                                string.data[string.size++] = (u8)(0x80 | (codepoint & 0x00003F) >> 0);
                                            }
                                            else
                                            {
                                                //// ERROR: Codepoint is out of UTF-8 range
                                                encountered_errors = true;
                                            }
                                        }
                                    }
                                }
                                else if (esc == 'x')
                                {
                                    ++i;
                                    
                                    if (i + 1 >= raw_string.size)
                                    {
                                        //// ERROR: Missing digits in byte value escape sequence
                                        encountered_errors = true;
                                    }
                                    else
                                    {
                                        u8 val = 0;
                                        for (umm j = i; j < i + 2 && !encountered_errors; ++j)
                                        {
                                            u8 d = raw_string.data[j];
                                            u8 n = 0;
                                            
                                            if      (IsNumeric(d))          n = d - '0';
                                            else if (d >= 'a' && d <= 'z') n = (d - 'a') + 10;
                                            else if (d >= 'A' && d <= 'Z') n = (d - 'a') + 10;
                                            else
                                            {
                                                //// ERROR: Invalid digit in byte value escape sequence
                                                encountered_errors = true;
                                            }
                                            
                                            val = val << 4 | d;
                                        }
                                        
                                        if (!encountered_errors)
                                        {
                                            string.data[string.size++] = val;
                                        }
                                    }
                                }
                                else
                                {
                                    switch (esc)
                                    {
                                        case 'a':  string.data[string.size++] = 0x07; break;
                                        case 'b':  string.data[string.size++] = 0x08; break;
                                        case 't':  string.data[string.size++] = 0x09; break;
                                        case 'n':  string.data[string.size++] = 0x0A; break;
                                        case 'v':  string.data[string.size++] = 0x0B; break;
                                        case 'f':  string.data[string.size++] = 0x0C; break;
                                        case 'r':  string.data[string.size++] = 0x0D; break;
                                        case 'e':  string.data[string.size++] = 0x1B; break;
                                        case '"':  string.data[string.size++] = 0x22; break;
                                        case '\'': string.data[string.size++] = 0x27; break;
                                        case '\\': string.data[string.size++] = 0x5C; break;
                                        
                                        default:
                                        {
                                            //// ERROR: Invalid character escape sequence
                                            encountered_errors = true;
                                        } break;
                                    }
                                }
                            }
                        }
                        
                        if (encountered_errors) Arena_EndTemp(MM.string_arena, marker);
                        else
                        {
                            if (c[0] == '\'')
                            {
                                if (string.size == 0)
                                {
                                    //// ERROR: Empty char lit
                                    encountered_errors = true;
                                }
                                else if (!((i8)string.data[0] > 0 && string.size == 1          ||
                                           (string.data[0] & 0xC0) == 0xC0 && string.size == 2 ||
                                           (string.data[0] & 0xE0) == 0xE0 && string.size == 3 ||
                                           (string.data[0] & 0xF0) == 0xF0 && string.size == 4))
                                {
                                    //// ERROR: Too many characters in char lit
                                    encountered_errors = true;
                                }
                                else
                                {
                                    token.kind = Token_Character;
                                    
                                    token.character = 0;
                                    Copy(string.data, &token.character, string.size);
                                }
                                Arena_EndTemp(MM.string_arena, marker);
                            }
                            else
                            {
                                token.kind = Token_String;
                                
                                u32 hash;
                                Interned_String_Entry** slot = MM_GetInternedStringSlot(string, &hash);
                                
                                if (*slot != 0)
                                {
                                    token.string = (u64)*slot;
                                    Arena_EndTemp(MM.string_arena, marker);
                                }
                                else
                                {
                                    Arena_ReifyTemp(MM.string_arena, marker);
                                    *slot = entry;
                                    
                                    *entry = (Interned_String_Entry){
                                        .next = 0,
                                        .hash = hash,
                                        .size = (u32)string.size,
                                    };
                                    
                                    Copy(string.data, entry + 1, string.size);
                                    
                                    token.string = (u64)*slot;
                                }
                            }
                        }
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