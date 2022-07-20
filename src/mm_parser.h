typedef struct MM_Parser
{
    MM_Lexer lexer;
} MM_Parser;

inline void*
MM_Parser_PushNode(MM_Parser* parser, MM_u32 kind)
{
    void* result = 0;
    MM_NOT_IMPLEMENTED;
    return result;
}

#define MM_GetToken() MM_Lexer_CurrentToken(&parser->lexer)
#define MM_NextToken() MM_Lexer_NextToken(&parser->lexer)
#define MM_IsToken(k) (MM_GetToken().kind == (k))
#define MM_IsTokenGroup(k) (MM_GetToken().kind_group == (k))
#define MM_EatToken(k) (MM_GetToken().kind == (k) ? MM_Lexer_NextToken(&parser->lexer), MM_true : MM_false)

MM_bool MM_Parser_ParseExpression(MM_Parser* parser, MM_Expression** expression);
MM_bool MM_Parser_ParseBlock(MM_Parser* parser, MM_Block_Statement** statement);
MM_bool MM_Parser_ParseStatement(MM_Parser* parser, MM_Statement** statement);

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
        
        *next_arg = MM_Parser_PushNode(parser, MM_AST_Argument);
        (*next_arg)->name  = name;
        (*next_arg)->value = value;
        
        (*next_arg)->next = 0;
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
            
            *next_param = MM_Parser_PushNode(parser, MM_AST_Parameter);
            (*next_param)->names = 0;
            (*next_param)->type  = expr;
            (*next_param)->value = 0;
            
            (*next_param)->next = 0;
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
                //// ERROR: Missing type of parameter
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
                    
                    *next_param = MM_Parser_PushNode(parser, MM_AST_Parameter);
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
        //// ERROR: Missing closing brace after arguments in struct literal expression
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    
    *expression = MM_Parser_PushNode(parser, MM_AST_StructLiteral);
    (*expression)->unary_expr.struct_lit_expr.type = 0;
    (*expression)->unary_expr.struct_lit_expr.args = args;
    
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
        //// ERROR: Missing closing bracket after arguments in array literal expression
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    
    *expression = MM_Parser_PushNode(parser, MM_AST_ArrayLiteral);
    (*expression)->unary_expr.array_lit_expr.elem_type = elem_type;
    (*expression)->unary_expr.array_lit_expr.args      = args;
    
    return MM_true;
}

