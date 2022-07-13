// NOTE: Token kinds are organized into blocks of max 16 in size. This is to get checking for binary operators and their
//       precedence for free, as well as mapping binary assignment tokens to their operators

#define MM_TOKEN_KIND_BLOCK_SIZE 16
#define MM_TOKEN_BLOCK_INDEX_FROM_KIND(kind) ((kind) / MM_TOKEN_KIND_BLOCK_SIZE)
#define MM_TOKEN_BLOCK_OFFSET_FROM_KIND(kind) ((kind) / MM_TOKEN_KIND_BLOCK_SIZE)
#define MM_TOKEN_BINARY_ASSIGNMENT_TO_BINARY_KIND(kind) ((MM_TOKEN_BLOCK_INDEX_FROM_KIND(kind) + 5)*MM_TOKEN_KIND_BLOCK_SIZE + MM_TOKEN_BLOCK_OFFSET_FROM_KIND(kind))
#define MM_TOKEN_BINARY_TO_BINARY_ASSIGNMENT_KIND(kind) ((MM_TOKEN_BLOCK_INDEX_FROM_KIND(kind) - 5)*MM_TOKEN_KIND_BLOCK_SIZE + MM_TOKEN_BLOCK_OFFSET_FROM_KIND(kind))

enum MM_TOKEN_KIND
{
    MM_Token_Invalid = 0,
    MM_Token_EndOfStream,
    
    MM_Token_Comment,
    MM_Token_Keyword,
    MM_Token_Builtin,
    MM_Token_Identifier,
    MM_Token_BlankIdentifier,
    MM_Token_Int,
    MM_Token_Float,
    MM_Token_String,
    MM_Token_Codepoint,
    
    MM_Token_FirstAssignment,
    MM_Token_Equals = MM_Token_FirstAssignment,
    
    MM_Token_FirstBinaryAssignment = 1*16,
    MM_Token_FirstMulLevelAssignment = MM_Token_FirstAssignment,
    MM_Token_StarEquals = MM_Token_FirstMulLevelAssignment,
    MM_Token_SlashEquals,
    MM_Token_PercentEquals,
    MM_Token_AndEquals,
    MM_Token_LessLessEquals,
    MM_Token_GreaterGreaterEquals,
    MM_Token_TripleGreaterEquals,
    MM_Token_LastMulLevelAssignment = MM_Token_GreaterGreaterEquals,
    
    MM_Token_FirstAddLevelAssignment = 2*16,
    MM_Token_TildeEquals = MM_Token_FirstAddLevelAssignment,
    MM_Token_OrEquals,
    MM_Token_MinusEquals,
    MM_Token_PlusEquals,
    MM_Token_LastAddLevelAssignment = MM_Token_PlusEquals,
    
    MM_Token_AndAndEquals = 4*16,
    
    MM_Token_OrOrEquals = 5*16,
    MM_Token_LastBinaryAssignment = MM_Token_OrOrEquals,
    MM_Token_LastAssignment = MM_Token_LastBinaryAssignment,
    
    MM_Token_FirstBinary = 6*16,
    MM_Token_FirstMulLevel = MM_Token_FirstBinary,
    MM_Token_Star = MM_Token_FirstMulLevel,
    MM_Token_Slash,
    MM_Token_Percent,
    MM_Token_And,
    MM_Token_LessLess,
    MM_Token_GreaterGreater,
    MM_Token_TripleGreater,
    MM_Token_LastMulLevel = MM_Token_TripleGreater,
    
    MM_Token_FirstAddLevel = 7*16,
    MM_Token_Tilde = MM_Token_FirstAddLevel,
    MM_Token_Or,
    MM_Token_Minus,
    MM_Token_Plus,
    MM_Token_LastAddLevel = MM_Token_Plus,
    
    MM_Token_FirstCmpLevel = 8*16,
    MM_Token_Greater = MM_Token_FirstCmpLevel,
    MM_Token_GreaterEquals,
    MM_Token_Less,
    MM_Token_LessEquals,
    MM_Token_EqualsEquals,
    MM_Token_BangEquals,
    MM_Token_LastCmpLevel = MM_Token_BangEquals,
    
    MM_Token_AndAnd = 9*16,
    
