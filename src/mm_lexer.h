enum TOKEN_KIND
{
    Token_Invalid = 0,
    
    Token_Not,                                  // !
    Token_Complement,                           // ~
    Token_Period,                               // .
    Token_TripleMinus,                          // ---
    Token_Arrow,                                // ->
    Token_OpenParen,                            // (
    Token_CloseParen,                           // )
    Token_OpenBracket,                          // [
    Token_CloseBracket,                         // ]
    Token_OpenBrace,                            // {
    Token_CloseBrace,                           // }
    Token_Comma,                                // ,
    Token_Colon,                                // :
    Token_Semicolon,                            // ;
    Token_Cash,                                 // $
    Token_At,                                   // @
    Token_Underscore,                           // _
    Token_QuestionMark,                         // ?
    Token_Pound,                                // #
    
    Token_FirstAssignment,
    Token_Equals = Token_FirstAssignment,       // =
    Token_StarEquals,                           // *=
    Token_SlashEquals,                          // /=
    Token_RemEquals,                            // %=
    Token_AndEquals,                            // &=
    Token_ArithmeticRightShiftEquals,           // >>>=
    Token_RightShiftEquals,                     // >>=
    Token_LeftShiftEquals,                      // <<=
    Token_PlusEquals,                           // +=
    Token_MinusEquals,                          // -=
    Token_OrEquals,                             // |=
    Token_HatEquals,                            // ^=
    Token_AndAndEquals,                         // &&=
    Token_OrOrEquals,                           // ||=
    Token_LastAssignment = Token_LeftShiftEquals,
    
    Token_FirstRangeLevel = 80,
    Token_Elipsis = Token_FirstRangeLevel,      // ..
    Token_ElipsisLess,                          // ..<
    Token_LastRangeLevel = Token_ElipsisLess,
    
    Token_FirstMulLevel = 100,
    Token_Star = Token_FirstMulLevel,           // *
    Token_Slash,                                // /
    Token_Rem,                                  // %
    Token_And,                                  // &
    Token_ArithmeticRightShift,                 // >>>
    Token_RightShift,                           // >>
    Token_LeftShift,                            // <<
    Token_LastMulLevel = Token_LeftShift,
    
    Token_FirstAddLevel = 120,
    Token_Plus = Token_FirstAddLevel,           // +
    Token_Minus,                                // -
    Token_Or,                                   // |
    Token_Hat,                                  // ^
    Token_LastAddLevel = Token_Hat,
    
    Token_FirstComparative = 140,
    Token_EqualEquals = Token_FirstComparative, // ==
    Token_NotEquals,                            // !=
    Token_Less,                                 // <
    Token_Greater,                              // >
    Token_LessEquals,                           // <=
    Token_GreaterEquals,                        // >=
    Token_LastComparative = Token_GreaterEquals,
    
    Token_AndAnd = 160,                         // &&
    
    Token_OrOr = 180,                           // ||
    
    Token_Identifier,
    Token_String,
    Token_Character,
    Token_Number,
    
    Token_EndOfStream,
};

typedef struct Token
{
    u8 kind;
    u32 start_of_line;
    u32 line;
    u32 column;
    u32 size;
    
    union
    {
        String raw_string;
        Interned_String identifier;
        Number number;
    };
    
} Token;

typedef struct Lexer
{
    u8* file_contents;
    u8* cursor;
    u8* start_of_line;
    u32 line;
} Lexer;

internal inline Lexer
Lexer_Init(u8* file_contents)
{
    return (Lexer){
        .file_contents = file_contents,
        .cursor        = file_contents,
        .start_of_line = file_contents,
        .line          = 1,
    };
}

