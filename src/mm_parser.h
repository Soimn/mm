#define PARSER_PEEK_MAX 2

typedef struct Parser
{
    Workspace* workspace;
    Lexer lexer;
    Token peek[1 + PARSER_PEEK_MAX];
    u8 peek_cursor;
} Parser;

internal Token
GetToken(Parser* parser)
{
    return parser->peek[parser->peek_cursor];
}

internal Token
PeekToken(Parser* parser, umm index)
{
    ASSERT(index + 1 < ARRAY_SIZE(parser->peek));
    return parser->peek[(parser->peek_cursor + index) % ARRAY_SIZE(parser->peek)];
}

internal Token
NextToken(Parser* parser)
{
    parser->peek_cursor = (parser->peek_cursor + 1) % ARRAY_SIZE(parser->peek);
    parser->peek[parser->peek_cursor] = Lexer_Advance(parser->workspace, &parser->lexer);
    
    return parser->peek[parser->peek_cursor];
}

internal bool
EatToken(Parser* parser, TOKEN_KIND kind)
{
    return (parser->peek[parser->peek_cursor].kind == kind ? NextToken(parser), true : false);
}

internal AST_Node*
PushNode(Parser* parser, AST_NODE_KIND kind)
{
    AST_Node* result = 0;
    NOT_IMPLEMENTED;
    ZeroStruct(result);
    return result;
}

internal bool ParseExpression(Parser* parser, AST_Node** expression);
internal bool ParseStatement(Parser* parser, AST_Node** statement);
internal bool ParseBlock(Parser* parser, AST_Node** block);

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
ParseParameterList(Parser* parser, AST_Node** params)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

internal bool
ParseAssignmentDeclarationOrExpression(Parser* parser, AST_Node** statement)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

