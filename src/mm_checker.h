// TODO: common type might conflict with procedures without return values

// TODO: take 4.0 = 4 into account when choosing common type
// TODO: Convert from Big_Int <-> Big_Float when common type differs from src type
// TODO: Bool vs Untyped Bool as result from  &&, ||, ==, !=

enum CHECK_RESULT
{
    Check_Incomplete, // NOTE: no progress was made resolving the decl
    Check_Completing, // NOTE: some progress was made resolving the decl
    Check_Complete,   // NOTE: decl has been resolved
    Check_Error,      // NOTE: the decl is erroneous
};

typedef struct Check_Info
{
    Enum8(CHECK_RESULT) result;
    Type_ID type;
    Const_Val const_val;
    bool is_const;
} Check_Info;

typedef struct Scope_Chain
{
    struct Scope_Chain* next;
    Symbol_Table* table;
} Scope_Chain;

typedef struct Checker_State
{
    Scope_Chain* chain;
    bool in_error_sweep;
} Checker_State;

internal bool MM_AddFile(String path, File_ID includer_id, u32 includer_offset, File_ID* id);
internal Enum8(CHECK_RESULT) CheckStatement(Checker_State* state, AST_Node* statement);
internal Enum8(CHECK_RESULT) CheckScope(Checker_State* state, AST_Node* scope_node);
internal Check_Info CheckExpression(Checker_State* state, AST_Node* expression);

