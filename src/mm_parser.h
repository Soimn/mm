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
    AST_Node** next = list;
    while (true)
    {
        AST_Node* entry = PushNode(parser, AST_NamedValue);
        *next = entry;
        next  = &(*next)->next;
        
        if (!ParseExpression(parser, &entry->named_value.value)) return false;
        
        if (EatToken(parser, Token_Equals))
        {
            entry->named_value.name = entry->named_value.value;
            if (!ParseExpression(parser, &entry->named_value.value)) return false;
        }
        
        if (EatToken(parser, Token_Comma)) continue;
        else                               break;
    }
    
    return true;
}

internal bool
ParseParameterList(Parser* parser, AST_Node** params)
{
    AST_Node** next_param = params;
    while (true)
    {
        AST_Node* name  = 0;
        AST_Node* type  = 0;
        AST_Node* value = 0;
        bool is_using = false;
        
        Token token = GetToken(parser);
        if (token.kind == Token_Identifier && token.string == Keyword_Using)
        {
            NextToken(parser);
            is_using = true;
        }
        
        if (!ParseExpression(parser, &type)) return false;
        
        if (EatToken(parser, Token_Colon))
        {
            name = type;
            type = 0;
            if (GetToken(parser).kind != Token_Equals && !ParseExpression(parser, &type)) return false;
        }
        
        if (EatToken(parser, Token_Equals) && !ParseExpression(parser, &type)) return false;
        
        *next_param = PushNode(parser, AST_Parameter);
        (*next_param)->parameter.name     = name;
        (*next_param)->parameter.type     = type;
        (*next_param)->parameter.value    = value;
        (*next_param)->parameter.is_using = is_using;
        
        next_param = &(*next_param)->next;
        
        if (EatToken(parser, Token_Comma)) continue;
        else                               break;
    }
    
    return true;
}

internal bool
ParseAssignmentDeclarationOrExpression(Parser* parser, AST_Node** statement)
{
    bool is_using = false;
    
    Token token = GetToken(parser);
    if (token.kind == Token_Identifier && token.string == Keyword_Using)
    {
        NextToken(parser);
        is_using = true;
    }
    
    AST_Node* expressions      = 0;
    AST_Node** next_expression = &expressions;
    while (true)
    {
        if (!ParseExpression(parser, next_expression)) return false;
        
        next_expression = &(*next_expression)->next;
        
        if (EatToken(parser, Token_Comma)) continue;
        else                               break;
    }
    
    if (EatToken(parser, Token_Colon))
    {
        bool is_const = false;
        AST_Node* names       = expressions;
        AST_Node* type        = 0;
        AST_Node* values      = 0;
        bool is_uninitialized = false;
        
        token = GetToken(parser);
        if (token.kind != Token_Equals && token.kind != Token_Colon)
        {
            if (!ParseExpression(parser, &type)) return false;
            
            if (GetToken(parser).kind == Token_Comma)
            {
                //// ERROR: Multiple types are illegal
                return false;
            }
        }
        
        token = GetToken(parser);
        if (token.kind == Token_Equals || token.kind == Token_Colon)
        {
            is_const = (token.kind == Token_Colon);
            NextToken(parser);
            
            if (EatToken(parser, Token_TripleMinus))
            {
                if (!is_const) is_uninitialized = true;
                else
                {
                    //// ERROR: Constants must be initialized
                    return false;
                }
            }
            else
            {
                next_expression = &values;
                while (true)
                {
                    if (!ParseExpression(parser, next_expression)) return false;
                    
                    next_expression = &(*next_expression)->next;
                    
                    if (EatToken(parser, Token_Comma)) continue;
                    else                               break;
                }
            }
        }
        
        if (is_const)
        {
            *statement = PushNode(parser, AST_Constant);
            (*statement)->constant_decl.names    = names;
            (*statement)->constant_decl.type     = type;
            (*statement)->constant_decl.values   = values;
            (*statement)->constant_decl.is_using = is_using;
        }
        else
        {
            *statement = PushNode(parser, AST_Variable);
            (*statement)->variable_decl.names            = names;
            (*statement)->variable_decl.type             = type;
            (*statement)->variable_decl.values           = values;
            (*statement)->variable_decl.is_using         = is_using;
            (*statement)->variable_decl.is_uninitialized = is_uninitialized;
        }
    }
    else
    {
        token = GetToken(parser);
        
        if (token.kind >= Token_FirstAssignment && token.kind <= Token_LastAssignment)
        {
            AST_NODE_KIND op = (token.kind == Token_Equals ? AST_Nil : token.kind + (Token_FirstMulLevel - Token_FirstMulLevelAssignment));
            AST_Node* lhs = expressions;
            AST_Node* rhs = 0;
            
            NextToken(parser);
            
            next_expression = &rhs;
            while (true)
            {
                if (!ParseExpression(parser, next_expression)) return false;
                
                next_expression = &(*next_expression)->next;
                
                if (EatToken(parser, Token_Comma)) continue;
                else                               break;
            }
            
            *statement = PushNode(parser, AST_Assignment);
            (*statement)->assignment_statement.lhs = lhs;
            (*statement)->assignment_statement.rhs = rhs;
            (*statement)->assignment_statement.op  = op;
        }
        else if (is_using)
        {
            AST_Node* symbol_paths = expressions;
            Interned_String alias  = INTERNED_STRING_NIL;
            
            if (token.kind == Token_Identifier && token.string == Keyword_As)
            {
                token = NextToken(parser);
                
                if (token.kind != Token_Identifier)
                {
                    //// ERROR: Missing name of alias after 'as' keyword in using declaration
                    return false;
                }
                else if (InternedString_IsKeyword(token.string))
                {
                    //// ERROR: Illegal use of keyword as alias
                    return false;
                }
                else if (token.string == BLANK_IDENTIFIER)
                {
                    //// ERROR: Illegal use of keyword as alias
                    return false;
                }
                else if (symbol_paths->next != 0)
                {
                    //// ERROR: Cannot alias multiple symbols under the same alias
                    return false;
                }
                
                alias = token.string;
                NextToken(parser);
            }
            
            *statement = PushNode(parser, AST_Using);
            (*statement)->using_decl.symbol_paths = symbol_paths;
            (*statement)->using_decl.alias        = alias;
        }
        else
        {
            if (expressions->next != 0)
            {
                //// ERROR: A list of expressions cannot be used by itself
                return false;
            }
            
            *statement = expressions;
        }
    }
    
    return true;
}

