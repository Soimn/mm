MM__internal inline MM_bool
MM__IsAlpha(MM_u8 c)
{
    return (c >= 'a' && c <= 'z' ||
            c >= 'A' && c <= 'Z');
}

MM__internal inline MM_bool
MM__IsNumeric(MM_u8 c)
{
    return (c >= '0' && c <= '9');
}

MM__internal inline MM_bool
MM__IsAlphaNumericOrUnderscore(MM_u8 c)
{
    return (MM__IsAlpha(c) || MM__IsNumeric(c) || c == '_');
}

MM_API MM_bool
MM_LexString(MM_String string, MM_String_Table* string_table, MM_DynArray(MM_Token)* tokens, MM_Error* error)
{
    MM_bool encountered_errors = false;
    
    MM_umm offset = 0;
    MM_umm line   = 1;
    MM_umm column = 1;
    
    while (!encountered_errors)
    {
        /// Skip comments and whitespace
        while (offset < string.size)
        {
            MM_u8 c = string.data[offset];
            
            if (c == ' '  || c == '\t' ||
                c == '\v' || c == '\r')
            {
                offset += 1;
                column += 1;
            }
            else if (c == '\n')
            {
                offset += 1;
                line   += 1;
                column  = 1;
            }
            else if (c == '/' && string.data[offset + 1] == '/')
            {
                offset += 2;
                
                while (offset < string.size && string.data[offset] != '\n') offset += 1;
            }
            else if (c == '/' && offset + 1 < string.size && string.data[offset + 1] == '*')
            {
                offset += 2;
                
                MM_u32 nest_level = 1;
                while (offset < string.size && nest_level != 0)
                {
                    if (string.data[offset] == '\n')
                    {
                        offset += 1;
                        line   += 1;
                        column  = 1;
                    }
                    else
                    {
                        MM_u8 c0 = string.data[offset];
                        MM_u8 c1 = (offset + 1 < string.size ? string.data[offset + 1] : 0);
                        
                        if (c0 == '/' && c1 == '*')
                        {
                            nest_level += 1;
                            offset     += 2;
                        }
                        else if (c0 == '*' && c1 == '/')
                        {
                            nest_level -= 1;
                            offset     += 2;
                        }
                        else offset += 1;
                    }
                }
            }
            
            else break;
        }
        
        /// 
        
        MM_Token* token = 0;
        MM__NOT_IMPLEMENTED;
        
        MM_u8 c[3] = {
            offset + 0 < string.size ? string.data[offset + 0] : 0,
            offset + 1 < string.size ? string.data[offset + 1] : 0,
            offset + 2 < string.size ? string.data[offset + 2] : 0,
        };
        
        if (c[0] == 0) break;
        else
        {
            offset += 1;
            
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
                case '~': token->kind = MM_Token_Complement;   break;
                
#define SINGLE_OR_EQ(single_c, single, eq) \
case single_c:                         \
{                                      \
token->kind = single;              \
if (c[1] == '=')                   \
{                                  \
token->kind = eq;              \
offset += 1;                   \
}                                  \
} break
                
                SINGLE_OR_EQ('!', MM_Token_Not, MM_Token_NotEquals);
                SINGLE_OR_EQ('+', MM_Token_Plus, MM_Token_PlusEquals);
                SINGLE_OR_EQ('*', MM_Token_Star, MM_Token_StarEquals);
                SINGLE_OR_EQ('=', MM_Token_Equals, MM_Token_EqualEquals);
                SINGLE_OR_EQ('/', MM_Token_Slash, MM_Token_SlashEquals);
                SINGLE_OR_EQ('%', MM_Token_Rem, MM_Token_RemEquals);
                SINGLE_OR_EQ('^', MM_Token_Hat, MM_Token_HatEquals);
                
#undef SINGLE_OR_EQ
                
                case '|':
                {
                    token->kind = MM_Token_Or;
                    
                    if (c[1] == '=')
                    {
                        token->kind = MM_Token_OrEquals;
                        offset     += 1;
                    }
                    
                    if (c[1] == '|')
                    {
                        token->kind = MM_Token_OrOr;
                        offset     += 1;
                        
                        if (c[2] == '=')
                        {
                            token->kind = MM_Token_OrOrEquals;
                            offset     += 1;
                        }
                    }
                } break;
                
                case '&':
                {
                    token->kind = MM_Token_And;
                    
                    if (c[1] == '=')
                    {
                        token->kind = MM_Token_AndEquals;
                        offset     += 1;
                    }
                    
                    if (c[1] == '&')
                    {
                        token->kind = MM_Token_AndAnd;
                        offset     += 1;
                        
                        if (c[2] == '=')
                        {
                            token->kind = MM_Token_AndAndEquals;
                            offset     += 1;
                        }
                    }
                } break;
                
                case '<':
                {
                    token->kind = MM_Token_Less;
                    
                    if (c[1] == '=')
                    {
                        token->kind = MM_Token_LessEquals;
                        offset     += 1;
                    }
                    
                    if (c[1] == '<')
                    {
                        token->kind = MM_Token_LeftShift;
                        offset     += 1;
                        
                        if (c[2] == '=')
                        {
                            token->kind = MM_Token_LeftShiftEquals;
                            offset     += 1;
                        }
                        
                        if (c[2] == '<')
                        {
                            token->kind = MM_Token_SplatLeftShift;
                            offset     += 1;
                            
                            if (c[3] == '=')
                            {
                                token->kind = MM_Token_SplatLeftShiftEquals;
                                offset     += 1;
                            }
                        }
                    }
                } break;
                
                case '>':
                {
                    token->kind = MM_Token_Greater;
                    
                    if (c[1] == '=')
                    {
                        token->kind = MM_Token_GreaterEquals;
                        offset     += 1;
                    }
                    
                    if (c[1] == '>')
                    {
                        token->kind = MM_Token_RightShift;
                        offset     += 1;
                        
                        if (c[2] == '=')
                        {
                            token->kind = MM_Token_RightShiftEquals;
                            offset     += 1;
                        }
                        
                        if (c[2] == '>')
                        {
                            token->kind = MM_Token_ArithmeticRightShift;
                            offset     += 1;
                            
                            if (c[3] == '=')
                            {
                                token->kind = MM_Token_ArithmeticRightShiftEquals;
                                offset     += 1;
                            }
                        }
                    }
                } break;
                
                case '.':
                {
                    token->kind = MM_Token_Period;
                    
                    if (c[1] == '{')
                    {
                        token->kind = MM_Token_OpenPeriodBrace;
                        offset     += 1;
                    }
                    
                    if (c[1] == '[')
                    {
                        token->kind = MM_Token_OpenPeriodBracket;
                        offset     += 1;
                    }
                    
                    if (c[1] == '(')
                    {
                        token->kind = MM_Token_OpenPeriodParen;
                        offset     += 1;
                    }
                } break;
                
                case '-':
                {
                    token->kind = MM_Token_Minus;
                    
                    if (c[1] == '=')
                    {
                        token->kind = MM_Token_MinusEquals;
                        offset     += 1;
                    }
                    
                    if (c[1] == '>')
                    {
                        token->kind = MM_Token_Arrow;
                        offset     += 1;
                    }
                    
                    if (c[1] == '-' && c[2] == '-')
                    {
                        token->kind = MM_Token_TripleMinus;
                        offset     += 2;
                    }
                } break;
                
                default:
                {
                    if (c[0] == '_' && !MM__IsAlphaNumericOrUnderscore(c[1]))
                    {
                        token->kind = MM_Token_Underscore;
                    }
                    
                    else if (c[0] == '_' || MM__IsAlpha(c[0]))
                    {
                        MM_String identifier = {
                            .data = string.data + offset - 1,
                            .size = 1,
                        };
                        
                        while (offset < string.size && MM__IsAlphaNumericOrUnderscore(string.data[offset]))
                        {
                            identifier.size += 1;
                            offset          += 1;
                        }
                        
                        token->identifier = MM_InternedString_FromString(identifier);
                    }
                    
                    else if (MM__IsNumeric(c[0]))
                    {
                        MM_umm base      = 10;
                        MM_bool is_float = false;
                        
                        MM_Big_Int integer          = {0};
                        MM_umm integer_digit_count  = 0;
                        MM_Big_Int fraction         = {0};
                        MM_umm fraction_digit_count = 0;
                        MM_Big_Int exponent         = {0};
                        
                        if (c[0] == '0')
                        {
                            if      (c[1] == 'x') base = 16;
                            else if (c[1] == 'h') base = 16, is_float = true;
                            else if (c[1] == 'b') base = 2;
                        }
                        
                        if (base != 10) offset += 1;
                        else            integer = MM_BigInt_FromU64(c[0] - '0'), digit_count += 1;
                        
                        MM_Big_Int big_base = MM_BigInt_FromU64(base);
                        MM_Big_Int big_10   = MM_BigInt_FromU64(10);
                        
                        while (offset < string.size)
                        {
                            MM_i8 digit = 0;
                            
                            MM_u8 ch = string.data[offset];
                            if      (ch >= '0' && ch <= '9') digit = ch - '0';
                            else if (ch >= 'a' && ch <= 'f') digit = (ch - 'a') + 10;
                            else if (ch >= 'A' && ch <= 'F') digit = (ch - 'A') + 10;
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
                                integer = MM_BigInt_Add(MM_BigInt_Mul(integer, big_base), MM_BigInt_FromU64(digit));
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
                            else
                            {
                                if (offset < string.size && string.data[offset] == '.')
                                {
                                    if (is_float)
                                    {
                                        //// ERROR: Hexfloats must be separated from periods
                                        encountered_errors = true;
                                    }
                                    else
                                    {
                                        offset += 1;
                                        
                                        while (offset < string.size)
                                        {
                                            MM_i8 digit = 0;
                                            
                                            MM_u8 ch = string.data[offset];
                                            if      (ch >= '0' && ch <= '9') digit = ch - '0';
                                            else if (ch == '_') continue;
                                            else                break;
                                            
                                            fraction = MM_BigInt_Add(MM_BigInt_Mul(fraction, big_10), MM_BigInt_FromU64(digit));
                                            fraction_digit_count += 1;
                                        }
                                        
                                        if (fraction_digit_count == 0)
                                        {
                                            //// ERROR: Missing digits after decimal point
                                            encountered_errors = true;
                                        }
                                    }
                                }
                                
                                if (!encountered_errors)
                                {
                                    if (offset < string.size && (string.data[offset] == 'e' || string.data[offset] == 'E'))
                                    {
                                        if (base != 10)
                                        {
                                            //// ERROR: scientific notation is only allowed for base 10
                                            encountered_errors = true;
                                        }
                                        else
                                        {
                                            offset += 1;
                                            
                                            MM_Big_Int exponent         = {0};
                                            MM_umm exponent_digit_count = 0;
                                            
                                            MM_imm sign = 1;
                                            if (offset < string.size && (string.data[offset] == '+' || string.data[offset] == '-'))
                                            {
                                                offset += 1;
                                                sign = (string.size[offset] == '+' ? 1 : -1);
                                            }
                                            
                                            while (offset < string.size)
                                            {
                                                MM_i8 digit = 0;
                                                
                                                MM_u8 ch = string.data[offset];
                                                if      (ch >= '0' && ch <= '9') digit = ch - '0';
                                                else if (ch == '_') continue;
                                                else                break;
                                                
                                                exponent = MM_BigInt_Add(MM_BigInt_Mul(exponent, big_10), MM_BigInt_FromU64(digit));
                                                exponent_digit_count += 1;
                                            }
                                            
                                            if (exponent_digit_count == 0)
                                            {
                                                //// ERROR: Missing digits of exponent after scientific notation suffix
                                                encountered_errors = true;
                                            }
                                            else
                                            {
                                                if (sign == -1) exponent = MM_BigInt_Neg(exponent);
                                            }
                                        }
                                    }
                                    else if (offset < string.size && MM__IsAlpha(string.data[offset]))
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
                                if (digit_count == 4)
                                {
                                    MM__NOT_IMPLEMENTED;
                                }
                                else if (digit_count == 8)
                                {
                                    MM__NOT_IMPLEMENTED;
                                }
                                else if (digit_count == 16)
                                {
                                    MM__NOT_IMPLEMENTED;
                                }
                                else
                                {
                                    //// ERROR: Invalid digit count for hex float. Hex floats must be either 4, 8 or 16 digits long
                                    encountered_errors = true;
                                }
                            }
                            else if (is_float)
                            {
                                token->kind = MM_Token_Int;
                                token->floating = MM_BigFloat_FromParts(integer, fraction, exponent);
                            }
                            else
                            {
                                token->kind = MM_Token_Int;
                                token->integer = MM_BigInt_Mul(integer, MM_BigInt_Pow10(exponent));
                            }
                        }
                    }
                    
                    else if (c[0] == '"')
                    {
                        MM_String string = {
                            .data = string.data + offset,
                            .size = 0,
                        };
                        
                        while (offset < string.size && string.size[offset] != '"')
                        {
                            if (string.data[offset] == '\\') offset += 1;
                            offset += 1;
                        }
                        
                        if (offset == string.size)
                        {
                            //// ERROR: Unterminated string literal
                            encountered_errors = true;
                        }
                        else
                        {
                            offset += 1;
                            
                            token->string = string;
                        }
                    }
                    
                    else if (c[0] == '\'')
                    {
                        token->kind = MM_Token_Character;
                        
                        MM_umm terminator_index = 2;
                        
                        if (c[1] != '\\')
                        {
                            token->character = c[1];
                            
                            if (c[1] == '\'')
                            {
                                //// ERROR: Missing character in character litteral
                                encountered_errors = true;
                            }
                        }
                        else
                        {
                            switch (c[2])
                            {
                                case 'a':  token->character = 0x07; break;
                                case 'b':  token->character = 0x08; break;
                                case 'e':  token->character = 0x1B; break;
                                case 'f':  token->character = 0x0C; break;
                                case 'n':  token->character = 0x0A; break;
                                case 'r':  token->character = 0x0D; break;
                                case 't':  token->character = 0x09; break;
                                case 'v':  token->character = 0x0B; break;
                                case '"':  token->character = 0x22; break;
                                case '\\': token->character = 0x5C; break;
                                case '\'': token->character = 0x27; break;
                                
                                default:
                                {
                                    //// ERROR: Invalid character escape sequence
                                    encountered_errors = true;
                                } break;
                            }
                            
                            terminator_index = 3;
                        }
                        
                        if (c[terminator_index] != '\'')
                        {
                            //// ERROR: Missing terminating ' character in character litteral
                            encountered_errors = true;
                        }
                    }
                    else
                    {
                        //// ERROR: Unknown symbol
                        encountered_errors = true;
                    }
                } break;
            }
        }
        MM__NOT_IMPLEMENTED;
    }
    
    return !encountered_errors;
}