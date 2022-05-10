typedef struct Parser
{
    Workspace* workspace;
    Lexer lexer;
    Token token;
} Parser;

internal Token
GetToken(Parser* parser)
{
    return parser->token;
}

internal Token
NextToken(Parser* parser)
{
    parser->token = Lexer_Advance(parser->workspace, &parser->lexer);
    return parser->token;
}

internal bool
EatToken(Parser* parser, TOKEN_KIND kind)
{
    return (parser->token.kind == kind ? NextToken(parser), true : false);
}

internal AST_Node*
PushNode(Parser* parser, AST_NODE_KIND kind)
{
    NOT_IMPLEMENTED;
    return 0;
}

internal bool ParseExpression(Parser* parser, AST_Node** expression);

internal bool
ParseNamedValueList(Parser* parser, AST_Node** list)
{
    bool encountered_errors = false;
    
    AST_Node** next = list;
    while (!encountered_errors)
    {
        AST_Node* entry = PushNode(parser, AST_NamedValue);
        *next = entry;
        if (!ParseExpression(parser, &entry->named_value.value)) encountered_errors = true;
        else
        {
            if (EatToken(parser, Token_Equals))
            {
                entry->named_value.name = entry->named_value.value;
                if (!ParseExpression(parser, &entry->named_value.value)) encountered_errors = true;
            }
            
            if (!encountered_errors)
            {
                if (EatToken(parser, Token_Comma)) continue;
                else                               break;
            }
        }
    }
    
    return !encountered_errors;
}

internal bool
ParsePrimaryExpression(Parser* parser, AST_Node** expression)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

