// TODO: the checker relies too much on the c type system
// TODO: common type might conflict with procedures without return values

// NOTE: possible casts

/// implicit
// type -> type
// type -> any
// byte -> i8 | u8 | bool
// untyped type -> default type
// untyped string -> cstring
// ^byte -> all pointers
// all pointerds -> ^byte
// untyped string -> []char | []byte

/// implicit on no information loss on round trip conversion
// all implicit casts
// untyped int -> byte
// untyped numeric | untyped char | untyped bool -> numeric | char | bool
// untyped string -> [N]char | [N]byte

/// excplicit
// all implicit casts
// aliased numeric | aliased char | aliased bool | aliased typeid | aliased byte -> aliased numeric | aliased char | aliased bool | alised byte
// all pointers -> all pointers


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
                    switch (expression->kind)
                    {
                        /// Numeric
                        case AST_Add:
                        case AST_Sub:
                        case AST_Mul:
                        case AST_Div:
                        case AST_Rem:
                        {
                            NOT_IMPLEMENTED;
                        } break;
                        
                        /// Integral
                        case AST_BitwiseOr:
                        case AST_BitwiseXor:
                        case AST_BitwiseAnd:
                        case AST_ArithmeticRightShift:
                        case AST_RightShift:
                        case AST_LeftShift:
                        case AST_ClosedRange:
                        case AST_HalfOpenRange:
                        {
                            NOT_IMPLEMENTED;
                        } break;
                        
                        /// Primitive
                        case AST_IsEqual:
                        case AST_IsNotEqual:
                        {
                            NOT_IMPLEMENTED;
                        } break;
                        
                        /// Numeric
                        case AST_IsStrictlyLess:
                        case AST_IsStrictlyGreater:
                        case AST_IsLess:
                        case AST_IsGreater:
                        {
                            NOT_IMPLEMENTED;
                        } break;
                        
                        /// Boolean
                        case AST_And:
                        case AST_Or:
                        {
                            NOT_IMPLEMENTED;
                        } break;
                        
                        INVALID_DEFAULT_CASE;
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
                    /// Type
                    case AST_PointerType:
                    case AST_SliceType:
                    case AST_DynArrayType:
                    {
                        NOT_IMPLEMENTED;
                    } break;
                    
                    case AST_Reference:
                    {
                        NOT_IMPLEMENTED;
                    } break;
                    
                    case AST_Dereference:
                    {
                        NOT_IMPLEMENTED;
                    } break;
                    
                    /// Numeric
                    case AST_Negation:
                    {
                        if (!Type_IsImplicitlyCastableToNumericType(operand_info.type))
                        {
                            //// ERROR: arithmetic operator requires numeric operand
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            if (!Type_IsImplicitlyCastableToIntegralType(operand_info.type))
                            {
                                NOT_IMPLEMENTED;
                            }
                            
                            else
                            {
                                NOT_IMPLEMENTED;
                            }
                        }
                    } break;
                    
                    /// Integral
                    case AST_Complement:
                    {
                        if (!Type_IsImplicitlyCastableToIntegralType(operand_info.type))
                        {
                            //// ERROR: bitwise operator requires integral type
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            // TODO: preserve alias
                            // TODO: compute value
                            NOT_IMPLEMENTED;
                        }
                    } break;
                    
                    /// Boolean
                    case AST_Not:
                    {
                        if (!Type_IsImplicitlyCastableTo(operand_info.type, Type_Bool))
                        {
                            //// ERROR: logical operator requires boolean type
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            NOT_IMPLEMENTED;
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
                        .const_val = ConstVal_FromUntypedString(expression->string)
                    };
                } break;
                
                case AST_Char:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedChar,
                        .is_const  = true,
                        .const_val = ConstVal_FromUntypedChar(expression->character)
                    };
                } break;
                
                case AST_Int:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedInt,
                        .is_const  = true,
                        .const_val = ConstVal_FromUntypedInt(expression->integer)
                    };
                } break;
                
                case AST_Float:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedFloat,
                        .is_const  = true,
                        .const_val = ConstVal_FromUntypedFloat(expression->floating)
                    };
                } break;
                
                case AST_Boolean:
                {
                    info = (Check_Info){
                        .result    = Check_Complete,
                        .type      = Type_UntypedBool,
                        .is_const  = true,
                        .const_val = ConstVal_FromUntypedBool(expression->boolean)
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
        
        else if (statement->kind == AST_Return)
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