typedef struct MM_Lexer
{
    union { struct MM_String; MM_String string; };
    union { struct MM_Text_Pos; MM_Text_Pos pos; };
    MM_Token token;
} MM_Lexer;

MM_Token MM_Lexer_NextToken(MM_Lexer* lexer);

MM_Lexer
MM_Lexer_Init(MM_String string, MM_Text_Pos start_pos)
{
    MM_ASSERT(start_pos.line > 0 && start_pos.col > 0);
    
    MM_Lexer lexer = {
        .string    = string,
        .pos       = start_pos,
    };
    
    MM_Lexer_NextToken(&lexer);
    return lexer;
}

void
MM_Lexer__Advance(MM_Lexer* lexer, MM_u32 amount)
{
    // NOTE: newlines are detected before advancing, and handled after. See first while loop of NextToken
    MM_ASSERT(amount <= lexer->string.size);
    lexer->size   -= amount;
    lexer->data   += amount;
    lexer->offset += amount;
    lexer->col    += amount;
}

#define MM_Advance(amount) MM_Lexer__Advance(lexer, (amount))

void
MM_Lexer__Revert(MM_Lexer* lexer, MM_Text_Pos pos)
{
    MM_imm amount = lexer->offset - pos.offset;
    MM_ASSERT(amount >= 0);
    
    lexer->size -= (MM_u32)amount;
    lexer->data += (MM_u32)amount;
    lexer->pos   = pos;
}

