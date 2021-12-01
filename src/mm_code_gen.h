internal void
CGCD_GenNode(AST_Node* node)
{
    umm precedence = node->kind / 20;
    
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
    
    else if (precedence == 1 || precedence == 3)
    {
        /*
        // precedence 1
        AST_PointerType,
        AST_SliceType,
        AST_ArrayType,
        AST_DynamicArrayType,
        //AST_PolymorphicType,
        
        // precedence 3
        AST_Negation,
        AST_Complement,
        AST_Not,
        AST_Reference,
        AST_Dereference,
        AST_Spread,
        */
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_Call || node->kind == AST_InfixCall)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_Subscript)
    {
        NOT_IMPLEMENTED;
    }
    
    else if ((precedence == 2 || precedence > 4 && precedence <= 9) &&
             node->kind != AST_UfcsOf && node->kind != AST_Mod && node->kind != AST_RightShift)
    {
        /*
AST_ElementOf,
        AST_Mul,
        AST_Div,
        AST_Rem,
        AST_BitwiseAnd,
        AST_ArithmeticRightShift,
        AST_LeftShift,
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
        //AST_Conditional,
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_Scope)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_If)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_When)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_While)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_Break || node->kind == AST_Continue)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_Defer)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_Return)
    {
        NOT_IMPLEMENTED;
    }
    
    
    else if (node->kind == AST_Using)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_Assignment)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_VariableDecl)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (node->kind == AST_ConstantDecl)
    {
        NOT_IMPLEMENTED;
    }
    
    else INVALID_CODE_PATH;
}

internal void
CG_GenCCodeDirectly()
{
    
}