internal Check_Info
CheckExpression(Checker_State* state, AST_Node* expression)
{
    Check_Info info = { .result = Check_Error };
    
    if (expression == 0)
    {
        //// ERROR: Missing expression
        info.result = Check_Error;
    }
    
    else
    {
        if (PRECEDENCE_FROM_KIND(expression->kind) >= 4 && PRECEDENCE_FROM_KIND(expression->kind) <= 9)
        {
            Check_Info lhs_info = CheckExpression(state, expression->binary_expr.left);
            
            if (lhs_info.result != Check_Complete) info.result = lhs_info.result;
            else
            {
                Check_Info rhs_info = CheckExpression(state, expression->binary_expr.right);
                
                if (rhs_info.result != Check_Complete) info.result = rhs_info.result;
                else
                {
                    Type_ID common_type = Type_NoType;
                    Big_Num lhs, rhs = {0};
                    
                    {
                        Type_ID t0 = lhs_info.type;
                        Type_ID t1 = rhs_info.type;
                        
                        Big_Num v0 = lhs_info.const_val.num;
                        Big_Num v1 = rhs_info.const_val.num;
                        
                        bool is_flipped = false;
                        
                        if (t0 == t1) common_type = t0;
                        else if (!Type_IsTyped(t0) || !Type_IsTyped(t1))
                        {
                            if (Type_IsTyped(t0) || Type_IsTyped(t1))
                            {
                                
                                if (Type_IsTyped(t1))
                                {
                                    Type_ID t_tmp = t1;
                                    t1 = t0;
                                    t0 = t_tmp;
                                    
                                    Big_Num v_tmp = v1;
                                    v1 = v0;
                                    v0 = v_tmp;
                                    
                                    is_flipped = true;
                                }
                                
                                if (t1 == Type_UntypedInt || t1 == Type_UntypedChar)
                                {
                                    if (Type_IsIntegral(t0) || t0 == Type_Typeid || t0 == Type_Byte)
                                    {
                                        if (BigNum_IEffectiveSize(v1) <= Type_Sizeof(t0)) common_type = t0;
                                        else
                                        {
                                            //// ERROR: conversion loss
                                            common_type = Type_NoType;
                                        }
                                    }
                                    
                                    else if (Type_IsFloating(t0))
                                    {
                                        if (BigNum_FEffectiveSize(v1) <= Type_Sizeof(t0)) common_type = t0;
                                        else
                                        {
                                            //// ERROR: conversion loss
                                            common_type = Type_NoType;
                                        }
                                    }
                                }
                                
                                else if (t1 == Type_UntypedFloat)
                                {
                                    if (Type_IsFloating(t0))
                                    {
                                        if (BigNum_FEffectiveSize(v1) <= Type_Sizeof(t0)) common_type = t0;
                                        else
                                        {
                                            //// ERROR: conversion loss
                                            common_type = Type_NoType;
                                        }
                                    }
                                    
                                    else if (Type_IsIntegral(t0))
                                    {
                                        if (BigNum_IsEqual(BigNum_Truncate(v1), v1) &&
                                            BigNum_IEffectiveSize(v1) <= Type_Sizeof(t0)) common_type = t0;
                                        else
                                        {
                                            //// ERROR: conversion loss
                                            common_type = Type_NoType;
                                        }
                                    }
                                }
                                
                                else if (t1 == Type_UntypedBool)
                                {
                                    if (Type_IsBoolean(t0))
                                    {
                                        common_type = t0;
                                    }
                                }
                                
                                else common_type = Type_NoType;
                            }
                            
                            else
                            {
                                if (t0 == Type_UntypedInt  && t1 == Type_UntypedChar ||
                                    t0 == Type_UntypedChar && t1 == Type_UntypedInt)
                                {
                                    common_type = Type_UntypedChar;
                                }
                                
                                else if (t0 == Type_UntypedInt   && t1 == Type_UntypedFloat ||
                                         t0 == Type_UntypedFloat && t1 == Type_UntypedInt)
                                {
                                    common_type = Type_UntypedFloat;
                                }
                            }
                        }
                        
                        lhs = (is_flipped ? v1 : v0);
                        rhs = (is_flipped ? v0 : v1);
                    }
                    
                    if (common_type == Type_NoType)
                    {
                        //// ERROR: No common type found
                        info.result = Check_Error;
                    }
                    
                    else
                    {
                        info = (Check_Info){
                            .result   = Check_Error, // NOTE: Assume the worst, correct if wrong
                            .is_const = (lhs_info.is_const && rhs_info.is_const)
                        };
                        
                        imm byte_size = (Type_IsTyped(common_type) ? Type_Sizeof(common_type) : -1);
                        
                        switch (expression->kind)
                        {
                            case AST_Add:
                            case AST_Sub:
                            case AST_IsStrictlyLess:
                            case AST_IsStrictlyGreater:
                            case AST_IsLess:
                            case AST_IsGreater:
                            {
                                // TODO: pointer arithmetic
                                if (!Type_IsNumeric(common_type))
                                {
                                    //// ERROR: operator requires common numeric type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info.result = Check_Complete;
                                    info.type   = (expression->kind == AST_Add || expression->kind == AST_Sub
                                                   ? common_type
                                                   : Type_UntypedBool);
                                    
                                    if (Type_IsFloating(common_type) && (expression->kind == AST_Add || expression->kind == AST_Sub))
                                    {
                                        if (expression->kind == AST_Add)
                                        {
                                            info.const_val.num = BigNum_FAdd(lhs, rhs, byte_size);
                                        }
                                        
                                        else
                                        {
                                            info.const_val.num = BigNum_FSub(lhs, rhs, byte_size);
                                        }
                                    }
                                    
                                    else
                                    {
                                        switch (expression->kind)
                                        {
                                            case AST_Add:
                                            {
                                                info.const_val.num = BigNum_IAdd(lhs, rhs, byte_size);
                                            } break;
                                            
                                            case AST_Sub:
                                            {
                                                info.const_val.num = BigNum_ISub(lhs, rhs, byte_size);
                                            } break;
                                            
                                            case AST_IsGreater:
                                            case AST_IsStrictlyLess:
                                            {
                                                bool val = BigNum_IsStrictLess(lhs, rhs);
                                                info.const_val.num = BigNum_FromU64((expression->kind == AST_IsGreater) ^ val);
                                            } break;
                                            
                                            case AST_IsLess:
                                            case AST_IsStrictlyGreater:
                                            {
                                                bool val = BigNum_IsStrictGreater(lhs, rhs);
                                                info.const_val.num = BigNum_FromU64((expression->kind == AST_IsLess) ^ val);
                                            } break;
                                            
                                            INVALID_DEFAULT_CASE;
                                        }
                                    }
                                }
                            } break;
                            
                            case AST_Mul:
                            case AST_Div:
                            {
                                if (!Type_IsNumeric(common_type))
                                {
                                    //// ERROR: operator requires common numeric type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info.result = Check_Complete;
                                    info.type   = common_type;
                                    
                                    if (Type_IsFloating(common_type))
                                    {
                                        if (expression->kind == AST_Mul) info.const_val.num = BigNum_FMul(lhs, rhs, byte_size);
                                        else                             info.const_val.num = BigNum_FDiv(lhs, rhs, byte_size);
                                    }
                                    
                                    else
                                    {
                                        if (expression->kind == AST_Mul) info.const_val.num = BigNum_IMul(lhs, rhs, byte_size);
                                        else                             info.const_val.num = BigNum_IDiv(lhs, rhs, byte_size);
                                    }
                                }
                            } break;
                            
                            case AST_Rem:
                            case AST_BitwiseOr:
                            case AST_BitwiseXor:
                            case AST_BitwiseAnd:
                            case AST_ClosedRange:
                            case AST_HalfOpenRange:
                            case AST_LeftShift:
                            case AST_RightShift:
                            case AST_ArithRightShift:
                            {
                                if (!Type_IsIntegral(common_type))
                                {
                                    //// ERROR: operator requires common integral type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info.result = Check_Complete;
                                    info.type   = common_type;
                                    
                                    switch (expression->kind)
                                    {
                                        case AST_Rem:             info.const_val.num = BigNum_Rem(lhs, rhs, byte_size);         break;
                                        case AST_BitwiseOr:       info.const_val.num = BigNum_BitOr(lhs, rhs);                  break;
                                        case AST_BitwiseXor:      info.const_val.num = BigNum_BitXor(lhs, rhs);                 break;
                                        case AST_BitwiseAnd:      info.const_val.num = BigNum_BitAnd(lhs, rhs);                 break;
                                        case AST_LeftShift:       info.const_val.num = BigNum_LeftShift(lhs, rhs, byte_size);   break;
                                        case AST_RightShift:      info.const_val.num = BigNum_RightShift(lhs, rhs, byte_size);  break;
                                        case AST_ArithRightShift: info.const_val.num = BigNum_ARightShift(lhs, rhs, byte_size); break;
                                        
                                        case AST_ClosedRange:
                                        {
                                            NOT_IMPLEMENTED;
                                        } break;
                                        
                                        case AST_HalfOpenRange:
                                        {
                                            NOT_IMPLEMENTED;
                                        } break;
                                        
                                        
                                        
                                        INVALID_DEFAULT_CASE;
                                    }
                                }
                            } break;
                            
                            case AST_IsEqual:
                            case AST_IsNotEqual:
                            {
                                // TODO: pointers
                                if (!Type_IsNumeric(common_type) && !Type_IsBoolean(common_type) &&
                                    common_type != Type_Typeid && common_type != Type_Byte)
                                {
                                    //// ERROR: operator requires numeric, boolean, typeid or byte type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info.result = Check_Complete;
                                    info.type   = Type_UntypedBool;
                                    
                                    info.const_val.num = BigNum_FromU64(BigNum_IsEqual(lhs, rhs) ^ (expression->kind == AST_IsNotEqual));
                                }
                            } break;
                            
                            case AST_And:
                            case AST_Or:
                            {
                                if (!Type_IsBoolean(common_type))
                                {
                                    //// ERROR: operator requires common boolean type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info.result = Check_Complete;
                                    info.type   = Type_UntypedBool;
                                    
                                    u64 lhs_u64 = BigNum_ToU64(lhs);
                                    u64 rhs_u64 = BigNum_ToU64(rhs);
                                    
                                    if (expression->kind == AST_And) info.const_val.num = BigNum_FromU64(lhs_u64 && rhs_u64);
                                    else                             info.const_val.num = BigNum_FromU64(lhs_u64 || rhs_u64);
                                }
                            } break;
                            
                            INVALID_DEFAULT_CASE;
                        }
                    }
                }
            }
        }
        
        else if (expression->kind == AST_ArrayType)
        {
            /// Type
            NOT_IMPLEMENTED;
        }
        
        else if (PRECEDENCE_FROM_KIND(expression->kind) == 1 || PRECEDENCE_FROM_KIND(expression->kind) == 3)
        {
            Check_Info operand_info = CheckExpression(state, expression->unary_expr);
            
            if (operand_info.result != Check_Complete) info.result = operand_info.result;
            else
            {
                switch (expression->kind)
                {
                    case AST_PointerType:
                    case AST_SliceType:
                    case AST_DynArrayType:
                    {
                        if (operand_info.type != Type_Typeid)
                        {
                            //// ERROR: operand to type decorator must be of the type typeid
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            NOT_IMPLEMENTED;
                        }
                    } break;
                    
                    case AST_Reference:
                    {
                        NOT_IMPLEMENTED;
                    } break;
                    
                    case AST_Dereference:
                    {
                        NOT_IMPLEMENTED;
                    } break;
                    
                    case AST_Negation:
                    {
                        if (!Type_IsNumeric(operand_info.type))
                        {
                            //// ERROR: operand to arithmetic operator must be of numeric type
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            info = (Check_Info){
                                .result   = Check_Complete,
                                .type     = operand_info.type,
                                .is_const = operand_info.is_const,
                            };
                            
                            imm byte_size = (Type_IsTyped(info.type) ? Type_Sizeof(info.type) : -1);
                            
                            if (Type_IsFloating(info.type))
                            {
                                info.const_val.num = BigNum_FSub(Big_0, operand_info.const_val.num, byte_size);
                            }
                            else info.const_val.num = BigNum_ISub(Big_0, operand_info.const_val.num, byte_size);
                        }
                    } break;
                    
                    case AST_Complement:
                    {
                        if (!Type_IsTyped(operand_info.type) || !Type_IsIntegral(operand_info.type))
                        {
                            //// ERROR: bitwise operator requires typed integral type
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            info = (Check_Info){
                                .result    = Check_Complete,
                                .type      = operand_info.type,
                                .is_const  = operand_info.is_const,
                                .const_val.num = BigNum_Complement(operand_info.const_val.num, Type_Sizeof(info.type)),
                            };
                        }
                    } break;
                    
                    case AST_Not:
                    {
                        if (!Type_IsBoolean(operand_info.type))
                        {
                            //// ERROR: logical operator requires boolean type
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            info = (Check_Info){
                                .result    = Check_Complete,
                                .type      = operand_info.type,
                                .is_const  = operand_info.is_const,
                                .const_val.num = BigNum_FromU64(BigNum_IsEqual(operand_info.const_val.num, Big_0)),
                            };
                        }
                    } break;
                    
                    /// Const len array or slice
                    case AST_Spread:
                    {
                        NOT_IMPLEMENTED;
                    } break;
                    
                    INVALID_DEFAULT_CASE;
                }
            }
        }
        
        else
        {
            switch (expression->kind)
            {
                case AST_Identifier:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_String:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedString,
                        .is_const  = true,
                        .const_val.string = expression->string
                    };
                } break;
                
                case AST_Char:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedChar,
                        .is_const  = true,
                        .const_val.num = BigNum_FromU64(expression->character)
                    };
                } break;
                
                case AST_Int:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedInt,
                        .is_const  = true,
                        .const_val.num = expression->number
                    };
                } break;
                
                case AST_Float:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedFloat,
                        .is_const  = true,
                        .const_val.num = expression->number
                    };
                } break;
                
                case AST_Boolean:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedBool,
                        .is_const  = true,
                        .const_val.num = BigNum_FromU64(expression->boolean)
                    };
                } break;
                
                case AST_StructLiteral:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_ArrayLiteral:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Proc: 
                case AST_ProcType: 
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Struct:
                case AST_Union:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Enum:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Directive:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Subscript:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Slice:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Call:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Cast:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_ElementOf:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_UfcsOf:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Conditional:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                default:
                {
                    //// ERROR: Invalid expression kind
                    info.result = Check_Error;
                } break;
            }
        }
    }
    
    return info;
}

