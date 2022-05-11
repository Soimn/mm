typedef enum TOKEN_KIND
{
    Token_Invalid = 0,
    Token_EndOfStream,
    
    Token_OpenParen,
    Token_CloseParen,
    Token_OpenBracket,
    Token_CloseBracket,
    Token_OpenBrace,
    Token_CloseBrace,
    Token_Colon,
    Token_Comma,
    Token_Semicolon,
    Token_QuestionMark,
    Token_Hat,
    Token_Cash,
    Token_Pound,
    Token_At,
    Token_Arrow,
    Token_TripleMinus,
    Token_Period,
    Token_PeriodOpenBrace,
    Token_PeriodOpenBracket,
    Token_Identifier,
    Token_String,
    Token_Character,
    Token_Int,
    Token_Float,
    Token_Equals,
    Token_Bang,
    
    Token_FirstAssignment = 3*16,
    Token_FirstMulLevelAssignment = Token_FirstAssignment,
    Token_StarEquals = Token_FirstMulLevelAssignment,
    Token_SlashEquals,
    Token_PercentEquals,
    Token_AndEquals,
    Token_LessLessEquals,
    Token_GreaterGreaterEquals,
    Token_TripleGreaterEquals,
    Token_LastMulLevelAssignment = Token_GreaterGreaterEquals,
    
    Token_FirstAddLevelAssignment = 4*16,
    Token_NotEquals = Token_FirstAddLevelAssignment,
    Token_OrEquals,
    Token_MinusEquals,
    Token_PlusEquals,
    Token_LastAddLevelAssignment = Token_PlusEquals,
    
    Token_AndAndEquals = 6*16,
    
    Token_OrOrEquals = 7*16,
    Token_LastAssignment = Token_OrOrEquals,
    
    Token_FirstBinary = 8*16,
    Token_FirstMulLevel = Token_FirstBinary,
    Token_Star = Token_FirstMulLevel,
    Token_Slash,
    Token_Percent,
    Token_And,
    Token_LessLess,
    Token_GreaterGreater,
    Token_TripleGreater,
    Token_LastMulLevel = Token_TripleGreater,
    
    Token_FirstAddLevel = 9*16,
    Token_Not = Token_FirstAddLevel,
    Token_Or,
    Token_Minus,
    Token_Plus,
    Token_LastAddLevel = Token_Plus,
    
    Token_FirstCmpLevel = 10*16,
    Token_Greater = Token_FirstCmpLevel,
    Token_GreaterEquals,
    Token_Less,
    Token_LessEquals,
    Token_EqualsEquals,
    Token_BangEquals,
    Token_LastCmpLevel = Token_BangEquals,
    
    Token_AndAnd = 11*16,
    
    Token_OrOr = 12*16,
    Token_LastBinary = Token_OrOr,
} TOKEN_KIND;

typedef struct Token
{
    TOKEN_KIND kind;
    u32 offset_raw;
    u32 offset;
    u32 line;
    u32 column;
    u32 size;
    
    union
    {
        Interned_String string;
        u32 character;
        
        struct
        {
            union
            {
                Big_Int integer;
                Big_Float floating;
            };
            i8 base;
        };
    };
} Token;

typedef struct Lexer
{
    u8* content;
    u8* cursor;
    u32 offset_to_line;
    u32 line;
} Lexer;

internal Lexer
Lexer_Init(Workspace* workspace, u8* content)
{
    return (Lexer){
        .content        = content,
        .cursor         = content,
        .offset_to_line = 0,
        .line           = 1,
    };
}

internal bool Lexer__DecodeCharacter(u8** cursor, u8* result, umm* advancement);

