typedef struct MM_Parser
{
    MM_Lexer lexer;
    MM_Arena* ast_arena;
} MM_Parser;

void*
MM_Parser__PushNode(MM_Parser* parser, MM_AST_Kind kind)
{
    MM_AST* node = MM_Arena_Push(parser->ast_arena, sizeof(MM_AST), MM_ALIGNOF(MM_AST));
    node->kind = kind;
    node->next = 0;
    
    return node;
}

#define MM_NextToken() MM_Lexer_NextToken(&parser->lexer)
#define MM_GetToken()  MM_Lexer_GetToken(&parser->lexer)
#define MM_IsToken(k)  (MM_GetToken().kind == (k))
#define MM_EatToken(k) (MM_IsToken(k) ? MM_NextToken(), MM_true : MM_false)
#define MM_PushNode(k) MM_Parser__PushNode(parser, (k))

MM_bool MM_Parser__ParseExpression(MM_Parser* parser, MM_Expression** expression);
MM_bool MM_Parser__ParseDeclaration(MM_Parser* parser, MM_Declaration** declaration);
MM_bool MM_Parser__ParseBlock(MM_Parser* parser, MM_Block_Statement** block);

MM_bool
MM_Parser__ParseExpressionList(MM_Parser* parser, MM_Expression** expressions)
{
    MM_Expression** next_expr = expressions;
    for (;;)
    {
        if (!MM_Parser__ParseExpression(parser, next_expr)) return MM_false;
        else
        {
            if (!MM_EatToken(MM_Token_Comma)) break;
            else
            {
                next_expr = &(*next_expr)->next;
                continue;
            }
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseParameterList(MM_Parser* parser, MM_Parameter** params)
{
    MM_Expression* expressions;
    if (!MM_Parser__ParseExpressionList(parser, &expressions)) return MM_false;
    else
    {
        // NOTE: special type of parameter list, where only types are used, no names and values
        if (!MM_IsToken(MM_Token_Colon))
        {
            MM_Parameter** next_param = params;
            for (MM_Expression* expr = expressions; expr != 0; expr = expr->next)
            {
                *next_param = MM_PushNode(MM_AST_Parameter);
                (*next_param)->names = 0;
                (*next_param)->type  = expr;
                (*next_param)->value = 0;
                
                next_param = &(*next_param)->next;
            }
        }
        else
        {
            MM_Parameter** next_param = params;
            MM_Expression* names      = expressions;
            for (;;)
            {
                if (!MM_EatToken(MM_Token_Colon))
                {
                    //// ERROR: Missing colon before type of parameters
                    return MM_false;
                }
                else
                {
                    MM_Expression* type = 0;
                    if (!MM_IsToken(MM_Token_Equals))
                    {
                        if (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
                    }
                    
                    MM_Expression* value = 0;
                    if (MM_EatToken(MM_Token_Equals))
                    {
                        if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
                    }
                    
                    *next_param = MM_PushNode(MM_AST_Parameter);
                    (*next_param)->names = names;
                    (*next_param)->type  = type;
                    (*next_param)->value = value;
                    
                    if (!MM_EatToken(MM_Token_Comma)) break;
                    else
                    {
                        if (!MM_Parser__ParseExpressionList(parser, &names)) return MM_false;
                        next_param = &(*next_param)->next;
                    }
                }
            }
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseReturnValueList(MM_Parser* parser, MM_Return_Value** return_vals)
{
    MM_Expression* expressions;
    if (!MM_Parser__ParseExpressionList(parser, &expressions)) return MM_false;
    else
    {
        // NOTE: special type of return value list, where only types are used, no names and values
        if (!MM_IsToken(MM_Token_Colon))
        {
            MM_Return_Value** next_ret_val = return_vals;
            for (MM_Expression* expr = expressions; expr != 0; expr = expr->next)
            {
                *next_ret_val = MM_PushNode(MM_AST_ReturnValue);
                (*next_ret_val)->names = 0;
                (*next_ret_val)->type  = expr;
                (*next_ret_val)->value = 0;
                
                next_ret_val = &(*next_ret_val)->next;
            }
        }
        else
        {
            MM_Return_Value** next_ret_val = return_vals;
            MM_Expression* names           = expressions;
            for (;;)
            {
                if (!MM_EatToken(MM_Token_Colon))
                {
                    //// ERROR: Missing colon before type of parameters
                    return MM_false;
                }
                else
                {
                    MM_Expression* type = 0;
                    if (!MM_IsToken(MM_Token_Equals))
                    {
                        if (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
                    }
                    
                    MM_Expression* value = 0;
                    if (MM_EatToken(MM_Token_Equals))
                    {
                        if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
                    }
                    
                    *next_ret_val = MM_PushNode(MM_AST_ReturnValue);
                    (*next_ret_val)->names = names;
                    (*next_ret_val)->type  = type;
                    (*next_ret_val)->value = value;
                    
                    if (!MM_EatToken(MM_Token_Comma)) break;
                    else
                    {
                        if (!MM_Parser__ParseExpressionList(parser, &names)) return MM_false;
                        next_ret_val = &(*next_ret_val)->next;
                    }
                }
            }
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseArgumentList(MM_Parser* parser, MM_Argument** arguments)
{
    MM_Argument** next_arg = arguments;
    for (;;)
    {
        MM_Expression* expr;
        if (MM_Parser__ParseExpression(parser, &expr)) return MM_false;
        else
        {
            MM_Expression* name = 0;
            MM_Expression* value;
            if (!MM_EatToken(MM_Token_Colon)) value = expr;
            else
            {
                name = expr;
                if (MM_Parser__ParseExpression(parser, &value)) return MM_false;
            }
            
            *next_arg = MM_PushNode(MM_AST_Argument);
            (*next_arg)->name  = name;
            (*next_arg)->value = value;
            
            if (MM_EatToken(MM_Token_Comma)) next_arg = &(*next_arg)->next;
            else                             break;
        }
    }
    
    return MM_true;
}



MM_bool
MM_Parser__ParsePrimary(MM_Parser* parser, MM_Expression** expression)
{
    if (MM_IsToken(MM_Token_Identifier))
    {
        MM_NOT_IMPLEMENTED;
    }
    else if (MM_IsToken(MM_Token_Int))
    {
        *expression = MM_PushNode(MM_AST_Int);
        (*expression)->integer = MM_GetToken().i128;
        MM_NextToken();
    }
    else if (MM_IsToken(MM_Token_Float))
    {
        *expression = MM_PushNode(MM_AST_Int);
        (*expression)->floating = MM_GetToken().f64;
        MM_NextToken();
    }
    else if (MM_IsToken(MM_Token_String))
    {
        MM_NOT_IMPLEMENTED;
    }
    else if (MM_IsToken(MM_Token_True) || MM_IsToken(MM_Token_False))
    {
        MM_bool is_true = MM_IsToken(MM_Token_True);
        MM_NextToken();
        
        *expression = MM_PushNode(MM_AST_Bool);
        (*expression)->boolean = is_true;
    }
    else if (MM_EatToken(MM_Token_Proc))
    {
        MM_Parameter* parameters = 0;
        if (MM_EatToken(MM_Token_OpenParen))
        {
            if (!MM_IsToken(MM_Token_CloseParen))
            {
                if (!MM_Parser__ParseParameterList(parser, &parameters)) return MM_false;
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                //// ERROR: Missing closing paren after procedure parameter list
                return MM_false;
            }
        }
        
        MM_Return_Value* return_vals = 0;
        if (MM_EatToken(MM_Token_Arrow))
        {
            if (MM_IsToken(MM_Token_OpenParen))
            {
                if (!MM_Parser__ParseReturnValueList(parser, &return_vals)) return MM_false;
                
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    //// ERROR: Missing closing paren after procedure return value list
                    return MM_false;
                }
            }
            else
            {
                MM_Expression* type;
                if (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
                else
                {
                    return_vals = MM_PushNode(MM_AST_ReturnValue);
                    return_vals->names = 0;
                    return_vals->type  = type;
                    return_vals->value = 0;
                }
            }
        }
        
        if (MM_EatToken(MM_Token_TpMinus))
        {
            *expression = MM_PushNode(MM_AST_ProcLit);
            (*expression)->proc_lit_expr.params      = parameters;
            (*expression)->proc_lit_expr.return_vals = return_vals;
            (*expression)->proc_lit_expr.body        = 0;
        }
        else if (MM_IsToken(MM_Token_OpenBrace))
        {
            MM_Block_Statement* body;
            if (!MM_Parser__ParseBlock(parser, &body)) return MM_false;
            else
            {
                *expression = MM_PushNode(MM_AST_ProcLit);
                (*expression)->proc_lit_expr.params      = parameters;
                (*expression)->proc_lit_expr.return_vals = return_vals;
                (*expression)->proc_lit_expr.body        = body;
            }
        }
        else
        {
            *expression = MM_PushNode(MM_AST_ProcType);
            (*expression)->proc_type_expr.params      = parameters;
            (*expression)->proc_type_expr.return_vals = return_vals;
        }
    }
    else if (MM_EatToken(MM_Token_Struct))
    {
        if (!MM_EatToken(MM_Token_OpenBrace))
        {
            //// ERROR: Missing body of struct
            return MM_false;
        }
        else
        {
            MM_Declaration* members = 0;
            MM_Declaration** next_member = &members;
            for (;;)
            {
                if      (MM_EatToken(MM_Token_CloseBrace)) break;
                else if (MM_IsToken(MM_Token_EOF))
                {
                    //// ERROR: Missing closing brace after struct body
                    return MM_false;
                }
                else
                {
                    if (!MM_Parser__ParseDeclaration(parser, next_member)) return MM_false;
                    else next_member = &(*next_member)->next;
                }
            }
            
            *expression = MM_PushNode(MM_AST_StructType);
            (*expression)->struct_type_expr.members = members;
        }
    }
    else if (MM_EatToken(MM_Token_OpenParen))
    {
        MM_Expression* expr;
        if      (!MM_Parser__ParseExpression(parser, &expr)) return MM_false;
        else if (!MM_EatToken(MM_Token_CloseParen))
        {
            //// ERROR: Missing closing paren after compound expression
            return MM_false;
        }
        else
        {
            *expression = MM_PushNode(MM_AST_Compound);
            (*expression)->compound_expr.expr = expr;
        }
    }
    else if (MM_EatToken(MM_Token_PeriodBrace))
    {
        MM_Argument* args = 0;
        if (!MM_IsToken(MM_Token_CloseBrace))
        {
            if (!MM_Parser__ParseArgumentList(parser, &args)) return MM_false;
        }
        
        if (!MM_EatToken(MM_Token_CloseBrace))
        {
            //// ERROR: Missing closing brace after arguments to struct literal
            return MM_false;
        }
        else
        {
            *expression = MM_PushNode(MM_AST_StructLit);
            (*expression)->struct_lit_expr.type = 0;
            (*expression)->struct_lit_expr.args = args;
        }
    }
    else if (MM_EatToken(MM_Token_PeriodBracket))
    {
        MM_Argument* args = 0;
        if (!MM_IsToken(MM_Token_CloseBrace))
        {
            if (!MM_Parser__ParseArgumentList(parser, &args)) return MM_false;
        }
        
        if (!MM_EatToken(MM_Token_CloseBrace))
        {
            //// ERROR: Missing closing brace after arguments to array literal
            return MM_false;
        }
        else
        {
            *expression = MM_PushNode(MM_AST_ArrayLit);
            (*expression)->array_lit_expr.type = 0;
            (*expression)->array_lit_expr.args = args;
        }
    }
    else
    {
        if (MM_IsToken(MM_Token_Invalid))
        {
            //// ERROR: Lexer error
            return MM_false;
        }
        else
        {
            //// ERROR: Missing primary expression
            return MM_false;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseTypePrefix(MM_Parser* parser, MM_Expression** expression)
{
    for (;;)
    {
        if (MM_EatToken(MM_Token_Hat))
        {
            *expression = MM_PushNode(MM_AST_PointerType);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_OpenBracket))
        {
            if (MM_EatToken(MM_Token_CloseBracket))
            {
                *expression = MM_PushNode(MM_AST_SliceType);
                expression = &(*expression)->unary_expr.operand;
            }
            else
            {
                MM_Expression* size;
                if (!MM_Parser__ParseExpression(parser, &size)) return MM_false;
                
                if (!MM_EatToken(MM_Token_CloseBracket))
                {
                    //// ERROR: Missing closing bracket after size in array type prefix
                    return MM_false;
                }
                else
                {
                    *expression = MM_PushNode(MM_AST_ArrayType);
                    (*expression)->array_type_expr.size = size;
                    
                    expression = &(*expression)->array_type_expr.array;
                }
            }
        }
        else break;
    }
    
    return MM_Parser__ParsePrimary(parser, expression);
}

MM_bool
MM_Parser__ParsePostfix(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser__ParseTypePrefix(parser, expression)) return MM_false;
    else
    {
        for (;;)
        {
            MM_Expression* operand = *expression;
            
            if (MM_EatToken(MM_Token_Hat))
            {
                *expression = MM_PushNode(MM_AST_Dereference);
                (*expression)->unary_expr.operand = operand;
            }
            else if (MM_EatToken(MM_Token_OpenBracket))
            {
                MM_Expression* expr = 0;
                
                if (!MM_IsToken(MM_Token_Colon))
                {
                    if (!MM_Parser__ParseExpression(parser, &expr)) return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_Colon))
                {
                    if (!MM_EatToken(MM_Token_CloseBracket))
                    {
                        //// ERROR: Missing closing bracket after index in subscript expression
                        return MM_false;
                    }
                    else
                    {
                        *expression = MM_PushNode(MM_AST_Subscript);
                        (*expression)->subscript_expr.array = operand;
                        (*expression)->subscript_expr.index = expr;
                    }
                }
                else
                {
                    MM_Expression* start_index    = expr;
                    MM_Expression* past_end_index = 0;
                    
                    if (!MM_IsToken(MM_Token_CloseBracket))
                    {
                        if (!MM_Parser__ParseExpression(parser, &past_end_index)) return MM_false;
                    }
                    
                    if (!MM_EatToken(MM_Token_CloseBracket))
                    {
                        //// ERROR: Missing closing bracket after range in slice expression
                        return MM_false;
                    }
                    else
                    {
                        *expression = MM_PushNode(MM_AST_Slice);
                        (*expression)->slice_expr.array          = operand;
                        (*expression)->slice_expr.start_index    = start_index;
                        (*expression)->slice_expr.past_end_index = past_end_index;
                    }
                }
            }
            else if (MM_EatToken(MM_Token_OpenParen))
            {
                MM_Argument* args = 0;
                if (!MM_IsToken(MM_Token_CloseParen))
                {
                    if (!MM_Parser__ParseArgumentList(parser, &args)) return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    //// ERROR: Missing closing paren after argument list to procedure call
                    return MM_false;
                }
                else
                {
                    *expression = MM_PushNode(MM_AST_Call);
                    (*expression)->call_expr.proc = operand;
                    (*expression)->call_expr.args = args;
                }
            }
            else if (MM_EatToken(MM_Token_Period))
            {
                if (!MM_IsToken(MM_Token_Identifier))
                {
                    //// ERROR: Missing name of member after member access operator
                    return MM_false;
                }
                else
                {
                    *expression = MM_PushNode(MM_AST_Member);
                    //(*expression)->member_expr.member = MM_GetToken().identifier;
                    MM_NOT_IMPLEMENTED;
                }
            }
            else if (MM_EatToken(MM_Token_PeriodBrace))
            {
                MM_Argument* args = 0;
                if (!MM_IsToken(MM_Token_CloseBrace))
                {
                    if (!MM_Parser__ParseArgumentList(parser, &args)) return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_CloseBrace))
                {
                    //// ERROR: Missing closing brace after arguments to struct literal
                    return MM_false;
                }
                else
                {
                    *expression = MM_PushNode(MM_AST_StructLit);
                    (*expression)->struct_lit_expr.type = operand;
                    (*expression)->struct_lit_expr.args = args;
                }
            }
            else if (MM_EatToken(MM_Token_PeriodBracket))
            {
                MM_Argument* args = 0;
                if (!MM_IsToken(MM_Token_CloseBrace))
                {
                    if (!MM_Parser__ParseArgumentList(parser, &args)) return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_CloseBrace))
                {
                    //// ERROR: Missing closing brace after arguments to array literal
                    return MM_false;
                }
                else
                {
                    *expression = MM_PushNode(MM_AST_ArrayLit);
                    (*expression)->array_lit_expr.type = operand;
                    (*expression)->array_lit_expr.args = args;
                }
            }
            else break;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParsePrefix(MM_Parser* parser, MM_Expression** expression)
{
    for (;;)
    {
        MM_AST_Kind kind = MM_Token_Invalid;
        switch (MM_GetToken().kind)
        {
            case MM_Token_Plus:  kind = MM_AST_Pos;       break;
            case MM_Token_Minus: kind = MM_AST_Neg;       break;
            case MM_Token_Bang:  kind = MM_AST_Not;       break;
            case MM_Token_Tilde: kind = MM_AST_BitNot;    break;
            case MM_Token_Hat:   kind = MM_AST_Reference; break;
        }
        
        if (kind == MM_Token_Invalid) break;
        else
        {
            *expression = MM_PushNode(kind);
            expression = &(*expression)->unary_expr.operand;
        }
    }
    
    return MM_Parser__ParsePostfix(parser, expression);
}

MM_bool
MM_Parser__ParseBinary(MM_Parser* parser, MM_Expression** expression)
{
    if (MM_Parser__ParsePrefix(parser, expression)) return MM_false;
    else
    {
        for (;;)
        {
            MM_Token token = MM_GetToken();
            if (!(token.kind >= MM_Token__FirstBinary && token.kind <= MM_Token__LastBinary)) break;
            else
            {
                // NOTE: This relation is manually maintained, MM_*_BLOCK is used to define precedence (see mm_ast.h and mm_tokens.h)
                MM_AST_Kind kind = (MM_AST_Kind)token.kind;
                
                MM_NextToken();
                
                MM_Expression* right;
                if (!MM_Parser__ParseExpression(parser, &right)) return MM_false;
                else
                {
                    MM_Expression** slot = expression;
                    while (MM_AST_BLOCK_INDEX((*slot)->kind) > MM_AST_BLOCK_INDEX(kind))
                    {
                        MM_ASSERT((*slot)->kind >= MM_Token__FirstBinary && (*slot)->kind <= MM_Token__LastBinary);
                        slot = &(*slot)->binary_expr.right;
                    }
                    
                    MM_Expression* left = *slot;
                    
                    if (MM_AST_BLOCK_INDEX(left->kind) == MM_AST_CMP_BLOCK_INDEX &&
                        MM_AST_BLOCK_INDEX(kind)       == MM_AST_CMP_BLOCK_INDEX)
                    {
                        //// ERROR: Comparison operators cannot be chained
                        return MM_false;
                    }
                    else
                    {
                        *slot = MM_PushNode(kind);
                        (*slot)->binary_expr.left  = left;
                        (*slot)->binary_expr.right = right;
                    }
                }
            }
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseExpression(MM_Parser* parser, MM_Expression** expression)
{
    return MM_Parser__ParseBinary(parser, expression);
}



MM_bool
MM_Parser__ParseDeclaration(MM_Parser* parser, MM_Declaration** declaration)
{
    MM_NOT_IMPLEMENTED;
    return MM_true;
}



MM_bool
MM_Parser__ParseBlock(MM_Parser* parser, MM_Block_Statement** block)
{
    MM_NOT_IMPLEMENTED;
    return MM_true;
}

#undef MM_NextToken
#undef MM_GetToken
#undef MM_IsToken
#undef MM_EatToken
#undef MM_PushNode