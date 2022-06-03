// NOTE: Helper functions
MM_Internal MM_bool MM_Lexer__AdvanceCharacter(MM_Lexer* lexer);
MM_Internal inline MM_String MM_Lexer__TokenString(MM_Lexer* lexer, MM_Token token);
MM_Internal MM_u32 MM_Lexer__ParseCodepoint(MM_String string, MM_umm* offset);

MM_API MM_Lexer
MM_Lexer_Init(MM_String string)
{
    return (MM_Lexer){
        .string         = string,
        .offset         = 0,
        .offset_to_line = 0,
        .line           = 1,
    };
}

// NOTE: The lexer is designed to only verify the input, not parse it.
//       This is not the best for performance, since verifying the input
//       is a huge part of parsing it. However, lexing without full
//       parsing to AST is not very useful outside of text editors.
//       Therefore, the lexer that will be exposed by the compiler API
//       is catered to text editors, and might be far from optimal for
//       the general case (it might be neccessary to use another
//       implementation for parsing in the compiler). This decision
//       will probably be reconsidered later, but the lexer will stay
//       this way for the time being.
MM_API MM_umm
MM_Lexer_NextTokens(MM_Lexer* lexer, MM_Token* buffer, MM_umm amount)
{
    // NOTE: The maximum string size for lexing is 4GB, this is enforced by the MM_String type by limiting size
    //       the range of MM_u32
    // NOTE: lexer->offset is 64-bit to ensure offet + n does not overflow (for any reasonable lookahead)
    
    MM_umm i = 0;
    for (; i < amount; ++i)
    {
        MM_u32 start_offset = lexer->offset;
        
        while (lexer->offset < lexer->string.size)
        {
            char c = lexer->string.data[lexer->offset];
            
            if (c == ' '  || c == '\t' ||
                c == '\v' || c == '\f' ||
                c == '\r')
            {
                lexer->offset += 1;
            }
            else if (c == '\n')
            {
                lexer->offset += 1;
                lexer->line   += 1;
                lexer->offset_to_line = lexer->offset;
            }
            else break;
        }
        
        MM_Token* token = &buffer[i];
        *token = (MM_Token){
            .kind              = MM_Token_Invalid, // NOTE: Assume invalid until proved otherwise
            .offset            = lexer->offset,
            .line              = lexer->line,
            .column            = lexer->offset - lexer->offset_to_line,
            .preceding_spacing = lexer->offset - start_offset,
            .length            = 0, // NOTE: Updated later
        };
        
        if (lexer->offset >= lexer->string.size)
        {
            MM_ASSERT(lexer->offset == lexer->string.size);
            
            token->kind = MM_Token_EndOfStream;
        }
        else if (lexer->offset + 1 < lexer->string.size && lexer->string.data[lexer->offset] == '/' &&
                 (lexer->string.data[lexer->offset + 1] == '/' || lexer->string.data[lexer->offset + 1] == '*'))
        {
            if (lexer->string.data[lexer->offset + 1] == '/')
            {
                token->kind = MM_Token_Comment;
                
                while (lexer->offset < lexer->string.size && (lexer->string.data[lexer->offset] != '\r' &&
                                                              lexer->string.data[lexer->offset] != '\n'))
                {
                    lexer->offset += 1;
                }
            }
            else
            {
                token->kind = MM_Token_BlockComment;
                
                lexer->offset += 2;
                
                MM_imm level = 1;
                while (lexer->offset + 1 < lexer->string.size)
                {
                    if (lexer->string.data[lexer->offset] == '/' && lexer->string.data[lexer->offset] == '*')
                    {
                        lexer->offset += 2;
                        level += 1;
                    }
                    else if (lexer->string.data[lexer->offset] == '*' && lexer->string.data[lexer->offset] == '/')
                    {
                        level -= 1;
                        if (level == 0) break;
                        
                        // NOTE: the offset is incremented after the loop exit oppertunity such that the two exit conditions
                        //       (lexer->offset + 1 >= lexer->string.size and level == 0) can be distinguished
                        lexer->offset += 2;
                    }
                    else lexer->offset += 1;
                }
                
                if (lexer->offset + 1 >= lexer->string.size)
                {
                    //// ERROR: Unterminated comment
                    MM_NOT_IMPLEMENTED;
                }
                else lexer->offset += 2; // NOTE: terminated comment, skipping the terminating "*/"
            }
        }
        else
        {
            char c[4];
            c[0] = lexer->string.data[lexer->offset++];
            c[1] = (lexer->offset + 0 < lexer->string.size ? lexer->string.data[lexer->offset + 0] : 0);
            c[2] = (lexer->offset + 1 < lexer->string.size ? lexer->string.data[lexer->offset + 1] : 0);
            c[3] = (lexer->offset + 2 < lexer->string.size ? lexer->string.data[lexer->offset + 2] : 0);
            
            switch (c[0])
            {
                case '(': token->kind = MM_Token_OpenParen;    break;
                case ')': token->kind = MM_Token_CloseParen;   break;
                case '[': token->kind = MM_Token_OpenBracket;  break;
                case ']': token->kind = MM_Token_CloseBracket; break;
                case '{': token->kind = MM_Token_OpenBrace;    break;
                case '}': token->kind = MM_Token_CloseBrace;   break;
                case ':': token->kind = MM_Token_Colon;        break;
                case ',': token->kind = MM_Token_Comma;        break;
                case ';': token->kind = MM_Token_Semicolon;    break;
                case '?': token->kind = MM_Token_QuestionMark; break;
                case '^': token->kind = MM_Token_Hat;          break;
                
                case '*': token->kind = (c[1] == '=' ? ++lexer->offset, MM_Token_StarEquals    : MM_Token_Star);    break;
                case '/': token->kind = (c[1] == '=' ? ++lexer->offset, MM_Token_SlashEquals   : MM_Token_Slash);   break;
                case '%': token->kind = (c[1] == '=' ? ++lexer->offset, MM_Token_PercentEquals : MM_Token_Percent); break;
                case '+': token->kind = (c[1] == '=' ? ++lexer->offset, MM_Token_PlusEquals    : MM_Token_Plus);    break;
                case '=': token->kind = (c[1] == '=' ? ++lexer->offset, MM_Token_EqualsEquals  : MM_Token_Equals);  break;
                case '!': token->kind = (c[1] == '=' ? ++lexer->offset, MM_Token_BangEquals    : MM_Token_Bang);    break;
                case '~': token->kind = (c[1] == '=' ? ++lexer->offset, MM_Token_TildeEquals   : MM_Token_Tilde);   break;
                
                // NOTE: I wrote this in a sort of cmov fashion, which might be a bad idea if the compiler does not
                //       understand it, or disagrees with the decision, but it looks nice, and should hopefully not
                //       matter that much
                case '-':
                {
                    token->kind = MM_Token_Minus;
                    if (c[1] == '=')                lexer->offset += 1, token->kind = MM_Token_MinusEquals;
                    if (c[1] == '>')                lexer->offset += 1, token->kind = MM_Token_Arrow;
                    if (c[1] == '-' && c[2] == '-') lexer->offset += 2, token->kind = MM_Token_TripleMinus;
                } break;
                
                case '.':
                {
                    token->kind = MM_Token_Period;
                    if (c[1] == '{') ++lexer->offset, token->kind = MM_Token_PeriodOpenBrace;
                    if (c[1] == '[') ++lexer->offset, token->kind = MM_Token_PeriodOpenBracket;
                }
                
                case '>':
                {
                    token->kind = MM_Token_Greater;
                    if (c[1] == '=')                               lexer->offset += 1, token->kind = MM_Token_GreaterEquals;
                    if (c[1] == '>' && c[2] != '=')                lexer->offset += 1, token->kind = MM_Token_GreaterGreater;
                    if (c[1] == '>' && c[2] == '=')                lexer->offset += 2, token->kind = MM_Token_GreaterGreaterEquals;
                    if (c[1] == '>' && c[2] == '>' && c[3] != '=') lexer->offset += 2, token->kind = MM_Token_TripleGreater;
                    if (c[1] == '>' && c[2] == '>' && c[3] == '=') lexer->offset += 3, token->kind = MM_Token_TripleGreaterEquals;
                } break;
                
                case '<':
                {
                    token->kind = MM_Token_Less;
                    if (c[1] == '=')                lexer->offset += 1, token->kind = MM_Token_LessEquals;
                    if (c[1] == '<' && c[2] != '=') lexer->offset += 1, token->kind = MM_Token_LessLess;
                    if (c[1] == '<' && c[2] == '=') lexer->offset += 2, token->kind = MM_Token_LessLessEquals;
                } break;
                
                case '&':
                {
                    token->kind = MM_Token_And;
                    if (c[1] == '=')                lexer->offset += 1, token->kind = MM_Token_AndEquals;
                    if (c[1] == '&' && c[2] != '=') lexer->offset += 1, token->kind = MM_Token_AndAnd;
                    if (c[1] == '&' && c[2] == '=') lexer->offset += 2, token->kind = MM_Token_AndAndEquals;
                } break;
                
                case '|':
                {
                    token->kind = MM_Token_Or;
                    if (c[1] == '=')                lexer->offset += 1, token->kind = MM_Token_OrEquals;
                    if (c[1] == '|' && c[2] != '=') lexer->offset += 1, token->kind = MM_Token_OrOr;
                    if (c[1] == '|' && c[2] == '=') lexer->offset += 2, token->kind = MM_Token_OrOrEquals;
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
                        
                        if (lexer->offset - ident_offset == 1 && c[0] == '_') token->kind = MM_Token_BlankIdentifier;
                        else
                        {
                            MM_NOT_IMPLEMENTED;
                            // TODO: identifier || keyword
                        }
                    }
                    else if (c[0] >= '0' && c[1] <= '9')
                    {
                        MM_bool is_float   = MM_false;
                        MM_umm base        = 10;
                        MM_umm digit_count = 0;
                        
                        if (c[0] == '0')
                        {
                            if      (c[1] == 'b') base = 2;
                            else if (c[1] == 'o') base = 8;
                            else if (c[1] == 'd') base = 10;
                            else if (c[1] == 'z') base = 12;
                            else if (c[1] == 'x') base = 16;
                            else if (c[1] == 'h') base = 16, is_float = MM_true;
                            else if (c[1] == 'y') base = 32;
                            else if (c[1] == 's') base = 60;
                        }
                        
                        if (base == 0) digit_count   += 1;
                        else           lexer->offset += 1;
                        
                        while (lexer->offset < lexer->string.size)
                        {
                            char c      = lexer->string.data[lexer->offset];
                            MM_u8 digit = 0;
                            if      (c >= '0' && c <= '9') digit = c - '0';
                            else if (c >= 'A' && c <= 'Z') digit = (c - 'A') + 9;
                            else if (c >= 'a' && c <= 'z') digit = (c - 'a') + 25;
                            else if (c == '_')
                            {
                                lexer->offset += 1;
                                continue;
                            }
                            else if (c == '.' && !is_float)
                            {
                                lexer->offset += 1;
                                break;
                            }
                            else break;
                            
                            if (digit >= base) break;
                            else
                            {
                                lexer->offset += 1;
                                digit_count   += 1;
                            }
                        }
                        
                        if (is_float)
                        {
                            if (base == 16)
                            {
                                if (digit_count != 4 && digit_count != 8 && digit_count != 16)
                                {
                                    //// ERROR: Invalid digit count for hex float literal
                                    MM_NOT_IMPLEMENTED;
                                }
                            }
                            else
                            {
                                if (lexer->offset == lexer->string.size ||
                                    !(lexer->string.data[lexer->offset] >= '0' && lexer->string.data[lexer->offset] <= '9'))
                                {
                                    //// ERROR: Missing fraction after decimal point in floating point literal
                                    MM_NOT_IMPLEMENTED;
                                }
                                else
                                {
                                    while (lexer->offset < lexer->string.size &&
                                           lexer->string.data[lexer->offset] >= '0' && lexer->string.data[lexer->offset] <= '9')
                                    {
                                        lexer->offset += 1;
                                    }
                                    
                                    if (lexer->offset < lexer->string.size && lexer->string.data[lexer->offset] == 'e')
                                    {
                                        lexer->offset += 1;
                                        
                                        if (lexer->offset < lexer->string.size &&
                                            (lexer->string.data[lexer->offset] == '+' || lexer->string.data[lexer->offset] == '-'))
                                        {
                                            lexer->offset += 1;
                                        }
                                        
                                        if (lexer->offset == lexer->string.size ||
                                            !(lexer->string.data[lexer->offset] >= '0' && lexer->string.data[lexer->offset] <= '9'))
                                        {
                                            //// ERROR: Missing exponent after exponent suffix
                                            MM_NOT_IMPLEMENTED;
                                        }
                                        else while (lexer->offset < lexer->string.size &&
                                                    lexer->string.data[lexer->offset] >= '0' && lexer->string.data[lexer->offset] <= '9')
                                        {
                                            lexer->offset += 1;
                                        }
                                    }
                                }
                            }
                        }
                        else token->kind = MM_Token_Int;
                    }
                    else if (c[0] == '"')
                    {
                        token->kind = MM_Token_String;
                        
                        MM_bool encountered_errors = MM_false;
                        while (lexer->offset < lexer->string.size && lexer->string.data[lexer->offset] != '"' && MM_Lexer__AdvanceCharacter(lexer));
                        
                        if (!encountered_errors)
                        {
                            if (lexer->offset == lexer->string.size)
                            {
                                //// ERROR: Unterminated string literal
                                MM_NOT_IMPLEMENTED;
                            }
                            else lexer->offset += 1;
                        }
                    }
                    else if (c[0] == '\'')
                    {
                        token->kind = MM_Token_Codepoint;
                        
                        if (MM_Lexer__AdvanceCharacter(lexer))
                        {
                            if (lexer->offset < lexer->string.size && lexer->string.data[lexer->offset] == '\'') lexer->offset += 1;
                            else
                            {
                                //// ERROR: Unterminated character literal
                                MM_NOT_IMPLEMENTED;
                            }
                        }
                    }
                    else
                    {
                        //// ERROR: Unknown byte
                        MM_NOT_IMPLEMENTED;
                    }
                } break;
            }
        }
        
        token->length = lexer->offset - token->offset;
    }
    
    return i;
}