internal Enum8(CHECK_RESULT)
CheckScope(Checker_State* state, AST_Node* scope_node)
{
    Scope_Chain new_chain = (Scope_Chain){
        .next  = state->chain,
        .table = &scope_node->scope_statement.symbol_table
    };
    
    Checker_State new_state = *state;
    new_state.chain = &new_chain;
    
    Enum8(CHECK_RESULT) result = Check_Complete;
    
    AST_Node** link = &scope_node->scope_statement.body;
    AST_Node* decl  = *link;
    
    AST_Node** next_link;
    AST_Node* next_decl;
    
    for (; decl != 0 && result == Check_Complete;
         link = next_link, decl = next_decl)
    {
        next_link = &decl->next;
        next_decl = decl->next;
        
        result = CheckStatement(&new_state, decl);
        
        if (decl->kind == AST_When && result == Check_Complete)
        {
            ASSERT(decl->when_statement.condition->kind == AST_Boolean);
            
            AST_Node* body;
            if (decl->when_statement.condition->boolean) body = decl->when_statement.true_body;
            else                                         body = decl->when_statement.false_body;
            
            // NOTE: replace when statement node with its body
            *link = body;
            
            AST_Node** body_link = link;
            for (; *body_link != 0; body_link = &(*body_link)->next);
            *body_link = next_decl;
            //
            
            // NOTE: to avoid skipping the first statement in the newly inserted body
            next_link = link;
            next_decl = *link;
        }
    }
    
    return result;
}

