typedef struct MM_Parser
{
    MM_Lexer lexer;
    MM_Arena* ast_arena;
    MM_String_Intern_Table* intern_table;
} MM_Parser;

#define MM_GetToken() MM_Lexer_CurrentToken(&parser->lexer)
#define MM_NextToken() MM_Lexer_NextToken(&parser->lexer)
#define MM_IsToken(k) (MM_GetToken().kind == (k))
#define MM_IsTokenGroup(k) (MM_GetToken().kind_group == (k))
#define MM_EatToken(k) (MM_GetToken().kind == (k) ? MM_Lexer_NextToken(&parser->lexer), MM_true : MM_false)

// TODO: Sort out error reporting
#define MM_DEBUG_PARSER_ERROR_LINE() MM_DEBUG_ParserErrorLine = __LINE__
MM_umm MM_DEBUG_ParserErrorLine = 0;

MM_bool MM_Parser_ParseExpression(MM_Parser* parser, MM_Expression** expression);
MM_bool MM_Parser_ParseBlock(MM_Parser* parser, MM_Block_Statement** statement);
MM_bool MM_Parser_ParseStatement(MM_Parser* parser, MM_Statement** statement);

void*
MM_Parser_PushNode(MM_Parser* parser, MM_u32 kind, MM_umm size)
{
    MM_AST* node = MM_Arena_Push(parser->ast_arena, size, sizeof(MM_u64));
    
    node->kind = kind;
    node->next = 0;
    
    return (void*)node;
}

// NOTE: This is just to incentivize the optimizer to replace the call to MM_AST_SizeFromKind with a constant
#define MM_PushNode(parser, k) MM_Arena_Push((parser)->ast_arena, (k), MM_AST_SizeFromKind((MM_AST_Kind){ .kind = (k) }))

MM_Identifier
MM_Parser_PushIdentifier(MM_Parser* parser, MM_Token token)
{
    return (MM_Identifier)MM_String_Intern(parser->intern_table, MM_Token_ToString(token));
}

MM_bool
MM_Parser_ParseArguments(MM_Parser* parser, MM_Argument** args)
{
    MM_Argument** next_arg = args;
    
    while (MM_true)
    {
        MM_Expression* name  = 0;
        MM_Expression* value = 0;
        
        if (!MM_Parser_ParseExpression(parser, &value)) return MM_false;
        
        if (MM_EatToken(MM_Token_Equals))
        {
            name = value;
            if (!MM_Parser_ParseExpression(parser, &value)) return MM_false;
        }
        
        *next_arg = MM_PushNode(parser, MM_AST_Argument);
        (*next_arg)->name  = name;
        (*next_arg)->value = value;
        
        next_arg = &(*next_arg)->next;
        
        if (MM_EatToken(MM_Token_Comma)) continue;
        else                             break;
    }
    
    return MM_true;
}

