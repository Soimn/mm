internal void
CGC_Print(Cap_Buffer* out, const char* format, ...)
{
    va_list arg_list;
    va_start(arg_list, format);
    
    umm bytes_written = String_FormatArgList(CapBuffer_FreeSpace(*out), format, arg_list);
    CapBuffer_Grow(out, bytes_written);
    
    va_end(arg_list);
}

internal void
CGC_GenExpression(Cap_Buffer* out, AST_Node* expression)
{
    umm precedence = PRECEDENCE_FROM_KIND(expression->kind);
    
    if      (expression->kind == AST_Identifier) CGC_Print(out, "%S ", (String)expression->identifier);
    else if (expression->kind == AST_String)     CGC_Print(out, "\"%S\" ", (String)expression->string);
    else if (expression->kind == AST_Boolean)    CGC_Print(out, "%b", expression->boolean);
    else if (expression->kind == AST_Char)       CGC_Print(out, "%u", expression->character);
    else if (expression->kind == AST_Number)
    {
        if (expression->number.is_float)
        {
            if (expression->number.width == 64) CGC_Print(out, "%f", expression->number.floating);
            else                                CGC_Print(out, "%ff", (f32)expression->number.floating);
        }
        
        else
        {
            if (expression->number.is_negative) CGC_Print(out, "-");
            CGC_Print(out, "%u", expression->number.integer);
        }
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
    
    else if (precedence == 1)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Subscript || expression->kind == AST_Slice)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Call || expression->kind == AST_InfixCall)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (precedence == 3)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (precedence >= 4 && precedence <= 9 || expression->kind == AST_ElementOf || expression->kind == AST_UfcsOf)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expression->kind == AST_Conditional)
    {
        NOT_IMPLEMENTED;
    }
    
    else INVALID_CODE_PATH;
}

internal void
CGC_GenStatement(AST_Node* statement)
{
    NOT_IMPLEMENTED;
}

internal void
CG_GenCCode(AST_Node* statements, Cap_Buffer* out)
{
    NOT_IMPLEMENTED;
}