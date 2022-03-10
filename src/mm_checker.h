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
    Value_Memory    = 0, // < Addressable
    Value_Parameter = 1, // v Not addresable
    Value_Register  = 2,
    Value_Constant  = 3,
} VALUE_KIND;

typedef struct Check_Info
{
    CHECK_RESULT result;
    Type_ID type;
    VALUE_KIND value_kind;
    Const_Val const_value;
    String error_message;
} Check_Info;

#define CHECK_RETURN_ERROR(msg) return (Check_Info){ .result = Check_Error, .error_message = STRING(msg) }
#define CHECK_RETURN_ON_NOT_COMPLETE(info) if ((info).result != Check_Complete) return (info)

internal Check_Info
CheckExpression(Check_Context* context, AST_Node* expression)
{
    if (expression == 0)
    {
        //// ERROR
        CHECK_RETURN_ERROR("Missing expression");
    }
    else if (expression->kind < AST_FirstExpression || expression->kind > AST_LastExpression)
    {
        if (expression->kind == AST_PolyConstant)
        {
            // TODO: If this is in a proc param type decl, register poly type?
            NOT_IMPLEMENTED;
        }
        else
        {
            //// ERROR
            CHECK_RETURN_ERROR("Node is not an expression");
        }
    }
    else if (expression->kind >= AST_FirstBinary && expression->kind <= AST_LastBinary)
    {
        Check_Info rhs_info = CheckExpression(context, expression->binary_expr.right);
        CHECK_RETURN_ON_NOT_COMPLETE(rhs_info);
        
        Check_Info lhs_info = CheckExpression(context, expression->binary_expr.left);
        CHECK_RETURN_ON_NOT_COMPLETE(lhs_info);
        
        Type_ID common_type = Type_CommonType(lhs_info.type, rhs_info.type);
        
        if (common_type == Type_None)
        {
            //// ERROR
            CHECK_RETURN_ERROR("No common type");
        }
        
        bool lhs_representable, rhs_representable;
        Const_Val lhs = ConstVal_ConvertTo(lhs_info.const_value, lhs_info.type, common_type, &lhs_representable);
        Const_Val rhs = ConstVal_ConvertTo(rhs_info.const_value, rhs_info.type, common_type, &rhs_representable);
        if (!lhs_representable)
        {
            //// ERROR
            CHECK_RETURN_ERROR("Lhs is not representable as the common type X");
        }
        if (!rhs_representable)
        {
            //// ERROR
            CHECK_RETURN_ERROR("Rhs is not representable as the common type X");
        }
        
        if (expression->kind == AST_Mul)
        {
            if (!Type_IsNumeric(common_type))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Operand to arithmetic multiplication operator must have a common numeric type");
            }
            
            return (Check_Info){
                .result = Check_Complete,
                .type = common_type,
                .value_kind = MAX(Value_Register, MIN(rhs_info.value_kind, lhs_info.value_kind)),
                .const_value = ConstVal_Mul(lhs, rhs, common_type),
            };
        }
        else if (expression->kind == AST_Div)
        {
            if (!Type_IsNumeric(common_type))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Operand to arithmetic division operator must have a common numeric type");
            }
            else if (rhs_info.value_kind == Value_Constant && 
                     (Type_IsFloat(common_type) 
                      ? ConstVal_IsEqual(rhs, ConstVal_FromBigFloat(BigFloat_0), Type_SoftFloat)
                      : ConstVal_IsEqual(rhs, ConstVal_FromBigInt(BigInt_0), Type_SoftInt)))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Illegal division by zero");
            }
            
            return (Check_Info){
                .result = Check_Complete,
                .type = common_type,
                .value_kind = MAX(Value_Register, MIN(rhs_info.value_kind, lhs_info.value_kind)),
                .const_value = ConstVal_Div(lhs, rhs, common_type),
            };
        }
        else if (expression->kind == AST_Rem)
        {
            if (!Type_IsInteger(common_type))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Operands to arithmetic remainder operator must have a common integer type");
            }
            else if (rhs_info.value_kind == Value_Constant &&
                     ConstVal_IsEqual(rhs, ConstVal_FromBigInt(BigInt_0), Type_SoftInt))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Illegal division by zero");
            }
            
            return (Check_Info){
                .result = Check_Complete,
                .type = common_type,
                .value_kind = MAX(Value_Register, MIN(rhs_info.value_kind, lhs_info.value_kind)),
                .const_value = ConstVal_Mod(lhs, rhs, common_type),
            };
        }
        else if (expression->kind == AST_BitAnd || expression->kind == AST_BitOr || expression->kind == AST_BitXor)
        {
            if (!Type_IsInteger(common_type) && !Type_IsBoolean(common_type))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Operands to bitwise and/or/xor operator must have a common integer or boolean type");
            }
            
            Const_Val val;
            switch (expression->kind)
            {
                case AST_BitAnd: val = ConstVal_BitAnd(lhs, rhs, common_type); break;
                case AST_BitOr:  val = ConstVal_BitOr(lhs, rhs, common_type);  break;
                case AST_BitXor: val = ConstVal_BitXor(lhs, rhs, common_type); break;
                INVALID_DEFAULT_CASE;
            }
            
            return (Check_Info){
                .result      = Check_Complete,
                .type        = common_type,
                .value_kind  = MAX(Value_Register, MIN(rhs_info.value_kind, lhs_info.value_kind)),
                .const_value = val,
            };
        }
        else if (expression->kind >= AST_BitSar && expression->kind <= AST_BitShl)
        {
            if (!Type_IsInteger(common_type))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Operands to bitwise shift operator must have a common integertype");
            }
            else if (rhs_info.value_kind == Value_Constant &&
                     !ConstVal_IsLess(rhs, ConstVal_FromU64(8*ABS(Type_Sizeof(common_type))), Type_SoftInt))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Shift amount to large");
            }
            
            Const_Val val;
            switch (expression->kind)
            {
                case AST_BitShr:      val = ConstVal_BitShr(lhs, rhs, common_type);       break;
                case AST_BitSar:      val = ConstVal_BitSar(lhs, rhs, common_type);       break;
                case AST_BitShl:      val = ConstVal_BitShl(lhs, rhs, common_type);       break;
                case AST_BitSplatShl: val = ConstVal_BitSplatLeft(lhs, rhs, common_type); break;
                INVALID_DEFAULT_CASE;
            }
            
            return (Check_Info){
                .result      = Check_Complete,
                .type        = common_type,
                .value_kind  = MAX(Value_Register, MIN(rhs_info.value_kind, lhs_info.value_kind)),
                .const_value = val,
            };
        }
        else if (expression->kind == AST_Add || expression->kind == AST_Sub)
        {
            if (!Type_IsNumeric(common_type) && !Type_IsPointer(common_type))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Operands to arithmetic add/sub operator must have a common numeric or pointer type");
            }
            
            return (Check_Info){
                .result      = Check_Complete,
                .type        = common_type,
                .value_kind  = MAX(Value_Register, MIN(rhs_info.value_kind, lhs_info.value_kind)),
                .const_value = (expression->kind == AST_Add 
                                ? ConstVal_Add(lhs, rhs, common_type) 
                                : ConstVal_Sub(lhs, rhs, common_type)),
            };
        }
        else if (expression->kind == AST_IsEqual || expression->kind == AST_IsNotEqual)
        {
            NOT_IMPLEMENTED; // TODO: Should struct == struct and any == any be possible?
            return (Check_Info){
                .result      = Check_Complete,
                .type        = Type_SoftBool,
                .value_kind  = MAX(Value_Register, MIN(rhs_info.value_kind, lhs_info.value_kind)),
                .const_value = (expression->kind == AST_IsNotEqual) ^ ConstVal_IsEqual(lhs, rhs, common_type),
            };
        }
        else if (expression->kind >= AST_IsLess && expression->kind <= AST_IsGreaterEqual)
        {
            if (!Type_IsNumeric(common_type))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Operands to ordinal comparison operator must have a common numeric type");
            }
            
            bool is_less    = ConstVal_IsLess(lhs, rhs, common_type);
            bool is_greater = ConstVal_IsGreater(lhs, rhs, common_type);
            
            Const_Val val;
            switch (expression->kind)
            {
                case AST_IsLess:         val = ConstVal_FromBool( is_less);    break;
                case AST_IsLessEqual:    val = ConstVal_FromBool(!is_greater); break;
                case AST_IsGreater:      val = ConstVal_FromBool( is_greater); break;
                case AST_IsGreaterEqual: val = ConstVal_FromBool(!is_less);    break;
                INVALID_DEFAULT_CASE;
            }
            
            return (Check_Info){
                .result      = Check_Complete,
                .type        = Type_SoftBool,
                .value_kind  = MAX(Value_Register, MIN(rhs_info.value_kind, lhs_info.value_kind)),
                .const_value = val,
            };
        }
        else if (expression->kind == AST_And || expression->kind == AST_Or)
        {
            if (!Type_IsBoolean(common_type))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Operands to logical operator must have a common boolean type");
            }
            
            return (Check_Info){
                .result      = Check_Complete,
                .type        = Type_SoftBool, // NOTE: this is to make b16 = b8 && b8 less inconvenient
                .value_kind  = MAX(Value_Register, MIN(rhs_info.value_kind, lhs_info.value_kind)),
                .const_value = (expression->kind == AST_And 
                                ? ConstVal_And(lhs, rhs, common_type) 
                                : ConstVal_Or(lhs, rhs, common_type)),
            };
        }
        else
        {
            //// ERROR
            CHECK_RETURN_ERROR("Invalid expression kind");
        }
    }
    else if (expression->kind >= AST_FirstTypeLevel   && expression->kind <= AST_LastTypeLevel ||
             expression->kind >= AST_FirstPrefixLevel && expression->kind <= AST_LastPrefixLevel)
    {
        if (expression->kind == AST_ArrayType)
        {
            Check_Info type_info = CheckExpression(context, expression->array_type.type);
            CHECK_RETURN_ON_NOT_COMPLETE(type_info);
            
            Check_Info size_info = CheckExpression(context, expression->array_type.type);
            CHECK_RETURN_ON_NOT_COMPLETE(size_info);
            
            if (type_info.type != Type_Typeid ||
                type_info.value_kind != Value_Constant)
            {
                //// ERROR
                CHECK_RETURN_ERROR("Operand to type descriptor must be a constant typeid");
            }
            else if (!Type_Exists(type_info.const_value.type_id)) // TODO: Polymorphism
            {
                //// ERROR
                CHECK_RETURN_ERROR("Given type does not exist");
            }
            else if (!Type_IsInteger(size_info.type)        ||
                     size_info.value_kind != Value_Constant ||
                     !ConstVal_IsGreater(size_info.const_value, ConstVal_FromBigInt(BigInt_0), Type_SoftInt))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Size of array type must be a constant positive integer");
            }
            else if (!ConstVal_IsGreater(size_info.const_value, ConstVal_FromBigInt(BigInt_U64_MAX), Type_SoftInt))
            {
                //// ERROR
                CHECK_RETURN_ERROR("Sorry, exabyte arrays are not yet supported... In the meantime buy more ram, like a lot.");
            }
            
            return (Check_Info){
                .result              = Check_Complete,
                .type                = Type_Typeid,
                .value_kind          = Value_Constant,
                .const_value.type_id = Type_ArrayOf(type_info.const_value.type_id,
                                                    BigInt_ToU64(size_info.const_value.soft_int)),
            };
        }
        else
        {
            Check_Info operand_info = CheckExpression(context, expression->unary_expr);
            CHECK_RETURN_ON_NOT_COMPLETE(operand_info);
            
            if (expression->kind == AST_SliceType ||
                expression->kind == AST_Reference && operand_info.type == Type_Typeid && operand_info.value_kind == Value_Constant)
            {
                if (operand_info.type != Type_Typeid ||
                    operand_info.value_kind != Value_Constant)
                {
                    //// ERROR
                    CHECK_RETURN_ERROR("Operand to type descriptor must be a constant typeid");
                }
                else if (!Type_Exists(operand_info.const_value.type_id))
                {
                    //// ERROR
                    CHECK_RETURN_ERROR("Given type does not exist");
                }
                
                return (Check_Info){
                    .result              = Check_Complete,
                    .type                = Type_Typeid,
                    .value_kind          = Value_Constant,
                    .const_value.type_id = Type_SliceOf(operand_info.const_value.type_id),
                };
            }
            else if (expression->kind == AST_Reference)
            {
                if (operand_info.value_kind != Value_Memory)
                {
                    //// ERROR
                    CHECK_RETURN_ERROR("Only addressable values can be taken the address of");
                }
                
                return (Check_Info){
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
                    //// ERROR
                    CHECK_RETURN_ERROR("Only pointer types can be dereferenced");
                }
                
                return (Check_Info){
                    .result     = Check_Complete,
                    .type       = Type_StripPointer(operand_info.type),
                    .value_kind = Value_Memory,
                };
            }
            else if (expression->kind == AST_Not)
            {
                if (!Type_IsBoolean(operand_info.type))
                {
                    //// ERROR
                    CHECK_RETURN_ERROR("Operand to logical not must be of boolean type");
                }
                
                return (Check_Info){
                    .result              = Check_Complete,
                    .type                = operand_info.type,
                    .value_kind          = MAX(Value_Register, operand_info.value_kind),
                    .const_value.boolean = !operand_info.const_value.boolean,
                };
            }
            else if (expression->kind == AST_BitNot)
            {
                if (!Type_IsInteger(operand_info.type))
                {
                    //// ERROR
                    CHECK_RETURN_ERROR("Operand to bitwise not must be of integer type");
                }
                
                return (Check_Info){
                    .result      = Check_Complete,
                    .type        = operand_info.type,
                    .value_kind  = MAX(Value_Register, operand_info.value_kind),
                    .const_value = ConstVal_BitNot(operand_info.const_value, operand_info.type),
                };
            }
            else if (expression->kind == AST_BitNot)
            {
                if (!Type_IsNumeric(operand_info.type))
                {
                    //// ERROR
                    CHECK_RETURN_ERROR("Operand to negation must be of numeric type");
                }
                
                return (Check_Info){
                    .result      = Check_Complete,
                    .type        = operand_info.type,
                    .value_kind  = MAX(Value_Register, operand_info.value_kind),
                    .const_value = ConstVal_Neg(operand_info.const_value, operand_info.type),
                };
            }
            else
            {
                //// ERROR
                CHECK_RETURN_ERROR("Invalid expression kind");
            }
        }
    }
    else if (expression->kind == AST_Identifier)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_String)
    {
        return (Check_Info){
            .result      = Check_Complete,
            .type        = Type_String,
            .value_kind  = Value_Constant,
            .const_value = ConstVal_FromString(expression->string),
        };
    }
    else if (expression->kind == AST_Int)
    {
        return (Check_Info){
            .result      = Check_Complete,
            .type        = Type_SoftInt,
            .value_kind  = Value_Constant,
            .const_value = ConstVal_FromBigInt(expression->integer),
        };
    }
    else if (expression->kind == AST_Float)
    {
        return (Check_Info){
            .result      = Check_Complete,
            .type        = Type_SoftFloat,
            .value_kind  = Value_Constant,
            .const_value = ConstVal_FromBigFloat(expression->floating),
        };
    }
    else if (expression->kind == AST_Boolean)
    {
        return (Check_Info){
            .result      = Check_Complete,
            .type        = Type_SoftBool,
            .value_kind  = Value_Constant,
            .const_value = ConstVal_FromBool(expression->boolean),
        };
    }
    else if (expression->kind == AST_Proc || expression->kind == AST_ProcLiteral)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Struct || expression->kind == AST_Union)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Enum)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Compound)
    {
        Check_Info compound_info = CheckExpression(context, expression->compound_expr);
        CHECK_RETURN_ON_NOT_COMPLETE(compound_info);
        
        return compound_info;
    }
    else if (expression->kind == AST_Selector)
    {
        // NOTE: Only enum selector expressions will end up here. Selector expressions in
        //       return .a = 0 and Struct.{ .a = 0 }, int.[ .0 = 12 ], etc., are all handled 
        //       by the caller.
        // TODO: Should the . syntax for anything but enum selector expressions be moved to their
        //       own node kinds in the AST?
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_StructLiteral)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_ArrayLiteral)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Cast)
    {
        Check_Info expr_info = CheckExpression(context, expression->cast_expr.expr);
        CHECK_RETURN_ON_NOT_COMPLETE(expr_info);
        
        Check_Info type_info = CheckExpression(context, expression->cast_expr.type);
        CHECK_RETURN_ON_NOT_COMPLETE(type_info);
        
        if (type_info.type != Type_Typeid ||
            type_info.value_kind != Value_Constant)
        {
            //// ERROR
            CHECK_RETURN_ERROR("Destination type must be a constant value of type typeid");
        }
        else if (!Type_Exists(type_info.const_value.type_id))
        {
            //// ERROR
            CHECK_RETURN_ERROR("Destination type does not exist");
        }
        else if (Type_IsCastableTo(expr_info.type, type_info.const_value.type_id))
        {
            //// ERROR
            CHECK_RETURN_ERROR("Expression is not castable to the destination type");
        }
        
        return (Check_Info){
            .result = Check_Complete,
            .type = type_info.const_value.type_id,
            .value_kind = MAX(Value_Register, expr_info.value_kind),
            .const_value = ConstVal_ConvertTo(expr_info.const_value, expr_info.type, type_info.const_value.type_id, 0),
        };
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Conditional)
    {
        Check_Info condition_info = CheckExpression(context, expression->conditional_expr.condition);
        CHECK_RETURN_ON_NOT_COMPLETE(condition_info);
        
        Check_Info true_info = CheckExpression(context, expression->conditional_expr.true_clause);
        CHECK_RETURN_ON_NOT_COMPLETE(true_info);
        
        Check_Info false_info = CheckExpression(context, expression->conditional_expr.false_clause);
        CHECK_RETURN_ON_NOT_COMPLETE(false_info);
        
        if (!Type_IsBoolean(condition_info.type))
        {
            //// ERROR
            CHECK_RETURN_ERROR("Condition of conditional expression must be of boolean type");
        }
        
        Type_ID common_type = Type_CommonType(true_info.type, false_info.type);
        
        if (common_type == Type_None)
        {
            //// ERROR
            CHECK_RETURN_ERROR("No common type");
        }
        
        bool is_true_rep, is_false_rep;
        Const_Val true_clause  = ConstVal_ConvertTo(true_info.const_value, true_info.type, common_type, &is_true_rep);
        Const_Val false_clause = ConstVal_ConvertTo(false_info.const_value, false_info.type, common_type, &is_false_rep);
        if (!is_true_rep)
        {
            //// ERROR
            CHECK_RETURN_ERROR("True is not representable as the common type X");
        }
        if (!is_false_rep)
        {
            //// ERROR
            CHECK_RETURN_ERROR("False clause is not representable as the common type X");
        }
        
        if (condition_info.value_kind == Value_Constant)
        {
            bool clause = condition_info.const_value.boolean;
            return (Check_Info){
                .result      = Check_Complete,
                .type        = common_type,
                .value_kind  = (clause ? true_info.value_kind  : false_info.value_kind),
                .const_value = (clause ? true_clause           : false_clause),
            };
        }
        else
        {
            return (Check_Info){
                .result     = Check_Complete,
                .type       = common_type,
                .value_kind = Value_Register,
            };
        }
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Call)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_ElementOf)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Subscript)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Slice)
    {
        NOT_IMPLEMENTED;
    }
    else
    {
        //// ERROR
        CHECK_RETURN_ERROR("Unknown expression kind");
    }
}