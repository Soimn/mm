typedef struct MM_Parser
{
    MM_Lexer lexer;
    MM_Arena* ast_arena;
    MM_Arena* str_arena;
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
MM_bool MM_Parser__ParseStatement(MM_Parser* parser, MM_Statement** statement);
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
        if (!MM_Parser__ParseExpression(parser, &expr)) return MM_false;
        else
        {
            MM_Expression* name = 0;
            MM_Expression* value;
            if (!MM_EatToken(MM_Token_Colon)) value = expr;
            else
            {
                name = expr;
                if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
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
        *expression = MM_PushNode(MM_AST_Identifier);
        (*expression)->identifier_expr.value = MM_GetToken().identifier; // TODO: memory lifetime
        MM_NextToken();
    }
    else if (MM_IsToken(MM_Token_Int))
    {
        *expression = MM_PushNode(MM_AST_Int);
        (*expression)->int_expr.value = MM_GetToken().i128;
        MM_NextToken();
    }
    else if (MM_IsToken(MM_Token_Float))
    {
        *expression = MM_PushNode(MM_AST_Float);
        (*expression)->float_expr.value = MM_GetToken().f64;
        MM_NextToken();
    }
    else if (MM_IsToken(MM_Token_String))
    {
        *expression = MM_PushNode(MM_AST_String);
        (*expression)->string_expr.value = MM_GetToken().string;
        MM_NextToken();
    }
    else if (MM_IsToken(MM_Token_True) || MM_IsToken(MM_Token_False))
    {
        MM_bool is_true = MM_IsToken(MM_Token_True);
        MM_NextToken();
        
        *expression = MM_PushNode(MM_AST_Bool);
        (*expression)->bool_expr.value = is_true;
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
                    MM_Statement* statement = 0;
                    if (!MM_Parser__ParseStatement(parser, &statement)) return MM_false;
                    else
                    {
                        if (statement->kind >= MM_AST__FirstDeclaration && statement->kind <= MM_AST__LastDeclaration)
                        {
                            *next_member = (MM_Declaration*)statement;
                            next_member = &(*next_member)->next;
                        }
                        else
                        {
                            //// ERROR: Only declarations may appear in struct body
                            return MM_false;
                        }
                    }
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
    else if (MM_GetToken().kind >= MM_Token__FirstBuiltin && MM_GetToken().kind <= MM_Token__LastBuiltin)
    {
        MM_BuiltinCall_Kind builtin_kind = (MM_BuiltinCall_Kind)(MM_GetToken().kind - MM_Token__FirstBuiltin);
        MM_NextToken();
        
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            //// ERROR: Missing open paren before argument list for builtin call
            return MM_false;
        }
        else
        {
            MM_Argument* args = 0;
            if (!MM_IsToken(MM_Token_CloseParen))
            {
                if (!MM_Parser__ParseArgumentList(parser, &args)) return MM_false;
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                //// ERROR: Missing closing paren after argument list to builtin call
                return MM_false;
            }
            else
            {
                *expression = MM_PushNode(MM_AST_BuiltinCall);
                (*expression)->builtin_call_expr.builtin_kind = builtin_kind;
                (*expression)->builtin_call_expr.args         = args;
            }
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
                    
                    expression = &(*expression)->array_type_expr.type;
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
                    (*expression)->member_expr.member = MM_GetToken().identifier;
                    MM_NextToken();
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
            MM_NextToken();
        }
    }
    
    return MM_Parser__ParsePostfix(parser, expression);
}

MM_bool
MM_Parser__ParseBinary(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser__ParsePrefix(parser, expression)) return MM_false;
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
                if (!MM_Parser__ParsePrefix(parser, &right)) return MM_false;
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
MM_Parser__ParseBlock(MM_Parser* parser, MM_Block_Statement** block)
{
    MM_ASSERT(MM_IsToken(MM_Token_OpenBrace));
    MM_NextToken();
    
    MM_Statement* body            = 0;
    MM_Statement** next_statement = &body;
    while (!MM_EatToken(MM_Token_CloseBrace))
    {
        if (MM_IsToken(MM_Token_EOF))
        {
            //// ERROR: Missing closing brace after block body
            return MM_false;
        }
        else
        {
            if (!MM_Parser__ParseStatement(parser, next_statement)) return MM_false;
            else                                                    next_statement = &(*next_statement)->next;
        }
    }
    
    *block = MM_PushNode(MM_AST_Block);
    (*block)->label = (MM_String){0}; // NOTE: Labels are assigned by the caller
    (*block)->body  = body;
    
    return MM_true;
}

MM_bool
MM_Parser__ParseDeclarationAssignmentOrExpression(MM_Parser* parser, MM_Statement** statement)
{
    if (MM_EatToken(MM_Token_When))
    {
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            //// ERROR: Missing open paren before when statement header
            return MM_false;
        }
        else
        {
            MM_Statement* first;
            if (!MM_Parser__ParseDeclarationAssignmentOrExpression(parser, &first)) return MM_false;
            else
            {
                MM_Expression* condition;
                if (first->kind >= MM_AST__FirstExpression && first->kind <= MM_AST__LastExpression) condition = (MM_Expression*)first;
                else
                {
                    //// ERROR: When condition must be an expression
                    return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    //// ERROR: Missing closing paren after when statement header
                    return MM_false;
                }
                else
                {
                    MM_Statement* true_body;
                    MM_Statement* false_body = 0;
                    
                    if      (!MM_Parser__ParseStatement(parser, &true_body))                                return MM_false;
                    else if (MM_EatToken(MM_Token_Else) && !MM_Parser__ParseStatement(parser, &false_body)) return MM_false;
                    else
                    {
                        *statement = MM_PushNode(MM_AST_When);
                        (*statement)->declaration.when_decl.condition  = condition;
                        (*statement)->declaration.when_decl.true_body  = true_body;
                        (*statement)->declaration.when_decl.false_body = false_body;
                    }
                }
            }
        }
    }
    else
    {
        MM_Expression* expressions;
        if (!MM_Parser__ParseExpressionList(parser, &expressions)) return MM_false;
        else
        {
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
                    MM_Expression* values;
                    if (!MM_Parser__ParseExpressionList(parser, &values)) return MM_false;
                    else
                    {
                        *statement = MM_PushNode(MM_AST_Const);
                        (*statement)->declaration.const_decl.names  = names;
                        (*statement)->declaration.const_decl.type   = type;
                        (*statement)->declaration.const_decl.values = values;
                    }
                }
                else
                {
                    MM_Expression* values    = 0;
                    MM_bool is_uninitialized = MM_false;
                    if (MM_EatToken(MM_Token_Equals))
                    {
                        if (MM_EatToken(MM_Token_TpMinus)) is_uninitialized = MM_true;
                        else
                        {
                            if (!MM_Parser__ParseExpressionList(parser, &values)) return MM_false;
                        }
                    }
                    
                    *statement = MM_PushNode(MM_AST_Var);
                    (*statement)->declaration.var_decl.names            = names;
                    (*statement)->declaration.var_decl.type             = type;
                    (*statement)->declaration.var_decl.values           = values;
                    (*statement)->declaration.var_decl.is_uninitialized = is_uninitialized;
                }
            }
            else if (MM_GetToken().kind >= MM_Token__FirstAssignment && MM_GetToken().kind <= MM_Token__LastAssignment)
            {
                MM_AST_Kind assign_op = (MM_IsToken(MM_Token_Equals) ? MM_AST_Invalid : MM_TOKEN_ASSIGNMENT_TO_BINARY(MM_GetToken().kind));
                MM_NextToken();
                
                MM_Expression* lhs = expressions;
                MM_Expression* rhs;
                if (!MM_Parser__ParseExpressionList(parser, &rhs)) return MM_false;
                else
                {
                    *statement = MM_PushNode(MM_AST_Assignment);
                    (*statement)->assignment_stmnt.assign_op = assign_op;
                    (*statement)->assignment_stmnt.lhs       = lhs;
                    (*statement)->assignment_stmnt.rhs       = rhs;
                }
            }
            else
            {
                if (expressions->next == 0) *statement = (MM_Statement*)expressions;
                else
                {
                    //// ERROR: Expression lists cannot be used in isolation
                    return MM_false;
                }
            }
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseStatement(MM_Parser* parser, MM_Statement** statement)
{
    if (MM_IsToken(MM_Token_Semicolon))
    {
        //// ERROR: Stray semicolon
        return MM_false;
    }
    else if (MM_EatToken(MM_Token_Else))
    {
        //// ERROR: Else without matching if/when
        return MM_false;
    }
    else if (MM_EatToken(MM_Token_Colon))
    {
        if (!MM_IsToken(MM_Token_Identifier))
        {
            //// ERROR: Missing name of label
            return MM_false;
        }
        else
        {
            MM_String label = MM_GetToken().identifier;
            MM_NextToken();
            
            MM_Token_Kind token_kind = MM_GetToken().kind;
            if (token_kind != MM_Token_Invalid && token_kind != MM_Token_EOF && // NOTE: Avoid overriding error messages more relevant to these
                (token_kind != MM_Token_OpenBrace && token_kind != MM_Token_If && token_kind != MM_Token_While))
            {
                //// ERROR: Labels can only be applies to if, while and block statements
                return MM_false;
            }
            else
            {
                if (!MM_Parser__ParseStatement(parser, statement)) return MM_false;
                else
                {
                    if      ((*statement)->kind == MM_AST_Block) (*statement)->block_stmnt.label = label;
                    else if ((*statement)->kind == MM_AST_If)    (*statement)->if_stmnt.label    = label;
                    else if ((*statement)->kind == MM_AST_While) (*statement)->while_stmnt.label = label;
                    else MM_ILLEGAL_CODE_PATH;
                }
            }
        }
    }
    else if (MM_IsToken(MM_Token_OpenBrace))
    {
        return MM_Parser__ParseBlock(parser, (MM_Block_Statement**)statement);
    }
    else if (MM_EatToken(MM_Token_If))
    {
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            //// ERROR: Missing open paren before if statement header
            return MM_false;
        }
        else
        {
            MM_Statement* first = 0;
            if (!MM_IsToken(MM_Token_Semicolon) && !MM_Parser__ParseDeclarationAssignmentOrExpression(parser, &first)) return MM_false;
            else
            {
                MM_Statement* second = 0;
                if (MM_EatToken(MM_Token_Semicolon))
                {
                    if (!MM_Parser__ParseDeclarationAssignmentOrExpression(parser, &second)) return MM_false;
                }
                else
                {
                    second = first;
                    first  = 0;
                }
                
                MM_Statement* init = first;
                MM_Expression* condition;
                if (second->kind >= MM_AST__FirstExpression && second->kind <= MM_AST__LastExpression) condition = (MM_Expression*)second;
                else
                {
                    //// ERROR: If condition must be an expression
                    return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    //// ERROR: Missing closing paren after if statement header
                    return MM_false;
                }
                else
                {
                    MM_Statement* true_body;
                    MM_Statement* false_body = 0;
                    
                    if      (!MM_Parser__ParseStatement(parser, &true_body))                                return MM_false;
                    else if (MM_EatToken(MM_Token_Else) && !MM_Parser__ParseStatement(parser, &false_body)) return MM_false;
                    else
                    {
                        *statement = MM_PushNode(MM_AST_If);
                        (*statement)->if_stmnt.label      = (MM_String){0};
                        (*statement)->if_stmnt.init       = init;
                        (*statement)->if_stmnt.condition  = condition;
                        (*statement)->if_stmnt.true_body  = true_body;
                        (*statement)->if_stmnt.false_body = false_body;
                    }
                }
            }
        }
    }
    else if (MM_EatToken(MM_Token_While))
    {
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            //// ERROR: Missing open paren before while statement header
            return MM_false;
        }
        else
        {
            MM_Statement* init = 0;
            if (!MM_IsToken(MM_Token_Semicolon) && !MM_Parser__ParseDeclarationAssignmentOrExpression(parser, &init)) return MM_false;
            else
            {
                MM_Statement* condition_slot = 0;
                MM_Statement* step           = 0;
                if (MM_EatToken(MM_Token_Semicolon))
                {
                    if (!MM_IsToken(MM_Token_Semicolon) && !MM_Parser__ParseDeclarationAssignmentOrExpression(parser, &condition_slot)) return MM_false;
                    
                    if (MM_EatToken(MM_Token_Semicolon))
                    {
                        if (!MM_Parser__ParseDeclarationAssignmentOrExpression(parser, &step)) return MM_false;
                    }
                }
                else
                {
                    condition_slot = init;
                    init           = 0;
                }
                
                MM_Expression* condition;
                if (condition_slot->kind >= MM_AST__FirstExpression && condition_slot->kind <= MM_AST__LastExpression) condition = (MM_Expression*)condition_slot;
                else
                {
                    //// ERROR: While condition must be an expression
                    return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    //// ERROR: Missing closing paren after while statement header
                    return MM_false;
                }
                else
                {
                    MM_Statement* body;
                    if (!MM_Parser__ParseStatement(parser, &body)) return MM_false;
                    else
                    {
                        *statement = MM_PushNode(MM_AST_While);
                        (*statement)->while_stmnt.label      = (MM_String){0};
                        (*statement)->while_stmnt.init       = init;
                        (*statement)->while_stmnt.condition  = condition;
                        (*statement)->while_stmnt.step       = step;
                        (*statement)->while_stmnt.body       = body;
                    }
                }
            }
        }
    }
    else if (MM_IsToken(MM_Token_Break) || MM_IsToken(MM_Token_Continue))
    {
        MM_bool is_break = MM_IsToken(MM_Token_Break);
        MM_NextToken();
        
        MM_String label = {0};
        if (MM_IsToken(MM_Token_Identifier))
        {
            label = MM_GetToken().identifier;
            MM_NextToken();
        }
        
        if (!MM_EatToken(MM_Token_Semicolon))
        {
            //// ERROR: Missing terminating semicolon after break/continue statement
            return MM_false;
        }
        else
        {
            *statement = MM_PushNode(is_break ? MM_AST_Break : MM_AST_Continue);
            (*statement)->jump_stmnt.label = label;
        }
    }
    else if (MM_EatToken(MM_Token_Return))
    {
        MM_Argument* args = 0;
        if (!MM_IsToken(MM_Token_Semicolon))
        {
            if (!MM_Parser__ParseArgumentList(parser, &args)) return MM_false;
        }
        
        if (!MM_EatToken(MM_Token_Semicolon))
        {
            //// ERROR: Missing terminating semicolon after return statement
            return MM_false;
        }
        else
        {
            *statement = MM_PushNode(MM_AST_Return);
            (*statement)->return_stmnt.args = args;
        }
    }
    else
    {
        if (!MM_Parser__ParseDeclarationAssignmentOrExpression(parser, statement)) return MM_false;
        else
        {
            MM_bool should_have_semicolon = MM_true;
            if ((*statement)->kind == MM_AST_Const)
            {
                MM_Expression* values = (*statement)->declaration.const_decl.values;
                if (values->next == 0 && (values->kind == MM_AST_ProcLit || values->kind == MM_AST_StructType))
                {
                    should_have_semicolon = MM_false;
                }
            }
            
            if (should_have_semicolon && !MM_EatToken(MM_Token_Semicolon))
            {
                //// ERROR: Missing terminating semicolon after statement
                return MM_false;
            }
            else if (!should_have_semicolon && MM_IsToken(MM_Token_Semicolon))
            {
                //// ERROR: Stray semicolon. Single procedure literal or struct type bound to constant declarations should not be followed by a semicolon
                return MM_false;
            }
        }
    }
    
    return MM_true;
}