MM_API MM_Token
MM_Lexer_NextToken(MM_Lexer* lexer)
{
    MM_Token token;
    
    MM_umm lexed_tokens = MM_Lexer_NextTokens(lexer, &token, 1);
    MM_ASSERT(lexed_tokens == 1);
    
    return token;
}

MM_API MM_i128
MM_Lexer_ParseInt(MM_Lexer* lexer, MM_Token token)
{
    MM_String string = MM_Lexer__TokenString(lexer, token);
    
    MM_umm offset = 0;
    MM_umm base   = 10;
    if (string.size > 1 && string.data[0] == '0' && (string.data[1] < '0' || string.data[1] > '9'))
    {
        if      (string.data[1] == 'b') base = 2;
        else if (string.data[1] == 'o') base = 8;
        else if (string.data[1] == 'd') base = 10;
        else if (string.data[1] == 'z') base = 12;
        else if (string.data[1] == 'x') base = 16;
        else if (string.data[1] == 'y') base = 32;
        else if (string.data[1] == 's') base = 60;
        else MM_INVALID_CODE_PATH;
        
        offset += 2;
    }
    
    MM_i128 integer = 0;
    while (offset < string.size)
    {
        MM_u8 digit = 0;
        char c      = string.data[offset];
        if      (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'A' && c <= 'Z') digit = (c - 'A') + 9;
        else if (c >= 'a' && c <= 'z') digit = (c - 'a') + 25;
        else continue;
        
        integer *= base;
        integer += digit;
    }
    
    return integer;
}

