#define PARSER_PEEK_MAX 2

typedef struct Parser
{
    Workspace* workspace;
    Lexer lexer;
    Token peek[1 + PARSER_PEEK_MAX];
    u8 peek_cursor;
    File_ID file_id;
} Parser;

internal Token
GetToken(Parser* parser)
{
    return parser->peek[parser->peek_cursor];
}

internal Token
PeekToken(Parser* parser, umm index)
{
    ASSERT(index <= PARSER_PEEK_MAX);
    return parser->peek[(parser->peek_cursor + index) % ARRAY_SIZE(parser->peek)];
}

internal Token
NextToken(Parser* parser, Whitespace_Info* ws)
{
    *ws = WhitespaceInfo_FromToken(parser->file_id, parser->peek[parser->peek_cursor]);
    
    parser->peek_cursor = (parser->peek_cursor + 1) % ARRAY_SIZE(parser->peek);
    parser->peek[(parser->peek_cursor + PARSER_PEEK_MAX) % ARRAY_SIZE(parser->peek)] = Lexer_Advance(parser->workspace, &parser->lexer);
    
    return parser->peek[parser->peek_cursor];
}

internal bool
EatToken(Parser* parser, TOKEN_KIND kind, Whitespace_Info* ws)
{
    return (parser->peek[parser->peek_cursor].kind == kind ? NextToken(parser, ws), true : false);
}

internal AST_Node*
PushNode(Parser* parser, AST_NODE_KIND kind, u32 line, u32 col, AST_Whitespace_Info ws_info)
{
    AST_Node* result = Arena_PushSize(parser->workspace->ast_arena, sizeof(AST_Node), ALIGNOF(AST_Node));
    ZeroStruct(result);
    
    AST_Node_Info* info = Arena_PushSize(parser->workspace->ast_info_arena, sizeof(AST_Node_Info), ALIGNOF(AST_Node_Info));
    ZeroStruct(info);
    
    result->kind = kind;
    result->info = info;
    
    info->file_id = parser->file_id;
    info->line = line;
    info->col  = col;
    Copy(ws_info.ws, info->ws, sizeof(info->ws));
    
    return result;
}
#define LIST_PARSE_ERROR_CODES()                                                                                        \
X(0,  ParseError_UsingUnnamedParam,                "Illegal use of 'using' on unnamed parameter")                   \
X(1,  ParseError_MultipleTypesParam,               "Multiple types are illegal")                                    \
X(2,  ParseError_UninitConstant,                   "Constants must be initialized")                                 \
X(3,  ParseError_MissingAlias,                     "Missing name of alias after 'as' keyword in using declaration") \
X(4,  ParseError_KeywordAsAlias,                   "Illegal use of keyword as alias")                               \
X(5,  ParseError_BlankAsAlias,                     "Illegal use of blank identifier as alias")                      \
X(6,  ParseError_MultipleSymSameAlias,             "Cannot alias multiple symbols under the same alias")            \
X(7,  ParseError_IsolatedExprList,                 "A list of expressions cannot be used by itself")                \
X(8,  ParseError_MissingParenAfterParams,          "Missing closing paren after parameters")                        \
X(9,  ParseError_MissingParenAfterRetTypes,        "Missing closing paren after return type list")                  \
X(10, ParseError_MissingUnionBody,                 "Missing body of union")                                         \
X(11, ParseError_MissingStructBody,                "Missing body of struct")                                        \
X(12, ParseError_MissingEnumBody,                  "Missing body of enum")                                          \
X(13, ParseError_MissingBraceAfterEnumBody,        "Missing closing brace after body of enum")                      \
X(14, ParseError_MissingArgsToCall,                "Missing arguments to call")                                     \
X(15, ParseError_MissingParenAfterCallArgs,        "Missing closing paren after arguments to call")                 \
X(16, ParseError_IllegalKeywordInExpr,             "Illegal use of keyword in expression")                          \
X(17, ParseError_MissingBraceAfterStructLitArgs,   "Missing closing brace after arguments to struct literal")       \
X(18, ParseError_MissingBraceAfterArrayLitArgs,    "Missing closing bracket after arguments to array literal")      \
X(19, ParseError_MissingParenAfterCompound,        "Missing closing paren after compound expression body")          \
X(20, ParseError_MissingEnumSelectorName,          "Missing name of enum in enum selector expression")              \
X(21, ParseError_MissingPolyName,                  "Missing name of polymorphic value")                             \
X(22, ParseError_MissingPrimaryExpr,               "Missing primary expression")                                    \
X(23, ParseError_MissingMemberAccessName,          "Missing name of member to access")                              \
X(24, ParseError_MemberAccessBlank,                "Member name must be a non blank identifier")                    \
X(25, ParseError_MissingBracketAfterSusbcript,     "Missing closing bracket after subscript index")                 \
X(26, ParseError_MissingBracketAfterSlice,         "Missing closing bracket after slice arguments")                 \
X(27, ParseError_MissingBracketAfterArrayTypeSize, "Missing closing bracket after size in array type qualifier")    \
X(28, ParseError_MissingConditionalFalseExpr,      "Missing false expr")                                            \
X(29, ParseError_MissingStatement,                 "Missing statement before semicolon")                            \
X(30, ParseError_KeywordAsLabel,                   "Illegal use of keyword as label name")                          \
X(31, ParseError_BlankLabel,                       "Label name cannot be blank")                                    \
X(32, ParseError_LabelElse,                        "Illegal use of label on else statement")                        \
X(33, ParseError_MissingIfCondition,               "Missing condition of if statement")                             \
X(34, ParseError_MissingParenAfterIfCondition,     "Missing closing paren after if condition")                      \
X(35, ParseError_IsolatedElse,                     "Illegal else without matching if")                              \
X(36, ParseError_MissingWhileCondition,            "Missing condition of while statement")                          \
X(37, ParseError_MissingParenAfterWhileHeader,     "Missing closing paren after while header")                      \
X(38, ParseError_IllegalLabel,                     "Illegal label")                                                 \
X(39, ParseError_MissingSemicolon,                 "Missing terminating semicolon after statement")                 \
X(40, ParseError_DefaultValOnUnnamedParam,         "Default values cannot be used with unnamed parameters")         \

typedef enum PARSER_ERROR_CODE
{
    PARSER_ERROR_CODE_BASE_SENTINEL = PARSER_ERROR_CODE_BASE,
    
#define X(i, e, s) e = PARSER_ERROR_CODE_BASE_SENTINEL + i,
    LIST_PARSE_ERROR_CODES()
#undef X
    
    PARSER_ERROR_CODE_MAX
} PARSER_ERROR_CODE;