internal Token
Lexer_Advance(Lexer* lexer)
{
    Token token = { .kind = Token_Invalid };
    
    while (*lexer->cursor != 0)
    {
        if (IsWhitespace(*lexer->cursor))
        {
            if (*lexer->cursor == '\n')
            {
                lexer->line += 1;
                lexer->start_of_line = lexer->cursor + 1;
            }
            
            lexer->cursor += 1;
        }
        
        else if (lexer->cursor[0] == '/' && lexer->cursor[1] == '/')
        {
            while (*lexer->cursor != 0 && *lexer->cursor != '\n')
            {
                lexer->cursor += 1;
            }
            
            if (*lexer->cursor != 0)
            {
                lexer->cursor += 1;
                lexer->line   += 1;
                lexer->start_of_line = lexer->cursor;
            }
        }
        
        else if (lexer->cursor[0] == '/' && lexer->cursor[1] == '*')
        {
            lexer->cursor += 2;
            
            umm nesting_level = 0;
            while (*lexer->cursor != 0)
            {
                if (lexer->cursor[0] == '/' && lexer->cursor[1] == '*')
                {
                    lexer->cursor += 2;
                    nesting_level += 1;
                }
                
                else if (lexer->cursor[0] == '*' && lexer->cursor[1] == '/')
                {
                    lexer->cursor += 2;
                    
                    if (nesting_level == 0) break;
                    else
                    {
                        nesting_level -= 1;
                        continue;
                    }
                }
                
                else lexer->cursor += 1;
            }
        }
        
        else break;
    }
    
    u8* start_of_token = lexer->cursor;
    
    u8 c = *lexer->cursor;
    lexer->cursor += 1;
    
    switch (c)
    {
        case 0: token.kind = Token_EndOfStream; break;
        
        case '~': token.kind = Token_Complement;   break;
        case '(': token.kind = Token_OpenParen;    break;
        case ')': token.kind = Token_CloseParen;   break;
        case '[': token.kind = Token_OpenBracket;  break;
        case ']': token.kind = Token_CloseBracket; break;
        case '{': token.kind = Token_OpenBrace;    break;
        case '}': token.kind = Token_CloseBrace;   break;
        case ',': token.kind = Token_Comma;        break;
        case ':': token.kind = Token_Colon;        break;
        case ';': token.kind = Token_Semicolon;    break;
        case '$': token.kind = Token_Cash;         break;
        case '@': token.kind = Token_At;           break;
        case '?': token.kind = Token_QuestionMark; break;
        
        case '+':
        {
            token.kind = Token_Plus;
            
            if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_PlusEquals;
            }
        } break;
        
        case '-':
        {
            token.kind = Token_Minus;
            
            if (*lexer->cursor == '-')
            {
                lexer->cursor += 1;
                token.kind = Token_TripleMinus;
            }
            
            else if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_MinusEquals;
            }
            
            else if (*lexer->cursor == '>')
            {
                lexer->cursor += 1;
                token.kind = Token_Arrow;
            }
        } break;
        
        case '*':
        {
            token.kind = Token_Star;
            
            if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_StarEquals;
            }
        } break;
        
        case '/':
        {
            token.kind = Token_Slash;
            
            if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_SlashEquals;
            }
        } break;
        
        case '^':
        {
            token.kind = Token_Hat;
            
            if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_HatEquals;
            }
        } break;
        
        case '!':
        {
            token.kind = Token_Not;
            
            if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_NotEquals;
            }
        } break;
        
        case '=':
        {
            token.kind = Token_Equals;
            
            if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_EqualEquals;
            }
        } break;
        
        case '%':
        {
            token.kind = Token_Rem;
            
            if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_RemEquals;
            }
        } break;
        
        case '&':
        {
            token.kind = Token_And;
            
            if (*lexer->cursor == '&')
            {
                lexer->cursor += 1;
                
                token.kind = Token_AndAnd;
                
                if (*lexer->cursor == '=')
                {
                    lexer->cursor += 1;
                    token.kind = Token_AndAndEquals;
                }
            }
            
            else if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_AndEquals;
            }
        } break;
        
        case '|':
        {
            token.kind = Token_Or;
            
            if (*lexer->cursor == '|')
            {
                lexer->cursor += 1;
                
                token.kind = Token_OrOr;
                
                if (*lexer->cursor == '=')
                {
                    lexer->cursor += 1;
                    token.kind = Token_OrOrEquals;
                }
            }
            
            else if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_OrOrEquals;
            }
        } break;
        
        case '<':
        {
            token.kind = Token_Less;
            
            if (*lexer->cursor == '<')
            {
                lexer->cursor += 1;
                
                token.kind = Token_LeftShift;
                
                if (*lexer->cursor == '=')
                {
                    lexer->cursor += 1;
                    token.kind = Token_LeftShiftEquals;
                }
            }
            
            else if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_LessEquals;
            }
        } break;
        
        case '>':
        {
            token.kind = Token_Greater;
            
            if (*lexer->cursor == '>')
            {
                lexer->cursor += 1;
                
                token.kind = Token_RightShift;
                
                if (*lexer->cursor == '>')
                {
                    lexer->cursor += 1;
                    
                    token.kind = Token_ArithmeticRightShift;
                    
                    if (*lexer->cursor == '=')
                    {
                        lexer->cursor += 1;
                        token.kind = Token_ArithmeticRightShiftEquals;
                    }
                }
                
                else if (*lexer->cursor == '=')
                {
                    lexer->cursor += 1;
                    token.kind = Token_RightShiftEquals;
                }
            }
            
            else if (*lexer->cursor == '=')
            {
                lexer->cursor += 1;
                token.kind = Token_GreaterEquals;
            }
        } break;
        
        case '.':
        {
            token.kind = Token_Period;
            
            if (*lexer->cursor == '.')
            {
                lexer->cursor += 1;
                token.kind = Token_Elipsis;
            }
            
            else if (*lexer->cursor == '<')
            {
                lexer->cursor += 1;
                token.kind = Token_ElipsisLess;
            }
        } break;
        
        default:
        {
            if (c == '#')
            {
                u8* lookahead = lexer->cursor;
                
                String ident = {
                    .data = lookahead - 1,
                    .size = 1
                };
                
                while (*lookahead == '_' || IsAlpha(*lookahead) || IsDigit(*lookahead))
                {
                    lookahead += 1;
                }
                
                ident.size = lookahead - start_of_token;
                
                if (!String_Compare(ident, STRING("here_string")))
                {
                    token.kind = Token_Pound;
                }
                
                else
                {
                    lexer->cursor = lookahead;
                    
                    if (*lexer->cursor == '\n' || lexer->cursor[0] == '\r' && lexer->cursor[1] == '\n')
                    {
                        lexer->cursor += 1 + (*lexer->cursor == '\n');
                        lexer->line   += 1;
                        lexer->start_of_line = lexer->cursor;
                    }
                    
                    token.kind = Token_String;
                    token.raw_string.data = lexer->cursor;
                    token.raw_string.size = 0;
                    
                    while (true)
                    {
                        if (*lexer->cursor == 0)
                        {
                            //// ERROR: unterminated here string
                            token.kind = Token_Invalid;
                        }
                        
                        else if (*lexer->cursor != '\n') lexer->cursor += 1;
                        else
                        {
                            lexer->cursor += 1;
                            lexer->line   += 1;
                            lexer->start_of_line = lexer->cursor;
                            
                            ident = (String){
                                .data = lexer->cursor - 1,
                                .size = 1
                            };
                            
                            while (*lexer->cursor == '_' || IsAlpha(*lexer->cursor) || IsDigit(*lexer->cursor))
                            {
                                lexer->cursor += 1;
                            }
                            
                            ident.size = lexer->cursor - start_of_token;
                            
                            if (String_Compare(ident, STRING("HERE_STRING_END")))
                            {
                                token.raw_string.size = (ident.data - 1) - token.raw_string.data;
                                break;
                            }
                        }
                    }
                }
            }
            
            else if (c == '_' || IsAlpha(c))
            {
                token.kind = Token_Identifier;
                
                String ident = {
                    .data = lexer->cursor - 1,
                    .size = 1
                };
                
                while (*lexer->cursor == '_' || IsAlpha(*lexer->cursor) || IsDigit(*lexer->cursor))
                {
                    lexer->cursor += 1;
                }
                
                ident.size = lexer->cursor - start_of_token;
                
                token.identifier = String_Intern(ident);
            }
            
            else if (c >= '0' && c <= '9')
            {
                bool is_hex    = false;
                bool is_binary = false;
                bool is_float  = false;
                
                if (c == '0')
                {
                    if      (*lexer->cursor == 'x') is_hex = true;
                    else if (*lexer->cursor == 'h') is_hex = true, is_float = true;
                    else if (*lexer->cursor == 'b') is_binary = true;
                    
                    if (is_hex || is_binary) lexer->cursor += 1;
                }
                
                umm digit_count = !(is_hex || is_binary);
                umm base = (is_hex ? 16 : (is_binary ? 2 : 10));
                
                umm integer           = (c - '0');
                bool integer_overflow = false;
                
                while (true)
                {
                    u8 digit = 0;
                    
                    if (!is_binary && IsDigit(*lexer->cursor))
                    {
                        digit = *lexer->cursor - '0';
                    }
                    
                    else if (is_binary && *lexer->cursor - '0' <= 1)
                    {
                        digit = *lexer->cursor - '0';
                    }
                    
                    else if (is_hex && ToUpperCase(*lexer->cursor) >= 'A' && ToUpperCase(*lexer->cursor) <= 'F')
                    {
                        digit = (ToUpperCase(*lexer->cursor) - 'A') + 10;
                    }
                    
                    else if (*lexer->cursor == '_')
                    {
                        lexer->cursor += 1;
                        continue;
                    }
                    
                    else if (*lexer->cursor == '.' && !is_float)
                    {
                        is_float = true;
                        lexer->cursor += 1;
                        break;
                    }
                    
                    else break;
                    
                    lexer->cursor += 1;
                    digit_count    += 1;
                    
                    umm new_integer = integer * 10 + digit * base;
                    
                    integer_overflow = integer_overflow || (new_integer < integer);
                    integer          = new_integer;
                }
                
                if (integer_overflow)
                {
                    token.kind = Token_Invalid;
                    //// ERROR: NumericLiteralTooLarge;
                }
                
                else
                {
                    if (is_float)
                    {
                        if (is_hex)
                        {
                            if (digit_count == 8)
                            {
                                f32 f;
                                Copy(&integer, &f, sizeof(u32));
                                
                                token.kind = Token_Number;
                                token.number.is_float = true;
                                token.number.floating = f;
                            }
                            
                            else if (digit_count == 16)
                            {
                                Copy(&integer, &token.number.floating, sizeof(u32));
                                
                                token.kind = Token_Number;
                                token.number.is_float    = true;
                            }
                            
                            else
                            {
                                token.kind = Token_Invalid;
                                //// ERROR: invalid digit count for hex float
                            }
                        }
                        
                        else
                        {
                            f64 fraction            = 0;
                            umm fraction_adjustment = 1;
                            
                            while (IsDigit(*lexer->cursor))
                            {
                                fraction            *= 10;
                                fraction_adjustment *= 10;
                                fraction += *lexer->cursor - '0';
                            }
                            
                            fraction /= fraction_adjustment;
                            
                            f64 adjustment = 1;
                            if (ToUpperCase(*lexer->cursor) == 'E')
                            {
                                lexer->cursor += 1;
                                
                                bool is_negative = false;
                                
                                if (*lexer->cursor == '+') lexer->cursor += 1;
                                else if (*lexer->cursor == '-')
                                {
                                    lexer->cursor += 1;
                                    is_negative    = true;
                                }
                                
                                umm exponent = 0;
                                while (IsDigit(*lexer->cursor))
                                {
                                    exponent *= 10;
                                    exponent += *lexer->cursor - '0';
                                }
                                
                                for (umm i = 0; i < exponent; ++i)
                                {
                                    adjustment *= 10;
                                }
                                
                                if (is_negative)
                                {
                                    adjustment = 1 / adjustment;
                                }
                            }
                            
                            token.kind = Token_Number;
                            token.number.is_float = true;
                            token.number.floating = (integer + fraction) * adjustment;
                        }
                    }
                    
                    else
                    {
                        token.kind = Token_Number;
                        token.number.is_float = false;
                        token.number.integer  = integer;
                    }
                }
            }
            
            else if (c == '\'' || c == '"')
            {
                while (*lexer->cursor != 0 && *lexer->cursor != c && *lexer->cursor != '\n')
                {
                    if (lexer->cursor[0] == '\\' && lexer->cursor[1] != 0) lexer->cursor += 2;
                    else                                                   lexer->cursor += 1;
                }
                
                if (*lexer->cursor != c)
                {
                    token.kind = Token_Invalid;
                    //// ERROR: Unterminated * literal
                }
                
                else
                {
                    token.kind = (c == '"' ? Token_String : Token_Character);
                    
                    token.raw_string = (String){
                        .data = start_of_token + 1,
                        .size = lexer->cursor - (start_of_token + 1)
                    };
                    
                    lexer->cursor += 1;
                }
            }
        } break;
    }
    
    token.start_of_line = (u32)(lexer->start_of_line - lexer->file_contents);
    token.line          = lexer->line;
    token.column        = (u32)(start_of_token - lexer->start_of_line);
    token.size          = (u32)(lexer->cursor - start_of_token);
    
    if (token.kind == Token_Invalid || token.kind == Token_EndOfStream)
    {
        lexer->cursor = start_of_token;
    }
    
    return token;
}