internal bool
ParsePrimaryExpression(Parser* parser, AST_Node** expression)
{
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
                    if      (!ParseParameterList(parser, &params)) return false;
                    else if (!EatToken(parser, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren after parameters
                        return false;
                    }
                }
                
                if (EatToken(parser, Token_Arrow))
                {
                    bool is_multi = EatToken(parser, Token_OpenParen);
                    
                    AST_Node** next_type = &return_types;
                    while (true)
                    {
                        AST_Node* first;
                        if (!ParseExpression(parser, &first)) return false;
                        
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
                            if (!ParseExpression(parser, &value)) return false;
                        }
                        
                        *next_type = PushNode(parser, AST_NamedValue);
                        
                        if (is_multi && EatToken(parser, Token_Comma)) continue;
                        else                                           break;
                    }
                    
                    if (is_multi && !EatToken(parser, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren after return type list
                        return false;
                    }
                }
                
                token = GetToken(parser);
                if (token.kind == Token_Identifier && token.string == Keyword_Where)
                {
                    NextToken(parser);
                    if (!ParseExpression(parser, &where_clause)) return false;
                }
                
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
                    
                    if (!EatToken(parser, Token_TripleMinus) && !ParseBlock(parser, &body)) return false;
                    
                    *expression = PushNode(parser, AST_ProcLiteral);
                    (*expression)->proc_literal.params       = params;
                    (*expression)->proc_literal.return_types = return_types;
                    (*expression)->proc_literal.where_clause = where_clause;
                    (*expression)->proc_literal.body         = body;
                }
            }
            else if (token.string == Keyword_Struct || token.string == Keyword_Union)
            {
                bool is_union    = (token.string == Keyword_Union);
                AST_Node* params = 0;
                AST_Node* body   = 0;
                
                if (EatToken(parser, Token_OpenParen))
                {
                    if      (!ParseParameterList(parser, &params)) return false;
                    else if (!EatToken(parser, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren after parameter list
                        return false;
                    }
                }
                
                if (!EatToken(parser, Token_OpenBrace))
                {
                    //// ERROR: Missing body of X
                    return false;
                }
                else
                {
                    AST_Node** next_statement = &body;
                    while (!EatToken(parser, Token_CloseBrace))
                    {
                        if (!ParseStatement(parser, next_statement)) return false;
                        if (*next_statement) next_statement = &(*next_statement)->next;
                    }
                }
                
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
            else if (token.string == Keyword_Enum)
            {
                token = NextToken(parser);
                
                AST_Node* type = 0;
                AST_Node* body = 0;
                
                if (token.kind != Token_OpenBrace && !ParseExpression(parser, &type)) return false;
                else if (!EatToken(parser, Token_OpenBrace))
                {
                    //// ERROR: Missing body of enum
                    return false;
                }
                
                if      (!ParseNamedValueList(parser, &body)) return false;
                else if (!EatToken(parser, Token_CloseBrace))
                {
                    //// ERROR: Missing closing brace after body of enum
                    return false;
                }
                
                *expression = PushNode(parser, AST_Enum);
                (*expression)->enum_type.type = type;
                (*expression)->enum_type.body = body;
            }
            else if (token.string == Keyword_Cast || token.string == Keyword_Transmute)
            {
                Interned_String proc = token.string;
                
                NextToken(parser);
                if (!EatToken(parser, Token_OpenParen) || GetToken(parser).kind == Token_CloseParen)
                {
                    //// ERROR: Missing arguments to cast
                    return false;
                }
                
                AST_Node* args = 0;
                if (!ParseNamedValueList(parser, &args)) return false;
                
                *expression = PushNode(parser, AST_IntrinsicCall);
                (*expression)->intrinsic_call_expr.proc = proc;
                (*expression)->intrinsic_call_expr.args = args;
            }
            else
            {
                //// ERROR: Illegal use of keyword in expression
                return false;
            }
        }
    }
    else if (EatToken(parser, Token_PeriodOpenBrace))
    {
        AST_Node* args = 0;
        
        if (!EatToken(parser, Token_CloseBrace))
        {
            if      (!ParseNamedValueList(parser, &args)) return false;
            else if (!EatToken(parser, Token_CloseBrace))
            {
                //// ERROR: Missing closing brace after arguments to struct literal
                return false;
            }
        }
        
        *expression = PushNode(parser, AST_StructLiteral);
        (*expression)->struct_literal.type = 0;
        (*expression)->struct_literal.args = args;
    }
    else if (EatToken(parser, Token_PeriodOpenBracket))
    {
        AST_Node* args = 0;
        
        if (!EatToken(parser, Token_CloseBracket))
        {
            if      (!ParseNamedValueList(parser, &args)) return false;
            else if (!EatToken(parser, Token_CloseBracket))
            {
                //// ERROR: Missing closing bracket after arguments to array literal
                return false;
            }
        }
        
        *expression = PushNode(parser, AST_ArrayLiteral);
        (*expression)->array_literal.type = 0;
        (*expression)->array_literal.args = args;
    }
    else if (EatToken(parser, Token_OpenParen))
    {
        AST_Node* compound;
        if      (!ParseExpression(parser, &compound)) return false;
        else if (!EatToken(parser, Token_CloseParen))
        {
            //// ERROR: Missing closing paren after compound expression body
            return false;
        }
        
        *expression = PushNode(parser, AST_Compound);
        (*expression)->compound_expr = compound;
    }
    else if (EatToken(parser, Token_Period))
    {
        token = GetToken(parser);
        if (token.kind != Token_Identifier)
        {
            //// ERROR: Missing name of enum in enum selector expression
            return false;
        }
        
        Interned_String enum_name = token.string;
        NextToken(parser);
        
        *expression = PushNode(parser, AST_Selector);
        (*expression)->selector_expr = enum_name;
    }
    else if (EatToken(parser, Token_Cash))
    {
        token = GetToken(parser);
        if (token.kind != Token_Identifier)
        {
            //// ERROR: Missing name of polymorphic value
            return false;
        }
        
        Interned_String poly_name = token.string;
        NextToken(parser);
        
        *expression = PushNode(parser, AST_PolymorphicName);
        (*expression)->poly_name = poly_name;
    }
    else
    {
        //// ERROR: Missing primary expression
        return false;
    }
    
    return true;
}

