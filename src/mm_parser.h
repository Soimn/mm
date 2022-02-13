typedef struct Parser_State
{
    Lexer lexer;
    Token token;
} Parser_State;

internal inline Token
GetToken(Parser_State* state)
{
    return state->token;
}

internal inline Token
NextToken(Parser_State* state)
{
    state->token = Lexer_NextToken(&state->lexer);
    return state->token;
}

internal inline bool
EatTokenOfKind(Parser_State* state, TOKEN_KIND kind)
{
    bool result = false;
    
    if (state->token.kind == kind)
    {
        NextToken(state);
        result = true;
    }
    
    return result;
}

internal inline AST_Node*
PushNode(Parser_State* state, AST_NODE_KIND kind)
{
    AST_Node* node = 0;
    NOT_IMPLEMENTED;
    ZeroStruct(node);
    
    node->kind = kind;
    
    return node;
}

internal bool ParseExpression(Parser_State* state, AST_Node** expression);
internal bool ParseScope(Parser_State* state, AST_Node** scope);

internal bool
ParseNamedValueList(Parser_State* state, AST_Node** list)
{
    bool encountered_errors = false;
    
    AST_Node** next = list;
    while (!encountered_errors)
    {
        AST_Node* name  = 0;
        AST_Node* value = 0;
        if (!ParseExpression(state, &value)) encountered_errors = true;
        else
        {
            if (EatTokenOfKind(state, Token_Equals))
            {
                name = value;
                if (!ParseExpression(state, &value))
                {
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                *next = PushNode(state, AST_NamedValue);
                (*next)->named_value.name  = name;
                (*next)->named_value.value = value;
                
                next = &(*next)->next;
                
                if (EatTokenOfKind(state, Token_Comma)) continue;
                else                                    break;
            }
        }
    }
    
    return !encountered_errors;
}

internal bool
ParseSimpleVariableDeclaration(Parser_State* state, AST_Node** decl, bool allow_using, bool allow_value)
{
    bool encountered_errors = false;
    
    *decl = PushNode(state, AST_Variable);
    
    Token token = GetToken(state);
    if (token.kind == Token_Identifier && token.identifier == Keyword_Using)
    {
        if (!allow_using)
        {
            //// ERROR: Using is not allowed in this context
        }
        else
        {
            NextToken(state);
            (*decl)->var_decl.is_using = true;
        }
    }
    
    AST_Node** next_name = &(*decl)->var_decl.names;
    while (!encountered_errors)
    {
        token = GetToken(state);
        
        if (token.kind != Token_Identifier)
        {
            //// ERROR: Missing variable name
            encountered_errors = true;
        }
        else if (token.identifier <= KEYWORD_KIND_MAX)
        {
            //// ERROR: Illegal use of keyword as variable name
            encountered_errors = true;
        }
        else
        {
            NextToken(state);
            
            *next_name = PushNode(state, AST_Identifier);
            (*next_name)->identifier = token.identifier;
            
            next_name = &(*next_name)->next;
            
            if (EatTokenOfKind(state, Token_Comma)) continue;
            else                                    break;
        }
    }
    
    if (!encountered_errors)
    {
        if (!EatTokenOfKind(state, Token_Colon))
        {
            //// ERROR: Missing type of variable
            encountered_errors = true;
        }
        else
        {
            token = GetToken(state);
            if (token.kind != Token_Equals)
            {
                if (!ParseExpression(state, &(*decl)->var_decl.type))
                {
                    encountered_errors = true;
                }
            }
        }
    }
    
    if (!encountered_errors)
    {
        if (EatTokenOfKind(state, Token_Equals))
        {
            if (!allow_value)
            {
                //// ERROR: Assignment is not allowed in this context
                encountered_errors = true;
            }
            else
            {
                if (!ParseExpression(state, &(*decl)->var_decl.values))
                {
                    encountered_errors = true;
                }
            }
        }
    }
    
    return !encountered_errors;
}

internal bool
ParsePrimaryExpression(Parser_State* state, AST_Node** expression)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    if (token.kind == Token_String)
    {
        *expression = PushNode(state, AST_String);
        (*expression)->string = token.string;
        
        NextToken(state);
    }
    else if (token.kind == Token_Character)
    {
        *expression = PushNode(state, AST_Char);
        (*expression)->character = token.character;
        
        NextToken(state);
    }
    else if (token.kind == Token_Int)
    {
        *expression = PushNode(state, AST_Int);
        (*expression)->integer = token.integer;
        
        NextToken(state);
    }
    else if (token.kind == Token_Float)
    {
        *expression = PushNode(state, AST_Float);
        (*expression)->floating = token.floating;
        
        NextToken(state);
    }
    else if (token.kind == Token_Identifier)
    {
        if (token.identifier == Keyword_True || token.identifier == Keyword_False)
        {
            *expression = PushNode(state, AST_Boolean);
            (*expression)->boolean = (token.identifier == Keyword_True);
            
            NextToken(state);
        }
        else if (token.identifier == Keyword_Proc)
        {
            NextToken(state);
            
            AST_Node* params        = 0;
            AST_Node* return_values = 0;
            
            if (EatTokenOfKind(state, Token_OpenParen))
            {
                if (!EatTokenOfKind(state, Token_CloseParen))
                {
                    AST_Node** next_param = &params;
                    
                    while (!encountered_errors)
                    {
                        if (!ParseSimpleVariableDeclaration(state, next_param, true, true)) encountered_errors = true;
                        else
                        {
                            next_param = &(*next_param)->next;
                            
                            if (EatTokenOfKind(state, Token_Comma)) continue;
                            else break;
                        }
                    }
                    
                    if (!EatTokenOfKind(state, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren
                        encountered_errors = true;
                    }
                }
            }
            
            if (EatTokenOfKind(state, Token_Arrow))
            {
                bool is_list = EatTokenOfKind(state, Token_OpenParen);
                
                if (!is_list || is_list && !EatTokenOfKind(state, Token_CloseParen))
                {
                    AST_Node** next_value = &return_values;
                    
                    while (!encountered_errors)
                    {
                        if (!ParseSimpleVariableDeclaration(state, next_value, false, false)) encountered_errors = true;
                        else
                        {
                            next_value = &(*next_value)->next;
                            
                            if (is_list && EatTokenOfKind(state, Token_Comma)) continue;
                            else break;
                        }
                    }
                    
                    if (is_list && !EatTokenOfKind(state, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren
                        encountered_errors = true;
                    }
                }
            }
            
            token = GetToken(state);
            if (token.kind != Token_OpenBrace && token.kind != Token_TripleMinus)
            {
                *expression = PushNode(state, AST_Proc);
                (*expression)->proc_type.params        = params;
                (*expression)->proc_type.return_values = return_values;
            }
            else
            {
                AST_Node* body = 0;
                
                if (!EatTokenOfKind(state, Token_TripleMinus))
                {
                    if (!ParseScope(state, &body))
                    {
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(state, AST_ProcLiteral);
                    (*expression)->proc_literal.params        = params;
                    (*expression)->proc_literal.return_values = return_values;
                    (*expression)->proc_literal.body          = body;
                }
            }
        }
        else if (token.identifier == Keyword_Struct ||
                 token.identifier == Keyword_Union)
        {
            NextToken(state);
            
            if (!EatTokenOfKind(state, Token_OpenBrace))
            {
                //// ERROR: Missing body of struct/enum
                encountered_errors = true;
            }
            else
            {
                AST_Node* members = 0;
                
                if (!EatTokenOfKind(state, Token_CloseBrace))
                {
                    AST_Node** next_member = &members;
                    
                    while (!encountered_errors)
                        
                    {
                        if (!ParseSimpleVariableDeclaration(state, next_member, true, false)) encountered_errors = true;
                        else
                        {
                            next_member = &(*next_member)->next;
                            
                            if (EatTokenOfKind(state, Token_Comma)) continue;
                            else                                    break;
                        }
                    }
                    
                    if (!EatTokenOfKind(state, Token_CloseBrace))
                    {
                        //// ERROR: Missing closing brace
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(state, (token.identifier == Keyword_Struct ? AST_Struct : AST_Union));
                    (*expression)->struct_type.members = members;
                }
            }
        }
        else if (token.identifier == Keyword_Enum)
        {
            NextToken(state);
            
            AST_Node* type = 0;
            
            if (!EatTokenOfKind(state, Token_OpenBrace))
            {
                if (!ParseExpression(state, &type)) encountered_errors = true;
                else
                {
                    if (!EatTokenOfKind(state, Token_OpenBrace))
                    {
                        //// ERROR: Missing body of enum
                        encountered_errors = true;
                    }
                }
            }
            
            AST_Node* members = 0;
            if (!EatTokenOfKind(state, Token_CloseBrace))
            {
                if (!ParseNamedValueList(state, &members)) encountered_errors = true;
                else if (!EatTokenOfKind(state, Token_CloseBrace))
                {
                    //// ERROR: Missing closing brace
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                *expression = PushNode(state, AST_Enum);
                (*expression)->enum_type.member_type = type;
                (*expression)->enum_type.members     = members;
            }
        }
        else
        {
            *expression = PushNode(state, AST_Identifier);
            (*expression)->identifier = token.identifier;
            
            NextToken(state);
        }
    }
    else if (EatTokenOfKind(state, Token_OpenParen))
    {
        AST_Node* expr;
        if (!ParseExpression(state, &expr)) encountered_errors = true;
        else
        {
            if (!EatTokenOfKind(state, Token_CloseParen))
            {
                //// ERROR: Missing closing paren
                encountered_errors = true;
            }
            else
            {
                *expression = PushNode(state, AST_Compound);
                (*expression)->compound_expr = expr;
            }
        }
    }
    else if (EatTokenOfKind(state, Token_Period))
    {
        token = GetToken(state);
        
        if (token.kind != Token_Identifier)
        {
            //// ERROR: Missing name of element
            encountered_errors = true;
        }
        else
        {
            Interned_String element = token.identifier;
            
            *expression = PushNode(state, AST_ElementOf);
            (*expression)->element_of.structure = 0;
            (*expression)->element_of.element   = element;
        }
    }
    else if (EatTokenOfKind(state, Token_OpenPeriodBrace))
    {
        AST_Node* args = 0;
        
        if (!EatTokenOfKind(state, Token_CloseBrace))
        {
            if (!ParseNamedValueList(state, &args)) encountered_errors = true;
            else
            {
                if (!EatTokenOfKind(state, Token_CloseBrace))
                {
                    //// ERROR: Missing closing brace
                    encountered_errors = true;
                }
            }
        }
        
        if (!encountered_errors)
        {
            *expression = PushNode(state, AST_StructLiteral);
            (*expression)->struct_literal.type = 0;
            (*expression)->struct_literal.args = args;
        }
    }
    else if (EatTokenOfKind(state, Token_OpenPeriodBracket))
    {
        AST_Node* args = 0;
        
        if (!EatTokenOfKind(state, Token_CloseBracket))
        {
            if (!ParseNamedValueList(state, &args)) encountered_errors = true;
            else
            {
                if (!EatTokenOfKind(state, Token_CloseBracket))
                {
                    //// ERROR: Missing closing bracket
                    encountered_errors = true;
                }
            }
        }
        
        if (!encountered_errors)
        {
            *expression = PushNode(state, AST_ArrayLiteral);
            (*expression)->array_literal.type = 0;
            (*expression)->array_literal.args = args;
        }
    }
    else if (EatTokenOfKind(state, Token_OpenPeriodParen))
    {
        AST_Node* expr = 0;
        
        if (!EatTokenOfKind(state, Token_CloseParen))
        {
            if (!ParseExpression(state, &expr)) encountered_errors = true;
            else
            {
                if (!EatTokenOfKind(state, Token_CloseParen))
                {
                    //// ERROR: Missing closing paren
                    encountered_errors = true;
                }
            }
        }
        
        if (!encountered_errors)
        {
            *expression = PushNode(state, AST_Cast);
            (*expression)->cast_expr.type = 0;
            (*expression)->cast_expr.expr = expr;
        }
    }
    else
    {
        if (token.kind != Token_Error)
        {
            //// ERROR: missing primary expression
            encountered_errors = true;
        }
    }
    
    
    return !encountered_errors;
}

internal bool
ParseTypeExpression(Parser_State* state, AST_Node** expression)
{
    bool encountered_errors = false;
    
    AST_Node** slot = expression;
    while (!encountered_errors)
    {
        if (EatTokenOfKind(state, Token_Hat))
        {
            *slot = PushNode(state, AST_PointerType);
            slot = &(*slot)->unary_expr;
        }
        else if (EatTokenOfKind(state, Token_OpenBracket))
        {
            if (EatTokenOfKind(state, Token_CloseBracket))
            {
                *slot = PushNode(state, AST_SliceType);
                slot = &(*slot)->unary_expr;
            }
            else
            {
                AST_Node* size;
                if (!ParseExpression(state, &size)) encountered_errors = true;
                else
                {
                    *slot = PushNode(state, AST_ArrayType);
                    (*slot)->array_type.size = size;
                    slot = &(*slot)->array_type.type;
                }
            }
        }
        else
        {
            if (!ParsePrimaryExpression(state, slot)) encountered_errors = true;
            break;
        }
    }
    
    return !encountered_errors;
}

internal bool
ParsePostfixExpression(Parser_State* state, AST_Node** expression)
{
    bool encountered_errors = false;
    
    if (!ParseTypeExpression(state, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            if (EatTokenOfKind(state, Token_OpenParen))
            {
                AST_Node* func = *expression;
                AST_Node* args = 0;
                
                if (!EatTokenOfKind(state, Token_CloseParen))
                {
                    if (!ParseNamedValueList(state, &args)) encountered_errors = true;
                    else
                    {
                        if (!EatTokenOfKind(state, Token_CloseParen))
                        {
                            //// ERROR: Missing closing paren
                            encountered_errors = true;
                        }
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(state, AST_Call);
                    (*expression)->call_expr.func = func;
                    (*expression)->call_expr.args = args;
                }
            }
            else if (EatTokenOfKind(state, Token_Period))
            {
                AST_Node* structure = *expression;
                
                Token token = GetToken(state);
                
                if (token.kind != Token_Identifier)
                {
                    //// ERROR: Missing name of element
                    encountered_errors = true;
                }
                else
                {
                    Interned_String element = token.identifier;
                    
                    *expression = PushNode(state, AST_ElementOf);
                    (*expression)->element_of.structure = structure;
                    (*expression)->element_of.element   = element;
                }
            }
            else if (EatTokenOfKind(state, Token_OpenPeriodBrace))
            {
                AST_Node* type = *expression;
                AST_Node* args = 0;
                
                if (!EatTokenOfKind(state, Token_CloseBrace))
                {
                    if (!ParseNamedValueList(state, &args)) encountered_errors = true;
                    else
                    {
                        if (!EatTokenOfKind(state, Token_CloseBrace))
                        {
                            //// ERROR: Missing closing brace
                            encountered_errors = true;
                        }
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(state, AST_StructLiteral);
                    (*expression)->struct_literal.type = type;
                    (*expression)->struct_literal.args = args;
                }
            }
            else if (EatTokenOfKind(state, Token_OpenPeriodBracket))
            {
                AST_Node* type = *expression;
                AST_Node* args = 0;
                
                if (!EatTokenOfKind(state, Token_CloseBracket))
                {
                    if (!ParseNamedValueList(state, &args)) encountered_errors = true;
                    else
                    {
                        if (!EatTokenOfKind(state, Token_CloseBracket))
                        {
                            //// ERROR: Missing closing bracket
                            encountered_errors = true;
                        }
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(state, AST_ArrayLiteral);
                    (*expression)->array_literal.type = type;
                    (*expression)->array_literal.args = args;
                }
            }
            else if (EatTokenOfKind(state, Token_OpenPeriodParen))
            {
                AST_Node* type = *expression;
                AST_Node* expr = 0;
                
                if (!EatTokenOfKind(state, Token_CloseParen))
                {
                    if (!ParseExpression(state, &expr)) encountered_errors = true;
                    else
                    {
                        if (!EatTokenOfKind(state, Token_CloseParen))
                        {
                            //// ERROR: Missing closing paren
                            encountered_errors = true;
                        }
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(state, AST_Cast);
                    (*expression)->cast_expr.type = type;
                    (*expression)->cast_expr.expr = expr;
                }
            }
            else if (EatTokenOfKind(state, Token_OpenBracket))
            {
                AST_Node* array = *expression;
                AST_Node* index = 0;
                
                Token token = GetToken(state);
                if (token.kind != Token_CloseBracket)
                {
                    if (!ParseExpression(state, &index))
                    {
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    if (EatTokenOfKind(state, Token_Colon))
                    {
                        AST_Node* past_end = 0;
                        
                        if (!EatTokenOfKind(state, Token_CloseBracket))
                        {
                            if (!ParseExpression(state, &past_end)) encountered_errors = true;
                            else
                            {
                                if (!EatTokenOfKind(state, Token_CloseBracket))
                                {
                                    //// ERROR: Missing closing bracket
                                    encountered_errors = true;
                                }
                            }
                        }
                        
                        if (!encountered_errors)
                        {
                            *expression = PushNode(state, AST_Slice);
                            (*expression)->slice_expr.array    = array;
                            (*expression)->slice_expr.start    = index;
                            (*expression)->slice_expr.past_end = past_end;
                        }
                    }
                    else if (EatTokenOfKind(state, Token_CloseBracket))
                    {
                        *expression = PushNode(state, AST_Subscript);
                        (*expression)->subscript_expr.array = array;
                        (*expression)->subscript_expr.index = index;
                    }
                    else
                    {
                        //// ERROR: Missing closing bracket
                        encountered_errors = true;
                    }
                }
            }
            else break;
        }
    }
    
    return !encountered_errors;
}

internal bool
ParsePrefixExpression(Parser_State* state, AST_Node** expression)
{
    bool encountered_errors = false;
    
    AST_Node** slot = expression;
    while (!encountered_errors)
    {
        AST_NODE_KIND op = AST_Invalid;
        
        if      (EatTokenOfKind(state, Token_Plus)) continue;
        else if (EatTokenOfKind(state, Token_Minus))      op = AST_Neg;
        else if (EatTokenOfKind(state, Token_Star))       op = AST_Dereference;
        else if (EatTokenOfKind(state, Token_And))        op = AST_Reference;
        else if (EatTokenOfKind(state, Token_Not))        op = AST_Not;
        else if (EatTokenOfKind(state, Token_Complement)) op = AST_BitNot;
        else
        {
            if (!ParsePostfixExpression(state, slot)) encountered_errors = true;
            break;
        }
        
        *slot = PushNode(state, op);
        slot = &(*slot)->unary_expr;
    }
    
    return !encountered_errors;
}

internal bool
ParseBinaryExpression(Parser_State* state, AST_Node** expression)
{
    bool encountered_errors = false;
    
    if (!ParsePrefixExpression(state, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            AST_NODE_KIND op = GetToken(state).kind;
            
            if (op < AST_FirstBinary || op > AST_LastBinary) break;
            else
            {
                NextToken(state);
                
                AST_Node** slot = expression;
                AST_Node* right;
                
                if (!ParsePrefixExpression(state, &right)) encountered_errors = true;
                else
                {
                    umm block = op >> 4;
                    
                    for (;;)
                    {
                        if ((*slot)->kind >> 4 > block) slot = &(*slot)->binary_expr.right;
                        else
                        {
                            AST_Node* left = *slot;
                            
                            *slot = PushNode(state, op);
                            (*slot)->binary_expr.left  = left;
                            (*slot)->binary_expr.right = right;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    return !encountered_errors;
}

internal bool
ParseExpression(Parser_State* state, AST_Node** expression)
{
    bool encountered_errors = false;
    
    if (!ParseBinaryExpression(state, expression)) encountered_errors = true;
    else
    {
        if (EatTokenOfKind(state, Token_QuestionMark))
        {
            AST_Node* condition = *expression;
            AST_Node* true_clause;
            AST_Node* false_clause;
            
            if (!ParseBinaryExpression(state, &true_clause)) encountered_errors = true;
            else
            {
                if (!EatTokenOfKind(state, Token_Colon))
                {
                    //// ERROR: Missing false clause
                    encountered_errors = true;
                }
                else
                {
                    if (!ParseBinaryExpression(state, &false_clause)) encountered_errors = true;
                    else
                    {
                        *expression = PushNode(state, AST_Conditional);
                        (*expression)->conditional_expr.condition    = condition;
                        (*expression)->conditional_expr.true_clause  = true_clause;
                        (*expression)->conditional_expr.false_clause = false_clause;
                    }
                }
            }
        }
    }
    
    return !encountered_errors;
}

internal bool
ParseStatement(Parser_State* state, AST_Node** statement)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

internal bool
ParseScope(Parser_State* state, AST_Node** scope)
{
    bool encountered_errors = false;
    
    bool is_braced = false;
    AST_Node* body = 0;
    
    if (EatTokenOfKind(state, Token_OpenBrace))
    {
        is_braced = true;
    }
    
    AST_Node** next_statement = &body;
    while (!encountered_errors)
    {
        if (!ParseStatement(state, ))
    }
    
    if (!encountered_errors)
    {
        *scope = PushNode(state, AST_Scope);
        (*scope)->block_statement.body      = body;
        (*scope)->block_statement.label     = BLANK_IDENTIFIER;
        (*scope)->block_statement.is_braced = is_braced;
    }
    
    return !encountered_errors;
}