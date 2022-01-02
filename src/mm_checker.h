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
// NOTE: hash consing
CheckExpression(AST_Node* expr, Check_Info* info) // context info
{
    Enum8(CHECK_RESULT) result = Check_Error;
    
    if (expr->kind == AST_Identifier)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_String)
    {
        result = Check_Complete;
        
        *info = (Check_Info){
            .type      = Type_UntypedString,
            .is_const  = true,
            .const_val = ConstVal_FromString(expr->string),
        };
    }
    
    else if (expr->kind == AST_Char)
    {
        result = Check_Complete;
        
        *info = (Check_Info){
            .type      = Type_UntypedChar,
            .is_const  = true,
            .const_val = ConstVal_FromChar(expr->character),
        };
    }
    
    else if (expr->kind == AST_Int)
    {
        result = Check_Complete;
        
        *info = (Check_Info){
            .type      = Type_UntypedInt,
            .is_const  = true,
            .const_val = ConstVal_FromU64(expr->integer),
        };
    }
    
    else if (expr->kind == AST_Float)
    {
        result = Check_Complete;
        
        *info = (Check_Info){
            .type      = Type_UntypedFloat,
            .is_const  = true,
            .const_val = ConstVal_FromF64(expr->floating),
        };
    }
    
    else if (expr->kind == AST_Boolean)
    {
        result = Check_Complete;
        
        *info = (Check_Info){
            .type      = Type_UntypedBool,
            .is_const  = true,
            .const_val = ConstVal_FromBool(expr->boolean),
        };
    }
    
    else if (expr->kind == AST_StructLiteral)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_ArrayLiteral)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_Proc)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_ProcType)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_Struct)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_Union)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_Enum)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_Directive)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_Compound)
    {
        result = CheckExpression(expr->compound_expr, info);
    }
    
    else if (expr->kind == AST_ArrayType)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (PRECEDENCE_FROM_KIND(expr->kind) == 1)
    {
        Check_Info elem_info;
        Enum8(CHECK_RESULT) elem_result = CheckExpression(expr->unary_expr, &elem_info);
        
        if (elem_result != Check_Complete) result = elem_result;
        else
        {
            if (Type_StripAlias(elem_info.type) != Type_Typeid)
            {
                //// ERROR: operand to type construction operator must be of type typeid
                result = Check_Error;
            }
            
            else if (!elem_info.is_const)
            {
                //// ERROR: operand to type construction operator must be constant
                result = Check_Error;
            }
            
            else
            {
                Type_ID type = Type_Nil;
                if      (expr->kind == AST_PointerType)  type = Type_PointerTo(elem_info.type);
                else if (expr->kind == AST_SliceType)    type = Type_SliceOf(elem_info.type);
                else if (expr->kind == AST_DynArrayType) type = Type_DynArrayOf(elem_info.type);
                else INVALID_CODE_PATH;
                
                result = Check_Complete;
                
                *info = (Check_Info){
                    .type      = Type_Typeid,
                    .is_const  = true,
                    .const_val = ConstVal_FromTypeid(type)
                };
            }
        }
    }
    
    else if (PRECEDENCE_FROM_KIND(expr->kind) >= 4 && PRECEDENCE_FROM_KIND(expr->kind) <= 9 ||
             expr->kind == AST_ElementOf || expr->kind == AST_UfcsOf)
    {
        Check_Info lhs_info, rhs_info;
        Enum8(CHECK_RESULT) lhs_result = CheckExpression(expr->binary_expr.left, &lhs_info);
        
        if (lhs_result != Check_Complete) result = lhs_result;
        else
        {
            Enum8(CHECK_RESULT) rhs_result = CheckExpression(expr->binary_expr.right, &rhs_info);
            
            if (rhs_result != Check_Complete) result = rhs_result;
            else
            {
                
                /*
                AST_IsEqual = AST_FirstComparative,
                AST_IsNotEqual,
                AST_IsStrictlyLess,
                AST_IsStrictlyGreater,
                AST_IsLess,
                AST_IsGreater,
                
        expr->kind == AST_ElementOf || expr->kind == AST_UfcsOf
                */
                
                if (expr->kind == AST_ClosedRange || expr->kind == AST_HalfOpenRange)
                {
                    if (!Type_IsIntegral(lhs_info.type))
                    {
                        //// ERROR: left hand side of range operator is not of integral type
                        result = Check_Error;
                    }
                    
                    else if (!Type_IsIntegral(rhs_info.type))
                    {
                        //// ERROR: right hand side of range operator is not of integral type
                        result = Check_Error;
                    }
                    
                    else
                    {
                        Type_ID common_type = Type_CommonType(lhs_info.type, rhs_info.type);
                        
                        if (common_type == Type_Nil)
                        {
                            //// ERROR: no common type between both sides of range operator
                            result = Check_Error;
                        }
                        
                        else
                        {
                            result = Check_Error;
                            
                            *info = (Check_Info){
                                .type = common_type,
                            };
                            
                            NOT_IMPLEMENTED; // TODO: const range
                        }
                    }
                }
                
                else if (expr->kind >= AST_BitwiseAnd && expr->kind <= AST_LeftShift ||
                         expr->kind >= AST_BitwiseOr  && expr->kind <= AST_BitwiseXor)
                {
                    if (!Type_IsIntegral(lhs_info.type))
                    {
                        //// ERROR: left hand side of bitwise operator is not of an integral type
                        result = Check_Error;
                    }
                    
                    else if (!Type_IsIntegral(rhs_info.type))
                    {
                        //// ERROR: right hand side of bitwise operator is not of an integral type
                        result = Check_Error;
                    }
                    
                    else
                    {
                        Type_ID common_type = Type_CommonType(lhs_info.type, rhs_info.type);
                        
                        if (common_type == Type_Nil)
                        {
                            //// ERROR: no common type fot both sides of bitwise operator
                            result = Check_Error;
                        }
                        
                        else
                        {
                            result = Check_Complete;
                            
                            
                            *info = (Check_Info){
                                .type     = common_type,
                                .is_const = (lhs_info.is_const && rhs_info.is_const)
                            };
                            
                            u64 lhs = ConstVal_ToU64(lhs_info.const_val);
                            u64 rhs = ConstVal_ToU64(rhs_info.const_val);
                            
                            u64 val = 0;
                            
                            switch (expr->kind)
                            {
                                case AST_BitwiseAnd:           val = lhs & rhs;       break;
                                case AST_BitwiseOr:            val = lhs | rhs;       break;
                                case AST_BitwiseXor:           val = lhs ^ rhs;       break;
                                case AST_LeftShift:            val = lhs << rhs;      break;
                                case AST_RightShift:           val = lhs >> rhs;      break;
                                case AST_ArithmeticRightShift: val = (i64)lhs >> rhs; break;
                                INVALID_DEFAULT_CASE;
                            }
                            
                            if      (common_type == Type_I8)  val = (i8)val;
                            else if (common_type == Type_I16) val = (i16)val;
                            else if (common_type == Type_I32) val = (i32)val;
                            NOT_IMPLEMENTED; // TODO: implications
                            
                            info->const_val = ConstVal_FromU64(val);
                        }
                    }
                }
                
                else if (expr->kind >= AST_Mul && expr->kind <= AST_Rem ||
                         expr->kind >= AST_Add && expr->kind <= AST_Sub)
                {
                    if (!Type_IsNumeric(lhs_info.type))
                    {
                        //// ERROR: left hand side of arithmetic operator must be of numeric type
                        result = Check_Error;
                    }
                    
                    else if (!Type_IsNumeric(rhs_info.type))
                    {
                        //// ERROR: right hand side of arithmetic operator must be of numeric type
                        result = Check_Error;
                    }
                    
                    else
                    {
                        Type_ID common_type = Type_CommonType(lhs_info.type, rhs_info.type);
                        
                        if (common_type == Type_Nil)
                        {
                            //// ERROR: no common type between both sides of arithmetic operator
                            result = Check_Error;
                        }
                        
                        else
                        {
                            result = Check_Complete;
                            
                            *info = (Check_Info){
                                .type     = common_type,
                                .is_const = (lhs_info.is_const && rhs_info.is_const)
                            };
                            
                            common_type = Type_StripAlias(common_type);
                            if (Type_IsIntegral(common_type))
                            {
                                if (Type_IsSigned(common_type))
                                {
                                    i64 lhs = ConstVal_ToI64(lhs_info.const_val);
                                    i64 rhs = ConstVal_ToI64(rhs_info.const_val);
                                    
                                    i64 val = 0;
                                    
                                    switch (expr->kind)
                                    {
                                        case AST_Mul: val = lhs * rhs; break;
                                        case AST_Div: val = lhs / rhs; break;
                                        case AST_Rem: val = lhs % rhs; break;
                                        case AST_Add: val = lhs + rhs; break;
                                        case AST_Sub: val = lhs - rhs; break;
                                        INVALID_DEFAULT_CASE;
                                    }
                                    
                                    if      (common_type == Type_I8)  val = (i8)val;
                                    else if (common_type == Type_I16) val = (i16)val;
                                    else if (common_type == Type_I32) val = (i32)val;
                                    NOT_IMPLEMENTED; // TODO: implications
                                    
                                    info->const_val = ConstVal_FromI64(val);
                                }
                                
                                else
                                {
                                    u64 lhs = ConstVal_ToU64(lhs_info.const_val);
                                    u64 rhs = ConstVal_ToU64(rhs_info.const_val);
                                    
                                    u64 val = 0;
                                    
                                    switch (expr->kind)
                                    {
                                        case AST_Mul: val = lhs * rhs; break;
                                        case AST_Div: val = lhs / rhs; break;
                                        case AST_Rem: val = lhs % rhs; break;
                                        case AST_Add: val = lhs + rhs; break;
                                        case AST_Sub: val = lhs - rhs; break;
                                        INVALID_DEFAULT_CASE;
                                    }
                                    
                                    info->const_val = ConstVal_FromU64(val);
                                }
                            }
                            
                            else
                            {
                                f64 val = 0;
                                
                                f64 lhs = ConstVal_ConvertToF64(lhs_info.const_val, lhs_info.type);
                                f64 rhs = ConstVal_ConvertToF64(rhs_info.const_val, rhs_info.type);
                                
                                if (common_type == Type_Float || common_type == Type_F64)
                                {
                                    switch (expr->kind)
                                    {
                                        case AST_Mul: val = lhs * rhs; break;
                                        case AST_Div: val = lhs / rhs; break;
                                        case AST_Rem: NOT_IMPLEMENTED; break;
                                        case AST_Add: val = lhs + rhs; break;
                                        case AST_Sub: val = lhs - rhs; break;
                                        INVALID_DEFAULT_CASE;
                                    }
                                }
                                
                                else if (common_type == Type_F32)
                                {
                                    switch (expr->kind)
                                    {
                                        case AST_Mul: val = (f32)lhs * (f32)rhs; break;
                                        case AST_Div: val = (f32)lhs / (f32)rhs; break;
                                        case AST_Rem: NOT_IMPLEMENTED; break;
                                        case AST_Add: val = (f32)lhs + (f32)rhs; break;
                                        case AST_Sub: val = (f32)lhs - (f32)rhs; break;
                                        INVALID_DEFAULT_CASE;
                                    }
                                }
                                
                                else
                                {
                                    ASSERT(common_type == Type_F16);
                                    NOT_IMPLEMENTED;
                                }
                                
                                info->const_val = ConstVal_FromF64(val);
                            }
                        }
                    }
                }
                
                else if (expr->kind == AST_And || expr->kind == AST_Or)
                {
                    if (!Type_IsImplicitlyCastableTo(lhs_info.type, Type_Bool))
                    {
                        //// ERROR: lhs of logical operator cannot be implicitly converted to bool
                        result = Check_Error;
                    }
                    
                    else if (!Type_IsImplicitlyCastableTo(rhs_info.type, Type_Bool))
                    {
                        //// ERROR: rhs of logical operator cannot be implicitly converted to bool
                        result = Check_Error;
                    }
                    
                    else
                    {
                        result = Check_Complete;
                        
                        *info = (Check_Info){
                            .type     = Type_Bool,
                            .is_const = (lhs_info.is_const && rhs_info.is_const),
                        };
                        
                        bool lhs = ConstVal_ToBool(lhs_info.const_val);
                        bool rhs = ConstVal_ToBool(rhs_info.const_val);
                        
                        if (expr->kind == AST_And) info->const_val = ConstVal_FromBool(lhs && rhs);
                        else                       info->const_val = ConstVal_FromBool(lhs || rhs);
                    }
                }
                NOT_IMPLEMENTED;
            }
        }
    }
    
    else if (expr->kind == AST_Subscript)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_Slice)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_Call)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (PRECEDENCE_FROM_KIND(expr->kind) == 3)
    {
        Check_Info operand_info;
        Enum8(CHECK_RESULT) operand_result = CheckExpression(expr->unary_expr, &operand_info);
        
        if (operand_result != Check_Complete) result = operand_result;
        else
        {
            Type_ID operand_type = Type_StripAlias(operand_info.type);
            
            if (expr->kind == AST_Negation)
            {
                
                if (!Type_IsNumeric(operand_type))
                {
                    //// ERROR: operand of negation operator must be of a numeric type
                    result = Check_Error;
                }
                
                else if (Type_IsTyped(operand_type) && Type_IsIntegral(operand_type) && !Type_IsSigned(operand_type))
                {
                    //// ERROR: operand of negation operator cannot be of an unsigned integral type
                    result = Check_Error;
                }
                
                else
                {
                    result = Check_Complete;
                    
                    *info = (Check_Info){
                        .type     = operand_info.type,
                        .is_const = operand_info.is_const
                    };
                    
                    if (Type_IsIntegral(operand_type))
                    {
                        info->const_val = ConstVal_FromI64(-ConstVal_ToI64(operand_info.const_val));
                    }
                    
                    else
                    {
                        ASSERT(Type_IsFloating(operand_type));
                        info->const_val = ConstVal_FromF64(-ConstVal_ToF64(operand_info.const_val));
                    }
                }
            }
            
            else if (expr->kind == AST_Complement)
            {
                if (!Type_IsIntegral(operand_type))
                {
                    //// ERROR: operand to bitwise operator must be of integral type
                    result = Check_Error;
                }
                
                else
                {
                    result = Check_Complete;
                    
                    *info = (Check_Info){
                        .type      = operand_info.type,
                        .is_const  = operand_info.is_const,
                        .const_val = ConstVal_FromU64(~ConstVal_ToU64(operand_info.const_val)),
                    };
                }
            }
            
            else if (expr->kind == AST_Not)
            {
                if (!Type_IsImplicitlyCastableTo(operand_type, Type_Bool))
                {
                    //// ERROR: operand to logical not must be either boolean or of an integral type
                    result = Check_Error;
                }
                
                else
                {
                    result = Check_Complete;
                    
                    *info = (Check_Info){
                        .type      = operand_info.type,
                        .is_const  = operand_info.is_const,
                        .const_val = ConstVal_FromBool(!ConstVal_ToBool(operand_info.const_val))
                    };
                }
            }
        }
        // precedence 3: 60 - 79
        /*
        AST_Reference,
        AST_Dereference,
        AST_Spread,
    */
        NOT_IMPLEMENTED;
    }
    
    else if (expr->kind == AST_Conditional)
    {
        Check_Info condition_info, true_info, false_info;
        Enum8(CHECK_RESULT) condition_result = CheckExpression(expr->conditional_expr.condition, &condition_info);
        
        if (condition_result != Check_Complete) result = condition_result;
        else
        {
            Enum8(CHECK_RESULT) true_result = CheckExpression(expr->conditional_expr.true_clause, &true_info);
            
            if (true_result != Check_Complete) result = true_result;
            else
            {
                Enum8(CHECK_RESULT) false_result = CheckExpression(expr->conditional_expr.false_clause, &false_info);
                
                if (false_result != Check_Complete) result = false_result;
                else
                {
                    if (!Type_IsImplicitlyCastableTo(condition_info.type, Type_Bool))
                    {
                        //// ERROR: condition in conditional expression must be of either boolean or integral type
                        result = Check_Error;
                    }
                    
                    else
                    {
                        Type_ID common_type = Type_CommonType(true_info.type, false_info.type);
                        
                        if (common_type == Type_Nil)
                        {
                            //// ERROR: no common type for both clauses of conditional expression
                            result = Check_Error;
                        }
                        
                        else
                        {
                            result = Check_Complete;
                            
                            *info = (Check_Info){
                                .type = common_type,
                            };
                            
                            if (condition_info.is_const)
                            {
                                if (ConstVal_ToBool(condition_info.const_val))
                                {
                                    info->is_const  = true_info.is_const;
                                    info->const_val = true_info.const_val;
                                }
                                
                                else
                                {
                                    info->is_const  = false_info.is_const;
                                    info->const_val = false_info.const_val;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    else
    {
        //// ERROR: Invalid expression kind
        result = Check_Error;
    }
    
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
                        
                        else
                        {
                            decl->var_decl.symbol = MM_AddSymbol(node->identifier, Symbol_Var);
                            
                            //NOT_IMPLEMENTED;
                        }
                    }
                }
                
                if (encountered_errors) result = Check_Error;
                else                    result = Check_Completing;
            }
        }
        
        else
        {
            //NOT_IMPLEMENTED;
            result = Check_Incomplete;
        }
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
                        
                        else
                        {
                            decl->const_decl.symbol = MM_AddSymbol(node->identifier, Symbol_Const);
                            
                            //NOT_IMPLEMENTED;
                        }
                    }
                }
                
                if (encountered_errors) result = Check_Error;
                else                    result = Check_Completing;
            }
        }
        
        else
        {
            //NOT_IMPLEMENTED;
            result = Check_Incomplete;
        }
    }
    
    else if (decl->kind == AST_When)
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
                *link      = decl->next;
                decl->next = 0;
                
                bool condition_val = ConstVal_ToBool(condition_info.const_val);
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
    
    return result;
}