internal bool
ParsePrimaryExpression(Parser* parser, AST_Node** expression)
{
    bool encountered_errors = false;
    
    Token token = GetToken(parser);
    
    if (token.kind == Token_String)
    {
        *expression = PushNode(parser, AST_String);
        (*expression)->string = token.string;
    }
    else if (token.kind == Token_Int)
    {
        *expression = PushNode(parser, AST_Int);
        (*expression)->integer = token.integer;
    }
    else if (token.kind == Token_Float)
    {
        *expression = PushNode(parser, AST_Float);
        (*expression)->floating = token.floating;
    }
    else if (token.kind == Token_Character)
    {
        *expression = PushNode(parser, AST_Char);
        (*expression)->character = token.character;
    }
    else if (token.kind == Token_Identifier)
    {
        // NOTE: This also catches INTERNED_STRING_NIL, EMPTY_STRING and
        //       BLANK_IDENTIFIER, but since the token is an identifier,
        //       INTERNED_STRING_NIL and EMPTY_STRING cannot appear
        if (!InternedString_IsKeyword(token.string))
        {
            *expression = PushNode(parser, AST_Identifier);
            (*expression)->string = token.string;
        }
        else
        {
            if (token.string == Keyword_True || token.string == Keyword_False)
            {
                *expression = PushNode(parser, AST_Bool);
                (*expression)->boolean = (token.string == Keyword_True);
            }
            else if (token.string == Keyword_Proc)
            {
                NextToken(parser);
                
                AST_Node* params       = 0;
                AST_Node* return_types = 0;
                AST_Node* where_clause = 0;
                
                if (EatToken(parser, Token_OpenParen))
                {
                    if      (!ParseParameterList(parser, &params)) encountered_errors = true;
                    else if (!EatToken(parser, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren after parameters
                        encountered_errors = true;
                    }
                }
                
                if (EatToken(parser, Token_Arrow))
                {
                    bool is_multi = EatToken(parser, Token_OpenParen);
                    
                    AST_Node** next_type = &return_types;
                    while (!encountered_errors)
                    {
                        AST_Node* first;
                        if (!ParseExpression(parser, &first)) encountered_errors = true;
                        else
                        {
                            AST_Node* name;
                            AST_Node* value;
                            if (!EatToken(parser, Token_Colon))
                            {
                                name  = 0;
                                value = first;
                            }
                            else
                            {
                                name = first;
                                if (!ParseExpression(parser, &value)) encountered_errors = true;
                            }
                            
                            *next_type = PushNode(parser, AST_NamedValue);
                            
                            if (is_multi && EatToken(parser, Token_Comma)) continue;
                            else                                           break;
                        }
                    }
                    
                    if (!encountered_errors && is_multi && !EatToken(parser, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren after return type list
                        encountered_errors = true;
                    }
                }
                
                token = GetToken(parser);
                if (token.kind == Token_Identifier && token.string == Keyword_Where)
                {
                    NextToken(parser);
                    if (!ParseExpression(parser, &where_clause)) encountered_errors = true;
                }
                
                if (!encountered_errors)
                {
                    token = GetToken(parser);
                    
                    if (token.kind != Token_TripleMinus && token.kind != Token_OpenBrace)
                    {
                        *expression = PushNode(parser, AST_Proc);
                        (*expression)->proc_type.params       = params;
                        (*expression)->proc_type.return_types = return_types;
                        (*expression)->proc_type.where_clause = where_clause;
                    }
                    else
                    {
                        AST_Node* body = 0;
                        
                        if (!EatToken(parser, Token_TripleMinus))
                        {
                            if (!ParseBlock(parser, &body)) encountered_errors = true;
                        }
                        
                        if (!encountered_errors)
                        {
                            *expression = PushNode(parser, AST_ProcLiteral);
                            (*expression)->proc_literal.params       = params;
                            (*expression)->proc_literal.return_types = return_types;
                            (*expression)->proc_literal.where_clause = where_clause;
                            (*expression)->proc_literal.body         = body;
                        }
                    }
                }
            }
            else if (token.string == Keyword_Struct || token.string == Keyword_Union)
            {
                bool is_union    = (token.string == Keyword_Union);
                AST_Node* params = 0;
                AST_Node* body   = 0;
                
                if (EatToken(parser, Token_OpenParen))
                {
                    if      (!ParseParameterList(parser, &params)) encountered_errors = true;
                    else if (!EatToken(parser, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren after parameter list
                        encountered_errors = true;
                    }
                }
                
                if (!EatToken(parser, Token_OpenBrace))
                {
                    //// ERROR: Missing body of X
                    encountered_errors = true;
                }
                else
                {
                    AST_Node** next_statement = &body;
                    while (!encountered_errors && !EatToken(parser, Token_CloseBrace))
                    {
                        if (!ParseStatement(parser, next_statement)) encountered_errors = true;
                        if (*next_statement) next_statement = &(*next_statement)->next;
                    }
                }
                
                if (!encountered_errors)
                {
                    if (is_union)
                    {
                        *expression = PushNode(parser, AST_Union);
                        (*expression)->union_type.params = params;
                        (*expression)->union_type.body   = body;
                    }
                    else
                    {
                        *expression = PushNode(parser, AST_Struct);
                        (*expression)->struct_type.params = params;
                        (*expression)->struct_type.body   = body;
                    }
                }
            }
            else if (token.string == Keyword_Enum)
            {
                token = NextToken(parser);
                
                AST_Node* type = 0;
                AST_Node* body = 0;
                
                if (token.kind != Token_OpenBrace)
                {
                    if (!ParseExpression(parser, &type)) encountered_errors = true;
                }
                
                if (!EatToken(parser, Token_OpenBrace))
                {
                    //// ERROR: Missing body of enum
                    encountered_errors = true;
                }
                else
                {
                    if      (!ParseNamedValueList(parser, &body)) encountered_errors = true;
                    else if (!EatToken(parser, Token_CloseBrace))
                    {
                        //// ERROR: Missing closing brace after body of enum
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(parser, AST_Enum);
                    (*expression)->enum_type.type = type;
                    (*expression)->enum_type.body = body;
                }
            }
            else if (token.string == Keyword_Cast || token.string == Keyword_Transmute)
            {
                Interned_String proc = token.string;
                
                NextToken(parser);
                if (!EatToken(parser, Token_OpenParen) || GetToken(parser).kind == Token_CloseParen)
                {
                    //// ERROR: Missing arguments to cast
                    encountered_errors = true;
                }
                else
                {
                    AST_Node* args = 0;
                    if (!ParseNamedValueList(parser, &args)) encountered_errors = true;
                    else
                    {
                        *expression = PushNode(parser, AST_IntrinsicCall);
                        (*expression)->intrinsic_call_expr.proc = proc;
                        (*expression)->intrinsic_call_expr.args = args;
                    }
                }
            }
            else
            {
                //// ERROR: Illegal use of keyword in expression
                encountered_errors = true;
            }
        }
    }
    else if (EatToken(parser, Token_PeriodOpenBrace))
    {
        AST_Node* args = 0;
        
        if (!EatToken(parser, Token_CloseBrace))
        {
            if      (!ParseNamedValueList(parser, &args)) encountered_errors = true;
            else if (!EatToken(parser, Token_CloseBrace))
            {
                //// ERROR: Missing closing brace after arguments to struct literal
                encountered_errors = true;
            }
        }
        
        if (!encountered_errors)
        {
            *expression = PushNode(parser, AST_StructLiteral);
            (*expression)->struct_literal.type = 0;
            (*expression)->struct_literal.args = args;
        }
    }
    else if (EatToken(parser, Token_PeriodOpenBracket))
    {
        AST_Node* args = 0;
        
        if (!EatToken(parser, Token_CloseBracket))
        {
            if      (!ParseNamedValueList(parser, &args)) encountered_errors = true;
            else if (!EatToken(parser, Token_CloseBracket))
            {
                //// ERROR: Missing closing bracket after arguments to array literal
                encountered_errors = true;
            }
        }
        
        if (!encountered_errors)
        {
            *expression = PushNode(parser, AST_ArrayLiteral);
            (*expression)->array_literal.type = 0;
            (*expression)->array_literal.args = args;
        }
    }
    else if (EatToken(parser, Token_OpenParen))
    {
        AST_Node* compound;
        if      (!ParseExpression(parser, &compound)) encountered_errors = true;
        else if (!EatToken(parser, Token_CloseParen))
        {
            //// ERROR: Missing closing paren after compound expression body
            encountered_errors = true;
        }
        else
        {
            *expression = PushNode(parser, AST_Compound);
            (*expression)->compound_expr = compound;
        }
    }
    else if (EatToken(parser, Token_Period))
    {
        token = GetToken(parser);
        if (token.kind != Token_Identifier)
        {
            //// ERROR: Missing name of enum in enum selector expression
            encountered_errors = true;
        }
        else
        {
            Interned_String enum_name = token.string;
            NextToken(parser);
            
            *expression = PushNode(parser, AST_Selector);
            (*expression)->selector_expr = enum_name;
        }
    }
    else
    {
        NOT_IMPLEMENTED;
    }
    
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
                    if      (!ParseNamedValueList(parser, &args)) encountered_errors = true;
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
                    if      (!ParseNamedValueList(parser, &args)) encountered_errors = true;
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
                    if      (!ParseNamedValueList(parser, &args)) encountered_errors = true;
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
                if      (ParseExpression(parser, &size)) encountered_errors = true;
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
    
    if      (!ParseBinaryExpression(parser, expression)) encountered_errors = true;
    else if (EatToken(parser, Token_QuestionMark))
    {
        AST_Node* condition  = *expression;
        AST_Node* true_expr;
        AST_Node* false_expr;
        
        if      (!ParseBinaryExpression(parser, &true_expr)) encountered_errors = true;
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

internal bool
ParseStatement(Parser* parser, AST_Node** statement)
{
    bool encountered_errors = false;
    
    Token token     = GetToken(parser);
    Token peek      = PeekToken(parser, 1);
    Token peek_next = PeekToken(parser, 2);
    
    bool token_is_identifier = (token.kind == Token_Identifier);
    
    bool should_check_for_semicolon = true;
    
    if (token.kind == Token_Semicolon);
    else if (token.kind == Token_OpenBrace)
    {
        should_check_for_semicolon = false;
        
        if (!ParseBlock(parser, statement)) encountered_errors = true;
    }
    else if (token_is_identifier && peek.kind == Token_Colon && (peek_next.kind == Token_OpenBrace ||
                                                                 peek_next.kind == Token_Identifier && (peek_next.string == Keyword_If || peek_next.string == Keyword_While ||
                                                                                                        peek_next.string == Keyword_Else)))
    {
        if (InternedString_IsKeyword(token.string))
        {
            //// ERROR: Illegal use of keyword as label name
            encountered_errors = true;
        }
        else if (peek_next.kind == Token_Identifier && peek_next.string == Keyword_Else)
        {
            //// ERROR: Illegal use of label on else statement
            encountered_errors = true;
        }
        else
        {
            Interned_String label = token.string;
            
            NextToken(parser);
            NextToken(parser);
            
            if (!ParseStatement(parser, statement)) encountered_errors = true;
            else
            {
                if      ((*statement)->kind == AST_Block) (*statement)->block_statement.label = label, should_check_for_semicolon = false;
                else if ((*statement)->kind == AST_If)    (*statement)->if_statement.label    = label;
                else if ((*statement)->kind == AST_While) (*statement)->while_statement.label = label;
                else INVALID_CODE_PATH;
            }
        }
    }
    else if (token_is_identifier && token.string == Keyword_If)
    {
        should_check_for_semicolon = false;
        
        NextToken(parser);
        
        if (!EatToken(parser, Token_OpenParen))
        {
            //// ERROR: Missing condition of if statement
            encountered_errors = true;
        }
        else
        {
            AST_Node* init       = 0;
            AST_Node* condition  = 0;
            AST_Node* true_body  = 0;
            AST_Node* false_body = 0;
            
            token = GetToken(parser);
            if (token.kind != Token_Semicolon && !ParseAssignmentDeclarationOrExpression(parser, &condition)) encountered_errors = true;
            else if (EatToken(parser, Token_Semicolon))
            {
                init = condition;
                if (!ParseAssignmentDeclarationOrExpression(parser, &condition)) encountered_errors = true;
            }
            
            if (!encountered_errors)
            {
                if (!EatToken(parser, Token_CloseParen))
                {
                    //// ERROR: Missing closing paren after if condition
                    encountered_errors = true;
                }
                else
                {
                    if (!ParseStatement(parser, &true_body)) encountered_errors = true;
                    else
                    {
                        token = GetToken(parser);
                        if (token.kind == Token_Identifier && token.string == Keyword_Else)
                        {
                            NextToken(parser);
                            if (!ParseStatement(parser, &false_body)) encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        *statement = PushNode(parser, AST_If);
                        (*statement)->if_statement.init       = init;
                        (*statement)->if_statement.condition  = condition;
                        (*statement)->if_statement.true_body  = true_body;
                        (*statement)->if_statement.false_body = false_body;
                        (*statement)->if_statement.label      = INTERNED_STRING_NIL;
                    }
                }
            }
        }
    }
    else if (token_is_identifier && token.string == Keyword_Else)
    {
        //// ERROR: Illegal else without matching if
        encountered_errors = true;
    }
    else if (token_is_identifier && token.string == Keyword_While)
    {
        should_check_for_semicolon = false;
        
        NextToken(parser);
        
        if (!EatToken(parser, Token_OpenParen))
        {
            //// ERROR: Missing condition of while statement
            encountered_errors = true;
        }
        else
        {
            AST_Node* init      = 0;
            AST_Node* condition = 0;
            AST_Node* step      = 0;
            AST_Node* body      = 0;
            
            token = GetToken(parser);
            if (token.kind != Token_Semicolon && !ParseAssignmentDeclarationOrExpression(parser, &condition)) encountered_errors = true;
            else if (EatToken(parser, Token_Semicolon))
            {
                init = condition;
                
                token = GetToken(parser);
                if (token.kind != Token_Semicolon && !ParseAssignmentDeclarationOrExpression(parser, &condition)) encountered_errors = true;
                else if (EatToken(parser, Token_Semicolon))
                {
                    if (token.kind != Token_Semicolon && !ParseAssignmentDeclarationOrExpression(parser, &step)) encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                if (!EatToken(parser, Token_CloseParen))
                {
                    //// ERROR: Missing closing paren after while header
                    encountered_errors = true;
                }
                else if (!ParseStatement(parser, &body)) encountered_errors = true;
                else
                {
                    *statement = PushNode(parser, AST_While);
                    (*statement)->while_statement.init      = init;
                    (*statement)->while_statement.condition = condition;
                    (*statement)->while_statement.step      = step;
                    (*statement)->while_statement.body      = body;
                    (*statement)->while_statement.label     = INTERNED_STRING_NIL;
                }
            }
        }
    }
    else if (token_is_identifier && (token.string == Keyword_Break || token.string == Keyword_Continue))
    {
        AST_NODE_KIND kind    = (token.string == Keyword_Break);
        Interned_String label = INTERNED_STRING_NIL;
        
        token = NextToken(parser);
        if (token.kind == Token_Identifier)
        {
            if (!InternedString_IsNonBlankIdentifier(token.string))
            {
                //// ERROR: Illegal label
                encountered_errors = true;
            }
            else
            {
                label = token.string;
                NextToken(parser);
            }
        }
        
        if (!encountered_errors)
        {
            *statement = PushNode(parser, kind);
            (*statement)->jump_label = label;
        }
    }
    else if (token_is_identifier && token.string == Keyword_Defer)
    {
        NextToken(parser);
        
        AST_Node* defer_statement = 0;
        if (!ParseStatement(parser, &defer_statement)) encountered_errors = true;
        else
        {
            *statement = PushNode(parser, AST_Defer);
            (*statement)->defer_statement = defer_statement;
        }
    }
    else if (token_is_identifier && token.string == Keyword_Return)
    {
        token = NextToken(parser);
        
        AST_Node* return_values = 0;
        if (token.kind != Token_Semicolon && !ParseNamedValueList(parser, &return_values)) encountered_errors = true;
        
        if (!encountered_errors)
        {
            *statement = PushNode(parser, AST_Return);
            (*statement)->return_values = return_values;
        }
    }
    else
    {
        if (!ParseAssignmentDeclarationOrExpression(parser, statement)) encountered_errors = true;
        else
        {
            if ((*statement)->kind == AST_Constant && (*statement)->constant_decl.names->next == 0 && (*statement)->constant_decl.values->next == 0 &&
                ((*statement)->constant_decl.values->kind == AST_ProcLiteral  || (*statement)->constant_decl.values->kind == AST_Struct ||
                 (*statement)->constant_decl.values->kind == AST_Union        || (*statement)->constant_decl.values->kind == AST_Enum))
            {
                should_check_for_semicolon = false;
            }
        }
    }
    
    if (!encountered_errors && should_check_for_semicolon && !EatToken(parser, Token_Semicolon))
    {
        //// ERROR: Missing terminating semicolon after statement
        encountered_errors = true;
    }
    
    return !encountered_errors;
}


internal bool
ParseBlock(Parser* parser, AST_Node** block)
{
    bool encountered_errors = false;
    
    bool is_block = EatToken(parser, Token_OpenBrace);
    ASSERT(is_block);
    
    AST_Node* body = 0;
    
    AST_Node** next_statement = &body;
    while (!encountered_errors && !EatToken(parser, Token_CloseBrace)) encountered_errors = !ParseStatement(parser, next_statement);
    
    if (!encountered_errors)
    {
        *block = PushNode(parser, AST_Block);
        (*block)->block_statement.label = INTERNED_STRING_NIL;
        (*block)->block_statement.body  = body;
    }
    
    return !encountered_errors;
}