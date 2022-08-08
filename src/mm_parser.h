typedef struct MM_Parser
{
    MM_Lexer lexer;
    MM_Arena* ast_arena;
    MM_String_Intern_Table* ident_table;
    MM_String_Intern_Table* string_table;
    MM_Error error;
} MM_Parser;

#define MM_GetToken() MM_Lexer_CurrentToken(&parser->lexer)
#define MM_NextToken() MM_Lexer_NextToken(&parser->lexer)
#define MM_IsToken(k) (MM_GetToken().kind == (k))
#define MM_IsTokenGroup(k) (MM_GetToken().kind_group == (k))
#define MM_EatToken(k) (MM_GetToken().kind == (k) ? MM_Lexer_NextToken(&parser->lexer), MM_true : MM_false)
#define MM_GetPos() (MM_GetToken().pos)

MM_bool MM_Parser__ParseExpression(MM_Parser* parser, MM_Expression** expression);
MM_bool MM_Parser__ParseBlock(MM_Parser* parser, MM_Block_Statement** statement);
MM_bool MM_Parser__ParseStatement(MM_Parser* parser, MM_Statement** statement);

void*
MM_Parser__PushNode(MM_Parser* parser, MM_u32 kind, MM_umm size, MM_Text_Pos pos)
{
    MM_AST* node = MM_Arena_Push(parser->ast_arena, size, sizeof(MM_u64));
    
    node->pos = pos;
    node->kind = kind;
    node->next = 0;
    
    return (void*)node;
}

// NOTE: This is just to incentivize the optimizer to replace the call to MM_AST_SizeFromKind with a constant
#define MM_PushNode(k, p) MM_Parser__PushNode(parser, (k), MM_AST_SizeFromKind((MM_AST_Kind){ .kind = (k) }), (p))

MM_Identifier
MM_Parser__PushIdentifier(MM_Parser* parser, MM_Token token)
{
    return (MM_Identifier)MM_String_Intern(parser->ident_table, MM_Lexer_TokenString(&parser->lexer, token));
}

void
MM_Parser__Error(MM_Parser* parser, MM_u32 code)
{
    MM_ASSERT(parser->error.code == MM_Error_None);
    
    parser->error = (MM_Error){
        .code = code,
    };
}

#define MM_ERROR(c) do { MM_Parser__Error(parser, (c)); return MM_false; } while (0)