internal bool
MM_ResolveNextDecl(AST_Node** resolved_decl)
{
    bool encountered_errors = false;
    
    if (MM.ast != 0)
    {
        while (!encountered_errors)
        {
            Enum8(CHECK_RESULT) acc_result = Check_Incomplete;
            
            AST_Node** link = &MM.ast;
            while (*link != 0)
            {
                Enum8(CHECK_RESULT) result = CheckGlobalDeclaration(*link, link);
                
                if (result == Check_Error)
                {
                    encountered_errors = true;
                    break;
                }
                
                else if (result == Check_Complete)
                {
                    acc_result = Check_Complete;
                    
                    if (resolved_decl != 0) *resolved_decl = *link;
                    break;
                }
                
                else
                {
                    acc_result = MAX(acc_result, result);
                    link = &(*link)->next;
                }
            }
            
            if (!encountered_errors)
            {
                if (acc_result == Check_Incomplete)
                {
                    // NOTE: no progress was made
                    // TODO:
                    encountered_errors = true;
                }
                
                else if (acc_result == Check_Completing) continue;
                else
                {
                    ASSERT(acc_result == Check_Complete);
                    break;
                }
            }
            
            MM.encountered_errors = (MM.encountered_errors || encountered_errors);
        }
    }
    
    return (!MM.encountered_errors && MM.ast != 0);
}