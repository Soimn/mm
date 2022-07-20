#define MM_TOKEN_KIND_FIRST(group) ((MM_u16)(group) << 8)
#define MM_TOKEN_GROUP_FROM_KIND(kind) ((kind) >> 8)
#define MM_TOKEN_BINARY_OPERATOR_BLOCK_SIZE 16
#define MM_TOKEN_BINARY_OPERATOR_BLOCK(kind) ((MM_u8)(kind) / MM_TOKEN_BINARY_OPERATOR_BLOCK_SIZE - 6)
#define MM_TOKEN_BINARY_OPERATOR_INDEX(kind) ((MM_u8)(kind) % MM_TOKEN_BINARY_OPERATOR_BLOCK_SIZE)

enum MM_TOKEN_GROUP_KIND
{
    MM_TokenGroup_None = 0,
    MM_TokenGroup_Integer,
    MM_TokenGroup_Float,
    MM_TokenGroup_Operator,
    MM_TokenGroup_Comment,
    MM_TokenGroup_Keyword,
    MM_TokenGroup_Builtin,
};

enum MM_TOKEN_KIND
{
    MM_Token_Invalid = MM_TOKEN_KIND_FIRST(MM_TokenGroup_None),
    MM_Token_EndOfStream,
    
    MM_Token_Identifier,
    MM_Token_BlankIdentifier,
    MM_Token_String,
    MM_Token_Codepoint,
    
    MM_Token_Int = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Integer),
    MM_Token_BinaryInt,
    MM_Token_DecimalInt,
    MM_Token_HexInt,
    
    MM_Token_Float = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Float),
    MM_Token_HexFloat16,
    MM_Token_HexFloat32,
    MM_Token_HexFloat64,
    
    MM_Token_FirstAssignment = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Operator),
    MM_Token_Equals = MM_Token_FirstAssignment,
    
    MM_Token_FirstBinaryAssignment = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Operator) + 1*16,
    MM_Token_FirstMulLevelAssignment = MM_Token_FirstAssignment,
    MM_Token_StarEquals = MM_Token_FirstMulLevelAssignment,
    MM_Token_SlashEquals,
    MM_Token_PercentEquals,
    MM_Token_AndEquals,
    MM_Token_LessLessEquals,
    MM_Token_GreaterGreaterEquals,
    MM_Token_TripleGreaterEquals,
    MM_Token_LastMulLevelAssignment = MM_Token_GreaterGreaterEquals,
    
    MM_Token_FirstAddLevelAssignment = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Operator) + 2*16,
    MM_Token_TildeEquals = MM_Token_FirstAddLevelAssignment,
    MM_Token_OrEquals,
    MM_Token_MinusEquals,
    MM_Token_PlusEquals,
    MM_Token_LastAddLevelAssignment = MM_Token_PlusEquals,
    
    MM_Token_AndAndEquals = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Operator) + 4*16,
    
    MM_Token_OrOrEquals = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Operator) + 5*16,
    MM_Token_LastBinaryAssignment = MM_Token_OrOrEquals,
    MM_Token_LastAssignment = MM_Token_LastBinaryAssignment,
    
    MM_Token_FirstBinary = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Operator) + 6*16,
    MM_Token_FirstMulLevel = MM_Token_FirstBinary,
    MM_Token_Star = MM_Token_FirstMulLevel,
    MM_Token_Slash,
    MM_Token_Percent,
    MM_Token_And,
    MM_Token_LessLess,
    MM_Token_GreaterGreater,
    MM_Token_TripleGreater,
    MM_Token_LastMulLevel = MM_Token_TripleGreater,
    
    MM_Token_FirstAddLevel = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Operator) + 7*16,
    MM_Token_Plus  = MM_Token_FirstAddLevel,
    MM_Token_Minus,
    MM_Token_Or,
    MM_Token_Tilde,
    MM_Token_LastAddLevel = MM_Token_Tilde,
    
    MM_Token_FirstCmpLevel = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Operator) + 8*16,
    MM_Token_Greater = MM_Token_FirstCmpLevel,
    MM_Token_GreaterEquals,
    MM_Token_Less,
    MM_Token_LessEquals,
    MM_Token_EqualsEquals,
    MM_Token_BangEquals,
    MM_Token_LastCmpLevel = MM_Token_BangEquals,
    
    MM_Token_AndAnd = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Operator) + 9*16,
    
    MM_Token_OrOr = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Operator) + 10*16,
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
    
    MM_Token_SingleLineComment = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Comment),
    MM_Token_MultiLineComment,
    
    MM_Token_Include = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Keyword),
    MM_Token_Proc,
    MM_Token_ProcSet,
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
    
    MM_Token_Cast = MM_TOKEN_KIND_FIRST(MM_TokenGroup_Builtin),
    MM_Token_Transmute,
    MM_Token_Sizeof,
    MM_Token_Alignof,
    MM_Token_Offsetof,
    MM_Token_Typeof,
    MM_Token_Shadowed,
    MM_Token_Min,
    MM_Token_Max,
    MM_Token_Len,
};

