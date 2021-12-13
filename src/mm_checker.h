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

typedef struct Check_Result
{
    Type_ID type;
    bool encountered_errors;
} Check_Result;

internal inline bool
Type_IsResolved(Type_ID type)
{
    return (type >= Type_ResolvedThreshold);
}

internal inline bool
Type_IsImplicitlyCovertibleToBool(Type_ID type)
{
    return (type >= Type_FirstBaseType && type <= Type_LastBaseType);
}

internal bool CheckScope(Checker_State* state, AST_Node* scope);

internal Check_Result
CheckExpression(Checker_State* state, AST_Node* expression)
{
    Check_Result result = {
        .type               = Type_Unresolved,
        .encountered_errors = false,
    };
    
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
        NOT_IMPLEMENTED;
    }
    
    else
    {
        //// ERROR: Invalid expression kind
        result.encountered_errors = true;
    }
    
    return result;
}

internal bool
CheckStatement(Checker_State* state, AST_Node* statement)
{
    bool encountered_errors = false;
    
    if (statement->kind == AST_When)
    {
        AST_Node* init      = statement->when_statement.init;
        AST_Node* condition = statement->when_statement.condition;
        
        if (init != 0)
        {
            // TODO:
            //// ERROR: When init statements are not yet supported
            encountered_errors = true;
        }
        
        if (!encountered_errors)
        {
            if (condition == 0)
            {
                //// ERROR: Missing when statement condition
                encountered_errors = true;
            }
            
            else if (!IS_EXPRESSION(condition->kind))
            {
                //// ERROR: When statement condition is not an expression
                encountered_errors = true;
            }
            
            else
            {
                NOT_IMPLEMENTED;
            }
        }
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
                // TODO: Validate path
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
                AST_Node* init      = statement->if_statement.init;
                AST_Node* condition = statement->if_statement.condition;
                
                if (init != 0)
                {
                    if (init->kind == AST_Call)
                    {
                        Check_Result result = CheckExpression(state, init);
                        
                        if (result.encountered_errors) encountered_errors = true;
                        else
                        {
                            NOT_IMPLEMENTED;
                            if (has_return_values)
                            {
                                //// ERROR: Return values
                                encountered_errors = true;
                            }
                        }
                    }
                    
                    else if (init->kind == AST_VariableDecl || init->kind == AST_ConstantDecl || init->kind == AST_Assignment)
                    {
                        
                    }
                    
                    else
                    {
                        //// ERROR: Invalid init statement
                        encountered_errors = true;
                    }
                }
                
                if (!encountered_errors)
                {
                    if (condition == 0)
                    {
                        //// ERROR: Missing if statement condition
                        encountered_errors = true;
                    }
                    
                    else if (!IS_EXPRESSION(condition->kind))
                    {
                        //// ERROR: If statement condition is not an expression
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        Check_Result result = CheckExpression(state, condition);
                        
                        if (result.encountered_errors) encountered_errors = true;
                        else
                        {
                            if (Type_IsResolved(result.type) && !Type_IsImplicitlyCovertibleToBool(result.type))
                            {
                                //// ERROR: If statement condition cannot be implicitly convertible to bool
                                encountered_errors = true;
                            }
                        }
                    }
                }
                
                if (!encountered_errors)
                {
                    if (statement->if_statement.true_body == 0)
                    {
                        //// ERROR: Missing body of if statement
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        CheckScope(statement->if_statement.true_body);
                        if (statement->if_statement.false_body != 0) CheckStatement(statement->if_statement.false_body);
                    }
                }
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
                Check_Result result = CheckExpression(state, statement);
                
                if (result.encountered_errors)
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
CheckPackage(Package_ID package_id)
{
    bool encountered_errors = false;
    
    Package* package = Package_FromID(package_id);
    
    Checker_State state = { .package = package_id };
    
    Scope_Chain link = {
        .next         = 0,
        .symbol_table = package->symbol_table
    };
    
    state.scope_chain = &link;
    
    for (umm i = 0; i < package->file_count; ++i)
    {
        File* file = &package->files[i];
        
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