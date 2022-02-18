typedef struct Parser_State
{
    Arena* ast_arena;
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
    ASSERT(state->token.kind != Token_Error && state->token.kind != Token_EndOfStream);
    
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
    AST_Node* node = Arena_PushSize(state->ast_arena, sizeof(AST_Node), ALIGNOF(AST_Node));
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
        *expression = PushNode(state, AST_Int);
        (*expression)->integer = BigInt_FromU64(token.character);
        
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
            
            if (!encountered_errors && EatTokenOfKind(state, Token_Arrow))
            {
                if (!EatTokenOfKind(state, Token_OpenParen))
                {
                    if (!ParseExpression(state, &return_values))
                    {
                        encountered_errors = true;
                    }
                }
                else
                {
                    AST_Node** next_value = &return_values;
                    
                    while (!encountered_errors)
                    {
                        if (!ParseSimpleVariableDeclaration(state, next_value, false, false)) encountered_errors = true;
                        else
                        {
                            next_value = &(*next_value)->next;
                            
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
            
            if (!encountered_errors)
            {
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
ParseExpressionList(Parser_State* state, AST_Node** list)
{
    bool encountered_errors = false;
    
    AST_Node** next_expr = list;
    while (!encountered_errors)
    {
        if (!ParseExpression(state, next_expr)) encountered_errors = true;
        else
        {
            next_expr = &(*next_expr)->next;
            
            if (EatTokenOfKind(state, Token_Comma)) continue;
            else                                    break;
        }
    }
    
    return !encountered_errors;
}

internal bool
ParseUsingExpressionVariableOrConstant(Parser_State* state, AST_Node** node)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    bool is_using = false;
    if (token.kind == Token_Identifier && token.identifier == Keyword_Using)
    {
        NextToken(state);
        is_using = true;
    }
    
    AST_Node* expressions;
    if (!ParseExpressionList(state, &expressions)) encountered_errors = true;
    else
    {
        if (EatTokenOfKind(state, Token_Colon))
        {
            token = GetToken(state);
            
            AST_Node* type = 0;
            if (token.kind != Token_Colon && token.kind != Token_Equals)
            {
                if (!ParseExpression(state, &type))
                {
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                if (EatTokenOfKind(state, Token_Colon))
                {
                    AST_Node* values;
                    if (!ParseExpressionList(state, &values)) encountered_errors = true;
                    else
                    {
                        *node = PushNode(state, AST_Constant);
                        (*node)->const_decl.names    = expressions;
                        (*node)->const_decl.type     = type;
                        (*node)->const_decl.values   = values;
                        (*node)->const_decl.is_using = is_using;
                    }
                }
                else
                {
                    AST_Node* values      = 0;
                    bool is_uninitialized = false;
                    
                    if (EatTokenOfKind(state, Token_Equals))
                    {
                        if (EatTokenOfKind(state, Token_TripleMinus)) is_uninitialized = true;
                        else if (!ParseExpressionList(state, &values))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        *node = PushNode(state, AST_Variable);
                        (*node)->var_decl.names            = expressions;
                        (*node)->var_decl.type             = type;
                        (*node)->var_decl.values           = values;
                        (*node)->var_decl.is_using         = is_using;
                        (*node)->var_decl.is_uninitialized = is_uninitialized;
                    }
                }
            }
        }
        else
        {
            token = GetToken(state);
            
            if (token.kind >= Token_FirstAssignment && token.kind <= Token_LastAssignment)
            {
                AST_Node* lhs = expressions;
                AST_Node* rhs = 0;
                
                AST_NODE_KIND op = AST_Invalid;
                if (token.kind != Token_Equals) op = token.kind + 6*16;
                
                if (!ParseExpressionList(state, &rhs)) encountered_errors = true;
                else
                {
                    *node = PushNode(state, AST_Assignment);
                    (*node)->assignment_statement.lhs = lhs;
                    (*node)->assignment_statement.rhs = rhs;
                    (*node)->assignment_statement.op  = op;
                }
            }
            else
            {
                if (expressions->next != 0)
                {
                    //// ERROR: Illegal use of expression list
                    encountered_errors = true;
                }
                else if (is_using)
                {
                    token = GetToken(state);
                    
                    Interned_String alias = BLANK_IDENTIFIER;
                    if (token.kind == Token_Identifier && token.identifier == Keyword_As)
                    {
                        token = NextToken(state);
                        
                        if (token.kind != Token_Identifier)
                        {
                            //// ERROR: Missing name of alias
                            encountered_errors = true;
                        }
                        else if (token.identifier <= KEYWORD_KIND_MAX)
                        {
                            //// ERROR: Illegal use of keyword as alias name
                            encountered_errors = true;
                        }
                        else
                        {
                            alias = token.identifier;
                            NextToken(state);
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        *node = PushNode(state, AST_Using);
                        (*node)->using_statement.symbol = expressions;
                        (*node)->using_statement.alias  = alias;
                    }
                }
                else
                {
                    *node = expressions;
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
    
    Interned_String label = BLANK_IDENTIFIER;
    if (EatTokenOfKind(state, Token_Colon))
    {
        Token token = GetToken(state);
        
        if (token.kind != Token_Identifier)
        {
            //// ERROR: Missing name of label
            encountered_errors = true;
        }
        else if (token.identifier <= KEYWORD_KIND_MAX)
        {
            //// ERROR: Illegal use of keyword as label name
            encountered_errors = true;
        }
        else
        {
            label = token.identifier;
            
            token = NextToken(state);
            
            if (token.kind != Token_OpenBrace &&
                (token.kind != Token_Identifier || (token.identifier != Keyword_If && token.identifier != Keyword_While)))
            {
                //// ERROR: Illegal use of label
                encountered_errors = true;
            }
        }
    }
    
    if (!encountered_errors)
    {
        Token token = GetToken(state);
        
        if (token.kind == Token_Semicolon)
        {
            while (EatTokenOfKind(state, Token_Semicolon));
        }
        else if (token.kind == Token_OpenBrace)
        {
            if (!ParseScope(state, statement)) encountered_errors = true;
            else (*statement)->block_statement.label = label;
        }
        else if (token.kind == Token_Identifier && token.identifier == Keyword_If)
        {
            NextToken(state);
            
            AST_Node* init       = 0;
            AST_Node* condition  = 0;
            AST_Node* true_body  = 0;
            AST_Node* false_body = 0;
            
            if (!EatTokenOfKind(state, Token_OpenParen))
            {
                //// ERROR: Missing open paren after if
                encountered_errors = true;
            }
            else
            {
                AST_Node* first  = 0;
                AST_Node* second = 0;
                
                token = GetToken(state);
                
                if (token.kind != Token_Semicolon)
                {
                    if (!ParseUsingExpressionVariableOrConstant(state, &first)) 
                    {
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    if (EatTokenOfKind(state, Token_Semicolon))
                    {
                        if (!ParseUsingExpressionVariableOrConstant(state, &second)) 
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        if (second != 0)
                        {
                            init      = first;
                            condition = second;
                        }
                        else
                        {
                            init      = 0;
                            condition = first;
                        }
                        
                        if (condition->kind < AST_FirstExpression || condition->kind > AST_LastExpression)
                        {
                            //// ERROR: If condition must be an expression
                            encountered_errors = true;
                        }
                        else if (!EatTokenOfKind(state, Token_CloseParen))
                        {
                            //// ERROR: Missing closing after if condition
                            encountered_errors = true;
                        }
                    }
                }
            }
            
            if (!encountered_errors)
            {
                if (!ParseScope(state, &true_body)) encountered_errors = true;
                else
                {
                    token = GetToken(state);
                    if (token.kind == Token_Identifier && token.identifier == Keyword_Else)
                    {
                        token = NextToken(state);
                        
                        if (token.kind == Token_Identifier && token.identifier == Keyword_If)
                        {
                            if (!ParseStatement(state, &false_body))
                            {
                                encountered_errors = true;
                            }
                        }
                        else if (!ParseScope(state, &false_body))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        *statement = PushNode(state, AST_If);
                        (*statement)->if_statement.label      = label;
                        (*statement)->if_statement.init       = init;
                        (*statement)->if_statement.condition  = condition;
                        (*statement)->if_statement.true_body  = true_body;
                        (*statement)->if_statement.false_body = false_body;
                    }
                }
            }
        }
        else if (token.kind == Token_Identifier && token.identifier == Keyword_When)
        {
            NextToken(state);
            
            AST_Node* condition  = 0;
            AST_Node* true_body  = 0;
            AST_Node* false_body = 0;
            
            if (!EatTokenOfKind(state, Token_OpenParen))
            {
                //// ERROR: Missing open paren after when
                encountered_errors = true;
            }
            else
            {
                if (!ParseExpression(state, &condition)) encountered_errors = true;
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
                if (!ParseScope(state, &true_body)) encountered_errors = true;
                else
                {
                    token = GetToken(state);
                    if (token.kind == Token_Identifier && token.identifier == Keyword_Else)
                    {
                        token = NextToken(state);
                        
                        if (token.kind == Token_Identifier && token.identifier == Keyword_When)
                        {
                            if (!ParseStatement(state, &false_body))
                            {
                                encountered_errors = true;
                            }
                        }
                        else if (!ParseScope(state, &false_body))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        *statement = PushNode(state, AST_When);
                        (*statement)->if_statement.condition  = condition;
                        (*statement)->if_statement.true_body  = true_body;
                        (*statement)->if_statement.false_body = false_body;
                    }
                }
            }
        }
        else if (token.kind == Token_Identifier && token.identifier == Keyword_Else)
        {
            //// ERROR: Illegal use of else without matching if or when
            encountered_errors = true;
        }
        else if (token.kind == Token_Identifier && token.identifier == Keyword_While)
        {
            NextToken(state);
            
            AST_Node* init       = 0;
            AST_Node* condition  = 0;
            AST_Node* step       = 0;
            AST_Node* body;
            
            if (!EatTokenOfKind(state, Token_OpenParen))
            {
                //// ERROR: Missing open paren after while
                encountered_errors = true;
            }
            else
            {
                AST_Node* first = 0;
                
                token = GetToken(state);
                
                if (token.kind != Token_Semicolon)
                {
                    if (!ParseUsingExpressionVariableOrConstant(state, &first))
                    {
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    if (!EatTokenOfKind(state, Token_Semicolon))
                    {
                        init      = 0;
                        condition = first;
                        step      = 0;
                    }
                    else
                    {
                        init = first;
                        
                        token = GetToken(state);
                        
                        if (token.kind != Token_Semicolon)
                        {
                            if (!ParseUsingExpressionVariableOrConstant(state, &condition))
                            {
                                encountered_errors = true;
                            }
                        }
                        
                        if (!encountered_errors)
                        {
                            if (EatTokenOfKind(state, Token_Semicolon))
                            {
                                token = GetToken(state);
                                
                                if (!ParseUsingExpressionVariableOrConstant(state, &step))
                                {
                                    encountered_errors = true;
                                }
                            }
                        }
                    }
                }
                
                if (!encountered_errors)
                {
                    if (!EatTokenOfKind(state, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren
                        encountered_errors = true;
                    }
                    else if (condition != 0 && condition->kind < AST_FirstExpression && condition->kind > AST_LastExpression)
                    {
                        //// ERROR: While condition must be an expression
                        encountered_errors = true;
                    }
                    else if (step != 0 && (step->kind < AST_FirstExpression && step->kind > AST_LastExpression &&
                                           step->kind != AST_Assignment))
                    {
                        //// ERROR: While step statement must be either an expression or an assignment
                        encountered_errors = true;
                    }
                }
            }
            
            if (!encountered_errors)
            {
                if (!ParseScope(state, &body)) encountered_errors = true;
                else
                {
                    *statement = PushNode(state, AST_While);
                    (*statement)->while_statement.label     = label;
                    (*statement)->while_statement.init      = init;
                    (*statement)->while_statement.condition = condition;
                    (*statement)->while_statement.step      = step;
                    (*statement)->while_statement.body      = body;
                }
            }
        }
        else if (token.kind == Token_Identifier && (token.identifier == Keyword_Break || token.identifier == Keyword_Continue))
        {
            bool is_break = (token.identifier == Keyword_Break);
            
            token = NextToken(state);
            
            Interned_String jmp_label = BLANK_IDENTIFIER;
            if (token.kind == Token_Identifier)
            {
                if (token.identifier <= KEYWORD_KIND_MAX)
                {
                    //// ERROR: Illegal use of keyword as jump label
                    encountered_errors = true;
                }
                else
                {
                    jmp_label = token.identifier;
                    NextToken(state);
                }
            }
            
            if (!encountered_errors)
            {
                if (!EatTokenOfKind(state, Token_Semicolon))
                {
                    //// ERROR: Missing terminating semicolon
                    encountered_errors = true;
                }
                else
                {
                    *statement = PushNode(state, is_break ? AST_Break : AST_Continue);
                    (*statement)->jmp_statement.label = jmp_label;
                }
            }
        }
        else if (token.kind == Token_Identifier && token.identifier == Keyword_Defer)
        {
            NextToken(state);
            
            AST_Node* defer_statement;
            if (!ParseScope(state, &defer_statement)) encountered_errors = true;
            else if (!EatTokenOfKind(state, Token_Semicolon))
            {
                //// ERROR: Missing terminating semicolon
                encountered_errors = true;
            }
            else
            {
                *statement = PushNode(state, AST_Defer);
                (*statement)->defer_statement = defer_statement;
            }
        }
        else if (token.kind == Token_Identifier && token.identifier == Keyword_Return)
        {
            token = NextToken(state);
            
            AST_Node* args = 0;
            if (token.kind != Token_Semicolon)
            {
                if (!ParseNamedValueList(state, &args))
                {
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                if (!EatTokenOfKind(state, Token_Semicolon))
                {
                    //// ERROR: Missing terminating semicolon
                    encountered_errors = true;
                }
                else
                {
                    *statement = PushNode(state, AST_Return);
                    (*statement)->return_statement.args = args;
                }
            }
        }
        else if (token.kind == Token_Identifier && token.identifier == Keyword_Include)
        {
            token = NextToken(state);
            
            if (token.kind != Token_String)
            {
                //// ERROR: Missing path after include keyword
                encountered_errors = true;
            }
            else
            {
                NextToken(state);
                
                if (!EatTokenOfKind(state, Token_Semicolon))
                {
                    //// ERROR: Missing terminating semicolon
                    encountered_errors = true;
                }
                else
                {
                    *statement = PushNode(state, AST_Include);
                    (*statement)->include_decl.path = token.string;
                }
            }
        }
        else
        {
            if (!ParseUsingExpressionVariableOrConstant(state, statement)) encountered_errors = true;
            else
            {
                if (!EatTokenOfKind(state, Token_Semicolon))
                {
                    // NOTE: ignore missing semicolon on declarations of the kind
                    //       NAME :: proc ...
                    //       NAME :: struct ...
                    //       NAME :: union ...
                    //       NAME :: enum ...
                    // NOTE: missing semicolons are not ignored on procedure types
                    AST_Node* stmnt = *statement;
                    if (stmnt->kind == AST_Constant         &&
                        stmnt->const_decl.names->next == 0  &&
                        stmnt->const_decl.values->next == 0 &&
                        (stmnt->const_decl.values->kind == AST_ProcLiteral || 
                         stmnt->const_decl.values->kind == AST_Struct      || 
                         stmnt->const_decl.values->kind == AST_Union       || 
                         stmnt->const_decl.values->kind == AST_Enum));
                    else
                    {
                        //// ERROR: Missing terminating semicolon
                        encountered_errors = true;
                    }
                }
            }
        }
    }
    
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
        if (is_braced && EatTokenOfKind(state, Token_CloseBrace)) break;
        else if (!ParseStatement(state, next_statement)) encountered_errors = true;
        else
        {
            if (!is_braced) break;
            
            if (*next_statement != 0)
            {
                next_statement = &(*next_statement)->next;
            }
        }
    }
    
    if (!encountered_errors)
    {
        *scope = PushNode(state, AST_Block);
        (*scope)->block_statement.body      = body;
        (*scope)->block_statement.label     = BLANK_IDENTIFIER;
        (*scope)->block_statement.is_braced = is_braced;
    }
    
    return !encountered_errors;
}

internal bool
ParseString(String contents, Arena* ast_arena, AST_Node** result)
{
    bool encountered_errors = false;
    
    Parser_State state = {0};
    state.ast_arena = ast_arena;
    state.lexer     = Lexer_Init(contents);
    state.token     = Lexer_NextToken(&state.lexer);
    
    AST_Node** next_decl = result;
    while (!encountered_errors)
    {
        Token token = GetToken(&state);
        
        if (token.kind == Token_EndOfStream) break;
        else
        {
            if (!ParseStatement(&state, next_decl)) encountered_errors = true;
            else
            {
                if (*next_decl == 0)
                {
                    //// ERROR: Illegal use of statement in global scope
                    encountered_errors = true;
                }
                else
                {
                    next_decl = &(*next_decl)->next;
                }
            }
        }
    }
    
    return !encountered_errors;
}