char* ParserErrorCodeMessages[PARSER_ERROR_CODE_MAX - LEXER_ERROR_CODE_BASE_SENTINEL] = {
#define X(i, e, s) [e - PARSER_ERROR_CODE_BASE_SENTINEL] = s,
    LIST_PARSE_ERROR_CODES()
#undef X
};

internal void
ParserError(Parser* parser, PARSER_ERROR_CODE code, Text_Interval text)
{
    parser->workspace->report = (Error_Report){
        .code    = code,
        .message = String_WrapCString(ParserErrorCodeMessages[code - PARSER_ERROR_CODE_BASE]),
        .text    = text,
        .file_id = parser->file_id,
    };
}

internal bool ParseExpression(Parser* parser, AST_Node** expression);
internal bool ParseStatement(Parser* parser, AST_Node** statement);
internal bool ParseBlock(Parser* parser, AST_Node** block);

internal bool
ParseNamedValueList(Parser* parser, AST_Node** list)
{
    AST_Node** next = list;
    bool should_continue = true;
    while (should_continue)
    {
        Token token = GetToken(parser);
        u32 line = token.text.line;
        u32 col  = token.text.column;
        AST_Whitespace_Info whitespace_info = {0};
        
        AST_Node* name  = 0;
        AST_Node* value = 0;
        
        if (!ParseExpression(parser, &value)) return false;
        if (EatToken(parser, Token_Equals, &whitespace_info.named_value_info.ws_equals))
        {
            name = value;
            if (!ParseExpression(parser, &value)) return false;
        }
        
        should_continue = EatToken(parser, Token_Comma, &whitespace_info.ws_terminator);
        
        *next = PushNode(parser, AST_NamedValue, line, col, whitespace_info);
        (*next)->named_value.name  = name;
        (*next)->named_value.value = value;
        
        next = &(*next)->next;
    }
    
    return true;
}

internal bool
ParseParameter(Parser* parser, AST_Node** param)
{
    Token token = GetToken(parser);
    u32 line = token.text.line;
    u32 col  = token.text.column;
    AST_Whitespace_Info whitespace_info = { .ws_before = WhitespaceInfo_FromToken(parser->file_id, token) };
    
    AST_Node* name  = 0;
    AST_Node* type  = 0;
    AST_Node* value = 0;
    bool is_using = false;
    
    if (token.kind == Token_Identifier && token.string == Keyword_Using)
    {
        NextToken(parser, &whitespace_info.ws_before);
        is_using = true;
    }
    
    if (!ParseExpression(parser, &type)) return false;
    
    if (EatToken(parser, Token_Colon, &whitespace_info.parameter_info.ws_colon))
    {
        name = type;
        type = 0;
        if (GetToken(parser).kind != Token_Equals && !ParseExpression(parser, &type)) return false;
    }
    else if (is_using)
    {
        ParserError(parser, ParseError_UsingUnnamedParam, TextInterval_BetweenStartPoints(token.text, GetToken(parser).text));
        return false;
    }
    
    if (EatToken(parser, Token_Equals, &whitespace_info.parameter_info.ws_equals))
    {
        if (type != 0)
        {
            ParserError(parser, ParseError_DefaultValOnUnnamedParam, TextInterval_BetweenStartPoints(token.text, GetToken(parser).text));
            return false;
        }
        else if (!ParseExpression(parser, &value)) return false;
    }
    
    *param = PushNode(parser, AST_Parameter, line, col, whitespace_info);
    (*param)->parameter.name     = name;
    (*param)->parameter.type     = type;
    (*param)->parameter.value    = value;
    (*param)->parameter.is_using = is_using;
    
    return true;
}

internal bool
ParseParameterList(Parser* parser, AST_Node** params)
{
    AST_Node** next_param = params;
    while (true)
    {
        if      (!ParseParameter(parser, next_param)) return false;
        else if (EatToken(parser, Token_Comma, &(*next_param)->info->ws_terminator))
        {
            next_param = &(*next_param)->next;
            continue;
        }
        else break;
        
    }
    
    return true;
}

