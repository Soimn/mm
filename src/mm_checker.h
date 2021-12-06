typedef struct Scope_Chain
{
    struct Scope_Chain* prev;
    struct Scope_Chain* next;
    Symbol_Table symbol_table;
} Scope_Chain;

typedef struct Checker_State
{
    Package_ID package;
    Scope_Chain* scope_chain;
} Checker_State;

internal bool CheckScope(Checker_State* state, AST_Node* scope);

internal bool
CheckExpression(Checker_State* state, AST_Node* expression)
{
    bool encountered_errors = false;
    
    umm precedence = PRECEDENCE_FROM_KIND(expression->kind);
    
    if (expression->kind == AST_Identifier)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_String)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Char)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Number)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Boolean)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_StructLiteral)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_ArrayLiteral)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Proc)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_ProcType)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Struct)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Union)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Enum)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Directive)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (precedence == 1 || precedence == 3)
    {
        /*
        AST_PointerType,
        AST_SliceType,
        AST_ArrayType,
        AST_DynamicArrayType,
        
        AST_Negation,
        AST_Complement,
        AST_Not,
        AST_Reference,
        AST_Dereference,
        AST_Spread,
        */
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Subscript)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Slice)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Call)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_ElementOf)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_UfcsOf)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (precedence >= 4 && precedence <= 9)
    {
        /*
        AST_ClosedRange,
        AST_HalfOpenRange,
        AST_Mul,
        AST_Div,
        AST_Rem,
        AST_BitwiseAnd,
        AST_ArithmeticRightShift,
        AST_RightShift,
        AST_LeftShift,
        AST_InfixCall,
        AST_Add,
        AST_Sub,
        AST_BitwiseOr,
        AST_BitwiseXor,
        AST_IsEqual,
        AST_IsNotEqual,
        AST_IsStrictlyLess,
        AST_IsStrictlyGreater,
        AST_IsLess,
        AST_IsGreater,
        AST_And,
        AST_Or,
        */
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Conditional)
    {
        
    }
    
    else INVALID_CODE_PATH;
    
    return !encountered_errors;
}

internal bool
CheckStatement(Checker_State* state, AST_Node* statement)
{
    bool encountered_errors = false;
    
    if (statement->kind == AST_When)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (statement->kind == AST_VariableDecl)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (statement->kind == AST_ConstantDecl)
    {
        NOT_IMPLEMENTED;
    }
    
    else
    {
        if (state->scope_chain->prev == 0)
        {
            if (statement->kind == AST_ImportDecl)
            {
                NOT_IMPLEMENTED;
            }
            
            else if (statement->kind == AST_ForeignDecl)
            {
                NOT_IMPLEMENTED;
            }
            
            else
            {
                //// ERROR: illegal use of statement in global scope
                encountered_errors = true;
            }
        }
        
        else
        {
            if (statement->kind == AST_ImportDecl)
            {
                //// ERROR: import declarations are only legal in global scope
                encountered_errors = true;
            }
            
            else if (statement->kind == AST_ForeignDecl)
            {
                //// ERROR: foreign import declarations are only legal in global scope
                encountered_errors = true;
            }
            
            else if (statement->kind == AST_Scope)
            {
                if (!CheckScope(state, statement))
                {
                    encountered_errors = true;
                }
            }
            
            else if (statement->kind == AST_If)
            {
                NOT_IMPLEMENTED;
            }
            
            else if (statement->kind == AST_While)
            {
                NOT_IMPLEMENTED;
            }
            
            else if (statement->kind == AST_Break)
            {
                NOT_IMPLEMENTED;
                // check if in loop
                // verify label
            }
            
            else if (statement->kind == AST_Continue)
            {
                NOT_IMPLEMENTED;
                // check if in loop
                // verify label
            }
            
            else if (statement->kind == AST_Defer)
            {
                if (!CheckStatement(state, statement->defer_statement))
                {
                    encountered_errors = true;
                }
            }
            
            else if (statement->kind == AST_Return)
            {
                NOT_IMPLEMENTED;
            }
            
            else if (statement->kind == AST_Using)
            {
                NOT_IMPLEMENTED;
            }
            
            else if (statement->kind == AST_Assignment)
            {
                NOT_IMPLEMENTED;
            }
            
            else
            {
                if (!CheckExpression(state, statement))
                {
                    encountered_errors = true;
                }
            }
        }
    }
    
    return !encountered_errors;
}

internal bool
CheckScope(Checker_State* state, AST_Node* scope)
{
    bool encountered_errors = false;
    
    Scope_Chain link = {
        .prev = state->scope_chain,
        .next = 0,
        .symbol_table = scope->scope_statement.symbol_table
    };
    
    state->scope_chain = &link;
    
    for (AST_Node* statement = scope->scope_statement.body;
         statement != 0 && !encountered_errors;
         statement = statement->next)
    {
        if (!CheckStatement(state, statement))
        {
            encountered_errors = true;
        }
    }
    
    state->scope_chain = link.prev;
    
    return !encountered_errors;
}

internal bool
CheckPackage(Package* package)
{
    bool encountered_errors = false;
    
    Checker_State state = { .package = package->id };
    
    Scope_Chain link = {
        .next         = 0,
        .symbol_table = package->symbol_table
    };
    
    state.scope_chain = &link;
    
    for (File* file = package->files;
         file != 0 && !encountered_errors;
         file = file->next)
    {
        for (AST_Node* statement = file->ast;
             statement != 0;
             statement = statement->next)
        {
            if (!CheckStatement(&state, statement))
            {
                encountered_errors = true;
                break;
            }
        }
    }
    
    return !encountered_errors;
}