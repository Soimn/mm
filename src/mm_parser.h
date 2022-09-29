typedef struct MM_Parser
{
    MM_String string;
    MM_Text_Pos pos;
    MM_Token token;
} MM_Parser;

MM_Token
MM_Parser__NextToken(MM_Parser* parser)
{
    parser->token = MM_Token_FirstFromString(parser->string, parser->pos.offset, parser->pos, 0, &parser->pos);
    return parser->token;
}

void*
MM_Parser__PushNode(MM_Parser* parser, MM_AST_Kind kind)
{
    MM_NOT_IMPLEMENTED;
    return 0;
}

#define MM_GetToken() (parser->token)
#define MM_IsToken(k) (parser->token.kind == (k))
#define MM_NextToken() MM_Parser__NextToken(parser)
#define MM_EatToken(k) (parser->token.kind == (k) && (MM_Parser__NextToken(parser), MM_true))
#define MM_PushNode(k) MM_Parser__PushNode(parser, (k))

MM_bool MM_Parser__ParseExpression(MM_Parser* parser, MM_Expression** expression);

MM_bool
MM_Parser__ParseArguments(MM_Parser* parser, MM_Argument** args)
{
    MM_Argument** next_arg = args;
    while (MM_true)
    {
        MM_Expression* first;
        if (!MM_Parser__ParseExpression(parser, &first)) return MM_false;
        else
        {
            MM_Expression* label = 0;
            MM_Expression* value = 0;
            if (!MM_EatToken(MM_Token_Colon)) value = first;
            else
            {
                label = first;
                if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
            }
            
            *next_arg = MM_PushNode(MM_AST_Argument);
            (*next_arg)->label = label;
            (*next_arg)->value = value;;
            
            next_arg = &(*next_arg)->next;
            
            if (MM_EatToken(MM_Token_Comma)) continue;
            else                             break;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParsePrimaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (MM_IsToken(MM_Token_Identifier) || MM_IsToken(MM_Token_Blank))
    {
        MM_NOT_IMPLEMENTED;
    }
    else if (MM_IsToken(MM_Token_String))
    {
        MM_NOT_IMPLEMENTED;
    }
    else if (MM_IsToken(MM_Token_Int))
    {
        *expression = MM_PushNode(MM_AST_Int);
        (*expression)->int_expr.soft_int = MM_GetToken().soft_int;
        MM_NextToken();
    }
    else if (MM_IsToken(MM_Token_Float))
    {
        *expression = MM_PushNode(MM_AST_Float);
        (*expression)->float_expr.f64 = MM_GetToken().f64;
        MM_NextToken();
    }
    else if (MM_IsToken(MM_Token_True) || MM_IsToken(MM_Token_False))
    {
        *expression = MM_PushNode(MM_AST_Bool);
        (*expression)->bool_expr.boolean = MM_IsToken(MM_Token_True);
        MM_NextToken();
    }
    else if (MM_EatToken(MM_Token_Proc))
    {
        MM_Parameter* params      = 0;
        MM_Return_Value* ret_vals = 0;
        
        if (MM_EatToken(MM_Token_OpenParen))
        {
            MM_Parameter** next_param = &params;
            if (!MM_IsToken(MM_Token_CloseParen))
            {
                while (MM_true)
                {
                    MM_NOT_IMPLEMENTED;
                    
                    if (MM_EatToken(MM_Token_Comma)) continue;
                    else                             break;
                }
            }
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                //// ERROR: Missing closing parenthesis afer parameter list
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
        }
        
        if (MM_EatToken(MM_Token_Arrow))
        {
            if (MM_EatToken(MM_Token_OpenParen))
            {
                MM_NOT_IMPLEMENTED;
            }
            else
            {
                MM_NOT_IMPLEMENTED;
            }
        }
        
        MM_NOT_IMPLEMENTED;
    }
    else if (MM_EatToken(MM_Token_Struct))
    {
        MM_NOT_IMPLEMENTED;
    }
    else if (MM_EatToken(MM_Token_Enum))
    {
        MM_NOT_IMPLEMENTED;
    }
    else if (MM_EatToken(MM_Token_OpenParen))
    {
        MM_Expression* compound;
        if (!MM_Parser__ParseExpression(parser, &compound)) return MM_false;
        else
        {
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                //// ERROR: Missing closing parenthesis after compound expression body
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                *expression = MM_PushNode(MM_AST_Compound);
                (*expression)->compound_expr.expr = compound;
            }
        }
    }
    else if (MM_GetToken().kind >= MM_Token__FirstBuiltinKeyword && MM_GetToken().kind <= MM_Token__LastBuiltinKeyword)
    {
        MM_Builtin_Proc_ID proc_id = (MM_GetToken().kind - MM_Token__FirstBuiltinKeyword) + MM_BuiltinProc_Cast;
        MM_NextToken();
        
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            //// ERROR: Missing open parenthesis after builtin proc keyword
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Argument* args = 0;
            if (!MM_IsToken(MM_Token_CloseParen) && !MM_Parser__ParseArguments(parser, &args)) return MM_false;
            
            if (!MM_EatToken(MM_Token_CloseParen))
            {
                //// ERROR: Missing closing parenthesis after arguments to builtin procedure call
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                *expression = MM_PushNode(MM_AST_BuiltinCall);
                (*expression)->builtin_call_expr.proc_id = proc_id;
                (*expression)->builtin_call_expr.args    = args;
            }
        }
    }
    else if (MM_EatToken(MM_Token_PeriodBrace))
    {
        MM_Argument* args = 0;
        if (!MM_IsToken(MM_Token_CloseBrace) && !MM_Parser__ParseArguments(parser, &args)) return MM_false;
        else
        {
            if (!MM_EatToken(MM_Token_CloseBrace))
            {
                //// ERROR: Missing closing brace after arguments to struct literal
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                *expression = MM_PushNode(MM_AST_StructLit);
                (*expression)->struct_lit_expr.type = 0;
                (*expression)->struct_lit_expr.args = args;
            }
        }
    }
    else if (MM_EatToken(MM_Token_PeriodBracket))
    {
        MM_Argument* args = 0;
        if (!MM_IsToken(MM_Token_CloseBracket) && !MM_Parser__ParseArguments(parser, &args)) return MM_false;
        else
        {
            if (!MM_EatToken(MM_Token_CloseBracket))
            {
                //// ERROR: Missing closing bracket after arguments to array literal
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                *expression = MM_PushNode(MM_AST_ArrayLit);
                (*expression)->array_lit_expr.type = 0;
                (*expression)->array_lit_expr.args = args;
            }
        }
    }
    else
    {
        if (MM_GetToken().kind == MM_Token_Invalid)
        {
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
MM_Parser__ParseTypePrefixExpression(MM_Parser* parser, MM_Expression** expression)
{
    while (MM_true)
    {
        if (MM_EatToken(MM_Token_Hat))
        {
            *expression = MM_PushNode(MM_AST_PointerTo);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_OpenBracket))
        {
            if (MM_EatToken(MM_Token_CloseBracket))
            {
                *expression = MM_PushNode(MM_AST_SliceOf);
                expression = &(*expression)->unary_expr.operand;
            }
            else
            {
                MM_Expression* size;
                if (!MM_Parser__ParseExpression(parser, &size)) return MM_false;
                else
                {
                    if (!MM_EatToken(MM_Token_CloseBracket))
                    {
                        //// ERROR: Missing closing bracket after size in array-of type prefix
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else
                    {
                        *expression = MM_PushNode(MM_AST_ArrayOf);
                        (*expression)->array_of_expr.size = size;
                        expression = &(*expression)->array_of_expr.type;
                    }
                }
            }
        }
        else break;
    }
    
    return MM_Parser__ParsePrimaryExpression(parser, expression);
}

MM_bool
MM_Parser__ParsePostfixExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser__ParseTypePrefixExpression(parser, expression)) return MM_false;
    else
    {
        while (MM_true)
        {
            MM_Expression* operand = *expression;
            
            if (MM_EatToken(MM_Token_Hat))
            {
                *expression = MM_PushNode(MM_AST_Deref);
                (*expression)->unary_expr.operand = operand;
            }
            else if (MM_EatToken(MM_Token_OpenBracket))
            {
                MM_Expression* first = 0;
                if (!MM_IsToken(MM_Token_Colon))
                {
                    if (!MM_Parser__ParseExpression(parser, &first)) return MM_false;
                }
                
                if (!MM_EatToken(MM_Token_Colon))
                {
                    if (!MM_EatToken(MM_Token_CloseBracket))
                    {
                        //// ERROR: Missing closing bracket after index in subscript expression
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else
                    {
                        *expression = MM_PushNode(MM_AST_Subscript);
                        (*expression)->subscript_expr.array = operand;
                        (*expression)->subscript_expr.index = first;
                    }
                }
                else
                {
                    MM_Expression* one_past_end = 0;
                    
                    if (!MM_IsToken(MM_Token_CloseBracket))
                    {
                        if (!MM_Parser__ParseExpression(parser, &one_past_end)) return MM_false;
                    }
                    
                    if (!MM_EatToken(MM_Token_CloseBracket))
                    {
                        *expression = MM_PushNode(MM_AST_Slice);
                        (*expression)->slice_expr.array        = operand;
                        (*expression)->slice_expr.start        = first;
                        (*expression)->slice_expr.one_past_end = one_past_end;
                    }
                }
            }
            else if (MM_EatToken(MM_Token_OpenParen))
            {
                MM_Argument* args = 0;
                if (!MM_IsToken(MM_Token_CloseParen) && !MM_Parser__ParseArguments(parser, &args)) return MM_false;
                else
                {
                    if (!MM_EatToken(MM_Token_CloseParen))
                    {
                        //// ERROR: Missing closing parenthesis after arguments to call
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else
                    {
                        *expression = MM_PushNode(MM_AST_Call);
                        (*expression)->call_expr.proc = operand;
                        (*expression)->call_expr.args = args;
                    }
                }
            }
            else if (MM_EatToken(MM_Token_Period))
            {
                if (!MM_IsToken(MM_Token_Identifier))
                {
                    //// ERROR: Missing name of member after '.'
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                else
                {
                    *expression = MM_PushNode(MM_AST_Member);
                    (*expression)->member_expr.symbol = operand;
                    //(*expression)->member_expr.member =
                    MM_NOT_IMPLEMENTED;
                }
            }
            else if (MM_EatToken(MM_Token_PeriodBrace))
            {
                MM_Argument* args = 0;
                if (!MM_IsToken(MM_Token_CloseBrace) && !MM_Parser__ParseArguments(parser, &args)) return MM_false;
                else
                {
                    if (!MM_EatToken(MM_Token_CloseBrace))
                    {
                        //// ERROR: Missing closing brace after arguments to struct literal
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else
                    {
                        *expression = MM_PushNode(MM_AST_StructLit);
                        (*expression)->struct_lit_expr.type = operand;
                        (*expression)->struct_lit_expr.args = args;
                    }
                }
            }
            else if (MM_EatToken(MM_Token_PeriodBracket))
            {
                MM_Argument* args = 0;
                if (!MM_IsToken(MM_Token_CloseBracket) && !MM_Parser__ParseArguments(parser, &args)) return MM_false;
                else
                {
                    if (!MM_EatToken(MM_Token_CloseBracket))
                    {
                        //// ERROR: Missing closing bracket after arguments to array literal
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else
                    {
                        *expression = MM_PushNode(MM_AST_ArrayLit);
                        (*expression)->array_lit_expr.type = operand;
                        (*expression)->array_lit_expr.args = args;
                    }
                }
            }
            else break;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParsePrefixExpression(MM_Parser* parser, MM_Expression** expression)
{
    while (MM_true)
    {
        if      (MM_EatToken(MM_Token_Plus));
        else if (MM_EatToken(MM_Token_Minus))
        {
            *expression = MM_PushNode(MM_AST_Neg);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Bang))
        {
            *expression = MM_PushNode(MM_AST_Not);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_Tilde))
        {
            *expression = MM_PushNode(MM_AST_BitNot);
            expression = &(*expression)->unary_expr.operand;
        }
        else if (MM_EatToken(MM_Token_And))
        {
            *expression = MM_PushNode(MM_AST_Ref);
            expression = &(*expression)->unary_expr.operand;
        }
        else break;
    }
    
    return MM_Parser__ParsePostfixExpression(parser, expression);
}

MM_bool
MM_Parser__ParseBinaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (!MM_Parser__ParsePrefixExpression(parser, expression)) return MM_false;
    else
    {
        while (MM_true)
        {
            MM_Token token = MM_GetToken();
            if (!(token.kind >= MM_Token__FirstBinary && token.kind <= MM_Token__LastBinary)) break;
            else
            {
                // NOTE: Simple sanity check, does not cover entire mapping
                MM_STATIC_ASSERT(MM_AST__FirstBinary == MM_Token__FirstBinary);
                MM_STATIC_ASSERT(MM_AST__LastBinary == MM_Token__LastBinary);
                
                MM_AST_Kind kind = (MM_AST_Kind)token.kind;
                
                MM_NextToken();
                
                MM_Expression* right;
                if (!MM_Parser__ParsePrefixExpression(parser, &right)) return MM_false;
                else
                {
                    MM_Expression** spot = expression;
                    
                    while ((*spot)->kind > kind) spot = &(*spot)->binary_expr.right;
                    
                    MM_Expression* left = *spot;
                    
                    *spot = MM_PushNode(kind);
                    (*spot)->binary_expr.left  = left;
                    (*spot)->binary_expr.right = right;
                }
            }
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseExpression(MM_Parser* parser, MM_Expression** expression)
{
    return MM_Parser__ParseBinaryExpression(parser, expression);
}

#undef MM_GetToken
#undef MM_IsToken
#undef MM_NextToken
#undef MM_EatToken