internal bool
ParseAssignmentDeclarationOrExpression(Parser* parser, AST_Node** statement)
{
    Token token = GetToken(parser);
    u32 line = token.text.line;
    u32 col  = token.text.column;
    AST_Whitespace_Info whitespace_info = {0};
    
    Text_Interval first_token_text = token.text;
    
    bool is_using = false;
    
    if (token.kind == Token_Identifier && token.string == Keyword_Using)
    {
        NextToken(parser, &whitespace_info.ws_before);
        is_using = true;
    }
    
    AST_Node* expressions      = 0;
    AST_Node** next_expression = &expressions;
    while (true)
    {
        if (!ParseExpression(parser, next_expression)) return false;
        
        if (EatToken(parser, Token_Comma, &(*next_expression)->info->ws_terminator))
        {
            next_expression = &(*next_expression)->next;
            continue;
        }
        else break;
    }
    
    Whitespace_Info tmp_colon_info;
    if (EatToken(parser, Token_Colon, &tmp_colon_info))
    {
        bool is_const = false;
        AST_Node* names       = expressions;
        AST_Node* type        = 0;
        AST_Node* values      = 0;
        bool is_uninitialized = false;
        
        token = GetToken(parser);
        if (token.kind != Token_Equals && token.kind != Token_Colon)
        {
            if (!ParseExpression(parser, &type)) return false;
            
            if (GetToken(parser).kind == Token_Comma)
            {
                ParserError(parser, ParseError_MultipleTypesParam, TextInterval_BetweenStartPoints(token.text, GetToken(parser).text));
                return false;
            }
        }
        
        token = GetToken(parser);
        if (token.kind == Token_Equals || token.kind == Token_Colon)
        {
            is_const = (token.kind == Token_Colon);
            token = NextToken(parser, (is_const ? &whitespace_info.constant_info.ws_colon_1 : &whitespace_info.variable_info.ws_equals));
            
            if (token.kind == Token_TripleMinus)
            {
                NextToken(parser, &whitespace_info.variable_info.ws_uninit);
                
                if (!is_const) is_uninitialized = true;
                else
                {
                    ParserError(parser, ParseError_UninitConstant, token.text);
                    return false;
                }
            }
            else
            {
                next_expression = &values;
                while (true)
                {
                    if (!ParseExpression(parser, next_expression)) return false;
                    
                    
                    if (EatToken(parser, Token_Comma, &(*next_expression)->info->ws_terminator))
                    {
                        next_expression = &(*next_expression)->next;
                        continue;
                    }
                    else break;
                }
            }
        }
        
        if (is_const)
        {
            whitespace_info.constant_info.ws_colon_0 = tmp_colon_info;
            
            *statement = PushNode(parser, AST_Constant, line, col, whitespace_info);
            (*statement)->constant_decl.names    = names;
            (*statement)->constant_decl.type     = type;
            (*statement)->constant_decl.values   = values;
            (*statement)->constant_decl.is_using = is_using;
        }
        else
        {
            whitespace_info.variable_info.ws_colon = tmp_colon_info;
            
            *statement = PushNode(parser, AST_Variable, line, col, whitespace_info);
            (*statement)->variable_decl.names            = names;
            (*statement)->variable_decl.type             = type;
            (*statement)->variable_decl.values           = values;
            (*statement)->variable_decl.is_using         = is_using;
            (*statement)->variable_decl.is_uninitialized = is_uninitialized;
        }
    }
    else
    {
        token = GetToken(parser);
        
        if (token.kind >= Token_FirstAssignment && token.kind <= Token_LastAssignment)
        {
            AST_NODE_KIND op = (token.kind == Token_Equals ? AST_Nil : token.kind + (Token_FirstMulLevel - Token_FirstMulLevelAssignment));
            AST_Node* lhs = expressions;
            AST_Node* rhs = 0;
            
            NextToken(parser, &whitespace_info.binary_info.ws_op);
            
            next_expression = &rhs;
            while (true)
            {
                if (!ParseExpression(parser, next_expression)) return false;
                
                
                if (EatToken(parser, Token_Comma, &(*next_expression)->info->ws_terminator))
                {
                    next_expression = &(*next_expression)->next;
                    continue;
                }
                else break;
            }
            
            *statement = PushNode(parser, AST_Assignment, line, col, whitespace_info);
            (*statement)->assignment_statement.lhs = lhs;
            (*statement)->assignment_statement.rhs = rhs;
            (*statement)->assignment_statement.op  = op;
        }
        else if (is_using)
        {
            AST_Node* symbol_paths = expressions;
            Interned_String alias  = INTERNED_STRING_NIL;
            
            if (token.kind == Token_Identifier && token.string == Keyword_As)
            {
                token = NextToken(parser, &whitespace_info.using_info.ws_as);
                
                if (token.kind != Token_Identifier)
                {
                    ParserError(parser, ParseError_MissingAlias, TextInterval_BetweenStartPoints(first_token_text, token.text));
                    return false;
                }
                else if (InternedString_IsKeyword(token.string))
                {
                    ParserError(parser, ParseError_KeywordAsAlias, token.text);
                    return false;
                }
                else if (token.string == BLANK_IDENTIFIER)
                {
                    ParserError(parser, ParseError_BlankAsAlias, token.text);
                    return false;
                }
                else if (symbol_paths->next != 0)
                {
                    ParserError(parser, ParseError_MultipleSymSameAlias, TextInterval_ContainingBoth(first_token_text, token.text));
                    return false;
                }
                
                alias = token.string;
                NextToken(parser, &whitespace_info.using_info.ws_alias);
            }
            
            *statement = PushNode(parser, AST_Using, line, col, whitespace_info);
            (*statement)->using_decl.symbol_paths = symbol_paths;
            (*statement)->using_decl.alias        = alias;
        }
        else
        {
            if (expressions->next != 0)
            {
                ParserError(parser, ParseError_IsolatedExprList, TextInterval_BetweenStartPoints(first_token_text, GetToken(parser).text));
                return false;
            }
            
            *statement = expressions;
        }
    }
    
    return true;
}