MM_bool
MM_Parser__ParseTopLevelDeclarations(MM_Parser* parser, MM_Declaration** declarations)
{
    MM_Declaration** next_decl = declarations;
    while (!MM_IsToken(MM_Token_EOF))
    {
        MM_Statement* statement;
        if (!MM_Parser__ParseStatement(parser, &statement)) return MM_false;
        else
        {
            if (!(statement->kind >= MM_AST__FirstDeclaration && statement->kind <= MM_AST__LastDeclaration))
            {
                //// ERROR: Only declarations are allowed at the top level
                return MM_false;
            }
            else
            {
                *next_decl = (MM_Declaration*)statement;
                next_decl  = &(*next_decl)->next;
            }
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser_ParseTopLevelDeclarationsFromString(MM_String string, MM_Text_Pos pos, MM_Arena* ast_arena, MM_Arena* str_arena, MM_Declaration** declarations)
{
    MM_Parser parser = {
        .lexer     = MM_Lexer_Init(string, pos, str_arena),
        .ast_arena = ast_arena,
        .str_arena = str_arena,
    };
    
    return MM_Parser__ParseTopLevelDeclarations(&parser, declarations);
}

#undef MM_NextToken
#undef MM_GetToken
#undef MM_IsToken
#undef MM_EatToken
#undef MM_PushNode