MM_bool
MM_Parser_ParsePrimaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (MM_IsToken(MM_Token_Identifier) || MM_IsToken(MM_Token_BlankIdentifier))
    {
        // MM_AST_Identifier,
        MM_NOT_IMPLEMENTED;
    }
    else if (MM_IsTokenGroup(MM_TokenGroup_Integer))
    {
        MM_Token token = MM_GetToken();
        
        MM_u8 explicit_base = (token.kind == MM_Token_BinaryInt ? 2  :
                               token.kind == MM_Token_HexInt    ? 16 : 10);
        
        *expression = MM_Parser_PushNode(parser, MM_AST_Int);
        (*expression)->int_expr.value         = MM_Token_ParseInt(token);
        (*expression)->int_expr.explicit_base = explicit_base;
    }
    else if (MM_IsTokenGroup(MM_TokenGroup_Float))
    {
        MM_Token token = MM_GetToken();
        
        MM_u8 explicit_size = (token.kind == MM_Token_Float      ? 0 :
                               token.kind == MM_Token_HexFloat16 ? 2 :
                               token.kind == MM_Token_HexFloat32 ? 4 : 8);
        
        *expression = MM_Parser_PushNode(parser, MM_AST_Float);
        (*expression)->float_expr.value         = MM_Token_ParseFloat(token);
        (*expression)->float_expr.explicit_size = explicit_size;
    }
    else if (MM_IsToken(MM_Token_String))
    {
        *expression = MM_Parser_PushNode(parser, MM_AST_String);
        //(*expression)->string = ;
        MM_NOT_IMPLEMENTED;
    }
    else if (MM_IsToken(MM_Token_Codepoint))
    {
        MM_Token token = MM_GetToken();
        
        MM_u32 codepoint;
        MM_Token_Parse_Error error = MM_Token_ParseCodepoint(token, &codepoint);
        
        if (error.code != MM_TokenParseError_None)
        {
            //// ERROR
            MM_NOT_IMPLEMENTED;
        }
        else
        {
            *expression = MM_Parser_PushNode(parser, MM_AST_Codepoint);
            (*expression)->codepoint = codepoint;
        }
    }
    else if (MM_IsTokenGroup(MM_TokenGroup_Keyword))
    {
        if (MM_IsToken(MM_Token_True) || MM_IsToken(MM_Token_False))
        {
            MM_bool boolean = MM_IsToken(MM_Token_True);
            MM_NextToken();
            
            *expression = MM_Parser_PushNode(parser, MM_AST_Bool);
            (*expression)->boolean = boolean;
        }
        else if (MM_EatToken(MM_Token_This))
        {
            *expression = MM_Parser_PushNode(parser, MM_AST_This);
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
                    //// ERROR: Missing closing parenthesis after parameters in procedure header
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
                    
                    return_values = MM_Parser_PushNode(parser, MM_AST_ReturnValue);
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
                            
                            *next_ret_val = MM_Parser_PushNode(parser, MM_AST_ReturnValue);
                            (*next_ret_val)->names = 0;
                            (*next_ret_val)->type  = expr;
                            (*next_ret_val)->value = 0;
                            
                            (*next_ret_val)->next = 0;
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
                                //// ERROR: Missing type of return values
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
                                    
                                    *next_ret_val = MM_Parser_PushNode(parser, MM_AST_ReturnValue);
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
                *expression = MM_Parser_PushNode(parser, MM_AST_ProcLiteralFwdDecl);
                (*expression)->proc_lit_fwd_decl_expr.params      = params;
                (*expression)->proc_lit_fwd_decl_expr.return_vals = return_values;
            }
            else if (!MM_IsToken(MM_Token_OpenBrace))
            {
                *expression = MM_Parser_PushNode(parser, MM_AST_Proc);
                (*expression)->proc_expr.params      = params;
                (*expression)->proc_expr.return_vals = return_values;
            }
            else
            {
                MM_Block_Statement* body = 0;
                
                if (!MM_Parser_ParseBlock(parser, &body)) return MM_false;
                
                *expression = MM_Parser_PushNode(parser, MM_AST_ProcLiteral);
                (*expression)->proc_lit_expr.params      = params;
                (*expression)->proc_lit_expr.return_vals = return_values;
                (*expression)->proc_lit_expr.body        = body;
            }
        }
        else if (MM_EatToken(MM_Token_ProcSet))
        {
            if (MM_EatToken(MM_Token_TripleMinus))
            {
                *expression = MM_Parser_PushNode(parser, MM_AST_ProcSetFwdDecl);
            }
            else
            {
                MM_Expression* members = 0;
                
                if (!MM_EatToken(MM_Token_OpenBrace))
                {
                    //// ERROR: Missing members of proc set
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                else
                {
                    if (!MM_IsToken(MM_Token_CloseBrace))
                    {
                        MM_Expression** next_member = &members;
                        while (MM_true)
                        {
                            if (!MM_Parser_ParseExpression(parser, next_member)) return MM_false;
                            
                            next_member = &(*next_member)->next;
                            
                            if (MM_EatToken(MM_Token_Comma)) continue;
                            else                             break;
                        }
                    }
                    
                    if (!MM_EatToken(MM_Token_CloseBrace))
                    {
                        //// ERROR: Missing close brace after members in proc set
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else
                    {
                        *expression = MM_Parser_PushNode(parser, MM_AST_ProcSet);
                        (*expression)->proc_set_expr.members = members;
                    }
                }
            }
        }
        else if (MM_IsToken(MM_Token_Struct) || MM_IsToken(MM_Token_Union))
        {
            MM_bool is_struct = MM_IsToken(MM_Token_Struct);
            MM_NextToken();
            
            if (MM_EatToken(MM_Token_TripleMinus))
            {
                *expression = MM_Parser_PushNode(parser, MM_AST_StructFwdDecl);
            }
            else
            {
                MM_Declaration* body = 0;
                
                if (!MM_EatToken(MM_Token_OpenBrace))
                {
                    //// ERROR: Missing body of struct
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
                                    //// ERROR: Illegal use of loose expression in struct/union body
                                    MM_NOT_IMPLEMENTED;
                                    return MM_false;
                                }
                                else if (statement->kind_type == MM_ASTType_Statement)
                                {
                                    //// ERROR: Illegal use of statement in struct/union body
                                    MM_NOT_IMPLEMENTED;
                                    return MM_false;
                                }
                                else
                                {
                                    //// ERROR: Only declarations can be used in a struct/union body
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
                        //// ERROR: Missing closing brace after struct body
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    
                    if (is_struct)
                    {
                        *expression = MM_Parser_PushNode(parser, MM_AST_Struct);
                        (*expression)->struct_expr.body = body;
                    }
                    else
                    {
                        *expression = MM_Parser_PushNode(parser, MM_AST_Union);
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
                *expression = MM_Parser_PushNode(parser, MM_AST_EnumFwdDecl);
                (*expression)->enum_fwd_decl_expr.member_type = member_type;
            }
            else
            {
                MM_Enum_Member* members = 0;
                
                if (!MM_EatToken(MM_Token_OpenBrace))
                {
                    //// ERROR: Missing body of enum
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
                            
                            *next_member = MM_Parser_PushNode(parser, MM_AST_EnumMember);
                            (*next_member)->name  = name;
                            (*next_member)->value = value;
                            
                            next_member = &(*next_member)->next;
                            
                            if (MM_EatToken(MM_Token_Comma)) continue;
                            else                             break;
                        }
                    }
                    
                    if (!MM_EatToken(MM_Token_CloseBrace))
                    {
                        //// ERROR: Missing body of enum
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    
                    *expression = MM_Parser_PushNode(parser, MM_AST_Enum);
                    (*expression)->enum_expr.member_type = member_type;
                    (*expression)->enum_expr.members     = members;
                }
            }
        }
        else
        {
            //// ERROR: Invalid use of keyword in expression
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
            //// ERROR: Missing arguments to builtin procedure call
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
                //// ERROR: Missing closing parenthesis after arguments to builtin procedure call
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                *expression = MM_Parser_PushNode(parser, kind.kind);
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
            //// ERROR: Missing closing parenthesis after body of compound expression
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *expression = MM_Parser_PushNode(parser, MM_AST_Compound);
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
            //// ERROR
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

MM_bool
MM_Parser_ParsePostfixUnaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser_ParsePrimaryExpression(parser, expression)) return MM_false;
    
    while (MM_true)
    {
        if (MM_EatToken(MM_Token_Hat))
        {
            MM_Expression* operand = *expression;
            
            *expression = MM_Parser_PushNode(parser, MM_AST_Dereference);
            (*expression)->unary_expr.operand = operand;
        }
        else if (MM_EatToken(MM_Token_Period))
        {
            if (!MM_IsToken(MM_Token_Identifier))
            {
                //// ERROR: Missing name of member after member access operator
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            MM_Expression* symbol = *expression;
            MM_Identifier member = {}; MM_NOT_IMPLEMENTED;
            
            *expression = MM_Parser_PushNode(parser, MM_AST_MemberAccess);
            (*expression)->unary_expr.member_access_expr.symbol = symbol;
            (*expression)->unary_expr.member_access_expr.member = member;
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
                    //// ERROR: Missing closing bracket after slice interval in slice expression
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                
                *expression = MM_Parser_PushNode(parser, MM_AST_Slice);
                (*expression)->unary_expr.slice_expr.array    = array;
                (*expression)->unary_expr.slice_expr.start    = start;
                (*expression)->unary_expr.slice_expr.past_end = past_end;
            }
            else
            {
                if (!MM_EatToken(MM_Token_CloseBracket))
                {
                    //// ERROR: Missing closing bracket after subscript in subscript expression
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                
                *expression = MM_Parser_PushNode(parser, MM_AST_Subscript);
                (*expression)->unary_expr.subscript_expr.array = array;
                (*expression)->unary_expr.subscript_expr.index = first;
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
                //// ERROR: Missing closing paren after arguments in procedure call expression
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            
            *expression = MM_Parser_PushNode(parser, MM_AST_Call);
            (*expression)->unary_expr.call_expr.proc = proc;
            (*expression)->unary_expr.call_expr.args = args;
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
        if (MM_EatToken(MM_Token_Hat))
        {
            *expression = MM_Parser_PushNode(parser, MM_AST_Reference);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Minus))
        {
            *expression = MM_Parser_PushNode(parser, MM_AST_Neg);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Tilde))
        {
            *expression = MM_Parser_PushNode(parser, MM_AST_BitNot);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Bang))
        {
            *expression = MM_Parser_PushNode(parser, MM_AST_Not);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_OpenBracket))
        {
            if (MM_EatToken(MM_Token_CloseBracket))
            {
                *expression = MM_Parser_PushNode(parser, MM_AST_SliceType);
                expression = &(*expression)->unary_expr.operand;
            }
            else
            {
                MM_Expression* size = 0;
                if (!MM_Parser_ParseExpression(parser, &size)) return MM_false;
                
                *expression = MM_Parser_PushNode(parser, MM_AST_ArrayType);
                (*expression)->unary_expr.array_type_expr.size = size;
                expression = &(*expression)->unary_expr.array_type_expr.elem_type;
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
            
            *spot = MM_Parser_PushNode(parser, kind.kind);
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
            //// ERROR: Missing false clause of conditional expression
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        if (!MM_Parser_ParseBinaryExpression(parser, &false_expr)) return MM_false;
        
        *expression = MM_Parser_PushNode(parser, MM_AST_Conditional);
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
    MM_NOT_IMPLEMENTED;
    return MM_true;
}

MM_bool
MM_Parser_ParseStatement(MM_Parser* parser, MM_Statement** statement)
{
    MM_NOT_IMPLEMENTED;
    return MM_true;
}

#undef MM_GetToken
#undef MM_NextToken
#undef MM_IsToken
#undef MM_IsTokenGroup
#undef MM_EatToken