typedef struct Parser_State
{
    Lexer lexer;
    Token current_token;
} Parser_State;

internal inline Token
GetToken(Parser_State* state)
{
    return state->current_token;
}

internal inline Token
PeekToken(Parser_State* state, umm index)
{
    Token token = state->current_token;
    
    Lexer tmp_lexer = state->lexer;
    
    for (umm i = 1; i < index; ++i)
    {
        token = Lexer_Advance(&tmp_lexer);
    }
    
    return token;
}

internal inline void
SkipTokens(Parser_State* state, umm token_count)
{
    for (umm i = 0; i < token_count; ++i)
    {
        state->current_token = Lexer_Advance(&state->lexer);
    }
}

internal inline bool
EatTokenOfKind(Parser_State* state, u8 kind)
{
    Token token = GetToken(state);
    
    bool did_match = (token.kind == kind);
    
    if (did_match)
    {
        SkipTokens(state, 1);
    }
    
    return did_match;
}

internal inline AST_Node*
PushNode(Parser_State* state, u8 kind)
{
    AST_Node* result = 0;
    NOT_IMPLEMENTED;
    ZeroStruct(result);
    return result;
}

internal bool ParseExpression(Parser_State* state, AST_Node** expression);

internal bool
ParseNamedValueList(Parser_State* state, AST_Node** first_pointer)
{
    bool encountered_errors = false;
    
    AST_Node** next_pointer = first_pointer;
    
    while (!encountered_errors)
    {
        AST_Node* value = 0;
        
        if (!ParseExpression(state, &value)) encountered_errors = true;
        else
        {
            if (EatTokenOfKind(state, Token_Equals))
            {
                AST_Node* name = value;
                
                if (!ParseExpression(state, &value)) encountered_errors = true;
                else
                {
                    *next_pointer = PushNode(state, AST_Assignment);
                    (*next_pointer)->assignment_statement.left  = name;
                    (*next_pointer)->assignment_statement.right = value;
                    (*next_pointer)->assignment_statement.kind  = AST_Invalid;
                }
            }
            
            else
            {
                *next_pointer = value;
            }
            
            if (!encountered_errors)
            {
                next_pointer = &(*next_pointer)->next;
                
                if (EatTokenOfKind(state, Token_Comma)) continue;
                else                                    break;
            }
        }
    }
    
    return !encountered_errors;
}