internal bool
ParsePrimaryExpression(Parser* parser, AST_Node** expression)
{
    Token token = GetToken(parser);
    u32 line = token.text.line;
    u32 col  = token.text.column;
    AST_Whitespace_Info whitespace_info = {0};
    
    NextToken(parser, &whitespace_info.ws_before);
    
    if (token.kind == Token_String)
    {
        *expression = PushNode(parser, AST_String, line, col, whitespace_info);
        (*expression)->string = token.string;
    }
    else if (token.kind == Token_Int)
    {
        *expression = PushNode(parser, AST_Int, line, col, whitespace_info);
        (*expression)->integer.i256 = token.integer.i256;
        (*expression)->integer.base = token.integer.base;
    }
    else if (token.kind == Token_Float)
    {
        *expression = PushNode(parser, AST_Float, line, col, whitespace_info);
        (*expression)->floating.float64       = token.floating.float64;
        (*expression)->floating.hex_byte_size = token.floating.hex_byte_size;
    }
    else if (token.kind == Token_Character)
    {
        *expression = PushNode(parser, AST_Char, line, col, whitespace_info);
        (*expression)->character = token.character;
    }
    else if (token.kind == Token_Identifier)
    {
        // NOTE: This also catches INTERNED_STRING_NIL, EMPTY_STRING and
        //       BLANK_IDENTIFIER, but since the token is an identifier,
        //       INTERNED_STRING_NIL and EMPTY_STRING cannot appear
        if (!InternedString_IsKeyword(token.string))
        {
            *expression = PushNode(parser, AST_Identifier, line, col, whitespace_info);
            (*expression)->string = token.string;
        }
        else
        {
            if (token.string == Keyword_True || token.string == Keyword_False)
            {
                *expression = PushNode(parser, AST_Bool, line, col, whitespace_info);
                (*expression)->boolean = (token.string == Keyword_True);
            }
            else if (token.string == Keyword_Proc)
            {
                AST_Node* params       = 0;
                AST_Node* return_types = 0;
                AST_Node* where_clause = 0;
                
                if (EatToken(parser, Token_OpenParen, &whitespace_info.proc_info.ws_open_paren))
                {
                    if      (!ParseParameterList(parser, &params)) return false;
                    else if (!EatToken(parser, Token_CloseParen, &whitespace_info.proc_info.ws_close_paren))
                    {
                        token = GetToken(parser);
                        ParserError(parser, ParseError_MissingParenAfterParams, TextInterval_BetweenStartPoints(token.text, token.text));
                        return false;
                    }
                }
                
                if (EatToken(parser, Token_Arrow, &whitespace_info.proc_info.ws_arrow))
                {
                    bool is_multi = EatToken(parser, Token_OpenParen, &whitespace_info.proc_info.ws_open_paren_ret);
                    
                    AST_Node** next_type = &return_types;
                    while (true)
                    {
                        if (!ParseParameter(parser, next_type)) return false;
                        
                        if (is_multi && EatToken(parser, Token_Comma, &(*next_type)->info->ws_terminator))
                        {
                            next_type = &(*next_type)->next;
                            continue;
                        }
                        else break;
                        
                    }
                    
                    if (is_multi && !EatToken(parser, Token_CloseParen, &whitespace_info.proc_info.ws_close_paren_ret))
                    {
                        token = GetToken(parser);
                        ParserError(parser, ParseError_MissingParenAfterRetTypes, TextInterval_BetweenStartPoints(token.text, token.text));
                        return false;
                    }
                }
                
                token = GetToken(parser);
                if (token.kind == Token_Identifier && token.string == Keyword_Where)
                {
                    NextToken(parser, &whitespace_info.proc_info.ws_where);
                    if (!ParseExpression(parser, &where_clause)) return false;
                }
                
                token = GetToken(parser);
                if (token.kind != Token_TripleMinus && token.kind != Token_OpenBrace)
                {
                    *expression = PushNode(parser, AST_Proc, line, col, whitespace_info);
                    (*expression)->proc_type.params       = params;
                    (*expression)->proc_type.return_types = return_types;
                    (*expression)->proc_type.where_clause = where_clause;
                }
                else
                {
                    AST_Node* body = 0;
                    
                    if (!EatToken(parser, Token_TripleMinus, &whitespace_info.proc_info.ws_uninit) && !ParseBlock(parser, &body)) return false;
                    
                    *expression = PushNode(parser, AST_ProcLiteral, line, col, whitespace_info);
                    (*expression)->proc_literal.params       = params;
                    (*expression)->proc_literal.return_types = return_types;
                    (*expression)->proc_literal.where_clause = where_clause;
                    (*expression)->proc_literal.body         = body;
                }
            }
            else if (token.string == Keyword_Struct || token.string == Keyword_Union)
            {
                bool is_union    = (token.string == Keyword_Union);
                AST_Node* params = 0;
                AST_Node* body   = 0;
                
                if (EatToken(parser, Token_OpenParen, &whitespace_info.struct_info.ws_open_paren))
                {
                    if      (!ParseParameterList(parser, &params)) return false;
                    else if (!EatToken(parser, Token_CloseParen, &whitespace_info.struct_info.ws_close_paren))
                    {
                        token = GetToken(parser);
                        ParserError(parser, ParseError_MissingParenAfterParams, TextInterval_BetweenStartPoints(token.text, token.text));
                        return false;
                    }
                }
                
                if (!EatToken(parser, Token_OpenBrace, &whitespace_info.struct_info.ws_open_brace))
                {
                    token = GetToken(parser);
                    if (is_union) ParserError(parser, ParseError_MissingUnionBody, TextInterval_BetweenStartPoints(token.text, token.text));
                    else          ParserError(parser, ParseError_MissingStructBody, TextInterval_BetweenStartPoints(token.text, token.text));
                    return false;
                }
                else
                {
                    AST_Node** next_statement = &body;
                    while (!EatToken(parser, Token_CloseBrace, &whitespace_info.struct_info.ws_close_brace))
                    {
                        if (!ParseStatement(parser, next_statement)) return false;
                        if (*next_statement) next_statement = &(*next_statement)->next;
                    }
                }
                
                if (is_union)
                {
                    *expression = PushNode(parser, AST_Union, line, col, whitespace_info);
                    (*expression)->union_type.params = params;
                    (*expression)->union_type.body   = body;
                }
                else
                {
                    *expression = PushNode(parser, AST_Struct, line, col, whitespace_info);
                    (*expression)->struct_type.params = params;
                    (*expression)->struct_type.body   = body;
                }
            }
            else if (token.string == Keyword_Enum)
            {
                AST_Node* type = 0;
                AST_Node* body = 0;
                
                token = GetToken(parser);
                if (token.kind != Token_OpenBrace && !ParseExpression(parser, &type)) return false;
                else if (!EatToken(parser, Token_OpenBrace, &whitespace_info.enum_info.ws_open_brace))
                {
                    token = GetToken(parser);
                    ParserError(parser, ParseError_MissingEnumBody, TextInterval_BetweenStartPoints(token.text, token.text));
                    return false;
                }
                
                AST_Node** next = &body;
                while (true)
                {
                    token = GetToken(parser);
                    u32 member_line = token.text.line;
                    u32 member_col  = token.text.column;
                    AST_Whitespace_Info member_whitespace_info = {0};
                    
                    AST_Node* name  = 0;
                    AST_Node* value = 0;
                    
                    if (!ParseExpression(parser, &name))                                                                                        return false;
                    if (EatToken(parser, Token_Equals, &member_whitespace_info.enum_member_info.ws_equals) && !ParseExpression(parser, &value)) return false;
                    
                    *next = PushNode(parser, AST_EnumMember, member_line, member_col, member_whitespace_info);
                    (*next)->enum_member.name  = name;
                    (*next)->enum_member.value = value;
                    
                    
                    if (EatToken(parser, Token_Comma, &(*next)->info->ws_terminator))
                    {
                        next = &(*next)->next;
                        continue;
                    }
                    else break;
                }
                
                if (!EatToken(parser, Token_CloseBrace, &whitespace_info.enum_info.ws_close_brace))
                {
                    token = GetToken(parser);
                    ParserError(parser, ParseError_MissingBraceAfterEnumBody, TextInterval_BetweenStartPoints(token.text, token.text));
                    return false;
                }
                
                *expression = PushNode(parser, AST_Enum, line, col, whitespace_info);
                (*expression)->enum_type.type = type;
                (*expression)->enum_type.body = body;
            }
            else if (token.string == Keyword_Cast || token.string == Keyword_Transmute)
            {
                Interned_String proc = token.string;
                
                if (!EatToken(parser, Token_OpenParen, &whitespace_info.call_info.ws_open_paren))
                {
                    ParserError(parser, ParseError_MissingArgsToCall, TextInterval_BetweenStartPoints(token.text, GetToken(parser).text));
                    return false;
                }
                
                AST_Node* args = 0;
                if (!ParseNamedValueList(parser, &args)) return false;
                
                if (!EatToken(parser, Token_OpenParen, &whitespace_info.call_info.ws_close_paren))
                {
                    token = GetToken(parser);
                    ParserError(parser, ParseError_MissingParenAfterCallArgs, TextInterval_BetweenStartPoints(token.text, token.text));
                    return false;
                }
                
                *expression = PushNode(parser, AST_IntrinsicCall, line, col, whitespace_info);
                (*expression)->intrinsic_call_expr.proc = proc;
                (*expression)->intrinsic_call_expr.args = args;
            }
            else
            {
                token = GetToken(parser);
                ParserError(parser, ParseError_IllegalKeywordInExpr, TextInterval_BetweenStartPoints(token.text, token.text));
                return false;
            }
        }
    }
    else if (token.kind == Token_PeriodOpenBrace)
    {
        AST_Node* args = 0;
        
        if (!EatToken(parser, Token_CloseBrace, &whitespace_info.struct_literal_info.ws_close_brace))
        {
            if      (!ParseNamedValueList(parser, &args)) return false;
            else if (!EatToken(parser, Token_CloseBrace, &whitespace_info.struct_literal_info.ws_close_brace))
            {
                token = GetToken(parser);
                ParserError(parser, ParseError_MissingBraceAfterStructLitArgs, TextInterval_BetweenStartPoints(token.text, token.text));
                return false;
            }
        }
        
        *expression = PushNode(parser, AST_StructLiteral, line, col, whitespace_info);
        (*expression)->struct_literal.type = 0;
        (*expression)->struct_literal.args = args;
    }
    else if (token.kind == Token_PeriodOpenBracket)
    {
        AST_Node* args = 0;
        
        if (!EatToken(parser, Token_CloseBracket, &whitespace_info.array_literal_info.ws_close_bracket))
        {
            if      (!ParseNamedValueList(parser, &args)) return false;
            else if (!EatToken(parser, Token_CloseBracket, &whitespace_info.array_literal_info.ws_close_bracket))
            {
                token = GetToken(parser);
                ParserError(parser, ParseError_MissingBraceAfterArrayLitArgs, TextInterval_BetweenStartPoints(token.text, token.text));
                return false;
            }
        }
        
        *expression = PushNode(parser, AST_ArrayLiteral, line, col, whitespace_info);
        (*expression)->array_literal.type = 0;
        (*expression)->array_literal.args = args;
    }
    else if (token.kind == Token_OpenParen)
    {
        AST_Node* compound;
        if      (!ParseExpression(parser, &compound)) return false;
        else if (!EatToken(parser, Token_CloseParen, &whitespace_info.compound_info.ws_close_paren))
        {
            token = GetToken(parser);
            ParserError(parser, ParseError_MissingParenAfterCompound, TextInterval_BetweenStartPoints(token.text, token.text));
            return false;
        }
        
        *expression = PushNode(parser, AST_Compound, line, col, whitespace_info);
        (*expression)->compound_expr = compound;
    }
    else if (token.kind == Token_Period)
    {
        if (GetToken(parser).kind != Token_Identifier)
        {
            ParserError(parser, ParseError_MissingEnumSelectorName, TextInterval_BetweenStartPoints(token.text, GetToken(parser).text));
            return false;
        }
        else token = GetToken(parser);
        
        Interned_String enum_name = token.string;
        NextToken(parser, &whitespace_info.selector_info.ws_name);
        
        *expression = PushNode(parser, AST_Selector, line, col, whitespace_info);
        (*expression)->selector_expr = enum_name;
    }
    else if (token.kind == Token_Cash)
    {
        if (GetToken(parser).kind != Token_Identifier)
        {
            ParserError(parser, ParseError_MissingPolyName, TextInterval_BetweenStartPoints(token.text, GetToken(parser).text));
            return false;
        }
        else token = GetToken(parser);
        
        Interned_String poly_name = token.string;
        NextToken(parser, &whitespace_info.poly_name_info.ws_name);
        
        *expression = PushNode(parser, AST_PolymorphicName, line, col, whitespace_info);
        (*expression)->poly_name = poly_name;
    }
    else
    {
        if (token.kind == Token_Invalid)
        {
            parser->workspace->report = (Error_Report){
                .code    = token.error.code,
                .message = String_WrapCString(LexerErrorCodeMessages[token.error.code]),
                .text    = token.error.text,
                .file_id = parser->file_id,
            };
        }
        else
        {
            token = GetToken(parser);
            ParserError(parser, ParseError_MissingPrimaryExpr, TextInterval_BetweenStartPoints(token.text, token.text));
        }
        return false;
    }
    
    return true;
}

