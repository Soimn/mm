typedef struct MM_Lexer
{
    MM_String string;
    MM_Text_Pos pos;
    MM_Token token;
} MM_Lexer;

MM_Token MM_Lexer_NextToken(MM_Lexer* lexer);

MM_Lexer
MM_Lexer_Init(MM_String string, MM_Text_Pos init_pos)
{
    MM_ASSERT(string.size + init_pos.offset <= MM_U32_MAX);
    // NOTE: sanity checks, non exhaustive
    MM_ASSERT(init_pos.line <= init_pos.offset);
    MM_ASSERT(init_pos.col  <= init_pos.offset);
    MM_ASSERT(init_pos.line > 0 && init_pos.col > 0);
    
    MM_Lexer lexer = {
        .string = string,
        .pos    = init_pos,
    };
    
    lexer.token = MM_Lexer_NextToken(&lexer);
    
    return lexer;
}

MM_Token
MM_Lexer_GetToken(MM_Lexer* lexer)
{
    return lexer->token;
}

void
MM_Lexer__Refresh(MM_Lexer* lexer, MM_u8* cursor, MM_u8 (*peek)[3])
{
    MM_umm peek_len = MM_MIN(lexer->string.size, MM_ARRAY_SIZE(*peek));
    MM_Memcopy(*peek, lexer->string.data, peek_len);
    MM_Memzero(*peek + peek_len, MM_ARRAY_SIZE(*peek) - peek_len);
    
    *cursor = (*peek)[0];
}

void
MM_Lexer__Advance(MM_Lexer* lexer, MM_u32* offset_to_line, MM_u8* cursor, MM_u8 (*peek)[3], MM_u32 advancement)
{
    advancement = MM_MIN(advancement, lexer->string.size);
    
    for (MM_umm i = 0; i < advancement; ++i)
    {
        if (lexer->string.data[i] == '\n')
        {
            lexer->pos.line += 1;
            lexer->pos.col   = 0; // NOTE: Incremented to 1 later in the loop
            *offset_to_line  = 0;
        }
        
        lexer->pos.col += 1;
    }
    
    lexer->string.data += advancement;
    lexer->string.size -= advancement;
    
    MM_Lexer__Refresh(lexer, cursor, peek);
}

