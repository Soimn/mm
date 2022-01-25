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
    Scope_Index scope_index;
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
                    Type_ID common_type;
                    Const_Val lhs;
                    Const_Val rhs;
                    
                    ASSERT(Type_IsTyped(lhs_info.type) || lhs_info.is_const);
                    ASSERT(Type_IsTyped(rhs_info.type) || rhs_info.is_const);
                    if (!ConstVal_ConvertToCommonType(lhs_info.type, lhs_info.const_val, rhs_info.type, rhs_info.const_val, &common_type, &lhs, &rhs))
                    {
                        //// ERROR
                        info.result = Check_Error;
                    }
                    
                    else if (common_type == Type_NoType)
                    {
                        //// ERROR: No common type between left and right hand side of binary operator
                        info.result = Check_Error;
                    }
                    
                    else
                    {
                        Type_Info* common_type_info = MM_TypeInfoOf(common_type);
                        
                        switch (expression->kind)
                        {
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
                                    info = (Check_Info){
                                        .result   = Check_Complete,
                                        .type     = common_type,
                                        .is_const = (lhs_info.is_const && rhs_info.is_const)
                                    };
                                    
                                    if (expression->kind == AST_Mul)
                                    {
                                        if (common_type == Type_UntypedInt)
                                        {
                                            info.const_val.big_int = BigInt_Mul(lhs.big_int, rhs.big_int);
                                        }
                                        
                                        else if (common_type == Type_UntypedFloat)
                                        {
                                            info.const_val.big_float = BigFloat_Mul(lhs.big_float, rhs.big_float);
                                        }
                                        
                                        else if (common_type == Type_F64)
                                        {
                                            info.const_val.float64 = lhs.float64 * rhs.float64;
                                        }
                                        
                                        else if (common_type == Type_F32)
                                        {
                                            info.const_val.float32 = lhs.float32 * rhs.float32;
                                        }
                                        
                                        else if (Type_IsSignedInteger(common_type))
                                        {
                                            info.const_val.int64 = lhs.int64 * rhs.int64;
                                        }
                                        
                                        else
                                        {
                                            ASSERT(Type_IsIntegral(common_type));
                                            info.const_val.uint64 = lhs.uint64 * rhs.uint64;
                                        }
                                    }
                                    
                                    else
                                    {
                                        ASSERT(expression->kind == AST_Div);
                                        
                                        if (common_type == Type_UntypedInt)
                                        {
                                            if (BigInt_IsEqual(rhs.big_int, BigInt_0))
                                            {
                                                //// ERROR: division by zero
                                                info.result = Check_Error;
                                            }
                                            
                                            else
                                            {
                                                Big_Int r;
                                                info.const_val.big_int = BigInt_DivMod(lhs.big_int, rhs.big_int, &r);
                                            }
                                        }
                                        
                                        else if (Type_IsIntegral(common_type))
                                        {
                                            if (rhs.uint64 == 0)
                                            {
                                                //// ERROR: division by zero
                                                info.result = Check_Error;
                                            }
                                            
                                            else
                                            {
                                                if (Type_IsSignedInteger(common_type)) info.const_val.int64 = lhs.int64  / rhs.int64;
                                                else                                   info.const_val.int64 = lhs.uint64 / rhs.uint64;
                                            }
                                        }
                                        
                                        else if (common_type == Type_UntypedFloat)
                                        {
                                            if (BigFloat_IsEqual(rhs.big_float, BigFloat_0))
                                            {
                                                //// ERROR: division by zero
                                                info.result = Check_Error;
                                            }
                                            
                                            else
                                            {
                                                info.const_val.big_float = BigFloat_Div(lhs.big_float, rhs.big_float);
                                            }
                                        }
                                        
                                        else if (common_type == Type_F64)
                                        {
                                            if (rhs.float64 == 0)
                                            {
                                                //// ERROR: division by zero
                                                info.result = Check_Error;
                                            }
                                            
                                            else
                                            {
                                                info.const_val.float64 = lhs.float64 / rhs.float64;
                                            }
                                        }
                                        
                                        else
                                        {
                                            ASSERT(common_type == Type_F32);
                                            
                                            if (rhs.float32 == 0)
                                            {
                                                //// ERROR: division by zero
                                                info.result = Check_Error;
                                            }
                                            
                                            else
                                            {
                                                info.const_val.float32 = lhs.float32 / rhs.float32;
                                            }
                                        }
                                    }
                                }
                            } break;
                            
                            case AST_Rem:
                            {
                                if (!Type_IsIntegral(common_type))
                                {
                                    //// ERROR: operator requires common integral type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info = (Check_Info){
                                        .result   = Check_Complete,
                                        .type     = common_type,
                                        .is_const = (lhs_info.is_const && rhs_info.is_const)
                                    };
                                    
                                    if (common_type == Type_UntypedInt)
                                    {
                                        if (BigInt_IsEqual(rhs.big_int, BigInt_0))
                                        {
                                            //// ERROR: division by zero
                                            info.result = Check_Error;
                                        }
                                        
                                        else
                                        {
                                            BigInt_DivMod(lhs.big_int, rhs.big_int, &info.const_val.big_int);
                                        }
                                    }
                                    
                                    else
                                    {
                                        if (rhs.uint64 == 0)
                                        {
                                            //// ERROR: division by zero
                                            info.result = Check_Error;
                                        }
                                        
                                        else
                                        {
                                            if (Type_IsSignedInteger(common_type)) info.const_val.uint64 = lhs.int64  % rhs.int64;
                                            else                                   info.const_val.uint64 = lhs.uint64 % rhs.uint64;
                                        }
                                    }
                                }
                            } break;
                            
                            case AST_ClosedRange:
                            case AST_HalfOpenRange:
                            {
                                if (!Type_IsIntegral(common_type))
                                {
                                    //// ERROR: operator requires common integral type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info = (Check_Info){
                                        .result   = Check_Complete,
                                        .type     = (expression->kind == AST_ClosedRange 
                                                     ? Type_ClosedRangeOf(common_type)
                                                     : Type_HalfOpenRangeOf(common_type)),
                                        .is_const = (lhs_info.is_const && rhs_info.is_const)
                                    };
                                    
                                    if (info.is_const)
                                    {
                                        Big_Int start;
                                        Big_Int end;
                                        
                                        if (common_type == Type_UntypedInt)
                                        {
                                            start = lhs.big_int;
                                            end   = rhs.big_int;
                                        }
                                        
                                        else if (Type_IsSignedInteger(common_type))
                                        {
                                            start = BigInt_FromI64(lhs.int64);
                                            end   = BigInt_FromI64(rhs.int64);
                                        }
                                        
                                        else
                                        {
                                            ASSERT(Type_IsIntegral(common_type));
                                            start = BigInt_FromU64(lhs.uint64);
                                            end   = BigInt_FromU64(rhs.uint64);
                                        }
                                        
                                        if (expression->kind == AST_ClosedRange)
                                        {
                                            info.const_val.range = (Range){
                                                .start = start,
                                                .end   = end,
                                            };
                                        }
                                        
                                        else
                                        {
                                            ASSERT(expression->kind == AST_HalfOpenRange);
                                            
                                            end = BigInt_Sub(end, BigInt_FromU64(1));
                                            
                                            info.const_val.range = (Range){
                                                .start = start,
                                                .end   = end,
                                            };
                                        }
                                        
                                        if (!BigInt_IsGreater(start, end))
                                        {
                                            //// ERROR: Range does not have a positive length
                                            info.result = Check_Error;
                                        }
                                    }
                                }
                            } break;
                            
                            case AST_Add:
                            case AST_Sub:
                            {
                                if (!Type_IsNumeric(common_type) || (common_type_info == 0                      ||
                                                                     common_type_info->kind != TypeInfo_Pointer ||
                                                                     common_type_info->ptr_type != Type_Byte))
                                {
                                    //// ERROR: operator requires common numeric or byte pointer type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info = (Check_Info){
                                        .result   = Check_Complete,
                                        .type     = common_type,
                                        .is_const = (lhs_info.is_const && rhs_info.is_const)
                                    };
                                    
                                    if (expression->kind == AST_Add)
                                    {
                                        if (common_type == Type_UntypedInt)
                                        {
                                            info.const_val.big_int = BigInt_Add(lhs.big_int, rhs.big_int);
                                        }
                                        
                                        else if (common_type == Type_UntypedFloat)
                                        {
                                            info.const_val.big_float = BigFloat_Add(lhs.big_float, rhs.big_float);
                                        }
                                        
                                        else if (Type_IsSignedInteger(common_type))
                                        {
                                            info.const_val.int64 = (lhs.int64 + rhs.int64) & BYTE_MASK(Type_Sizeof(common_type));
                                        }
                                        
                                        else if (Type_IsInteger(common_type))
                                        {
                                            info.const_val.int64 = (lhs.uint64 + rhs.uint64) & BYTE_MASK(Type_Sizeof(common_type));
                                        }
                                        
                                        else if (common_type == Type_F64)
                                        {
                                            info.const_val.float64 = lhs.float64 + rhs.float64;
                                        }
                                        
                                        else if (common_type == Type_F32)
                                        {
                                            info.const_val.float32 = lhs.float32 + rhs.float32;
                                        }
                                        
                                        else
                                        {
                                            ASSERT(common_type_info != 0 && common_type_info->kind == TypeInfo_Pointer);
                                            // TODO: pointers
                                            NOT_IMPLEMENTED;
                                        }
                                    }
                                    
                                    else
                                    {
                                        if (common_type == Type_UntypedInt)
                                        {
                                            info.const_val.big_int = BigInt_Sub(lhs.big_int, rhs.big_int);
                                        }
                                        
                                        else if (common_type == Type_UntypedFloat)
                                        {
                                            info.const_val.big_float = BigFloat_Sub(lhs.big_float, rhs.big_float);
                                        }
                                        
                                        else if (Type_IsSignedInteger(common_type))
                                        {
                                            info.const_val.int64 = (lhs.int64 - rhs.int64) & BYTE_MASK(Type_Sizeof(common_type));
                                        }
                                        
                                        else if (Type_IsInteger(common_type))
                                        {
                                            info.const_val.int64 = (lhs.uint64 - rhs.uint64) & BYTE_MASK(Type_Sizeof(common_type));
                                        }
                                        
                                        else if (common_type == Type_F64)
                                        {
                                            info.const_val.float64 = lhs.float64 - rhs.float64;
                                        }
                                        
                                        else if (common_type == Type_F32)
                                        {
                                            info.const_val.float32 = lhs.float32 - rhs.float32;
                                        }
                                        
                                        else
                                        {
                                            ASSERT(common_type_info != 0 && common_type_info->kind == TypeInfo_Pointer);
                                            // TODO: pointers
                                            NOT_IMPLEMENTED;
                                        }
                                    }
                                }
                            }break;
                            
                            case AST_IsStrictlyLess:
                            case AST_IsStrictlyGreater:
                            case AST_IsLess:
                            case AST_IsGreater:
                            {
                                if (!Type_IsNumeric(common_type) || (common_type_info == 0                      ||
                                                                     common_type_info->kind != TypeInfo_Pointer))
                                {
                                    //// ERROR: operator requires common numeric or pointer type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info = (Check_Info){
                                        .result   = Check_Complete,
                                        .type     = Type_UntypedBool,
                                        .is_const = (lhs_info.is_const && rhs_info.is_const)
                                    };
                                    
                                    bool strict_less;
                                    bool strict_greater;
                                    if (common_type == Type_UntypedInt)
                                    {
                                        strict_less    = BigInt_IsLess(lhs.big_int, rhs.big_int);
                                        strict_greater = BigInt_IsGreater(lhs.big_int, rhs.big_int);
                                    }
                                    
                                    else if (Type_IsSignedInteger(common_type))
                                    {
                                        strict_less    = lhs.int64 < rhs.int64;
                                        strict_greater = lhs.int64 > rhs.int64;
                                    }
                                    
                                    else if (Type_IsIntegral(common_type))
                                    {
                                        strict_less    = lhs.uint64 < rhs.uint64;
                                        strict_greater = lhs.uint64 > rhs.uint64;
                                    }
                                    
                                    else if (common_type == Type_UntypedFloat)
                                    {
                                        strict_less    = BigFloat_IsLess(lhs.big_float, rhs.big_float);
                                        strict_greater = BigFloat_IsGreater(lhs.big_float, rhs.big_float);
                                    }
                                    
                                    else if (common_type == Type_F64)
                                    {
                                        strict_less    = lhs.float64 < rhs.float64;
                                        strict_greater = lhs.float64 > rhs.float64;
                                    }
                                    
                                    else if (common_type == Type_F32)
                                    {
                                        strict_less    = lhs.float32 < rhs.float32;
                                        strict_greater = lhs.float32 > rhs.float32;
                                    }
                                    
                                    else
                                    {
                                        ASSERT(common_type_info != 0 && common_type_info->kind == TypeInfo_Pointer);
                                        // TODO: pointers
                                        strict_less = false;
                                        strict_greater = false;
                                        NOT_IMPLEMENTED;
                                    }
                                    
                                    if      (expression->kind == AST_IsStrictlyLess)    info.const_val.boolean =  strict_less;
                                    else if (expression->kind == AST_IsStrictlyGreater) info.const_val.boolean =  strict_greater;
                                    else if (expression->kind == AST_IsLess)            info.const_val.boolean = !strict_greater;
                                    else                                                info.const_val.boolean = !strict_less;
                                }
                            } break;
                            
                            case AST_BitwiseXor:
                            {
                                if (!Type_IsBoolean(common_type) && (!Type_IsTyped(common_type) || !Type_IsIntegral(common_type)))
                                {
                                    //// ERROR: operator requires common boolean or typed integral type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info = (Check_Info){
                                        .result   = Check_Complete,
                                        .type     = common_type,
                                        .is_const = (lhs_info.is_const && rhs_info.is_const),
                                    };
                                    
                                    if (Type_IsBoolean(common_type)) info.const_val.boolean = lhs.boolean ^ rhs.boolean;
                                    else                             info.const_val.uint64  = lhs.uint64  ^ rhs.uint64;
                                }
                            } break;
                            
                            case AST_BitwiseOr:
                            case AST_BitwiseAnd:
                            case AST_ArithRightShift:
                            case AST_RightShift:
                            case AST_LeftShift:
                            {
                                if (!Type_IsTyped(common_type) || !Type_IsIntegral(common_type))
                                {
                                    //// ERROR: operator requires common typed integral type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info = (Check_Info){
                                        .result   = Check_Complete,
                                        .type     = common_type,
                                        .is_const = (lhs_info.is_const && rhs_info.is_const),
                                    };
                                    
                                    umm byte_size = Type_Sizeof(common_type);
                                    
                                    switch (expression->kind)
                                    {
                                        case AST_BitwiseOr:  info.const_val.uint64 = lhs.uint64 | rhs.uint64; break;
                                        case AST_BitwiseAnd: info.const_val.uint64 = lhs.uint64 & rhs.uint64; break;
                                        
                                        case AST_ArithRightShift:
                                        {
                                            umm shift_amount = rhs.uint64 & (8*Type_Sizeof(common_type)-1);
                                            
                                            switch (Type_Sizeof(common_type))
                                            {
                                                case 1: info.const_val.uint64 = (i8) lhs.uint64 >> shift_amount; break;
                                                case 2: info.const_val.uint64 = (i16)lhs.uint64 >> shift_amount; break;
                                                case 4: info.const_val.uint64 = (i32)lhs.uint64 >> shift_amount; break;
                                                case 8: info.const_val.uint64 = (i64)lhs.uint64 >> shift_amount; break;
                                                
                                                INVALID_DEFAULT_CASE;
                                            }
                                        } break;
                                        
                                        case AST_RightShift:
                                        {
                                            info.const_val.uint64 = (lhs.uint64 >> (rhs.uint64 & (8*byte_size-1)));
                                        } break;
                                        
                                        case AST_LeftShift:
                                        {
                                            info.const_val.uint64 = (lhs.uint64 << (rhs.uint64 & (8*byte_size-1))) & BYTE_MASK(byte_size);
                                        } break;
                                        
                                        INVALID_DEFAULT_CASE;
                                    }
                                }
                            } break;
                            
                            /// numeric, typeid, byte, bool, pointer
                            case AST_IsEqual:
                            case AST_IsNotEqual:
                            {
                                if (!Type_IsNumeric(common_type) && Type_IsBoolean(common_type) &&
                                    common_type != Type_Typeid   && common_type != Type_Byte    &&
                                    (common_type_info == 0 || common_type_info->kind != TypeInfo_Pointer))
                                {
                                    //// ERROR: operator requires common numeric, boolean, typeid, byte or pointer type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    info = (Check_Info){
                                        .result   = Check_Complete,
                                        .type     = Type_UntypedBool,
                                        .is_const = (lhs_info.is_const && rhs_info.is_const),
                                    };
                                    
                                    bool val;
                                    if      (Type_IsBoolean(common_type))      val = (lhs.boolean == rhs.boolean);
                                    else if (common_type == Type_Typeid)       val = (lhs.uint64 == rhs.uint64);
                                    else if (common_type == Type_UntypedInt)   val = BigInt_IsEqual(lhs.big_int, rhs.big_int);
                                    else if (Type_IsIntegral(common_type))     val = (lhs.uint64 == rhs.uint64);
                                    else if (common_type == Type_UntypedFloat) val = BigFloat_IsEqual(lhs.big_float, rhs.big_float);
                                    else if (common_type == Type_F64)          val = (lhs.float64 == rhs.float64);
                                    else if (common_type == Type_F32)          val = (lhs.float32 == rhs.float32);
                                    else if (common_type == Type_Byte)         val = (lhs.uint64 == rhs.uint64);
                                    else
                                    {
                                        ASSERT(common_type_info != 0 && common_type_info->kind == TypeInfo_Pointer);
                                        // TODO: pointers
                                        val = false;
                                        NOT_IMPLEMENTED;
                                    }
                                    
                                    info.const_val.boolean = val ^ (expression->kind == AST_IsNotEqual);
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
                                    info = (Check_Info){
                                        .result = Check_Complete,
                                        .type   = Type_UntypedBool,
                                    };
                                    
                                    if (expression->kind == AST_And)
                                    {
                                        if (lhs_info.is_const && !lhs.boolean)
                                        {
                                            info.is_const          = true;
                                            info.const_val.boolean = false;
                                        }
                                        
                                        else if (lhs_info.is_const && rhs_info.is_const)
                                        {
                                            info.is_const          = true;
                                            info.const_val.boolean = lhs.boolean && rhs.boolean;
                                        }
                                    }
                                    
                                    else
                                    {
                                        if (lhs_info.is_const && lhs.boolean)
                                        {
                                            info.is_const          = true;
                                            info.const_val.boolean = true;
                                        }
                                        
                                        else if (lhs_info.is_const && rhs_info.is_const)
                                        {
                                            info.is_const          = true;
                                            info.const_val.boolean = lhs.boolean || rhs.boolean;
                                        }
                                    }
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
            Check_Info type_info = CheckExpression(state, expression->array_type.elem_type);
            
            if (type_info.result != Check_Complete) info.result = type_info.result;
            else
            {
                Check_Info size_info = CheckExpression(state, expression->array_type.size);
                
                if (size_info.result != Check_Complete) info.result = size_info.result;
                else
                {
                    if (type_info.type != Type_Typeid)
                    {
                        //// ERROR: operand to type decorator must be of type typeid
                        info.result = Check_Error;
                    }
                    
                    else if (!type_info.is_const)
                    {
                        //// ERROR: operand to type decorator must be constant
                        info.result = Check_Error;
                    }
                    
                    else if (!Type_IsIntegral(size_info.type))
                    {
                        //// ERROR: size of array type decorator must be of integral type
                        info.result = Check_Error;
                    }
                    
                    else if (!size_info.is_const)
                    {
                        //// ERROR: size of array type decorator must be constant
                        info.result = Check_Error;
                    }
                    
                    else
                    {
                        info = (Check_Info){
                            .result = Check_Complete,
                            .type   = Type_Typeid,
                            .is_const = true,
                        };
                        
                        Type_ID elem_type = (Type_ID)type_info.const_val.uint64;
                        
                        if (size_info.type == Type_UntypedInt)
                        {
                            if (BigInt_IsLess(size_info.const_val.big_int, BigInt_0) || BigInt_IsEqual(size_info.const_val.big_int, BigInt_0))
                            {
                                //// ERROR: size of array type decorator must be positive
                                info.result = Check_Error;
                            }
                            
                            else if (BigInt_IsGreater(size_info.const_val.big_int, BigInt_FromU64(U64_MAX)))
                            {
                                //// ERROR: Be ready to buy 16 EB of storage
                                info.result = Check_Error;
                            }
                            
                            else
                            {
                                info.const_val.uint64 = Type_ArrayOf(elem_type, BigInt_ToU64(size_info.const_val.big_int));
                            }
                        }
                        
                        else
                        {
                            if (Type_IsSignedInteger(size_info.type) && size_info.const_val.int64 < 0 ||
                                size_info.const_val.uint64 == 0)
                            {
                                //// ERROR: size of array type decorator must be positive
                                info.result = Check_Error;
                            }
                            
                            else 
                            {
                                info.const_val.uint64 = Type_ArrayOf(elem_type, size_info.const_val.uint64);
                            }
                        }
                    }
                }
            }
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
                        
                        else if (!operand_info.is_const)
                        {
                            //// ERROR: operand to type decorator must be constant
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            info = (Check_Info){
                                .result   = Check_Complete,
                                .type     = Type_Typeid,
                                .is_const = true,
                            };
                            
                            Type_ID type = (Type_ID)operand_info.const_val.uint64;
                            if (expression->kind == AST_PointerType)    info.const_val.uint64 = Type_PointerTo(type);
                            else if (expression->kind == AST_SliceType) info.const_val.uint64 = Type_SliceOf(type);
                            else                                        info.const_val.uint64 = Type_DynArrayOf(type);
                        }
                    } break;
                    
                    case AST_Reference:
                    {
                        // TODO: l-value vs r-value
                        NOT_IMPLEMENTED;
                    } break;
                    
                    case AST_Dereference:
                    {
                        Type_Info* operand_type_info = MM_TypeInfoOf(operand_info.type);
                        if (operand_type_info == 0 || operand_type_info->kind != TypeInfo_Pointer)
                        {
                            //// ERROR: operand to dereference operator must be of pointer type
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            // TODO: constval pointers
                            info = (Check_Info){
                                .result   = Check_Complete,
                                .type     = operand_type_info->ptr_type,
                                .is_const = false,
                            };
                        }
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
                            
                            if (operand_info.type == Type_UntypedFloat)
                            {
                                info.const_val.big_int = BigInt_Neg(operand_info.const_val.big_int);
                            }
                            
                            else if (operand_info.type == Type_F64)
                            {
                                info.const_val.float64 = -operand_info.const_val.float64;
                            }
                            
                            else if (operand_info.type == Type_F32)
                            {
                                info.const_val.float32 = -operand_info.const_val.float32;
                            }
                            
                            else if (operand_info.type == Type_UntypedInt)
                            {
                                info.const_val.big_int = BigInt_Neg(operand_info.const_val.big_int);
                            }
                            
                            else
                            {
                                ASSERT(Type_IsIntegral(operand_info.type));
                                info.const_val.uint64 = (~operand_info.const_val.uint64 + 1) & BYTE_MASK(Type_Sizeof(operand_info.type));
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
                        if (!Type_IsBoolean(operand_info.type))
                        {
                            //// ERROR: logical operator requires boolean type
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            info = (Check_Info){
                                .result   = Check_Complete,
                                .type     = operand_info.type,
                                .is_const = operand_info.is_const,
                                .const_val.boolean = !operand_info.const_val.boolean
                            };
                        }
                    } break;
                    
                    case AST_Spread:
                    {
                        Type_Info* operand_type_info = MM_TypeInfoOf(operand_info.type);
                        
                        if (operand_info.type != Type_Typeid && (operand_type_info == 0 || operand_type_info->kind != TypeInfo_Array))
                        {
                            //// ERROR: operator requires array or typeid type
                            info.result = Check_Error;
                        }
                        
                        else if (!operand_info.is_const)
                        {
                            //// ERROR: operator requires constant
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            if (operand_info.type == Type_Typeid)
                            {
                                info = (Check_Info){
                                    .result   = Check_Complete,
                                    .type     = Type_Typeid,
                                    .is_const = true,
                                    .const_val.uint64 = Type_VarArgSliceOf((Type_ID)operand_info.const_val.uint64)
                                };
                            }
                            
                            else
                            {
                                // TODO: This is easy to handle from the caller, but it would be nice to handle it
                                //       without some context hack. Either way, it would be required that this, and
                                //       multiple return values, do not ruin every other type check by introducing
                                //       multiple values with one CheckExpression call.
                                NOT_IMPLEMENTED;
                            }
                        }
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
                    // TODO: How should .{} be handled?
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_ArrayLiteral:
                {
                    // TODO: How should .[] be handled?
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
                    Check_Info elem_type_info = {0};
                    if (expression->enum_type.elem_type != 0) elem_type_info = CheckExpression(state, expression->enum_type.elem_type);
                    
                    if (expression->enum_type.elem_type != 0 && elem_type_info.result != Check_Complete) info.result = elem_type_info.result;
                    else
                    {
                        if (expression->enum_type.elem_type != 0 && elem_type_info.type != Type_Typeid)
                        {
                            //// ERROR: enum member type designator must be of typeid type
                            info.result = Check_Error;
                        }
                        
                        else if (expression->enum_type.elem_type != 0 && !elem_type_info.is_const)
                        {
                            //// ERROR: enum member type designator must be constant
                            info.result = Check_Error;
                        }
                        
                        else
                        {
                            //Type_ID elem_type = (expression->enum_type.elem_type != 0 ? (Type_ID)elem_type_info.const_val.uint64 : Type_Int);
                            
                            NOT_IMPLEMENTED;
                        }
                    }
                } break;
                
                case AST_Directive:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Subscript:
                {
                    Check_Info array_info = CheckExpression(state, expression->subscript_expr.array);
                    
                    if (array_info.result != Check_Complete) info.result = array_info.result;
                    else
                    {
                        Check_Info index_info = CheckExpression(state, expression->subscript_expr.index);
                        
                        if (index_info.result != Check_Complete) info.result = index_info.result;
                        else
                        {
                            Type_Info* array_type_info = MM_TypeInfoOf(array_info.type);
                            
                            if (array_type_info == 0 || (array_type_info->kind != TypeInfo_Array    &&
                                                         array_type_info->kind != TypeInfo_Slice    &&
                                                         array_type_info->kind != TypeInfo_DynArray &&
                                                         array_type_info->kind != TypeInfo_VarArgSlice))
                            {
                                //// ERROR: operator requires array like type
                                info.result = Check_Error;
                            }
                            
                            else if (!Type_IsIntegral(index_info.type))
                            {
                                //// ERROR: index must be of integral type
                                info.result = Check_Error;
                            }
                            
                            else
                            {
                                Type_ID elem_type;
                                if (array_type_info->kind != TypeInfo_Array) elem_type = array_type_info->array.elem_type;
                                else                                         elem_type = array_type_info->elem_type;
                                
                                info = (Check_Info){
                                    .result = Check_Complete,
                                    .type   = elem_type,
                                    .is_const = (array_info.is_const && index_info.is_const),
                                };
                                
                                // TODO: perform bounds checking & const val subscript
                                NOT_IMPLEMENTED;
                            }
                        }
                    }
                } break;
                
                case AST_Slice:
                {
                    Check_Info array_info = CheckExpression(state, expression->slice_expr.array);
                    
                    if (array_info.result != Check_Complete) info.result = array_info.result;
                    else
                    {
                        Check_Info start_info = {0};
                        Check_Info end_info   = {0};
                        
                        if (expression->slice_expr.start != 0) start_info = CheckExpression(state, expression->slice_expr.start);
                        
                        if (expression->slice_expr.start != 0 && start_info.result != Check_Complete) info.result = start_info.result;
                        else
                        {
                            if (expression->slice_expr.one_after_end != 0) end_info = CheckExpression(state, expression->slice_expr.one_after_end);
                            
                            if (expression->slice_expr.one_after_end != 0 && end_info.result != Check_Complete) info.result = end_info.result;
                            else
                            {
                                Type_Info* array_type_info = MM_TypeInfoOf(array_info.type);
                                
                                if (array_type_info == 0 || (array_type_info->kind != TypeInfo_Array    &&
                                                             array_type_info->kind != TypeInfo_Slice    &&
                                                             array_type_info->kind != TypeInfo_DynArray &&
                                                             array_type_info->kind != TypeInfo_VarArgSlice))
                                {
                                    //// ERROR: operator requires array like type
                                    info.result = Check_Error;
                                }
                                
                                else if (expression->slice_expr.start != 0 && !Type_IsIntegral(start_info.type))
                                {
                                    //// ERROR: start index must be of integral type
                                    info.result = Check_Error;
                                }
                                
                                else if (expression->slice_expr.one_after_end != 0 && !Type_IsIntegral(end_info.type))
                                {
                                    //// ERROR: past end index must be of integral type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    Type_ID elem_type;
                                    if (array_type_info->kind != TypeInfo_Array) elem_type = array_type_info->array.elem_type;
                                    else                                         elem_type = array_type_info->elem_type;
                                    
                                    info = (Check_Info){
                                        .result = Check_Complete,
                                        .type   = elem_type,
                                        .is_const = (array_info.is_const && start_info.is_const && end_info.is_const),
                                    };
                                    
                                    // TODO: perform bounds checking & const val slice
                                    NOT_IMPLEMENTED;
                                }
                            }
                        }
                    }
                } break;
                
                case AST_Call:
                {
                    NOT_IMPLEMENTED;
                } break;
                
                case AST_Cast:
                {
                    Check_Info operand_info = CheckExpression(state, expression->cast_expr.operand);
                    
                    if (operand_info.result != Check_Complete) info.result = operand_info.result;
                    else
                    {
                        Check_Info type_info = CheckExpression(state, expression->cast_expr.type);
                        
                        if (type_info.result != Check_Complete) info.result = type_info.result;
                        else
                        {
                            if (type_info.type != Type_Typeid)
                            {
                                //// ERROR: target type of cast expression must be of type typeid
                                info.result = Check_Error;
                            }
                            
                            else if (!type_info.is_const)
                            {
                                //// ERROR: target type of cast expression must be constant
                                info.result = Check_Error;
                            }
                            
                            else if (!Type_IsCastableTo(operand_info.type, (Type_ID)type_info.const_val.uint64))
                            {
                                //// ERROR: operand is not castable to target type
                                info.result = Check_Error;
                            }
                            
                            else
                            {
                                Type_ID dst_type = (Type_ID)type_info.const_val.uint64;
                                
                                info = (Check_Info){
                                    .result    = Check_Complete,
                                    .type      = dst_type,
                                    .is_const  = operand_info.is_const,
                                    .const_val = ConstVal_CastTo(operand_info.const_val, dst_type)
                                };
                            }
                        }
                    }
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
                    Check_Info condition_info = CheckExpression(state, expression->conditional_expr.condition);
                    
                    if (condition_info.result != Check_Complete) info.result = condition_info.result;
                    else
                    {
                        Check_Info true_info = CheckExpression(state, expression->conditional_expr.true_clause);
                        
                        if (true_info.result != Check_Complete) info.result = true_info.result;
                        else
                        {
                            Check_Info false_info = CheckExpression(state, expression->conditional_expr.false_clause);
                            
                            if (false_info.result != Check_Complete) info.result = false_info.result;
                            else
                            {
                                if (Type_IsBoolean(condition_info.type))
                                {
                                    //// ERROR: coindition of conditional operator is required to be of boolean type
                                    info.result = Check_Error;
                                }
                                
                                else
                                {
                                    Type_ID common_type;
                                    Const_Val true_val;
                                    Const_Val false_val;
                                    
                                    ASSERT(Type_IsTyped(true_info.type) || true_info.is_const);
                                    ASSERT(Type_IsTyped(false_info.type) || false_info.is_const);
                                    if (!ConstVal_ConvertToCommonType(true_info.type, true_info.const_val, false_info.type, false_info.const_val, &common_type, &true_val, &false_val))
                                    {
                                        //// ERROR
                                        info.result = Check_Error;
                                    }
                                    
                                    else if (common_type == Type_NoType)
                                    {
                                        //// ERROR: No common type between true and false clause of conditional operator
                                        info.result = Check_Error;
                                    }
                                    
                                    else
                                    {
                                        info = (Check_Info){
                                            .result = Check_Complete,
                                            .type   = common_type,
                                        };
                                        
                                        if (condition_info.is_const)
                                        {
                                            if (condition_info.const_val.boolean)
                                            {
                                                info.is_const  = true_info.is_const;
                                                info.const_val = true_info.const_val;
                                            }
                                            
                                            else
                                            {
                                                info.is_const  = false_info.is_const;
                                                info.const_val = false_info.const_val;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } break;
                
                default:
                {
                    //// ERROR: Invalid expression kind
                    info.result = Check_Error;
                } break;
            }
        }
    }
    
    // TODO: Figure out how this should work generally
    // NOTE: Constant value folding
    if (info.result == Check_Complete && info.is_const)
    {
        if (info.type == Type_UntypedBool)
        {
            ZeroStruct(expression);
            expression->kind    = AST_Boolean;
            expression->boolean = info.const_val.boolean;
        }
        
        else if (info.type == Type_UntypedInt)
        {
            ZeroStruct(expression);
            expression->kind    = AST_Int;
            expression->integer = info.const_val.big_int;
        }
        
        else if (info.type == Type_UntypedChar)
        {
            ZeroStruct(expression);
            expression->kind      = AST_Char;
            expression->character = (Character)info.const_val.uint64;
        }
        
        else if (info.type == Type_UntypedFloat)
        {
            ZeroStruct(expression);
            expression->kind     = AST_Float;
            expression->floating = info.const_val.big_float;
        }
        
        else if (info.type == Type_UntypedString)
        {
            ZeroStruct(expression);
            expression->kind   = AST_String;
            expression->string = info.const_val.string;
        }
        
        else; // TODO: more folding
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
    
    Scope_Index scope_index = SCOPE_INDEX_FIRST_ORDERED;
    
    for (; decl != 0 && result == Check_Complete;
         link = next_link, decl = next_decl)
    {
        next_link = &decl->next;
        next_decl = decl->next;
        
        new_state.scope_index = scope_index;
        scope_index          += 1;
        
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
                
                else result = Check_Complete;
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
            NOT_IMPLEMENTED; // TODO: Check init statement
            
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
            if (statement->defer_statement == 0)
            {
                //// ERROR: missing deferred statement
                result = Check_Error;
            }
            
            else if (statement->defer_statement->kind == AST_Break        ||
                     statement->defer_statement->kind == AST_Continue     ||
                     statement->defer_statement->kind == AST_Return       ||
                     statement->defer_statement->kind == AST_Defer        ||
                     statement->defer_statement->kind == AST_Using        ||
                     statement->defer_statement->kind == AST_VariableDecl ||
                     statement->defer_statement->kind == AST_ConstantDecl ||
                     statement->defer_statement->kind == AST_IncludeDecl)
            {
                //// ERROR: illegal deferred statement
                result = Check_Error;
            }
            
            else
            {
                // TODO: disallow loop control and return inside the immediate scope and subscopes
                NOT_IMPLEMENTED;
                result = CheckStatement(state, statement->defer_statement);
            }
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
        .scope_index    = -1
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