MM_bool
MM_Parser_ParseParameters(MM_Parser* parser, MM_Parameter** params)
{
    MM_Expression* expressions = 0;
    
    MM_Expression** next_expression = &expressions;
    while (MM_true)
    {
        if (!MM_Parser_ParseExpression(parser, next_expression)) return MM_false;
        else
        {
            if (MM_EatToken(MM_Token_Comma)) continue;
            else                             break;
        }
    }
    
    if (!MM_IsToken(MM_Token_Colon))
    {
        MM_Parameter** next_param = params;
        MM_Expression* expr       = expressions;
        while (expr != 0)
        {
            MM_Expression* next_expr = expr->next;
            expr->next = 0;
            
            *next_param = MM_PushNode(parser, MM_AST_Parameter);
            (*next_param)->names = 0;
            (*next_param)->type  = expr;
            (*next_param)->value = 0;
            
            next_param = &(*next_param)->next;
            
            expr = next_expr;
        }
    }
    else
    {
        MM_Parameter** next_param = params;
        MM_Expression* names      = expressions;
        while (MM_true)
        {
            MM_Expression* type  = 0;
            MM_Expression* value = 0;
            
            if (!MM_EatToken(MM_Token_Colon))
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing type of parameter
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                if (!MM_Parser_ParseExpression(parser, &type)) return MM_false;
                else
                {
                    if (MM_EatToken(MM_Token_Equals))
                    {
                        if (!MM_Parser_ParseExpression(parser, &value)) return MM_false;
                    }
                    
                    *next_param = MM_PushNode(parser, MM_AST_Parameter);
                    (*next_param)->names = names;
                    (*next_param)->type  = type;
                    (*next_param)->value = value;
                    
                    (*next_param)->next = 0;
                    next_param = &(*next_param)->next;
                    
                    if (!MM_EatToken(MM_Token_Comma)) break;
                    else
                    {
                        names = 0;
                        MM_Expression** next_name = &names;
                        while (MM_true)
                        {
                            if (!MM_Parser_ParseExpression(parser, next_name)) return MM_false;
                            else
                            {
                                if (MM_EatToken(MM_Token_Comma)) continue;
                                else                             break;
                            }
                        }
                        
                        continue;
                    }
                }
            }
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser_ParseStructLiteral(MM_Parser* parser, MM_Expression* type, MM_Expression** expression)
{
    MM_Argument* args = 0;
    
    if (!MM_IsToken(MM_Token_CloseBrace))
    {
        if (MM_Parser_ParseArguments(parser, &args)) return MM_false;
    }
    
    if (!MM_EatToken(MM_Token_CloseBrace))
    {
        MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing brace after arguments in struct literal expression
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    
    *expression = MM_PushNode(parser, MM_AST_StructLiteral);
    (*expression)->struct_lit_expr.type = 0;
    (*expression)->struct_lit_expr.args = args;
    
    return MM_true;
}

MM_bool
MM_Parser_ParseArrayLiteral(MM_Parser* parser, MM_Expression* elem_type, MM_Expression** expression)
{
    MM_Argument* args = 0;
    
    if (!MM_IsToken(MM_Token_CloseBracket))
    {
        if (MM_Parser_ParseArguments(parser, &args)) return MM_false;
    }
    
    if (!MM_EatToken(MM_Token_CloseBracket))
    {
        MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing bracket after arguments in array literal expression
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    
    *expression = MM_PushNode(parser, MM_AST_ArrayLiteral);
    (*expression)->array_lit_expr.elem_type = elem_type;
    (*expression)->array_lit_expr.args      = args;
    
    return MM_true;
}

MM_bool
MM_Parser_ParseString(MM_Parser* parser, MM_Token token, MM_String_Literal* string_literal)
{
    MM_Arena* arena = MM_String_BeginIntern(parser->intern_table);
    
    MM_String string = {0};
    MM_u8* buffer    = MM_Arena_Push(arena, token.size, 1);
    
    MM_Token_Parse_Error error = MM_Token_ParseString(token, buffer, &string);
    if (error.code != MM_TokenParseError_None)
    {
        MM_String_AbortIntern(parser->intern_table);
        
        MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    else
    {
        MM_Arena_Pop(arena, token.size - string.size);
        
        *string_literal = MM_String_EndIntern(parser->intern_table, string);
    }
    
    return MM_true;
}

MM_bool
MM_Parser_ParsePrimaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (MM_IsToken(MM_Token_Identifier) || MM_IsToken(MM_Token_BlankIdentifier))
    {
        *expression = MM_PushNode(parser, MM_AST_Identifier);
        (*expression)->ident_expr.ident = MM_Parser_PushIdentifier(parser, MM_GetToken());
    }
    else if (MM_IsTokenGroup(MM_TokenGroup_Integer))
    {
        MM_Token token = MM_GetToken();
        
        MM_u8 explicit_base = (token.kind == MM_Token_BinaryInt ? 2  :
                               token.kind == MM_Token_HexInt    ? 16 : 10);
        
        *expression = MM_PushNode(parser, MM_AST_Int);
        (*expression)->int_expr.value         = MM_Token_ParseInt(token);
        (*expression)->int_expr.explicit_base = explicit_base;
    }
    else if (MM_IsTokenGroup(MM_TokenGroup_Float))
    {
        MM_Token token = MM_GetToken();
        
        MM_u8 explicit_size = (token.kind == MM_Token_Float      ? 0 :
                               token.kind == MM_Token_HexFloat16 ? 2 :
                               token.kind == MM_Token_HexFloat32 ? 4 : 8);
        
        *expression = MM_PushNode(parser, MM_AST_Float);
        (*expression)->float_expr.value         = MM_Token_ParseFloat(token);
        (*expression)->float_expr.explicit_size = explicit_size;
    }
    else if (MM_IsToken(MM_Token_String))
    {
        MM_String_Literal string;
        if (!MM_Parser_ParseString(parser, MM_GetToken(), &string)) return MM_false;
        else
        {
            *expression = MM_PushNode(parser, MM_AST_String);
            (*expression)->string_expr.str = string;
        }
    }
    else if (MM_IsToken(MM_Token_Codepoint))
    {
        MM_Token token = MM_GetToken();
        
        MM_u32 codepoint;
        MM_Token_Parse_Error error = MM_Token_ParseCodepoint(token, &codepoint);
        
        if (error.code != MM_TokenParseError_None)
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            *expression = MM_PushNode(parser, MM_AST_Codepoint);
            (*expression)->codepoint_expr.value = codepoint;
        }
    }
    else if (MM_IsTokenGroup(MM_TokenGroup_Keyword))
    {
        if (MM_IsToken(MM_Token_True) || MM_IsToken(MM_Token_False))
        {
            MM_bool boolean = MM_IsToken(MM_Token_True);
            MM_NextToken();
            
            *expression = MM_PushNode(parser, MM_AST_Bool);
            (*expression)->bool_expr.value = boolean;
        }
        else if (MM_EatToken(MM_Token_This))
        {
            *expression = MM_PushNode(parser, MM_AST_This);
        }
        else if (MM_EatToken(MM_Token_Proc))
        {
            MM_Parameter* params           = 0;
            MM_Return_Value* return_values = 0;
            
            if (MM_EatToken(MM_Token_OpenParen))
            {
                if (!MM_IsToken(MM_Token_CloseParen))
                {
                    if (!MM_Parser_ParseParameters(parser, &params)) return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing parenthesis after parameters in procedure header
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
            
            if (MM_EatToken(MM_Token_Arrow))
            {
                if (!MM_EatToken(MM_Token_OpenParen))
                {
                    MM_Expression* type = 0;
                    if (!MM_Parser_ParseExpression(parser, &type)) return MM_false;
                    
                    return_values = MM_PushNode(parser, MM_AST_ReturnValue);
                    return_values->next  = 0;
                    return_values->names = 0;
                    return_values->type  = type;
                    return_values->value = 0;
                }
                else
                {
                    MM_Expression* expressions = 0;
                    
                    MM_Expression** next_expression = &expressions;
                    while (MM_true)
                    {
                        if (!MM_Parser_ParseExpression(parser, next_expression)) return MM_false;
                        else
                        {
                            if (MM_EatToken(MM_Token_Comma)) continue;
                            else                             break;
                        }
                    }
                    
                    if (!MM_IsToken(MM_Token_Colon))
                    {
                        MM_Return_Value** next_ret_val = &return_values;
                        MM_Expression* expr            = expressions;
                        while (expr != 0)
                        {
                            MM_Expression* next_expr = expr->next;
                            expr->next = 0;
                            
                            *next_ret_val = MM_PushNode(parser, MM_AST_ReturnValue);
                            (*next_ret_val)->names = 0;
                            (*next_ret_val)->type  = expr;
                            (*next_ret_val)->value = 0;
                            
                            next_ret_val = &(*next_ret_val)->next;
                            
                            expr = next_expr;
                        }
                    }
                    else
                    {
                        MM_Return_Value** next_ret_val = &return_values;
                        MM_Expression* names = expressions;
                        while (MM_true)
                        {
                            MM_Expression* type  = 0;
                            MM_Expression* value = 0;
                            
                            if (!MM_EatToken(MM_Token_Colon))
                            {
                                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing type of return values
                                MM_NOT_IMPLEMENTED;
                                return MM_false;
                            }
                            else
                            {
                                if (!MM_Parser_ParseExpression(parser, &type)) return MM_false;
                                else
                                {
                                    if (MM_EatToken(MM_Token_Equals))
                                    {
                                        if (!MM_Parser_ParseExpression(parser, &value)) return MM_false;
                                    }
                                    
                                    *next_ret_val = MM_PushNode(parser, MM_AST_ReturnValue);
                                    (*next_ret_val)->names = names;
                                    (*next_ret_val)->type  = type;
                                    (*next_ret_val)->value = value;
                                    
                                    (*next_ret_val)->next = 0;
                                    next_ret_val = &(*next_ret_val)->next;
                                    
                                    if (!MM_EatToken(MM_Token_Comma)) break;
                                    else
                                    {
                                        names = 0;
                                        MM_Expression** next_name = &names;
                                        while (MM_true)
                                        {
                                            if (!MM_Parser_ParseExpression(parser, next_name)) return MM_false;
                                            else
                                            {
                                                if (MM_EatToken(MM_Token_Comma)) continue;
                                                else                             break;
                                            }
                                        }
                                        
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            if (MM_EatToken(MM_Token_TripleMinus))
            {
                *expression = MM_PushNode(parser, MM_AST_ProcLiteralFwdDecl);
                (*expression)->proc_lit_fwd_decl_expr.params      = params;
                (*expression)->proc_lit_fwd_decl_expr.return_vals = return_values;
            }
            else if (!MM_IsToken(MM_Token_OpenBrace))
            {
                *expression = MM_PushNode(parser, MM_AST_Proc);
                (*expression)->proc_expr.params      = params;
                (*expression)->proc_expr.return_vals = return_values;
            }
            else
            {
                MM_Block_Statement* body = 0;
                
                if (!MM_Parser_ParseBlock(parser, &body)) return MM_false;
                
                *expression = MM_PushNode(parser, MM_AST_ProcLiteral);
                (*expression)->proc_lit_expr.params      = params;
                (*expression)->proc_lit_expr.return_vals = return_values;
                (*expression)->proc_lit_expr.body        = body;
            }
        }
        else if (MM_IsToken(MM_Token_Struct) || MM_IsToken(MM_Token_Union))
        {
            MM_bool is_struct = MM_IsToken(MM_Token_Struct);
            MM_NextToken();
            
            if (MM_EatToken(MM_Token_TripleMinus))
            {
                *expression = MM_PushNode(parser, MM_AST_StructFwdDecl);
            }
            else
            {
                MM_Declaration* body = 0;
                
                if (!MM_EatToken(MM_Token_OpenBrace))
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing body of struct
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                else
                {
                    if (!MM_IsToken(MM_Token_CloseBrace))
                    {
                        MM_Declaration** next_decl = &body;
                        
                        while (MM_true)
                        {
                            MM_Statement* statement = 0;
                            if (!MM_Parser_ParseStatement(parser, &statement)) return MM_false;
                            
                            if (statement->kind_type != MM_ASTType_Declaration)
                            {
                                if (statement->kind_type == MM_ASTType_Expression)
                                {
                                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Illegal use of loose expression in struct/union body
                                    MM_NOT_IMPLEMENTED;
                                    return MM_false;
                                }
                                else if (statement->kind_type == MM_ASTType_Statement)
                                {
                                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Illegal use of statement in struct/union body
                                    MM_NOT_IMPLEMENTED;
                                    return MM_false;
                                }
                                else
                                {
                                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Only declarations can be used in a struct/union body
                                    MM_NOT_IMPLEMENTED;
                                    return MM_false;
                                }
                            }
                            else
                            {
                                *next_decl = (MM_Declaration*)statement;
                                next_decl  = &(*next_decl)->next;
                            }
                        }
                    }
                    
                    if (!MM_EatToken(MM_Token_CloseBrace))
                    {
                        MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing brace after struct body
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    
                    if (is_struct)
                    {
                        *expression = MM_PushNode(parser, MM_AST_Struct);
                        (*expression)->struct_expr.body = body;
                    }
                    else
                    {
                        *expression = MM_PushNode(parser, MM_AST_Union);
                        (*expression)->union_expr.body = body;
                    }
                }
            }
        }
        else if (MM_EatToken(MM_Token_Enum))
        {
            MM_Expression* member_type = 0;
            
            if (!MM_IsToken(MM_Token_TripleMinus) && !MM_IsToken(MM_Token_OpenBrace))
            {
                if (!MM_Parser_ParseExpression(parser, &member_type)) return MM_false;
            }
            
            if (MM_EatToken(MM_Token_TripleMinus))
            {
                *expression = MM_PushNode(parser, MM_AST_EnumFwdDecl);
                (*expression)->enum_fwd_decl_expr.member_type = member_type;
            }
            else
            {
                MM_Enum_Member* members = 0;
                
                if (!MM_EatToken(MM_Token_OpenBrace))
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing body of enum
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                else
                {
                    if (!MM_IsToken(MM_Token_CloseBrace))
                    {
                        MM_Enum_Member** next_member = &members;
                        while (MM_true)
                        {
                            MM_Expression* name  = 0;
                            MM_Expression* value = 0;
                            
                            if (!MM_Parser_ParseExpression(parser, &name)) return MM_false;
                            
                            if (MM_EatToken(MM_Token_Equals))
                            {
                                if (!MM_Parser_ParseExpression(parser, &value)) return MM_false;
                            }
                            
                            *next_member = MM_PushNode(parser, MM_AST_EnumMember);
                            (*next_member)->name  = name;
                            (*next_member)->value = value;
                            
                            next_member = &(*next_member)->next;
                            
                            if (MM_EatToken(MM_Token_Comma)) continue;
                            else                             break;
                        }
                    }
                    
                    if (!MM_EatToken(MM_Token_CloseBrace))
                    {
                        MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing brace after body of enum
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    
                    *expression = MM_PushNode(parser, MM_AST_Enum);
                    (*expression)->enum_expr.member_type = member_type;
                    (*expression)->enum_expr.members     = members;
                }
            }
        }
        else
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Invalid use of keyword in expression
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
    }
    else if (MM_IsTokenGroup(MM_TokenGroup_Builtin))
    {
        MM_Token token   = MM_GetToken();
        MM_AST_Kind kind = {
            .kind_type      = MM_ASTType_Expression,
            .kind_group     = MM_ASTGroup_PrimaryExpression,
            .kind_sub_group = MM_ASTSubGroup_Builtin,
            .kind_index     = token.kind_index,
        };
        
        MM_Argument* args = 0;
        
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing arguments to builtin procedure call
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            if (!MM_IsToken(MM_Token_CloseParen))
            {
                if (!MM_Parser_ParseArguments(parser, &args)) return MM_false;
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing parenthesis after arguments to builtin procedure call
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                *expression = MM_PushNode(parser, kind.kind);
                (*expression)->builtin_expr.args = args;
            }
        }
    }
    else if (MM_EatToken(MM_Token_OpenParen))
    {
        MM_Expression* body = 0;
        if (!MM_Parser_ParseExpression(parser, &body)) return MM_false;
        
        if (!MM_EatToken(MM_Token_CloseParen))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing parenthesis after body of compound expression
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *expression = MM_PushNode(parser, MM_AST_Compound);
        (*expression)->compound_expr.body = body;
    }
    else if (MM_EatToken(MM_Token_PeriodOpenBrace))
    {
        if (!MM_Parser_ParseStructLiteral(parser, 0, expression)) return MM_false;
    }
    else if (MM_EatToken(MM_Token_PeriodOpenBracket))
    {
        if (!MM_Parser_ParseArrayLiteral(parser, 0, expression)) return MM_false;
    }
    else
    {
        if (MM_IsToken(MM_Token_Invalid))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing primary expression
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser_ParsePostfixUnaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser_ParsePrimaryExpression(parser, expression)) return MM_false;
    
    while (MM_true)
    {
        if (MM_EatToken(MM_Token_Hat))
        {
            MM_Expression* operand = *expression;
            
            *expression = MM_PushNode(parser, MM_AST_Dereference);
            (*expression)->unary_expr.operand = operand;
        }
        else if (MM_EatToken(MM_Token_Period))
        {
            if (!MM_IsToken(MM_Token_Identifier))
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing name of member after member access operator
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                MM_Expression* symbol = *expression;
                MM_Identifier member  = MM_Parser_PushIdentifier(parser, MM_GetToken());
                MM_NextToken();
                
                *expression = MM_PushNode(parser, MM_AST_MemberAccess);
                (*expression)->member_access_expr.symbol = symbol;
                (*expression)->member_access_expr.member = member;
            }
        }
        else if (MM_EatToken(MM_Token_OpenBracket))
        {
            MM_Expression* array = *expression;
            MM_Expression* first = 0;
            
            if (!MM_IsToken(MM_Token_Colon))
            {
                if (!MM_Parser_ParseExpression(parser, &first)) return MM_false;
            }
            
            if (MM_EatToken(MM_Token_Colon))
            {
                MM_Expression* start    = first;
                MM_Expression* past_end = 0;
                
                if (!MM_IsToken(MM_Token_CloseBracket))
                {
                    if (!MM_Parser_ParseExpression(parser, &past_end)) return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_CloseBracket))
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing bracket after slice interval in slice expression
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                
                *expression = MM_PushNode(parser, MM_AST_Slice);
                (*expression)->slice_expr.array    = array;
                (*expression)->slice_expr.start    = start;
                (*expression)->slice_expr.past_end = past_end;
            }
            else
            {
                if (!MM_EatToken(MM_Token_CloseBracket))
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing bracket after subscript in subscript expression
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                
                *expression = MM_PushNode(parser, MM_AST_Subscript);
                (*expression)->subscript_expr.array = array;
                (*expression)->subscript_expr.index = first;
            }
        }
        else if (MM_EatToken(MM_Token_OpenParen))
        {
            MM_Expression* proc = *expression;
            MM_Argument* args   = 0;
            
            if (!MM_IsToken(MM_Token_CloseParen))
            {
                if (MM_Parser_ParseArguments(parser, &args)) return MM_false;
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing paren after arguments in procedure call expression
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            *expression = MM_PushNode(parser, MM_AST_Call);
            (*expression)->call_expr.proc = proc;
            (*expression)->call_expr.args = args;
        }
        else if (MM_EatToken(MM_Token_PeriodOpenBrace))
        {
            if (!MM_Parser_ParseStructLiteral(parser, *expression, expression)) return MM_false;
        }
        else if (MM_EatToken(MM_Token_PeriodOpenBracket))
        {
            if (!MM_Parser_ParseArrayLiteral(parser, *expression, expression)) return MM_false;
        }
        else break;
    }
    
    return MM_true;
}

MM_bool
MM_Parser_ParsePrefixUnaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    while (MM_true)
    {
        if (MM_EatToken(MM_Token_Backslash))
        {
            *expression = MM_PushNode(parser, MM_AST_BackScope);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Hat))
        {
            *expression = MM_PushNode(parser, MM_AST_Reference);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Minus))
        {
            *expression = MM_PushNode(parser, MM_AST_Neg);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Tilde))
        {
            *expression = MM_PushNode(parser, MM_AST_BitNot);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Bang))
        {
            *expression = MM_PushNode(parser, MM_AST_Not);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_OpenBracket))
        {
            if (MM_EatToken(MM_Token_CloseBracket))
            {
                *expression = MM_PushNode(parser, MM_AST_SliceType);
                expression = &(*expression)->unary_expr.operand;
            }
            else
            {
                MM_Expression* size = 0;
                if (!MM_Parser_ParseExpression(parser, &size)) return MM_false;
                
                *expression = MM_PushNode(parser, MM_AST_ArrayType);
                (*expression)->array_type_expr.size = size;
                expression = &(*expression)->array_type_expr.elem_type;
            }
        }
        else break;
    }
    
    return MM_Parser_ParsePostfixUnaryExpression(parser, expression);
}

MM_bool
MM_Parser_ParseBinaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser_ParsePrefixUnaryExpression(parser, expression)) return MM_false;
    
    while (MM_true)
    {
        MM_Token token = MM_GetToken();
        if (!(token.kind >= MM_Token_FirstBinary && token.kind <= MM_Token_LastBinary)) break;
        else
        {
            // NOTE: The ast kind and token kind enumerations are structured in a way that makes it easy to translate between them.
            //       The actual translation is at the moment less than ideal, since it involves a division, subtraction and remainder.
            MM_AST_Kind kind = {
                .kind_type      = MM_ASTType_Expression,
                .kind_group     = MM_ASTGroup_BinaryExpression,
                .kind_sub_group = MM_TOKEN_BINARY_OPERATOR_BLOCK(token.kind),
                .kind_index     = MM_TOKEN_BINARY_OPERATOR_INDEX(token.kind),
            };
            
            MM_Expression** spot = expression;
            MM_Expression* right = 0;
            if (!MM_Parser_ParsePrefixUnaryExpression(parser, &right)) return MM_false;
            
            // NOTE: Binary expression sub groups are sorted after precedence, and an expression with a lower precedence than a
            //       binary expression can never be the direct child of a binary expression.
            while (((*spot)->kind_group == MM_ASTGroup_BinaryExpression) & ((*spot)->kind_sub_group > kind.kind_sub_group))
            {
                spot = &(*spot)->binary_expr.right;
            }
            
            MM_Expression* left = *spot;
            
            *spot = MM_PushNode(parser, kind.kind);
            (*spot)->binary_expr.left  = left;
            (*spot)->binary_expr.right = right;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser_ParseConditionalExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser_ParseBinaryExpression(parser, expression)) return MM_false;
    
    if (MM_EatToken(MM_Token_QuestionMark))
    {
        MM_Expression* condition  = *expression;
        MM_Expression* true_expr  = 0;
        MM_Expression* false_expr = 0;
        
        if (!MM_Parser_ParseBinaryExpression(parser, &true_expr)) return MM_false;
        
        if (!MM_EatToken(MM_Token_Colon))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing false clause of conditional expression
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        if (!MM_Parser_ParseBinaryExpression(parser, &false_expr)) return MM_false;
        
        *expression = MM_PushNode(parser, MM_AST_Conditional);
        (*expression)->conditional_expr.condition  = condition;
        (*expression)->conditional_expr.true_expr  = true_expr;
        (*expression)->conditional_expr.false_expr = false_expr;
    }
    
    return MM_true;
}

MM_bool
MM_Parser_ParseExpression(MM_Parser* parser, MM_Expression** expression)
{
    return MM_Parser_ParseConditionalExpression(parser, expression);
}

MM_bool
MM_Parser_ParseBlock(MM_Parser* parser, MM_Block_Statement** block)
{
    MM_ASSERT(MM_IsToken(MM_Token_OpenBrace));
    MM_NextToken();
    
    MM_Statement* body = 0;
    MM_Statement** next_statement = &body;
    while (!MM_IsToken(MM_Token_CloseBrace))
    {
        if (MM_IsToken(MM_Token_EndOfStream))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Hit end of stream before closing brace
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else if (!MM_Parser_ParseStatement(parser, next_statement)) return MM_false;
        else
        {
            next_statement = &(*next_statement)->next;
            continue;
        }
    }
    
    MM_ASSERT(MM_IsToken(MM_Token_CloseBrace));
    MM_NextToken();
    
    *block = MM_PushNode(parser, MM_AST_Block);
    (*block)->body = body;
    
    return MM_true;
}

MM_bool
MM_Parser_ParseDeclAssignmentOrExpression(MM_Parser* parser, MM_Statement** statement)
{
    MM_bool is_using    = MM_false;
    MM_bool is_distinct = MM_false;
    
    while (MM_true)
    {
        if (MM_IsToken(MM_Token_Using))
        {
            if (is_using)
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Duplicate using
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                is_using = MM_true;
                MM_NextToken();
            }
        }
        else if (MM_IsToken(MM_Token_Distinct))
        {
            if (is_distinct)
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Duplicate distinct
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                is_distinct = MM_true;
                MM_NextToken();
            }
        }
        else break;
    }
    
    MM_Expression* expressions = 0;
    MM_Expression** next_expr = &expressions;
    while (MM_true)
    {
        if (!MM_Parser_ParseExpression(parser, next_expr)) return MM_false;
        else
        {
            next_expr = &(*next_expr)->next;
            
            if (MM_EatToken(MM_Token_Comma)) continue;
            else                             break;
        }
    }
    
    if (MM_EatToken(MM_Token_Colon))
    {
        MM_Expression* names = expressions;
        MM_Expression* type  = 0;
        
        if (!MM_IsToken(MM_Token_Colon) && !MM_IsToken(MM_Token_Equals))
        {
            if (!MM_Parser_ParseExpression(parser, &type)) return MM_false;
        }
        
        if (MM_EatToken(MM_Token_Colon))
        {
            if (MM_EatToken(MM_Token_TripleMinus))
            {
                if (type == 0)
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing type of forward declared constant
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                else
                {
                    *statement = MM_PushNode(parser, MM_AST_ConstantFwdDecl);
                    ((MM_Constant_Fwd_Declaration*)*statement)->names  = names;
                    ((MM_Constant_Fwd_Declaration*)*statement)->type   = type;
                }
            }
            else
            {
                MM_Expression* values = 0;
                
                MM_Expression** next_value = &values;
                while (MM_true)
                {
                    if (!MM_Parser_ParseExpression(parser, next_value)) return MM_false;
                    else
                    {
                        next_value = &(*next_value)->next;
                        
                        if (MM_EatToken(MM_Token_Comma)) continue;
                        else                             break;
                    }
                }
                
                *statement = MM_PushNode(parser, MM_AST_Constant);
                ((MM_Constant_Declaration*)*statement)->names       = names;
                ((MM_Constant_Declaration*)*statement)->type        = type;
                ((MM_Constant_Declaration*)*statement)->values      = values;
                ((MM_Constant_Declaration*)*statement)->is_using    = is_using;
                ((MM_Constant_Declaration*)*statement)->is_distinct = is_distinct;
            }
        }
        else
        {
            MM_Expression* values    = 0;
            MM_bool is_uninitialized = MM_false;
            if (MM_EatToken(MM_Token_Equals))
            {
                if (MM_EatToken(MM_Token_TripleMinus)) is_uninitialized = MM_true;
                else
                {
                    MM_Expression** next_value = &values;
                    while (MM_true)
                    {
                        if (!MM_Parser_ParseExpression(parser, next_value)) return MM_false;
                        else
                        {
                            next_value = &(*next_value)->next;
                            
                            if (MM_EatToken(MM_Token_Comma)) continue;
                            else                             break;
                        }
                    }
                }
            }
            
            *statement = MM_PushNode(parser, MM_AST_Variable);
            ((MM_Variable_Declaration*)*statement)->names            = names;
            ((MM_Variable_Declaration*)*statement)->type             = type;
            ((MM_Variable_Declaration*)*statement)->values           = values;
            ((MM_Variable_Declaration*)*statement)->is_using         = is_using;
            ((MM_Variable_Declaration*)*statement)->is_uninitialized = is_uninitialized;
        }
    }
    else
    {
        MM_Token token = MM_GetToken();
        
        if (token.kind >= MM_Token_FirstAssignment && token.kind <= MM_Token_LastAssignment)
        {
            if (is_using)
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Invalid use of using on assignment statement
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                MM_AST_Kind kind = {
                    .kind_type      = MM_ASTType_Statement,
                    .kind_group     = MM_ASTGroup_Assignment,
                    .kind_sub_group = MM_AST_None,
                    .kind_index     = token.kind_index,
                };
                
                MM_NextToken();
                
                MM_Expression* lhs = expressions;
                MM_Expression* rhs = 0;
                
                MM_Expression** next_value = &rhs;
                while (MM_true)
                {
                    if (!MM_Parser_ParseExpression(parser, next_value)) return MM_false;
                    else
                    {
                        next_value = &(*next_value)->next;
                        
                        if (MM_EatToken(MM_Token_Comma)) continue;
                        else                             break;
                    }
                }
                
                *statement = MM_PushNode(parser, kind.kind);
                (*statement)->assignment_statement.lhs = lhs;
                (*statement)->assignment_statement.rhs = rhs;
            }
        }
        else if (is_using)
        {
            if (expressions->next != 0)
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Cannot alias multiple symbols in one using statement
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                MM_Expression* symbol = expressions;
                MM_Identifier alias   = {};
                
                if (MM_EatToken(MM_Token_As))
                {
                    if (!MM_IsToken(MM_Token_Identifier))
                    {
                        MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing name of alias
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else
                    {
                        alias = MM_Parser_PushIdentifier(parser, MM_GetToken());
                        MM_NextToken();
                    }
                }
                
                *statement = MM_PushNode(parser, MM_AST_Using);
                ((MM_Using_Declaration*)*statement)->symbol = symbol;
                ((MM_Using_Declaration*)*statement)->alias  = alias;
            }
        }
        else
        {
            if (expressions->next != 0)
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Illegal use of expression list by itself
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else *statement = (MM_Statement*)expressions;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser_ParseStatement(MM_Parser* parser, MM_Statement** statement)
{
    if (MM_IsToken(MM_Token_Semicolon))
    {
        MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Empty statement
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    else if (MM_IsToken(MM_Token_Else))
    {
        MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Else without matching if
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    else if (MM_EatToken(MM_Token_Colon))
    {
        if (!MM_IsToken(MM_Token_Identifier))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing name of label
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Identifier label = MM_Parser_PushIdentifier(parser, MM_GetToken());
            MM_NextToken();
            
            if (!MM_Parser_ParseStatement(parser, statement)) return MM_false;
            else
            {
                if      ((*statement)->kind == MM_AST_If)    (*statement)->if_statement.label    = label;
                else if ((*statement)->kind == MM_AST_While) (*statement)->while_statement.label = label;
                else if ((*statement)->kind == MM_AST_Block) (*statement)->block_statement.label = label;
                else
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Statement label cannot be applied on x
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
        }
    }
    else if (MM_IsToken(MM_Token_OpenBrace))
    {
        if (!MM_Parser_ParseBlock(parser, (MM_Block_Statement**)&statement)) return MM_false;
    }
    else if (MM_EatToken(MM_Token_If))
    {
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing open paren after if keyword in if statement
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Statement* first = 0;
            
            if (!MM_IsToken(MM_Token_Semicolon))
            {
                if (!MM_Parser_ParseDeclAssignmentOrExpression(parser, &first)) return MM_false;
                else if (first->kind_type == MM_ASTType_Statement)
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Invalid use of labeled statement as init in if statement
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
            
            MM_Statement* init       = 0;
            MM_Expression* condition = 0;
            
            if (!MM_EatToken(MM_Token_Semicolon))
            {
                if (first->kind_type == MM_ASTType_Expression) condition = (MM_Expression*)first;
                else
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Invalid use of statement/declaration as if statement condition
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
            else
            {
                init = first;
                if (!MM_Parser_ParseExpression(parser, &condition)) return MM_false;
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing parenthesis after if statement condition
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                MM_Statement* true_body  = 0;
                MM_Statement* false_body = 0;
                
                if (!MM_Parser_ParseStatement(parser, &true_body)) return MM_false;
                else
                {
                    if (MM_EatToken(MM_Token_Else))
                    {
                        if (!MM_Parser_ParseStatement(parser, &false_body)) return MM_false;
                    }
                    
                    *statement = MM_PushNode(parser, MM_AST_If);
                    (*statement)->if_statement.init       = init;
                    (*statement)->if_statement.condition  = condition;
                    (*statement)->if_statement.true_body  = true_body;
                    (*statement)->if_statement.false_body = false_body;
                }
            }
        }
    }
    else if (MM_EatToken(MM_Token_When))
    {
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing open paren after when keyword in when declaration
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Expression* condition = 0;
            
            if (!MM_Parser_ParseExpression(parser, &condition)) return MM_false;
            else
            {
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing parenthesis after when declaration condition
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                else
                {
                    MM_Statement* true_body  = 0;
                    MM_Statement* false_body = 0;
                    
                    if (!MM_Parser_ParseStatement(parser, &true_body)) return MM_false;
                    else
                    {
                        if (MM_EatToken(MM_Token_Else))
                        {
                            if (!MM_Parser_ParseStatement(parser, &false_body)) return MM_false;
                        }
                        
                        *statement = MM_PushNode(parser, MM_AST_When);
                        ((MM_When_Declaration*)*statement)->condition  = condition;
                        ((MM_When_Declaration*)*statement)->true_body  = true_body;
                        ((MM_When_Declaration*)*statement)->false_body = false_body;
                    }
                }
            }
        }
    }
    else if (MM_EatToken(MM_Token_While))
    {
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing open paren after while keyword in while statement
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Statement* first = 0;
            
            if (!MM_IsToken(MM_Token_Semicolon))
            {
                if (!MM_Parser_ParseDeclAssignmentOrExpression(parser, &first)) return MM_false;
                else if (first->kind_type == MM_ASTType_Statement)
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Invalid use of labeled statement as init in while statement
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
            
            MM_Statement* init       = 0;
            MM_Expression* condition = 0;
            MM_Statement* step       = 0;
            
            if (!MM_EatToken(MM_Token_Semicolon))
            {
                if (first->kind_type == MM_ASTType_Expression) condition = (MM_Expression*)first;
                else
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Invalid use of statement/declaration as while statement condition
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
            else
            {
                init = first;
                
                if (!MM_IsToken(MM_Token_Semicolon))
                {
                    first = 0;
                    if (!MM_Parser_ParseDeclAssignmentOrExpression(parser, &first)) return MM_false;
                    else
                    {
                        if (first->kind_type == MM_ASTType_Expression) condition = (MM_Expression*)first;
                        else
                        {
                            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Invalid use of statement/declaration as while statement condition
                            MM_NOT_IMPLEMENTED;
                            return MM_false;
                        }
                    }
                }
                
                if (MM_EatToken(MM_Token_Semicolon))
                {
                    if (!MM_Parser_ParseDeclAssignmentOrExpression(parser, &step)) return MM_false;
                    else if (first->kind_type == MM_ASTType_Statement)
                    {
                        MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Invalid use of labeled statement as step in while statement
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                }
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing closing parenthesis after while statement condition
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                MM_Statement* body = 0;
                if (!MM_Parser_ParseStatement(parser, &body)) return MM_false;
                else
                {
                    *statement = MM_PushNode(parser, MM_AST_While);
                    (*statement)->while_statement.init      = init;
                    (*statement)->while_statement.condition = condition;
                    (*statement)->while_statement.step      = step;
                    (*statement)->while_statement.body      = body;
                }
            }
        }
    }
    else if (MM_IsToken(MM_Token_Continue) || MM_IsToken(MM_Token_Break))
    {
        MM_u32 kind = (MM_IsToken(MM_Token_Continue) ? MM_AST_Continue : MM_AST_Break);
        
        // TODO: none vs. blank vs. label
        MM_Identifier label = {};
        
        if (!MM_IsToken(MM_Token_Semicolon))
        {
            if (MM_IsToken(MM_Token_Identifier))
            {
                label = MM_Parser_PushIdentifier(parser, MM_GetToken());
                MM_NextToken();
            }
            else
            {
                MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Expected name of jump label
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
        }
        
        if (!MM_EatToken(MM_Token_Semicolon))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing semicolon after jump statement
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            *statement = MM_PushNode(parser, kind);
            (*statement)->jump_statement.label = label;
        }
    }
    else if (MM_EatToken(MM_Token_Defer))
    {
        MM_Statement* body = 0;
        
        if (!MM_Parser_ParseStatement(parser, &body)) return MM_false;
        
        *statement = MM_PushNode(parser, MM_AST_Defer);
        (*statement)->defer_statement.body = body;
    }
    else if (MM_EatToken(MM_Token_Return))
    {
        MM_Argument* args = 0;
        
        if (!MM_IsToken(MM_Token_Semicolon))
        {
            if (!MM_Parser_ParseArguments(parser, &args)) return MM_false;
        }
        
        if (MM_EatToken(MM_Token_Semicolon))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing semicolon after return statement
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            *statement = MM_PushNode(parser, MM_AST_Return);
            (*statement)->return_statement.args = args;
        }
    }
    else if (MM_EatToken(MM_Token_Include))
    {
        if (!MM_IsToken(MM_Token_String))
        {
            MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing path string after include keyword in include declaration
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_String_Literal path;
            if (!MM_Parser_ParseString(parser, MM_GetToken(), &path)) return MM_false;
            else
            {
                if (MM_EatToken(MM_Token_Semicolon))
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing semicolon after include statement
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                else
                {
                    *statement = MM_PushNode(parser, MM_AST_Include);
                    ((MM_Include_Declaration*)*statement)->path = path;
                }
            }
        }
    }
    else
    {
        if (!MM_Parser_ParseDeclAssignmentOrExpression(parser, statement)) return MM_false;
        else
        {
            MM_bool should_have_semi = MM_true;
            
            if ((*statement)->kind == MM_AST_Constant && ((MM_Constant_Declaration*)*statement)->values->next == 0)
            {
                MM_u32 kind = ((MM_Constant_Declaration*)*statement)->values->kind;
                should_have_semi = (kind == MM_AST_ProcLiteral || kind == MM_AST_Struct || kind == MM_AST_Union || kind == MM_AST_Enum);
            }
            
            if (should_have_semi)
            {
                if (!MM_EatToken(MM_Token_Semicolon))
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Missing semicolon after x
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
            else
            {
                if (MM_IsToken(MM_Token_Semicolon))
                {
                    MM_DEBUG_PARSER_ERROR_LINE(); //// ERROR: Stray semicolon after x
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
        }
    }
    
    return MM_true;
}

#undef MM_GetToken
#undef MM_NextToken
#undef MM_IsToken
#undef MM_IsTokenGroup
#undef MM_EatToken
#undef MM_PushNode