internal Enum8(CHECK_RESULT)
CheckStatement(Checker_State* state, AST_Node* statement)
{
    Enum8(CHECK_RESULT) result = Check_Error;
    
    switch (statement->kind)
    {
        case AST_When:
        {
            Check_Info condition_info = CheckExpression(state, statement->when_statement.condition);
            
            if (condition_info.result != Check_Complete) result = condition_info.result;
            else
            {
                if (!Type_IsBoolean(condition_info.type))
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
                    // TODO: constant expression collapsing
#if 0
                    bool condition_val = ConstVal_ToBool(ConstVal_CastTo(condition_info.const_val, Type_Bool));
                    // NOTE: overwrite previous condition expression with the computed result
                    //       the original expression is not needed anymore, since the when statement
                    //       is disolved when the condition value has been computed
                    AST_Node* condition_node = statement->when_statement.condition;
                    ZeroStruct(condition_node);
                    condition_node->kind    = AST_Boolean;
                    condition_node->boolean = condition_val;
#endif
                    
                    result = Check_Complete;
                }
            }
        } break;
        
        case AST_IncludeDecl:
        {
            //// ERROR: Include declarations are only legal in global scope
            result = Check_Error;
        } break;
        
        case AST_Scope:
        {
            result = CheckScope(state, statement);
        } break;
        
        case AST_If:
        {
            NOT_IMPLEMENTED;
            
            Check_Info condition_info = CheckExpression(state, statement->if_statement.condition);
            if (condition_info.result != Check_Complete) result = condition_info.result;
            else
            {
                NOT_IMPLEMENTED;
            }
        } break;
        
        case AST_While:
        {
            NOT_IMPLEMENTED;
        } break;
        
        case AST_Break: 
        case AST_Continue:
        {
            NOT_IMPLEMENTED;
        } break;
        
        case AST_Defer:
        {
            NOT_IMPLEMENTED;
        } break;
        
        case AST_Return:
        {
            NOT_IMPLEMENTED;
        } break;
        
        case AST_Using:
        {
            NOT_IMPLEMENTED;
        } break;
        
        case AST_Assignment:
        {
            NOT_IMPLEMENTED;
        } break;
        
        case AST_VariableDecl:
        {
            NOT_IMPLEMENTED;
        } break;
        
        case AST_ConstantDecl:
        {
            NOT_IMPLEMENTED;
        } break;
        
        default:
        {
            //// ERROR: Invalid statement kind
            result = Check_Error;
        } break;
    }
    
    return result;
}

