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
internal bool ParseTypeLevelExpression(Parser_State* state, AST_Node** expression);
internal bool ParseScope(Parser_State* state, AST_Node** scope);
internal bool ParseStatement(Parser_State* state, AST_Node** next_statement);

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
ParseUsingExpressionAssignmentVarOrConstDecl(Parser_State* state, bool eat_multiple_values, AST_Node** node)
{
    bool encountered_errors = false;
    
    bool is_using   = false;
    AST_Node* expressions = 0;
    
    Token token = GetToken(state);
    if (token.kind == Token_Identifier && token.keyword == Keyword_Using)
    {
        is_using = true;
        SkipTokens(state, 1);
    }
    
    AST_Node** next_expression = &expressions;
    
    while (!encountered_errors)
    {
        if (!ParseExpression(state, next_expression)) encountered_errors = true;
        else
        {
            next_expression = &(*next_expression)->next;
            
            if (EatTokenOfKind(state, Token_Comma)) continue;
            else                                    break;
        }
    }
    
    if (!encountered_errors)
    {
        token = GetToken(state);
        
        if (token.kind >= Token_FirstAssignment && token.kind <= Token_LastAssignment)
        {
            AST_Node* left  = expressions;
            AST_Node* right = 0;
            u8 kind         = AST_Invalid;
            
            if      (token.kind == Token_AndAndEquals) kind = AST_And;
            else if (token.kind == Token_OrOrEquals)   kind = AST_Or;
            else if (token.kind >= Token_StarEquals && token.kind <= Token_LeftShiftEquals) kind = AST_FirstMulLevel + (token.kind - Token_StarEquals);
            else if (token.kind >= Token_PlusEquals && token.kind <= Token_HatEquals)       kind = AST_FirstAddLevel + (token.kind - Token_PlusEquals);
            
            next_expression = &right;
            
            while (!encountered_errors)
            {
                if (!ParseExpression(state, next_expression)) encountered_errors = true;
                else
                {
                    next_expression = &(*next_expression)->next;
                    
                    if (EatTokenOfKind(state, Token_Comma)) continue;
                    else                                    break;
                }
            }
            
            if (!encountered_errors)
            {
                *node = PushNode(state, AST_Assignment);
                (*node)->assignment_statement.left  = left;
                (*node)->assignment_statement.right = right;
                (*node)->assignment_statement.kind  = kind;
            }
        }
        
        else if (token.kind == Token_Colon)
        {
            bool is_const = false;
            
            AST_Node* names       = expressions;
            AST_Node* type        = 0;
            AST_Node* values      = 0;
            bool is_uninitialized = false;
            
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
            
            if (!encountered_errors)
            {
                *node = PushNode(state, (is_const ? AST_ConstantDecl : AST_VariableDecl));
                (*node)->var_decl.names            = names;
                (*node)->var_decl.type             = type;
                (*node)->var_decl.values           = values;
                (*node)->var_decl.is_using         = is_using;
                (*node)->var_decl.is_uninitialized = is_uninitialized;
            }
        }
        
        else
        {
            if (is_using)
            {
                *node = PushNode(state, AST_Using);
                (*node)->using_statement = expressions;
            }
            
            else
            {
                *node = expressions;
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
        (*expression)->string = token.raw_string;
        
        SkipTokens(state, 1);
    }
    
    else if (token.kind == Token_Character)
    {
        *expression = PushNode(state, AST_Char);
        //(*expression)->character = token.character;
        NOT_IMPLEMENTED;
        // TODO: parse character literal
        
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
        //(*expression)->identifier = BLANK_IDENTIFIER;
        NOT_IMPLEMENTED;
        // TODO: define BLANK_IDENTIFIER
        
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
            
            AST_Node* params        = 0;
            AST_Node* return_values = 0;
            AST_Node* body          = 0;
            
            if (EatTokenOfKind(state, Token_OpenParen))
            {
                if (EatTokenOfKind(state, Token_CloseParen)); // NOTE: allow proc()
                else
                {
                    AST_Node** next_param = &params;
                    
                    while (!encountered_errors)
                    {
                        if (!ParseUsingExpressionAssignmentVarOrConstDecl(state, false, next_param)) encountered_errors = true;
                        else
                        {
                            next_param = &(*next_param)->next;
                            
                            if (EatTokenOfKind(state, Token_Comma)) continue;
                            else                                    break;
                        }
                    }
                    
                    if (!encountered_errors)
                    {
                        if (!EatTokenOfKind(state, Token_CloseParen))
                        {
                            //// ERROR: Missing closing paren after proc params
                            encountered_errors = true;
                        }
                    }
                }
            }
            
            if (!encountered_errors)
            {
                if (EatTokenOfKind(state, Token_Arrow))
                {
                    if (EatTokenOfKind(state, Token_OpenParen))
                    {
                        if (EatTokenOfKind(state, Token_CloseParen)); // NOTE: allow -> ()
                        else
                        {
                            if (!ParseNamedValueList(state, &return_values)) encountered_errors = true;
                            else
                            {
                                if (!EatTokenOfKind(state, Token_CloseParen))
                                {
                                    //// ERROR: Missing matching closing paren after return values
                                    encountered_errors = true;
                                }
                            }
                        }
                    }
                    
                    else
                    {
                        if (!ParseExpression(state, &return_values))
                        {
                            encountered_errors = true;
                        }
                    }
                }
            }
            
            if (!encountered_errors)
            {
                if (EatTokenOfKind(state, Token_TripleMinus))
                {
                    *expression = PushNode(state, AST_Proc);
                    (*expression)->proc_literal.params        = params;
                    (*expression)->proc_literal.return_values = return_values;
                    (*expression)->proc_literal.body          = 0;
                }
                
                else if (GetToken(state).kind == Token_OpenBrace)
                {
                    NOT_IMPLEMENTED;
                    
                    *expression = PushNode(state, AST_Proc);
                    (*expression)->proc_literal.params        = params;
                    (*expression)->proc_literal.return_values = return_values;
                    (*expression)->proc_literal.body          = body;
                }
                
                else
                {
                    *expression = PushNode(state, AST_ProcType);
                    (*expression)->proc_type.params        = params;
                    (*expression)->proc_type.return_values = return_values;
                }
            }
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
                        if (!ParseUsingExpressionAssignmentVarOrConstDecl(state, false, next_member)) encountered_errors = true;
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
                if (EatTokenOfKind(state, Token_CloseBrace)); // NOTE: allow enum type {}
                else
                {
                    if (!ParseNamedValueList(state, &members)) encountered_errors = true;
                    else
                    {
                        if (!EatTokenOfKind(state, Token_CloseBrace))
                        {
                            //// ERROR: Missing closing brace after enum members
                            encountered_errors = true;
                        }
                    }
                }
            }
            
            if (!encountered_errors)
            {
                *expression = PushNode(state, AST_Enum);
                (*expression)->enum_type.elem_type = elem_type;
                (*expression)->enum_type.members   = members;
                (*expression)->enum_type.is_decl   = is_decl;
            }
        }
        
        else
        {
            *expression = PushNode(state, AST_Identifier);
            //(*expression)->identifier = ;
            NOT_IMPLEMENTED;
            // TODO: hash identifier strings
            
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
                    token = GetToken(state);
                    
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
            
            if (!ParseTypeLevelExpression(state, &right)) encountered_errors = true;
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
            Identifier name = {0};
            AST_Node* params = 0;
            
            NOT_IMPLEMENTED;
            // TODO: hash identifier
            
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
                (*expression)->directive.name   = name;
                (*expression)->directive.params = params;
            }
        }
    }
    
    else
    {
        // NOTE: invalid tokens will force the parser to end up here
        //       if the token is valid we are missing an expression
        //       if the token is invalid a lexer error has happened somewhere
        //       Token_EndOfStream will also end up here if not caught by the main loop
        if (token.kind != Token_Invalid)
        {
            //// ERROR: Missing primary expression
            encountered_errors = true;
        }
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
            
            else if (EatTokenOfKind(state, Token_Arrow))
            {
                AST_Node* left  = *expression;
                AST_Node* right = 0;
                
                if (!ParseTypeLevelExpression(state, &right)) encountered_errors = true;
                else
                {
                    *expression = PushNode(state, AST_UfcsOf);
                    (*expression)->binary_expr.left  = left;
                    (*expression)->binary_expr.right = right;
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
ParseScope(Parser_State* state, AST_Node** scope)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    bool is_do_block = false;
    
    if (token.kind == Token_Identifier && token.keyword == Keyword_Do)
    {
        SkipTokens(state, 1);
        is_do_block = true;
    }
    
    else if (!EatTokenOfKind(state, Token_OpenBrace))
    {
        INVALID_CODE_PATH;
    }
    
    AST_Node* body            = 0;
    AST_Node** next_statement = &body;
    while (!encountered_errors)
    {
        if (EatTokenOfKind(state, Token_CloseBrace))
        {
            if (is_do_block)
            {
                //// ERROR: Missing statement
                encountered_errors = true;
            }
            
            else break;
        }
        
        else
        {
            if (!ParseStatement(state, next_statement)) encountered_errors = true;
            else
            {
                next_statement = &(*next_statement)->next;
            }
        }
        
        if (is_do_block) break;
        else             continue;
    }
    
    *scope = PushNode(state, AST_Scope);
    //(*scope)->scope_statement.label = BLANK_IDENTIFIER;
    (*scope)->scope_statement.body  = body;
    
    return !encountered_errors;
}

internal bool
ParseStatement(Parser_State* state, AST_Node** next_statement)
{
    bool encountered_errors = false;
    
    if (EatTokenOfKind(state, Token_Semicolon)); // NOTE: allow loose ;
    else
    {
        Token token      = GetToken(state);
        Token peek       = PeekToken(state, 1);
        Token peek_next  = PeekToken(state, 2);
        
        if (token.kind == Token_OpenBrace ||
            token.kind == Token_Identifier && peek.kind == Token_Colon && peek_next.kind == Token_OpenBrace)
        {
            //Identifier label = BLANK_IDENTIFIER;
            
            if (token.kind == Token_Identifier)
            {
                //label = ;
                // hash token.identifier
                NOT_IMPLEMENTED;
                
                SkipTokens(state, 2);
            }
            
            if (!ParseScope(state, next_statement)) encountered_errors = true;
            else
            {
                // (*next_statement)->scope_statement.label = label;
            }
        }
        
        else if (token.kind == Token_Identifier && (token.keyword == Keyword_If || token.keyword == Keyword_When || token.keyword == Keyword_While) ||
                 token.kind == Token_Identifier && peek.kind == Token_Colon &&
                 peek_next.kind == Token_Identifier && (peek_next.keyword == Keyword_If || peek_next.keyword == Keyword_When || peek_next.keyword == Keyword_While))
        {
            //Identifier label = BLANK_IDENTIFIER;
            AST_Node* init      = 0;
            AST_Node* condition = 0;
            AST_Node* step      = 0;
            AST_Node* body      = 0;
            
            if (token.kind == Token_Identifier && !(token.keyword == Keyword_If || token.keyword == Keyword_When || token.kind == Keyword_While))
            {
                //label = ;
                // hash token.identifier
                NOT_IMPLEMENTED;
                
                SkipTokens(state, 2);
            }
            
            token = GetToken(state);
            ASSERT(token.kind == Token_Identifier);
            
            bool is_when  = (token.keyword == Keyword_When);
            bool is_while = (token.keyword == Keyword_While);
            
            SkipTokens(state, 1);
            
            NOT_IMPLEMENTED;
            
            if (!ParseUsingExpressionAssignmentVarOrConstDecl(state, true, &init))
            {
                encountered_errors = true;
            }
            
            if (!encountered_errors)
            {
                if (EatTokenOfKind(state, Token_Semicolon))
                {
                    if (!ParseUsingExpressionAssignmentVarOrConstDecl(state, true, &condition))
                    {
                        encountered_errors = true;
                    }
                }
            }
            
            if (!encountered_errors && is_while)
            {
                if (EatTokenOfKind(state, Token_Semicolon))
                {
                    if (!ParseUsingExpressionAssignmentVarOrConstDecl(state, true, &step))
                    {
                        encountered_errors = true;
                    }
                }
            }
            
            if (!encountered_errors)
            {
                if (!ParseScope(state, &body))
                {
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                if (is_while)
                {
                    *next_statement = PushNode(state, AST_While);
                    NOT_IMPLEMENTED;
                }
                
                else
                {
                    NOT_IMPLEMENTED;
                }
            }
        }
        
        else if (token.kind == Token_Identifier && (token.keyword == Keyword_Break || token.keyword == Keyword_Continue))
        {
            bool is_break = (token.keyword == Keyword_Break);
            
            SkipTokens(state, 1);
            
            //Identifier label = BLANK_IDENTIFIER;
            
            if (EatTokenOfKind(state, Token_Semicolon)); // NOTE: allow break; and continue;
            else
            {
                token = GetToken(state);
                if (token.kind != Token_Identifier)
                {
                    //// ERROR: Missing label after break/continue
                    encountered_errors = true;
                }
                
                else
                {
                    // TODO: hash identifier
                    NOT_IMPLEMENTED;
                    
                    if (!EatTokenOfKind(state, Token_Semicolon))
                    {
                        //// ERROR: Missing semicolon after break/continue statement
                        encountered_errors = true;
                    }
                }
            }
            
            *next_statement = PushNode(state, (is_break ? AST_Break : AST_Continue));
            //(*next_statement)->break_statement.label = label;
        }
        
        else if (token.kind == Token_Identifier && token.keyword == Keyword_Defer)
        {
            SkipTokens(state, 1);
            
            AST_Node* statement = 0;
            
            if (EatTokenOfKind(state, Token_Semicolon)); // NOTE: allow defer;
            else
            {
                if (!ParseScope(state, &statement))
                {
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                *next_statement = PushNode(state, AST_Defer);
                (*next_statement)->defer_statement = statement;
            }
        }
        
        else if (token.kind == Token_Identifier && token.keyword == Keyword_Return)
        {
            SkipTokens(state, 1);
            
            AST_Node* values = 0;
            
            if (EatTokenOfKind(state, Token_Semicolon)); // NOTE: allow return;
            else
            {
                if (!ParseNamedValueList(state, &values)) encountered_errors = true;
                else
                {
                    if (!EatTokenOfKind(state, Token_Semicolon))
                    {
                        //// ERROR: Missing terminating semicolon after return statement
                        encountered_errors = true;
                    }
                }
            }
            
            if (!encountered_errors)
            {
                *next_statement = PushNode(state, AST_Return);
                (*next_statement)->return_statement.values = values;
            }
        }
        
        else
        {
            if (!ParseUsingExpressionAssignmentVarOrConstDecl(state, true, next_statement))encountered_errors = true;
            else
            {
                if (!EatTokenOfKind(state, Token_Semicolon))
                {
                    if ((*next_statement)->kind == AST_Proc   ||
                        (*next_statement)->kind == AST_Struct ||
                        (*next_statement)->kind == AST_Enum   ||
                        (((*next_statement)->kind == AST_VariableDecl || (*next_statement)->kind == AST_ConstantDecl) &&
                         (*next_statement)->var_decl.values != 0 && (*next_statement)->var_decl.values->next == 0 &&
                         ((*next_statement)->var_decl.values->kind == AST_Proc   ||
                          (*next_statement)->var_decl.values->kind == AST_Struct ||
                          (*next_statement)->var_decl.values->kind == AST_Enum)))
                    {
                        // NOTE: Ignore missing semicolon
                    }
                    
                    else
                    {
                        //// ERROR: Missing terminating semicolon after statement
                        encountered_errors = true;
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
    bool encountered_errors = false;
    
    Parser_State state = {
        .lexer = Lexer_Init(file_contents),
    };
    
    AST_Node** next_statement;
    
    while (!encountered_errors)
    {
        if (GetToken(&state).kind == Token_EndOfStream) break;
        else
        {
            if (!ParseStatement(&state, next_statement))
            {
                encountered_errors = true;
            }
        }
    }
    
    return !encountered_errors;
}