    MM_Token_OrOr = 10*16,
    MM_Token_LastBinary = MM_Token_OrOr,
    
    MM_Token_Bang,
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
    
    MM_TOKEN_KIND_MAX,
};

MM_STATIC_ASSERT(MM_Token_FirstAssignment < MM_Token_FirstBinaryAssignment);
MM_STATIC_ASSERT(MM_TOKEN_KIND_MAX <= MM_U8_MAX);

enum MM_TOKEN_COMMENT_SUB_KIND
{
    MM_TokenCommentSub_SingleLine = (MM_u16)MM_Token_Comment << 8,
    MM_TokenCommentSub_MultiLine
};

enum MM_TOKEN_KEYWORD_SUB_KIND
{
    MM_TokenKeywordSub_Include = (MM_u16)MM_Token_Keyword << 8,
    MM_TokenKeywordSub_Proc,
    MM_TokenKeywordSub_ProcSet,
    MM_TokenKeywordSub_Struct,
    MM_TokenKeywordSub_Union,
    MM_TokenKeywordSub_Enum,
    MM_TokenKeywordSub_True,
    MM_TokenKeywordSub_False,
    MM_TokenKeywordSub_As,
    MM_TokenKeywordSub_If,
    MM_TokenKeywordSub_When,
    MM_TokenKeywordSub_Else,
    MM_TokenKeywordSub_While,
    MM_TokenKeywordSub_Break,
    MM_TokenKeywordSub_Continue,
    MM_TokenKeywordSub_Using,
    MM_TokenKeywordSub_Defer,
    MM_TokenKeywordSub_Return,
    MM_TokenKeywordSub_This,
    MM_TokenKeywordSub_Inf,
    MM_TokenKeywordSub_Qnan,
    MM_TokenKeywordSub_Snan,
};

enum MM_TOKEN_BUILTIN_SUB_KIND
{
    MM_TokenBuiltinSub_Cast = (MM_u16)MM_Token_Builtin << 8,
    MM_TokenBuiltinSub_Transmute,
    MM_TokenBuiltinSub_Sizeof,
    MM_TokenBuiltinSub_Alignof,
    MM_TokenBuiltinSub_Offsetof,
    MM_TokenBuiltinSub_Typeof,
    MM_TokenBuiltinSub_Shadowed,
    MM_TokenBuiltinSub_Min,
    MM_TokenBuiltinSub_Max,
    MM_TokenBuiltinSub_Len,
};

enum MM_TOKEN_INT_SUB_KIND
{
    MM_TokenIntSub_None = (MM_u16)MM_Token_Int << 8,
    MM_TokenIntSub_Binary,
    MM_TokenIntSub_Decimal,
    MM_TokenIntSub_Hexadecimal,
};

enum MM_TOKEN_FLOAT_SUB_KIND
{
    MM_TokenFloatSub_None = (MM_u16)MM_Token_Float << 8,
    MM_TokenFloatSub_Hex16,
    MM_TokenFloatSub_Hex32,
    MM_TokenFloatSub_Hex64,
};

typedef struct MM_Token
{
    MM_u8* data;
    MM_u32 size;
    MM_u32 line;
    MM_u32 column;
    union
    {
        struct
        {
            MM_u8 _;
            MM_u8 kind;
        };
        
        MM_u16 sub_kind;
    };
} MM_Token;

typedef struct MM_Lexer
{
    MM_Token current_token;
    MM_String string;
    MM_u32 offset;
    MM_u32 offset_to_line;
    MM_u32 line;
} MM_Lexer;

MM_Lexer
MM_Lexer_Init(MM_String string)
{
    MM_Lexer lexer = (MM_Lexer){
        .current_token  = {},
        .string         = string,
        .offset         = 0,
        .offset_to_line = 0,
        .line           = 1,
    };
    
    MM_Lexer_NextToken(&lexer);
    
    return lexer;
}

MM_Token
MM_Lexer_CurrentToken(MM_Lexer* lexer)
{
    return lexer->current_token;
}