internal Token
Lexer_Advance(Workspace* workspace, Lexer* lexer)
{
    Token token = { .kind = Token_Invalid, .offset_raw = (u32)(lexer->cursor - lexer->content) };
    
    imm comment_level = 0;
    while (true)
    {
        if (*lexer->cursor == '\n')
        {
            ++lexer->cursor;
            ++lexer->line;
            lexer->offset_to_line = (u32)(lexer->cursor - lexer->content);
        }
        else if (lexer->cursor[0] == '/' && lexer->cursor[1] == '*')
        {
            lexer->cursor += 2;
            ++comment_level;
        }
        else if (comment_level > 0 && lexer->cursor[0] == '*' && lexer->cursor[1] == '/')
        {
            lexer->cursor += 2;
            --comment_level;
        }
        else if (*lexer->cursor != 0 && comment_level > 0 || *lexer->cursor == ' '  ||
                 *lexer->cursor == '\t'                   || *lexer->cursor == '\v' ||
                 *lexer->cursor == '\f'                   || *lexer->cursor == '\r')
        {
            ++lexer->cursor;
        }
        else if (lexer->cursor[0] == '/' && lexer->cursor[1] == '/')
        {
            ASSERT(comment_level == 0);
            while (*lexer->cursor != 0 && *lexer->cursor != '\n') ++lexer->cursor;
        }
        else break;
    }
    
    token.offset = (u32)(lexer->cursor - lexer->content);
    token.column = token.offset - lexer->offset_to_line;
    token.line   = lexer->line;
    // NOTE: defer token.size = (u32)(lexer->cursor - lexer->content) - token.offset;
    
    u8 c = *lexer->cursor;
    lexer->cursor += (c != 0);
    
    switch (c)
    {
        case 0:   token.kind = Token_EndOfStream;  break;
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
        case '^': token.kind = Token_Hat;          break;
        case '$': token.kind = Token_Cash;         break;
        case '#': token.kind = Token_Pound;        break;
        case '@': token.kind = Token_At;           break;
        
        case '+': token.kind = (*lexer->cursor == '=' ? ++lexer->cursor, Token_PlusEquals    : Token_Plus);    break;
        case '*': token.kind = (*lexer->cursor == '=' ? ++lexer->cursor, Token_StarEquals    : Token_Star);    break;
        case '/': token.kind = (*lexer->cursor == '=' ? ++lexer->cursor, Token_SlashEquals   : Token_Slash);   break;
        case '%': token.kind = (*lexer->cursor == '=' ? ++lexer->cursor, Token_PercentEquals : Token_Percent); break;
        case '~': token.kind = (*lexer->cursor == '=' ? ++lexer->cursor, Token_NotEquals     : Token_Not);     break;
        case '!': token.kind = (*lexer->cursor == '=' ? ++lexer->cursor, Token_BangEquals    : Token_Bang);    break;
        case '=': token.kind = (*lexer->cursor == '=' ? ++lexer->cursor, Token_EqualsEquals  : Token_Equals);  break;
        
        case '-':
        {
            if      (*lexer->cursor == '>')                              lexer->cursor += 1, token.kind = Token_Arrow;
            else if (*lexer->cursor == '=')                              lexer->cursor += 1, token.kind = Token_MinusEquals;
            else if (lexer->cursor[0] == '-' && lexer->cursor[1] == '-') lexer->cursor += 2, token.kind = Token_TripleMinus;
            else                                                                             token.kind = Token_Minus;
        } break;
        
        case '<':
        {
            if      (*lexer->cursor == '=') ++lexer->cursor, token.kind = Token_LessEquals;
            else if (*lexer->cursor == '<')
            {
                if (lexer->cursor[1] == '=') lexer->cursor += 2, token.kind = Token_LessLessEquals;
                else                         lexer->cursor += 1, token.kind = Token_LessLess;
            }
            else token.kind = Token_Less;
        } break;
        
        case '&':
        {
            if      (*lexer->cursor == '=') ++lexer->cursor, token.kind = Token_AndEquals;
            else if (*lexer->cursor == '&')
            {
                if (lexer->cursor[1] == '=') lexer->cursor += 2, token.kind = Token_AndAndEquals;
                else                         lexer->cursor += 1, token.kind = Token_AndAnd;
            }
            else token.kind = Token_And;
        } break;
        
        case '|':
        {
            if      (*lexer->cursor == '=') ++lexer->cursor, token.kind = Token_OrEquals;
            else if (*lexer->cursor == '|')
            {
                if (lexer->cursor[1] == '=') lexer->cursor += 2, token.kind = Token_OrOrEquals;
                else                         lexer->cursor += 1, token.kind = Token_OrOr;
            }
            else token.kind = Token_Or;
        } break;
        
        case '>':
        {
            if      (*lexer->cursor == '=') ++lexer->cursor, token.kind = Token_GreaterEquals;
            else if (*lexer->cursor == '>')
            {
                if (lexer->cursor[1] == '>')
                {
                    if (lexer->cursor[2] == '=') lexer->cursor += 3, token.kind = Token_TripleGreaterEquals;
                    else                         lexer->cursor += 2, token.kind = Token_TripleGreater;
                }
                else if (lexer->cursor[1] == '=') lexer->cursor += 2, token.kind = Token_GreaterGreaterEquals;
                else                              lexer->cursor += 1, token.kind = Token_GreaterGreater;
            }
            else token.kind = Token_Less;
        } break;
        
        case '.':
        {
            if      (*lexer->cursor == '{') ++lexer->cursor, token.kind = Token_PeriodOpenBrace;
            else if (*lexer->cursor == '[') ++lexer->cursor, token.kind = Token_PeriodOpenBracket;
            else                                             token.kind = Token_Period;
        } break;
        
        default:
        {
            if (c >= 'a' && c <= 'z' ||
                c >= 'A' && c <= 'Z' ||
                c == '_')
            {
                String identifier = {
                    .data = lexer->cursor - 1,
                    .size = 1,
                };
                
                while (*lexer->cursor >= 'a' && *lexer->cursor <= 'z' ||
                       *lexer->cursor >= 'A' && *lexer->cursor <= 'Z' ||
                       *lexer->cursor >= '0' && *lexer->cursor <= '9' ||
                       *lexer->cursor == '_')
                {
                    ++lexer->cursor;
                    ++identifier.size;
                }
                
                u64 hash                      = String_Hash(identifier);
                Interned_String_Entry** entry = InternedString__FindSpot(workspace, hash, identifier);
                
                if (*entry == 0)
                {
                    *entry = Arena_PushSize(workspace->workspace_arena, sizeof(Interned_String_Entry) + identifier.size, ALIGNOF(Interned_String_Entry));
                    **entry = (Interned_String_Entry){
                        .next = 0,
                        .hash = hash,
                        .size = identifier.size,
                    };
                    
                    Copy(identifier.data, *entry + 1, identifier.size);
                }
                
                token.kind   = Token_Identifier;
                token.string = InternedString__FromInternedStringEntry(workspace, *entry);
            }
            else if (c >= '0' && c <= '9')
            {
                bool encountered_errors = false;
                
                imm is_float    = -1; // NOTE: is_float is ternary: -1 undecided, 0 false, 1 true
                imm base        = -1;
                umm digit_count = 0;
                Big_Int integer;
                BIGNUM_STATUS status = BigNumStatus_None;
                
                if (c == '0')
                {
                    if      (*lexer->cursor == 'b') base = 2;
                    else if (*lexer->cursor == 't') base = 3;
                    else if (*lexer->cursor == 'o') base = 8;
                    else if (*lexer->cursor == 'd') base = 10;
                    else if (*lexer->cursor == 'h') base = 16;
                    else if (*lexer->cursor == 'x') base = 16;
                    else if (*lexer->cursor == 'y') base = 32;
                    else if (*lexer->cursor == 'z') base = 60;
                    
                    if (base != -1)
                    {
                        is_float = (*lexer->cursor == 'h');
                        ++lexer->cursor;
                    }
                }
                
                umm parse_base = (umm)base;
                if (base == -1)
                {
                    integer      = BigInt_FromU64(c - '0');
                    parse_base   = 10;
                    digit_count += 1;
                }
                else integer = BigInt_FromU64(0);
                
                while (!encountered_errors)
                {
                    umm digit;
                    if      (*lexer->cursor >= '0' && *lexer->cursor <= '9') digit = *lexer->cursor & 0x0F;
                    else if (*lexer->cursor >= 'A' && *lexer->cursor <= 'z') digit = *lexer->cursor & 0x1F + 9;
                    else if (*lexer->cursor >= 'a' && *lexer->cursor <= 'x') digit = *lexer->cursor & 0x1F + 25;
                    else if (*lexer->cursor == '_')
                    {
                        ++lexer->cursor;
                        continue;
                    }
                    else if (is_float == -1 && *lexer->cursor == '.')
                    {
                        is_float = true;
                        ++lexer->cursor;
                        break;
                    }
                    else break;
                    
                    if (digit >= (umm)base)
                    {
                        //// ERROR: digit is too large for current base in numeric literal
                        encountered_errors = true;
                    }
                    else
                    {
                        integer      = BigInt_MulAddU64(integer, parse_base, digit, &status);
                        digit_count += 1;
                        
                        ++lexer->cursor;
                    }
                }
                
                if (!encountered_errors)
                {
                    if ((status & BigNumStatus_Carry) != 0 || (status & BigNumStatus_Overflow) != 0)
                    {
                        //// ERROR: Numeric literal contains too many digits to be representable by any type
                        encountered_errors = true;
                    }
                    else
                    {
                        if (is_float == true)
                        {
                            if (base == 16)
                            {
                                if (digit_count == 2)
                                {
                                    token.kind     = Token_Float;
                                    token.floating = BigFloat_FromBits(integer, 16);
                                    token.base     = (i8)base;
                                }
                                else if (digit_count == 4)
                                {
                                    token.kind     = Token_Float;
                                    token.floating = BigFloat_FromBits(integer, 32);
                                    token.base     = (i8)base;
                                }
                                else if (digit_count == 8)
                                {
                                    token.kind     = Token_Float;
                                    token.floating = BigFloat_FromBits(integer, 64);
                                    token.base     = (i8)base;
                                }
                                else if (digit_count == 16)
                                {
                                    token.kind     = Token_Float;
                                    token.floating = BigFloat_FromBits(integer, 128);
                                    token.base     = (i8)base;
                                }
                                else
                                {
                                    //// ERROR: Invalid number of digits in hexadecimal floating point literal
                                    encountered_errors = true;
                                }
                            }
                            else
                            {
                                if (*lexer->cursor < '0' || *lexer->cursor > '9')
                                {
                                    //// ERROR: Missing fractional part after decimal point in floating point numeric literal
                                    encountered_errors = true;
                                }
                                else
                                {
                                    Big_Int fractional = BigInt_FromU64(0);
                                    Big_Int exponent = BigInt_FromU64(0);
                                    
                                    while (*lexer->cursor >= '0' && *lexer->cursor <= '9')
                                    {
                                        fractional = BigInt_MulAddU64(fractional, 10, *lexer->cursor & 0xF, &status);
                                        ++lexer->cursor;
                                    }
                                    
                                    if ((status & BigNumStatus_Carry) != 0 || (status & BigNumStatus_Overflow) != 0)
                                    {
                                        //// ERROR: Fractional part of numeric literal contains too many digits to be representable by any type
                                        encountered_errors = true;
                                    }
                                    else if (*lexer->cursor == 'e')
                                    {
                                        ++lexer->cursor;
                                        
                                        bool is_negative = false;
                                        if      (*lexer->cursor == '+') ++lexer->cursor;
                                        else if (*lexer->cursor == '-') ++lexer->cursor, is_negative = true;
                                        
                                        if (*lexer->cursor < '0' || *lexer->cursor > '9')
                                        {
                                            //// ERROR: Missing exponent
                                            encountered_errors = true;
                                        }
                                        else while (*lexer->cursor >= '0' && *lexer->cursor <= '9')
                                        {
                                            exponent = BigInt_MulAddU64(exponent, 10, *lexer->cursor & 0xF, &status);
                                            ++lexer->cursor;
                                        }
                                        
                                        if ((status & BigNumStatus_Carry) != 0 || (status & BigNumStatus_Overflow) != 0)
                                        {
                                            //// ERROR: Exponent part of numeric literal contains too many digits to be representable by any type
                                            encountered_errors = true;
                                        }
                                    }
                                    
                                    if (!encountered_errors)
                                    {
                                        token.kind     = Token_Float;
                                        token.floating = BigFloat_FromScientificNotation(integer, fractional, exponent);
                                        token.base     = (i8)base;
                                    }
                                }
                            }
                        }
                        else
                        {
                            token.kind    = Token_Int;
                            token.integer = integer;
                            token.base    = (i8)base;
                        }
                    }
                }
            }
            else if (c == '"')
            {
                bool encountered_errors = false;
                
                Interned_String interned_string = EMPTY_STRING;
                
                if (*lexer->cursor != '"')
                {
                    String raw_string = {
                        .data = lexer->cursor,
                        .size = 0,
                    };
                    
                    while (*lexer->cursor != 0 && *lexer->cursor != '"')
                    {
                        ++lexer->cursor;
                        ++raw_string.size;
                    }
                    
                    if (*lexer->cursor != '"')
                    {
                        //// ERROR: Missing terminating " after string contents in string literal
                        encountered_errors = true;
                    }
                    else
                    {
                        Arena_Marker marker = Arena_BeginTemp(workspace->workspace_arena);
                        
                        Interned_String_Entry* new_entry = Arena_PushSize(workspace->workspace_arena, sizeof(Interned_String_Entry) + raw_string.size, ALIGNOF(Interned_String_Entry));
                        
                        u8* raw_string_cursor = raw_string.data;
                        String string = {
                            .data = (u8*)(new_entry + 1),
                            .size = 0,
                        };
                        
                        while (!encountered_errors && *raw_string_cursor != '"')
                        {
                            if (!Lexer__DecodeCharacter(&raw_string_cursor, string.data, &string.size))
                            {
                                //// ERROR
                                encountered_errors = true;
                            }
                        }
                        
                        if (encountered_errors) Arena_EndTemp(workspace->workspace_arena, marker);
                        else
                        {
                            u64 hash                      = String_Hash(string);
                            Interned_String_Entry** entry = InternedString__FindSpot(workspace, hash, string);
                            
                            if (*entry != 0) Arena_EndTemp(workspace->workspace_arena, marker);
                            else
                            {
                                Arena_ReifyTemp(workspace->workspace_arena, marker);
                                
                                **entry = (Interned_String_Entry){
                                    .next = 0,
                                    .hash = hash,
                                    .size = raw_string.size,
                                };
                            }
                            
                            interned_string = InternedString__FromInternedStringEntry(workspace, *entry);
                        }
                    }
                }
                
                if (!encountered_errors)
                {
                    ASSERT(*lexer->cursor == '"');
                    ++lexer->cursor;
                    
                    token.kind   = Token_String;
                    token.string = interned_string;
                }
            }
            else if (c == '\'')
            {
                u8 bytes[4];
                umm advancement = 0;
                
                if (c == '\'')
                {
                    //// ERROR: Missing character in character literal
                }
                else if (!Lexer__DecodeCharacter(&lexer->cursor, bytes, &advancement))
                {
                    //// ERROR
                }
                else if (c != '\'')
                {
                    //// ERROR: Missing terminating ' after character in character literal
                }
                else
                {
                    ++lexer->cursor;
                    
                    u32 codepoint;
                    if      (advancement == 1) codepoint = (u32)(bytes[0] & 0x7F);
                    else if (advancement == 2) codepoint = (u32)(bytes[0] & 0x1F) << 6  | (u32)(bytes[1] & 0x3F);
                    else if (advancement == 3) codepoint = (u32)(bytes[0] & 0x0F) << 12 | (u32)(bytes[1] & 0x3F) << 6  | (u32)(bytes[2] & 0x3F);
                    else                       codepoint = (u32)(bytes[0] & 0x07) << 18 | (u32)(bytes[1] & 0x3F) << 12 | (u32)(bytes[2] & 0x3F) << 6 | (u32)(bytes[3] & 0x3F);
                    
                    token.kind      = Token_Character;
                    token.character = codepoint;
                }
            }
        } break;
    }
    
    token.size = (u32)(lexer->cursor - lexer->content) - token.offset;
    
    return token;
}

