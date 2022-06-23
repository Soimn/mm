typedef struct MM_Parser MM_Parser;
MM_TYPEDEF_FUNC(MM_Token, MM_Parser_GetToken_Func, MM_Parser* parser);
MM_TYPEDEF_FUNC(MM_Token, MM_Parser_NextToken_Func, MM_Parser* parser);
MM_TYPEDEF_FUNC(MM_String, MM_Parser_TokenToString_Func, MM_Parser* parser, MM_Token token);

typedef struct MM_Parser
{
    MM_Parser_GetToken_Func get_token;
    MM_Parser_GetToken_Func next_token;
    MM_Parser_TokenToString_Func token_to_string;
    MM_Arena* ast_arena;
    MM_Arena* str_arena;
    MM_Parser_Error* report;
    
    union
    {
        MM_Lexer* lexer;
        
        struct
        {
            MM_Token* tokens;
            MM_u8* string_base;
            MM_u32 count;
            MM_u32 cursor;
        };
    };
} MM_Parser;

#define GetToken(parser) (parser)->get_token(parser)
#define NextToken(parser) (parser)->next_token(parser)
#define EatToken(parser, kind) MM_Parser__EatToken((parser), (kind))
#define TokenToString(parser, token) (parser)->token_to_string((parser), (token))

MM_Internal MM_Token
MM_Parser__GetTokenLexer(MM_Parser* parser)
{
    if (parser->lexer->offset == 0) return MM_Lexer_NextToken(parser->lexer);
    else                            return parser->lexer->last_token;
}

MM_Internal MM_Token
MM_Parser__NextTokenLexer(MM_Parser* parser)
{
    return MM_Lexer_NextToken(parser->lexer);
}

MM_Internal MM_String
MM_Parser__TokenToStringLexer(MM_Parser* parser, MM_Token token)
{
    return MM_Lexer_TokenToString(parser->lexer, token);
}

MM_Internal MM_Token
MM_Parser__GetTokenTokens(MM_Parser* parser)
{
    if (parser->cursor < parser->count) return parser->tokens[parser->cursor];
    else
    {
        if (parser->count == 0) return (MM_Token){ .kind = MM_Token_EndOfStream };
        else
        {
            MM_Token prev_token = parser->tokens[parser->cursor - 1];
            
            return (MM_Token){
                .kind              = MM_Token_EndOfStream,
                .preceding_spacing = 0,
                .offset            = prev_token.offset + prev_token.length,
                .line              = prev_token.line,
                .column            = prev_token.column + prev_token.length,
                .length            = 0,
            };
        }
    }
}

MM_Internal MM_Token
MM_Parser__NextTokenTokens(MM_Parser* parser)
{
    parser->cursor += 1;
    return MM_Parser__GetTokenTokens(parser);
}

MM_Internal MM_String
MM_Parser__TokenToStringTokens(MM_Parser* parser, MM_Token token)
{
    return (MM_String){
        .data = parser->string_base + token.offset,
        .size = token.length
    };
}

MM_Internal MM_bool
MM_Parser__EatToken(MM_Parser* parser, MM_TOKEN_KIND kind)
{
    if (GetToken(parser).kind != kind) return MM_false;
    else
    {
        NextToken(parser);
        return MM_true;
    }
}

#define WithLexerParser {                                  \
.get_token       = &MM_Parser__GetTokenLexer,      \
.next_token      = &MM_Parser__NextTokenLexer,     \
.token_to_string = &MM_Parser__TokenToStringLexer, \
.ast_arena       = ast_arena,                      \
.str_arena       = str_arena,                      \
.lexer           = lexer,                          \
}

#define FromTokensParser {                                  \
.get_token       = &MM_Parser__GetTokenLexer,       \
.next_token      = &MM_Parser__NextTokenLexer,      \
.token_to_string = &MM_Parser__TokenToStringTokens, \
.ast_arena       = ast_arena,                       \
.str_arena       = str_arena,                       \
.tokens          = tokens.tokens,                   \
.string_base     = tokens.string_base,              \
.count           = tokens.count,                    \
.cursor          = 0,                               \
}

#define FromStringParser {                                 \
.get_token       = &MM_Parser__GetTokenLexer,      \
.next_token      = &MM_Parser__NextTokenLexer,     \
.token_to_string = &MM_Parser__TokenToStringLexer, \
.ast_arena       = ast_arena,                      \
.str_arena       = str_arena,                      \
.lexer           = &lexer,                         \
}

MM_Internal MM_bool MM_Parser__ParseExpression(MM_Parser* parser, MM_Expression** expression);
MM_Internal MM_bool MM_Parser__ParseStatement(MM_Parser* parser, MM_Statement** statement);
MM_Internal MM_bool MM_Parser__ParseAssignmentOrExpression(MM_Parser* parser, MM_AssignmentOrExpression** result);
MM_Internal MM_bool MM_Parser__ParseDeclAssignmentOrExpression(MM_Parser* parser, MM_DeclAssignmentOrExpression** result);

MM_API MM_Expression*
MM_Parser_ParseExpressionWithLexer(MM_Lexer* lexer, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Parser parser = WithLexerParser;
    MM_Expression* expr = 0;
    return (MM_Parser__ParseExpression(&parser, &expr) ? expr : 0);
}

MM_API MM_Statement*
MM_Parser_ParseStatementWithLexer(MM_Lexer* lexer, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Parser parser = WithLexerParser;
    MM_Statement* statement = 0;
    return (MM_Parser__ParseStatement(&parser, &statement) ? statement : 0);
}

MM_API MM_Statement*
MM_Parser_ParseStatementListWithLexer(MM_Lexer* lexer, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Parser parser = WithLexerParser;
    
    MM_Statement* statements = 0;
    for (MM_Statement** next_statement = &statements;
         GetToken(&parser).kind != MM_Token_EndOfStream;
         next_statement = &(*next_statement)->next)
    {
        if (!MM_Parser__ParseStatement(&parser, next_statement)) return 0;
        else                                                    continue;
    }
    
    return statements;
}