MM_Token
MM_Lexer_NextToken(MM_Lexer* lexer)
{
    while (lexer->offset < lexer->string.size)
    {
        char c = lexer->string.data[lexer->offset];
        
        if (c == ' '  || c == '\t' ||
            c == '\v' || c == '\f' ||
            c == '\r')
        {
            ++lexer->offset;
        }
        else if (c == '\n')
        {
            ++lexer->offset;
            ++lexer->line;
            
            lexer->offset_to_line = lexer->offset;
        }
        else break;
    }
    
    lexer->current_token = (MM_Token){
        .data   = lexer->string.data + lexer->offset,
        .line   = lexer->line,
        .column = lexer->offset - lexer->offset_to_line,
        
        // NOTE: assume an erroneous token of zero length, this will be updated to match the actual token later
        .kind     = MM_Token_Invalid,
        .size     = 0,
    };
    
    if (lexer->offset >= lexer->string.size) lexer->current_token.kind = MM_Token_EndOfStream;
    else
    {
        char c[4] = {
            [0] = lexer->string.data[lexer->offset],
            [1] = (lexer->offset + 1 < lexer->string.size ? lexer->string.data[lexer->offset + 1] : 0),
            [2] = (lexer->offset + 2 < lexer->string.size ? lexer->string.data[lexer->offset + 2] : 0),
            [3] = (lexer->offset + 3 < lexer->string.size ? lexer->string.data[lexer->offset + 3] : 0),
        };
        
        if (c[0] == '/' && c[1] == '/')
        {
            lexer->current_token.sub_kind = MM_TokenCommentSub_SingleLine;
            
            lexer->offset += 2;
            
            while (lexer->offset < lexer->string.size && lexer->string.data[lexer->offset] != '\n') ++lexer->offset;
        }
        else if (c[0] == '/' && c[1] == '*')
        {
            lexer->current_token.sub_kind = MM_TokenCommentSub_MultiLine;
            
            lexer->offset += 2;
            
            MM_umm depth = 1;
            
            while (depth != 0)
            {
                if (lexer->offset + 1 >= lexer->string.size)
                {
                    //// ERROR: Unterminated multi line comment
                    MM_NOT_IMPLEMENTED;
                }
                else
                {
                    char c0 = lexer->string.data[lexer->offset + 0];
                    char c1 = lexer->string.data[lexer->offset + 1];
                    
                    if (c0 == '/' && c1 == '*')
                    {
                        depth         += 1;
                        lexer->offset += 2;
                    }
                    else if (c0 == '*' && c1 == '/')
                    {
                        depth         -= 1;
                        lexer->offset += 2;
                    }
                    else ++lexer->offset;
                }
            }
        }
        else
        {
            // NOTE: Assume single symbol token
            lexer->offset += 1;
            
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
                            // TODO: Replace with hash table lookup
                            if      (MM_String_Match(identifier, "include"))   lexer->current_token.sub_kind = MM_TokenKeywordSub_Include;
                            else if (MM_String_Match(identifier, "proc"))      lexer->current_token.sub_kind = MM_TokenKeywordSub_Proc;
                            else if (MM_String_Match(identifier, "proc_set"))  lexer->current_token.sub_kind = MM_TokenKeywordSub_ProcSet;
                            else if (MM_String_Match(identifier, "struct"))    lexer->current_token.sub_kind = MM_TokenKeywordSub_Struct;
                            else if (MM_String_Match(identifier, "union"))     lexer->current_token.sub_kind = MM_TokenKeywordSub_Union;
                            else if (MM_String_Match(identifier, "enum"))      lexer->current_token.sub_kind = MM_TokenKeywordSub_Enum;
                            else if (MM_String_Match(identifier, "true"))      lexer->current_token.sub_kind = MM_TokenKeywordSub_True;
                            else if (MM_String_Match(identifier, "false"))     lexer->current_token.sub_kind = MM_TokenKeywordSub_False;
                            else if (MM_String_Match(identifier, "as"))        lexer->current_token.sub_kind = MM_TokenKeywordSub_As;
                            else if (MM_String_Match(identifier, "if"))        lexer->current_token.sub_kind = MM_TokenKeywordSub_If;
                            else if (MM_String_Match(identifier, "when"))      lexer->current_token.sub_kind = MM_TokenKeywordSub_When;
                            else if (MM_String_Match(identifier, "else"))      lexer->current_token.sub_kind = MM_TokenKeywordSub_Else;
                            else if (MM_String_Match(identifier, "while"))     lexer->current_token.sub_kind = MM_TokenKeywordSub_While;
                            else if (MM_String_Match(identifier, "break"))     lexer->current_token.sub_kind = MM_TokenKeywordSub_Break;
                            else if (MM_String_Match(identifier, "continue"))  lexer->current_token.sub_kind = MM_TokenKeywordSub_Continue;
                            else if (MM_String_Match(identifier, "using"))     lexer->current_token.sub_kind = MM_TokenKeywordSub_Using;
                            else if (MM_String_Match(identifier, "defer"))     lexer->current_token.sub_kind = MM_TokenKeywordSub_Defer;
                            else if (MM_String_Match(identifier, "return"))    lexer->current_token.sub_kind = MM_TokenKeywordSub_Return;
                            else if (MM_String_Match(identifier, "this"))      lexer->current_token.sub_kind = MM_TokenKeywordSub_This;
                            else if (MM_String_Match(identifier, "inf"))       lexer->current_token.sub_kind = MM_TokenKeywordSub_Inf;
                            else if (MM_String_Match(identifier, "qnan"))      lexer->current_token.sub_kind = MM_TokenKeywordSub_Qnan;
                            else if (MM_String_Match(identifier, "snan"))      lexer->current_token.sub_kind = MM_TokenKeywordSub_Snan;
                            else if (MM_String_Match(identifier, "cast"))      lexer->current_token.sub_kind = MM_TokenBuiltinSub_Cast;
                            else if (MM_String_Match(identifier, "transmute")) lexer->current_token.sub_kind = MM_TokenBuiltinSub_Transmute;
                            else if (MM_String_Match(identifier, "sizeof"))    lexer->current_token.sub_kind = MM_TokenBuiltinSub_Sizeof;
                            else if (MM_String_Match(identifier, "alignof"))   lexer->current_token.sub_kind = MM_TokenBuiltinSub_Alignof;
                            else if (MM_String_Match(identifier, "offsetof"))  lexer->current_token.sub_kind = MM_TokenBuiltinSub_Offsetof;
                            else if (MM_String_Match(identifier, "typeof"))    lexer->current_token.sub_kind = MM_TokenBuiltinSub_Typeof;
                            else if (MM_String_Match(identifier, "shadowed"))  lexer->current_token.sub_kind = MM_TokenBuiltinSub_Shadowed;
                            else if (MM_String_Match(identifier, "min"))       lexer->current_token.sub_kind = MM_TokenBuiltinSub_Min;
                            else if (MM_String_Match(identifier, "max"))       lexer->current_token.sub_kind = MM_TokenBuiltinSub_Max;
                            else if (MM_String_Match(identifier, "len"))       lexer->current_token.sub_kind = MM_TokenBuiltinSub_Len;
                        }
                    }
                    else if (c[0] >= '0' && c[1] <= '9')
                    {
                        MM_umm digit_count  = 1;
                        MM_u16 assumed_kind = MM_TokenIntSub_None;
                        MM_umm base         = 10;
                        MM_bool has_exp     = MM_false;
                        
                        if (c[0] == '0')
                        {
                            if      (c[1] == 'b') lexer->offset += 1, digit_count = 0, assumed_kind = MM_TokenIntSub_Binary,      base = 2;
                            else if (c[1] == 'd') lexer->offset += 1, digit_count = 0, assumed_kind = MM_TokenIntSub_Decimal,     base = 10;
                            else if (c[1] == 'x') lexer->offset += 1, digit_count = 0, assumed_kind = MM_TokenIntSub_Hexadecimal, base = 16;
                            else if (c[1] == 'h') lexer->offset += 1, digit_count = 0, assumed_kind = MM_TokenFloatSub_Hex32,     base = 16;
                        }
                        
                        lexer->offset += 1;
                        
                        while (lexer->offset < lexer->string.size)
                        {
                            MM_umm digit;
                            char c0 = lexer->string.data[lexer->offset];
                            
                            if (c0 == '_')
                            {
                                lexer->offset += 1;
                                continue;
                            }
                            else if (c0 == '.' && assumed_kind == MM_TokenIntSub_None)
                            {
                                lexer->offset += 1;
                                assumed_kind = MM_TokenFloatSub_None;
                                
                                if (lexer->offset < lexer->string.size) continue;
                                else
                                {
                                    //// ERROR: Missing fractional part of floating point literal
                                    MM_NOT_IMPLEMENTED;
                                    break;
                                }
                            }
                            else if (c0 == 'e' && assumed_kind == MM_TokenFloatSub_None && !has_exp)
                            {
                                lexer->offset += 1;
                                has_exp = MM_true;
                                
                                if (lexer->offset < lexer->string.size && (lexer->string.data[lexer->offset] == '+' ||
                                                                           lexer->string.data[lexer->offset] == '-'))
                                {
                                    lexer->offset += 1;
                                }
                                
                                if (lexer->offset < lexer->string) continue;
                                else
                                {
                                    //// ERROR: Missing exponent
                                    MM_NOT_IMPLEMENTED;
                                    break;
                                }
                            }
                            else if (c0 >= '0' && c0 <= '9') digit = c0 & 0xF;
                            else if (c0 >= 'A' && c0 <= 'Z') digit = (c0 & 0x1F) + 9;
                            else if (c0 >= 'a' && c0 <= 'z') digit = (c0 & 0x1F) + 35;
                            else break;
                            
                            if (digit >= base)
                            {
                                //// ERROR: digit is too large for the specified base
                                MM_NOT_IMPLEMENTED;
                                break;
                            }
                            else
                            {
                                lexer->offset += 1;
                                digit_count   += 1;
                                continue;
                            }
                        }
                        
                        if (assumed_kind == MM_TokenFloatSub_Hex32)
                        {
                            if      (digit_count == 4)  lexer->current_token.sub_kind =  = MM_TokenFloatSub_Hex16;
                            else if (digit_count == 8)  lexer->current_token.sub_kind =  = MM_TokenFloatSub_Hex32;
                            else if (digit_count == 16) lexer->current_token.sub_kind =  = MM_TokenFloatSub_Hex64;
                            else
                            {
                                //// ERROR: Invalid digit count in hexadecimal floating point literal
                                MM_NOT_IMPLEMENTED;
                            }
                        }
                        else lexer->current_token.sub_kind = assumed_kind;
                    }
                    else if (c[0] == '"' || c[0] == '\'')
                    {
                        char terminator;
                        
                        if (c[0] == '"')
                        {
                            lexer->current_token.kind = MM_Token_String;
                            terminator = '"';
                        }
                        else
                        {
                            lexer->current_token.kind = MM_Token_Codepoint;
                            terminator = '\'';
                        }
                        
                        while (MM_true)
                        {
                            if (lexer->offset >= lexer->string.size)
                            {
                                //// ERROR: Unterminated string literal
                                MM_NOT_IMPLEMENTED;
                            }
                            else
                            {
                                char c0 = lexer->string.data[lexer->offset];
                                
                                if (c0 == terminator)
                                {
                                    ++lexer->offset;
                                    break;
                                }
                                else if (c0 == '\\')
                                {
                                    if (lexer->offset + 1 < lexer->string.size) lexer->offset += 2;
                                    else
                                    {
                                        //// ERROR: Backslash with no escape sequence
                                        MM_NOT_IMPLEMENTED;
                                    }
                                }
                                else ++lexer->offset;
                            }
                        }
                    }
                    else
                    {
                        //// ERROR: Unknown symbol
                        MM_NOT_IMPLEMENTED;
                    }
                } break;
            }
        }
        
        lexer->current_token.size = (MM_u32)((lexer->string.data + lexer->offset) - lexer->current_token.data);
    }
    
    return lexer->current_token;
}

MM_umm
MM_Lexer_NextTokens(MM_Lexer* lexer, MM_Token* buffer, MM_umm buffer_size)
{
    MM_umm i = 0;
    for (; i < buffer_size; ++i)
    {
        buffer[i] = MM_Lexer_NextToken(lexer);
        
        if (buffer[i].kind == MM_Token_Invalid || buffer[i].kind == MM_Token_EndOfStream) break;
    }
    
    return i;
}