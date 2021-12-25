typedef struct Scope_Chain
{
    struct Scope_Chain* prev;
    struct Scope_Chain* next;
    Symbol_Table* symbol_table;
} Scope_Chain;

typedef struct Checker_State
{
    Package_ID package;
    Scope_Chain* scope_chain;
} Checker_State;

typedef struct Check_Result
{
    Type_ID type;
} Check_Result;

#define UNRESOLVED_CHECK_RESULT (Check_Result){ .type = Type_Unresolved }

internal inline bool
Type_IsIntegral(Type_ID type)
{
    return (type >= Type_Int && type <= Type_U64);
}

internal inline bool
Type_IsSigned(Type_ID type)
{
    return (type >= Type_Int && type <= Type_I64);
}

internal inline bool
Type_IsFloating(Type_ID type)
{
    return (type >= Type_Float && type <= Type_F64);
}

internal inline bool
Type_IsNumeric(Type_ID type)
{
    return (type >= Type_Int && type <= Type_F64);
}

internal inline Type_Info*
TypeInfo_FromID(Type_ID type)
{
    Type_Info* info = 0;
    
    NOT_IMPLEMENTED;
    
    return info;
}

internal bool CheckScope(Checker_State* state, AST_Node* scope);

internal Check_Result
CheckExpression(Checker_State* state, AST_Node* expression)
{
    Check_Result result = { .type = Type_Erroneous };
    
    umm precedence = PRECEDENCE_FROM_KIND(expression->kind);
    
    if (expression->kind == AST_Identifier)
    {
        Symbol* symbol = 0;
        NOT_IMPLEMENTED;
        
        if (symbol == 0) result = UNRESOLVED_CHECK_RESULT;
        else
        {
            result = (Check_Result){
                .type = symbol->type,
            };
        }
    }
    
    else if (expression->kind == AST_String)
    {
        result = (Check_Result){
            .type = Type_String,
        };
    }
    
    else if (expression->kind == AST_Char)
    {
        result = (Check_Result){
            .type = Type_Char,
        };
    }
    
    else if (expression->kind == AST_Number)
    {
        result = (Check_Result){
            .type = (expression->number.is_float ? Type_Float : Type_Int),
        };
    }
    
    else if (expression->kind == AST_Boolean)
    {
        result = (Check_Result){
            .type = Type_Bool,
        };
    }
    
    else if (expression->kind == AST_StructLiteral)
    {
        // TODO:
        //// ERROR: struct literals are not yet implemented
    }
    
    else if (expression->kind == AST_ArrayLiteral)
    {
        // TODO:
        //// ERROR: array literals are not yet implemented
    }
    
    else if (expression->kind == AST_Proc)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_ProcType)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Struct || expression->kind == AST_Union)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Enum)
    {
        // TODO:
        //// ERROR: enums are not yet implemented
    }
    
    else if (expression->kind == AST_Directive)
    {
        // TODO:
        //// ERROR: directives are not yet implemented
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
        if (expression->subscript_expr.array == 0)
        {
            //// ERROR: Missing array node in subscript expression
        }
        
        else if (expression->subscript_expr.index == 0)
        {
            //// ERROR: Missing index node in subscript expression
        }
        
        else
        {
            Check_Result array = CheckExpression(state, expression->subscript_expr.array);
            Check_Result index = CheckExpression(state, expression->subscript_expr.index);
            
            if (array.type != Type_Erroneous && index.type != Type_Erroneous)
            {
                if (array.type == Type_Unresolved || index.type == Type_Unresolved) result = UNRESOLVED_CHECK_RESULT;
                else
                {
                    if (!Type_IsIntegral(index.type))
                    {
                        //// ERROR: array indices must be of integral type
                    }
                    
                    else if (Type_IsSigned(index.type))
                    {
                        //// ERROR: array indices must be unsigned
                    }
                    
                    else
                    {
                        Type_Info* info = TypeInfo_FromID(array.type);
                        
                        // TODO: bounds checking on constant arrays
                        //if (index < 0 || index >= info->array_size)
                        
                        if (info->kind != Type_Array && info->kind != Type_DynamicArray &&
                            info->kind != Type_Slice)
                        {
                            //// ERROR: only arrays, dynamic arrays and slices can be taken the subscript of
                        }
                        
                        else
                        {
                            result = (Check_Result){
                                .type = info->sub_type,
                            };
                        }
                    }
                }
            }
        }
    }
    
    else if (expression->kind == AST_Slice)
    {
        if (expression->slice_expr.array)
        {
            //// ERROR: Missing array node in slice expression
        }
        
        else
        {
            Check_Result array = CheckExpression(state, expression->slice_expr.array);
            
            if (array.type != Type_Erroneous)
            {
                if (array.type == Type_Unresolved) result = UNRESOLVED_CHECK_RESULT;
                else
                {
                    Type_Info* info = TypeInfo_FromID(array.type);
                    
                    if (info->kind != Type_Array && info->kind != Type_DynamicArray &&
                        info->kind != Type_Slice)
                    {
                        //// ERROR: only arrays, dynamic arrays and slices can be taken the slice of
                    }
                    
                    else
                    {
                        // TODO: bounds checking
                        AST_Node* nodes[] = {
                            expression->slice_expr.start,
                            expression->slice_expr.one_after_end,
                        };
                        
                        for (umm i = 0; i < ARRAY_SIZE(nodes); ++i)
                        {
                            AST_Node* node = nodes[i];
                            
                            if (node != 0)
                            {
                                Check_Result node_result = CheckExpression(state, node);
                                
                                if      (node_result.type == Type_Erroneous) break;
                                else if (node_result.type == Type_Unresolved)
                                {
                                    result = UNRESOLVED_CHECK_RESULT;
                                    break;
                                }
                                
                                else if (!Type_IsIntegral(node_result.type))
                                {
                                    //// ERROR: slice indices must be intral
                                }
                                
                                else if (Type_IsSigned(node_result.type))
                                {
                                    //// ERROR: slice indices must be unsigned
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    else if (expression->kind == AST_Call)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (precedence >= 4 && precedence <= 9 ||
             expression->kind == AST_ElementOf  ||
             expression->kind == AST_UfcsOf)
    {
        if (expression->binary_expr.left == 0)
        {
            //// ERROR: Missing left side of binary expression
        }
        
        else if (expression->binary_expr.right == 0)
        {
            //// ERROR: Missing right side of binary expression
        }
        
        else
        {
            Type_ID left  = CheckExpression(state, expression->binary_expr.left);
            Type_ID right = CheckExpression(state, expression->binary_expr.right);
            
            if (left.type != Type_Erroneous && right.type != Type_Erroneous)
            {
                if (expression->kind == AST_ElementOf)
                {
                    if (left.type == Type_Unresolved)
                    {
                        NOT_IMPLEMENTED;
                    }
                    
                    else
                    {
                        Type_Info* info = TypeInfo_FromID(left.type);
                        
                        NOT_IMPLEMENTED;
                    }
                }
                
                else if (left.type == Type_Unresolved || right.type = Type_Unresolved)
                {
                    result = UNRESOLVED_CHECK_RESULT;
                }
                
                else
                {
                    if (expression->kind == AST_UfcsOf)
                    {
                        // TODO:
                        //// ERROR: ufcs is not yet implemented
                    }
                    
                    else if (expression->kind == AST_ClosedRange || expression->kind == AST_HalfOpenRange)
                    {
                        // TODO:
                        //// ERROR: ranges are not yet implemented
                    }
                    
                    else if (expression->kind >= AST_BitwiseAnd && expression->kind <= AST_LeftShift ||
                             expression->kind >= AST_BitwiseOr  && expression->kind <= AST_BitwiseXor)
                    {
                        if (!Type_IsIntegral(left.type) || !Type_IsIntegral(right.type))
                        {
                            //// ERROR: operands to bitwise operators must be integral
                        }
                        
                        // TODO: breaks on type alias
                        else if (left.type != right.type)
                        {
                            //// ERROR: Both sides of bitwise operator must be of the same type
                        }
                        
                        else
                        {
                            result = (Check_Result){
                                .type = left.type
                            };
                        }
                    }
                    
                    else if (expression->kind >= AST_Mul && expression->kind <= AST_Rem ||
                             expression->kind >= AST_Add && expression->kind <= AST_Sub)
                    {
                        if (!Type_IsNumeric(left.type) || !Type_IsNumeric(right.type))
                        {
                            //// ERROR: operands must be of a numeric type
                        }
                        
                        // TODO: breaks on type alias
                        else if (left.type != right.type)
                        {
                            //// ERROR: both operands must be of the same type
                        }
                        
                        else
                        {
                            result = (Check_Result){
                                .type = left.type
                            };
                        }
                    }
                    
                    else if (expression->kind >= AST_FirstComparative && expression->kind <= AST_LastComparative)
                    {
                        // TODO: pointers and procs
                        if (left.type > Type_LastBaseType || right.type > Type_LastBaseType)
                        {
                            //// ERROR: only base types can be compared
                        }
                        
                        // TODO: breaks on type alias
                        else if (left.type != right.type)
                        {
                            //// ERROR: both operands must be of the same type
                        }
                        
                        else
                        {
                            result = (Check_Result){
                                .type = Type_Bool
                            };
                        }
                    }
                    
                    else
                    {
                        ASSERT(expression->kind == AST_And || expression->kind == AST_Or);
                        
                        // TODO: implicit conversion
                        // TODO: breaks on type alias
                        if (left.type != Type_Bool || right.type != Type_Bool)
                        {
                            //// ERROR: non boolean types used as operands for logical opeerator
                        }
                        
                        else
                        {
                            result = (Check_Result){
                                .type = Type_Bool
                            };
                        }
                    }
                }
            }
        }
    }
    
    else if (expression->kind == AST_Conditional)
    {
        if (expression->conditional_expr.condition == 0)
        {
            //// ERROR: Missing condition
        }
        
        else if (expression->conditional_expr.true_clause == 0)
        {
            //// ERROR: Missing true clause
        }
        
        else if (expression->conditional_expr.false_clause == 0)
        {
            //// ERROR: Missing false clause
        }
        
        else
        {
            Check_Result condition    = CheckExpression(expression->conditional_expr.condition);
            Check_Result true_clause  = CheckExpression(expression->conditional_expr.true_clause);
            Check_Result false_clause = CheckExpression(expression->conditional_expr.false_clause);
            
            if (condition.type    != Type_Erroneous && true_clause.type != Type_Erroneous &&
                false_clause.type != Type_Erroneous)
            {
                if (condition.type    == Type_Unresolved || true_clause.type == Type_Unresolved ||
                    false_clause.type == Type_Unresolved)
                {
                    result = UNRESOLVED_CHECK_RESULT;
                }
                
                // TODO: implicit conversion
                // TODO: breaks on type alias
                else if (condition.type != Type_Bool)
                {
                    //// ERROR: condition must be of boolean type
                }
                
                // TODO: implicit conversion
                // TODO: breaks on type alias
                else if (true_clause.type != false_clause.type)
                {
                    //// ERROR: both branches must be of the same type
                }
                
                else
                {
                    result = (Check_Result){
                        .type = true_clause.type,
                    };
                }
            }
        }
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
        //// ERROR: when statements are not yet supported
        encountered_errors = true;
    }
    
    else if (statement->kind == AST_VariableDecl)
    {
        if (statement->var_decl.names == 0)
        {
            //// ERROR: missing name of variable in variable declaration
            encountered_errors = true;
        }
        
        else if (statement->var_decl.type == 0)
        {
            if (statement->var_decl.values == 0 || statement->var_decl.is_uninitialized)
            {
                //// ERROR: cannot infer type without a value
                encountered_errors = true;
            }
            
            else
            {
                // TODO:
                //// ERROR: type inference is not yet supported
                encountered_errors = true;
            }
        }
        
        else
        {
            NOT_IMPLEMENTED;
        }
    }
    
    else if (statement->kind == AST_ConstantDecl)
    {
        if (statement->const_decl.type == 0)
        {
            // TODO:
            //// ERROR: type inference is not yet supported
            encountered_errors = true;
        }
        
        else
        {
            NOT_IMPLEMENTED;
        }
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
                if (statement->if_statement.init != 0)
                {
                    //// ERROR: if init statements are not yet supported
                }
                
                if (!encountered_errors)
                {
                    if (statement->if_statement.condition == 0)
                    {
                        //// ERROR: Missing if statement condition
                        encountered_errors = true;
                    }
                    
                    else if (!IS_EXPRESSION(statement->if_statement.condition->kind))
                    {
                        //// ERROR: If statement condition is not an expression
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        Check_Result condition = CheckExpression(state, statement->if_statement.condition);
                        
                        if (condition.type == Type_Erroneous) encountered_errors = true;
                        else if (condition.type != Type_Unresolved)
                        {
                            // TODO: implicit conversions
                            if (condition.type != Type_Bool)
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
                        encountered_errors = CheckScope(state, statement->if_statement.true_body);
                        
                        if (!encountered_errors && statement->if_statement.false_body != 0)
                        {
                            encountered_errors = CheckStatement(state, statement->if_statement.false_body);
                        }
                    }
                }
            }
            
            else if (statement->kind == AST_While)
            {
                NOT_IMPLEMENTED;
            }
            
            else if (statement->kind == AST_Break || statement->kind == AST_Continue)
            {
                NOT_IMPLEMENTED;
                // check if in loop
                
                if (statement->break_statement.label != BLANK_IDENTIFIER)
                {
                    // TODO:
                    //// ERROR: labels are not yet supported
                    encountered_errors = true;
                }
            }
            
            else if (statement->kind == AST_Defer)
            {
                // TODO:
                //// ERROR: defer statements are not yet supported
                encountered_errors = true;
            }
            
            else if (statement->kind == AST_Return)
            {
                NOT_IMPLEMENTED;
            }
            
            else if (statement->kind == AST_Using)
            {
                // TODO:
                //// ERROR: defer statements are not yet supported
                encountered_errors = true;
            }
            
            else if (statement->kind == AST_Assignment)
            {
                NOT_IMPLEMENTED;
            }
            
            else
            {
                Check_Result result = CheckExpression(state, statement);
                NOT_IMPLEMENTED;
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