MM_API MM_f64
MM_Lexer_ParseFloat(MM_Lexer* lexer, MM_Token token)
{
    MM_f64 flt = 0;
    
    MM_String string = MM_Lexer__TokenString(lexer, token);
    
    if (string.size > 1 && string.data[0] == '0' && string.data[1] == 'h')
    {
        MM_u64 bits = 0;
        
        MM_umm offset = 2;
        while (offset < string.size)
        {
            char c = string.data[offset];
            
            bits *= 16;
            bits += (c >= '0' && c <= '9' ? c - '0' : (c - 'A') + 9);
        }
        
        MM_umm digit_count = string.size - offset;
        
        if (digit_count == 4)
        {
            flt = MM_F64_FromF16((MM_f16)bits);
        }
        else if (digit_count == 8)
        {
            MM_f32 f;
            MM_Copy(&bits, &f, sizeof(MM_u32));
            
            flt = (MM_f64)f;
        }
        else MM_Copy(&bits, &flt, sizeof(MM_u64));
    }
    else
    {
        MM_NOT_IMPLEMENTED;
    }
    
    return flt;
}

MM_API MM_u32
MM_Lexer_ParseChar(MM_Lexer* lexer, MM_Token token)
{
    MM_umm offset = 1;
    MM_String string = MM_Lexer__TokenString(lexer, token);
    return MM_Lexer__ParseCodepoint(string, &offset);
}

