// TODO: the checker relies too much on the c type system
// TODO: common type might conflict with procedures without return values

enum CHECK_RESULT
{
    Check_Error,      // NOTE: the decl is erroneous
    Check_Incomplete, // NOTE: no progress was made resolving the decl
    Check_Completing, // NOTE: some progress was made resolving the decl
    Check_Complete    // NOTE: decl has been resolved
};

internal bool MM_AddFile(String path, File_ID includer_id, u32 includer_offset, File_ID* id);
internal Enum8(CHECK_RESULT) CheckStatement(Scope_Chain chain, AST_Node* statement, AST_Node** link, bool error_sweep);

typedef struct Scope_Chain
{
    struct Scope_Chain* next;
    Symbol_Table* table;
} Scope_Chain;

internal Enum8(CHECK_RESULT)
CheckScope(Scope_Chain* chain, AST_Node* scope_node, bool error_sweep)
{
    Scope_Chain chain_link = {
        .next  = chain,
        .table = &scope_node->scope_statement.symbol_table
    };
    
    Enum8(CHECK_RESULT) result = Check_Complete;
    
    for (AST_Node** link = &scope_node->body;
         *link != 0 && result == Check_Complete;
         link = &(*link)->next)
    {
        result = CheckStatement(&chain_link, *link, link, error_sweep);
    }
    
    return acc_result;
}

internal Enum8(CHECK_RESULT)
CheckStatement(Scope_Chain chain, AST_Node* statement, AST_Node** link, bool error_sweep)
{
    Enum8(CHECK_RESULT) result = Check_Error;
    
    if (decl->kind == AST_When)
    {
        Check_Info condition_info;
        Enum8(CHECK_RESULT) condition_result = CheckExpression(decl->when_statement.condition, &condition_info);
        
        if (condition_result != Check_Complete) result = condition_result;
        else
        {
            if (!Type_IsImplicitlyCastableTo(condition_info.type, Type_Bool))
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
                bool condition_val = ConstVal_ConvertToBool(condition_info.const_val, Type_StripAlias(condition_info.type));
                AST_Node* body;
                if (condition_val) body = decl->when_statement.true_body->scope_statement.body;
                else               body = decl->when_statement.false_body->scope_statement.body;
                
                AST_Node* last_in_body = body;
                for (; last_in_body != 0 && last_in_body->next != 0; last_in_body = last_in_body->next);
                
                if (last_in_body)
                {
                    last_in_body->next = *link;
                    *link = body;
                }
                
                result = Check_Completing;
            }
        }
    }
    
    else NOT_IMPLEMENTED;
    
    return result;
}

internal bool
MM_ResolveNextDecl(AST_Node** resolved_decl)
{
    bool encountered_errors = false;
    
    if (MM.ast != 0)
    {
        bool error_sweep = false:
        
        while (!encountered_errors)
        {
            Enum8(CHECK_RESULT) acc_result = Check_Incomplete;
            
            for (AST_Node** link = &MM.ast;
                 *link != 0 && acc_result != Check_Error && acc_result != Check_Complete;
                 link = &(*link)->next)
            {
                Enum8(CHECK_RESULT) result = Check_Error;
                
                AST_Node* decl = *link;
                
                if (decl->kind == AST_VariableDecl || decl->kind == AST_ConstantDecl ||
                    decl->kind == AST_When)
                {
                    result = CheckStatement(0, decl, link, error_sweep);
                    
                    if (result == Check_Complete && decl->kind != AST_When)
                    {
                        *link      = decl->next;
                        decl->next = 0;
                        
                        *resolved_decl = decl;
                    }
                }
                
                else if (decl->kind == AST_IncludeDecl)
                {
                    if (!MM_AddFile(StringLiteral_Of(decl->include.path), decl->file_id, decl->file_offset, &decl->include.file_id))
                    {
                        result = Check_Error;
                    }
                    
                    else
                    {
                        *link      = decl->next;
                        decl->next = 0;
                        
                        result = Check_Completing;
                    }
                }
                
                else
                {
                    //// ERROR: Illegal use of statement in global scope
                    result = Check_Error;
                }
                
                acc_result = (result == Check_Error ? Check_Error : MAX(acc_result, result));
            }
            
            if      (acc_result == Check_Complete)   break;
            else if (acc_result == Check_Completing) continue;
            else if (acc_result == Check_Incomplete)
            {
                error_sweep = true;
                continue;
            }
            
            else if (acc_result == Check_Error)
            {
                MM.encountered_errors = true;
            }
        }
    }
    
    return (!MM.encountered_errors && MM.ast != 0);
}