internal bool
ParsePostfixExpression(Parser* parser, AST_Node** expression)
{
    Token token = GetToken(parser);
    u32 line = token.text.line;
    u32 col  = token.text.column;
    AST_Whitespace_Info whitespace_info = {0};
    
    if (!ParsePrimaryExpression(parser, expression)) return false;
    else
    {
        while (true)
        {
            Text_Interval start_text = GetToken(parser).text;
            
            if (EatToken(parser, Token_Hat, &whitespace_info.deref_info.ws_hat))
            {
                AST_Node* operand = *expression;
                
                *expression = PushNode(parser, AST_Dereference, line, col, whitespace_info);
                (*expression)->unary_expr = operand;
            }
            else if (EatToken(parser, Token_Period, &whitespace_info.member_info.ws_period))
            {
                AST_Node* expr = *expression;
                
                token = GetToken(parser);
                
                if (token.kind != Token_Identifier)
                {
                    ParserError(parser, ParseError_MissingMemberAccessName, TextInterval_BetweenStartPoints(start_text, token.text));
                    return false;
                }
                else if (!InternedString_IsNonBlankIdentifier(token.string))
                {
                    ParserError(parser, ParseError_MemberAccessBlank, token.text);
                    return false;
                }
                
                NextToken(parser, &whitespace_info.member_info.ws_member);
                
                *expression = PushNode(parser, AST_Member, line, col, whitespace_info);
                (*expression)->member_expr.expr   = expr;
                (*expression)->member_expr.member = token.string;
            }
            else if (token.kind == Token_OpenBracket)
            {
                Whitespace_Info tmp_open_bracket;
                NextToken(parser, &tmp_open_bracket);
                
                AST_Node* array = *expression;
                AST_Node* first = 0;
                
                if (GetToken(parser).kind != Token_Colon && !ParseExpression(parser, &first)) return false;
                
                if (!EatToken(parser, Token_Colon, &whitespace_info.slice_info.ws_colon))
                {
                    if (!EatToken(parser, Token_CloseBracket, &whitespace_info.subscript_info.ws_close_bracket))
                    {
                        token = GetToken(parser);
                        ParserError(parser, ParseError_MissingBracketAfterSusbcript, TextInterval_BetweenStartPoints(token.text, token.text));
                        return false;
                    }
                    
                    whitespace_info.subscript_info.ws_open_bracket = tmp_open_bracket;
                    *expression = PushNode(parser, AST_Subscript, line, col, whitespace_info);
                    (*expression)->subscript_expr.array = array;
                    (*expression)->subscript_expr.index = first;
                }
                else
                {
                    AST_Node* start    = first;
                    AST_Node* past_end = 0;
                    
                    if (GetToken(parser).kind != Token_CloseBracket && !ParseExpression(parser, &past_end)) return false;
                    
                    if (!EatToken(parser, Token_CloseBracket, &whitespace_info.subscript_info.ws_close_bracket))
                    {
                        token = GetToken(parser);
                        ParserError(parser, ParseError_MissingBracketAfterSlice, TextInterval_BetweenStartPoints(token.text, token.text));
                        return false;
                    }
                    
                    whitespace_info.slice_info.ws_open_bracket = tmp_open_bracket;
                    *expression = PushNode(parser, AST_Slice, line, col, whitespace_info);
                    (*expression)->slice_expr.array    = array;
                    (*expression)->slice_expr.start    = start;
                    (*expression)->slice_expr.past_end = past_end;
                }
            }
            else if (EatToken(parser, Token_OpenParen, &whitespace_info.call_info.ws_open_paren))
            {
                AST_Node* proc = *expression;
                AST_Node* args = 0;
                
                if (!EatToken(parser, Token_CloseParen, &whitespace_info.call_info.ws_close_paren))
                {
                    if      (!ParseNamedValueList(parser, &args)) return false;
                    else if (!EatToken(parser, Token_CloseParen, &whitespace_info.call_info.ws_close_paren))
                    {
                        token = GetToken(parser);
                        ParserError(parser, ParseError_MissingParenAfterCallArgs, TextInterval_BetweenStartPoints(token.text, token.text));
                        return false;
                    }
                }
                
                *expression = PushNode(parser, AST_Call, line, col, whitespace_info);
                (*expression)->call_expr.proc = proc;
                (*expression)->call_expr.args = args;
            }
            else if (EatToken(parser, Token_PeriodOpenBrace, &whitespace_info.struct_literal_info.ws_period_brace))
            {
                AST_Node* type = *expression;
                AST_Node* args = 0;
                
                if (!EatToken(parser, Token_CloseBrace, &whitespace_info.struct_literal_info.ws_close_brace))
                {
                    if      (!ParseNamedValueList(parser, &args)) return false;
                    else if (!EatToken(parser, Token_CloseBrace, &whitespace_info.struct_literal_info.ws_close_brace))
                    {
                        token = GetToken(parser);
                        ParserError(parser, ParseError_MissingBraceAfterStructLitArgs, TextInterval_BetweenStartPoints(token.text, token.text));
                        return false;
                    }
                }
                
                *expression = PushNode(parser, AST_StructLiteral, line, col, whitespace_info);
                (*expression)->struct_literal.type = type;
                (*expression)->struct_literal.args = args;
            }
            else if (EatToken(parser, Token_PeriodOpenBracket, &whitespace_info.array_literal_info.ws_period_bracket))
            {
                AST_Node* type = *expression;
                AST_Node* args = 0;
                
                if (!EatToken(parser, Token_CloseBracket, &whitespace_info.array_literal_info.ws_close_bracket))
                {
                    if      (!ParseNamedValueList(parser, &args)) return false;
                    else if (!EatToken(parser, Token_CloseBracket, &whitespace_info.array_literal_info.ws_close_bracket))
                    {
                        token = GetToken(parser);
                        ParserError(parser, ParseError_MissingBraceAfterArrayLitArgs, TextInterval_BetweenStartPoints(token.text, token.text));
                        return false;
                    }
                }
                
                *expression = PushNode(parser, AST_ArrayLiteral, line, col, whitespace_info);
                (*expression)->array_literal.type = type;
                (*expression)->array_literal.args = args;
            }
            else break;
            
            token = GetToken(parser);
            line  = token.text.line;
            col   = token.text.column;
            ZeroStruct(&whitespace_info);
        }
    }
    
    return true;
}

