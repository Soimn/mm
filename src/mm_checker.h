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
                    
                    Type_ID t0 = lhs_info.type;
                    Type_ID t1 = rhs_info.type;
                    
                    Const_Val lhs = lhs_info.const_val;
                    Const_Val rhs = rhs_info.const_val;
                    
                    if (Type_IsTyped(t0) && Type_IsTyped(t1))
                    {
                        if (t0 == t1) common_type = t0;
                        else NOT_IMPLEMENTED; // TODO: pointers
                    }
                    
                    else if (Type_IsTyped(t0) ^ Type_IsTyped(t1))
                    {
                        if (Type_IsTyped(t1))
                        {
                            if (!ConstVal_CoerceUntypedToType(t1, t0, rhs, &rhs))
                            {
                                //// ERROR
                                info.result = Check_Error;
                            }
                        }
                        
                        else
                        {
                            if (!ConstVal_CoerceUntypedToType(t1, t0, lhs, &lhs))
                            {
                                //// ERROR
                                info.result = Check_Error;
                            }
                        }
                    }
                    
                    else
                    {
                        if (t0 == Type_UntypedInt && t1 == Type_UntypedFloat ||
                            t1 == Type_UntypedInt && t0 == Type_UntypedFloat)
                        {
                            common_type = Type_UntypedFloat;
                            
                            if (t0 == Type_UntypedFloat) rhs.big_float = BigFloat_FromBigInt(rhs.big_int);
                            else                         lhs.big_float = BigFloat_FromBigInt(lhs.big_int);
                        }
                        
                        if (t0 == Type_UntypedChar && t1 == Type_UntypedFloat ||
                            t1 == Type_UntypedChar && t0 == Type_UntypedFloat)
                        {
                            common_type = Type_UntypedFloat;
                            
                            if (t0 == Type_UntypedFloat) rhs.big_float = BigFloat_FromF64((f64)rhs.uint64);
                            else                         lhs.big_float = BigFloat_FromF64((f64)lhs.uint64);
                        }
                        
                        else if (t0 == Type_UntypedInt  && t1 == Type_UntypedChar ||
                                 t0 == Type_UntypedChar && t1 == Type_UntypedInt)
                        {
                            common_type = Type_UntypedChar;
                            
                            Big_Int i;
                            
                            if (t0 == Type_UntypedInt)
                            {
                                i          = lhs.big_int;
                                lhs.uint64 = BigInt_ToU64(lhs.big_int);
                            }
                            
                            else
                            {
                                i          = rhs.big_int;
                                rhs.uint64 = BigInt_ToU64(rhs.big_int);
                            }
                            
                            if (BigInt_IsLess(i, BigInt_0))
                            {
                                //// ERROR: Cannot coerce negative untyped integer to untyped char
                                info.result = Check_Error;
                            }
                            
                            else if (BigInt_IsGreater(i, BigInt_FromU64(U32_MAX)))
                            {
                                //// ERROR: untyped integer is too large too fit into untyped char
                                info.result = Check_Error;
                            }
                        }
                        
                        else if (t0 == Type_UntypedInt && t1 == Type_UntypedBool ||
                                 t1 == Type_UntypedInt && t0 == Type_UntypedBool)
                        {
                            if (t0 == Type_UntypedInt) lhs.boolean = !BigInt_IsEqual(lhs.big_int, BigInt_0);
                            else                       rhs.boolean = !BigInt_IsEqual(rhs.big_int, BigInt_0);
                        }
                        
                        else if (t0 == Type_UntypedChar && t1 == Type_UntypedBool ||
                                 t1 == Type_UntypedChar && t0 == Type_UntypedBool)
                        {
                            if (t0 == Type_UntypedChar) lhs.boolean = (lhs.uint64 != 0);
                            else                        rhs.boolean = (rhs.uint64 != 0);
                        }
                    }
                    
                    if (common_type == Type_NoType)
                    {
                        //// ERROR: No common type between left and right hand side of binary operator
                        info.result = Check_Error;
                    }
                    
                    else
                    {
                        switch (expression->kind)
                        {
                            /// numeric
                            AST_Mul,
                            AST_Div,
                            
                            /// integral
                            AST_Rem,
                            AST_ClosedRange,
                            AST_HalfOpenRange,
                            
                            /// numeric, pointer
                            AST_Add,
                            AST_Sub,
                            AST_IsStrictlyLess,
                            AST_IsStrictlyGreater,
                            AST_IsLess,
                            AST_IsGreater,
                            
                            /// typed integral
                            AST_BitwiseOr,
                            AST_BitwiseXor,
                            AST_BitwiseAnd,
                            AST_ArithRightShift,
                            AST_RightShift,
                            AST_LeftShift,
                            
                            /// int, float, ~~string~~, char, any?, typeid, byte?, bool, pointer, proc....
                            AST_IsEqual,
                            AST_IsNotEqual,
                            
                            TypeInfo_Pointer,
                            TypeInfo_DynArray,
                            TypeInfo_Slice,
                            
                            TypeInfo_Struct,
                            TypeInfo_Union,
                            TypeInfo_Enum,
                            TypeInfo_Proc,
                            
                            TypeInfo_Range,
                            
                            /// boolean
                            AST_And,
                            AST_Or,
                            
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
                            
                            if (Type_IsFloating(operand_info.type))
                            {
                                if (!Type_IsTyped(operand_info.type))
                                {
                                    info.const_val.big_int = BigInt_Neg(operand_info.const_val.big_int);
                                }
                                
                                else if (operand_info.type == Type_F64)
                                {
                                    info.const_val.float64 = -operand_info.const_val.float64;
                                }
                                
                                else
                                {
                                    ASSERT(operand_info.type == Type_F32);
                                    info.const_val.float32 = -operand_info.const_val.float32;
                                }
                            }
                            
                            else
                            {
                                if (!Type_IsTyped(operand_info.type))
                                {
                                    info.const_val.big_int = BigInt_Neg(operand_info.const_val.big_int);
                                }
                                
                                else
                                {
                                    info.const_val.uint64 = (~operand_info.const_val.uint64 + 1) & BYTE_MASK(Type_Sizeof(operand_info.type));
                                }
                            }
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
                                .const_val.uint64 = ~operand_info.const_val.uint64 & BYTE_MASK(Type_Sizeof(operand_info.type))
                            };
                        }
                    } break;
                    
                    case AST_Not:
                    {
                        if (!Type_IsBoolean(operand_info.type) && !Type_IsIntegral(operand_info.type))
                        {
                            //// ERROR: logical operator requires boolean or integral type
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            info = (Check_Info){
                                .result   = Check_Complete,
                                .type     = operand_info.type,
                                .is_const = operand_info.is_const,
                            };
                            
                            if (Type_IsBoolean(operand_info.type)) info.const_val.boolean = !operand_info.const_val.boolean;
                            else
                            {
                                if (operand_info.type == Type_UntypedInt)
                                {
                                    info.const_val.big_int = BigInt_FromU64(BigInt_IsEqual(operand_info.const_val.big_int, BigInt_0));
                                }
                                
                                else info.const_val.uint64 = (info.const_val.uint64 == 0 ? 1 : 0);
                            }
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
                        .const_val.uint64 = expression->character
                    };
                } break;
                
                case AST_Int:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedInt,
                        .is_const  = true,
                        .const_val.big_int = expression->integer
                    };
                } break;
                
                case AST_Float:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedFloat,
                        .is_const  = true,
                        .const_val.big_float = expression->floating
                    };
                } break;
                
                case AST_Boolean:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedBool,
                        .is_const  = true,
                        .const_val.boolean = expression->boolean
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