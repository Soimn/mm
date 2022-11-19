typedef struct MM_Checker
{
    MM_bool stub;
} MM_Checker;

typedef struct MM_Check_Result
{
    MM_bool is_valid;
    MM_Typeid type;
    // value
    // is const
} MM_Check_Result;

MM_Check_Result
MM_Checker__CheckExpression(MM_Checker* checker, MM_Expression* expression)
{
    if (expression->kind == MM_AST_Identifier)
    {
        // MM_AST_Identifier,
        MM_NOT_IMPLEMENTED;
    }
    else if (expression->kind == MM_AST_Int)
    {
        return (MM_Check_Result){
            .is_valid = MM_true,
            .type     = MM_Typeid_SoftInt,
        };
    }
    else if (expression->kind == MM_AST_Float)
    {
        return (MM_Check_Result){
            .is_valid = MM_true,
            .type     = MM_Typeid_SoftFloat,
        };
    }
    else if (expression->kind == MM_AST_Bool)
    {
        return (MM_Check_Result){
            .is_valid = MM_true,
            .type     = MM_Typeid_SoftBool,
        };
    }
    else if (expression->kind == MM_AST_String)
    {
        return (MM_Check_Result){
            .is_valid = MM_true,
            .type     = MM_Typeid_SoftString,
        };
    }
    else if (expression->kind == MM_AST_ProcType || expression->kind == MM_AST_ProcLit)
    {
        // MM_AST_ProcType,
        // MM_AST_ProcLit,
        MM_NOT_IMPLEMENTED;
    }
    else if (expression->kind == MM_AST_StructType)
    {
        // MM_AST_StructType,
        MM_NOT_IMPLEMENTED;
    }
    else if (expression->kind == MM_AST_Compound)
    {
        return MM_Checker__CheckExpression(checker, expression->compound_expr.expr);
    }
    else if (expression->kind == MM_AST_BuiltinCall)
    {
        // MM_AST_BuiltinCall,
        MM_NOT_IMPLEMENTED;
    }
    else if (expression->kind >= MM_AST__FirstBinary && expression->kind <= MM_AST__LastBinary)
    {
        MM_Check_Result left_result = MM_Checker__CheckExpression(checker, expression->binary_expr.left);
        
        if (!left_result.is_valid) return left_result;
        else
        {
            MM_Check_Result right_result = MM_Checker__CheckExpression(checker, expression->binary_expr.right);
            
            if (!right_result.is_valid) return right_result;
            else
            {
                MM_Typeid common_type = MM_Typeid_CommonType(left_result.type, right_result.type);
                
                if (common_type == MM_Typeid_None)
                {
                    //// ERROR: No common type between left and right hand side
                    MM_NOT_IMPLEMENTED;
                }
                else
                {
                    switch (expression->kind)
                    {
                        case MM_AST_Mul:
                        case MM_AST_Div:
                        case MM_AST_Rem:
                        case MM_AST_Add:
                        case MM_AST_Sub:
                        {
                            if (!MM_Typeid_IsNumeric(common_type))
                            {
                                //// ERROR: Common type must be numeric
                                MM_NOT_IMPLEMENTED;
                            }
                            else
                            {
                                return (MM_Check_Result){
                                    .is_valid = MM_true,
                                    .type     = common_type,
                                };
                            }
                        } break;
                        
                        case MM_AST_BitAnd:
                        case MM_AST_BitOr:
                        case MM_AST_BitXor:
                        {
                            if (!MM_Typeid_IsIntegral(common_type) && !MM_Typeid_IsBoolean(common_type))
                            {
                                //// ERROR: Common type must be either integral or boolean
                                MM_NOT_IMPLEMENTED;
                            }
                            else
                            {
                                return (MM_Check_Result){
                                    .is_valid = MM_true,
                                    .type     = common_type,
                                };
                            }
                        } break;
                        
                        case MM_AST_BitShl:
                        case MM_AST_BitShr:
                        case MM_AST_BitSar:
                        {
                            if (!MM_Typeid_IsIntegral(common_type))
                            {
                                //// ERROR: Common type must be integral
                                MM_NOT_IMPLEMENTED;
                            }
                            else
                            {
                                return (MM_Check_Result){
                                    .is_valid = MM_true,
                                    .type     = common_type,
                                };
                            }
                        } break;
                        
                        // any two types? What about structs, arrays, slices?
                        case MM_AST_CmpEqual:
                        case MM_AST_CmpNotEQ:
                        {
                            MM_NOT_IMPLEMENTED;
                        } break;
                        
                        
                        case MM_AST_CmpLess:
                        case MM_AST_CmpLessEQ:
                        case MM_AST_CmpGreater:
                        case MM_AST_CmpGreaterEQ:
                        {
                            if (!MM_Typeid_IsNumeric(common_type))
                            {
                                //// ERROR: Common type must be numeric
                                MM_NOT_IMPLEMENTED;
                            }
                            else
                            {
                                return (MM_Check_Result){
                                    .is_valid = MM_true,
                                    .type     = common_type,
                                };
                            }
                        } break;
                        
                        case MM_AST_And:
                        case MM_AST_Or:
                        {
                            if (!MM_Typeid_IsBoolean(common_type))
                            {
                                //// ERROR: Common type must be boolean
                                MM_NOT_IMPLEMENTED;
                            }
                            else
                            {
                                return (MM_Check_Result){
                                    .is_valid = MM_true,
                                    .type     = common_type,
                                };
                            }
                        } break;
                        
                        default:
                        {
                            //// ERROR: Illegal expression kind
                            MM_NOT_IMPLEMENTED;
                        } break;
                    }
                }
            }
        }
    }
    else if (expression->kind >= MM_AST__FirstPrefixUnary && expression->kind <= MM_AST__LastPrefixUnary ||
             expression->kind == MM_AST_Dereference || expression->kind == MM_AST_PointerType            ||
             expression->kind == MM_AST_SliceType)
    {
        MM_Check_Result operand_result = MM_Checker__CheckExpression(expression->unary_expr.operand);
        
        if (!operand.is_valid) return operand_result;
        else
        {
            switch (expression->kind)
            {
                case MM_AST_Pos:
                case MM_AST_Neg:
                {
                    if (!MM_Typeid_IsNumeric(operand_result.type))
                    {
                        //// ERROR: Operand must be of numeric type
                        MM_NOT_IMPLEMENTED;
                    }
                    else
                    {
                        return (MM_Check_Result){
                            .is_valid = MM_true,
                            .type     = operand_result.type,
                        };
                    }
                } break;
                
                case MM_AST_Not:
                {
                    if (!MM_Typeid_IsBoolean(operand_result.type))
                    {
                        //// ERROR: Operand must be of boolean type
                        MM_NOT_IMPLEMENTED;
                    }
                    else
                    {
                        return (MM_Check_Result){
                            .is_valid = MM_true,
                            .type     = operand_result.type,
                        };
                    }
                } break;
                
                case MM_AST_BitNot:
                {
                    if (!MM_Typeid_IsIntegral(operand_result.type))
                    {
                        //// ERROR: Operand must be of integral type
                        MM_NOT_IMPLEMENTED;
                    }
                    else
                    {
                        return (MM_Check_Result){
                            .is_valid = MM_true,
                            .type     = operand_result.type,
                        };
                    }
                } break;
                
                case MM_AST_Reference:
                {
                    // l value
                    MM_NOT_IMPLEMENTED;
                } break;
                
                case MM_AST_Dereference:
                {
                    // pointer
                    MM_NOT_IMPLEMENTED;
                } break;
                
                case MM_AST_PointerType:
                case MM_AST_SliceType:
                {
                    // type
                    MM_NOT_IMPLEMENTED;
                } break;
                
                default:
                {
                    //// ERROR: Illegal expression kind
                    MM_NOT_IMPLEMENTED;
                } break;
            }
        }
    }
    else if (expression->kind MM_AST_ArrayType)
    {
        // type
        MM_NOT_IMPLEMENTED;
    }
    else if (expression->kind == MM_AST_Subscript || expression->kind == MM_AST_Slice)
    {
        // array
        // MM_AST_Subscript,
        // MM_AST_Slice,
        MM_NOT_IMPLEMENTED;
    }
    else if (expression->kind == MM_AST_Call)
    {
        // proc
        // MM_AST_Call,
        MM_NOT_IMPLEMENTED;
    }
    else if (expression->kind == MM_AST_Member)
    {
        // struct
        // MM_AST_Member,
        MM_NOT_IMPLEMENTED;
    }
    else if (expression->kind == MM_AST_StructLit || expression->kind == MM_AST_ArrayLit)
    {
        // type
        // MM_AST_StructLit,
        // MM_AST_ArrayLit,
        MM_NOT_IMPLEMENTED;
    }
    else
    {
        //// ERROR: Illegal expression kind
        MM_NOT_IMPLEMENTED;
    }
}

MM_Check_Result
MM_Checker__CheckStatement(MM_Checker* checker, MM_Statement* statement)
{
    MM_NOT_IMPLEMENTED;
}