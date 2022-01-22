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
    AST_Node* result = Arena_PushSize(&MM.ast_arena, sizeof(AST_Node), ALIGNOF(AST_Node));
    ZeroStruct(result);
    
    result->kind = kind;
    
    return result;
}

// NOTE: cursor is assumed to be in the range [0, string.size), and the string is assumed to not end in a backslash
internal bool
ParseCharacter(String string, umm* cursor, Character* character)
{
    bool encountered_errors = false;
    
    ZeroStruct(character);
    
    if (string.data[*cursor] == '\\')
    {
        *cursor += 1;
        
        if (string.data[*cursor] == 'u' || string.data[*cursor] == 'x')
        {
            umm req_digit_count = (string.data[*cursor] == 'u' ? 6 : 2);
            
            umm codepoint = 0;
            
            for (umm digit_count = 0; digit_count < req_digit_count; ++digit_count, ++*cursor)
            {
                if (*cursor < string.size && string.data[*cursor] >= '0' && string.data[*cursor] <= '9')
                {
                    codepoint *= 16;
                    codepoint += string.data[*cursor] - '0';
                }
                
                else if (*cursor < string.size && ToUpperCase(string.data[*cursor]) >= 'A' && ToUpperCase(string.data[*cursor]) <= 'F')
                {
                    codepoint *= 16;
                    codepoint += (ToUpperCase(string.data[*cursor]) - 'A') + 10;
                }
                
                else
                {
                    //// ERROR: missing digits in codepoint/byte escape sequence
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                if (codepoint > 0x10FFFF)
                {
                    //// ERROR: Unicode codepoint out of UTF-8 range
                    encountered_errors = true;
                }
                
                else *character = (Character)codepoint;
            }
        }
        
        else
        {
            ASSERT(*cursor < string.size);
            
            switch (string.data[*cursor])
            {
                case '\"': *character = '\"'; break;
                case '\'': *character = '\''; break;
                case '\\': *character = '\\'; break;
                
                case 'a': *character = '\a'; break;
                case 'b': *character = '\b'; break;
                case 'f': *character = '\f'; break;
                case 'n': *character = '\n'; break;
                case 'r': *character = '\r'; break;
                case 't': *character = '\t'; break;
                case 'v': *character = '\v'; break;
                
                default:
                {
                    //// ERROR: Unknown escape sequence
                    encountered_errors = true;
                } break;
            }
            
            if (!encountered_errors) *cursor += 1;
        }
    }
    
    else
    {
        *character = string.data[*cursor];
        *cursor += 1;
    }
    
    return !encountered_errors;
}

internal bool
ParseStringLiteral(Parser_State* state, String raw_string, Interned_String* result)
{
    bool encountered_errors = false;
    
    Memory_Arena_Marker marker = Arena_BeginTempMemory(&MM.temp_arena);
    
    String string = {
        .data = Arena_PushSize(&MM.temp_arena, raw_string.size, 1),
        .size = 0
    };
    
    umm cursor = 0;
    while (!encountered_errors && cursor < raw_string.size)
    {
        Character character;
        if (!ParseCharacter(raw_string, &cursor, &character)) encountered_errors = true;
        else
        {
            UTF8_Word word = Character_ToUTF8Word(character);
            
            umm advance = 4;
            
            for (; advance > 1; --advance)
            {
                if (word.bytes[advance - 1] != 0) break;
            }
            
            Copy(word.bytes, string.data + string.size, advance);
            string.size += advance;
        }
    }
    
    *result = String_Intern(string);
    
    Arena_EndTempMemory(&MM.temp_arena, marker);
    
    return !encountered_errors;
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
    if (token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Using))
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
        
        else if (EatTokenOfKind(state, Token_Colon))
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
                (*node)->using_statement.expression = expressions;
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
        
        if (!ParseStringLiteral(state, token.raw_string, &(*expression)->string)) encountered_errors = true;
        else
        {
            SkipTokens(state, 1);
        }
    }
    
    else if (token.kind == Token_Character)
    {
        *expression = PushNode(state, AST_Char);
        
        umm cursor = 0;
        if (!ParseCharacter(token.raw_string, &cursor, &(*expression)->character) || token.raw_string.size != cursor) encountered_errors = true;
        {
            SkipTokens(state, 1);
        }
    }
    
    else if (token.kind == Token_Int)
    {
        *expression = PushNode(state, AST_Int);
        (*expression)->integer = token.integer;
        
        SkipTokens(state, 1);
    }
    
    else if (token.kind == Token_Float)
    {
        *expression = PushNode(state, AST_Float);
        (*expression)->floating = token.floating;
        
        SkipTokens(state, 1);
    }
    
    else if (token.kind == Token_Identifier)
    {
        if (String_IsKeyword(token.identifier, Keyword_True) || String_IsKeyword(token.identifier, Keyword_False))
        {
            *expression = PushNode(state, AST_Boolean);
            (*expression)->boolean = String_IsKeyword(token.identifier, Keyword_True);
            
            SkipTokens(state, 1);
        }
        
        else if (String_IsKeyword(token.identifier, Keyword_Proc))
        {
            SkipTokens(state, 1);
            
            AST_Node* params        = 0;
            AST_Node* return_values = 0;
            
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
                
                else
                {
                    token = GetToken(state);
                    if (token.kind == Token_OpenBrace || token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Do))
                    {
                        AST_Node* body = 0;
                        
                        if (!ParseScope(state, &body)) encountered_errors = true;
                        else
                        {
                            *expression = PushNode(state, AST_Proc);
                            (*expression)->proc_literal.params        = params;
                            (*expression)->proc_literal.return_values = return_values;
                            (*expression)->proc_literal.body          = body;
                        }
                    }
                    
                    else
                    {
                        *expression = PushNode(state, AST_ProcType);
                        (*expression)->proc_type.params        = params;
                        (*expression)->proc_type.return_values = return_values;
                    }
                }
            }
        }
        
        else if (String_IsKeyword(token.identifier, Keyword_Struct) || String_IsKeyword(token.identifier, Keyword_Union))
        {
            SkipTokens(state, 1);
            
            AST_Node* members = 0;
            
            if (!EatTokenOfKind(state, Token_OpenBrace))
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
            }
        }
        
        else if (String_IsKeyword(token.identifier, Keyword_Enum))
        {
            SkipTokens(state, 1);
            
            AST_Node* elem_type = 0;
            AST_Node* members   = 0;
            
            token = GetToken(state);
            if (token.kind != Token_TripleMinus && token.kind != Token_OpenBrace)
            {
                if (!ParseExpression(state, &elem_type))
                {
                    encountered_errors = true;
                }
            }
            
            if (!EatTokenOfKind(state, Token_OpenBrace))
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
            }
        }
        
        else
        {
            *expression = PushNode(state, AST_Identifier);
            (*expression)->identifier = token.identifier;
            
            SkipTokens(state, 1);
        }
    }
    
    else if (EatTokenOfKind(state, Token_Period))
    {
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
    
    else if (EatTokenOfKind(state, Token_OpenParen))
    {
        if (!ParseExpression(state, expression)) encountered_errors = true;
        else
        {
            if (PRECEDENCE_FROM_KIND((*expression)->kind) >= 4 && PRECEDENCE_FROM_KIND((*expression)->kind) <= 9)
            {
                // NOTE: leave "is_compound" note to avoid precedence fixing ripping apart compound
                //       expressions
                (*expression)->binary_expr.is_compound = true;
            }
            
            if (!EatTokenOfKind(state, Token_CloseParen))
            {
                //// ERROR: Missing matching closing parenthesis
                encountered_errors = true;
            }
        }
    }
    
    else if (EatTokenOfKind(state, Token_Pound))
    {
        token = GetToken(state);
        
        if (token.kind != Token_Identifier)
        {
            //// ERROR: Missing name of directive
            encountered_errors = true;
        }
        
        else
        {
            Interned_String name = token.identifier;
            AST_Node* params = 0;
            
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
        
        Lexer tmp_lexer = state->lexer;
        Token peek = Lexer_Advance(&tmp_lexer);
        
        if (EatTokenOfKind(state, Token_CloseBracket))
        {
            *expression = PushNode(state, AST_SliceType);
            type = &(*expression)->unary_expr;
        }
        
        else if (GetToken(state).kind == Token_Elipsis && peek.kind == Token_CloseBracket)
        {
            *expression = PushNode(state, AST_DynArrayType);
            type = &(*expression)->unary_expr;
            
            if (!EatTokenOfKind(state, Token_CloseBracket))
            {
                //// ERROR: Missing closing bracket in dynamic array type specifier
                encountered_errors = true;
            }
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
            
            u8 op = token.kind;
            umm precedence = PRECEDENCE_FROM_KIND(op);
            
            if (precedence <= 3 || precedence >= 10) break;
            else
            {
                SkipTokens(state, 1);
                
                AST_Node* right = 0;
                
                if (!ParsePrefixExpression(state, &right)) encountered_errors = true;
                else
                {
                    AST_Node** slot = expression;
                    
                    for (;;)
                    {
                        // NOTE: Without compensating for compound expressions, this would just be
                        //       if (PRECEDENCE_FROM_KIND((*slot)->kind) <= precedence)
                        //       However, since there is no AST_Compound node, something has to
                        //       prevent this loop from ripping apart () in expressions such as
                        //       (a + b) * c
                        //       This is done by checking if the expression is binary and a "is_compound"
                        //       note was left by ParsePrimaryExpression, or that it would be impossible
                        //       to find an expression of that precedence level without (), e.g.
                        //       conditional expressions (which have precedence 10)
                        //       The is binary check is uses short circuiting of ||
                        if (PRECEDENCE_FROM_KIND((*slot)->kind) <= precedence ||
                            PRECEDENCE_FROM_KIND((*slot)->kind) > 9           ||
                            PRECEDENCE_FROM_KIND((*slot)->kind) >= 4 && (*slot)->binary_expr.is_compound)
                        {
                            AST_Node* left = *slot;
                            
                            *expression = PushNode(state, op);
                            (*expression)->binary_expr.left  = left;
                            (*expression)->binary_expr.right = right;
                            break;
                        }
                        
                        else slot = &(*slot)->binary_expr.right;
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
ParseScopeBody(Parser_State* state, AST_Node** body, bool* is_do_block)
{
    bool encountered_errors = false;
    
    Token token = GetToken(state);
    
    *is_do_block = false;
    if (token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Do))
    {
        SkipTokens(state, 1);
        *is_do_block = true;
    }
    
    else if (!EatTokenOfKind(state, Token_OpenBrace))
    {
        INVALID_CODE_PATH;
    }
    
    *body                     = 0;
    AST_Node** next_statement = body;
    while (!encountered_errors)
    {
        if (EatTokenOfKind(state, Token_CloseBrace))
        {
            if (*is_do_block)
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
        
        if (*is_do_block) break;
        else              continue;
    }
    
    return !encountered_errors;
}

internal bool
ParseScope(Parser_State* state, AST_Node** scope)
{
    bool encountered_errors = false;
    
    *scope = PushNode(state, AST_Scope);
    (*scope)->scope_statement.label = BLANK_IDENTIFIER;
    
    if (!ParseScopeBody(state, &(*scope)->scope_statement.body, &(*scope)->scope_statement.is_do))
    {
        encountered_errors = true;
    }
    
    return !encountered_errors;
}

internal bool
IsIfWhenOrWhile(Token token, Token peek, Token peek_next)
{
    bool result = false;
    
    if (token.kind == Token_Identifier)
    {
        Interned_String check_ident = token.identifier;
        
        if (String_IsKeyword(token.identifier, Keyword_Invalid) && peek.kind == Token_Colon)
        {
            check_ident = peek_next.identifier;
        }
        
        result = (String_IsKeyword(check_ident, Keyword_If)   ||
                  String_IsKeyword(check_ident, Keyword_When) ||
                  String_IsKeyword(check_ident, Keyword_While));
    }
    
    return result;
}

internal bool
ParseStatement(Parser_State* state, AST_Node** next_statement)
{
    bool encountered_errors = false;
    
    if (EatTokenOfKind(state, Token_Semicolon)); // NOTE: allow loose ;
    else
    {
        Token token     = GetToken(state);
        Token peek      = {0};
        Token peek_next = {0};
        
        if (token.kind == Token_Identifier)
        {
            Lexer tmp_lexer = state->lexer;
            
            peek = Lexer_Advance(&tmp_lexer);
            
            if (peek.kind == Token_Colon)
            {
                peek_next = Lexer_Advance(&tmp_lexer);
            }
        }
        
        if (token.kind == Token_OpenBrace ||
            (token.kind == Token_Identifier                          &&
             String_IsKeyword(token.identifier, Keyword_Invalid) &&
             peek.kind == Token_Colon                                &&
             peek_next.kind == Token_OpenBrace))
        {
            Interned_String label = BLANK_IDENTIFIER;
            
            if (token.kind == Token_Identifier)
            {
                label = token.identifier;
                
                SkipTokens(state, 2);
            }
            
            if (!ParseScope(state, next_statement)) encountered_errors = true;
            else
            {
                (*next_statement)->scope_statement.label = label;
            }
        }
        
        else if (token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Include))
        {
            SkipTokens(state, 1);
            token = GetToken(state);
            
            if (token.kind != Token_String)
            {
                //// ERROR: Missing include file path
                encountered_errors = true;
            }
            
            else
            {
                SkipTokens(state, 1);
                
                *next_statement = PushNode(state, AST_IncludeDecl);
                
                if (!ParseStringLiteral(state, token.raw_string, &(*next_statement)->include.path))
                {
                    encountered_errors = true;
                }
            }
        }
        
        else if (token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Else))
        {
            //// ERROR: Encountered else without matching if
            encountered_errors = true;
        }
        
        else if (IsIfWhenOrWhile(token, peek, peek_next))
        {
            AST_Node* first  = 0;
            AST_Node* second = 0;
            AST_Node* third  = 0;
            
            if (String_IsKeyword(token.identifier, Keyword_Invalid))
            {
                *next_statement = PushNode(state, AST_Scope);
                (*next_statement)->scope_statement.label = token.identifier;
                (*next_statement)->scope_statement.is_do = false;
                
                next_statement = &(*next_statement)->scope_statement.body;
                
                SkipTokens(state, 2);
            }
            
            token = GetToken(state);
            ASSERT(token.kind == Token_Identifier);
            
            bool is_when  = String_IsKeyword(token.identifier, Keyword_When);
            bool is_while = String_IsKeyword(token.identifier, Keyword_While);
            
            SkipTokens(state, 1);
            
            if (GetToken(state).kind != Token_Semicolon)
            {
                if (!ParseUsingExpressionAssignmentVarOrConstDecl(state, true, &first))
                {
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors && !is_when)
            {
                if (EatTokenOfKind(state, Token_Semicolon))
                {
                    if (GetToken(state).kind == Token_Semicolon && is_while && first != 0);
                    else
                    {
                        if (!ParseUsingExpressionAssignmentVarOrConstDecl(state, true, &second))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    if (!encountered_errors && is_while)
                    {
                        if (EatTokenOfKind(state, Token_Semicolon))
                        {
                            if (!ParseUsingExpressionAssignmentVarOrConstDecl(state, true, &third))
                            {
                                encountered_errors = true;
                            }
                        }
                    }
                }
            }
            
            if (!encountered_errors)
            {
                if (is_while)
                {
                    AST_Node* body;
                    if (!ParseScope(state, &body)) encountered_errors = true;
                    else
                    {
                        *next_statement = PushNode(state, AST_While);
                        
                        if (first && !second && !third)
                        {
                            (*next_statement)->while_statement.init      = 0;
                            (*next_statement)->while_statement.condition = first;
                            (*next_statement)->while_statement.step      = 0;
                        }
                        
                        else
                        {
                            (*next_statement)->while_statement.init      = first;
                            (*next_statement)->while_statement.condition = second;
                            (*next_statement)->while_statement.step      = third;
                        }
                    }
                }
                
                else if (is_when)
                {
                    AST_Node* true_body  = 0;
                    AST_Node* false_body = 0;
                    
                    bool ignored;
                    if (!ParseScopeBody(state, &true_body, &ignored)) encountered_errors = true;
                    else
                    {
                        token = GetToken(state);
                        if (token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Else))
                        {
                            SkipTokens(state, 1);
                            token = GetToken(state);
                            
                            if (token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_When))
                            {
                                if (!ParseStatement(state, &false_body))
                                {
                                    encountered_errors = true;
                                }
                            }
                            
                            else if (token.kind == Token_OpenBrace || token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Do))
                            {
                                if (!ParseScopeBody(state, &false_body, &ignored))
                                {
                                    encountered_errors = true;
                                }
                            }
                            
                            else
                            {
                                //// ERROR: Missing body of else statement
                                encountered_errors = true;
                            }
                        }
                        
                        if (!encountered_errors)
                        {
                            *next_statement = PushNode(state, AST_When);
                            (*next_statement)->when_statement.condition  = first;
                            (*next_statement)->when_statement.true_body  = true_body;
                            (*next_statement)->when_statement.false_body = false_body;
                        }
                    }
                }
                
                else
                {
                    AST_Node* true_body  = 0;
                    AST_Node* false_body = 0;
                    
                    if (!ParseScope(state, &true_body)) encountered_errors = true;
                    else
                    {
                        token = GetToken(state);
                        if (token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Else))
                        {
                            SkipTokens(state, 1);
                            token = GetToken(state);
                            
                            if (token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_If))
                            {
                                if (!ParseStatement(state, &false_body))
                                {
                                    encountered_errors = true;
                                }
                            }
                            
                            else if (token.kind == Token_OpenBrace || token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Do))
                            {
                                if (!ParseScope(state, &false_body))
                                {
                                    encountered_errors = true;
                                }
                            }
                            
                            else
                            {
                                //// ERROR: Missing body of else statement
                                encountered_errors = true;
                            }
                        }
                        
                        if (!encountered_errors)
                        {
                            *next_statement = PushNode(state, AST_If);
                            
                            if (first && !second)
                            {
                                (*next_statement)->if_statement.init      = 0;
                                (*next_statement)->if_statement.condition = first;
                            }
                            
                            else
                            {
                                (*next_statement)->if_statement.init      = first;
                                (*next_statement)->if_statement.condition = second;
                            }
                            
                            (*next_statement)->if_statement.true_body  = true_body;
                            (*next_statement)->if_statement.false_body = false_body;
                        }
                    }
                }
            }
        }
        
        else if (token.kind == Token_Identifier && (String_IsKeyword(token.identifier, Keyword_Break) ||
                                                    String_IsKeyword(token.identifier, Keyword_Continue)))
        {
            bool is_break = String_IsKeyword(token.identifier, Keyword_Break);
            
            SkipTokens(state, 1);
            
            Interned_String label = BLANK_IDENTIFIER;
            
            if (GetToken(state).kind == Token_Semicolon); // NOTE: allow break; and continue;
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
                    label = token.identifier;
                }
            }
            
            if (!encountered_errors)
            {
                if (!EatTokenOfKind(state, Token_Semicolon))
                {
                    //// ERROR: Missing terminating semicolon after statement
                    encountered_errors = true;
                }
                
                else
                {
                    *next_statement = PushNode(state, (is_break ? AST_Break : AST_Continue));
                    (*next_statement)->break_statement.label = label;
                }
            }
        }
        
        else if (token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Defer))
        {
            SkipTokens(state, 1);
            
            AST_Node* statement = 0;
            
            if (!ParseStatement(state, &statement))
            {
                *next_statement = PushNode(state, AST_Defer);
                (*next_statement)->defer_statement = statement;
            }
        }
        
        else if (token.kind == Token_Identifier && String_IsKeyword(token.identifier, Keyword_Return))
        {
            SkipTokens(state, 1);
            
            AST_Node* values = 0;
            
            if (GetToken(state).kind == Token_Semicolon); // NOTE: allow return;
            else
            {
                if (!ParseNamedValueList(state, &values))
                {
                    encountered_errors = true;
                }
            }
            
            if (!encountered_errors)
            {
                if (!EatTokenOfKind(state, Token_Semicolon))
                {
                    //// ERROR: Missing terminating semicolon after statement
                    encountered_errors = true;
                }
                
                else
                {
                    *next_statement = PushNode(state, AST_Return);
                    (*next_statement)->return_statement.values = values;
                }
            }
        }
        
        else
        {
            if (!ParseUsingExpressionAssignmentVarOrConstDecl(state, true, next_statement)) encountered_errors = true;
            else
            {
                if (!EatTokenOfKind(state, Token_Semicolon))
                {
                    if (((*next_statement)->kind == AST_VariableDecl || (*next_statement)->kind == AST_ConstantDecl) &&
                        (*next_statement)->var_decl.values != 0 && (*next_statement)->var_decl.values->next == 0 &&
                        ((*next_statement)->var_decl.values->kind == AST_Proc   ||
                         (*next_statement)->var_decl.values->kind == AST_Struct ||
                         (*next_statement)->var_decl.values->kind == AST_Union  ||
                         (*next_statement)->var_decl.values->kind == AST_Enum))
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
ParseFile(File* file)
{
    bool encountered_errors = false;
    
    Parser_State state = {
        .lexer = Lexer_Init(file->contents.data),
    };
    
    // NOTE: initialize state.current_token
    SkipTokens(&state, 1);
    
    AST_Node** next_statement = (MM.ast == 0 ? &MM.ast : &MM.ast_last_node->next);
    
    while (!encountered_errors)
    {
        if (GetToken(&state).kind == Token_EndOfStream) break;
        else
        {
            if (!ParseStatement(&state, next_statement)) encountered_errors = true;
            else
            {
                if (next_statement != 0)
                {
                    MM.ast_last_node = *next_statement;
                    next_statement          = &(*next_statement)->next;
                }
            }
        }
    }
    
    return !encountered_errors;
}