internal bool
ParsePrefixExpression(Parser* parser, AST_Node** expression)
{
    while (true)
    {
        Token token = GetToken(parser);
        u32 line = token.text.line;
        u32 col  = token.text.column;
        AST_Whitespace_Info whitespace_info = {0};
        
        if      (EatToken(parser, Token_Plus, &whitespace_info.ws_before));
        else if (EatToken(parser, Token_Minus, &whitespace_info.ws_before)) expression = &(*expression = PushNode(parser, AST_Neg, line, col, whitespace_info))->unary_expr;
        else if (EatToken(parser, Token_Bang, &whitespace_info.ws_before))  expression = &(*expression = PushNode(parser, AST_Not, line, col, whitespace_info))->unary_expr;
        else if (EatToken(parser, Token_Not, &whitespace_info.ws_before))   expression = &(*expression = PushNode(parser, AST_BitNot, line, col, whitespace_info))->unary_expr;
        else if (EatToken(parser, Token_Hat, &whitespace_info.ws_before))   expression = &(*expression = PushNode(parser, AST_Reference, line, col, whitespace_info))->unary_expr;
        else if (EatToken(parser, Token_OpenBracket, &whitespace_info.ws_before))
        {
            if (EatToken(parser, Token_CloseBracket, &whitespace_info.slice_type_info.ws_close_bracket))
            {
                expression = &(*expression = PushNode(parser, AST_SliceType, line, col, whitespace_info))->unary_expr;
            }
            else
            {
                AST_Node* size;
                if      (ParseExpression(parser, &size)) return false;
                else if (!EatToken(parser, Token_CloseBracket, &whitespace_info.array_type_info.ws_close_bracket))
                {
                    token = GetToken(parser);
                    ParserError(parser, ParseError_MissingBracketAfterArrayTypeSize, TextInterval_BetweenStartPoints(token.text, token.text));
                    return false;
                }
                
                AST_Node* array_type = PushNode(parser, AST_ArrayType, line, col, whitespace_info);
                array_type->array_type.size = size;
                expression                  = &array_type->array_type.elem_type;
            }
        }
        else return ParsePostfixExpression(parser, expression);
    }
    
    return true;
}