MM_STATIC_ASSERT(MM_TOKEN_BINARY_OPERATOR_BLOCK(MM_Token_FirstBinary) == 0);

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
            MM_u8 kind_index;
            MM_u8 kind_group;
        };
        MM_u16 kind;
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
            lexer->current_token.kind = MM_Token_SingleLineComment;
            
            lexer->offset += 2;
            
            while (lexer->offset < lexer->string.size && lexer->string.data[lexer->offset] != '\n') ++lexer->offset;
        }
        else if (c[0] == '/' && c[1] == '*')
        {
            lexer->current_token.kind = MM_Token_MultiLineComment;
            
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
                case '(': lexer->current_token.kind = MM_Token_OpenParen;    break;
                case ')': lexer->current_token.kind = MM_Token_CloseParen;   break;
                case '[': lexer->current_token.kind = MM_Token_OpenBracket;  break;
                case ']': lexer->current_token.kind = MM_Token_CloseBracket; break;
                case '{': lexer->current_token.kind = MM_Token_OpenBrace;    break;
                case '}': lexer->current_token.kind = MM_Token_CloseBrace;   break;
                case ':': lexer->current_token.kind = MM_Token_Colon;        break;
                case ',': lexer->current_token.kind = MM_Token_Comma;        break;
                case ';': lexer->current_token.kind = MM_Token_Semicolon;    break;
                case '?': lexer->current_token.kind = MM_Token_QuestionMark; break;
                case '^': lexer->current_token.kind = MM_Token_Hat;          break;
                
                case '*': lexer->current_token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_StarEquals    : MM_Token_Star);    break;
                case '/': lexer->current_token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_SlashEquals   : MM_Token_Slash);   break;
                case '%': lexer->current_token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_PercentEquals : MM_Token_Percent); break;
                case '+': lexer->current_token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_PlusEquals    : MM_Token_Plus);    break;
                case '=': lexer->current_token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_EqualsEquals  : MM_Token_Equals);  break;
                case '!': lexer->current_token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_BangEquals    : MM_Token_Bang);    break;
                case '~': lexer->current_token.kind = (c[1] == '=' ? ++lexer->offset, MM_Token_TildeEquals   : MM_Token_Tilde);   break;
                
                // NOTE: I wrote this in a sort of cmov fashion, which might be a bad idea if the compiler does not
                //       understand it, or disagrees with the decision, but it looks nice, and should hopefully not
                //       matter that much
                case '-':
                {
                    lexer->current_token.kind = MM_Token_Minus;
                    if (c[1] == '=')                lexer->offset += 1, lexer->current_token.kind = MM_Token_MinusEquals;
                    if (c[1] == '>')                lexer->offset += 1, lexer->current_token.kind = MM_Token_Arrow;
                    if (c[1] == '-' && c[2] == '-') lexer->offset += 2, lexer->current_token.kind = MM_Token_TripleMinus;
                } break;
                
                case '.':
                {
                    lexer->current_token.kind = MM_Token_Period;
                    if (c[1] == '{') ++lexer->offset, lexer->current_token.kind = MM_Token_PeriodOpenBrace;
                    if (c[1] == '[') ++lexer->offset, lexer->current_token.kind = MM_Token_PeriodOpenBracket;
                }
                
                case '>':
                {
                    lexer->current_token.kind = MM_Token_Greater;
                    if (c[1] == '=')                               lexer->offset += 1, lexer->current_token.kind = MM_Token_GreaterEquals;
                    if (c[1] == '>' && c[2] != '=')                lexer->offset += 1, lexer->current_token.kind = MM_Token_GreaterGreater;
                    if (c[1] == '>' && c[2] == '=')                lexer->offset += 2, lexer->current_token.kind = MM_Token_GreaterGreaterEquals;
                    if (c[1] == '>' && c[2] == '>' && c[3] != '=') lexer->offset += 2, lexer->current_token.kind = MM_Token_TripleGreater;
                    if (c[1] == '>' && c[2] == '>' && c[3] == '=') lexer->offset += 3, lexer->current_token.kind = MM_Token_TripleGreaterEquals;
                } break;
                
                case '<':
                {
                    lexer->current_token.kind = MM_Token_Less;
                    if (c[1] == '=')                lexer->offset += 1, lexer->current_token.kind = MM_Token_LessEquals;
                    if (c[1] == '<' && c[2] != '=') lexer->offset += 1, lexer->current_token.kind = MM_Token_LessLess;
                    if (c[1] == '<' && c[2] == '=') lexer->offset += 2, lexer->current_token.kind = MM_Token_LessLessEquals;
                } break;
                
                case '&':
                {
                    lexer->current_token.kind = MM_Token_And;
                    if (c[1] == '=')                lexer->offset += 1, lexer->current_token.kind = MM_Token_AndEquals;
                    if (c[1] == '&' && c[2] != '=') lexer->offset += 1, lexer->current_token.kind = MM_Token_AndAnd;
                    if (c[1] == '&' && c[2] == '=') lexer->offset += 2, lexer->current_token.kind = MM_Token_AndAndEquals;
                } break;
                
                case '|':
                {
                    lexer->current_token.kind = MM_Token_Or;
                    if (c[1] == '=')                lexer->offset += 1, lexer->current_token.kind = MM_Token_OrEquals;
                    if (c[1] == '|' && c[2] != '=') lexer->offset += 1, lexer->current_token.kind = MM_Token_OrOr;
                    if (c[1] == '|' && c[2] == '=') lexer->offset += 2, lexer->current_token.kind = MM_Token_OrOrEquals;
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
                            .data = lexer->string.data + ident_offset,
                            .size = lexer->offset - ident_offset,
                        };
                        
                        if (identifier.size == 1 && identifier.data[0] == '_') lexer->current_token.kind = MM_Token_BlankIdentifier;
                        else
                        {
                            // TODO: Replace with hash table lookup
#define MM_Match(S) MM_String_Match(identifier, MM_STRING(S))
                            if      (MM_Match("include"))   lexer->current_token.kind = MM_Token_Include;
                            else if (MM_Match("proc"))      lexer->current_token.kind = MM_Token_Proc;
                            else if (MM_Match("proc_set"))  lexer->current_token.kind = MM_Token_ProcSet;
                            else if (MM_Match("struct"))    lexer->current_token.kind = MM_Token_Struct;
                            else if (MM_Match("union"))     lexer->current_token.kind = MM_Token_Union;
                            else if (MM_Match("enum"))      lexer->current_token.kind = MM_Token_Enum;
                            else if (MM_Match("true"))      lexer->current_token.kind = MM_Token_True;
                            else if (MM_Match("false"))     lexer->current_token.kind = MM_Token_False;
                            else if (MM_Match("as"))        lexer->current_token.kind = MM_Token_As;
                            else if (MM_Match("if"))        lexer->current_token.kind = MM_Token_If;
                            else if (MM_Match("when"))      lexer->current_token.kind = MM_Token_When;
                            else if (MM_Match("else"))      lexer->current_token.kind = MM_Token_Else;
                            else if (MM_Match("while"))     lexer->current_token.kind = MM_Token_While;
                            else if (MM_Match("break"))     lexer->current_token.kind = MM_Token_Break;
                            else if (MM_Match("continue"))  lexer->current_token.kind = MM_Token_Continue;
                            else if (MM_Match("using"))     lexer->current_token.kind = MM_Token_Using;
                            else if (MM_Match("defer"))     lexer->current_token.kind = MM_Token_Defer;
                            else if (MM_Match("return"))    lexer->current_token.kind = MM_Token_Return;
                            else if (MM_Match("this"))      lexer->current_token.kind = MM_Token_This;
                            else if (MM_Match("inf"))       lexer->current_token.kind = MM_Token_Inf;
                            else if (MM_Match("qnan"))      lexer->current_token.kind = MM_Token_Qnan;
                            else if (MM_Match("snan"))      lexer->current_token.kind = MM_Token_Snan;
                            else if (MM_Match("cast"))      lexer->current_token.kind = MM_Token_Cast;
                            else if (MM_Match("transmute")) lexer->current_token.kind = MM_Token_Transmute;
                            else if (MM_Match("sizeof"))    lexer->current_token.kind = MM_Token_Sizeof;
                            else if (MM_Match("alignof"))   lexer->current_token.kind = MM_Token_Alignof;
                            else if (MM_Match("offsetof"))  lexer->current_token.kind = MM_Token_Offsetof;
                            else if (MM_Match("typeof"))    lexer->current_token.kind = MM_Token_Typeof;
                            else if (MM_Match("shadowed"))  lexer->current_token.kind = MM_Token_Shadowed;
                            else if (MM_Match("min"))       lexer->current_token.kind = MM_Token_Min;
                            else if (MM_Match("max"))       lexer->current_token.kind = MM_Token_Max;
                            else if (MM_Match("len"))       lexer->current_token.kind = MM_Token_Len;
#undef MM_Match
                        }
                    }
                    else if (c[0] >= '0' && c[1] <= '9')
                    {
                        MM_umm digit_count  = 1;
                        MM_u16 assumed_kind = MM_Token_Int;
                        MM_umm base         = 10;
                        MM_bool has_exp     = MM_false;
                        
                        if (c[0] == '0')
                        {
                            if      (c[1] == 'b') lexer->offset += 1, digit_count = 0, assumed_kind = MM_Token_BinaryInt,  base = 2;
                            else if (c[1] == 'd') lexer->offset += 1, digit_count = 0, assumed_kind = MM_Token_DecimalInt, base = 10;
                            else if (c[1] == 'x') lexer->offset += 1, digit_count = 0, assumed_kind = MM_Token_HexInt,     base = 16;
                            else if (c[1] == 'h') lexer->offset += 1, digit_count = 0, assumed_kind = MM_Token_HexFloat32, base = 16;
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
                            else if (c0 == '.' && assumed_kind == MM_Token_Int)
                            {
                                lexer->offset += 1;
                                assumed_kind = MM_Token_Float;
                                
                                if (lexer->offset < lexer->string.size) continue;
                                else
                                {
                                    //// ERROR: Missing fractional part of floating point literal
                                    MM_NOT_IMPLEMENTED;
                                    break;
                                }
                            }
                            else if (c0 == 'e' && assumed_kind == MM_Token_Float && !has_exp)
                            {
                                lexer->offset += 1;
                                has_exp = MM_true;
                                
                                if (lexer->offset < lexer->string.size && (lexer->string.data[lexer->offset] == '+' ||
                                                                           lexer->string.data[lexer->offset] == '-'))
                                {
                                    lexer->offset += 1;
                                }
                                
                                if (lexer->offset < lexer->string.size) continue;
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
                        
                        if (assumed_kind == MM_Token_HexFloat32)
                        {
                            if      (digit_count == 4)  lexer->current_token.kind = MM_Token_HexFloat16;
                            else if (digit_count == 8)  lexer->current_token.kind = MM_Token_HexFloat32;
                            else if (digit_count == 16) lexer->current_token.kind = MM_Token_HexFloat64;
                            else
                            {
                                //// ERROR: Invalid digit count in hexadecimal floating point literal
                                MM_NOT_IMPLEMENTED;
                            }
                        }
                        else lexer->current_token.kind = assumed_kind;
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
                                //// ERROR: Unterminated string/char literal
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

MM_Soft_Int
MM_Token_ParseInt(MM_Token token)
{
    MM_ASSERT(token.kind_group == MM_TokenGroup_Integer);
    
    MM_Soft_Int result = 0;
    
    if (token.kind == MM_Token_BinaryInt)
    {
        for (MM_umm i = 2; i < token.size; ++i)
        {
            char c = token.data[i];
            
            if (c != '_')
            {
                MM_ASSERT(c >= '0' && c <= '1');
                result <<= 1;
                result  |= c & 0xF;
            }
        }
    }
    else if (token.kind == MM_Token_HexInt)
    {
        for (MM_umm i = 2; i < token.size; ++i)
        {
            char c = token.data[i];
            
            if (c != '_')
            {
                MM_ASSERT(c >= '0' && c <= '9' || c >= 'A' && c <= 'F');
                result <<= 4;
                if (c < 'A') result |= c & 0xF;
                else         result |= (c & 0x1F) + 9;
            }
        }
    }
    else
    {
        MM_ASSERT(token.kind == MM_Token_Int || token.kind == MM_Token_DecimalInt);
        
        for (MM_umm i = (token.kind == MM_Token_DecimalInt ? 2 : 0); i < token.size; ++i)
        {
            char c = token.data[i];
            
            if (c != '_')
            {
                MM_ASSERT(c >= '0' && c <= '9');
                result *= 10;
                result += c & 0xF;
            }
        }
    }
    
    return result;
}

MM_Soft_Float
MM_Token_ParseFloat(MM_Token token)
{
    MM_ASSERT(token.kind_group == MM_TokenGroup_Float);
    
    MM_Soft_Float result = {0};
    
    if (token.kind == MM_Token_Float)
    {
        MM_NOT_IMPLEMENTED;
    }
    else
    {
        MM_u64 bits = 0;
        
        for (MM_umm i = 2; i < token.size; ++i)
        {
            char c = token.data[i];
            
            if (c != '_')
            {
                MM_ASSERT(c >= '0' && c <= '9' || c >= 'A' && c <= 'F');
                bits <<= 4;
                if (c < 'A') bits |= c & 0xF;
                else         bits |= (c & 0x1F) + 9;
            }
        }
        
        if (token.kind == MM_Token_HexFloat16)
        {
            result = MM_F64_FromF16((MM_f16)bits);
        }
        else if (token.kind == MM_Token_HexFloat32)
        {
            MM_f32 f32;
            MM_Copy(&bits, &f32, sizeof(MM_f32));
            result = f32;
        }
        else
        {
            MM_ASSERT(token.kind == MM_Token_HexFloat64);
            MM_f64 f64;
            MM_Copy(&bits, &f64, sizeof(MM_f64));
            result = f64;
        }
    }
    
    return result;
}

typedef enum MM_TOKEN_PARSE_ERROR_CODE
{
    MM_TokenParseError_None = 0,
    MM_TokenParseError_NoCodepointInCodepointLiteral,
    MM_TokenParseError_MissingDigitsInByteEscSeq,
    MM_TokenParseError_MissingDigitsInShortCodepointEscSeq,
    MM_TokenParseError_MissingDigitsInCodepointEscSeq,
    MM_TokenParseError_InvalidDigitInByteEscSeq,
    MM_TokenParseError_InvalidDigitInShortCodepointEscSeq,
    MM_TokenParseError_InvalidDigitInCodepointEscSeq,
    MM_TokenParseError_InvalidFirstByteOfUTF8,
    MM_TokenParseError_Missing1TrailingByteUTF8,
    MM_TokenParseError_Missing2TrailingByteUTF8,
    MM_TokenParseError_Missing3TrailingByteUTF8,
    MM_TokenParseError_MoreThanOneCodepoint,
    MM_TokenParseError_CodepointOutOfRange,
} MM_TOKEN_PARSE_ERROR_CODE;

typedef struct MM_Token_Parse_Error
{
    MM_TOKEN_PARSE_ERROR_CODE code;
    MM_u32 index;
} MM_Token_Parse_Error;

MM_Token_Parse_Error
MM_Token__ParseEscSeq(MM_Token token, MM_u32* result, MM_umm* index)
{
    MM_umm codepoint = 0;
    MM_umm i         = *index;
    
    char c = token.data[i];
    if      (c == 'a')  i += 1, codepoint = '\a';
    else if (c == 'b')  i += 1, codepoint = '\b';
    else if (c == 'f')  i += 1, codepoint = '\f';
    else if (c == 'n')  i += 1, codepoint = '\n';
    else if (c == 'r')  i += 1, codepoint = '\r';
    else if (c == 't')  i += 1, codepoint = '\t';
    else if (c == 'v')  i += 1, codepoint = '\v';
    else if (c == '\\') i += 1, codepoint = '\\';
    else if (c == '\'') i += 1, codepoint = '\'';
    else if (c == '"')  i += 1, codepoint = '"';
    else if (c == 'x' || c == 'u' || c == 'U')
    {
        MM_umm expected_digit_count = (c == 'x' ? 2 :
                                       c == 'u' ? 4 : 6);
        
        i += 1;
        
        for (MM_umm j = 0; j < expected_digit_count; ++j)
        {
            if (i + j >= token.size - 1)
            {
                //// ERROR
                MM_umm edc = expected_digit_count;
                if      (edc == 2) return (MM_Token_Parse_Error){ .code = MM_TokenParseError_MissingDigitsInByteEscSeq,           .index = i + j };
                else if (edc == 4) return (MM_Token_Parse_Error){ .code = MM_TokenParseError_MissingDigitsInShortCodepointEscSeq, .index = i + j };
                else               return (MM_Token_Parse_Error){ .code = MM_TokenParseError_MissingDigitsInCodepointEscSeq,      .index = i + j };
            }
            else
            {
                codepoint <<= 4;
                
                if      (c >= '0' && c <= '9') codepoint |= c & 0xF;
                else if (c >= 'A' && c <= 'F') codepoint |= c & 0x1F + 9;
                else
                {
                    //// ERROR
                    MM_umm edc = expected_digit_count;
                    if      (edc == 2) return (MM_Token_Parse_Error){ .code = MM_TokenParseError_InvalidDigitInByteEscSeq,           .index = i + j };
                    else if (edc == 4) return (MM_Token_Parse_Error){ .code = MM_TokenParseError_InvalidDigitInShortCodepointEscSeq, .index = i + j };
                    else               return (MM_Token_Parse_Error){ .code = MM_TokenParseError_InvalidDigitInCodepointEscSeq,      .index = i + j };
                }
            }
        }
        
        i += expected_digit_count;
    }
    
    *result = codepoint;
    *index  = i;
    return (MM_Token_Parse_Error){ .code = MM_TokenParseError_None };
}

MM_Token_Parse_Error
MM_Token_ParseCodepoint(MM_Token token, MM_u32* result)
{
    MM_ASSERT(token.size >= 2 && token.data[0] == '\'');
    
    MM_u32 codepoint = 0;
    
    if (token.size == 2)
    {
        //// ERROR
        return (MM_Token_Parse_Error){ .code = MM_TokenParseError_NoCodepointInCodepointLiteral, .index = 1 };
    }
    else
    {
        MM_ASSERT(token.size > 2);
        
        MM_umm i = 1;
        char c   = token.data[i];
        
        if (c == '\\')
        {
            i += 1;
            MM_Token_Parse_Error error = MM_Token__ParseEscSeq(token, &codepoint, &i);
            if (error.code != MM_TokenParseError_None) return error;
        }
        else if ((c & 0x80) != 0)
        {
            MM_umm len;
            if ((c & 0xF8) == 0xF0)
            {
                codepoint |= c & 0x07;
                len = 4;
            }
            else if ((c & 0xF0) == 0xE0)
            {
                codepoint |= c & 0x0F;
                len = 3;
            }
            else if ((c & 0xE0) == 0xC0)
            {
                codepoint |= c & 0x1F;
                len = 2;
            }
            else
            {
                //// ERROR
                return (MM_Token_Parse_Error){ .code = MM_TokenParseError_InvalidFirstByteOfUTF8, .index = i };
            }
            
            MM_umm j = i + 1;
            for (; j < i + len && j < token.size - 1; ++j)
            {
                c = token.data[j];
                if ((c & 0xC0) == 0x80) break;
                else
                {
                    codepoint <<= 6;
                    codepoint  |= c & 0x3F;
                }
            }
            
            if (i + j == token.size - 1) i += len;
            else
            {
                //// ERROR
                MM_ASSERT(i + j < token.size);
                MM_umm diff = (i + len) - j;
                if      (diff == 1) return (MM_Token_Parse_Error){ .code = MM_TokenParseError_Missing1TrailingByteUTF8, .index = i + j };
                else if (diff == 2) return (MM_Token_Parse_Error){ .code = MM_TokenParseError_Missing2TrailingByteUTF8, .index = i + j };
                else if (diff == 3) return (MM_Token_Parse_Error){ .code = MM_TokenParseError_Missing3TrailingByteUTF8, .index = i + j };
            }
        }
        else
        {
            codepoint = c;
            i += 1;
        }
        
        if (i > token.size - 1)
        {
            //// ERROR
            return (MM_Token_Parse_Error){ .code = MM_TokenParseError_MoreThanOneCodepoint, .index = i };
        }
        else MM_ASSERT(i == token.size - 1 && token.data[i] == '\'');
    }
    
    *result = codepoint;
    
    return (MM_Token_Parse_Error){ .code = MM_TokenParseError_None };
}

MM_Token_Parse_Error
MM_Token_ParseString(MM_Token token, MM_u8* buffer, MM_String* result)
{
    MM_ASSERT(token.size >= 2 && token.data[0] == '"');
    
    result->data = buffer;
    result->size = 0;
    
    MM_umm i = 1;
    while (i < token.size - 1)
    {
        char c = token.data[i];
        
        if (c == '\\')
        {
            i += 1;
            
            MM_u32 codepoint;
            MM_Token_Parse_Error error = MM_Token__ParseEscSeq(token, &codepoint, &i);
            
            if (error.code != MM_TokenParseError_None) return error;
            else
            {
                if (codepoint <= 0x7F)
                {
                    result->data[result->size++] = (MM_u8)codepoint;
                }
                else if (codepoint <= 0x07FF)
                {
                    result->data[result->size++] = (MM_u8)((codepoint >> 6)   | 0xC0);
                    result->data[result->size++] = (MM_u8)((codepoint & 0x3F) | 0x80);
                }
                else if (codepoint <= 0xFFFF)
                {
                    result->data[result->size++] = (MM_u8)((codepoint >> 12)         | 0xE0);
                    result->data[result->size++] = (MM_u8)(((codepoint >> 6) & 0x3F) | 0x80);
                    result->data[result->size++] = (MM_u8)((codepoint & 0x3F)        | 0x80);
                }
                else if (codepoint <= 0x10FFFF)
                {
                    result->data[result->size++] = (MM_u8)((codepoint >> 18)          | 0xF0);
                    result->data[result->size++] = (MM_u8)(((codepoint >> 12) & 0x3F) | 0x80);
                    result->data[result->size++] = (MM_u8)(((codepoint >> 6)  & 0x3F) | 0x80);
                    result->data[result->size++] = (MM_u8)((codepoint & 0x3F)         | 0x80);
                }
                else
                {
                    //// ERROR
                    return (MM_Token_Parse_Error){ .code = MM_TokenParseError_CodepointOutOfRange, .index = i };
                }
            }
        }
        else if ((c & 0x80) != 0)
        {
            MM_umm len;
            if      ((c & 0xF8) == 0xF0) len = 4;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xE0) == 0xC0) len = 2;
            else
            {
                //// ERROR
                return (MM_Token_Parse_Error){ .code = MM_TokenParseError_InvalidFirstByteOfUTF8, .index = i };
            }
            
            result->data[result->size++] = c;
            
            MM_umm j = i + 1;
            for (; j < i + len && j < token.size - 1; ++j)
            {
                c = token.data[j];
                if ((c & 0xC0) == 0x80) break;
                else result->data[result->size++] = c;
            }
            
            if (i + j == token.size - 1) i += len;
            else
            {
                //// ERROR
                MM_ASSERT(i + j < token.size);
                MM_umm diff = (i + len) - j;
                if      (diff == 1) return (MM_Token_Parse_Error){ .code = MM_TokenParseError_Missing1TrailingByteUTF8, .index = i + j };
                else if (diff == 2) return (MM_Token_Parse_Error){ .code = MM_TokenParseError_Missing2TrailingByteUTF8, .index = i + j };
                else if (diff == 3) return (MM_Token_Parse_Error){ .code = MM_TokenParseError_Missing3TrailingByteUTF8, .index = i + j };
            }
        }
        else
        {
            result->data[result->size++] = c;
            i += 1;
        }
    }
    
    MM_ASSERT(i == token.size - 1 && token.data[i] == '"');
    
    return (MM_Token_Parse_Error){ .code = MM_TokenParseError_None };
}