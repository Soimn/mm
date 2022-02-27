typedef enum CHECK_RESULT
{
    Check_Incomplete = 0,
    Check_Completing,
    Check_Complete,
    Check_Error,
} CHECK_RESULT;

typedef struct Check_Context
{
    Symbol_Table* table;
} Check_Context;

typedef enum VALUE_KIND
{
    Value_Memory   = 0,
    Value_Register = 1,
    Value_Constant = 2,
} VALUE_KIND;

typedef struct Check_Info
{
    CHECK_RESULT result;
    Type_ID type;
    VALUE_KIND value_kind;
    Const_Val const_value;
} Check_Info;

internal CHECK_RESULT CheckScope(Check_Context* context, AST_Node* scope);
internal CHECK_RESULT CheckStatement(Check_Context* context, AST_Node* statement);

#define CHECK_RETURN_IF_NOT_COMPLETE(info) if ((info).result != Check_Complete) return (Check_Info){ .result = (info).result }

internal Check_Info
CheckExpression(Check_Context* context, AST_Node* expression, Type_ID guiding_type)
{
    Check_Info info = { .result = Check_Error };
    
    if (expression == 0)
    {
        //// ERROR: Missing expression
        return (Check_Info){ .result = Check_Error };
    }
    else if (expression->kind >= AST_FirstSpecial && expression->kind <= AST_LastSpecial)
    {
        //// ERROR: x is not an expression
        return (Check_Info){ .result = Check_Error };
    }
    else if (expression->kind == AST_Compound)
    {
        info = CheckExpression(context, expression->compound_expr);
    }
    else if (expression->kind >= AST_FirstBinary && expression->kind <= AST_LastBinary)
    {
        Check_Info left_info = CheckExpression(context, expression->binary_expr.left);
        CHECK_RETURN_IF_NOT_COMPLETE(left_info);
        
        Check_Info right_info = CheckExpression(context, expression->binary_expr.right);
        CHECK_RETURN_IF_NOT_COMPLETE(right_info);
        
        Type_ID common_type = Type_None;
        Const_Val lhs = left_info.const_value;
        Const_Val rhs = right_info.const_value;
        
        if (left_info.type == right_info.type)
        {
            if (expression->kind >= AST_BitAnd && expression->kind <= AST_BitShl ||
                expression->kind >= AST_BitOr  && expression->kind <= AST_BitXor &&
                left_info.type == Type_SoftInt)
            {
                common_type = Type_Harden(left_info.type);
            }
            else common_type = left_info.type;
        }
        else if (Type_IsSoft(left_info.type) && Type_IsSoft(right_info.type))
        {
            if (guiding_type == Type_None)
            {
                //// ERROR: Not enough context to harden types
                return (Check_Info){ .result = Check_Error };
            }
            else
            {
                if (!Type_HardenTo(left_info.type, guiding_type, &lhs))
                {
                    //// ERROR: Cannot harden type x to y
                    return (Check_Info){ .result = Check_Error };
                }
                else if (!Type_HardenTo(left_info.type, guiding_type, &lhs))
                {
                    //// ERROR: Cannot harden type x to y
                    return (Check_Info){ .result = Check_Error };
                }
                
                common_type = guiding_type;
            }
        }
        else if (Type_IsSoft(left_info.type) ^ Type_IsSoft(right_info.type))
        {
            Type_ID t0;
            Type_ID t1;
            Const_Val v1;
            
            if (Type_IsSoft(left_info.type))
            {
                t0 = right_info.type;
                t1 = left_info.type;
                v1 = lhs;
            }
            else
            {
                t0 = left_info.type;
                t1 = right_info.type;
                v1 = rhs;
            }
            
            if (Type_HardenTo(t1, t0, &v1))
            {
                common_type = t0;
            }
            
            if (Type_IsSoft(left_info.type)) lhs = v1;
            else                             rhs = v1;
        }
        else
        {
            //// ERROR: Incompatible types
            return (Check_Info){ .result = Check_Error };
        }
        
        if (expression->kind == AST_Mul || expression->kind == AST_Div)
        {
            if (!Type_IsNumeric(common_type))
            {
                //// ERROR: operator requires common numeric type
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result = Check_Complete,
                .type   = common_type,
                .value_kind = MAX(Value_Register, MIN(left_info.value_kind, right_info.value_kind)),
            };
            
            if (info.value_kind == Value_Constant)
            {
                if (expression->kind == AST_Mul)
                {
                    if (common_type == Type_SoftInt)
                    {
                        info.const_value.soft_int = BigInt_Mul(lhs.soft_int, rhs.soft_int);
                    }
                    else if (Type_IsUnsignedInteger(common_type))
                    {
                        info.const_value.uint64 = ChopUint(lhs.uint64 * rhs.uint64, Type_SizeOf(common_type));
                    }
                    else if (Type_IsSignedInteger(common_type))
                    {
                        umm size = Type_SizeOf(common_type);
                        info.const_value.int64 = SignExtend(ChopInt(lhs.int64 * rhs.int64, size), size);
                    }
                    else if (common_type == Type_SoftFloat)
                    {
                        info.const_value.soft_float = BigFloat_Mul(lhs.soft_float, rhs.soft_float);
                    }
                    else if (common_type == Type_F64)
                    {
                        info.const_value.float64 = lhs.float64 * rhs.float64;
                    }
                    else
                    {
                        ASSERT(common_type == Type_F32);
                        
                        info.const_value.float32 = lhs.float32 * rhs.float32;
                    }
                }
                else
                {
                    bool is_zero = false;
                    
                    if (common_type == Type_SoftInt)
                    {
                        if (BigInt_IsEqual(rhs.soft_int, BigInt_0)) is_zero = true;
                        else
                        {
                            info.const_value.soft_int = BigInt_Div(lhs.soft_int, rhs.soft_int);
                        }
                    }
                    else if (Type_IsUnsignedInteger(common_type))
                    {
                        if (rhs.uint64 == 0) is_zero = true;
                        else
                        {
                            info.const_value.uint64 = ChopUint(lhs.uint64 / rhs.uint64,
                                                               Type_SizeOf(common_type));
                        }
                    }
                    else if (Type_IsSignedInteger(common_type))
                    {
                        if (rhs.int64 == 0) is_zero = true;
                        else
                        {
                            umm size = Type_SizeOf(common_type);
                            info.const_value.int64 = SignExtend(ChopInt(lhs.int64 / rhs.int64, size), size);
                        }
                    }
                    else if (common_type == Type_SoftFloat)
                    {
                        if (BigFloat_IsEqual(rhs.soft_float, BigFloat_0) ||
                            BigFloat_IsEqual(rhs.soft_float, BigFloat_Neg0)) is_zero = true;
                        else
                        {
                            info.const_value.soft_float = BigFloat_Div(lhs.soft_float, rhs.soft_float);
                        }
                    }
                    else if (common_type == Type_F64)
                    {
                        if (AbsF64(rhs.float64) == 0) is_zero = true;
                        else
                        {
                            info.const_value.float64 = lhs.float64 / rhs.float64;
                        }
                    }
                    else
                    {
                        ASSERT(common_type == Type_F32);
                        
                        if (AbsF32(rhs.float32) == 0) is_zero = true;
                        else
                        {
                            info.const_value.float32 = lhs.float32 / rhs.float32;
                        }
                    }
                    
                    if (is_zero)
                    {
                        //// ERROR: Divide by zero
                        return (Check_Info){ .result = Check_Error };
                    }
                }
            }
        }
        else if (expression->kind == AST_Rem)
        {
            if (!Type_IsInteger(common_type))
            {
                //// ERROR: operator requires common integer type
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = common_type,
                .value_kind = MAX(Value_Register, MIN(left_info.value_kind, right_info.value_kind)),
            };
            
            if (info.value_kind == Value_Constant)
            {
                bool is_zero = false;
                
                if (common_type == Type_SoftInt)
                {
                    if (BigInt_IsEqual(rhs.soft_int, BigInt_0)) is_zero = true;
                    else
                    {
                        info.const_value.soft_int = BigInt_Rem(lhs.soft_int, rhs.soft_int);
                    }
                }
                else if (Type_IsUnsignedInteger(common_type))
                {
                    if (rhs.uint64 == 0) is_zero = true;
                    else
                    {
                        info.const_value.uint64 = lhs.uint64 % rhs.uint64;
                    }
                }
                else
                {
                    ASSERT(Type_IsSignedInteger(common_type));
                    
                    if (rhs.int64 == 0) is_zero = true;
                    else
                    {
                        info.const_value.int64 = lhs.int64 % rhs.int64;
                    }
                }
                
                if (is_zero)
                {
                    //// ERROR: Divide by zero
                    return (Check_Info){ .result = Check_Error };
                }
            }
        }
        else if (expression->kind == AST_Add || expression->kind == AST_Sub)
        {
            Type_Info* type_info = Type_InfoOf(common_type);
            
            if (!Type_IsNumeric(common_type) && type_info != 0 && type_info->kind != Type_Pointer)
            {
                //// ERROR: operator requires common numeric or pointer type
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = common_type,
                .value_kind = MAX(Value_Register, MIN(left_info.value_kind, right_info.value_kind)),
            };
            
            if (info.value_kind == Value_Constant)
            {
                if (expression->kind == AST_Add)
                {
                    if (common_type == Type_SoftInt)
                    {
                        info.const_value.soft_int = BigInt_Add(lhs.soft_int, rhs.soft_int);
                    }
                    else if (common_type == Type_SoftFloat)
                    {
                        info.const_value.soft_float = BigFloat_Add(lhs.soft_float, rhs.soft_float);
                    }
                    else if (Type_IsInteger(common_type))
                    {
                        // NOTE: Wraps on overflow
                        info.const_value.uint64 = ChopUint(lhs.uint64 + rhs.uint64, Type_SizeOf(common_type));
                    }
                    else if (common_type == Type_F32)     
                    {
                        info.const_value.float32 = lhs.float32 + rhs.float32;
                    }
                    else if (Type_IsFloat(common_type))   
                    {
                        info.const_value.float64 = lhs.float64 + rhs.float64;
                    }
                    else
                    {
                        ASSERT(type_info != 0 && type_info->kind == Type_Pointer);
                        info.const_value.pointer = lhs.pointer + rhs.pointer;
                    }
                }
                else
                {
                    if (common_type == Type_SoftInt)
                    {
                        info.const_value.soft_int = BigInt_Sub(lhs.soft_int, rhs.soft_int);
                    }
                    else if (common_type == Type_SoftFloat)
                    {
                        info.const_value.soft_float = BigFloat_Sub(lhs.soft_float, rhs.soft_float);
                    }
                    else if (Type_IsInteger(common_type))
                    {
                        // NOTE: Wraps on overflow
                        info.const_value.uint64 = ChopUint(lhs.uint64 + ~rhs.uint64 + 1,
                                                           Type_SizeOf(common_type));
                    }
                    else if (common_type == Type_F32)
                    {
                        info.const_value.float32 = lhs.float32 - rhs.float32;
                    }
                    else if (Type_IsFloat(common_type))
                    {
                        info.const_value.float64 = lhs.float64 - rhs.float64;
                    }
                    else
                    {
                        ASSERT(type_info != 0 && type_info->kind == Type_Pointer);
                        info.const_value.pointer = lhs.pointer - rhs.pointer;
                    }
                }
            }
        }
        else if (expression->kind >= AST_IsLess && expression->kind <= AST_IsGreaterEqual)
        {
            Type_Info* type_info = Type_InfoOf(common_type);
            
            if (!Type_IsNumeric(common_type) && type_info != 0 && type_info->kind != Type_Pointer)
            {
                //// ERROR: operator requires common numeric or pointer type
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = Type_Bool,
                .value_kind = MAX(Value_Register, MIN(left_info.value_kind, right_info.value_kind)),
            };
            
            if (info.value_kind == Value_Constant)
            {
                bool is_less, is_greater;
                
                if (common_type == Type_SoftInt)
                {
                    is_less    = BigInt_IsLess(lhs.soft_int, rhs.soft_int);
                    is_greater = BigInt_IsGreater(lhs.soft_int, rhs.soft_int);
                }
                else if (common_type == Type_SoftFloat)
                {
                    is_less    = BigFloat_IsLess(lhs.soft_float, rhs.soft_float);
                    is_greater = BigFloat_IsGreater(lhs.soft_float, rhs.soft_float);
                }
                else if (Type_IsUnsignedInteger(common_type))
                {
                    is_less    = (lhs.uint64 < rhs.uint64);
                    is_greater = (lhs.uint64 > rhs.uint64);
                }
                else if (Type_IsSignedInteger(common_type))
                {
                    is_less    = (lhs.int64 < rhs.int64);
                    is_greater = (lhs.int64 > rhs.int64);
                }
                else if (common_type == Type_F32)
                {
                    is_less    = (lhs.float32 < rhs.float32);
                    is_greater = (lhs.float32 > rhs.float32);
                }
                else if (Type_IsFloat(common_type))
                {
                    is_less    = (lhs.float64 < rhs.float64);
                    is_greater = (lhs.float64 > rhs.float64);
                }
                else
                {
                    ASSERT(type_info != 0 && type_info->kind == Type_Pointer);
                    is_less    = (lhs.pointer < rhs.pointer);
                    is_greater = (lhs.pointer > rhs.pointer);
                }
                
                switch (expression->kind)
                {
                    case AST_IsLess:         info.const_value.boolean = is_less;     break;
                    case AST_IsGreater:      info.const_value.boolean = is_greater;  break;
                    case AST_IsLessEqual:    info.const_value.boolean = !is_greater; break;
                    case AST_IsGreaterEqual: info.const_value.boolean = !is_less;    break;
                    INVALID_DEFAULT_CASE;
                }
            }
        }
        else if (expression->kind == AST_BitAnd ||
                 expression->kind >= AST_BitOr && expression->kind <= AST_BitXor)
        {
            if (!Type_IsInteger(common_type) && common_type != Type_Bool)
            {
                //// ERROR: operator requires common hard integer or boolean type
                return (Check_Info){ .result = Check_Error };
            }
            
            // NOTE: This is guarded against when choosing the common type
            ASSERT(!Type_IsSoft(common_type));
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = common_type,
                .value_kind = MAX(Value_Register, MIN(left_info.value_kind, right_info.value_kind)),
            };
            
            if (info.value_kind == Value_Constant)
            {
                if (common_type == Type_Bool)
                {
                    bool l = lhs.boolean;
                    bool r = rhs.boolean;
                    
                    if      (expression->kind == AST_BitOr)  info.const_value.boolean = l | r;
                    else if (expression->kind == AST_BitXor) info.const_value.boolean = l ^ r;
                    else                                     info.const_value.boolean = l & r;
                }
                else
                {
                    u64 l = lhs.uint64;
                    u64 r = rhs.uint64;
                    
                    if      (expression->kind == AST_BitOr)  info.const_value.uint64 = l | r;
                    else if (expression->kind == AST_BitXor) info.const_value.uint64 = l ^ r;
                    else                                     info.const_value.uint64 = l & r;
                }
            }
        }
        else if (expression->kind >= AST_BitSar && expression->kind <= AST_BitShl)
        {
            if (!Type_IsInteger(common_type))
            {
                //// ERROR: operator requires common hard integer type
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = common_type,
                .value_kind = MAX(Value_Register, MIN(left_info.value_kind, right_info.value_kind)),
            };
            
            if (info.value_kind == Value_Constant)
            {
                u64 l = lhs.uint64;
                u64 r = rhs.uint64;
                
                umm size = Type_SizeOf(common_type);
                ASSERT(size >= 1 && size <= 8);
                
                u64 masks[] = {
                    0x07,
                    0x0F,
                    0x1F,
                    0x3F,
                };
                
                l  = SignExtend(l, size);
                r &= masks[size - 1];
                
                u64 result;
                if      (expression->kind == AST_BitSar) result = (u64)((i64)l >> r);
                else if (expression->kind == AST_BitShr) result = l >> r;
                else if (expression->kind == AST_BitShl) result = l << r;
                else
                {
                    result = l << r | (((l & 1) << r) - 1);
                }
                
                info.const_value.uint64 = ChopInt(result, size);
            }
        }
        else if (expression->kind == AST_IsEqual || expression->kind == AST_IsNotEqual)
        {
            Type_Info* type_info = Type_InfoOf(common_type);
            
            if (common_type == Type_Any ||
                !Type_IsPrimitive(common_type) && type_info != 0 && type_info->kind != Type_Pointer)
            {
                //// ERROR: operator requires common non any primitive or pointer type
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = Type_Bool,
                .value_kind = MAX(Value_Register, MIN(left_info.value_kind, right_info.value_kind)),
            };
            
            if (info.value_kind == Value_Constant)
            {
                if (common_type == Type_SoftInt)
                {
                    info.const_value.boolean = BigInt_IsEqual(lhs.soft_int, rhs.soft_int);
                }
                else if (common_type == Type_SoftFloat)
                {
                    info.const_value.boolean = BigFloat_IsEqual(lhs.soft_float, rhs.soft_float);
                }
                else if (common_type == Type_Byte)    info.const_value.boolean = (lhs.byte    == rhs.byte);
                else if (common_type == Type_Typeid)  info.const_value.boolean = (lhs.type_id == rhs.type_id);
                else if (Type_IsInteger(common_type)) info.const_value.boolean = (lhs.uint64  == rhs.uint64);
                else if (common_type == Type_F32)     info.const_value.boolean = (lhs.float32 == rhs.float32);
                else if (Type_IsFloat(common_type))   info.const_value.boolean = (lhs.float64 == rhs.float64);
                else if (common_type == Type_String)  info.const_value.boolean = (lhs.string  == rhs.string);
                else if (common_type == Type_Bool)    info.const_value.boolean = (lhs.boolean == rhs.boolean);
                else
                {
                    ASSERT(type_info != 0 && type_info->kind == Type_Pointer);
                    info.const_value.boolean = (lhs.pointer == rhs.pointer);
                }
            }
        }
        else
        {
            ASSERT(expression->kind == AST_And || expression->kind == AST_Or);
            
            if (common_type != Type_Bool)
            {
                //// ERROR: operator requires common boolean type
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = Type_Bool,
                .value_kind = MAX(Value_Register, MIN(left_info.value_kind, right_info.value_kind)),
            };
            
            if (info.value_kind == Value_Constant)
            {
                bool l = lhs.boolean;
                bool r = rhs.boolean;
                
                if (expression->kind == AST_And) info.const_value.boolean = l && r;
                else                             info.const_value.boolean = l || r;
            }
        }
    }
    else if (expression->kind >= AST_FirstPrefixLevel && expression->kind <= AST_LastPrefixLevel)
    {
        Check_Info operand_info = CheckExpression(context, expression->array_type.type);
        CHECK_RETURN_IF_NOT_COMPLETE(operand_info);
        
        if (expression->kind == AST_Reference)
        {
            // TODO: Is This correct?
            if (operand_info.value_kind != Value_Memory)
            {
                //// ERROR: operand to reference operator must be an l value
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = Type_PointerTo(operand_info.type),
                .value_kind = Value_Memory,
            };
        }
        else if (expression->kind == AST_Dereference)
        {
            Type_Info* type_info = Type_InfoOf(operand_info.type);
            
            if (type_info == 0 || type_info->kind != Type_Pointer)
            {
                //// ERROR: operand to dereference operator must be of pointer type
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = type_info->elem_type,
                .value_kind = Value_Memory,
            };
        }
        else if (expression->kind == AST_Neg)
        {
            if (!Type_IsNumeric(operand_info.type))
            {
                //// ERROR: operand to arithmetic negation operator must be of numeric type
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = operand_info.type,
                .value_kind = MAX(operand_info.value_kind, Value_Register),
            };
            
            if (info.value_kind == Value_Constant)
            {
                if (info.type == Type_SoftFloat)
                {
                    info.const_value.soft_float = BigFloat_Neg(operand_info.const_value.soft_float);
                }
                else if (info.type == Type_F32)
                {
                    info.const_value.float32 = -operand_info.const_value.float32;
                }
                else if (info.type == Type_F64)
                {
                    info.const_value.float64 = -operand_info.const_value.float64;
                }
                else if (info.type == Type_SoftInt)
                {
                    info.const_value.soft_int = BigInt_Neg(operand_info.const_value.soft_int);
                }
                else if (Type_IsUnsignedInteger(info.type))
                {
                    info.const_value.uint64 = ~operand_info.const_value.uint64 + 1;
                }
                else
                {
                    ASSERT(Type_IsSignedInteger(info.type));
                    
                    info.const_value.int64 = -operand_info.const_value.int64;
                }
            }
        }
        else if (expression->kind == AST_BitNot)
        {
            if (!Type_IsInteger(operand_info.type))
            {
                //// ERROR: operand to bitwise not operator must be of integer type
                return (Check_Info){ .result = Check_Error };
            }
            else if (Type_IsSoft(operand_info.type))
            {
                //// ERROR: operand to bitwise not operator must be a hard type
                return (Check_Info){ .result = Check_Error };
            }
            else
            {
                info = (Check_Info){
                    .result     = Check_Complete,
                    .type       = operand_info.type,
                    .value_kind = MAX(operand_info.value_kind, Value_Register),
                };
                
                if (info.value_kind == Value_Constant)
                {
                    info.const_value.uint64 = ~operand_info.const_value.uint64;
                }
            }
        }
        else if (expression->kind == AST_Not)
        {
            if (operand_info.type != Type_Bool)
            {
                //// ERROR: operand to logical not operator must be of boolean type
                return (Check_Info){ .result = Check_Error };
            }
            else
            {
                info = (Check_Info){
                    .result     = Check_Complete,
                    .type       = operand_info.type,
                    .value_kind = MAX(operand_info.value_kind, Value_Register),
                };
                
                if (info.value_kind == Value_Constant)
                {
                    info.const_value.boolean = !operand_info.const_value.boolean;
                }
            }
        }
        else INVALID_CODE_PATH;
    }
    else if (expression->kind >= AST_FirstTypeLevel && expression->kind <= AST_LastTypeLevel)
    {
        if (expression->kind == AST_ArrayType)
        {
            Check_Info operand_info = CheckExpression(context, expression->array_type.type);
            CHECK_RETURN_IF_NOT_COMPLETE(operand_info);
            
            Check_Info size_info = CheckExpression(context, expression->array_type.size);
            CHECK_RETURN_IF_NOT_COMPLETE(size_info);
            
            if (operand_info.type != Type_Typeid)
            {
                //// ERROR: operand of type descriptor must be of type typeid
                return (Check_Info){ .result = Check_Error };
            }
            else if (operand_info.value_kind != Value_Constant)
            {
                //// ERROR: operand of type descriptor must be constant
                return (Check_Info){ .result = Check_Error };
            }
            else if (Type_IsInteger(size_info.type))
            {
                //// ERROR: size of array type descriptor must be of integer type
                return (Check_Info){ .result = Check_Error };
            }
            else if (size_info.value_kind != Value_Constant)
            {
                //// ERROR: size of array type descriptor must be constant
                return (Check_Info){ .result = Check_Error };
            }
            else if (Type_IsSignedInteger(size_info.type) && (size_info.value_kind != Value_Constant || size_info.const_value.int64 < 0) ||
                     size_info.type == Type_SoftInt && !BigInt_IsGreater(size_info.const_value.soft_int, BigInt_0))
            {
                //// ERROR: size of array type descriptor must be positive
                return (Check_Info){ .result = Check_Error };
            }
            else if (size_info.type == Type_SoftInt && BigInt_IsGreater(size_info.const_value.soft_int, BigInt_U64_MAX))
            {
                //// ERROR: Buy more ram
                return (Check_Info){ .result = Check_Error };
            }
            else
            {
                u64 size = info.const_value.uint64;
                if (size_info.type == Type_SoftInt) size = BigInt_ToU64(size_info.const_value.soft_int);
                
                info = (Check_Info){
                    .result              = Check_Complete,
                    .type                = Type_Typeid,
                    .value_kind          = Value_Constant,
                    .const_value.type_id = Type_ArrayOf(operand_info.type, size),
                };
            }
        }
        else
        {
            Check_Info operand_info = CheckExpression(context, expression->unary_expr);
            CHECK_RETURN_IF_NOT_COMPLETE(operand_info);
            
            if (operand_info.type != Type_Typeid)
            {
                //// ERROR: operand of type descriptor must be of type typeid
                return (Check_Info){ .result = Check_Error };
            }
            else if (operand_info.value_kind != Value_Constant)
            {
                //// ERROR: operand of type descriptor must be constant
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = Type_Typeid,
                .value_kind = Value_Constant,
            };
            
            Type_ID elem_type = operand_info.const_value.type_id;
            if      (expression->kind == AST_PointerType) info.const_value.type_id = Type_PointerTo(elem_type);
            else if (expression->kind == AST_SliceType)   info.const_value.type_id = Type_SliceOf(elem_type);
            else INVALID_CODE_PATH;
        }
    }
    else if (expression->kind == AST_Identifier)
    {
        Symbol* symbol = SymbolTable_GetSymbol(context->table, expression->identifier);
        
        if (symbol == 0) info.result = Check_Incomplete;
        else
        {
            if (symbol->kind == Symbol_Var)
            {
                info = (Check_Info){
                    .result     = Check_Complete,
                    .type       = symbol->variable.type,
                    .value_kind = Value_Memory,
                };
            }
            else if (symbol->kind == Symbol_Parameter)
            {
                info = (Check_Info){
                    .result     = Check_Complete,
                    .type       = symbol->parameter.type,
                    .value_kind = Value_Register,
                };
            }
            else if (symbol->kind == Symbol_Const)
            {
                info = (Check_Info){
                    .result      = Check_Complete,
                    .type        = symbol->constant.type,
                    .value_kind  = Value_Constant,
                    .const_value = symbol->constant.value,
                };
            }
            else INVALID_CODE_PATH;
        }
    }
    else if (expression->kind == AST_String)
    {
        info = (Check_Info){
            .result             = Check_Complete,
            .type               = Type_String,
            .value_kind         = Value_Constant,
            .const_value.string = expression->string,
        };
    }
    else if (expression->kind == AST_Int)
    {
        info = (Check_Info){
            .result               = Check_Complete,
            .type                 = Type_SoftInt,
            .value_kind           = Value_Constant,
            .const_value.soft_int = expression->integer,
        };
    }
    else if (expression->kind == AST_Float)
    {
        info = (Check_Info){
            .result                 = Check_Complete,
            .type                   = Type_SoftFloat,
            .value_kind             = Value_Constant,
            .const_value.soft_float = expression->floating,
        };
    }
    else if (expression->kind == AST_Boolean)
    {
        info = (Check_Info){
            .result              = Check_Complete,
            .type                = Type_Bool,
            .value_kind          = Value_Constant,
            .const_value.boolean = expression->boolean,
        };
    }
    else if (expression->kind == AST_Proc)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_ProcLiteral)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Struct)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Union)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Enum)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Call)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_StructLiteral)
    {
        Type_ID type = Type_None;
        
        if (expression->struct_literal.type == 0)
        {
            if (guiding_type == Type_None)
            {
                //// ERROR: no guiding type for struct literal without type specifier
                return (Check_Info){ .result = Check_Error };
            }
            else type = guiding_type;
        }
        else
        {
            Check_Info struct_info = CheckExpression(context, expression->struct_literal.type);
            CHECK_RETURN_IF_NOT_COMPLETE(struct_info);
            
            if (struct_info.type != Type_Typeid)
            {
                //// ERROR: type specifier of array literal must be of type typeid
                return (Check_Info){ .result = Check_Error };
            }
            else if (struct_info.value_kind != Value_Constant)
            {
                //// ERROR: type specifier of array literal must be constant
                return (Check_Info){ .result = Check_Error };
            }
            
            type = struct_info.const_value.type_id;
        }
        
        Type_Info* type_info = Type_InfoOf(guiding_type);
        
        if (type_info->kind != Type_Struct && type_info->kind != Type_Union &&
            type_info->kind != Type_Array && type_info->kind != Type_Slice)
        {
            //// ERROR: struct literals can only be used to create struct, union, array and slice types
            return (Check_Info){ .result = Check_Error };
        }
        else
        {
            NOT_IMPLEMENTED;
        }
    }
    else if (expression->kind == AST_ArrayLiteral)
    {
        Type_ID type = Type_None;
        
        if (expression->array_literal.type == 0)
        {
            if (guiding_type == Type_None)
            {
                //// ERROR: no guiding type for array literal without type specifier
                return (Check_Info){ .result = Check_Error };
            }
            
            type = guiding_type;
        }
        else
        {
            Check_Info array_info = CheckExpression(context, expression->array_literal.type);
            CHECK_RETURN_IF_NOT_COMPLETE(array_info);
            
            if (array_info.type != Type_Typeid)
            {
                //// ERROR: type specifier of array literal must be of type typeid
                return (Check_Info){ .result = Check_Error };
            }
            else if (array_info.value_kind != Value_Constant)
            {
                //// ERROR: type specifier of array literal must be constant
                return (Check_Info){ .result = Check_Error };
            }
            
            type = array_info.const_value.type_id;
        }
        
        umm num_args  = 0;
        umm num_elems = 0;
        for (AST_Node* arg = expression->array_literal.args; arg != 0; arg = arg->next)
        {
            if (arg->kind != AST_NamedValue)
            {
                //// ERROR: element in array literal must be a named value
                return (Check_Info){ .result = Check_Error };
            }
            else if (arg->named_value.value == 0)
            {
                //// ERROR: element in array literal must have a value
                return (Check_Info){ .result = Check_Error };
            }
            
            umm start_index = 0;
            umm end_index   = 0;
            if (arg->named_value.name == 0)
            {
                start_index = num_elems;
                end_index   = num_elems;
            }
            else
            {
                if (arg->named_value.name->kind != AST_Range)
                {
                    //// ERROR: name of named array element must be a range
                    return (Check_Info){ .result = Check_Error };
                }
                
                Check_Info start_info = CheckExpression(context, arg->named_value.name->range.start);
                CHECK_RETURN_IF_NOT_COMPLETE(start_info);
                
                Check_Info end_info = CheckExpression(context, arg->named_value.name->range.end);
                CHECK_RETURN_IF_NOT_COMPLETE(end_info);
                
                // TODO: Type coercion
                if (!Type_IsInteger(start_info.type) || !Type_IsInteger(end_info.type))
                {
                    //// ERROR: Both sides of range must be of integer type
                    return (Check_Info){ .result = Check_Error };
                }
                else if (start_info.value_kind != Value_Constant || end_info.value_kind != Value_Constant)
                {
                    //// ERROR: Both sides of range must be constant
                    return (Check_Info){ .result = Check_Error };
                }
                else if (Type_IsSignedInteger(start_info.type) && start_info.const_value.int64 < 0 ||
                         Type_IsSignedInteger(end_info.type)   && end_info.const_value.int64   < 0)
                {
                    //// ERROR: Index specifier of named element in array literal cannot be negative
                    return (Check_Info){ .result = Check_Error };
                }
                else if (start_info.type == Type_SoftInt &&
                         BigInt_IsGreater(start_info.const_value.soft_int, BigInt_U64_MAX))
                {
                    //// ERROR: Buy more ram
                    return (Check_Info){ .result = Check_Error };
                }
                else if (end_info.type == Type_SoftInt &&
                         BigInt_IsGreater(end_info.const_value.soft_int, BigInt_U64_MAX))
                {
                    //// ERROR: Buy more ram
                    return (Check_Info){ .result = Check_Error };
                }
                
                if (start_info.type == Type_SoftInt) start_index = BigInt_ToU64(start_info.const_value.soft_int);
                else                                 start_index = start_info.const_value.uint64;
                
                if (end_info.type == Type_SoftInt) end_index = BigInt_ToU64(end_info.const_value.soft_int);
                else                               end_index = end_info.const_value.uint64;
                
                end_index = (end_index == 0 ? 0 : end_index - arg->named_value.name->range.is_open);
                
                if (end_index < start_index)
                {
                    //// ERROR: End index of range is less than start index
                    return (Check_Info){ .result = Check_Error };
                }
                else if (start_index < num_elems)
                {
                    //// ERROR: Overlapping ranges in array literal
                    return (Check_Info){ .result = Check_Error };
                }
            }
            
            Check_Info value_info = CheckExpression(context, arg->named_value.value);
            CHECK_RETURN_IF_NOT_COMPLETE(value_info);
            
            // TODO: Type coercion
            if (value_info.type != type)
            {
                //// ERROR: Invalid type in array literal
                return (Check_Info){ .result = Check_Error };
            }
            else
            {
                num_elems = end_index + 1;
                num_args += 1;
            }
        }
        
        info = (Check_Info){
            .result = Check_Complete,
            .type = Type_ArrayOf(type, num_elems),
            .value_kind = Value_Register, // TODO: Support contant array literals
        };
    }
    else if (expression->kind == AST_ElementOf)
    {
        Type_ID type           = Type_None;
        Type_Info* type_info   = 0;
        Check_Info struct_info = {0};
        if (expression->element_of.structure == 0)
        {
            if (guiding_type == Type_None)
            {
                //// ERROR: Cannot determine type of implicit selector expression
                return (Check_Info){ .result = Check_Error };
            }
            else
            {
                type_info = Type_InfoOf(guiding_type);
                
                if (type_info == 0 || type_info->kind != Type_Enum)
                {
                    //// ERROR: Selector expressions can only be used on enumerations
                    return (Check_Info){ .result = Check_Error };
                }
                
                type = guiding_type;
            }
        }
        else
        {
            struct_info = CheckExpression(context, expression->element_of.structure);
            CHECK_RETURN_IF_NOT_COMPLETE(struct_info);
            
            type_info = Type_InfoOf(struct_info.type);
            
            if (type_info == 0)
            {
                if (struct_info.type != Type_Typeid                   ||
                    struct_info.value_kind != Value_Constant          ||
                    Type_IsPrimitive(struct_info.const_value.type_id) ||
                    Type_InfoOf(struct_info.const_value.type_id)->kind != Type_Enum)
                {
                    //// ERROR: Illegal use of element of operator
                    return (Check_Info){ .result = Check_Error };
                }
                
                type      = struct_info.const_value.type_id;
                type_info = Type_InfoOf(type);
            }
            else if (type_info->kind == Type_Struct || type_info->kind == Type_Union)
            {
                type = struct_info.type;
            }
            else
            {
                //// ERROR: Illegal use of element of operator
                return (Check_Info){ .result = Check_Error };
            }
        }
        
        if (type_info->kind == Type_Enum)
        {
            Symbol* symbol = SymbolTable_GetSymbol(&type_info->enumeration.members, expression->element_of.element);
            
            if (symbol == 0)
            {
                //// ERROR: No enum member with the name x
                return (Check_Info){ .result = Check_Error };
            }
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = type,
                .value_kind = Value_Constant,
            };
            
            Type_ID base_type    = type_info->enumeration.backing_type;
            Const_Val member_val = symbol->enum_member.value;
            if      (base_type == Type_SoftInt)         info.const_value.soft_int = member_val.soft_int;
            else if (Type_IsUnsignedInteger(base_type)) info.const_value.uint64   = member_val.uint64;
            else                                        info.const_value.int64    = member_val.int64;
        }
        else
        {
            ASSERT(type_info->kind == Type_Struct || type_info->kind == Type_Union);
            
            // TODO: Incomplete elements (e.g. circular pointers)
            Symbol* symbol = SymbolTable_GetSymbol(&type_info->structure.members, expression->element_of.element);
            
            if (symbol == 0)
            {
                //// ERROR: Struct/union has no element named x
                return (Check_Info){ .result = Check_Error };
            }
            
            // TODO: Maybe stop mixing asserts and error returns in prep for metprogramming?
            ASSERT(symbol->kind == Symbol_Var);
            
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = symbol->variable.type,
                .value_kind = struct_info.value_kind,
            };
            
            if (info.value_kind == Value_Constant)
            {
                NOT_IMPLEMENTED;
            }
        }
    }
    else if (expression->kind == AST_Conditional)
    {
        Check_Info condition_info = CheckExpression(context, expression->conditional_expr.condition);
        CHECK_RETURN_IF_NOT_COMPLETE(condition_info);
        
        Check_Info true_info = CheckExpression(context, expression->conditional_expr.true_clause);
        CHECK_RETURN_IF_NOT_COMPLETE(condition_info);
        
        Check_Info false_info = CheckExpression(context, expression->conditional_expr.false_clause);
        CHECK_RETURN_IF_NOT_COMPLETE(condition_info);
        
        if (condition_info.type != Type_Bool)
        {
            //// ERROR: Condition of conditional expression must be of boolean type
            return (Check_Info){ .result = Check_Error };
        }
        
        // TODO: Type coercion
        Type_ID common_type = true_info.type;
        Const_Val true_val  = {0};
        Const_Val false_val = {0};
        if (true_info.type != false_info.type)
        {
            //// ERROR: Both clauses of conditional expression must share a common type 
            return (Check_Info){ .result = Check_Error };
        }
        
        info = (Check_Info){
            .result     = Check_Complete,
            .type       = common_type,
            .value_kind = MAX(Value_Register, condition_info.value_kind),
        };
        
        if (info.value_kind == Value_Constant)
        {
            info.const_value = (condition_info.const_value.boolean ? true_val : false_val);
        }
    }
    else if (expression->kind == AST_Cast)
    {
        Type_ID type = Type_None;
        
        if (expression->cast_expr.type == 0)
        {
            if (guiding_type == Type_None)
            {
                //// ERROR: Cannot infer target type of cast expression without a guiding type
                return (Check_Info){ .result = Check_Error };
            }
            
            type = guiding_type;
        }
        else
        {
            Check_Info target_info = CheckExpression(context, expression->cast_expr.type);
            CHECK_RETURN_IF_NOT_COMPLETE(target_info);
            
            if (target_info.type != Type_Typeid)
            {
                //// ERROR: Target type of cast must be of type typeid
                return (Check_Info){ .result = Check_Error };
            }
            else if (target_info.value_kind != Value_Constant)
            {
                //// ERROR: Target type of cast must be constant
                return (Check_Info){ .result = Check_Error };
            }
            
            type = target_info.const_value.type_id;
        }
        
        Check_Info expr_info = CheckExpression(context, expression->cast_expr.expr);
        CHECK_RETURN_IF_NOT_COMPLETE(expr_info);
        
        if (!Type_IsCastableTo(expr_info.type, type))
        {
            //// ERROR: Expression is not castable to target type
            return (Check_Info){ .result = Check_Error };
        }
        
        info = (Check_Info){
            .result     = Check_Complete,
            .type       = type,
            .value_kind = MAX(Value_Register, expr_info.value_kind),
        };
        
        if (info.value_kind == Value_Constant)
        {
            NOT_IMPLEMENTED;
        }
    }
    else if (expression->kind == AST_Subscript)
    {
        Check_Info array_info = CheckExpression(context, expression->subscript_expr.array);
        CHECK_RETURN_IF_NOT_COMPLETE(array_info);
        
        Check_Info index_info = CheckExpression(context, expression->subscript_expr.index);
        CHECK_RETURN_IF_NOT_COMPLETE(index_info);
        
        Type_Info* type_info = Type_InfoOf(array_info.type);
        
        if (type_info == 0 || (type_info->kind != Type_Array && type_info->kind != Type_Slice &&
                               type_info->kind != Type_Pointer))
        {
            //// ERROR: Subscript operator requires an array, slice of pointer operand type
            return (Check_Info){ .result = Check_Error };
        }
        else if (!Type_IsInteger(index_info.type))
        {
            //// ERROR: Index must be of integer type
            return (Check_Info){ .result = Check_Error };
        }
        else if (Type_IsSignedInteger(index_info.type) && (index_info.value_kind != Value_Constant || index_info.const_value.int64 < 0) || 
                 index_info.type == Type_SoftInt && BigInt_IsLess(index_info.const_value.soft_int, BigInt_0))
        {
            //// ERROR: Index must be positive
            return (Check_Info){ .result = Check_Error };
        }
        else if (index_info.type == Type_SoftInt && BigInt_IsGreater(index_info.const_value.soft_int, BigInt_U64_MAX))
        {
            //// ERROR: Buy more ram
            return (Check_Info){ .result = Check_Error };
        }
        
        u64 index = index_info.const_value.uint64;
        if (index_info.type == Type_SoftInt) index = BigInt_ToU64(info.const_value.soft_int);
        
        if (type_info->kind == Type_Array && index > type_info->array.size)
        {
            //// ERROR: Index too great
            return (Check_Info){ .result = Check_Error };
        }
        else
        {
            info = (Check_Info){
                .result     = Check_Complete,
                .type       = (type_info->kind == Type_Array ? type_info->array.elem_type : type_info->elem_type),
                .value_kind = MAX(Value_Register, MIN(array_info.value_kind, index_info.value_kind)),
            };
            
            if (info.value_kind == Value_Constant)
            {
                NOT_IMPLEMENTED;
            }
        }
    }
    else if (expression->kind == AST_Slice)
    {
        NOT_IMPLEMENTED;
    }
    else INVALID_CODE_PATH;
    
    return info;
}

internal CHECK_RESULT
CheckScope(Check_Context* context, AST_Node* scope)
{
    CHECK_RESULT result = Check_Error;
    
    NOT_IMPLEMENTED;
    
    return result;
}

internal CHECK_RESULT
CheckStatement(Check_Context* context, AST_Node* statement)
{
    CHECK_RESULT result = Check_Error;
    
    if (statement->kind == AST_Block)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_If)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_While)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_Break)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_Continue)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_Defer)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_Return)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_Using)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_Assignment)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_Variable)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_Constant)
    {
        NOT_IMPLEMENTED;
    }
    else if (statement->kind == AST_Include)
    {
        NOT_IMPLEMENTED;
    }
    else
    {
        ASSERT(statement->kind == AST_When);
        NOT_IMPLEMENTED;
    }
    
    return result;
}