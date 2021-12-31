enum CHECK_RESULT
{
    Check_Error,      // NOTE: the decl is erroneous
    Check_Incomplete, // NOTE: no progress was made resolving the decl
    Check_Completing, // NOTE: some progress was made resolving the decl
    Check_Complete    // NOTE: decl has been resolved
};

internal bool MM_AddFile(String path, File_ID includer_id, u32 includer_offset, File_ID* id);

typedef struct Check_Info
{
    Const_Val const_val;
    Type_ID type;
    bool is_const;
} Check_Info;

internal Enum8(CHECK_RESULT)
CheckExpression(AST_Node* expr, Check_Info* info)
{
    Enum8(CHECK_RESULT) result = Check_Error;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal Enum8(CHECK_RESULT)
CheckGlobalDeclaration(AST_Node* decl, AST_Node** link)
{
    Enum8(CHECK_RESULT) result = Check_Error;
    
    if (decl->kind == AST_VariableDecl)
    {
        if (decl->var_decl.symbol == 0)
        {
            if (decl->var_decl.names == 0)
            {
                //// ERROR: missing names of variables in variable declaration
                result = Check_Error;
            }
            
            else
            {
                bool encountered_errors = false;
                
                for (AST_Node* node = decl->var_decl.names;
                     node != 0 && !encountered_errors;
                     node = node->next)
                {
                    if (node->kind != AST_Identifier)
                    {
                        //// ERROR: variable name must be an identifier
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        Symbol* duplicate = MM_GetSymbol(node->identifier);
                        if (duplicate != 0)
                        {
                            //// ERROR: duplicate symbol definition
                            encountered_errors = true;
                        }
                        
                        else MM_AddSymbol(node->identifier, Symbol_Var);
                    }
                }
                
                if (encountered_errors) result = Check_Error;
                else                    result = Check_Completing;
            }
        }
        
        NOT_IMPLEMENTED;
    }
    
    else if (decl->kind == AST_ConstantDecl)
    {
        if (decl->const_decl.symbol == 0)
        {
            if (decl->const_decl.names == 0)
            {
                //// ERROR: missing names of constants in constant declaration
                result = Check_Error;
            }
            
            else
            {
                bool encountered_errors = false;
                
                for (AST_Node* node = decl->const_decl.names;
                     node != 0 && !encountered_errors;
                     node = node->next)
                {
                    if (node->kind != AST_Identifier)
                    {
                        //// ERROR: constant name must be an identifier
                        encountered_errors = true;
                    }
                    
                    else
                    {
                        Symbol* duplicate = MM_GetSymbol(node->identifier);
                        if (duplicate != 0)
                        {
                            //// ERROR: duplicate symbol definition
                            encountered_errors = true;
                        }
                        
                        else MM_AddSymbol(node->identifier, Symbol_Var);
                    }
                }
                
                if (encountered_errors) result = Check_Error;
                else                    result = Check_Completing;
            }
        }
        
        NOT_IMPLEMENTED;
    }
    
    else if (decl->kind == AST_When)
    {
        Check_Info condition_info;
        Enum8(CHECK_RESULT) condition_result = CheckExpression(decl->when_statement.condition, &condition_info);
        
        if (condition_result != Check_Complete) result = condition_result;
        else
        {
            if (!Type_IsImplicitlyConvertibleTo(condition_info.type, Type_Bool))
            {
                //// ERROR: Condition of when statement must be of boolean type
                result = Check_Error;
            }
            
            else if (!condition_info.is_const)
            {
                //// ERROR: Condition of when statement must be constant
                result = Check_Error;
            }
            
            else
            {
                bool condition_val = ConstVal_ToBool(condition_info.const_val);
                AST_Node* body;
                if (condition_val) body = decl->when_statement.true_body->scope_statement.body;
                else               body = decl->when_statement.false_body->scope_statement.body;
                
                AST_Node* last_in_body = body;
                for (; last_in_body != 0 && last_in_body->next != 0; last_in_body = last_in_body->next);
                
                if (last_in_body)
                {
                    *link = body;
                    last_in_body->next = decl->next;
                }
                
                result = Check_Completing;
            }
        }
    }
    
    else if (decl->kind == AST_IncludeDecl)
    {
        if (!MM_AddFile(StringLiteral_Of(decl->include.path), decl->file_id, decl->file_offset, &decl->include.file_id))
        {
            result = Check_Error;
        }
        
        else result = Check_Complete;
    }
    
    else
    {
        //// ERROR: Illegal use of statement in global scope
        result = Check_Error;
    }
    
    return result;
}

internal bool
MM_ResolveNextDecl(AST_Node** resolved_decl)
{
    bool encountered_errors = false;
    
    Enum8(CHECK_RESULT) acc_result = Check_Incomplete;
    
    AST_Node** link = &MM.ast;
    AST_Node* scan  = MM.ast;
    for (; scan != 0; scan = scan->next, link = &(*link)->next)
    {
        Enum8(CHECK_RESULT) result = CheckGlobalDeclaration(scan, link);
        
        if (result == Check_Error)
        {
            encountered_errors = true;
            break;
        }
        
        else if (result == Check_Complete)
        {
            acc_result = Check_Complete;
            
            if (resolved_decl != 0) *resolved_decl = scan;
            
            break;
        }
        
        else acc_result = MAX(acc_result, result);
    }
    
    if (!encountered_errors)
    {
        if (acc_result == Check_Incomplete)
        {
            // NOTE: no progress was made
            NOT_IMPLEMENTED;
            encountered_errors = true;
        }
    }
    
    return !encountered_errors && scan != 0;
}