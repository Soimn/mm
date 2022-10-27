typedef struct MM_Parser
{
    MM_Lexer lexer;
    MM_Arena* ast_arena;
    MM_Arena* string_arena;
} MM_Parser;

void*
MM_Parser__PushNode(MM_Parser* parser, MM_AST_Kind kind)
{
    MM_AST* node = MM_Arena_Push(parser->ast_arena, sizeof(MM_AST), MM_ALIGNOF(MM_AST));
    node->kind = kind;
    node->next = 0;
    
    return (void*)node;
}

#define MM_GetToken() (MM_Lexer_GetToken(&parser->lexer))
#define MM_IsToken(k) (MM_Lexer_GetToken(&parser->lexer).kind == (k))
#define MM_NextToken() MM_Lexer_NextToken(&parser->lexer)
#define MM_EatToken(k) (MM_Lexer_GetToken(&parser->lexer).kind == (k) && (MM_Lexer_NextToken(&parser->lexer), MM_true))
#define MM_PushNode(k) MM_Parser__PushNode(parser, (k))

MM_bool MM_Parser__ParseExpression(MM_Parser* parser, MM_Expression** expression);
MM_bool MM_Parser__ParseBlock(MM_Parser* parser, MM_Statement_Block** block);
MM_bool MM_Parser__ParseStatement(MM_Parser* parser, MM_Statement** statement);