MM_API MM_String
MM_Lexer_ParseIdentifier(MM_Lexer* lexer, MM_Token token)
{
    return MM_Lexer__TokenString(lexer, token);
}

MM_API MM_String
MM_Lexer_ParseString(MM_Lexer* lexer, MM_Token token, MM_u8* buffer) // NOTE: buffer should be at least token.size long
{
    MM_String string = MM_Lexer__TokenString(lexer, token);
    
    MM_umm read_offset  = 0;
    MM_umm write_offset = 0;
    while (read_offset < string.size)
    {
        MM_u32 codepoint = MM_Lexer__ParseCodepoint(string, &read_offset);
        
        if (codepoint <= 0x7F)
        {
            buffer[write_offset++] = (MM_u8)codepoint;
        }
        else if (codepoint <= 0x7FF)
        {
            buffer[write_offset++] = (MM_u8)((codepoint & 0x7C) >> 6) | 0xC0;
            buffer[write_offset++] = (MM_u8)((codepoint & 0x3F) >> 0) | 0x80;
        }
        else if (codepoint <= 0xFFFF)
        {
            buffer[write_offset++] = (MM_u8)((codepoint & 0xF000) >> 12) | 0xE0;
            buffer[write_offset++] = (MM_u8)((codepoint & 0x0FC0) >> 6)  | 0x80;
            buffer[write_offset++] = (MM_u8)((codepoint & 0x003F) >> 0)  | 0x80;
        }
        else if (codepoint <= 0x10FFFF)
        {
            buffer[write_offset++] = (MM_u8)((codepoint & 0x700000) >> 18) | 0xF0;
            buffer[write_offset++] = (MM_u8)((codepoint & 0x03F000) >> 12) | 0x80;
            buffer[write_offset++] = (MM_u8)((codepoint & 0x000FC0) >> 6)  | 0x80;
            buffer[write_offset++] = (MM_u8)((codepoint & 0x00003F) >> 0)  | 0x80;
        }
    }
    return (MM_String){ .data = buffer, .size = write_offset };
}