MM_API MM_AssignmentOrExpression*
MM_Parser_ParseAssignmentOrExpressionWithLexer(MM_Lexer* lexer, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Parser parser = WithLexerParser;
    MM_AssignmentOrExpression* result = 0;
    return (MM_Parser__ParseAssignmentOrExpression(&parser, &result) ? result : 0);
}

MM_API MM_DeclAssignmentOrExpression*
MM_Parser_ParseDeclAssignmentOrExpressionWithLexer(MM_Lexer* lexer, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Parser parser = WithLexerParser;
    MM_DeclAssignmentOrExpression* result = 0;
    return (MM_Parser__ParseDeclAssignmentOrExpression(&parser, &result) ? result : 0);
}

MM_API MM_Expression*
MM_Parser_ParseExpressionFromTokens(MM_Token_Array tokens, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Parser parser = FromTokensParser;
    
    MM_Expression* expr = 0;
    if (!MM_Parser__ParseExpression(&parser, &expr)) return 0;
    else if (GetToken(&parser).kind != MM_Token_EndOfStream)
    {
        //// ERROR: trailing tokens
        MM_NOT_IMPLEMENTED;
        return 0;
    }
    else return expr;
}

MM_API MM_Statement*
MM_Parser_ParseStatementFromTokens(MM_Token_Array tokens, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Parser parser = FromTokensParser;
    
    MM_Statement* statement = 0;
    if (!MM_Parser__ParseStatement(&parser, &statement)) return 0;
    else if (GetToken(&parser).kind != MM_Token_EndOfStream)
    {
        //// ERROR: trailing tokens
        MM_NOT_IMPLEMENTED;
        return 0;
    }
    else return statement;
}

MM_API MM_Statement*
MM_Parser_ParseStatementListFromTokens(MM_Token_Array tokens, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Parser parser = FromTokensParser;
    
    MM_Statement* statements = 0;
    for (MM_Statement** next_statement = &statements;
         GetToken(&parser).kind != MM_Token_EndOfStream;
         next_statement = &(*next_statement)->next)
    {
        if (!MM_Parser__ParseStatement(&parser, next_statement)) return 0;
        else                                                    continue;
    }
    
    return statements;
}

MM_API MM_AssignmentOrExpression*
MM_Parser_ParseAssignmentOrExpressionFromTokens(MM_Token_Array tokens, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Parser parser = FromTokensParser;
    
    MM_AssignmentOrExpression* result = 0;
    if (!MM_Parser__ParseAssignmentOrExpression(&parser, &result)) return 0;
    else if (GetToken(&parser).kind != MM_Token_EndOfStream)
    {
        //// ERROR: trailing tokens
        MM_NOT_IMPLEMENTED;
        return 0;
    }
    else return result;
}

MM_API MM_DeclAssignmentOrExpression*
MM_Parser_ParseDeclAssignmentOrExpressionFromTokens(MM_Token_Array tokens, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Parser parser = FromTokensParser;
    
    MM_DeclAssignmentOrExpression* result = 0;
    if (!MM_Parser__ParseDeclAssignmentOrExpression(&parser, &result)) return 0;
    else if (GetToken(&parser).kind != MM_Token_EndOfStream)
    {
        //// ERROR: trailing tokens
        MM_NOT_IMPLEMENTED;
        return 0;
    }
    else return result;
}

MM_API MM_Expression*
MM_Parser_ParseExpressionFromString(MM_String string, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Lexer lexer   = MM_Lexer_Init(string);
    MM_Parser parser = FromStringParser;
    
    MM_Expression* expr = 0;
    if (!MM_Parser__ParseExpression(&parser, &expr)) return 0;
    else if (GetToken(&parser).kind != MM_Token_EndOfStream)
    {
        //// ERROR: trailing tokens
        MM_NOT_IMPLEMENTED;
        return 0;
    }
    else return expr;
}

MM_API MM_Statement*
MM_Parser_ParseStatementFromString(MM_String string, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Lexer lexer   = MM_Lexer_Init(string);
    MM_Parser parser = FromStringParser;
    
    MM_Statement* statement = 0;
    if (!MM_Parser__ParseStatement(&parser, &statement)) return 0;
    else if (GetToken(&parser).kind != MM_Token_EndOfStream)
    {
        //// ERROR: trailing tokens
        MM_NOT_IMPLEMENTED;
        return 0;
    }
    else return statement;
}

MM_API MM_Statement*
MM_Parser_ParseStatementListFromString(MM_String string, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Lexer lexer   = MM_Lexer_Init(string);
    MM_Parser parser = FromStringParser;
    
    MM_Statement* statements = 0;
    for (MM_Statement** next_statement = &statements;
         GetToken(&parser).kind != MM_Token_EndOfStream;
         next_statement = &(*next_statement)->next)
    {
        if (!MM_Parser__ParseStatement(&parser, next_statement)) return 0;
        else                                                    continue;
    }
    
    return statements;
}

MM_API MM_AssignmentOrExpression*
MM_Parser_ParseAssignmentOrExpressionFromString(MM_String string, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Lexer lexer   = MM_Lexer_Init(string);
    MM_Parser parser = FromStringParser;
    
    MM_AssignmentOrExpression* result = 0;
    if (!MM_Parser__ParseAssignmentOrExpression(&parser, &result)) return 0;
    else if (GetToken(&parser).kind != MM_Token_EndOfStream)
    {
        //// ERROR: trailing tokens
        MM_NOT_IMPLEMENTED;
        return 0;
    }
    else return result;
}

