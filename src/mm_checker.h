typedef struct MM_Checker
{
    MM_bool stub;
} MM_Checker;

typedef struct MM_Constant_Value
{
    union
    {
        MM_i128 i128;
        MM_f64 f64;
        MM_bool boolean;
        MM_String string;
        MM_Typeid typeid;
    };
} MM_Constant_Value;

typedef struct MM_Check_Result
{
    MM_bool is_valid;
    MM_bool is_constant;
    MM_Typeid type;
    MM_Constant_Value const_val;
} MM_Check_Result;

MM_Constant_Value
MM_Checker__DoArithmeticAndBitwiseBinaryIntegerOp(MM_Checker* checker, MM_Constant_Value a, MM_Constant_Value b, MM_AST_Kind operation, MM_Typeid type)
{
    MM_i128 result;
    
    MM_i128_Error_Kind error = MM_i128_Error_None;
    
    switch (operation)
    {
        case MM_AST_Mul:    result = MM_i128_Mul   (a.i128, b.i128, &error); break;
        case MM_AST_Div:    result = MM_i128_Div   (a.i128, b.i128, &error); break;
        case MM_AST_Rem:    result = MM_i128_Rem   (a.i128, b.i128, &error); break;
        case MM_AST_BitAnd: result = MM_i128_BitAnd(a.i128, b.i128, &error); break;
        case MM_AST_BitShl: result = MM_i128_BitShl(a.i128, b.i128, &error); break;
        case MM_AST_BitShr: result = MM_i128_BitShr(a.i128, b.i128, &error); break;
        case MM_AST_BitSar: result = MM_i128_BitSar(a.i128, b.i128, &error); break;
        case MM_AST_Add:    result = MM_i128_Add   (a.i128, b.i128, &error); break;
        case MM_AST_Sub:    result = MM_i128_Sub   (a.i128, b.i128, &error); break;
        case MM_AST_BitOr:  result = MM_i128_BitOr (a.i128, b.i128, &error); break;
        case MM_AST_BitXor: result = MM_i128_BitXor(a.i128, b.i128, &error); break;
        default: MM_ILLEGAL_CODE_PATH; break;
    }
    
    
    MM_umm size = MM_Typeid_Sizeof(type);
    return (MM_Constant_Value){ .i128 = MM_i128_SignExtendFromByteSize(MM_i128_MaskToByteSize(result, size), size) };
}

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
            .is_valid    = MM_true,
            .is_constant = MM_true,
            .type        = MM_Typeid_SoftInt,
            .const_val   = { .i128 = expression->int_expr.value },
        };
    }
    else if (expression->kind == MM_AST_Float)
    {
        return (MM_Check_Result){
            .is_valid    = MM_true,
            .is_constant = MM_true,
            .type        = MM_Typeid_SoftFloat,
            .const_val   = { .f64 = expression->float_expr.value },
        };
    }
    else if (expression->kind == MM_AST_Bool)
    {
        return (MM_Check_Result){
            .is_valid    = MM_true,
            .is_constant = MM_true,
            .type        = MM_Typeid_SoftBool,
            .const_val   = { .boolean = expression->bool_expr.value },
        };
    }
    else if (expression->kind == MM_AST_String)
    {
        return (MM_Check_Result){
            .is_valid    = MM_true,
            .is_constant = MM_true,
            .type        = MM_Typeid_SoftString,
            .const_val   = { .string = expression->string_expr.value },
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
                MM_Constant_Value lhs_val = ;
                MM_Constant_Value rhs_val = ;
                
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
                                    .is_valid    = MM_true,
                                    .is_constant = ,
                                    .type        = common_type,
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
                                    .is_valid    = MM_true,
                                    .is_constant = ,
                                    .type        = common_type,
                                    .const_val   = ,
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
                                    .is_valid    = MM_true,
                                    .is_constant = ,
                                    .type        = common_type,
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
                                MM_Constant_Value const_val;
                                MM_imm cmp;
                                if (MM_Typeid_IsIntegral(common_type)) cmp = MM_i128_Cmp(lhs_val.i128, rhs_val.i128);
                                else                                   cmp = (lhs_val.f64 < rhs_val.f64 ? -1 : (lhs_val.f64 > rhs_val.f64 ? 1 : 0));
                                if      (expression->kind == MM_AST_CmpLess)    const_val = (MM_Constant_Value){ .boolean = (cmp == -1)};
                                else if (expression->kind == MM_AST_CmpLessEQ)  const_val = (MM_Constant_Value){ .boolean = (cmp !=  1)};
                                else if (expression->kind == MM_AST_CmpGreater) const_val = (MM_Constant_Value){ .boolean = (cmp ==  1)};
                                else                                            const_val = (MM_Constant_Value){ .boolean = (cmp != -1)};
                                
                                return (MM_Check_Result){
                                    .is_valid    = MM_true,
                                    .is_constant = lhs_result.is_constant && rhs_result.is_constant,
                                    .type        = MM_Typeid_SoftBool,
                                    .const_val   = const_val,
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
                                MM_Constant_Value const_val;
                                if (expression->kind == MM_AST_And) const_value = (MM_Constant_Value){ .boolean = lhs_val.boolean & rhs_val.boolean };
                                else                                const_value = (MM_Constant_Value){ .boolean = lhs_val.boolean | rhs_val.boolean };
                                
                                return (MM_Check_Result){
                                    .is_valid    = MM_true,
                                    .is_constant = lhs_result.is_constant && rhs_result.is_constant,
                                    .type        = MM_Typeid_SoftBool,
                                    .const_val   = const_val,
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
                        MM_Constant_Value const_val = operand_result.const_val;
                        if (expression->kind == MM_AST_Neg)
                        {
                            MM_umm size = MM_Typeid_Sizeof(operand_result.type);
                            if      (MM_Typeid_IsIntegral(operand_result.type)) const_val = (MM_Constant_Value){ .i128 = MM_i128_MaskToByteSize(MM_i128_Neg(operand_result.const_val.i128), size) };
                            else                                                const_val = (MM_Constant_Value){ .f64  = MM_f64_MapToByteSize(-operand_result.const_val.f64, size)                };
                        }
                        
                        return (MM_Check_Result){
                            .is_valid    = MM_true,
                            .is_constant = operand_result.is_constant,
                            .type        = operand_result.type,
                            .const_val   = const_val,
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
                            .is_valid    = MM_true,
                            .is_constant = operand_result.is_constant,
                            .type        = operand_result.type,
                            .const_val   = { .i128 = MM_i128_MaskToByteSize(MM_i128_LogicalNot(operand_result.const_val.i128)) },
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
                            .is_valid    = MM_true,
                            .is_constant = operand_result.is_constant,
                            .type        = operand_result.type,
                            .const_val   = { .i128 = MM_i128_MaskToByteSize(MM_i128_BitNot(operand_result.const_val.i128), MM_Typeid_Sizeof(operand_result.type)) },
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
                    if (!MM_Typeid_IsPointer(operand_result.type))
                    {
                        //// ERROR: Operand must be of pointer type
                        MM_NOT_IMPLEMENTED;
                    }
                    else
                    {
                        // TODO: Maybe warn deref of constants?
                        return (MM_Check_Result){
                            .is_valid    = MM_true,
                            .is_constant = MM_false,
                            .type        = { .typeid = MM_Typeid_ElemType(operand_result.type) },
                        };
                    }
                } break;
                
                case MM_AST_PointerType:
                case MM_AST_SliceType:
                {
                    if (operand_result.type != MM_Typeid_Typeid || !operand_result.is_constant)
                    {
                        //// ERROR: Element type must be a constant of type typeid
                        MM_NOT_IMPLEMENTED;
                    }
                    else
                    {
                        return (MM_Check_Result){
                            .is_valid    = MM_true,
                            .is_constant = MM_true,
                            .type        = MM_Typeid_Typeid,
                            .const_val   = { .typeid = MM_Typeid_ArrayOf(operand_result.const_val.typeid) },
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
    else if (expression->kind MM_AST_ArrayType)
    {
        MM_Check_Result size_result = MM_Checker__CheckExpression(checker, expression->array_type_expr.size);
        
        if (!size_result.is_valid) return size_result;
        else
        {
            MM_Check_Result type_result = MM_Checker__CheckExpression(checker, expression->array_type_expr.type);
            
            if (!type_result.is_valid) return type_result;
            else
            {
                if (!MM_Typeid_IsIntegral(size_result.type) || !size_result.is_constant)
                {
                    //// ERROR: Size of array type must be a constant integral expression
                    MM_NOT_IMPLEMENTED;
                }
                else if (type_result.type != MM_Typeid_Typeid || !type_result.is_constant)
                {
                    //// ERROR: Element type must be a constant of type typeid
                    MM_NOT_IMPLEMENTED;
                }
                else
                {
                    return (MM_Check_Result){
                        .is_valid    = MM_true,
                        .is_constant = MM_true,
                        .type        = MM_Typeid_Typeid,
                        .const_val   = { .typeid = MM_Typeid_ArrayOf(type_result.const_val.typeid) },
                    };
                }
            }
        }
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