//
// NOTE: Helper functions
//

MM_Internal MM_bool
MM_Lexer__AdvanceCharacter(MM_Lexer* lexer)
{
    MM_bool encountered_errors = MM_false;
    
    if (lexer->string.data[lexer->offset] == '\\')
    {
        lexer->offset += 1;
        
        if (lexer->offset != lexer->string.size)
        {
            char c = lexer->string.data[lexer->offset];
            if (c == 'a' || c == 'b'  || c == 'f'  ||
                c == 'n' || c == 'r'  || c == 't'  ||
                c == 'v' || c == '\\' || c == '\'' ||
                c == '"')
            {
                lexer->offset += 1;
            }
            else if (c == 'x' || c == 'u' || c == 'U')
            {
                lexer->offset += 1;
                
                MM_umm remaining_digits = (c == 'x' ? 2 : (c == 'u' ? 4 : 6));
                MM_umm codepoint        = 0;
                
                while (lexer->offset < lexer->string.size && remaining_digits > 0)
                {
                    c = lexer->string.data[lexer->offset];
                    
                    MM_u8 digit;
                    if      (c >= '0' && c <= '9') digit = c - '0';
                    else if (c >= 'A' && c <= 'F') digit = (c - 'A') + 9;
                    else break;
                    
                    lexer->offset    += 1;
                    remaining_digits -= 1;
                    
                    codepoint *= 16;
                    codepoint += digit;
                }
                
                
                if (remaining_digits != 0)
                {
                    //// ERROR: Missing digits in codepoint escape sequence
                    encountered_errors = MM_true;
                    MM_NOT_IMPLEMENTED;
                }
                else if (codepoint > 0x10FFFF)
                {
                    //// ERROR: Codepoint is out of UTF-8 range
                    encountered_errors = MM_true;
                    MM_NOT_IMPLEMENTED;
                }
            }
            else
            {
                //// ERROR: Unknown escape sequence
                encountered_errors = MM_true;
                MM_NOT_IMPLEMENTED;
            }
        }
    }
    else lexer->offset += 1;
    
    return !encountered_errors;
}