internal bool
Lexer__DecodeCharacter(u8** cursor, u8* result, umm* advancement)
{
    bool encountered_errors = false;
    
    *advancement += 1; // NOTE: assume one byte char length, is corrected for multibyte codepoints
    if (**cursor == '\\')
    {
        *cursor += 1;
        
        switch (**cursor)
        {
            case 'a':  *result = '\a'; break;
            case 'b':  *result = '\b'; break;
            case 'f':  *result = '\f'; break;
            case 'n':  *result = '\n'; break;
            case 'r':  *result = '\r'; break;
            case 't':  *result = '\t'; break;
            case 'v':  *result = '\v'; break;
            case '\\': *result = '\\'; break;
            case '\'': *result = '\''; break;
            case '"':  *result = '"';  break;
            
            case 'x':
            case 'u':
            case 'U':
            {
                umm digit_count = (**cursor == 'x' ? 2 : (**cursor == 'u' ? 4 : 8));
                
                umm codepoint;
                for (umm i = 0; i < digit_count; ++i)
                {
                    if      ((*cursor)[1] >= '0' && (*cursor)[1] <= '9') codepoint = codepoint << 4 | ((*cursor)[1] & 0x0F);
                    else if ((*cursor)[1] >= 'A' && (*cursor)[1] <= 'F') codepoint = codepoint << 4 | ((*cursor)[1] & 0x0F + 9);
                    else
                    {
                        //// ERROR: Missing digits of x escape sequence
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    if      (codepoint <= 0x7F) *result = (u8)codepoint;
                    else if (codepoint <= 0x7FF)
                    {
                        result[0] = (u8)((codepoint & 0x7C) >> 6) | 0xC0;
                        result[1] = (u8)((codepoint & 0x3F) >> 0) | 0x80;
                        *advancement += 1;
                    }
                    else if (codepoint <= 0xFFFF)
                    {
                        result[0] = (u8)((codepoint & 0xF000) >> 12) | 0xE0;
                        result[1] = (u8)((codepoint & 0x0FC0) >> 6)  | 0x80;
                        result[2] = (u8)((codepoint & 0x003F) >> 0)  | 0x80;
                        *advancement += 2;
                    }
                    else if (codepoint <= 0x10FFFF)
                    {
                        result[0] = (u8)((codepoint & 0x700000) >> 18) | 0xF0;
                        result[1] = (u8)((codepoint & 0x03F000) >> 12) | 0x80;
                        result[2] = (u8)((codepoint & 0x000FC0) >> 6)  | 0x80;
                        result[3] = (u8)((codepoint & 0x00003F) >> 0)  | 0x80;
                        *advancement += 3;
                    }
                    else
                    {
                        //// ERROR: Codepoint out of UTF-8 range
                        encountered_errors = true;
                    }
                }
            } break;
            
            default:
            {
                if (**cursor == 0)
                {
                    //// ERROR: Missing escape sequence after backslash
                    encountered_errors = true;
                }
                else
                {
                    //// ERROR: Illegal escape sequence
                    encountered_errors = true;
                }
            } break;
        }
    }
    else if ((**cursor & 0x80) != 0)
    {
        umm length = 1;
        if      ((**cursor & 0xF8) == 0xF0) length = 4;
        else if ((**cursor & 0xF0) == 0xE0) length = 3;
        else if ((**cursor & 0xE0) == 0xC0) length = 2;
        else
        {
            //// ERROR: Illegal UTF-8 byte
            encountered_errors = true;
        }
        
        if (!encountered_errors)
        {
            *result++     = **cursor;
            *cursor      += 1;
            *advancement += length - 1;
            
            for (umm i = 0; i < length; ++i, ++cursor)
            {
                if ((**cursor & 0xC0) == 0x80) *result++ = **cursor;
                else
                {
                    //// ERROR: Missing sequence byte in UTF-8 codepoint
                    encountered_errors = true;
                }
            }
        }
    }
    else *result = **cursor;
    
    *cursor += 1;
    
    return !encountered_errors;
}