MM_bool
MM_Parser__ParseStringLiteral(MM_Parser* parser, MM_Token token, MM_String_Literal* string_literal)
{
    MM_ASSERT(token.kind == MM_Token_String);
    
    MM_String string = token.string;
    
    *string_literal = (MM_String){
        .data = MM_Arena_Push(parser->string_arena, string.size, MM_ALIGNOF(MM_u8)),
        .size = 0,
    };
    
    for (MM_umm i = 0; i < string.size; ++i)
    {
        if (string.data[i] != '\\') string_literal->data[string_literal->size++] = string.data[i];
        else
        {
            i += 1;
            if      (i == string.size) MM_ILLEGAL_CODE_PATH; // NOTE: Impossible, since " would be escaped
            else if (string.data[i] == 'a')  string_literal->data[string_literal->size++] = '\a';
            else if (string.data[i] == 'b')  string_literal->data[string_literal->size++] = '\b';
            else if (string.data[i] == 'f')  string_literal->data[string_literal->size++] = '\f';
            else if (string.data[i] == 'n')  string_literal->data[string_literal->size++] = '\n';
            else if (string.data[i] == 'r')  string_literal->data[string_literal->size++] = '\r';
            else if (string.data[i] == 't')  string_literal->data[string_literal->size++] = '\t';
            else if (string.data[i] == 'v')  string_literal->data[string_literal->size++] = '\v';
            else if (string.data[i] == '\\') string_literal->data[string_literal->size++] = '\\';
            else if (string.data[i] == '\'') string_literal->data[string_literal->size++] = '\'';
            else if (string.data[i] == '"')  string_literal->data[string_literal->size++] = '\"';
            else if (string.data[i] == 'x' || string.data[i] == 'u' || string.data[i] == 'U')
            {
                MM_umm codepoint = 0;
                
                MM_umm remaining_digits = (string.data[i] == 'x' ? 2 : (string.data[i] == 'u' ? 4 : 8));
                
                for (; remaining_digits > 0 && i < string.size; --remaining_digits, ++i)
                {
                    MM_u8 digit = 0;
                    
                    if      (string.data[i] >= '0' && string.data[i] <= '9') digit = string.data[i] & 0xF;
                    else if (string.data[i] >= 'A' && string.data[i] <= 'F') digit = (string.data[i] & 0x1F) + 9;
                    else if (string.data[i] >= 'a' && string.data[i] <= 'f') digit = (string.data[i] & 0x1F) + 9;
                    else
                    {
                        //// ERROR: Not a valid hexadecimal digit
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    
                    codepoint = (codepoint << 4) | digit;
                }
                
                MM_ASSERT(remaining_digits >= 0);
                if (remaining_digits > 0)
                {
                    //// ERROR: Missing digits
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                else
                {
                    if (codepoint <= 0x7F)
                    {
                        string_literal->data[string_literal->size++] = (MM_u8)codepoint;
                    }
                    else if (codepoint <= 0x7FF)
                    {
                        string_literal->data[string_literal->size++] = ((MM_u8)(codepoint >> 6) & 0x1F) | 0xC0;
                        string_literal->data[string_literal->size++] = ((MM_u8)(codepoint >> 0) & 0x3F) | 0x80;
                    }
                    else if (codepoint <= 0xFFFF)
                    {
                        string_literal->data[string_literal->size++] = ((MM_u8)(codepoint >> 12) & 0x0F) | 0xE0;
                        string_literal->data[string_literal->size++] = ((MM_u8)(codepoint >> 6)  & 0x3F) | 0x80;
                        string_literal->data[string_literal->size++] = ((MM_u8)(codepoint >> 0)  & 0x3F) | 0x80;
                    }
                    else if (codepoint <= 0x10FFFF)
                    {
                        string_literal->data[string_literal->size++] = ((MM_u8)(codepoint >> 18) & 0x07) | 0xF0;
                        string_literal->data[string_literal->size++] = ((MM_u8)(codepoint >> 12) & 0x3F) | 0x80;
                        string_literal->data[string_literal->size++] = ((MM_u8)(codepoint >> 6)  & 0x3F) | 0x80;
                        string_literal->data[string_literal->size++] = ((MM_u8)(codepoint >> 0)  & 0x3F) | 0x80;
                    }
                    else
                    {
                        //// ERROR: Codepoint is out of UTF-8 range
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                }
            }
        }
    }
    
    MM_Arena_Pop(parser->string_arena, string.size - string_literal->size);
    
    return MM_true;
}

MM_bool
MM_Parser__ParseArguments(MM_Parser* parser, MM_Argument** args)
{
    MM_Argument** next_arg = args;
    for (;;)
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
            (*next_arg)->value = value;
            
            next_arg = &(*next_arg)->next;
            
            if (MM_EatToken(MM_Token_Comma)) continue;
            else                             break;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseExpressionList(MM_Parser* parser, MM_Expression** expressions)
{
    MM_Expression** next_expr = expressions;
    for (;;)
    {
        if (!MM_Parser__ParseExpression(parser, next_expr)) return MM_false;
        else
        {
            next_expr = &(*next_expr)->next;
            
            if (MM_EatToken(MM_Token_Comma)) continue;
            else                             break;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParsePrimaryExpression(MM_Parser* parser, MM_Expression** expression)
{
    if (MM_EatToken(MM_Token_Blank))
    {
        *expression = MM_PushNode(MM_AST_BlankIdentifier);
    }
    else if (MM_IsToken(MM_Token_Identifier))
    {
        *expression = MM_PushNode(MM_AST_Identifier);
        (*expression)->identifier_expr.ident = MM_GetToken().identifier;
        MM_NextToken();
    }
    else if (MM_IsToken(MM_Token_String))
    {
        MM_String_Literal string_lit;
        if (!MM_Parser__ParseStringLiteral(parser, MM_GetToken(), &string_lit)) return MM_false;
        else
        {
            MM_NextToken();
            
            *expression = MM_PushNode(MM_AST_String);
            (*expression)->string_expr.string = string_lit;
        }
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
            if (!MM_IsToken(MM_Token_CloseParen))
            {
                MM_Expression* expr_list;
                if (!MM_Parser__ParseExpressionList(parser, &expr_list)) return MM_false;
                else
                {
                    if (MM_IsToken(MM_Token_Equals))
                    {
                        //// ERROR: some clever error message distinguishing a, b, c = from a, b, c missing )
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else if (!MM_IsToken(MM_Token_Colon))
                    {
                        MM_Parameter** next_param = &params;
                        for (MM_Expression* expr = expr_list; expr != 0; expr = expr->next)
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
                        MM_Parameter** next_param = &params;
                        MM_Expression* names      = expr_list;
                        for (;;)
                        {
                            if (!MM_EatToken(MM_Token_Colon))
                            {
                                //// ERROR: missing type of parameter[s]
                                MM_NOT_IMPLEMENTED;
                                return MM_false;
                            }
                            else
                            {
                                MM_Expression* type  = 0;
                                MM_Expression* value = 0;
                                if (!MM_IsToken(MM_Token_Equals) && !MM_Parser__ParseExpression(parser, &type)) return MM_false;
                                else
                                {
                                    if (MM_EatToken(MM_Token_Equals))
                                    {
                                        if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
                                    }
                                    
                                    *next_param = MM_PushNode(MM_AST_Parameter);
                                    (*next_param)->names = names;
                                    (*next_param)->type  = type;
                                    (*next_param)->value = value;
                                    
                                    next_param = &(*next_param)->next;
                                    
                                    if (!MM_EatToken(MM_Token_Comma)) break;
                                    else
                                    {
                                        if (!MM_Parser__ParseExpressionList(parser, &names)) return MM_false;
                                        continue;
                                    }
                                }
                            }
                        }
                    }
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
            if (!MM_EatToken(MM_Token_OpenParen))
            {
                MM_Expression* type;
                if (MM_Parser__ParseExpression(parser, &type)) return MM_false;
                else
                {
                    ret_vals = MM_PushNode(MM_AST_ReturnValue);
                    ret_vals->names = 0;
                    ret_vals->type  = type;
                    ret_vals->value = 0;
                }
            }
            else
            {
                MM_Expression* expr_list;
                if (!MM_Parser__ParseExpressionList(parser, &expr_list)) return MM_false;
                else
                {
                    if (MM_IsToken(MM_Token_Equals))
                    {
                        //// ERROR: some clever error message distinguishing a, b, c = from a, b, c missing )
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else if (!MM_IsToken(MM_Token_Colon))
                    {
                        MM_Return_Value** next_ret_val = &ret_vals;
                        for (MM_Expression* expr = expr_list; expr != 0; expr = expr->next)
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
                        MM_Return_Value** next_ret_val = &ret_vals;
                        MM_Expression* names           = expr_list;
                        for (;;)
                        {
                            if (!MM_EatToken(MM_Token_Colon))
                            {
                                //// ERROR: missing type of return value[s]
                                MM_NOT_IMPLEMENTED;
                                return MM_false;
                            }
                            else
                            {
                                MM_Expression* type  = 0;
                                MM_Expression* value = 0;
                                if (!MM_IsToken(MM_Token_Equals) && !MM_Parser__ParseExpression(parser, &type)) return MM_false;
                                else
                                {
                                    if (MM_EatToken(MM_Token_Equals))
                                    {
                                        if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
                                    }
                                    
                                    *next_ret_val = MM_PushNode(MM_AST_ReturnValue);
                                    (*next_ret_val)->names = names;
                                    (*next_ret_val)->type  = type;
                                    (*next_ret_val)->value = value;
                                    
                                    next_ret_val = &(*next_ret_val)->next;
                                    
                                    if (!MM_EatToken(MM_Token_Comma)) break;
                                    else
                                    {
                                        if (!MM_Parser__ParseExpressionList(parser, &names)) return MM_false;
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    //// ERROR: Missing closing parenthesis afer return value list
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
        }
        
        if (MM_IsToken(MM_Token_TpMinus))
        {
            *expression = MM_PushNode(MM_AST_ProcLit);
            (*expression)->proc_lit_expr.params   = params;
            (*expression)->proc_lit_expr.ret_vals = ret_vals;
            (*expression)->proc_lit_expr.body     = 0;
        }
        else if (!MM_IsToken(MM_Token_OpenBrace))
        {
            *expression = MM_PushNode(MM_AST_ProcType);
            (*expression)->proc_type_expr.params   = params;
            (*expression)->proc_type_expr.ret_vals = ret_vals;
        }
        else
        {
            MM_Statement_Block* body;
            if (!MM_Parser__ParseBlock(parser, &body)) return MM_false;
            else
            {
                *expression = MM_PushNode(MM_AST_ProcLit);
                (*expression)->proc_lit_expr.params   = params;
                (*expression)->proc_lit_expr.ret_vals = ret_vals;
                (*expression)->proc_lit_expr.body     = body;
            }
        }
    }
    else if (MM_EatToken(MM_Token_Struct))
    {
        if (MM_EatToken(MM_Token_TpMinus))
        {
            MM_NOT_IMPLEMENTED;
        }
        else if (!MM_EatToken(MM_Token_OpenBrace))
        {
            //// ERROR: Missing body of struct
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Struct_Member* members = 0;
            
            if (!MM_IsToken(MM_Token_CloseBrace))
            {
                MM_Struct_Member** next_member = &members;
                for (;;)
                {
                    MM_Expression* names;
                    MM_Expression* type        = 0;
                    MM_Expression* const_value = 0;
                    
                    if (!MM_Parser__ParseExpressionList(parser, &names)) return MM_false;
                    else if (!MM_EatToken(MM_Token_Colon))
                    {
                        //// ERROR: Missing type of struct member
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else
                    {
                        if (!(MM_IsToken(MM_Token_Equals) || MM_IsToken(MM_Token_Colon)))
                        {
                            if (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
                        }
                        
                        if (MM_IsToken(MM_Token_Equals))
                        {
                            //// ERROR: Struct members cannot have a default value
                            MM_NOT_IMPLEMENTED;
                            return MM_false;
                        }
                        else if (MM_EatToken(MM_Token_Colon))
                        {
                            if (!MM_Parser__ParseExpression(parser, &const_value)) return MM_false;
                        }
                        
                        *next_member = MM_PushNode(MM_AST_StructMember);
                        (*next_member)->names       = names;
                        (*next_member)->type        = type;
                        (*next_member)->const_value = const_value;
                        
                        next_member = &(*next_member)->next;
                        
                        if (MM_EatToken(MM_Token_Comma)) continue;
                        else                             break;
                    }
                }
            }
            
            if (!MM_EatToken(MM_Token_CloseBrace))
            {
                //// ERROR: Missing closing brace after struct members
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                *expression = MM_PushNode(MM_AST_Struct);
                (*expression)->struct_expr.members = members;
            }
        }
    }
    else if (MM_EatToken(MM_Token_Enum))
    {
        MM_Expression* member_type = 0;
        if (!MM_IsToken(MM_Token_OpenBrace))
        {
            if (!MM_Parser__ParseExpression(parser, &member_type)) return MM_false;
        }
        
        if (!MM_EatToken(MM_Token_OpenBrace))
        {
            //// ERROR: Missing enum body
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Enum_Member* members = 0;
            if (!MM_IsToken(MM_Token_CloseBrace))
            {
                MM_Enum_Member** next_member = &members;
                for (;;)
                {
                    if (MM_IsToken(MM_Token_Blank))
                    {
                        //// ERROR: Name of enum member cannot be blank
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else if (!MM_IsToken(MM_Token_Identifier))
                    {
                        //// ERROR: Missing name of enum member
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else
                    {
                        MM_Identifier name = MM_GetToken().identifier;
                        MM_NextToken();
                        
                        MM_Expression* value = 0;
                        if (MM_EatToken(MM_Token_Equals))
                        {
                            if (!MM_Parser__ParseExpression(parser, &value)) return MM_false;
                        }
                        
                        *next_member = MM_PushNode(MM_AST_EnumMember);
                        (*next_member)->name  = name;
                        (*next_member)->value = value;
                        
                        next_member = &(*next_member)->next;
                        
                        if (MM_EatToken(MM_Token_Comma)) continue;
                        else                             break;
                    }
                }
            }
            
            if (!MM_EatToken(MM_Token_CloseBrace))
            {
                //// ERROR: Missing closing brace after enum members
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else
            {
                *expression = MM_PushNode(MM_AST_Enum);
                (*expression)->enum_expr.member_type = member_type;
                (*expression)->enum_expr.members     = members;
            }
        }
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
    for (;;)
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
        for (;;)
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
                if (MM_IsToken(MM_Token_Blank))
                {
                    //// ERROR: Member name cannot be blank
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
                else if (!MM_IsToken(MM_Token_Identifier))
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
    for (;;)
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
        for (;;)
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

MM_bool
MM_Parser__ParseBlock(MM_Parser* parser, MM_Statement_Block** block)
{
    MM_bool starts_with_brace = MM_EatToken(MM_Token_OpenBrace);
    MM_ASSERT(starts_with_brace);
    
    MM_Statement* body = 0;
    MM_Statement** next_stmnt = &body;
    while (!MM_IsToken(MM_Token_CloseBrace))
    {
        if (MM_IsToken(MM_Token_EOF))
        {
            //// ERROR: End of file reached before end of block
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            if (!MM_Parser__ParseStatement(parser, next_stmnt)) return MM_false;
            else                                                next_stmnt = &(*next_stmnt)->next;
        }
    }
    
    if (MM_EatToken(MM_Token_CloseBrace));
    else MM_ILLEGAL_CODE_PATH;
    
    *block = MM_PushNode(MM_AST_Block);
    (*block)->label = (MM_Identifier){0};
    (*block)->body  = body;
    
    return MM_true;
}

MM_bool
MM_Parser__ParseDeclarationExpressionOrAssignment(MM_Parser* parser, MM_Statement** statement)
{
    MM_Expression* expr_list;
    if (!MM_Parser__ParseExpressionList(parser, &expr_list)) return MM_false;
    else
    {
        if (MM_EatToken(MM_Token_Colon))
        {
            MM_Expression* names = expr_list;
            MM_Expression* type  = 0;
            if (!MM_IsToken(MM_Token_Equals) && !MM_IsToken(MM_Token_Colon))
            {
                if      (!MM_Parser__ParseExpression(parser, &type)) return MM_false;
                else if (MM_IsToken(MM_Token_Comma))
                {
                    //// ERROR: Only one type can be explicitly specified in a variable/constant decl
                    MM_NOT_IMPLEMENTED;
                    return MM_false;
                }
            }
            
            if (MM_EatToken(MM_Token_Colon))
            {
                MM_Expression* values;
                
                if (MM_IsToken(MM_Token_TpMinus))
                {
                    MM_NOT_IMPLEMENTED;
                }
                else if (!MM_Parser__ParseExpressionList(parser, &values)) return MM_false;
                else
                {
                    *statement = MM_PushNode(MM_AST_Const);
                    ((MM_Declaration_Const*)*statement)->names  = names;
                    ((MM_Declaration_Const*)*statement)->type   = type;
                    ((MM_Declaration_Const*)*statement)->values = values;
                }
            }
            else
            {
                MM_Expression* values    = 0;
                MM_bool is_uninitialized = MM_false;
                if (MM_EatToken(MM_Token_Equals))
                {
                    if      (MM_EatToken(MM_Token_TpMinus))                    is_uninitialized = MM_true;
                    else if (!MM_Parser__ParseExpressionList(parser, &values)) return MM_false;
                }
                
                *statement = MM_PushNode(MM_AST_Var);
                ((MM_Declaration_Var*)*statement)->names            = names;
                ((MM_Declaration_Var*)*statement)->type             = type;
                ((MM_Declaration_Var*)*statement)->values           = values;
                ((MM_Declaration_Var*)*statement)->is_uninitialized = is_uninitialized;
            }
        }
        else if (MM_GetToken().kind >= MM_Token__FirstAssignment && MM_GetToken().kind <= MM_Token__LastAssignment)
        {
            MM_umm index     = MM_GetToken().kind - MM_Token__FirstAssignment;
            MM_AST_Kind kind = (MM_AST_Kind)(index + MM_AST__FirstAssignmentStatement);
            
            MM_NextToken();
            
            MM_Expression* lhs = expr_list;
            MM_Expression* rhs;
            if (!MM_Parser__ParseExpressionList(parser, &rhs)) return MM_false;
            else
            {
                *statement = MM_PushNode(kind);
                (*statement)->assignment_stmnt.lhs = lhs;
                (*statement)->assignment_stmnt.rhs = rhs;
            }
        }
        else
        {
            if (expr_list->next != 0)
            {
                //// ERROR: a list of expressions cannot be used by itself
                MM_NOT_IMPLEMENTED;
                return MM_false;
            }
            else *statement = (MM_Statement*)expr_list;
        }
    }
    
    return MM_true;
}

MM_bool
MM_Parser__ParseStatement(MM_Parser* parser, MM_Statement** statement)
{
    MM_bool check_semicolon = MM_true;
    
    if (MM_IsToken(MM_Token_Semicolon))
    {
        //// ERROR: Stray semicolon
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    else if (MM_IsToken(MM_Token_Else))
    {
        //// ERROR: Illegal else without matching if/when
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    else if (MM_EatToken(MM_Token_Colon))
    {
        if (MM_IsToken(MM_Token_Blank))
        {
            //// ERROR: Name of label cannot be blank
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else if (!MM_IsToken(MM_Token_Identifier))
        {
            //// ERROR: Missing name of label
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Identifier label = MM_GetToken().identifier;
            MM_NextToken();
            
            if (!(MM_IsToken(MM_Token_OpenBrace) || MM_IsToken(MM_Token_If) || MM_IsToken(MM_Token_While)))
            {
                //// ERROR: Illegal use of label. Labels may only be applied on block, if and while statements
                MM_NOT_IMPLEMENTED;
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
                    
                    check_semicolon = MM_false;
                }
            }
        }
    }
    else if (MM_IsToken(MM_Token_OpenBrace))
    {
        if (!MM_Parser__ParseBlock(parser, (MM_Statement_Block**)statement)) return MM_false;
    }
    else if (MM_EatToken(MM_Token_If))
    {
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            //// ERROR: Missing open parenthesis after if keyword
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Expression* condition;
            if (!MM_Parser__ParseExpression(parser, &condition)) return MM_false;
            else
            {
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    //// ERROR: Missing closing parenthesis after if condition
                    MM_NOT_IMPLEMENTED;
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
                        (*statement)->if_stmnt.label      = (MM_Identifier){0};
                        (*statement)->if_stmnt.condition  = condition;
                        (*statement)->if_stmnt.true_body  = true_body;
                        (*statement)->if_stmnt.false_body = false_body;
                        
                        check_semicolon = MM_false;
                    }
                }
            }
        }
    }
    else if (MM_EatToken(MM_Token_When))
    {
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            //// ERROR: Missing open parenthesis after when keyword
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Expression* condition;
            if (!MM_Parser__ParseExpression(parser, &condition)) return MM_false;
            else
            {
                if (!MM_EatToken(MM_Token_CloseParen))
                {
                    //// ERROR: Missing closing parenthesis after when condition
                    MM_NOT_IMPLEMENTED;
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
                        (*statement)->when_stmnt.condition  = condition;
                        (*statement)->when_stmnt.true_body  = true_body;
                        (*statement)->when_stmnt.false_body = false_body;
                        
                        check_semicolon = MM_false;
                    }
                }
            }
        }
    }
    else if (MM_EatToken(MM_Token_While))
    {
        if (!MM_EatToken(MM_Token_OpenParen))
        {
            //// ERROR: Missing open parenthesis after while keyword
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        else
        {
            MM_Statement* first = 0;
            if (!MM_IsToken(MM_Token_Semicolon) && !MM_Parser__ParseDeclarationExpressionOrAssignment(parser, &first)) return MM_false;
            else
            {
                MM_Statement* init       = 0;
                MM_Expression* condition = 0;
                MM_Statement* step       = 0;
                if (!MM_EatToken(MM_Token_Semicolon))
                {
                    if (!(first->kind >= MM_AST__FirstExpression && first->kind <= MM_AST__LastExpression))
                    {
                        //// ERROR: While condition must be an expression
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else condition = (MM_Expression*)first;
                }
                else
                {
                    init = first;
                    if (!MM_IsToken(MM_Token_Semicolon) && !MM_Parser__ParseExpression(parser, &condition)) return MM_false;
                    else if (MM_IsToken(MM_Token_Comma) || MM_IsToken(MM_Token_Colon))
                    {
                        //// ERROR: While condition must be an expression
                        MM_NOT_IMPLEMENTED;
                        return MM_false;
                    }
                    else if (MM_EatToken(MM_Token_Semicolon))
                    {
                        if (!MM_Parser__ParseDeclarationExpressionOrAssignment(parser, &step)) return MM_false;
                    }
                }
                
                MM_Statement* body;
                if (!MM_Parser__ParseStatement(parser, &body)) return MM_false;
                else
                {
                    *statement = MM_PushNode(MM_AST_While);
                    (*statement)->while_stmnt.label     = (MM_Identifier){0};
                    (*statement)->while_stmnt.init      = init;
                    (*statement)->while_stmnt.condition = condition;
                    (*statement)->while_stmnt.step      = step;
                    (*statement)->while_stmnt.body      = body;
                }
            }
        }
    }
    else if (MM_IsToken(MM_Token_Break) || MM_IsToken(MM_Token_Continue))
    {
        MM_bool is_break = MM_IsToken(MM_Token_Break);
        MM_NextToken();
        
        MM_Identifier label = {0};
        if (MM_IsToken(MM_Token_Identifier))
        {
            label = MM_GetToken().identifier;
            MM_NextToken();
        }
        else if (MM_IsToken(MM_Token_Blank))
        {
            //// ERROR: Label name cannot be blank
            MM_NOT_IMPLEMENTED;
            return MM_false;
        }
        
        *statement = MM_PushNode(is_break ? MM_AST_Break : MM_AST_Continue);
        (*statement)->jump_stmnt.label = label;
    }
    else if (MM_EatToken(MM_Token_Return))
    {
        MM_Argument* args = 0;
        if (!MM_IsToken(MM_Token_Semicolon) && !MM_Parser__ParseArguments(parser, &args)) return MM_false;
        else
        {
            *statement = MM_PushNode(MM_AST_Return);
            (*statement)->return_stmnt.args = args;
        }
    }
    else if (!MM_Parser__ParseDeclarationExpressionOrAssignment(parser, statement)) return MM_false;
    
    if (check_semicolon && !MM_EatToken(MM_Token_Semicolon))
    {
        //// ERROR: Missing terminating semicolon after statement
        MM_NOT_IMPLEMENTED;
        return MM_false;
    }
    
    return MM_true;
}

MM_bool
MM_Parser_ParseString(MM_String string, MM_Text_Pos pos, MM_Arena* ast_arena, MM_Arena* string_arena, MM_AST** ast)
{
    MM_Parser* parser = &(MM_Parser){
        .lexer        = MM_Lexer_Init(string, pos),
        .ast_arena    = ast_arena,
        .string_arena = string_arena,
    };
    
    MM_Statement** next_statement = (MM_Statement**)ast;
    while (!MM_IsToken(MM_Token_EOF))
    {
        if (!MM_Parser__ParseStatement(parser, next_statement)) return MM_false;
        else                                                    next_statement = &(*next_statement)->next;
    }
    
    return MM_true;
}

#undef MM_GetToken
#undef MM_IsToken
#undef MM_NextToken
#undef MM_EatToken