MM_API MM_DeclAssignmentOrExpression*
MM_Parser_ParseDeclAssignmentOrExpressionFromString(MM_String string, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Parser_Error* report)
{
    MM_Lexer lexer   = MM_Lexer_Init(string);
    MM_Parser parser = FromStringParser;
    
    MM_DeclAssignmentOrExpression* result = 0;
    if (!MM_Parser__ParseDeclAssignmentOrExpression(&parser, &result)) return 0;
    else if (GetToken(&parser).kind != MM_Token_EndOfStream)
    {
        //// ERROR: trailing tokens
        MM_NOT_IMPLEMENTED;
        return 0;
    }
    else return result;
}

MM_Internal void*
MM_Parser__PushNode(MM_Parser* parser, MM_AST_KIND kind)
{
    MM_AST* node = MM_Arena_Push(parser->ast_arena, sizeof(MM_AST), MM_ALIGNOF(MM_AST));
    MM_ZeroStruct(node);
    node->kind = kind;
    
    return (void*)node;
}

MM_Internal MM_bool
MM_Parser__ParseExpressionList(MM_Parser* parser, MM_Expression** expression)
{
    for (MM_Expression** next_expression = expression;; next_expression = &(*next_expression)->next)
    {
        if      (!MM_Parser__ParseExpression(parser, next_expression)) return MM_false;
        else if (EatToken(parser, MM_Token_Comma))                     continue;
        else                                                           break;
    }
    
    return MM_true;
}

MM_Internal MM_bool
MM_Parser__ParseArgumentList(MM_Parser* parser, MM_Argument** args)
{
    for (MM_Argument** next_arg = args;; next_arg = &(*next_arg)->next)
    {
        MM_Expression* name  = 0;
        MM_Expression* value = 0;
        if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
        
        if (EatToken(parser, MM_Token_Equals))
        {
            name = value;
            if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
        }
        
        *next_arg = MM_Parser__PushNode(parser, MM_AST_Argument);
        (*next_arg)->name  = name;
        (*next_arg)->value = value;
        
        if (EatToken(parser, MM_Token_Comma)) continue;
        else                                  break;
    }
    
    return MM_true;
}

