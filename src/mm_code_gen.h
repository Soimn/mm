internal void
CGCD_Print(Buffer* out, const char* format, ...)
{
    va_list arg_list;
    va_start(arg_list, format);
    
    umm advancement = String_FormatArgList(*out, format, arg_list);
    Buffer_Advance(out, advancement);
    
    va_end(arg_list);
}

internal void
CGCD_Indent(Buffer* out, u64 indent)
{
    for (umm i = 0; i < indent; ++i)
    {
        if (out->size != 0) *out->data = '\t';
        Buffer_Advance(out, 1);
    }
}

internal void
CGCD_GenNode(Buffer* out, AST_Node* node, u64 indent)
{
    CGCD_Indent(out, indent);
    
    umm precedence = node->kind / 20;
    
    if (precedence == 0)
    {
        switch (node->kind)
        {
            case AST_Identifier: CGCD_Print(out, " %S ", node->identifier);                 break;
            case AST_String:     CGCD_Print(out, "\"%S\"", node->string);                   break;
            case AST_Char:       CGCD_Print(out, "'%c'", node->character.bytes[0]);         break;
            case AST_Boolean:    CGCD_Print(out, "%s", (node->boolean ? "true" : "false")); break;
            case AST_Number:
            {
                if (node->number.is_float) CGCD_Print(out, "%ff", node->number.floating);
                else                       CGCD_Print(out, "%c%U", (node->number.is_negative ? '-' : '+'), node->number.integer);
            } break;
        }
    }
    
    else if (precedence == 1 || precedence == 3)
    {
        if (node->kind == AST_PointerType)
        {
            CGCD_GenNode(out, node->unary_expr, 0);
            CGCD_Print(out, "*");
        }
        
        else
        {
            switch (node->kind)
            {
                case AST_Negation:    CGCD_Print(out, "-"); break;
                case AST_Complement:  CGCD_Print(out, "~"); break;
                case AST_Not:         CGCD_Print(out, "!"); break;
                case AST_Reference:   CGCD_Print(out, "&"); break;
                case AST_Dereference: CGCD_Print(out, "*"); break;
                
                INVALID_DEFAULT_CASE;
            }
            
            CGCD_GenNode(out, node->unary_expr, indent);
        }
    }
    
    else if (node->kind == AST_Call || node->kind == AST_InfixCall)
    {
        CGCD_GenNode(out, node->call_expr.func, 0);
        CGCD_Print(out, "(");
        
        for (AST_Node* param = node->call_expr.params; param != 0; param = param->next)
        {
            CGCD_GenNode(out, param, 0);
            if (param->next != 0) CGCD_Print(out, ", ");
        }
        
        CGCD_Print(out, ")");
    }
    
    else if (node->kind == AST_Subscript)
    {
        CGCD_GenNode(out, node->subscript_expr.array, 0);
        CGCD_Print(out, "(");
        
        CGCD_GenNode(out, node->subscript_expr.index, 0);
        
        CGCD_GenNode(out, node->call_expr.params, 0);
        CGCD_Print(out, ")");
    }
    
    else if (precedence == 2 || precedence > 4 && precedence <= 9)
    {
        CGCD_GenNode(out, node->binary_expr.left, 0);
        
        switch (node->kind)
        {
            case AST_ElementOf:            CGCD_Print(out, ".");    break;
            case AST_Mul:                  CGCD_Print(out, " * ");  break;
            case AST_Div:                  CGCD_Print(out, " / ");  break;
            case AST_Rem:                  CGCD_Print(out, " % ");  break;
            case AST_BitwiseAnd:           CGCD_Print(out, " & ");  break;
            case AST_ArithmeticRightShift: CGCD_Print(out, " >> "); break;
            case AST_LeftShift:            CGCD_Print(out, " << "); break;
            case AST_Add:                  CGCD_Print(out, " + ");  break;
            case AST_Sub:                  CGCD_Print(out, " - ");  break;
            case AST_BitwiseOr:            CGCD_Print(out, " | ");  break;
            case AST_BitwiseXor:           CGCD_Print(out, " ^ ");  break;
            case AST_IsEqual:              CGCD_Print(out, " == "); break;
            case AST_IsNotEqual:           CGCD_Print(out, " != "); break;
            case AST_IsStrictlyLess:       CGCD_Print(out, " < ");  break;
            case AST_IsStrictlyGreater:    CGCD_Print(out, " > ");  break;
            case AST_IsLess:               CGCD_Print(out, " <= "); break;
            case AST_IsGreater:            CGCD_Print(out, " >= "); break;
            case AST_And:                  CGCD_Print(out, " && "); break;
            case AST_Or:                   CGCD_Print(out, " || "); break;
        }
        
        CGCD_GenNode(out, node->binary_expr.right, 0);
    }
    
    else if (precedence == 10)
    {
        CGCD_GenNode(out, node->conditional_expr.condition, 0);
        CGCD_Print(out, " ? ");
        CGCD_GenNode(out, node->conditional_expr.true_clause, 0);
        CGCD_Print(out, " : ");
        CGCD_GenNode(out, node->conditional_expr.false_clause, 0);
    }
    
    else if (node->kind == AST_Scope)
    {
        if (node->scope_statement.is_do)
        {
            CGCD_GenNode(out, node->scope_statement.body, 0);
        }
        
        else
        {
            CGCD_Print(out, "{\n");
            
            for (AST_Node* statement = node->scope_statement.body; statement != 0; statement = statement->next)
            {
                CGCD_GenNode(out, statement, indent + 1);
                CGCD_Print(out, ";\n");
            }
            
            CGCD_Print(out, "}\n");
        }
    }
    
    else if (node->kind == AST_If)
    {
        CGCD_Print(out, "if (");
        CGCD_GenNode(out, node->if_statement.condition, 0);
        CGCD_Print(out, ")\n");
        CGCD_GenNode(out, node->if_statement.true_body, indent);
        
        if (node->if_statement.false_body != 0)
        {
            AST_Node* els = node->if_statement.false_body;
            
            if (els->kind == AST_Scope)
            {
                CGCD_Indent(out, indent);
                CGCD_Print(out, "else%c", (els->scope_statement.is_do ? ' ' : '\n'));
                
                CGCD_GenNode(out, els, indent);
            }
            
            else
            {
                ASSERT(els->kind == AST_If);
                
                CGCD_Indent(out, indent);
                CGCD_Print(out, "else ");
                CGCD_GenNode(out, els, 0);
            }
        }
    }
    
    else if (node->kind == AST_While)
    {
        CGCD_Print(out, "while (");
        CGCD_GenNode(out, node->while_statement.condition, 0);
        CGCD_Print(out, ")\n");
        CGCD_GenNode(out, node->while_statement.body, indent);
    }
    
    else if (node->kind == AST_Break || node->kind == AST_Continue)
    {
        CGCD_Print(out, "%s;\n", (node->kind == AST_Break ? "break" : "continue"));
    }
    
    else if (node->kind == AST_Return)
    {
        CGCD_Print(out, "return ");
        CGCD_GenNode(out, node->return_statement.values, 0);
        CGCD_Print(out, ";\n");
    }
    
    else if (node->kind == AST_Assignment)
    {
        CGCD_GenNode(out, node->assignment_statement.left, 0);
        
        switch (node->assignment_statement.kind)
        {
            case Token_Equals:                     CGCD_Print(out, " = ");   break;
            case Token_StarEquals:                 CGCD_Print(out, " *= ");  break;
            case Token_SlashEquals:                CGCD_Print(out, " /= ");  break;
            case Token_RemEquals:                  CGCD_Print(out, " %= ");  break;
            case Token_AndEquals:                  CGCD_Print(out, " &= ");  break;
            case Token_ArithmeticRightShiftEquals: CGCD_Print(out, " >>= "); break;
            case Token_RightShiftEquals:           CGCD_Print(out, " >>= "); break;
            case Token_LeftShiftEquals:            CGCD_Print(out, " <<= "); break;
            case Token_PlusEquals:                 CGCD_Print(out, " += ");  break;
            case Token_MinusEquals:                CGCD_Print(out, " -= ");  break;
            case Token_OrEquals:                   CGCD_Print(out, " |= ");  break;
            case Token_HatEquals:                  CGCD_Print(out, " ^= ");  break;
            
        }
        
        CGCD_GenNode(out, node->assignment_statement.right, 0);
        CGCD_Print(out, ";\n");
    }
    
    else if (node->kind == AST_VariableDecl)
    {
        CGCD_GenNode(out, node->var_decl.type, 0);
        CGCD_GenNode(out, node->var_decl.names, 0);
        
        if (node->var_decl.values != 0)
        {
            CGCD_Print(out, " = ");
            CGCD_GenNode(out, node->var_decl.values, 0);
        }
        
        CGCD_Print(out, ";\n");
    }
    
    else if (node->kind == AST_ConstantDecl)
    {
        if (node->const_decl.values->kind == AST_Proc)
        {
            AST_Node* proc = node->const_decl.values;
            
            if (proc->proc_literal.return_values == 0) CGCD_Print(out, " void ");
            else
            {
                CGCD_GenNode(out, node->const_decl.values->proc_literal.return_values, 0);
            }
            
            CGCD_GenNode(out, node->const_decl.names, 0);
            
            CGCD_Print(out, "(");
            
            for (AST_Node* param = proc->proc_literal.params; param != 0; param = param->next)
            {
                ASSERT(param->kind == AST_VariableDecl);
                
                CGCD_GenNode(out, param->var_decl.type, 0);
                CGCD_GenNode(out, param->var_decl.names, 0);
                
                if (param->next != 0) CGCD_Print(out, ", ");
            }
            
            CGCD_Print(out, ")\n");
            
            CGCD_GenNode(out, proc->proc_literal.body, indent);
        }
        
        else if (node->const_decl.values->kind == AST_Struct || node->const_decl.values->kind == AST_Union)
        {
            AST_Node* str = node->const_decl.values;
            
            CGCD_Print(out, "typedef %s ", (str->kind == AST_Struct ? "struct" : "union"));
            CGCD_GenNode(out, node->const_decl.names, 0);
            CGCD_Print(out, "\n");
            CGCD_Indent(out, indent);
            CGCD_Print(out, "{\n");
            
            for (AST_Node* member = str->struct_type.members; member != 0; member = member->next)
            {
                ASSERT(member->kind == AST_VariableDecl);
                
                CGCD_GenNode(out, member->var_decl.type, indent + 1);
                CGCD_GenNode(out, member->var_decl.names, 0);
                CGCD_Print(out, ";\n");
            }
            
            CGCD_Indent(out, indent);
            CGCD_Print(out, "} ");
            CGCD_GenNode(out, node->const_decl.names, 0);
            CGCD_Print(out, ";\n");
        }
        
        else if (node->const_decl.values->kind == AST_Enum)
        {
            AST_Node* str = node->const_decl.values;
            
            CGCD_Print(out, "enum ");
            CGCD_GenNode(out, node->const_decl.names, 0);
            CGCD_Print(out, "\n");
            CGCD_Indent(out, indent);
            CGCD_Print(out, "{\n");
            
            for (AST_Node* member = str->enum_type.members; member != 0; member = member->next)
            {
                ASSERT(member->kind == AST_Assignment && member->assignment_statement.kind == AST_Invalid ||
                       member->kind >= AST_FirstExpression && member->kind <= AST_LastExpression);
                
                if (member->kind == AST_Assignment)
                {
                    CGCD_GenNode(out, member->assignment_statement.left, indent + 1);
                    CGCD_Print(out, " = \n");
                    CGCD_GenNode(out, member->assignment_statement.right, 0);
                }
                
                else CGCD_GenNode(out, member, 0);
                
                CGCD_Print(out, ",\n");
            }
            
            CGCD_Indent(out, indent);
            CGCD_Print(out, "} ");
            CGCD_GenNode(out, node->const_decl.names, 0);
            CGCD_Print(out, ";\n");
        }
        
        else
        {
            CGCD_GenNode(out, node->const_decl.type, 0);
            CGCD_GenNode(out, node->const_decl.names, 0);
            
            if (node->const_decl.values != 0)
            {
                CGCD_Print(out, " = ");
                CGCD_GenNode(out, node->const_decl.values, 0);
            }
            
            CGCD_Print(out, ";\n");
        }
    }
    
    else INVALID_CODE_PATH;
}

internal u64
CG_GenCCodeDirectly(AST_Node* statements, Buffer out)
{
    Buffer advance_out = out;
    
    for (AST_Node* statement = statements; statement != 0; statement = statement->next)
    {
        CGCD_GenNode(&advance_out, statement, 0);
    }
    
    return out.size - advance_out.size;
}

internal void
CGC_Print(Dynamic_Buffer* out, const char* format, ...)
{
    va_list arg_list;
    va_start(arg_list, format);
    
    umm advancement = String_FormatArgList(*out, format, arg_list);
    Buffer_Advance(out, advancement);
    
    va_end(arg_list);
}

internal void
CGC_GenType(AST_Node* type)
{
    NOT_IMPLEMENTED;
}

internal void
CGC_GenExpression(AST_Node* expression)
{
    NOT_IMPLEMENTED;
}

internal void
CGC_GenStatement(AST_Node* statement)
{
    NOT_IMPLEMENTED;
}

internal Buffer
CG_GenCCode(AST_Node* statements, Dynamic_Buffer* out)
{
    NOT_IMPLEMENTED;
}