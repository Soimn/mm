internal bool
ParseBinaryExpression()
{
    bool encountered_errors = false;
    
    if (!ParsePrefixExpression(state, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            Token token = GetToken(state);
            
            // IMPORTANT NOTE: EXPRESSION_KIND is organized in blocks of values, each 20 in size
            //                 the blocks from 4 to 9 contain binary expressions
            
            umm precedence = token.kind / 20;
            
            if (precedence < 4 || precedence > 9) break;
            else
            {
                SkipPastCurrentToken(state);
                
                Binary_Expression* binary_expr = PushExpression(state, token.kind);
                
                if (!ParsePrefixExpression(state, &binary_expr->right)) encountered_errors = true;
                else
                {
                    Expression** slot = expression;
                    
                    for (;;)
                    {
                        if ((*slot)->kind / 20 <= precedence)
                        {
                            binary_expr->left = *slot;
                            *slot             = (Expression*)binary_expr;
                        }
                        
                        else
                        {
                            slot = (Expression**)&((Binary_Expression*)*slot)->right;
                        }
                    }
                }
            }
        }
    }
    
    return !encountered_errors;
}

internal bool
ParseFile(u8* file_contents)
{
    Lexer lexer = Lexer_Init(file_contents);
}