internal bool
ParsePostfixExpression(Parser* parser, AST_Node** expression)
{
    if (!ParsePrimaryExpression(parser, expression)) return false;
    else
    {
        while (true)
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
                    return false;
                }
                else if (!InternedString_IsNonBlankIdentifier(token.string))
                {
                    //// ERROR: Member name must be a non blank identifier
                    return false;
                }
                
                NextToken(parser);
                
                *expression = PushNode(parser, AST_Member);
                (*expression)->member_expr.expr   = expr;
                (*expression)->member_expr.member = token.string;
            }
            else if (EatToken(parser, Token_OpenBracket))
            {
                AST_Node* array = *expression;
                AST_Node* first = 0;
                
                if (GetToken(parser).kind != Token_Colon && !ParseExpression(parser, &first)) return false;
                
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
                    
                    if (GetToken(parser).kind != Token_CloseBracket && !ParseExpression(parser, &past_end)) return false;
                    
                    *expression = PushNode(parser, AST_Slice);
                    (*expression)->slice_expr.array    = array;
                    (*expression)->slice_expr.start    = start;
                    (*expression)->slice_expr.past_end = past_end;
                }
                
                if (!EatToken(parser, Token_CloseBracket))
                {
                    //// ERROR: Missing closing bracket after x
                    return false;
                }
            }
            else if (EatToken(parser, Token_OpenParen))
            {
                AST_Node* proc = *expression;
                AST_Node* args = 0;
                
                if (!EatToken(parser, Token_CloseParen))
                {
                    if      (!ParseNamedValueList(parser, &args)) return false;
                    else if (!EatToken(parser, Token_CloseParen))
                    {
                        //// ERROR: Missing closing paren after arguments to call expr
                        return false;
                    }
                }
                
                *expression = PushNode(parser, AST_Call);
                (*expression)->call_expr.proc = proc;
                (*expression)->call_expr.args = args;
            }
            else if (EatToken(parser, Token_PeriodOpenBrace))
            {
                AST_Node* type = *expression;
                AST_Node* args = 0;
                
                if (!EatToken(parser, Token_CloseBrace))
                {
                    if      (!ParseNamedValueList(parser, &args)) return false;
                    else if (!EatToken(parser, Token_CloseBrace))
                    {
                        //// ERROR: Missing closing brace after arguments to struct literal
                        return false;
                    }
                }
                
                *expression = PushNode(parser, AST_StructLiteral);
                (*expression)->struct_literal.type = type;
                (*expression)->struct_literal.args = args;
            }
            else if (EatToken(parser, Token_PeriodOpenBracket))
            {
                AST_Node* type = *expression;
                AST_Node* args = 0;
                
                if (!EatToken(parser, Token_CloseBracket))
                {
                    if      (!ParseNamedValueList(parser, &args)) return false;
                    else if (!EatToken(parser, Token_CloseBracket))
                    {
                        //// ERROR: Missing closing bracket after arguments to array literal
                        return false;
                    }
                }
                
                *expression = PushNode(parser, AST_ArrayLiteral);
                (*expression)->array_literal.type = type;
                (*expression)->array_literal.args = args;
            }
            else break;
        }
    }
    
    return true;
}