MM_bool
MM_Parser__ParseArguments(MM_Parser* parser, MM_Argument** args)
{
    MM_Argument** next_arg = args;
    
    while (MM_true)
    {
        MM_Text_Pos pos = MM_GetPos();
        MM_Expression* name  = 0;
        MM_Expression* value = 0;
        
        if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
        
        if (MM_EatToken(MM_Token_Equals))
        {
            name = value;
            if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
        }
        
        *next_arg = MM_PushNode(MM_AST_Argument, pos);
        (*next_arg)->name  = name;
        (*next_arg)->value = value;
        
        next_arg = &(*next_arg)->next;
        
        if (MM_EatToken(MM_Token_Comma)) continue;
        else                             break;
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseParameters(MM_Parser* parser, MM_Parameter** params)
{
    MM_Expression* expressions = 0;
    
    MM_Text_Pos pos = MM_GetPos();
    
    MM_Expression** next_expression = &expressions;
    while (MM_true)
    {
        if (!MM_Parser__ParseExpression(parser, next_expression)) return MM_false;
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
            
            *next_param = MM_PushNode(MM_AST_Parameter, pos);
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
                //// ERROR
                MM_ERROR(MM_ParserError_MissingColonBetweenParamNamesAndType);
            }
            else
            {
                if (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
                else
                {
                    if (MM_EatToken(MM_Token_Equals))
                    {
                        if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
                    }
                    
                    *next_param = MM_PushNode(MM_AST_Parameter, pos);
                    (*next_param)->names = names;
                    (*next_param)->type  = type;
                    (*next_param)->value = value;
                    
                    (*next_param)->next = 0;
                    next_param = &(*next_param)->next;
                    
                    if (!MM_EatToken(MM_Token_Comma)) break;
                    else
                    {
                        pos = MM_GetPos();
                        
                        names = 0;
                        MM_Expression** next_name = &names;
                        while (MM_true)
                        {
                            if (!MM_Parser__ParseExpression(parser, next_name)) return MM_false;
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
MM_Parser__ParseStructLiteralArgs(MM_Parser* parser, MM_Expression* type, MM_Expression** expression, MM_Text_Pos pos)
{
    MM_Argument* args = 0;
    
    if (!MM_IsToken(MM_Token_CloseBrace))
    {
        if (MM_Parser__ParseArguments(parser, &args)) return MM_false;
    }
    
    if (!MM_EatToken(MM_Token_CloseBrace))
    {
        //// ERROR
        MM_ERROR(MM_ParserError_MissingCloseBrace);
    }
    
    *expression = MM_PushNode(MM_AST_StructLiteral, pos);
    (*expression)->struct_lit_expr.type = 0;
    (*expression)->struct_lit_expr.args = args;
    
    return MM_true;
}

MM_bool
MM_Parser__ParseArrayLiteralArgs(MM_Parser* parser, MM_Expression* elem_type, MM_Expression** expression, MM_Text_Pos pos)
{
    MM_Argument* args = 0;
    
    if (!MM_IsToken(MM_Token_CloseBracket))
    {
        if (MM_Parser__ParseArguments(parser, &args)) return MM_false;
    }
    
    if (!MM_EatToken(MM_Token_CloseBracket))
    {
        //// ERROR
        MM_ERROR(MM_ParserError_MissingCloseBrace);
    }
    
    *expression = MM_PushNode(MM_AST_ArrayLiteral, pos);
    (*expression)->array_lit_expr.elem_type = elem_type;
    (*expression)->array_lit_expr.args      = args;
    
    return MM_true;
}

MM_bool
MM_Parser__ParseString(MM_Parser* parser, MM_Token token, MM_String_Literal* string_literal)
{
    MM_Arena* arena = MM_String_BeginIntern(parser->string_table);
    
    MM_String string = {0};
    MM_u8* buffer    = MM_Arena_Push(arena, token.size, 1);
    
    MM_Error error = MM_Lexer_ParseString(&parser->lexer, token, buffer, &string);
    if (error.code != MM_Error_None)
    {
        MM_String_AbortIntern(parser->string_table);
        
        //// ERROR
        MM_ASSERT(parser->error.code == MM_Error_None);
        parser->error = error;
        return MM_false;
    }
    else
    {
        MM_Arena_Pop(arena, token.size - string.size);
        
        *string_literal = MM_String_EndIntern(parser->string_table, string);
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParsePrimaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    MM_Text_Pos pos = MM_GetPos();
    
    if (MM_IsToken(MM_Token_Identifier) || MM_IsToken(MM_Token_BlankIdentifier))
    {
        *expression = MM_PushNode(MM_AST_Identifier, pos);
        (*expression)->ident_expr.ident = MM_Parser__PushIdentifier(parser, MM_GetToken());
    }
    else if (MM_IsTokenGroup(MM_TokenGroup_Integer))
    {
        MM_Token token = MM_GetToken();
        
        MM_u8 explicit_base = (token.kind == MM_Token_BinaryInt ? 2  :
                               token.kind == MM_Token_HexInt    ? 16 : 10);
        
        MM_Soft_Int value;
        MM_Error error = MM_Lexer_ParseInt(&parser->lexer, token, &value);
        
        if (error.code != MM_Error_None)
        {
            //// ERROR
            MM_ASSERT(parser->error.code == MM_Error_None);
            parser->error = error;
            return MM_false;
        }
        else
        {
            *expression = MM_PushNode(MM_AST_Int, pos);
            (*expression)->int_expr.value         = value;
            (*expression)->int_expr.explicit_base = explicit_base;
            
            MM_NextToken();
        }
    }
    else if (MM_IsTokenGroup(MM_TokenGroup_Float))
    {
        MM_Token token = MM_GetToken();
        
        MM_u8 explicit_size = (token.kind == MM_Token_Float      ? 0 :
                               token.kind == MM_Token_HexFloat16 ? 2 :
                               token.kind == MM_Token_HexFloat32 ? 4 : 8);
        
        MM_Soft_Float value;
        MM_Error error = MM_Lexer_ParseFloat(&parser->lexer, token, &value);
        
        if (error.code != MM_Error_None)
        {
            //// ERROR
            MM_ASSERT(parser->error.code == MM_Error_None);
            parser->error = error;
            return MM_false;
        }
        else
        {
            *expression = MM_PushNode(MM_AST_Float, pos);
            (*expression)->float_expr.value         = value;
            (*expression)->float_expr.explicit_size = explicit_size;
            
            MM_NextToken();
        }
    }
    else if (MM_IsToken(MM_Token_String))
    {
        MM_String_Literal string;
        if (!MM_Parser__ParseString(parser, MM_GetToken(), &string)) return MM_false;
        else
        {
            *expression = MM_PushNode(MM_AST_String, pos);
            (*expression)->string_expr.str = string;
            
            MM_NextToken();
        }
    }
    else if (MM_IsToken(MM_Token_Codepoint))
    {
        MM_Token token = MM_GetToken();
        
        MM_u32 codepoint;
        MM_Error error = MM_Lexer_ParseCodepoint(&parser->lexer, token, &codepoint);
        
        if (error.code != MM_Error_None)
        {
            //// ERROR
            MM_ASSERT(parser->error.code == MM_Error_None);
            parser->error = error;
            return MM_false;
        }
        else
        {
            *expression = MM_PushNode(MM_AST_Codepoint, pos);
            (*expression)->codepoint_expr.value = codepoint;
            
            MM_NextToken();
        }
    }
    else if (MM_IsTokenGroup(MM_TokenGroup_Keyword))
    {
        if (MM_IsToken(MM_Token_True) || MM_IsToken(MM_Token_False))
        {
            MM_bool boolean = MM_IsToken(MM_Token_True);
            
            *expression = MM_PushNode(MM_AST_Bool, pos);
            (*expression)->bool_expr.value = boolean;
            
            MM_NextToken();
        }
        else if (MM_IsToken(MM_Token_This))
        {
            *expression = MM_PushNode(MM_AST_This, pos);
            MM_NextToken();
        }
        else if (MM_EatToken(MM_Token_Proc))
        {
            MM_Parameter* params           = 0;
            MM_Return_Value* return_values = 0;
            
            if (MM_EatToken(MM_Token_OpenParen))
            {
                if (!MM_IsToken(MM_Token_CloseParen))
                {
                    if (!MM_Parser__ParseParameters(parser, &params)) return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    //// ERROR
                    MM_ERROR(MM_ParserError_MissingCloseParen);
                }
            }
            
            if (MM_EatToken(MM_Token_Arrow))
            {
                if (!MM_EatToken(MM_Token_OpenParen))
                {
                    MM_Text_Pos ret_pos = MM_GetPos();
                    
                    MM_Expression* type = 0;
                    if (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
                    
                    return_values = MM_PushNode(MM_AST_ReturnValue, ret_pos);
                    return_values->next  = 0;
                    return_values->names = 0;
                    return_values->type  = type;
                    return_values->value = 0;
                }
                else
                {
                    MM_Text_Pos ret_pos = MM_GetPos();
                    
                    MM_Expression* expressions = 0;
                    
                    MM_Expression** next_expression = &expressions;
                    while (MM_true)
                    {
                        if (!MM_Parser__ParseExpression(parser, next_expression)) return MM_false;
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
                            
                            *next_ret_val = MM_PushNode(MM_AST_ReturnValue, expr->pos);
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
                                //// ERROR
                                MM_ERROR(MM_ParserError_MissingColonBetweenRetValNamesAndType);
                            }
                            else
                            {
                                if (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
                                else
                                {
                                    if (MM_EatToken(MM_Token_Equals))
                                    {
                                        if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
                                    }
                                    
                                    *next_ret_val = MM_PushNode(MM_AST_ReturnValue, ret_pos);
                                    (*next_ret_val)->names = names;
                                    (*next_ret_val)->type  = type;
                                    (*next_ret_val)->value = value;
                                    
                                    (*next_ret_val)->next = 0;
                                    next_ret_val = &(*next_ret_val)->next;
                                    
                                    if (!MM_EatToken(MM_Token_Comma)) break;
                                    else
                                    {
                                        ret_pos = MM_GetPos();
                                        
                                        names = 0;
                                        MM_Expression** next_name = &names;
                                        while (MM_true)
                                        {
                                            if (!MM_Parser__ParseExpression(parser, next_name)) return MM_false;
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
                *expression = MM_PushNode(MM_AST_ProcLiteralFwdDecl, pos);
                (*expression)->proc_lit_fwd_decl_expr.params      = params;
                (*expression)->proc_lit_fwd_decl_expr.return_vals = return_values;
            }
            else if (!MM_IsToken(MM_Token_OpenBrace))
            {
                *expression = MM_PushNode(MM_AST_Proc, pos);
                (*expression)->proc_expr.params      = params;
                (*expression)->proc_expr.return_vals = return_values;
            }
            else
            {
                MM_Block_Statement* body = 0;
                
                if (!MM_Parser__ParseBlock(parser, &body)) return MM_false;
                
                *expression = MM_PushNode(MM_AST_ProcLiteral, pos);
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
                *expression = MM_PushNode(MM_AST_StructFwdDecl, pos);
            }
            else
            {
                MM_Declaration* body = 0;
                
                if (!MM_EatToken(MM_Token_OpenBrace))
                {
                    //// ERROR
                    if (is_struct) MM_ERROR(MM_ParserError_MissingStructBody);
                    else           MM_ERROR(MM_ParserError_MissingUnionBody);
                }
                else
                {
                    if (!MM_IsToken(MM_Token_CloseBrace))
                    {
                        MM_Declaration** next_decl = &body;
                        
                        while (MM_true)
                        {
                            MM_Statement* statement = 0;
                            if (!MM_Parser__ParseStatement(parser, &statement)) return MM_false;
                            
                            // NOTE: Whether this is an actual declaration or just a regular statement is caught in the checker
                            *next_decl = (MM_Declaration*)statement;
                            next_decl  = &(*next_decl)->next;
                        }
                    }
                    
                    if (!MM_EatToken(MM_Token_CloseBrace))
                    {
                        //// ERROR
                        MM_ERROR(MM_ParserError_MissingCloseBrace);
                    }
                    
                    if (is_struct)
                    {
                        *expression = MM_PushNode(MM_AST_Struct, pos);
                        (*expression)->struct_expr.body = body;
                    }
                    else
                    {
                        *expression = MM_PushNode(MM_AST_Union, pos);
                        (*expression)->union_expr.body = body;
                    }
                }
            }
        }
        else if (MM_IsToken(MM_Token_Enum))
        {
            MM_NextToken();
            
            MM_Expression* member_type = 0;
            
            if (!MM_IsToken(MM_Token_TripleMinus) && !MM_IsToken(MM_Token_OpenBrace))
            {
                if (!MM_Parser__ParseExpression(parser, &member_type)) return MM_false;
            }
            
            if (MM_EatToken(MM_Token_TripleMinus))
            {
                *expression = MM_PushNode(MM_AST_EnumFwdDecl, pos);
                (*expression)->enum_fwd_decl_expr.member_type = member_type;
            }
            else
            {
                MM_Enum_Member* members = 0;
                
                if (!MM_EatToken(MM_Token_OpenBrace))
                {
                    //// ERROR
                    MM_ERROR(MM_ParserError_MissingEnumBody);
                }
                else
                {
                    if (!MM_IsToken(MM_Token_CloseBrace))
                    {
                        MM_Text_Pos member_pos = MM_GetPos();
                        
                        MM_Enum_Member** next_member = &members;
                        while (MM_true)
                        {
                            MM_Expression* name  = 0;
                            MM_Expression* value = 0;
                            
                            if (!MM_Parser__ParseExpression(parser, &name)) return MM_false;
                            
                            if (MM_EatToken(MM_Token_Equals))
                            {
                                if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
                            }
                            
                            *next_member = MM_PushNode(MM_AST_EnumMember, member_pos);
                            (*next_member)->name  = name;
                            (*next_member)->value = value;
                            
                            next_member = &(*next_member)->next;
                            
                            if (MM_EatToken(MM_Token_Comma)) continue;
                            else                             break;
                        }
                    }
                    
                    if (!MM_EatToken(MM_Token_CloseBrace))
                    {
                        //// ERROR
                        MM_ERROR(MM_ParserError_MissingCloseBrace);
                    }
                    
                    *expression = MM_PushNode(MM_AST_Enum, pos);
                    (*expression)->enum_expr.member_type = member_type;
                    (*expression)->enum_expr.members     = members;
                }
            }
        }
        else
        {
            //// ERROR
            MM_ERROR(MM_ParserError_InvalidUseOfKeywordInExpression);
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
            //// ERROR
            MM_ERROR(MM_ParserError_MissingOpenParenAfterBuiltin);
        }
        else
        {
            if (!MM_IsToken(MM_Token_CloseParen))
            {
                if (!MM_Parser__ParseArguments(parser, &args)) return MM_false;
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                //// ERROR
                MM_ERROR(MM_ParserError_MissingCloseParen);
            }
            else
            {
                *expression = MM_PushNode(kind.kind, pos);
                (*expression)->builtin_expr.args = args;
            }
        }
    }
    else if (MM_EatToken(MM_Token_OpenParen))
    {
        MM_Expression* body = 0;
        if (!MM_Parser__ParseExpression(parser, &body)) return MM_false;
        
        if (!MM_EatToken(MM_Token_CloseParen))
        {
            //// ERROR
            MM_ERROR(MM_ParserError_MissingCloseParen);
        }
        
        *expression = MM_PushNode(MM_AST_Compound, pos);
        (*expression)->compound_expr.body = body;
    }
    else if (MM_EatToken(MM_Token_PeriodOpenBrace))
    {
        if (!MM_Parser__ParseStructLiteralArgs(parser, 0, expression, pos)) return MM_false;
    }
    else if (MM_EatToken(MM_Token_PeriodOpenBracket))
    {
        if (!MM_Parser__ParseArrayLiteralArgs(parser, 0, expression, pos)) return MM_false;
    }
    else
    {
        if (MM_IsToken(MM_Token_Invalid))
        {
            //// ERROR
            MM_ASSERT(parser->error.code == MM_Error_None);
            parser->error = MM_Lexer_GetError(&parser->lexer);
            return MM_false;
        }
        else
        {
            //// ERROR
            MM_ERROR(MM_ParserError_MissingPrimaryExpression);
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParsePostfixUnaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser__ParsePrimaryExpression(parser, expression)) return MM_false;
    
    while (MM_true)
    {
        if (MM_IsToken(MM_Token_Hat))
        {
            MM_Expression* operand = *expression;
            
            *expression = MM_PushNode(MM_AST_Dereference, operand->pos);
            (*expression)->unary_expr.operand = operand;
            
            MM_NextToken();
        }
        else if (MM_IsToken(MM_Token_Period))
        {
            if (!MM_IsToken(MM_Token_Identifier))
            {
                //// ERROR
                MM_ERROR(MM_ParserError_MissingNameOfMember);
            }
            else
            {
                MM_Expression* symbol = *expression;
                MM_Identifier member  = MM_Parser__PushIdentifier(parser, MM_GetToken());
                MM_NextToken();
                
                *expression = MM_PushNode(MM_AST_MemberAccess, symbol->pos);
                (*expression)->member_access_expr.symbol = symbol;
                (*expression)->member_access_expr.member = member;
            }
        }
        else if (MM_IsToken(MM_Token_OpenBracket))
        {
            MM_Expression* array = *expression;
            MM_Expression* first = 0;
            
            if (!MM_IsToken(MM_Token_Colon))
            {
                if (!MM_Parser__ParseExpression(parser, &first)) return MM_false;
            }
            
            if (MM_EatToken(MM_Token_Colon))
            {
                MM_Expression* start    = first;
                MM_Expression* past_end = 0;
                
                if (!MM_IsToken(MM_Token_CloseBracket))
                {
                    if (!MM_Parser__ParseExpression(parser, &past_end)) return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_CloseBracket))
                {
                    //// ERROR
                    MM_ERROR(MM_ParserError_MissingCloseBracket);
                }
                
                *expression = MM_PushNode(MM_AST_Slice, array->pos);
                (*expression)->slice_expr.array    = array;
                (*expression)->slice_expr.start    = start;
                (*expression)->slice_expr.past_end = past_end;
            }
            else
            {
                if (!MM_EatToken(MM_Token_CloseBracket))
                {
                    //// ERROR
                    MM_ERROR(MM_ParserError_MissingCloseBracket);
                }
                
                *expression = MM_PushNode(MM_AST_Subscript, array->pos);
                (*expression)->subscript_expr.array = array;
                (*expression)->subscript_expr.index = first;
            }
        }
        else if (MM_IsToken(MM_Token_OpenParen))
        {
            MM_Expression* proc = *expression;
            MM_Argument* args   = 0;
            
            if (!MM_IsToken(MM_Token_CloseParen))
            {
                if (MM_Parser__ParseArguments(parser, &args)) return MM_false;
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                //// ERROR
                MM_ERROR(MM_ParserError_MissingCloseParen);
            }
            
            *expression = MM_PushNode(MM_AST_Call, proc->pos);
            (*expression)->call_expr.proc = proc;
            (*expression)->call_expr.args = args;
        }
        else if (MM_EatToken(MM_Token_PeriodOpenBrace))
        {
            if (!MM_Parser__ParseStructLiteralArgs(parser, *expression, expression, (*expression)->pos)) return MM_false;
        }
        else if (MM_EatToken(MM_Token_PeriodOpenBracket))
        {
            if (!MM_Parser__ParseArrayLiteralArgs(parser, *expression, expression, (*expression)->pos)) return MM_false;
        }
        else break;
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParsePrefixUnaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    while (MM_true)
    {
        MM_Text_Pos pos = MM_GetPos();
        
        if (MM_EatToken(MM_Token_Backslash))
        {
            *expression = MM_PushNode(MM_AST_BackScope, pos);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Hat))
        {
            *expression = MM_PushNode(MM_AST_Reference, pos);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Minus))
        {
            *expression = MM_PushNode(MM_AST_Neg, pos);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Tilde))
        {
            *expression = MM_PushNode(MM_AST_BitNot, pos);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Bang))
        {
            *expression = MM_PushNode(MM_AST_Not, pos);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_OpenBracket))
        {
            if (MM_EatToken(MM_Token_CloseBracket))
            {
                *expression = MM_PushNode(MM_AST_SliceType, pos);
                expression = &(*expression)->unary_expr.operand;
            }
            else
            {
                MM_Expression* size = 0;
                if (!MM_Parser__ParseExpression(parser, &size)) return MM_false;
                
                *expression = MM_PushNode(MM_AST_ArrayType, pos);
                (*expression)->array_type_expr.size = size;
                expression = &(*expression)->array_type_expr.elem_type;
            }
        }
        else break;
    }
    
    return MM_Parser__ParsePostfixUnaryExpression(parser, expression);
}

MM_bool
MM_Parser__ParseBinaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser__ParsePrefixUnaryExpression(parser, expression)) return MM_false;
    
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
            if (!MM_Parser__ParsePrefixUnaryExpression(parser, &right)) return MM_false;
            
            // NOTE: Binary expression sub groups are sorted after precedence, and an expression with a lower precedence than a
            //       binary expression can never be the direct child of a binary expression.
            while (((*spot)->kind_group == MM_ASTGroup_BinaryExpression) & ((*spot)->kind_sub_group > kind.kind_sub_group))
            {
                spot = &(*spot)->binary_expr.right;
            }
            
            MM_Expression* left = *spot;
            
            *spot = MM_PushNode(kind.kind, left->pos);
            (*spot)->binary_expr.left  = left;
            (*spot)->binary_expr.right = right;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseConditionalExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser__ParseBinaryExpression(parser, expression)) return MM_false;
    
    if (MM_IsToken(MM_Token_QuestionMark))
    {
        MM_Expression* condition  = *expression;
        MM_Expression* true_expr  = 0;
        MM_Expression* false_expr = 0;
        
        if (!MM_Parser__ParseBinaryExpression(parser, &true_expr)) return MM_false;
        
        if (!MM_EatToken(MM_Token_Colon))
        {
            //// ERROR
            MM_ERROR(MM_ParserError_MissingFalseClause);
        }
        
        if (!MM_Parser__ParseBinaryExpression(parser, &false_expr)) return MM_false;
        
        *expression = MM_PushNode(MM_AST_Conditional, condition->pos);
        (*expression)->conditional_expr.condition  = condition;
        (*expression)->conditional_expr.true_expr  = true_expr;
        (*expression)->conditional_expr.false_expr = false_expr;
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseExpression(MM_Parser* parser, MM_Expression** expression)
{
    return MM_Parser__ParseConditionalExpression(parser, expression);
}

MM_bool
MM_Parser__ParseBlock(MM_Parser* parser, MM_Block_Statement** block)
{
    MM_ASSERT(MM_IsToken(MM_Token_OpenBrace));
    MM_Text_Pos pos = MM_GetPos();
    MM_NextToken();
    
    MM_Statement* body = 0;
    MM_Statement** next_statement = &body;
    while (!MM_IsToken(MM_Token_CloseBrace))
    {
        if (MM_IsToken(MM_Token_EndOfStream))
        {
            //// ERROR
            MM_ERROR(MM_ParserError_MissingCloseBrace);
        }
        else if (!MM_Parser__ParseStatement(parser, next_statement)) return MM_false;
        else
        {
            next_statement = &(*next_statement)->next;
            continue;
        }
    }
    
    MM_ASSERT(MM_IsToken(MM_Token_CloseBrace));
    MM_NextToken();
    
    *block = MM_PushNode(MM_AST_Block, pos);
    (*block)->body = body;
    
    return MM_true;
}

MM_bool
MM_Parser__ParseDeclAssignmentOrExpression(MM_Parser* parser, MM_Statement** statement)
{
    MM_Text_Pos pos = MM_GetPos();
    
    MM_bool is_using    = MM_false;
    MM_bool is_distinct = MM_false;
    
    while (MM_true)
    {
        if (MM_IsToken(MM_Token_Using))
        {
            if (is_using)
            {
                //// ERROR
                MM_ERROR(MM_ParserError_DuplicateUsing);
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
                //// ERROR
                MM_ERROR(MM_ParserError_DuplicateDistinct);
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
        if (!MM_Parser__ParseExpression(parser, next_expr)) return MM_false;
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
            if (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
        }
        
        if (MM_EatToken(MM_Token_Colon))
        {
            if (MM_EatToken(MM_Token_TripleMinus))
            {
                *statement = MM_PushNode(MM_AST_ConstantFwdDecl, pos);
                ((MM_Constant_Fwd_Declaration*)*statement)->names  = names;
                ((MM_Constant_Fwd_Declaration*)*statement)->type   = type;
            }
            else
            {
                MM_Expression* values = 0;
                
                MM_Expression** next_value = &values;
                while (MM_true)
                {
                    if (!MM_Parser__ParseExpression(parser, next_value)) return MM_false;
                    else
                    {
                        next_value = &(*next_value)->next;
                        
                        if (MM_EatToken(MM_Token_Comma)) continue;
                        else                             break;
                    }
                }
                
                *statement = MM_PushNode(MM_AST_Constant, pos);
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
                        if (!MM_Parser__ParseExpression(parser, next_value)) return MM_false;
                        else
                        {
                            next_value = &(*next_value)->next;
                            
                            if (MM_EatToken(MM_Token_Comma)) continue;
                            else                             break;
                        }
                    }
                }
            }
            
            *statement = MM_PushNode(MM_AST_Variable, pos);
            ((MM_Variable_Declaration*)*statement)->names            = names;
            ((MM_Variable_Declaration*)*statement)->type             = type;
            ((MM_Variable_Declaration*)*statement)->values           = values;
            ((MM_Variable_Declaration*)*statement)->is_using         = is_using;
            ((MM_Variable_Declaration*)*statement)->is_uninitialized = is_uninitialized;
        }
    }
    else
    {
        if (MM_IsTokenGroup(MM_TokenGroup_AssignmentOperator))
        {
            if (is_using)
            {
                //// ERROR
                MM_ERROR(MM_ParserError_UsingOnAssignment);
            }
            else
            {
                MM_AST_Kind kind = {
                    .kind_type      = MM_ASTType_Statement,
                    .kind_group     = MM_ASTGroup_Assignment,
                    .kind_sub_group = MM_AST_None,
                    .kind_index     = MM_GetToken().kind_index,
                };
                
                MM_NextToken();
                
                MM_Expression* lhs = expressions;
                MM_Expression* rhs = 0;
                
                MM_Expression** next_value = &rhs;
                while (MM_true)
                {
                    if (!MM_Parser__ParseExpression(parser, next_value)) return MM_false;
                    else
                    {
                        next_value = &(*next_value)->next;
                        
                        if (MM_EatToken(MM_Token_Comma)) continue;
                        else                             break;
                    }
                }
                
                *statement = MM_PushNode(kind.kind, pos);
                (*statement)->assignment_statement.lhs = lhs;
                (*statement)->assignment_statement.rhs = rhs;
            }
        }
        else if (is_using)
        {
            if (expressions->next != 0)
            {
                //// ERROR
                MM_ERROR(MM_ParserError_UsingMultipleSymbols);
            }
            else
            {
                MM_Expression* symbol = expressions;
                MM_Identifier alias   = {};
                
                if (MM_EatToken(MM_Token_As))
                {
                    if (!MM_IsToken(MM_Token_Identifier))
                    {
                        //// ERROR
                        MM_ERROR(MM_ParserError_MissingAliasName);
                    }
                    else
                    {
                        alias = MM_Parser__PushIdentifier(parser, MM_GetToken());
                        MM_NextToken();
                    }
                }
                
                *statement = MM_PushNode(MM_AST_Using, pos);
                ((MM_Using_Declaration*)*statement)->symbol = symbol;
                ((MM_Using_Declaration*)*statement)->alias  = alias;
            }
        }
        else
        {
            if (expressions->next != 0)
            {
                //// ERROR
                MM_ERROR(MM_ParserError_StrayExpressionList);
            }
            else *statement = (MM_Statement*)expressions;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseStatement(MM_Parser* parser, MM_Statement** statement)
{
    MM_Text_Pos pos = MM_GetPos();
    
    if (MM_IsToken(MM_Token_Semicolon))
    {
        //// ERROR
        MM_ERROR(MM_ParserError_StraySemicolon);
    }
    else if (MM_IsToken(MM_Token_Else))
    {
        //// ERROR
        MM_ERROR(MM_ParserError_ElseWithoutIf);
    }
    else if (MM_EatToken(MM_Token_Colon))
    {
        if (!MM_IsToken(MM_Token_Identifier))
        {
            if (MM_IsToken(MM_Token_BlankIdentifier))
            {
                //// ERROR
                MM_ERROR(MM_ParserError_BlankLabelName);
            }
            else
            {
                //// ERROR
                MM_ERROR(MM_ParserError_MissingLabelName);
            }
        }
        else
        {
            MM_Identifier label = MM_Parser__PushIdentifier(parser, MM_GetToken());
            MM_NextToken();
            
            if (!MM_Parser__ParseStatement(parser, statement)) return MM_false;
            else
            {
                (*statement)->pos = pos;
                
                if      ((*statement)->kind == MM_AST_If)    (*statement)->if_statement.label    = label;
                else if ((*statement)->kind == MM_AST_While) (*statement)->while_statement.label = label;
                else if ((*statement)->kind == MM_AST_Block) (*statement)->block_statement.label = label;
                else
                {
                    //// ERROR
                    MM_ERROR(MM_ParserError_InvalidUseOfLabel);
                }
            }
        }
    }
    else if (MM_IsToken(MM_Token_OpenBrace))
    {
        if (!MM_Parser__ParseBlock(parser, (MM_Block_Statement**)&statement)) return MM_false;
    }
    else if (MM_EatToken(MM_Token_If))
    {
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            //// ERROR
            MM_ERROR(MM_ParserError_MissingOpenParenAfterIf);
        }
        else
        {
            MM_Statement* first = 0;
            
            if (!MM_IsToken(MM_Token_Semicolon))
            {
                if (!MM_Parser__ParseDeclAssignmentOrExpression(parser, &first)) return MM_false;
            }
            
            MM_Statement* init       = 0;
            MM_Expression* condition = 0;
            
            if (!MM_EatToken(MM_Token_Semicolon)) condition = (MM_Expression*)first;
            else
            {
                init = first;
                if (!MM_Parser__ParseExpression(parser, &condition)) return MM_false;
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                //// ERROR
                MM_ERROR(MM_ParserError_MissingCloseParen);
            }
            else
            {
                MM_Statement* true_body  = 0;
                MM_Statement* false_body = 0;
                
                if (!MM_Parser__ParseStatement(parser, &true_body)) return MM_false;
                else
                {
                    if (MM_EatToken(MM_Token_Else))
                    {
                        if (!MM_Parser__ParseStatement(parser, &false_body)) return MM_false;
                    }
                    
                    *statement = MM_PushNode(MM_AST_If, pos);
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
            //// ERROR
            MM_ERROR(MM_ParserError_MissingOpenParenAfterWhen);
        }
        else
        {
            MM_Expression* condition = 0;
            
            if (!MM_Parser__ParseExpression(parser, &condition)) return MM_false;
            else
            {
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    //// ERROR
                    MM_ERROR(MM_ParserError_MissingCloseParen);
                }
                else
                {
                    MM_Statement* true_body  = 0;
                    MM_Statement* false_body = 0;
                    
                    if (!MM_Parser__ParseStatement(parser, &true_body)) return MM_false;
                    else
                    {
                        if (MM_EatToken(MM_Token_Else))
                        {
                            if (!MM_Parser__ParseStatement(parser, &false_body)) return MM_false;
                        }
                        
                        *statement = MM_PushNode(MM_AST_When, pos);
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
            //// ERROR
            MM_ERROR(MM_ParserError_MissingOpenParenAfterWhile);
        }
        else
        {
            MM_Statement* first = 0;
            
            if (!MM_IsToken(MM_Token_Semicolon))
            {
                if (!MM_Parser__ParseDeclAssignmentOrExpression(parser, &first)) return MM_false;
            }
            
            MM_Statement* init       = 0;
            MM_Expression* condition = 0;
            MM_Statement* step       = 0;
            
            if (!MM_EatToken(MM_Token_Semicolon)) condition = (MM_Expression*)first;
            else
            {
                init = first;
                
                if (!MM_IsToken(MM_Token_Semicolon))
                {
                    if (!MM_Parser__ParseDeclAssignmentOrExpression(parser, (MM_Statement**)&condition)) return MM_false;
                }
                
                if (MM_EatToken(MM_Token_Semicolon))
                {
                    if (!MM_Parser__ParseDeclAssignmentOrExpression(parser, &step)) return MM_false;
                }
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                //// ERROR
                MM_ERROR(MM_ParserError_MissingCloseParen);
            }
            else
            {
                MM_Statement* body = 0;
                if (!MM_Parser__ParseStatement(parser, &body)) return MM_false;
                else
                {
                    *statement = MM_PushNode(MM_AST_While, pos);
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
        MM_NextToken();
        
        
        MM_Identifier label = {};
        if (!MM_IsToken(MM_Token_Semicolon))
        {
            if (MM_IsToken(MM_Token_Identifier))
            {
                label = MM_Parser__PushIdentifier(parser, MM_GetToken());
                MM_NextToken();
            }
            else if (MM_IsToken(MM_Token_BlankIdentifier))
            {
                //// ERROR
                MM_ERROR(MM_ParserError_BlankJumpLabel);
            }
            else
            {
                //// ERROR
                MM_ERROR(MM_ParserError_MissingJumpLabel);
            }
        }
        
        if (!MM_EatToken(MM_Token_Semicolon))
        {
            //// ERROR
            MM_ERROR(MM_ParserError_MissingSemicolon);
        }
        else
        {
            *statement = MM_PushNode(kind, pos);
            (*statement)->jump_statement.label = label;
        }
    }
    else if (MM_EatToken(MM_Token_Defer))
    {
        MM_Statement* body = 0;
        
        if (!MM_Parser__ParseStatement(parser, &body)) return MM_false;
        
        *statement = MM_PushNode(MM_AST_Defer, pos);
        (*statement)->defer_statement.body = body;
    }
    else if (MM_EatToken(MM_Token_Return))
    {
        MM_Argument* args = 0;
        
        if (!MM_IsToken(MM_Token_Semicolon))
        {
            if (!MM_Parser__ParseArguments(parser, &args)) return MM_false;
        }
        
        if (MM_EatToken(MM_Token_Semicolon))
        {
            //// ERROR
            MM_ERROR(MM_ParserError_MissingSemicolon);
        }
        else
        {
            *statement = MM_PushNode(MM_AST_Return, pos);
            (*statement)->return_statement.args = args;
        }
    }
    else if (MM_EatToken(MM_Token_Include))
    {
        if (!MM_IsToken(MM_Token_String))
        {
            //// ERROR
            MM_ERROR(MM_ParserError_MissingPathAfterInclude);
        }
        else
        {
            MM_String_Literal path;
            if (!MM_Parser__ParseString(parser, MM_GetToken(), &path)) return MM_false;
            else
            {
                if (MM_EatToken(MM_Token_Semicolon))
                {
                    //// ERROR
                    MM_ERROR(MM_ParserError_MissingSemicolon);
                }
                else
                {
                    *statement = MM_PushNode(MM_AST_Include, pos);
                    ((MM_Include_Declaration*)*statement)->path = path;
                }
            }
        }
    }
    else
    {
        if (!MM_Parser__ParseDeclAssignmentOrExpression(parser, statement)) return MM_false;
        else
        {
            MM_bool should_have_semi = MM_true;
            
            if ((*statement)->kind == MM_AST_Constant && ((MM_Constant_Declaration*)*statement)->values->next == 0)
            {
                MM_u32 kind = ((MM_Constant_Declaration*)*statement)->values->kind;
                should_have_semi = !(kind == MM_AST_ProcLiteral || kind == MM_AST_Struct || kind == MM_AST_Union || kind == MM_AST_Enum);
            }
            
            if (should_have_semi)
            {
                if (!MM_EatToken(MM_Token_Semicolon))
                {
                    //// ERROR
                    MM_ERROR(MM_ParserError_MissingSemicolon);
                }
            }
        }
    }
    
    return MM_true;
}

MM_Error
MM_ParseString(MM_String string, MM_Text_Pos pos, MM_Arena* ast_arena,
               MM_String_Intern_Table* identifier_table, MM_String_Intern_Table* string_table,
               MM_AST** ast)
{
    MM_Parser _parser = {
        .lexer        = MM_Lexer_LexFromPos(string, pos),
        .ast_arena    = ast_arena,
        .ident_table  = identifier_table,
        .string_table = string_table,
        .error        = MM_ErrorNone,
    };
    
    MM_Parser* parser = &_parser;
    
    *ast = 0;
    MM_Statement** next_statement = (MM_Statement**)ast;
    while (!MM_IsToken(MM_Token_EndOfStream))
    {
        if (MM_Parser__ParseStatement(parser, next_statement)) next_statement = &(*next_statement)->next;
        else break;
    }
    
    return parser->error;
}

#undef MM_GetToken
#undef MM_NextToken
#undef MM_IsToken
#undef MM_IsTokenGroup
#undef MM_EatToken
#undef MM_GetPos
#undef MM_PushNode
#undef MM_ERROR