MM_Token
MM_Lexer_NextToken(MM_Lexer* lexer)
{
    if (ERROR) return lexer->token;
    else
    {
        MM_Token token = { .kind = MM_Token_Invalid };
        
        MM_u8 peek[3];
        MM_u8 cursor;
        MM_u32 offset_to_line = lexer->pos.offset - lexer->pos.col;
        MM_Lexer__Refresh(lexer, &cursor, &peek);
        
#define MM_Advance(N) MM_Lexer__Advance(lexer, &offset_to_line, &cursor, &peek, (N))
        
        MM_u32 comment_depth  = 0;
        while (cursor != 0)
        {
            if (cursor == ' '  || cursor == '\t' ||
                cursor == '\v' || cursor == '\f' ||
                cursor == '\r' || cursor == '\n')
            {
                MM_Advance(1);
            }
            else if (cursor == '/' && peek[1] == '/')
            {
                while (cursor != 0 && cursor != '\n') MM_Advance(1);
            }
            else if (cursor == '/' && peek[1] == '*')
            {
                MM_Advance(2);
                comment_depth += 1;
            }
            else if (comment_depth > 0 && cursor == '*' && peek[1] == '/')
            {
                MM_Advance(2);
                comment_depth -= 1;
            }
            else
            {
                if (comment_depth > 0) MM_Advance(1);
                else                   break;
            }
        }
        
        // NOTE: End of range is set before returning
        token.text.start = lexer->pos;
        
        // NOTE: Assume 1 character long token, advancement past end is stopped by MM_Advance
        MM_u8 c = cursor;
        MM_Advance(1);
        
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
            
#define MM_TOKEN_SINGLE_OR_EQ(ch, single, eq)                 \
case (ch):                                                \
{                                                         \
if (peek[1] == '=') token.kind = (eq), MM_Advance(1); \
else                token.kind = (single);            \
} break
            
            MM_TOKEN_SINGLE_OR_EQ('+', MM_Token_Plus,  MM_Token_PlusEQ);
            MM_TOKEN_SINGLE_OR_EQ('~', MM_Token_Tilde, MM_Token_TildeEQ);
            MM_TOKEN_SINGLE_OR_EQ('*', MM_Token_Star,  MM_Token_StarEQ);
            MM_TOKEN_SINGLE_OR_EQ('/', MM_Token_Slash, MM_Token_SlashEQ);
            MM_TOKEN_SINGLE_OR_EQ('!', MM_Token_Bang,  MM_Token_BangEQ);
            
#undef MM_TOKEN_SINGLE_OR_EQ
            
#define MM_TOKEN_SINGLE_DOUBLE_OR_EQ(ch, single, singleeq, dub, dubeq) \
case (ch):                                                         \
{                                                                  \
if (peek[1] == '=') token.kind = (singleeq), MM_Advance(1);    \
else if (peek[1] == (ch))                                      \
{                                                              \
if (peek[2] == '=') token.kind = (dubeq), MM_Advance(2);   \
else                token.kind = (dub),   MM_Advance(1);   \
}                                                              \
else token.kind = (single);                                    \
} break
            
            MM_TOKEN_SINGLE_DOUBLE_OR_EQ('&', MM_Token_And, MM_Token_AndEQ, MM_Token_AndAnd, MM_Token_AndAndEQ);
            MM_TOKEN_SINGLE_DOUBLE_OR_EQ('|', MM_Token_Or, MM_Token_OrEQ, MM_Token_OrOr, MM_Token_OrOrEQ);
            
#undef MM_TOKEN_SINGLE_DOUBLE_OR_EQ
            
            case '<':
            {
                if      (peek[1] == '=') token.kind = MM_Token_LessEQ, MM_Advance(1);
                else if (peek[1] == '<')
                {
                    if (peek[2] == '=') token.kind = MM_Token_ShlEQ, MM_Advance(2);
                    else                token.kind = MM_Token_Shl,   MM_Advance(1);
                }
                else token.kind = MM_Token_Less;
            } break;
            
            case '>':
            {
                if      (peek[1] == '=') token.kind = MM_Token_GreaterEQ, MM_Advance(1);
                else if (peek[1] == '>')
                {
                    if      (peek[2] == '=') token.kind = MM_Token_ShrEQ, MM_Advance(2);
                    else if (peek[2] == '>')
                    {
                        if (peek[3] == '=') token.kind = MM_Token_SarEQ, MM_Advance(3);
                        else                token.kind = MM_Token_Sar,   MM_Advance(2);
                    }
                    else token.kind = MM_Token_Shr, MM_Advance(1);
                }
                else token.kind = MM_Token_Greater;
            } break;
            
            case '-':
            {
                if      (peek[1] == '>')                   token.kind = MM_Token_Arrow,   MM_Advance(1);
                else if (peek[1] == '-' && peek[2] == '-') token.kind = MM_Token_TpMinus, MM_Advance(2);
                else                                       token.kind = MM_Token_Minus;
            } break;
            
            case '.':
            {
                if      (peek[1] == '(') token.kind = MM_Token_PeriodParen,   MM_Advance(1);
                else if (peek[1] == '[') token.kind = MM_Token_PeriodBracket, MM_Advance(1);
                else if (peek[1] == '{') token.kind = MM_Token_PeriodBrace,   MM_Advance(1);
                else                     token.kind = MM_Token_Period;
            } break;
            
            default:
            {
                if (c == '_' || c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z')
                {
                    MM_String identifier = {
                        .data = lexer->string.data - 1, // NOTE: cursor is the first element of string.data
                        .size = 1
                    };
                    
                    while (cursor == '_'                  ||
                           cursor >= 'A' && cursor <= 'Z' ||
                           cursor >= 'a' && cursor <= 'z' ||
                           cursor >= '0' && cursor <= '9')
                    {
                        MM_Advance(1);
                        identifier.size += 1;
                    }
                    
                    if (MM_String_Match(identifier, MM_STRING("_"))) token.kind = MM_Token_Blank;
                    else
                    {
                        
                        // NOTE: This is very ugly because of MSVC bullshit
                        // TODO: replace with hash table or something more clever
                        
#define MM_X(e, str) [(e) - MM_Token__FirstKeyword] = (MM_u8*)(str),
                        static MM_u8* keyword_cstrings[MM_Token__LastKeyword - MM_Token__FirstKeyword + 1] = {
                            MM_TOKEN_KEYWORD_LIST
                        };
#undef MM_X
                        
#define MM_X(e, str) [(e) - MM_Token__FirstKeyword] = sizeof(str) - 1,
                        static MM_u32 keyword_lengths[MM_Token__LastKeyword - MM_Token__FirstKeyword + 1] = {
                            MM_TOKEN_KEYWORD_LIST
                        };
#undef MM_X
                        
                        token.kind = MM_Token_Identifier;
                        for (MM_umm i = 0; i < MM_ARRAY_SIZE(keyword_cstrings); ++i)
                        {
                            MM_String keyword = { .data = keyword_cstrings[i], .size = keyword_lengths[i] };
                            if (MM_String_Match(identifier, keyword))
                            {
                                token.kind = (MM_Token_Kind)(MM_Token__FirstKeyword + i);
                                break;
                            }
                        }
                        //
                    }
                }
                else if (c >= '0' && c <= '9')
                {
                    MM_bool is_float = MM_false;
                    MM_bool has_exp  = MM_false;
                    MM_bool exp_neg  = MM_false;
                    MM_umm base      = 10;
                    
                    if (c == '0')
                    {
                        if      (cursor == 'b') base = 2;
                        else if (cursor == 'o') base = 8;
                        else if (cursor == 'x') base = 16;
                        else if (cursor == 'h') base = 16, is_float = MM_true;
                        else if (cursor == 'y') base = 32;
                    }
                    
                    // NOTE: digit count is only useful for hexfloats and floats. The type of a hexfloat is determined by its
                    //       size. Digit count for regular floats is only used to report missing fractional or exponent parts
                    MM_umm digit_count;
                    MM_umm init;
                    if (base != 10) init = 0,       digit_count = 0, MM_Advance(1);
                    else            init = c & 0xF, digit_count = 1;
                    
                    MM_Soft_Int integral   = MM_SoftInt_FromU64(init);
                    MM_Soft_Int fractional = MM_SoftInt_FromU64(0);
                    MM_Soft_Int exponent   = MM_SoftInt_FromU64(0);
                    
                    MM_Soft_Int* integer = &integral;
                    
                    for (;;)
                    {
                        MM_umm digit;
                        
                        if      (cursor >= '0' && cursor <= '9') digit = cursor & 0xF;
                        else if (cursor >= 'A' && cursor <= 'F') digit = (cursor & 0x1F) + 9;
                        else if (cursor >= 'a' && cursor <= 'f') digit = (cursor & 0x1F) + 9;
                        else if (cursor == '_')
                        {
                            MM_Advance(1);
                            continue;
                        }
                        else if (!is_float && cursor == '.')
                        {
                            is_float = MM_true;
                            integer  = &fractional;
                            
                            // NOTE: reset to catch missing digits in fractional part
                            digit_count = 0;
                            
                            MM_Advance(1);
                            continue;
                        }
                        else if (is_float && cursor == 'e')
                        {
                            MM_Advance(1);
                            
                            exp_neg = (cursor == '-');
                            if (cursor == '-' || cursor == '+') MM_Advance(1);
                            
                            integer = &exponent;
                            
                            // NOTE: reset to catch missing digits in exponent part
                            digit_count = 0;
                            
                            continue;
                        }
                        else break;
                        
                        MM_SoftInt_Mul();
                    }
                    MM_NOT_IMPLEMENTED;
                }
                else if (c == '"')
                {
                    MM_String string = {
                        .data = lexer->string.data, // NOTE: cursor is the first element of string.data
                        .size = 0
                    };
                    
                    while (cursor != 0 && cursor != '\"')
                    {
                        if (cursor == '\\') MM_Advance(2); // NOTE: skip \"
                        else                MM_Advance(1);
                    }
                    
                    if (cursor != '\"')
                    {
                        //// ERROR: Unterminated string literal
                        MM_NOT_IMPLEMENTED;
                    }
                    else
                    {
                        MM_Advance(1);
                        
                        token.kind   = MM_Token_String;
                        token.string = string;
                    }
                }
                else
                {
                    //// ERROR: Unknown symbol
                    MM_NOT_IMPLEMENTED;
                }
            } break;
        }
        
        token.text.past_end = lexer->pos;
        
        return token;
    }
}

#undef MM_Advance