internal bool
ParsePrefixExpression(Parser* parser, AST_Node** expression)
{
    while (true)
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
                if      (ParseExpression(parser, &size)) return false;
                else if (!EatToken(parser, Token_CloseBracket))
                {
                    //// ERROR: Missing closing bracket after size in array type qualifier
                    return false;
                }
                
                AST_Node* array_type = PushNode(parser, AST_ArrayType);
                array_type->array_type.size = size;
                expression                  = &array_type->array_type.elem_type;
            }
        }
        else return ParsePostfixExpression(parser, expression);
    }
    
    return true;
}

internal bool
ParseBinaryExpression(Parser* parser, AST_Node** expression)
{
    if (!ParsePrefixExpression(parser, expression)) return false;
    else
    {
        while (true)
        {
            Token token = GetToken(parser);
            if (token.kind < Token_FirstBinary || token.kind > Token_LastBinary) break;
            else                                                                 NextToken(parser);
            
            AST_Node** left = expression;
            AST_Node* right;
            
            if (!ParsePrefixExpression(parser, &right)) return false;
            
            umm precedence = token.kind >> 4;
            
            while ((*left)->kind >> 4 > precedence) left = &(*left)->binary_expr.right;
            
            AST_Node* new_expression = PushNode(parser, token.kind);
            new_expression->binary_expr.left  = *left;
            new_expression->binary_expr.right = right;
            
            *left = new_expression;
        }
    }
    
    return true;
}