MM_bool
MM_Lexer__IsAlpha(char c)
{
    return (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z');
}

MM_bool
MM_Lexer__IsDigit(char c)
{
    return (c >= '0' && c <= '9');
}

MM_Token
MM_Lexer_GetToken(MM_Lexer* lexer)
{
    return lexer->token;
}

MM_Token
MM_Lexer_NextToken(MM_Lexer* lexer)
{
    MM_Token token = { .kind = MM_Token_Invalid };
    
    MM_umm comment_depth = 0;
    while (lexer->size)
    {
        char c = lexer->data[0];
        if (c == '\n')
        {
            MM_Advance(1);
            
            lexer->line += 1;
            lexer->col   = 1;
        }
        else if (c == ' '  || c == '\t' ||
                 c == '\v' || c == '\f' ||
                 c == '\r')
        {
            MM_Advance(1);
        }
        else if (lexer->size > 1 && lexer->data[0] == '/' && lexer->data[1] == '/')
        {
            while (lexer->size && lexer->data[0] != '\n') MM_Advance(1);
        }
        else if (lexer->size > 1 && lexer->data[0] == '/' && lexer->data[1] == '*')
        {
            MM_Advance(2);
            comment_depth += 1;
        }
        else if (comment_depth > 0 && lexer->size > 1 && lexer->data[0] == '*' && lexer->data[1] == '/')
        {
            MM_Advance(2);
            comment_depth -= 1;
        }
        else
        {
            if (comment_depth > 0) continue;
            else                   break;
        }
    }
    
    MM_Text_Pos start_pos = lexer->pos;
    
    char c = 0;
    if (lexer->size)
    {
        c = lexer->data[0];
        MM_Advance(1);
    }
    
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
        
        // NOTE: Handled separately, since = does not belong to a token block
        case '=':
        {
            if (lexer->size && lexer->data[0] == '=') token.kind = MM_Token_EqualEQ, MM_Advance(1);
            else                                      token.kind = MM_Token_Equals;
        } break;
        
        case '+':
        case '~':
        case '*':
        case '/':
        case '!':
        {
            MM_Token_Kind base_kind;
            switch (c)
            {
                case '+': base_kind = MM_Token_Plus;  break;
                case '~': base_kind = MM_Token_Tilde; break;
                case '*': base_kind = MM_Token_Star;  break;
                case '/': base_kind = MM_Token_Slash; break;
                default:  base_kind = MM_Token_Bang;  break;
            }
            
            if (lexer->size && lexer->data[0] == '=') token.kind = MM_TOKEN_BINARY_TO_ASSIGNMENT(base_kind), MM_Advance(1);
            else                                      token.kind = base_kind;
        } break;
        
        case '&':
        case '|':
        {
            MM_Token_Kind base_kind = (c == '&' ? MM_Token_And : MM_Token_Or);
            
            if (lexer->size && lexer->data[0] == '=') token.kind = MM_TOKEN_BINARY_TO_ASSIGNMENT(base_kind), MM_Advance(1);
            else if (lexer->size && lexer->data[0] == c)
            {
                MM_Advance(1);
                
                base_kind = (c == '&' ? MM_Token_AndAnd : MM_Token_OrOr);
                
                if (lexer->size && lexer->data[0] == '=') token.kind = MM_TOKEN_BINARY_TO_ASSIGNMENT(base_kind), MM_Advance(1);
                else                                      token.kind = base_kind;
            }
            else token.kind = base_kind;
        } break;
        
        case '<':
        {
            MM_Token_Kind base_kind = MM_Token_Less;
            
            if      (lexer->size && lexer->data[0] == '=') token.kind = MM_Token_LessEQ, MM_Advance(1);
            else if (lexer->size && lexer->data[0] == c)
            {
                MM_Advance(1);
                
                base_kind = MM_Token_Shr;
                
                if (lexer->size && lexer->data[0] == '=') token.kind = MM_TOKEN_BINARY_TO_ASSIGNMENT(base_kind), MM_Advance(1);
                else                                      token.kind = base_kind;
            }
            else token.kind = base_kind;
        } break;
        
        case '>':
        {
            MM_Token_Kind base_kind = MM_Token_Greater;
            
            if      (lexer->size && lexer->data[0] == '=') token.kind = MM_Token_GreaterEQ, MM_Advance(1);
            else if (lexer->size && lexer->data[0] == c)
            {
                MM_Advance(1);
                
                base_kind = MM_Token_Shr;
                
                if      (lexer->size && lexer->data[0] == '=') token.kind = MM_TOKEN_BINARY_TO_ASSIGNMENT(base_kind), MM_Advance(1);
                else if (lexer->size && lexer->data[0] == c)
                {
                    MM_Advance(1);
                    
                    base_kind = MM_Token_Sar;
                    
                    if (lexer->size && lexer->data[0] == '=') token.kind = MM_TOKEN_BINARY_TO_ASSIGNMENT(base_kind), MM_Advance(1);
                    else                                      token.kind = base_kind;
                }
                else token.kind = base_kind;
            }
            else token.kind = base_kind;
        } break;
        
        case '-':
        {
            if      (lexer->size     && lexer->data[0] == '=')                      token.kind = MM_Token_MinusEQ, MM_Advance(1);
            else if (lexer->size     && lexer->data[0] == '>')                      token.kind = MM_Token_Arrow,   MM_Advance(1);
            else if (lexer->size > 1 && lexer->data[0] == c && lexer->data[1] == c) token.kind = MM_Token_TpMinus, MM_Advance(2);
            else                                                                    token.kind = MM_Token_Minus;
        } break;
        
        case '.':
        {
            if      (lexer->size && lexer->data[0] == '{') token.kind = MM_Token_PeriodBrace,   MM_Advance(1);
            else if (lexer->size && lexer->data[0] == '[') token.kind = MM_Token_PeriodBracket, MM_Advance(1);
            else                                           token.kind = MM_Token_Period;
        } break;
        
        default:
        {
            if (c == '_' || MM_Lexer__IsAlpha(c))
            {
                MM_String identifier = { .data = lexer->data - 1, .size = 1 };
                
                while (lexer->size && (lexer->data[0] == '_'             ||
                                       MM_Lexer__IsAlpha(lexer->data[0]) ||
                                       MM_Lexer__IsDigit(lexer->data[0])))
                {
                    MM_Advance(1);
                    identifier.size += 1;
                }
                
                { // NOTE: labeled break done with goto identifier_end
                    for (MM_umm i = 0; i < MM_ARRAY_SIZE(MM_Token_KeywordList); ++i)
                    {
                        if (MM_String_Match(identifier, MM_Token_KeywordList[i]))
                        {
                            token.kind = (MM_Token_Kind)(MM_Token__FirstKeyword + i);
                            goto identifier_end;
                        }
                    }
                    
                    for (MM_umm i = 0; i < MM_ARRAY_SIZE(MM_Token_BuiltinList); ++i)
                    {
                        if (MM_String_Match(identifier, MM_Token_BuiltinList[i]))
                        {
                            token.kind = (MM_Token_Kind)(MM_Token__FirstBuiltin + i);
                            goto identifier_end;
                        }
                    }
                    
                    token.kind = MM_Token_Identifier;
                    
                    identifier_end:;
                    token.identifier = identifier;
                }
            }
            else if (MM_Lexer__IsDigit(c))
            {
                MM_u8 base       = 10;
                MM_bool is_float = MM_false;
                
                if (c == '0' && lexer->size)
                {
                    if      (lexer->data[0] == 'b') base = 2;
                    else if (lexer->data[0] == 'x') base = 16;
                    else if (lexer->data[0] == 'h') base = 16, is_float = MM_true;
                }
                
                MM_i128 integer    = {0};
                MM_umm digit_count = 0;
                if (base != 10) MM_Advance(1);
                else
                {
                    digit_count += 1;
                    integer      = MM_i128_FromU64(c - '0');
                }
                
                while (lexer->size)
                {
                    char d      = lexer->data[0];
                    MM_u8 digit = 0;
                    
                    if      (MM_Lexer__IsDigit(d)) digit = d - '0';
                    else if (d >= 'A' && d <= 'F') digit = (d - 'A') + 10;
                    else if (d >= 'a' && d <= 'f') digit = (d - 'a') + 10;
                    else if (d == '_')
                    {
                        MM_Advance(1);
                        continue;
                    }
                    else if (d == '.')
                    {
                        MM_Advance(1);
                        MM_NOT_IMPLEMENTED;
                    }
                    else break;
                    
                    if (digit >= base)
                    {
                        //// ERROR: Digit is too large for base
                        token.kind = MM_Token_Invalid;
                        break;
                    }
                    else
                    {
                        digit_count += 1;
                        if (!MM_i128_AppendDigit(&integer, base, digit))
                        {
                            //// ERROR: Numeric literal contains too many digits
                            token.kind = MM_Token_Invalid;
                            break;
                        }
                        else MM_Advance(1);
                    }
                }
                
                if (digit_count == 0)
                {
                    //// ERROR: Numeric literals should contain at least 1 digit
                    token.kind = MM_Token_Invalid;
                }
                else if (is_float)
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
                            //// ERROR: Invalid hex float digit count. Must be either 4, 8 or 16, corresponding to f16, f32 and f64 respectively
                            token.kind = MM_Token_Invalid;
                        }
                    }
                    else
                    {
                        MM_NOT_IMPLEMENTED;
                    }
                }
                else
                {
                    token.kind = MM_Token_Int;
                    token.i128 = integer;
                }
            }
            else if (c == '"')
            {
                MM_NOT_IMPLEMENTED;
            }
            else
            {
                //// ERROR: Unknown symbol
                token.kind = MM_Token_Invalid;
            }
        } break;
    }
    
    token.text_pos = start_pos;
    token.size     = lexer->offset - token.offset;
    
    if (token.kind == MM_Token_Invalid) MM_Lexer__Revert(lexer, start_pos);
    
    lexer->token = token;
    return token;
}

#undef MM_Advance