MM_Internal inline MM_String
MM_Lexer__TokenString(MM_Lexer* lexer, MM_Token token)
{
    return (MM_String){
        .data = lexer->string.data + token.offset,
        .size = token.length
    };
}

MM_Internal MM_u32
MM_Lexer__ParseCodepoint(MM_String string, MM_umm* offset)
{
    MM_u32 codepoint;
    
    if (string.data[*offset] == '\\')
    {
        *offset += 1;
        
        char c = string.data[*offset];
        if      (c == 'a')  *offset += 1, codepoint = '\a';
        else if (c == 'b')  *offset += 1, codepoint = '\b';
        else if (c == 'f')  *offset += 1, codepoint = '\f';
        else if (c == 'n')  *offset += 1, codepoint = '\n';
        else if (c == 'r')  *offset += 1, codepoint = '\r';
        else if (c == 't')  *offset += 1, codepoint = '\t';
        else if (c == 'v')  *offset += 1, codepoint = '\v';
        else if (c == '\\') *offset += 1, codepoint = '\\';
        else if (c == '\'') *offset += 1, codepoint = '\'';
        else if (c == '"')  *offset += 1, codepoint = '"';
        else
        {
            *offset += 1;
            
            codepoint = 0;
            for (MM_umm i = 0; i < (c == 'x' ? 2 : (c == 'u' ? 4 : 6)); ++i)
            {
                c        = string.data[*offset];
                *offset += 1;
                
                codepoint *= 16;
                codepoint += (c >= '0' && c <= '9' ? c - '0' : (c - 'A') + 9);
            }
        }
    }
    else
    {
        codepoint = string.data[*offset];
        *offset += 1;
    }
    
    return codepoint;
}