internal bool
ParseBinaryExpression(Parser* parser, AST_Node** expression)
{
    Token token = GetToken(parser);
    u32 line = token.text.line;
    u32 col  = token.text.column;
    AST_Whitespace_Info whitespace_info = { .ws_before = WhitespaceInfo_FromToken(parser->file_id, token) };
    
    if (!ParsePrefixExpression(parser, expression)) return false;
    else
    {
        token = GetToken(parser);
        
        while (true)
        {
            if (token.kind < Token_FirstBinary || token.kind > Token_LastBinary) break;
            else                                                                 NextToken(parser, &whitespace_info.binary_info.ws_op);
            
            AST_Node** left = expression;
            AST_Node* right;
            
            if (!ParsePrefixExpression(parser, &right)) return false;
            
            umm precedence = token.kind / TOKEN_KIND_BLOCK_SIZE;
            
            while ((*left)->kind / TOKEN_KIND_BLOCK_SIZE > precedence) left = &(*left)->binary_expr.right;
            
            AST_Node* new_expression = PushNode(parser, token.kind, line, col, whitespace_info);
            new_expression->binary_expr.left  = *left;
            new_expression->binary_expr.right = right;
            
            *left = new_expression;
            
            token = GetToken(parser);
            line = token.text.line;
            col  = token.text.column;
            whitespace_info = (AST_Whitespace_Info){ .ws_before = WhitespaceInfo_FromToken(parser->file_id, token) };
        }
    }
    
    return true;
}

internal bool
ParseExpression(Parser* parser, AST_Node** expression)
{
    Token token = GetToken(parser);
    u32 line = token.text.line;
    u32 col  = token.text.column;
    AST_Whitespace_Info whitespace_info = { .ws_before = WhitespaceInfo_FromToken(parser->file_id, token) };
    
    if      (!ParseBinaryExpression(parser, expression)) return false;
    else if (EatToken(parser, Token_QuestionMark, &whitespace_info.conditional_info.ws_qmark))
    {
        AST_Node* condition  = *expression;
        AST_Node* true_expr;
        AST_Node* false_expr;
        
        if      (!ParseBinaryExpression(parser, &true_expr)) return false;
        else if (!EatToken(parser, Token_Colon, &whitespace_info.conditional_info.ws_colon))
        {
            ParserError(parser, ParseError_MissingConditionalFalseExpr, TextInterval_BetweenStartPoints(token.text, GetToken(parser).text));
            return false;
        }
        else if (!ParseBinaryExpression(parser, &false_expr)) return false;
        
        *expression = PushNode(parser, AST_Conditional, line, col, whitespace_info);
        (*expression)->conditional_expr.condition  = condition;
        (*expression)->conditional_expr.true_expr  = true_expr;
        (*expression)->conditional_expr.false_expr = false_expr;
    }
    
    return true;
}