internal bool
ParseVarOrConstDeclaration(Parser_State* state, bool eat_multiple_values, AST_Node** decl)
{
    ASSERT(terminator == Token_Comma || terminator == Token_Semicolon);
    
    bool encountered_errors = false;
    
    bool is_const = false;
    
    AST_Node* names       = 0;
    AST_Node* type        = 0;
    AST_Node* values      = 0;
    bool is_using         = false;
    bool is_uninitialized = false;
    
    Token token = GetToken(state);
    if (token.kind == Token_Identifier && token.keyword == Keyword_Using)
    {
        is_using = true;
        SkipTokens(state, 1);
    }
    
    AST_Node** next_name = &names;
    
    while (!encountered_errors)
    {
        if (!ParseExpression(state, next_name)) encountered_errors = true;
        else
        {
            next_name = &(*next_name)->next;
            
            if (EatTokenOfKind(state, Token_Comma)) continue;
            else                                    break;
        }
    }
    
    if (!EatTokenOfKind(state, Token_Colon))
    {
        //// ERROR: Missing type of variable or constant
        encountered_errors = true;
    }
    
    else
    {
        token = GetToken(state);
        
        if (token.kind != Token_Colon && token.kind != Token_Equals)
        {
            if (!ParseExpression(state, &type))
            {
                encountered_errors = true;
            }
        }
        
        token = GetToken(state);
        if (token.kind == Token_Colon || token.kind == Token_Equals)
        {
            is_const = (token.kind == Token_Colon);
            
            SkipTokens(state, 1);
            
            AST_Node** next_value = &values;
            
            while (!encountered_errors)
            {
                if (!ParseExpression(state, next_value)) encountered_errors = true;
                else
                {
                    next_value = &(*next_value)->next;
                    
                    if (EatTokenOfKind(state, Token_Comma) && eat_multiple_values) continue;
                    else                                                           break;
                }
            }
        }
    }
    
    if (!encountered_errors)
    {
        *decl = PushNode(state, (is_const ? AST_ConstantDecl : AST_VariableDecl));
        (*decl)->var_decl.names            = names;
        (*decl)->var_decl.type             = type;
        (*decl)->var_decl.values           = values;
        (*decl)->var_decl.is_using         = is_using;
        (*decl)->var_decl.is_uninitialized = is_uninitialized;
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
        (*expression)->string = token.raw_string;
        
        SkipTokens(state, 1);
    }
    
    else if (token.kind == Token_Character)
    {
        *expression = PushNode(state, AST_Char);
        (*expression)->character = token.character;
        
        SkipTokens(state, 1);
    }
    
    else if (token.kind == Token_Number)
    {
        *expression = PushNode(state, AST_Number);
        (*expression)->number = token.number;
        
        SkipTokens(state, 1);
    }
    
    else if (token.kind == Token_Underscore)
    {
        *expression = PushNode(state, AST_Identifier);
        (*expression)->identifier = BLANK_IDENTIFIER;
        
        SkipTokens(state, 1);
    }
    
    else if (token.kind == Token_Identifier)
    {
        if (token.keyword == Keyword_True || token.keyword == Keyword_False)
        {
            *expression = PushNode(state, AST_Boolean);
            (*expression)->boolean = (token.keyword == Keyword_True);
            
            SkipTokens(state, 1);
        }
        
        else if (token.keyword == Keyword_Proc)
        {
            SkipTokens(state, 1);
            
            NOT_IMPLEMENTED;
        }
        
        else if (token.keyword == Keyword_Struct || token.keyword == Keyword_Union)
        {
            SkipTokens(state, 1);
            
            AST_Node* members = 0;
            bool is_decl      = false;
            
            if (EatTokenOfKind(state, Token_TripleMinus))
            {
                is_decl = true;
            }
            
            else if (!EatTokenOfKind(state, Token_OpenBrace))
            {
                //// ERROR: Missing body of struct
                encountered_errors = true;
            }
            
            else
            {
                if (EatTokenOfKind(state, Token_CloseBrace));
                else
                {
                    AST_Node** next_member = &members;
                    
                    while (!encountered_errors)
                    {
                        if (!ParseVarOrConstDeclaration(state, next_member)) encountered_errors = true;
                        else
                        {
                            next_member = &(*next_member)->next;
                            
                            if (!EatTokenOfKind(state, Token_Comma)) break;
                            else
                            {
                                if (GetToken(state).kind == Token_CloseBrace) break;
                                else                                          continue;
                            }
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        if (!EatTokenOfKind(state, Token_CloseBrace))
                        {
                            //// ERROR: Missing closing brace after struct members
                            encountered_errors = true;
                        }
                    }
                }
            }
            
            if (!encountered_errors)
            {
                *expression = PushNode(state, AST_Struct);
                (*expression)->struct_type.members = members;
                (*expression)->struct_type.is_decl = is_decl;
            }
        }
        
        else if (token.keyword == Keyword_Enum)
        {
            SkipTokens(state, 1);
            
            AST_Node* elem_type = 0;
            AST_Node* members   = 0;
            bool is_decl        = false;
            
            token = GetToken(state);
            if (token.kind != Token_TripleMinus && token.kind != Token_OpenBrace)
            {
                if (!ParseExpression(state, &elem_type))
                {
                    encountered_errors = true;
                }
            }
            
            if (EatTokenOfKind(state, Token_TripleMinus))
            {
                is_decl = true;
            }
            
            else if (!EatTokenOfKind(state, Token_OpenBrace))
            {
                //// ERROR: Missing body of enum
                encountered_errors = true;
            }
            
            else
            {
                if (EatTokenOfKind(state, Token_CloseBrace));
                else
                {
                    if (!ParseNamedValueList(state, &members))
                    {
                        encountered_errors = true;
                    }
                }
            }
            
            if (!encountered_errors)
            {
                *expression = PushNode(state, AST_Enum);
                (*expression)->struct_type.elem_type = elem_type;
                (*expression)->struct_type.members   = members;
                (*expression)->struct_type.is_decl   = is_decl;
            }
        }
        
        else
        {
            *expression = PushNode(state, AST_Identifier);
            (*expression)->identifier = ;
            
            SkipTokens(state, 1);
        }
    }
    
    else if (token.kind == Token_Period)
    {
        SkipTokens(state, 1);
        
        if (EatTokenOfKind(state, Token_OpenBrace))
        {
            AST_Node* params = 0;
            
            if (EatTokenOfKind(state, Token_CloseBrace)); // NOTE: allow name.{}
            else
            {
                if (!ParseNamedValueList(state, &params)) encountered_errors = true;
                else
                {
                    if (EatTokenOfKind(state, Token_CloseBrace))
                    {
                        //// ERROR: Missing matching closing brace
                        encountered_errors = true;
                    }
                }
            }
            
            if (!encountered_errors)
            {
                *expression = PushNode(state, AST_StructLiteral);
                (*expression)->struct_literal.type   = 0;
                (*expression)->struct_literal.params = params;
            }
        }
        
        else if (EatTokenOfKind(state, Token_OpenBracket))
        {
            AST_Node* params = 0;
            
            if (EatTokenOfKind(state, Token_CloseBracket)); // NOTE: allow name.[]
            else
            {
                if (!ParseNamedValueList(state, &params)) encountered_errors = true;
                else
                {
                    Token token = GetToken(state);
                    
                    if (!EatTokenOfKind(state, Token_CloseBracket))
                    {
                        //// ERROR: Missing matching closing bracket
                        encountered_errors = true;
                    }
                }
            }
            
            if (!encountered_errors)
            {
                *expression = PushNode(state, AST_ArrayLiteral);
                (*expression)->struct_literal.type   = 0;
                (*expression)->struct_literal.params = params;
            }
        }
        
        else
        {
            AST_Node* right = 0;
            
            if (!ParsePrefixExpression(state, &right)) encountered_errors = true;
            else
            {
                *expression = PushNode(state, AST_ElementOf);
                (*expression)->binary_expr.left  = 0;
                (*expression)->binary_expr.right = right;
            }
        }
        
    }
    
    else if (token.kind == Token_OpenParen)
    {
        SkipTokens(state, 1);
        
        if (!ParseExpression(state, expression)) encountered_errors = true;
        else
        {
            if (!EatTokenOfKind(state, Token_CloseParen))
            {
                //// ERROR: Missing matching closing parenthesis
                encountered_errors = true;
            }
        }
    }
    
    else if (token.kind == Token_Pound)
    {
        SkipTokens(state, 1);
        
        token = GetToken(state);
        
        if (token.kind != Token_Identifier)
        {
            //// ERROR: Missing name of directive
            encountered_errors = true;
        }
        
        else
        {
            SkipTokens(state, 1);
            
            if (EatTokenOfKind(state, Token_OpenParen))
            {
                if (EatTokenOfKind(state, Token_CloseParen)); // NOTE: allows #name()
                else
                {
                    if (!ParseNamedValueList(state, &params)) encountered_errors = true;
                    else
                    {
                        if (!EatTokenOfKind(state, Token_CloseParen))
                        {
                            //// ERROR: Missing closing parenthesis
                            encountered_errors = true;
                        }
                    }
                }
            }
            
            if (!encountered_errors)
            {
                *expression = PushNode(state, AST_Directive);
                (*expression)->directive.name   = token.identifier;
                (*expression)->directive.params = params;
            }
        }
    }
    
    else
    {
        //// ERROR: Missing primary expression
        encountered_errors = true;
    }
    
    return !encountered_errors;
}

internal bool
ParseTypeLevelExpression(Parser_State* state, AST_Node** expression)
{
    bool encountered_errors = false;
    
    if (EatTokenOfKind(state, Token_Hat))
    {
        *expression = PushNode(state, AST_PointerType);
        
        if (!ParseTypeLevelExpression(state, &(*expression)->unary_expr))
        {
            encountered_errors = true;
        }
    }
    
    else if (EatTokenOfKind(state, Token_OpenBracket))
    {
        AST_Node** type = 0;
        if (EatTokenOfKind(state, Token_CloseBracket))
        {
            *expression = PushNode(state, AST_SliceType);
            type = &(*expression)->unary_expr;
        }
        
        else if (GetToken(state).kind == Token_Elipsis && PeekToken(state, 1).kind == Token_CloseBracket)
        {
            SkipTokens(state, 2);
            
            *expression = PushNode(state, AST_DynamicArrayType);
            type = &(*expression)->unary_expr;
        }
        
        else
        {
            *expression = PushNode(state, AST_ArrayType);
            type = &(*expression)->array_type.elem_type;
            
            if (!ParseExpression(state, &(*expression)->array_type.size)) encountered_errors = true;
            else
            {
                if (!EatTokenOfKind(state, Token_CloseBracket))
                {
                    //// ERROR: Missing matching closing bracket
                    encountered_errors = true;
                }
            }
        }
        
        if (!encountered_errors)
        {
            if (!ParseTypeLevelExpression(state, type))
            {
                encountered_errors = true;
            }
        }
    }
    
    else
    {
        if (!ParsePrimaryExpression(state, expression))
        {
            encountered_errors = true;
        }
    }
    
    return !encountered_errors;
}

internal bool
ParsePostfixExpression(Parser_State* state, AST_Node** expression)
{
    bool encountered_errors = false;
    
    if (!ParseTypeLevelExpression(state, expression)) encountered_errors = true;
    else
    {
        while (!encountered_errors)
        {
            if (EatTokenOfKind(state, Token_OpenBracket))
            {
                AST_Node* array = *expression;
                AST_Node* index = 0;
                
                if (!(EatTokenOfKind(state, Token_Colon) || ParseExpression(state, &index))) encountered_errors = true;
                else
                {
                    Token token = GetToken(state);
                    
                    if (EatTokenOfKind(state, Token_CloseBracket))
                    {
                        if (index == 0)
                        {
                            *expression = PushNode(state, AST_Slice);
                            (*expression)->slice_expr.array = array;
                        }
                        
                        else
                        {
                            *expression = PushNode(state, AST_Subscript);
                            (*expression)->subscript_expr.array = array;
                            (*expression)->subscript_expr.index = index;
                        }
                    }
                    
                    else
                    {
                        AST_Node* one_after_end = 0;
                        
                        if (!ParseExpression(state, &one_after_end)) encountered_errors = true;
                        else
                        {
                            *expression = PushNode(state, AST_Slice);
                            (*expression)->slice_expr.array         = array;
                            (*expression)->slice_expr.start         = index;
                            (*expression)->slice_expr.one_after_end = one_after_end;
                            
                            if (!EatTokenOfKind(state, Token_CloseBracket))
                            {
                                //// ERROR: Missing matching closing bracket
                                encountered_errors = true;
                            }
                        }
                    }
                }
            }
            
            else if (EatTokenOfKind(state, Token_OpenParen))
            {
                AST_Node* func   = *expression;
                AST_Node* params = 0;
                
                if (EatTokenOfKind(state, Token_CloseParen)); // NOTE: allow func()
                else
                {
                    if (!ParseNamedValueList(state, &params)) encountered_errors = true;
                    else
                    {
                        if (!EatTokenOfKind(state, Token_CloseParen))
                        {
                            //// ERROR: Missing matching closing parenthesis
                            encountered_errors = true;
                        }
                    }
                }
                
                if (!encountered_errors)
                {
                    *expression = PushNode(state, AST_Call);
                    (*expression)->call_expr.func   = func;
                    (*expression)->call_expr.params = params;
                }
            }
            
            else if (EatTokenOfKind(state, Token_Period))
            {
                if (EatTokenOfKind(state, Token_OpenBrace))
                {
                    AST_Node* type   = *expression;
                    AST_Node* params = 0;
                    
                    if (EatTokenOfKind(state, Token_CloseBrace)); // NOTE: allow name.{}
                    else
                    {
                        if (!ParseNamedValueList(state, &params)) encountered_errors = true;
                        else
                        {
                            if (EatTokenOfKind(state, Token_CloseBrace))
                            {
                                //// ERROR: Missing matching closing brace
                                encountered_errors = true;
                            }
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        *expression = PushNode(state, AST_StructLiteral);
                        (*expression)->struct_literal.type   = type;
                        (*expression)->struct_literal.params = params;
                    }
                }
                
                else if (EatTokenOfKind(state, Token_OpenBracket))
                {
                    AST_Node* type   = *expression;
                    AST_Node* params = 0;
                    
                    
                    if (EatTokenOfKind(state, Token_CloseBracket)); // NOTE: allow name.[]
                    else
                    {
                        if (!ParseNamedValueList(state, &params)) encountered_errors = true;
                        else
                        {
                            Token token = GetToken(state);
                            
                            if (!EatTokenOfKind(state, Token_CloseBracket))
                            {
                                //// ERROR: Missing matching closing bracket
                                encountered_errors = true;
                            }
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        *expression = PushNode(state, AST_ArrayLiteral);
                        (*expression)->struct_literal.type   = type;
                        (*expression)->struct_literal.params = params;
                    }
                }
                
                else
                {
                    AST_Node* left  = *expression;
                    AST_Node* right = 0;
                    
                    if (!ParseTypeLevelExpression(state, &right)) encountered_errors = true;
                    else
                    {
                        *expression = PushNode(state, AST_ElementOf);
                        (*expression)->binary_expr.left  = left;
                        (*expression)->binary_expr.right = right;
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
    
    Token token = GetToken(state);
    
    if (EatTokenOfKind(state, Token_Plus)); // NOTE:
    else
    {
        u8 kind = AST_Invalid;
        
        switch (token.kind)
        {
            case Token_Minus:      kind = AST_Negation;        break;
            case Token_Complement: kind = AST_Complement;      break;
            case Token_Not:        kind = AST_Not;             break;
            case Token_And:        kind = AST_Reference;       break;
            case Token_Star:       kind = AST_Dereference;     break;
            case Token_Elipsis:    kind = AST_Spread;          break;
        }
        
        if (kind != AST_Invalid)
        {
            SkipTokens(state, 1);
            
            *expression = PushNode(state, kind);
            if (!ParsePrefixExpression(state, &(*expression)->unary_expr))
            {
                encountered_errors = true;
            }
        }
        
        else
        {
            if (!ParsePostfixExpression(state, expression))
            {
                encountered_errors = true;
            }
        }
    }
    
    return !encountered_errors;
    
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
            Token token = GetToken(state);
            
            // IMPORTANT NOTE: EXPRESSION_KIND is organized in blocks of values, each 20 in size
            //                 the blocks from 4 to 9 contain binary expressions
            
            u8 op = token.kind;
            umm precedence = op / 20;
            
            if (precedence < 4 || precedence > 9) break;
            else
            {
                SkipTokens(state, 1);
                
                AST_Node* right = 0;
                
                if (!ParsePrefixExpression(state, &right)) encountered_errors = true;
                else
                {
                    AST_Node** left = expression;
                    
                    for (;;)
                    {
                        if ((*left)->kind / 20 <= precedence)
                        {
                            *expression = PushNode(state, op);
                            (*expression)->binary_expr.left  = *left;
                            (*expression)->binary_expr.right = right;
                        }
                        
                        else
                        {
                            // NOTE: this assumes there are no statements in the current tree
                            left = &(*left)->binary_expr.right;
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
    
    // NOTE: conditional expressions have the highest precedence
    if (ParseBinaryExpression(state, expression))
    {
        if (EatTokenOfKind(state, Token_QuestionMark))
        {
            AST_Node* condition    = *expression;
            AST_Node* true_clause  = 0;
            AST_Node* false_clause = 0;
            
            if (!ParseBinaryExpression(state, &true_clause)) encountered_errors = true;
            else
            {
                if (!EatTokenOfKind(state, Token_Colon))
                {
                    //// ERROR: Missing else clause
                    NOT_IMPLEMENTED;
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
ParseFile(u8* file_contents)
{
    Parser_State state = {
        .lexer = Lexer_Init(file_contents),
    };
    
    
}