MM_Internal MM_bool
MM_Parser__ParseParameterList(MM_Parser* parser, MM_Parameter** params)
{
    MM_bool is_using = EatToken(parser, MM_Token_Using);
    
    MM_Expression* list = 0;
    if (!MM_Parser__ParseExpressionList(parser, &list)) return MM_false;
    
    if (GetToken(parser).kind != MM_Token_Colon && GetToken(parser).kind != MM_Token_Equals)
    {
        if (is_using)
        {
            //// ERROR: using cannot be applied to unnamed parameters
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        MM_Parameter** next_param = params;
        for (MM_Expression* expr = list; expr != 0; expr = expr->next, next_param = &(*next_param)->next)
        {
            *next_param = MM_Parser__PushNode(parser, MM_AST_Parameter);
            (*next_param)->type = expr;
        }
    }
    else
    {
        MM_Expression* names = list;
        
        goto skip_parsing_names;
        for (MM_Parameter** next_param = params;; next_param = &(*next_param)->next)
        {
            is_using = EatToken(parser, MM_Token_Using);
            
            names = 0;
            if (!MM_Parser__ParseExpressionList(parser, &names)) return MM_false;
            skip_parsing_names:;
            
            if (!EatToken(parser, MM_Token_Colon))
            {
                //// ERROR: Missing type
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            MM_Expression* type = 0;
            if (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
            
            MM_Expression* value = 0;
            if (EatToken(parser, MM_Token_Equals) && !MM_Parser__ParseExpression(parser, &value)) return MM_false;
            
            *next_param = MM_Parser__PushNode(parser, MM_AST_Parameter);
            (*next_param)->names    = names;
            (*next_param)->type     = type;
            (*next_param)->value    = value;
            (*next_param)->is_using = is_using;
            
            if (EatToken(parser, MM_Token_Comma)) continue;
            else                                  break;
        }
    }
    
    return MM_true;
}

MM_Internal MM_bool
MM_Parser__ParseBlockStatement(MM_Parser* parser, MM_Block_Statement** block)
{
    MM_Statement* statements = 0;
    for (MM_Statement** next_statement = &statements;; next_statement = &(*next_statement)->next)
    {
        if      (EatToken(parser, MM_Token_CloseBrace))              break;
        else if (!MM_Parser__ParseStatement(parser, next_statement)) return MM_false;
    }
    
    *block = MM_Parser__PushNode(parser, MM_AST_Block);
    (*block)->body = statements;
    
    return MM_true;
}

MM_Internal MM_bool
MM_Parser__ParsePrimaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    MM_Token token = GetToken(parser);
    
    if (token.kind == MM_Token_Identifier || token.kind == MM_Token_BlankIdentifier)
    {
        NextToken(parser);
        
        *expression = MM_Parser__PushNode(parser, MM_AST_Identifier);
        if (token.kind == MM_Token_Identifier) (*expression)->identifier = TokenToString(parser, token);
        else                                   (*expression)->identifier = (MM_String){};
    }
    else if (token.kind == MM_Token_Int)
    {
        NextToken(parser);
        
        *expression = MM_Parser__PushNode(parser, MM_AST_Int);
        (*expression)->integer = MM_Lexer_ParseIntFromString(TokenToString(parser, token));
    }
    else if (token.kind == MM_Token_Float)
    {
        NextToken(parser);
        
        *expression = MM_Parser__PushNode(parser, MM_AST_Float);
        (*expression)->floating = MM_Lexer_ParseFloatFromString(TokenToString(parser, token));
    }
    else if (token.kind == MM_Token_String)
    {
        NextToken(parser);
        
        MM_String raw_string = TokenToString(parser, token);
        
        *expression = MM_Parser__PushNode(parser, MM_AST_String);
        (*expression)->string_literal = MM_Lexer_ParseStringFromString(raw_string, MM_Arena_Push(parser->str_arena, raw_string.size, MM_ALIGNOF(MM_u8)));
        
        MM_Arena_Pop(parser->str_arena, (*expression)->string_literal.size - raw_string.size);
    }
    else if (token.kind == MM_Token_Codepoint)
    {
        NextToken(parser);
        
        *expression = MM_Parser__PushNode(parser, MM_AST_Codepoint);
        (*expression)->codepoint = MM_Lexer_ParseCodepointFromString(TokenToString(parser, token));
    }
    else if (token.kind == MM_Token_True || token.kind == MM_Token_False)
    {
        NextToken(parser);
        
        *expression = MM_Parser__PushNode(parser, MM_AST_Bool);
        (*expression)->boolean = (token.kind == MM_Token_True);
    }
    else if (token.kind == MM_Token_Proc)
    {
        MM_Parameter* params        = 0;
        MM_Parameter* return_values = 0;
        
        NextToken(parser);
        
        if (EatToken(parser, MM_Token_OpenParen))
        {
            if (GetToken(parser).kind == MM_Token_CloseParen)
            {
                //// ERROR: Missing parameters
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            if (!MM_Parser__ParseParameterList(parser, &params)) return MM_false;
            
            if (!EatToken(parser, MM_Token_CloseParen))
            {
                //// ERROR: Missing closing paren after parameters in procedure header
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
        }
        
        if (EatToken(parser, MM_Token_Arrow))
        {
            if (!EatToken(parser, MM_Token_OpenParen))
            {
                MM_Expression* type = 0;
                if (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
                
                return_values = MM_Parser__PushNode(parser, MM_AST_Parameter);
                return_values->type = type;
            }
            else
            {
                if (GetToken(parser).kind == MM_Token_CloseParen)
                {
                    //// ERROR: Missing return values
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                
                if (!MM_Parser__ParseParameterList(parser, &return_values)) return MM_false;
                
                if (EatToken(parser, MM_Token_CloseParen))
                {
                    //// ERROR: Missing closing paren after return value list in procedure header
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
        }
        
        if (EatToken(parser, MM_Token_TripleMinus))
        {
            *expression = MM_Parser__PushNode(parser, MM_AST_Proc);
            (*expression)->proc_lit_expr.params        = params;
            (*expression)->proc_lit_expr.return_values = return_values;
            (*expression)->proc_lit_expr.body          = 0;
        }
        else if (GetToken(parser).kind == MM_Token_CloseParen)
        {
            MM_Block_Statement* body = 0;
            if (!MM_Parser__ParseBlockStatement(parser, &body)) return MM_false;
            
            *expression = MM_Parser__PushNode(parser, MM_AST_Proc);
            (*expression)->proc_lit_expr.params        = params;
            (*expression)->proc_lit_expr.return_values = return_values;
            (*expression)->proc_lit_expr.body          = body;
        }
        else
        {
            *expression = MM_Parser__PushNode(parser, MM_AST_Proc);
            (*expression)->proc_expr.params        = params;
            (*expression)->proc_expr.return_values = return_values;
        }
    }
    else if (token.kind == MM_Token_Struct || token.kind == MM_Token_Union)
    {
        MM_AST_KIND kind = (token.kind == MM_Token_Struct ? MM_AST_Struct : MM_AST_Union);
        NextToken(parser);
        
        MM_Block_Statement* body = 0;
        if (!MM_Parser__ParseBlockStatement(parser, &body)) return MM_false;
        
        *expression = MM_Parser__PushNode(parser, kind);
        (*expression)->struct_expr.body = body;
    }
    else if (token.kind == MM_Token_Enum)
    {
        NextToken(parser);
        
        MM_Expression* member_type = 0;
        MM_Enum_Member* members    = 0;
        
        if (!EatToken(parser, MM_Token_OpenBrace))
        {
            if (!MM_Parser__ParseExpression(parser, &member_type))
                
                if (!EatToken(parser, MM_Token_OpenBrace))
            {
                //// ERROR: Missing body of enum
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
        }
        
        if (GetToken(parser).kind != MM_Token_CloseBrace)
        {
            for (MM_Enum_Member** next_member = &members;; next_member = &(*next_member)->next)
            {
                token = GetToken(parser);
                
                if (token.kind == MM_Token_BlankIdentifier)
                {
                    //// ERROR: Illegal use of blank identifier as enum member name
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                if (token.kind != MM_Token_Identifier)
                {
                    //// ERROR: Missing name of enum member
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                
                MM_String name       = TokenToString(parser, token);
                MM_Expression* value = 0;
                
                NextToken(parser);
                
                if (EatToken(parser, MM_Token_Equals) && !MM_Parser__ParseExpression(parser, &value)) return MM_false;
                
                *next_member = MM_Parser__PushNode(parser, MM_AST_EnumMember);
                (*next_member)->name  = name;
                (*next_member)->value = value;
                
                if (EatToken(parser, MM_Token_Comma)) continue;
                else                                  break;
            }
        }
        
        if (!EatToken(parser, MM_Token_CloseBrace))
        {
            //// ERROR: Missing closing brace after members in enum body
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *expression = MM_Parser__PushNode(parser, MM_AST_Enum);
        (*expression)->enum_expr.member_type = member_type;
        (*expression)->enum_expr.members     = members;
    }
    else if (token.kind >= MM_Token_FirstBuiltin && token.kind <= MM_Token_LastBuiltin)
    {
        MM_Argument* args    = 0;
        MM_BUILTIN_KIND kind = (MM_BUILTIN_KIND)(token.kind - MM_Token_FirstBuiltin);
        NextToken(parser);
        
        if (!EatToken(parser, MM_Token_OpenParen))
        {
            //// ERROR: Missing parameters to builtin procedure call
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        if (!MM_Parser__ParseArgumentList(parser, &args)) return MM_false;
        
        if (!EatToken(parser, MM_Token_CloseParen))
        {
            //// ERROR: Missing close paren after arguments to builtin procedure call
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *expression = MM_Parser__PushNode(parser, MM_AST_Builtin);
        (*expression)->builtin_expr.kind = kind;
        (*expression)->builtin_expr.args = args;
    }
    else if (token.kind == MM_Token_OpenParen)
    {
        NextToken(parser);
        
        MM_Expression* compound_expr = 0;
        if (!MM_Parser__ParseExpression(parser, &compound_expr)) return MM_false;
        
        if (!EatToken(parser, MM_Token_CloseParen))
        {
            //// ERROR: Missing close paren after compound expression
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *expression = MM_Parser__PushNode(parser, MM_AST_Compound);
        (*expression)->compound_expr = compound_expr;
    }
    else if (EatToken(parser, MM_Token_PeriodOpenBrace))
    {
        MM_Token token = GetToken(parser);
        
        MM_Argument* args = 0;
        if (token.kind != MM_Token_CloseBrace && MM_Parser__ParseArgumentList(parser, &args)) return 0;
        
        if (token.kind != MM_Token_CloseBrace)
        {
            //// ERROR: Missing closing brace after arguments to struct literal
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *expression = MM_Parser__PushNode(parser, MM_AST_StructLiteral);
        (*expression)->struct_lit_expr.type = 0;
        (*expression)->struct_lit_expr.args = args;
    }
    else if (EatToken(parser, MM_Token_PeriodOpenBracket))
    {
        MM_Token token = GetToken(parser);
        
        MM_Argument* args = 0;
        if (token.kind != MM_Token_CloseBracket && MM_Parser__ParseArgumentList(parser, &args)) return 0;
        
        if (token.kind != MM_Token_CloseBracket)
        {
            //// ERROR: Missing closing bracket after arguments to array literal
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *expression = MM_Parser__PushNode(parser, MM_AST_ArrayLiteral);
        (*expression)->array_lit_expr.type = 0;
        (*expression)->array_lit_expr.args = args;
    }
    else
    {
        if (token.kind == MM_Token_Invalid)
        {
            // TODO: Lexer error
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            //// ERROR: Missing primary expression
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
    }
    
    return MM_true;
}

MM_Internal MM_bool
MM_Parser__ParsePostfixExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser__ParsePrimaryExpression(parser, expression)) return MM_false;
    
    while (MM_true)
    {
        MM_Expression* prefix = *expression;
        
        if (EatToken(parser, MM_Token_Hat))
        {
            *expression = MM_Parser__PushNode(parser, MM_AST_Dereference);
            (*expression)->unary_expr = prefix;
        }
        else if (EatToken(parser, MM_Token_Period))
        {
            MM_Token member = GetToken(parser);
            if (member.kind == MM_Token_BlankIdentifier)
            {
                //// ERROR: Name of member in member expression cannot be a blank identifier
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else if (member.kind != MM_Token_Identifier)
            {
                //// ERROR: Missing name of member to access in member expression
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            *expression = MM_Parser__PushNode(parser, MM_AST_MemberAccess);
            (*expression)->member_access_expr.symbol = prefix;
            (*expression)->member_access_expr.member = TokenToString(parser, member);
        }
        else if (EatToken(parser, MM_Token_OpenBracket))
        {
            MM_Expression* first = 0;
            if (GetToken(parser).kind != MM_Token_Colon && !MM_Parser__ParseExpression(parser, &first)) return MM_false;
            
            if (!EatToken(parser, MM_Token_Colon))
            {
                if (!EatToken(parser, MM_Token_CloseBracket))
                {
                    //// ERROR: Missing closing bracket after index in subscript expression
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                
                *expression = MM_Parser__PushNode(parser, MM_AST_Subscript);
                (*expression)->subscript_expr.array = prefix;
                (*expression)->subscript_expr.index = first;
            }
            else
            {
                MM_Expression* start_index    = first;
                MM_Expression* past_end_index = 0;
                
                if (GetToken(parser).kind != MM_Token_CloseBracket && !MM_Parser__ParseExpression(parser, &past_end_index)) return MM_false;
                
                if (!EatToken(parser, MM_Token_CloseBracket))
                {
                    //// ERROR: Missing closing bracket after past end index in slice expression
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                
                *expression = MM_Parser__PushNode(parser, MM_AST_Slice);
                (*expression)->slice_expr.array          = prefix;
                (*expression)->slice_expr.start_index    = start_index;
                (*expression)->slice_expr.past_end_index = past_end_index;
            }
        }
        else if (EatToken(parser, MM_Token_OpenParen))
        {
            MM_Argument* args = 0;
            if (GetToken(parser).kind != MM_Token_CloseParen && MM_Parser__ParseArgumentList(parser, &args)) return MM_false;
            
            if (!EatToken(parser, MM_Token_CloseParen))
            {
                //// ERROR: Missing closing paren after arguments to procedure call
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            *expression = MM_Parser__PushNode(parser, MM_AST_Call);
            (*expression)->call_expr.proc = prefix;
            (*expression)->call_expr.args = args;
        }
        else if (EatToken(parser, MM_Token_PeriodOpenBrace))
        {
            MM_Token token = GetToken(parser);
            
            MM_Argument* args = 0;
            if (token.kind != MM_Token_CloseBrace && MM_Parser__ParseArgumentList(parser, &args)) return 0;
            
            if (token.kind != MM_Token_CloseBrace)
            {
                //// ERROR: Missing closing brace after arguments to struct literal
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            *expression = MM_Parser__PushNode(parser, MM_AST_StructLiteral);
            (*expression)->struct_lit_expr.type = prefix;
            (*expression)->struct_lit_expr.args = args;
        }
        else if (EatToken(parser, MM_Token_PeriodOpenBracket))
        {
            MM_Token token = GetToken(parser);
            
            MM_Argument* args = 0;
            if (token.kind != MM_Token_CloseBracket && MM_Parser__ParseArgumentList(parser, &args)) return 0;
            
            if (token.kind != MM_Token_CloseBracket)
            {
                //// ERROR: Missing closing bracket after arguments to array literal
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            *expression = MM_Parser__PushNode(parser, MM_AST_ArrayLiteral);
            (*expression)->array_lit_expr.type = prefix;
            (*expression)->array_lit_expr.args = args;
        }
        else break;
    }
    
    return MM_true;
}

MM_Internal MM_bool
MM_Parser__ParsePrefixExpression(MM_Parser* parser, MM_Expression** expression)
{
    while (MM_true)
    {
        MM_Token token = GetToken(parser);
        
        if (token.kind == MM_Token_Hat)
        {
            *expression = MM_Parser__PushNode(parser, MM_AST_Reference);
            expression = &(*expression)->unary_expr;
        }
        else if (token.kind == MM_Token_Minus)
        {
            *expression = MM_Parser__PushNode(parser, MM_AST_Neg);
            expression = &(*expression)->unary_expr;
        }
        else if (token.kind == MM_Token_Tilde)
        {
            *expression = MM_Parser__PushNode(parser, MM_AST_BitNot);
            expression = &(*expression)->unary_expr;
        }
        else if (token.kind == MM_Token_Bang)
        {
            *expression = MM_Parser__PushNode(parser, MM_AST_Not);
            expression = &(*expression)->unary_expr;
        }
        else if (EatToken(parser, MM_Token_OpenBracket))
        {
            if (EatToken(parser, MM_Token_CloseBracket))
            {
                *expression = MM_Parser__PushNode(parser, MM_AST_SliceType);
                expression = &(*expression)->unary_expr;
            }
            else
            {
                MM_Expression* size = 0;
                if      (!MM_Parser__ParseExpression(parser, &size)) return MM_false;
                else if (!EatToken(parser, MM_Token_CloseBracket))
                {
                    //// ERROR: Missing closing bracket after size in array type prefix expression
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                
                *expression = MM_Parser__PushNode(parser, MM_AST_ArrayType);
                (*expression)->array_type_expr.size = size;
                
                expression = &(*expression)->array_type_expr.elem_type;
            }
        }
        else break;
    }
    
    return MM_Parser__ParsePostfixExpression(parser, expression);
}

MM_Internal MM_bool
MM_Parser__ParseBinaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser__ParsePrefixExpression(parser, expression)) return MM_false;
    
    while (MM_true)
    {
        MM_Token token = GetToken(parser);
        if (token.kind < MM_Token_FirstBinary || token.kind > MM_Token_LastBinary) break;
        
        MM_AST_KIND kind  = (MM_AST_KIND)token.kind;
        MM_umm precedence = MM_AST_BLOCK_INDEX_FROM_KIND(kind);
        
        MM_Expression** slot = expression;
        while ((MM_umm)MM_AST_BLOCK_INDEX_FROM_KIND((*slot)->kind) > precedence)
        {
            MM_ASSERT((*slot)->kind >= MM_AST_FirstBinary && (*slot)->kind <= MM_AST_LastBinary);
            slot = &(*slot)->binary_expr.right;
        }
        
        MM_Expression* left  = *slot;
        MM_Expression* right = 0;
        
        if (!MM_Parser__ParseExpression(parser, &right)) return MM_false;
        
        *slot = MM_Parser__PushNode(parser, kind);
        (*slot)->binary_expr.left  = left;
        (*slot)->binary_expr.right = right;
    }
    
    return MM_true;
}

MM_Internal MM_bool
MM_Parser__ParseConditionalExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser__ParseBinaryExpression(parser, expression)) return MM_false;
    
    if (EatToken(parser, MM_Token_QuestionMark))
    {
        MM_Expression* condition  = *expression;
        MM_Expression* true_expr  = 0;
        MM_Expression* false_expr = 0;
        
        if (!MM_Parser__ParseExpression(parser, &true_expr)) return MM_false;
        else if (!EatToken(parser, MM_Token_Colon))
        {
            if (GetToken(parser).kind == MM_Token_QuestionMark)
            {
                //// ERROR: Illegal use of conditional expression as true clause of parent conditional expression
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                //// ERROR: Missing colon after true expr in conditional expression
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
        }
        else if (!MM_Parser__ParseExpression(parser, &false_expr)) return MM_false;
        else if (GetToken(parser).kind == MM_Token_QuestionMark)
        {
            //// ERROR: Illegal use of conditional expression as false clause of parent conditional expression
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *expression = MM_Parser__PushNode(parser, MM_AST_Conditional);
        (*expression)->conditional_expr.condition  = condition;
        (*expression)->conditional_expr.true_expr  = true_expr;
        (*expression)->conditional_expr.false_expr = false_expr;
    }
    
    return MM_true;
}

MM_Internal MM_bool
MM_Parser__ParseExpression(MM_Parser* parser, MM_Expression** expression)
{
    return MM_Parser__ParseConditionalExpression(parser, expression);
}

MM_Internal MM_bool
MM_Parser__ParseAssignmentOrExpression(MM_Parser* parser, MM_AssignmentOrExpression** result)
{
    MM_Expression* expressions = 0;
    if (!MM_Parser__ParseExpressionList(parser, &expressions)) return MM_false;
    
    MM_Token token = GetToken(parser);
    if (token.kind >= MM_Token_FirstAssignment && token.kind <= MM_Token_LastAssignment)
    {
        MM_Expression* lhs = 0;
        MM_Expression* rhs = 0;
        MM_AST_KIND op     = MM_AST_Invalid;
        
        if (token.kind != MM_Token_Equals) op = MM_TOKEN_BINARY_ASSIGNMENT_TO_BINARY_KIND(token.kind);
        NextToken(parser);
        
        if (!MM_Parser__ParseExpressionList(parser, &rhs)) return MM_false;
        
        *result = MM_Parser__PushNode(parser, MM_AST_Assignment);
        (*result)->assignment_statement.lhs = lhs;
        (*result)->assignment_statement.rhs = rhs;
        (*result)->assignment_statement.op  = op;
    }
    else
    {
        if (expressions->next != 0)
        {
            //// ERROR: Illegal use of list of expressions in isolation
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *result = (MM_AssignmentOrExpression*)expressions;
    }
    
    return MM_true;
}

MM_Internal MM_bool
MM_Parser__ParseDeclAssignmentOrExpression(MM_Parser* parser, MM_DeclAssignmentOrExpression** result)
{
    MM_bool is_using = EatToken(parser, MM_Token_Using);
    
    MM_Expression* expressions = 0;
    if (!MM_Parser__ParseExpressionList(parser, &expressions)) return MM_false;
    
    MM_Token token = GetToken(parser);
    if (token.kind == MM_Token_Colon)
    {
        MM_bool is_constant      = MM_false;
        MM_bool is_uninitialized = MM_false;
        MM_Expression* names     = expressions;
        MM_Expression* types     = 0;
        MM_Expression* values    = 0;
        
        NextToken(parser);
        token = GetToken(parser);
        
        if (token.kind != MM_Token_Colon && token.kind != MM_Token_Equals)
        {
            if (!MM_Parser__ParseExpressionList(parser, &types)) return MM_false;
        }
        
        if (token.kind == MM_Token_Colon)
        {
            is_constant = MM_true;
            NextToken(parser);
            
            if (GetToken(parser).kind == MM_Token_TripleMinus)
            {
                //// ERROR: Constants cannot be uninitialized
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            if (!MM_Parser__ParseExpressionList(parser, &values)) return MM_false;
        }
        else if (token.kind == MM_Token_Equals)
        {
            NextToken(parser);
            
            if      (EatToken(parser, MM_Token_TripleMinus))           is_uninitialized = MM_true;
            else if (!MM_Parser__ParseExpressionList(parser, &values)) return MM_false;
        }
        
        if (is_constant)
        {
            *result = MM_Parser__PushNode(parser, MM_AST_Constant);
            (*result)->constant_decl.names    = names;
            (*result)->constant_decl.types    = types;
            (*result)->constant_decl.values   = values;
            (*result)->constant_decl.is_using = is_using;
        }
        else
        {
            *result = MM_Parser__PushNode(parser, MM_AST_Variable);
            (*result)->variable_decl.names            = names;
            (*result)->variable_decl.types            = types;
            (*result)->variable_decl.values           = values;
            (*result)->variable_decl.is_using         = is_using;
            (*result)->variable_decl.is_uninitialized = is_uninitialized;
        }
    }
    else if (token.kind >= MM_Token_FirstAssignment && token.kind <= MM_Token_LastAssignment)
    {
        if (is_using)
        {
            //// ERROR: Illegal use of using on assignment
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        MM_Expression* lhs = 0;
        MM_Expression* rhs = 0;
        MM_AST_KIND op     = MM_AST_Invalid;
        
        if (token.kind != MM_Token_Equals) op = MM_TOKEN_BINARY_ASSIGNMENT_TO_BINARY_KIND(token.kind);
        NextToken(parser);
        
        if (!MM_Parser__ParseExpressionList(parser, &rhs)) return MM_false;
        
        *result = MM_Parser__PushNode(parser, MM_AST_Assignment);
        (*result)->assignment_statement.lhs = lhs;
        (*result)->assignment_statement.rhs = rhs;
        (*result)->assignment_statement.op  = op;
    }
    else
    {
        if (is_using)
        {
            MM_Expression* symbols = expressions;
            MM_Expression* aliases = 0;
            
            if (EatToken(parser, MM_Token_As) && !MM_Parser__ParseExpressionList(parser, &aliases)) return MM_false;
            
            *result = MM_Parser__PushNode(parser, MM_AST_Using);
            (*result)->using_decl.symbols = symbols;
            (*result)->using_decl.aliases = aliases;
        }
        else
        {
            if (expressions->next != 0)
            {
                //// ERROR: Illegal use of list of expressions in isolation
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            *result = (MM_DeclAssignmentOrExpression*)expressions;
        }
    }
    
    return MM_true;
}

MM_Internal MM_bool
MM_Parser__ParseStatement(MM_Parser* parser, MM_Statement** statement)
{
    MM_Token token = GetToken(parser);
    
    if (token.kind == MM_Token_Semicolon)
    {
        //// ERROR: Empty statement
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    else if (token.kind == MM_Token_Colon)
    {
        NextToken(parser);
        
        token = GetToken(parser);
        if (token.kind == MM_Token_BlankIdentifier)
        {
            //// ERROR: Illegal use of blank identifier as label name
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else if (token.kind != MM_Token_Identifier)
        {
            //// ERROR: Missing name of label after colon
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        MM_String label = TokenToString(parser, token);
        
        if (GetToken(parser).kind == MM_Token_Else)
        {
            //// ERROR: Illegal use of label on else statement
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        if (!MM_Parser__ParseStatement(parser, statement)) return MM_false;
        
        switch ((*statement)->kind)
        {
            case MM_AST_Block: (*statement)->block_statement.label = label; break;
            case MM_AST_If:    (*statement)->if_statement.label    = label; break;
            case MM_AST_While: (*statement)->while_statement.label = label; break;
            MM_INVALID_DEFAULT_CASE;
        }
    }
    else if (token.kind == MM_Token_If)
    {
        NextToken(parser);
        
        MM_DeclAssignmentOrExpression* init = 0;
        MM_Expression* condition            = 0;
        MM_Statement* true_body             = 0;
        MM_Statement* false_body            = 0;
        
        if (!EatToken(parser, MM_Token_OpenParen))
        {
            //// ERROR: Missing open paren after if keyword
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        MM_DeclAssignmentOrExpression* first = 0;
        if (!MM_Parser__ParseDeclAssignmentOrExpression(parser, &first)) return MM_false;
        
        if (EatToken(parser, MM_Token_Semicolon))
        {
            init = first;
            if (!MM_Parser__ParseExpression(parser, &condition)) return MM_false;
        }
        else
        {
            if (first->kind < MM_AST_FirstExpression || first->kind > MM_AST_LastExpression)
            {
                //// ERROR: Missing condition of if statement
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            condition = (MM_Expression*)first;
        }
        
        if (!EatToken(parser, MM_Token_CloseParen))
        {
            //// ERROR: Missing closing paren after condition in if statement
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        if (!MM_Parser__ParseStatement(parser, &true_body))                                     return MM_false;
        if (EatToken(parser, MM_Token_Else) && !MM_Parser__ParseStatement(parser, &false_body)) return MM_false;
        
        *statement = MM_Parser__PushNode(parser, MM_AST_If);
        (*statement)->if_statement.init       = init;
        (*statement)->if_statement.condition  = condition;
        (*statement)->if_statement.true_body  = true_body;
        (*statement)->if_statement.false_body = false_body;
    }
    else if (token.kind == MM_Token_Else)
    {
        //// ERROR: Illegal use of else without matching if
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    else if (token.kind == MM_Token_While)
    {
        NextToken(parser);
        
        MM_DeclAssignmentOrExpression* init = 0;
        MM_Expression* condition            = 0;
        MM_AssignmentOrExpression* step     = 0;
        MM_Statement* body                  = 0;
        
        if (!EatToken(parser, MM_Token_OpenParen))
        {
            //// ERROR: Missing open paren after while keyword
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        MM_DeclAssignmentOrExpression* first = 0;
        if (!MM_Parser__ParseDeclAssignmentOrExpression(parser, &first)) return MM_false;
        
        if (EatToken(parser, MM_Token_Semicolon))
        {
            init = first;
            if (!MM_Parser__ParseExpression(parser, &condition)) return MM_false;
            
            if (EatToken(parser, MM_Token_Semicolon) && !MM_Parser__ParseAssignmentOrExpression(parser, &step)) return MM_false;
        }
        else
        {
            if (first->kind < MM_AST_FirstExpression || first->kind > MM_AST_LastExpression)
            {
                //// ERROR: Missing condition of while statement
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            condition = (MM_Expression*)first;
        }
        
        if (!EatToken(parser, MM_Token_CloseParen))
        {
            //// ERROR: Missing closing paren after condition in while statement
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        if (!MM_Parser__ParseStatement(parser, &body)) return MM_false;
        
        *statement = MM_Parser__PushNode(parser, MM_AST_While);
        (*statement)->while_statement.init      = init;
        (*statement)->while_statement.condition = condition;
        (*statement)->while_statement.step      = step;
        (*statement)->while_statement.body      = body;
    }
    else if (token.kind == MM_Token_Continue || token.kind == MM_Token_Break)
    {
        MM_String label  = {0};
        MM_AST_KIND kind = (token.kind == MM_Token_Continue ? MM_AST_Continue : MM_AST_Break);
        NextToken(parser);
        
        token = GetToken(parser);
        if (token.kind == MM_Token_BlankIdentifier)
        {
            //// ERROR: Illegal use of blank identifier as jump label
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else if (token.kind == MM_Token_Identifier)
        {
            label = TokenToString(parser, token);
            NextToken(parser);
        }
        
        if (!EatToken(parser, MM_Token_Semicolon))
        {
            //// ERROR: Missing semicolon after jump statement
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *statement = MM_Parser__PushNode(parser, kind);
        (*statement)->jump_statement.label = label;
    }
    else if (token.kind == MM_Token_Return)
    {
        NextToken(parser);
        
        MM_Argument* args = 0;
        
        if (GetToken(parser).kind != MM_Token_Semicolon && !MM_Parser__ParseArgumentList(parser, &args)) return MM_false;
        
        *statement = MM_Parser__PushNode(parser, MM_AST_Return);
        (*statement)->return_statement.args = args;
        
        if (!EatToken(parser, MM_Token_Semicolon))
        {
            //// ERROR: Missing semicolon after return statement
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
    }
    else
    {
        MM_DeclAssignmentOrExpression* result = 0;
        if (!MM_Parser__ParseDeclAssignmentOrExpression(parser, &result)) return MM_false;
        
        if (result->kind == MM_AST_Constant && result->constant_decl.values->next == 0 &&
            (result->constant_decl.values->kind == MM_AST_ProcLiteral && result->constant_decl.values->proc_lit_expr.body != 0 ||
             result->constant_decl.values->kind == MM_AST_Struct                                                               ||
             result->constant_decl.values->kind == MM_AST_Union                                                                ||
             result->constant_decl.values->kind == MM_AST_Enum))
        {
            if (GetToken(parser).kind == MM_Token_Semicolon)
            {
                //// ERROR: Procedures, structs, unions and enums do not need terminating semicolons, and
                ////        are required not to have them in order to simplify recreation
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
        }
        else if (!EatToken(parser, MM_Token_Semicolon))
        {
            //// ERROR: Missing semicolon after statement
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
    }
    
    return MM_true;
}

#undef GetToken
#undef NextToken
#undef EatToken
#undef TokenToString