internal bool
ParseStatement(Parser* parser, AST_Node** statement)
{
    Token token = GetToken(parser);
    u32 line = token.text.line;
    u32 col  = token.text.column;
    AST_Whitespace_Info whitespace_info = { .ws_before = WhitespaceInfo_FromToken(parser->file_id, token) };
    
    Token peek      = PeekToken(parser, 1);
    Token peek_next = PeekToken(parser, 2);
    
    bool token_is_identifier = (token.kind == Token_Identifier);
    
    if (token.kind == Token_Semicolon)
    {
        ParserError(parser, ParseError_MissingStatement, TextInterval_BetweenStartPoints(token.text, token.text));
        return false;
    }
    else if (token.kind == Token_OpenBrace)
    {
        if (!ParseBlock(parser, statement)) return false;
    }
    else if (token_is_identifier && peek.kind == Token_Colon && (peek_next.kind == Token_OpenBrace ||
                                                                 peek_next.kind == Token_Identifier && (peek_next.string == Keyword_If || peek_next.string == Keyword_While ||
                                                                                                        peek_next.string == Keyword_Else)))
    {
        if (InternedString_IsKeyword(token.string))
        {
            ParserError(parser, ParseError_KeywordAsLabel, token.text);
            return false;
        }
        else if (token.string == BLANK_IDENTIFIER)
        {
            ParserError(parser, ParseError_BlankLabel, token.text);
            return false;
        }
        else if (peek_next.kind == Token_Identifier && peek_next.string == Keyword_Else)
        {
            ParserError(parser, ParseError_LabelElse, peek_next.text);
            return false;
        }
        
        Interned_String label = token.string;
        
        Whitespace_Info tmp_label;
        Whitespace_Info tmp_colon;
        NextToken(parser, &tmp_label);
        NextToken(parser, &tmp_colon);
        
        if (!ParseStatement(parser, statement)) return false;
        
        (*statement)->info->line      = line;
        (*statement)->info->col       = col;
        (*statement)->info->ws_before = whitespace_info.ws_before;
        
        if ((*statement)->kind == AST_Block)
        {
            (*statement)->block_statement.label     = label;
            (*statement)->info->block_info.ws_colon = tmp_colon;
        }
        else if ((*statement)->kind == AST_If)
        {
            (*statement)->if_statement.label     = label;
            (*statement)->info->if_info.ws_colon = tmp_colon;
        }
        else if ((*statement)->kind == AST_While)
        {
            (*statement)->while_statement.label     = label;
            (*statement)->info->while_info.ws_colon = tmp_colon;
        }
        else INVALID_CODE_PATH;
    }
    else if (token_is_identifier && token.string == Keyword_If)
    {
        NextToken(parser, &whitespace_info.ws_before);
        
        if (!EatToken(parser, Token_OpenParen, &whitespace_info.if_info.ws_open_paren))
        {
            token = GetToken(parser);
            ParserError(parser, ParseError_MissingIfCondition, TextInterval_BetweenStartPoints(token.text, token.text));
            return false;
        }
        
        AST_Node* init       = 0;
        AST_Node* condition  = 0;
        AST_Node* true_body  = 0;
        AST_Node* false_body = 0;
        
        token = GetToken(parser);
        if (token.kind != Token_Semicolon && !ParseAssignmentDeclarationOrExpression(parser, &condition)) return false;
        else if (EatToken(parser, Token_Semicolon, &whitespace_info.if_info.ws_init_semi))
        {
            init = condition;
            if (!ParseAssignmentDeclarationOrExpression(parser, &condition)) return false;
        }
        
        if (!EatToken(parser, Token_CloseParen, &whitespace_info.if_info.ws_close_paren))
        {
            token = GetToken(parser);
            ParserError(parser, ParseError_MissingParenAfterIfCondition, TextInterval_BetweenStartPoints(token.text, token.text));
            return false;
        }
        
        if (!EatToken(parser, Token_Semicolon, &whitespace_info.if_info.ws_if_body_semi) && !ParseStatement(parser, &true_body)) return false;
        
        token = GetToken(parser);
        if (token.kind == Token_Identifier && token.string == Keyword_Else)
        {
            NextToken(parser, &whitespace_info.if_info.ws_else);
            if (!EatToken(parser, Token_Semicolon, &whitespace_info.if_info.ws_else_body_semi) && !ParseStatement(parser, &false_body)) return false;
        }
        
        *statement = PushNode(parser, AST_If, line, col, whitespace_info);
        (*statement)->if_statement.init       = init;
        (*statement)->if_statement.condition  = condition;
        (*statement)->if_statement.true_body  = true_body;
        (*statement)->if_statement.false_body = false_body;
        (*statement)->if_statement.label      = INTERNED_STRING_NIL;
    }
    else if (token_is_identifier && token.string == Keyword_Else)
    {
        ParserError(parser, ParseError_IsolatedElse, token.text);
        return false;
    }
    else if (token_is_identifier && token.string == Keyword_While)
    {
        NextToken(parser, &whitespace_info.ws_before);
        
        if (!EatToken(parser, Token_OpenParen, &whitespace_info.while_info.ws_open_paren))
        {
            token = GetToken(parser);
            ParserError(parser, ParseError_MissingWhileCondition, TextInterval_BetweenStartPoints(token.text, token.text));
            return false;
        }
        
        AST_Node* init      = 0;
        AST_Node* condition = 0;
        AST_Node* step      = 0;
        AST_Node* body      = 0;
        
        token = GetToken(parser);
        if      (token.kind != Token_Semicolon && !ParseAssignmentDeclarationOrExpression(parser, &condition)) return false;
        else if (EatToken(parser, Token_Semicolon, &whitespace_info.while_info.ws_init_semi))
        {
            init = condition;
            
            if      (!ParseAssignmentDeclarationOrExpression(parser, &condition))                                                                           return false;
            else if (EatToken(parser, Token_Semicolon, &whitespace_info.while_info.ws_cond_semi) && !ParseAssignmentDeclarationOrExpression(parser, &step)) return false;
        }
        
        if (!EatToken(parser, Token_CloseParen, &whitespace_info.while_info.ws_close_paren))
        {
            token = GetToken(parser);
            ParserError(parser, ParseError_MissingParenAfterWhileHeader, TextInterval_BetweenStartPoints(token.text, token.text));
            return false;
        }
        else if (!EatToken(parser, Token_Semicolon, &whitespace_info.while_info.ws_body_semi) && !ParseStatement(parser, &body)) return false;
        
        *statement = PushNode(parser, AST_While, line, col, whitespace_info);
        (*statement)->while_statement.init      = init;
        (*statement)->while_statement.condition = condition;
        (*statement)->while_statement.step      = step;
        (*statement)->while_statement.body      = body;
        (*statement)->while_statement.label     = INTERNED_STRING_NIL;
    }
    else if (token_is_identifier && (token.string == Keyword_Break || token.string == Keyword_Continue))
    {
        AST_NODE_KIND kind    = (token.string == Keyword_Break);
        Interned_String label = INTERNED_STRING_NIL;
        
        token = NextToken(parser, &whitespace_info.ws_before);
        if (token.kind == Token_Identifier)
        {
            if (!InternedString_IsNonBlankIdentifier(token.string))
            {
                ParserError(parser, ParseError_IllegalLabel, token.text);
                return false;
            }
            
            label = token.string;
            NextToken(parser, &whitespace_info.jump_info.ws_label);
        }
        
        *statement = PushNode(parser, kind, line, col, whitespace_info);
        (*statement)->jump_label = label;
    }
    else if (token_is_identifier && token.string == Keyword_Defer)
    {
        NextToken(parser, &whitespace_info.ws_before);
        
        AST_Node* defer_statement = 0;
        if (!ParseStatement(parser, &defer_statement)) return false;
        
        *statement = PushNode(parser, AST_Defer, line, col, whitespace_info);
        (*statement)->defer_statement = defer_statement;
    }
    else if (token_is_identifier && token.string == Keyword_Return)
    {
        token = NextToken(parser, &whitespace_info.ws_before);
        
        AST_Node* return_values = 0;
        if (token.kind != Token_Semicolon && !ParseNamedValueList(parser, &return_values)) return false;
        
        *statement = PushNode(parser, AST_Return, line, col, whitespace_info);
        (*statement)->return_values = return_values;
    }
    else if (!ParseAssignmentDeclarationOrExpression(parser, statement)) return false;
    
    if (AST_StatementNeedsSemicolon(*statement) && !EatToken(parser, Token_Semicolon, &(*statement)->info->ws_terminator))
    {
        token = GetToken(parser);
        ParserError(parser, ParseError_MissingSemicolon, TextInterval_BetweenStartPoints(token.text, token.text));
        return false;
    }
    
    return true;
}


internal bool
ParseBlock(Parser* parser, AST_Node** block)
{
    Token token = GetToken(parser);
    u32 line = token.text.line;
    u32 col  = token.text.column;
    AST_Whitespace_Info whitespace_info = {0};
    
    ASSERT(token.kind == Token_OpenBrace);
    NextToken(parser, &whitespace_info.ws_before);
    
    AST_Node* body = 0;
    
    AST_Node** next_statement = &body;
    while (!EatToken(parser, Token_CloseBrace, &whitespace_info.block_info.ws_close_brace))
    {
        if (!ParseStatement(parser, next_statement)) return false;
        next_statement = &(*next_statement)->next;
    }
    
    *block = PushNode(parser, AST_Block, line, col, whitespace_info);
    (*block)->block_statement.label = INTERNED_STRING_NIL;
    (*block)->block_statement.body  = body;
    
    return true;
}

internal bool
ParseFile(Workspace* workspace, File* file)
{
    Parser parser = {
        .workspace = workspace,
        .lexer = Lexer_Init(workspace, file->content.data), // NOTE: Files guarantee a null terminated string
        .file_id = file->id,
    };
    
    parser.peek[parser.peek_cursor + 0] = Lexer_Advance(workspace, &parser.lexer);
    parser.peek[parser.peek_cursor + 1] = Lexer_Advance(workspace, &parser.lexer);
    parser.peek[parser.peek_cursor + 2] = Lexer_Advance(workspace, &parser.lexer);
    
    AST_Node** next_statement = workspace->tail_ast;
    while (true)
    {
        Token token = GetToken(&parser);
        
        if (token.kind == Token_EndOfStream) break;
        else
        {
            if (!ParseStatement(&parser, next_statement)) return false;
            next_statement = &(*next_statement)->next;
        }
    }
    
    return true;
}
