enum CHECKER_RESULT
{
    Checker_Error = 0,
    Checker_Incomplete,
    Checker_Valid,
};

internal Enum8(CHECKER_RESULT)
CheckExpression(AST_Node* expression)
{
    Enum8(CHECKER_RESULT) result = Checker_Error;
    
    if (expression->kind < AST_FirstExpression || expression->kind > AST_LastExpression)
    {
        //// ERROR: expected expression
    }
    
    else
    {
        umm precedence = expression->kind / 20;
        
        if (precedence == 0)
        {
            /*
AST_Identifier,
    AST_String,
    AST_Char,
    AST_Number,
    AST_Boolean,
    AST_StructLiteral,
    AST_ArrayLiteral,
    AST_Proc,
    AST_ProcType,
    AST_Struct,
    AST_Union,
    AST_Enum,
    AST_Directive,
*/
            NOT_IMPLEMENTED;
        }
        
        else if (precedence == 1)
        {
            /*
AST_PointerType,
    AST_SliceType,
    AST_ArrayType,
    AST_DynamicArrayType,
    AST_PolymorphicType,
*/
            NOT_IMPLEMENTED;
        }
        
        else if (precedence == 2)
        {
            /*
AST_Subscript,
    AST_Slice,
    AST_Call,
    AST_ElementOf,
    AST_UfcsOf,
*/
            NOT_IMPLEMENTED;
        }
        
        else if (precedence == 3)
        {
            /*
            AST_Negation,
    AST_Complement,
    AST_Not,
    AST_Reference,
    AST_Dereference,
    AST_Spread,
*/
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
        
        else if (precedence == 10)
        {
            // AST_Conditional,
            NOT_IMPLEMENTED;
        }
        
        else
        {
            //// ERROR: Invalid expression kind
        }
    }
    
    return result;
}

internal Enum8(CHECKER_RESULT)
CheckStatement(AST_Node* statement)
{
    Enum8(CHECKER_RESULT) result = Checker_Error;
    
    if (statement->kind >= AST_FirstExpression && statement->kind <= AST_LastExpression)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (statement->kind == AST_If)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (statement->kind == AST_When)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (statement->kind == AST_While)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (statement->kind == AST_Break || statement->kind == AST_Continue)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (statement->kind == AST_Defer)
    {
        NOT_IMPLEMENTED;
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
    
    else if (statement->kind == AST_VariableDecl)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (statement->kind == AST_ConstantDecl)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (statement->kind == AST_ImportDecl)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (statement->kind == AST_ForeignDecl)
    {
        NOT_IMPLEMENTED;
    }
    
    else
    {
        //// ERROR: invalid statement kind
    }
    
    return result;
}