internal bool
MM_ResolveNextDecl(AST_Node** resolved_decl)
{
    bool did_resolve_decl = false;
    
    Checker_State state = {
        .chain          = 0,
        .in_error_sweep = false,
    };
    
    if (MM.ast != 0)
    {
        for (;;)
        {
            Enum8(CHECK_RESULT) acc_result = Check_Incomplete;
            
            AST_Node** link = &MM.ast;
            AST_Node* decl  = MM.ast;
            
            Enum8(CHECK_RESULT) result;
            AST_Node** next_link;
            AST_Node* next_decl;
            
            for (; decl != 0 && acc_result <= Check_Completing;
                 link = next_link, decl = next_decl, acc_result = MAX(acc_result, result))
            {
                result    = Check_Incomplete;
                next_link = &decl->next;
                next_decl = decl->next;
                
                if (decl->kind == AST_ConstantDecl || decl->kind == AST_VariableDecl)
                {
                    result = CheckStatement(&state, decl);
                    
                    if (result == Check_Complete)
                    {
                        did_resolve_decl = true;
                        
                        if (MM.checked_ast_last_node) MM.checked_ast_last_node->next = decl;
                        else                          MM.checked_ast                 = decl;
                        MM.checked_ast_last_node = decl;
                        
                        *resolved_decl = decl;
                        
                        break;
                    }
                }
                
                else if (decl->kind == AST_When)
                {
                    result = CheckStatement(&state, decl);
                    
                    if (result == Check_Complete)
                    {
                        ASSERT(decl->when_statement.condition->kind == AST_Boolean);
                        
                        AST_Node* body;
                        if (decl->when_statement.condition->boolean) body = decl->when_statement.true_body;
                        else                                         body = decl->when_statement.false_body;
                        
                        // NOTE: replace when statement node with its body
                        *link = body;
                        
                        AST_Node** body_link = link;
                        for (; *body_link != 0; body_link = &(*body_link)->next);
                        *body_link = next_decl;
                        //
                        
                        // NOTE: to avoid skipping the first statement in the newly inserted body
                        next_link = link;
                        next_decl = *link;
                        
                        result = Check_Completing;
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
                        *link = next_decl;
                        
                        next_link = link;
                        next_decl = *link;
                        
                        result = Check_Completing;
                    }
                }
                
                else
                {
                    //// ERROR: Illegal use of statement in global scope
                    result = Check_Error;
                }
            }
            
            if      (acc_result == Check_Complete)   break;
            else if (acc_result == Check_Completing) continue;
            else if (acc_result == Check_Incomplete)
            {
                state.in_error_sweep = true;
                continue;
            }
            
            else if (acc_result == Check_Error)
            {
                MM.encountered_errors = true;
                break;
            }
        }
    }
    
    return (did_resolve_decl && !MM.encountered_errors);
}