internal bool
ParseExpression(Parser* parser, AST_Node** expression)
{
    if      (!ParseBinaryExpression(parser, expression)) return false;
    else if (EatToken(parser, Token_QuestionMark))
    {
        AST_Node* condition  = *expression;
        AST_Node* true_expr;
        AST_Node* false_expr;
        
        if      (!ParseBinaryExpression(parser, &true_expr)) return false;
        else if (!EatToken(parser, Token_Colon))
        {
            //// ERROR: Missing false expr
            return false;
        }
        else if (!ParseBinaryExpression(parser, &false_expr)) return false;
        
        *expression = PushNode(parser, AST_Conditional);
        (*expression)->conditional_expr.condition  = condition;
        (*expression)->conditional_expr.true_expr  = true_expr;
        (*expression)->conditional_expr.false_expr = false_expr;
    }
    
    return true;
}

internal bool
ParseStatement(Parser* parser, AST_Node** statement)
{
    Token token     = GetToken(parser);
    Token peek      = PeekToken(parser, 1);
    Token peek_next = PeekToken(parser, 2);
    
    bool token_is_identifier = (token.kind == Token_Identifier);
    
    bool should_check_for_semicolon = true;
    
    if (token.kind == Token_Semicolon);
    else if (token.kind == Token_OpenBrace)
    {
        should_check_for_semicolon = false;
        
        if (!ParseBlock(parser, statement)) return false;
    }
    else if (token_is_identifier && peek.kind == Token_Colon && (peek_next.kind == Token_OpenBrace ||
                                                                 peek_next.kind == Token_Identifier && (peek_next.string == Keyword_If || peek_next.string == Keyword_While ||
                                                                                                        peek_next.string == Keyword_Else)))
    {
        if (InternedString_IsKeyword(token.string))
        {
            //// ERROR: Illegal use of keyword as label name
            return false;
        }
        else if (peek_next.kind == Token_Identifier && peek_next.string == Keyword_Else)
        {
            //// ERROR: Illegal use of label on else statement
            return false;
        }
        
        Interned_String label = token.string;
        
        NextToken(parser);
        NextToken(parser);
        
        if (!ParseStatement(parser, statement)) return false;
        
        if      ((*statement)->kind == AST_Block) (*statement)->block_statement.label = label, should_check_for_semicolon = false;
        else if ((*statement)->kind == AST_If)    (*statement)->if_statement.label    = label;
        else if ((*statement)->kind == AST_While) (*statement)->while_statement.label = label;
        else INVALID_CODE_PATH;
    }
    else if (token_is_identifier && token.string == Keyword_If)
    {
        should_check_for_semicolon = false;
        
        NextToken(parser);
        
        if (!EatToken(parser, Token_OpenParen))
        {
            //// ERROR: Missing condition of if statement
            return false;
        }
        
        AST_Node* init       = 0;
        AST_Node* condition  = 0;
        AST_Node* true_body  = 0;
        AST_Node* false_body = 0;
        
        token = GetToken(parser);
        if (token.kind != Token_Semicolon && !ParseAssignmentDeclarationOrExpression(parser, &condition)) return false;
        else if (EatToken(parser, Token_Semicolon))
        {
            init = condition;
            if (!ParseAssignmentDeclarationOrExpression(parser, &condition)) return false;
        }
        
        if (!EatToken(parser, Token_CloseParen))
        {
            //// ERROR: Missing closing paren after if condition
            return false;
        }
        
        if (!ParseStatement(parser, &true_body)) return false;
        
        token = GetToken(parser);
        if (token.kind == Token_Identifier && token.string == Keyword_Else)
        {
            NextToken(parser);
            if (!ParseStatement(parser, &false_body)) return false;
        }
        
        *statement = PushNode(parser, AST_If);
        (*statement)->if_statement.init       = init;
        (*statement)->if_statement.condition  = condition;
        (*statement)->if_statement.true_body  = true_body;
        (*statement)->if_statement.false_body = false_body;
        (*statement)->if_statement.label      = INTERNED_STRING_NIL;
    }
    else if (token_is_identifier && token.string == Keyword_Else)
    {
        //// ERROR: Illegal else without matching if
        return false;
    }
    else if (token_is_identifier && token.string == Keyword_While)
    {
        should_check_for_semicolon = false;
        
        NextToken(parser);
        
        if (!EatToken(parser, Token_OpenParen))
        {
            //// ERROR: Missing condition of while statement
            return false;
        }
        
        AST_Node* init      = 0;
        AST_Node* condition = 0;
        AST_Node* step      = 0;
        AST_Node* body      = 0;
        
        token = GetToken(parser);
        if (token.kind != Token_Semicolon && !ParseAssignmentDeclarationOrExpression(parser, &condition)) return false;
        else if (EatToken(parser, Token_Semicolon))
        {
            init = condition;
            
            if      (!ParseAssignmentDeclarationOrExpression(parser, &condition))                                 return false;
            else if (EatToken(parser, Token_Semicolon) && !ParseAssignmentDeclarationOrExpression(parser, &step)) return false;
        }
        
        if (!EatToken(parser, Token_CloseParen))
        {
            //// ERROR: Missing closing paren after while header
            return false;
        }
        else if (!ParseStatement(parser, &body)) return false;
        
        *statement = PushNode(parser, AST_While);
        (*statement)->while_statement.init      = init;
        (*statement)->while_statement.condition = condition;
        (*statement)->while_statement.step      = step;
        (*statement)->while_statement.body      = body;
        (*statement)->while_statement.label     = INTERNED_STRING_NIL;
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
                return false;
            }
            
            label = token.string;
            NextToken(parser);
        }
        
        *statement = PushNode(parser, kind);
        (*statement)->jump_label = label;
    }
    else if (token_is_identifier && token.string == Keyword_Defer)
    {
        NextToken(parser);
        
        AST_Node* defer_statement = 0;
        if (!ParseStatement(parser, &defer_statement)) return false;
        
        *statement = PushNode(parser, AST_Defer);
        (*statement)->defer_statement = defer_statement;
    }
    else if (token_is_identifier && token.string == Keyword_Return)
    {
        token = NextToken(parser);
        
        AST_Node* return_values = 0;
        if (token.kind != Token_Semicolon && !ParseNamedValueList(parser, &return_values)) return false;
        
        *statement = PushNode(parser, AST_Return);
        (*statement)->return_values = return_values;
    }
    else
    {
        if (!ParseAssignmentDeclarationOrExpression(parser, statement)) return false;
        
        if ((*statement)->kind == AST_Constant && (*statement)->constant_decl.names->next == 0 && (*statement)->constant_decl.values->next == 0 &&
            ((*statement)->constant_decl.values->kind == AST_ProcLiteral  || (*statement)->constant_decl.values->kind == AST_Struct ||
             (*statement)->constant_decl.values->kind == AST_Union        || (*statement)->constant_decl.values->kind == AST_Enum))
        {
            should_check_for_semicolon = false;
        }
    }
    
    if (should_check_for_semicolon && !EatToken(parser, Token_Semicolon))
    {
        //// ERROR: Missing terminating semicolon after statement
        return false;
    }
    
    return true;
}


internal bool
ParseBlock(Parser* parser, AST_Node** block)
{
    bool is_block = EatToken(parser, Token_OpenBrace);
    ASSERT(is_block);
    
    AST_Node* body = 0;
    
    AST_Node** next_statement = &body;
    while (!EatToken(parser, Token_CloseBrace))
    {
        if (!ParseStatement(parser, next_statement)) return false;
    }
    
    *block = PushNode(parser, AST_Block);
    (*block)->block_statement.label = INTERNED_STRING_NIL;
    (*block)->block_statement.body  = body;
    
    return true;
}