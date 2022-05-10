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
        NOT_IMPLEMENTED;
    }
    
    return !encountered_errors;
}

internal bool
ParseQualifierExpression(Parser* parser, AST_Node** expression)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

internal bool
ParseLiteralExpression(Parser* parser, AST_Node** expression)
{
    bool encountered_errors = false;
    
    if (!ParsePrimaryExpression(parser, expression)) encountered_errors = true;
    else if (EatToken(parser, Token_PeriodOpenBrace))
    {
        NOT_IMPLEMENTED;
    }
    else if (EatToken(parser, Token_PeriodOpenBrace))
    {
        NOT_IMPLEMENTED;
    }
    
    return !encountered_errors;
}

internal bool
ParseUnaryExpression(Parser* parser, AST_Node** expression)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

internal bool
ParseBinaryExpression(Parser* parser, AST_Node** expression)
{
    bool encountered_errors = false;
    
    if (!ParseUnaryExpression(parser, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            Token token = GetToken(parser);
            if (token.kind < Token_FirstBinary || token.kind > Token_LastBinary) break;
            else                                                                 NextToken(parser);
            
            AST_Node** left = expression;
            AST_Node* right;
            
            if (!ParseUnaryExpression(parser, &right)) encountered_errors = true;
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
        AST_Node* true_expr  = 0;
        AST_Node* false_expr = 0;
        
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