internal bool
ParsePostfixExpression(Parser* parser, AST_Node** expression)
{
    bool encountered_errors = false;
    
    if (!ParsePrimaryExpression(parser, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            if (EatToken(parser, Token_Hat))
            {
                AST_Node* operand = *expression;
                
                *expression = PushNode(parser, AST_Dereference);
                (*expression)->unary_expr = operand;
            }
            else if (EatToken(parser, Token_Period))
            {
                AST_Node* expr = *expression;
                
                Token token = GetToken(parser);
                
                if (token.kind != Token_Identifier)
                {
                    //// ERROR: Missing name of member to access
                    encountered_errors = true;
                }
                else if (!InternedString_IsNonBlankIdentifier(token.string))
                {
                    //// ERROR: Member name must be a non blank identifier
                    encountered_errors = true;
                }
                else
                {
                    NextToken(parser);
                    
                    *expression = PushNode(parser, AST_Member);
                    (*expression)->member_expr.expr   = expr;
                    (*expression)->member_expr.member = token.string;
                }
            }
            else if (EatToken(parser, Token_OpenBracket))
            {
                AST_Node* array = *expression;
                AST_Node* first = 0;
                
                if (GetToken(parser).kind != Token_Colon && !ParseExpression(parser, &first)) encountered_errors = true;
                else
                {
                    if (!EatToken(parser, Token_Colon))
                    {
                        *expression = PushNode(parser, AST_Subscript);
                        (*expression)->subscript_expr.array = array;
                        (*expression)->subscript_expr.index = first;
                    }
                    else
                    {
                        AST_Node* start    = first;
                        AST_Node* past_end = 0;
                        
                        if (GetToken(parser).kind != Token_CloseBracket && !ParseExpression(parser, &past_end)) encountered_errors = true;
                        else
                        {
                            *expression = PushNode(parser, AST_Slice);
                            (*expression)->slice_expr.array    = array;
                            (*expression)->slice_expr.start    = start;
                            (*expression)->slice_expr.past_end = past_end;
                        }
                    }
                }
                
                if (!encountered_errors && !EatToken(parser, Token_CloseBracket))
                {
                    //// ERROR: Missing closing bracket after x
                    encountered_errors = true;
                }
            }
            else if (EatToken(parser, Token_OpenParen))
            {
                AST_Node* proc = *expression;
                AST_Node* args = 0;
                
                if (!EatToken(parser, Token_CloseParen))
                {
                    if (!ParseNamedValueList(parser, &args)) encountered_errors = true;
                    else if (!EatToken(parser, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren after arguments to call expr
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(parser, AST_Call);
                    (*expression)->call_expr.proc = proc;
                    (*expression)->call_expr.args = args;
                }
            }
            else if (EatToken(parser, Token_PeriodOpenBrace))
            {
                AST_Node* type = *expression;
                AST_Node* args = 0;
                
                if (!EatToken(parser, Token_CloseBrace))
                {
                    if (!ParseNamedValueList(parser, &args)) encountered_errors = true;
                    else if (!EatToken(parser, Token_CloseBrace))
                    {
                        //// ERROR: Missing closing brace after arguments to struct literal
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(parser, AST_StructLiteral);
                    (*expression)->struct_literal.type = type;
                    (*expression)->struct_literal.args = args;
                }
            }
            else if (EatToken(parser, Token_PeriodOpenBracket))
            {
                AST_Node* type = *expression;
                AST_Node* args = 0;
                
                if (!EatToken(parser, Token_CloseBracket))
                {
                    if (!ParseNamedValueList(parser, &args)) encountered_errors = true;
                    else if (!EatToken(parser, Token_CloseBracket))
                    {
                        //// ERROR: Missing closing bracket after arguments to array literal
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(parser, AST_ArrayLiteral);
                    (*expression)->array_literal.type = type;
                    (*expression)->array_literal.args = args;
                }
            }
            else break;
        }
    }
    
    return !encountered_errors;
}

internal bool
ParsePrefixExpression(Parser* parser, AST_Node** expression)
{
    bool encountered_errors = false;
    
    while (!encountered_errors)
    {
        if      (EatToken(parser, Token_Plus));
        else if (EatToken(parser, Token_Minus)) expression = &(*expression = PushNode(parser, AST_Neg))->unary_expr;
        else if (EatToken(parser, Token_Bang))  expression = &(*expression = PushNode(parser, AST_Not))->unary_expr;
        else if (EatToken(parser, Token_Not))   expression = &(*expression = PushNode(parser, AST_BitNot))->unary_expr;
        else if (EatToken(parser, Token_Hat))   expression = &(*expression = PushNode(parser, AST_Reference))->unary_expr;
        else if (EatToken(parser, Token_OpenBracket))
        {
            if (EatToken(parser, Token_CloseBracket)) expression = &(*expression = PushNode(parser, AST_SliceType))->unary_expr;
            else
            {
                AST_Node* size;
                if (ParseExpression(parser, &size)) encountered_errors = true;
                else if (!EatToken(parser, Token_CloseBracket))
                {
                    //// ERROR: Missing closing bracket after size in array type qualifier
                    encountered_errors = true;
                }
                else
                {
                    AST_Node* array_type = PushNode(parser, AST_ArrayType);
                    array_type->array_type.size = size;
                    expression                  = &array_type->array_type.elem_type;
                }
            }
        }
        else
        {
            encountered_errors = !ParsePostfixExpression(parser, expression);
            break;
        }
    }
    
    return !encountered_errors;
}

internal bool
ParseBinaryExpression(Parser* parser, AST_Node** expression)
{
    bool encountered_errors = false;
    
    if (!ParsePrefixExpression(parser, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            Token token = GetToken(parser);
            if (token.kind < Token_FirstBinary || token.kind > Token_LastBinary) break;
            else                                                                 NextToken(parser);
            
            AST_Node** left = expression;
            AST_Node* right;
            
            if (!ParsePrefixExpression(parser, &right)) encountered_errors = true;
            else
            {
                umm precedence = token.kind >> 4;
                
                while ((*left)->kind >> 4 > precedence) left = &(*left)->binary_expr.right;
                
                AST_Node* new_expression = PushNode(parser, token.kind);
                new_expression->binary_expr.left  = *left;
                new_expression->binary_expr.right = right;
                
                *left = new_expression;
            }
        }
    }
    
    return !encountered_errors;
}

internal bool
ParseExpression(Parser* parser, AST_Node** expression)
{
    bool encountered_errors = false;
    
    if (!ParseBinaryExpression(parser, expression)) encountered_errors = true;
    else if (EatToken(parser, Token_QuestionMark))
    {
        AST_Node* condition  = *expression;
        AST_Node* true_expr;
        AST_Node* false_expr;
        
        if (!ParseBinaryExpression(parser, &true_expr)) encountered_errors = true;
        else if (!EatToken(parser, Token_Colon))
        {
            //// ERROR: Missing false expr
            encountered_errors = true;
        }
        else
        {
            if (!ParseBinaryExpression(parser, &false_expr)) encountered_errors = true;
            else
            {
                *expression = PushNode(parser, AST_Conditional);
                (*expression)->conditional_expr.condition  = condition;
                (*expression)->conditional_expr.true_expr  = true_expr;
                (*expression)->conditional_expr.false_expr = false_expr;
            }
        }
    }
    
    return !encountered_errors;
}