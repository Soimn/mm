typedef enum CHECK_RESULT
{
    Check_Incomplete = 0,
    Check_Completing,
    Check_Complete,
    Check_Error,
} CHECK_RESULT;

typedef struct Check_Context
{
    bool _;
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
internal CHECK_RESULT CheckDeclaration(Check_Context* context, AST_Node* decl);

internal void
FoldConstant(AST_Node* expression, Check_Info info)
{
    // TODO: Better constant folding
    if (info.result == Check_Complete && info.value_kind == Value_Constant)
    {
        if (info.type == Type_SoftInt)
        {
            ZeroStruct(expression);
            expression->kind    = AST_Int;
            expression->integer = info.const_value.soft_int;
        }
        else if (Type_IsSignedInteger(info.type))
        {
            ZeroStruct(expression);
            expression->kind    = AST_Int;
            expression->integer = BigInt_FromI64(info.const_value.int64);
        }
        else if (Type_IsUnsignedInteger(info.type))
        {
            ZeroStruct(expression);
            expression->kind    = AST_Int;
            expression->integer = BigInt_FromU64(info.const_value.uint64);
        }
        else if (info.type == Type_SoftFloat)
        {
            ZeroStruct(expression);
            expression->kind     = AST_Float;
            expression->floating = info.const_value.soft_float;
        }
        else if (Type_IsFloat(info.type))
        {
            ZeroStruct(expression);
            expression->kind     = AST_Float;
            expression->floating = BigFloat_FromF64(info.const_value.float64);
        }
        else if (info.type == Type_Bool)
        {
            ZeroStruct(expression);
            expression->kind = AST_Boolean;
            expression->boolean = info.const_value.boolean;
        }
        else if (info.type == Type_String)
        {
            ZeroStruct(expression);
            expression->kind = AST_String;
            expression->string = info.const_value.string;
        }
    }
}

internal Check_Info
CheckExpression(Check_Context* context, AST_Node* expression, Type_ID target_type)
{
    Check_Info info = { .result = Check_Error };
    
    if (expression == 0)
    {
        //// ERROR: Missing expression
    }
    else if (expression->kind == AST_Compound)
    {
        info = CheckExpression(context, expression->compound_expr);
    }
    else if (expression->kind >= AST_FirstBinary && expression->kind <= AST_LastBinary)
    {
        Check_Info left_info = CheckExpression(context, expression->binary_expr.left);
        
        if (left_info.result != Check_Complete) info.result = left_info.result;
        else
        {
            Check_Info right_info = CheckExpression(context, expression->binary_expr.right);
            
            if (right_info.result != Check_Complete) info.result = right_info.result;
            else
            {
                // TODO: Coercion
                NOT_IMPLEMENTED;
                
                /*
                        // numeric
                        AST_Mul,
                        AST_Div,
                        
                        // integer
                        AST_Rem,
                        
                        // numeric, pointer
                        AST_Add,
                        AST_Sub,
                        
                        // numeric, pointer, boolean?
                        AST_IsLess,
                        AST_IsGreater,
                        AST_IsLessEqual,
                        AST_IsGreaterEqual,
                        
                        // typed integer
                        AST_BitOr,
                        AST_BitXor,
                        AST_BitAnd,
                        AST_BitSar,
                        AST_BitShr,
                        AST_BitSplatShl,
                        AST_BitShl,
                        
                        // same
                        AST_IsEqual,
                        AST_IsNotEqual,
                        
                        // boolean, integer?
                        AST_And,
                        AST_Or,
                        */
            }
        }
    }
    else if (expression->kind >= AST_FirstPrefixLevel && expression->kind <= AST_LastPrefixLevel)
    {
        Check_Info operand_info = CheckExpression(context, expression->array_type.type);
        
        if (operand_info.result != Check_Complete) info.result = operand_info.result;
        else
        {
            if (expression->kind == AST_Reference)
            {
                if (operand_info.value_kind != Value_Memory)
                {
                    //// ERROR: operand to reference operator must be an l value
                    info.result = Check_Error;
                }
                else
                {
                    // TODO: restrict parameter, constant, etc.
                    NOT_IMPLEMENTED;
                }
            }
            else if (expression->kind == AST_Dereference)
            {
                Type_Info* type_info = Type_InfoOf(operand_info.type);
                
                if (type_info == 0 || type_info->kind != Type_Pointer)
                {
                    //// ERROR: operand to dereference operator must be of pointer type
                    info.result = Check_Error;
                }
                else
                {
                    info = (Check_Info){
                        .result     = Check_Complete,
                        .type       = type_info->elem_type,
                        .value_kind = Value_Memory,
                    };
                }
            }
            else if (expression->kind == AST_Neg)
            {
                if (!Type_IsNumeric(operand_info.type))
                {
                    //// ERROR: operand to arithmetic negation operator must be of numeric type
                    info.result = Check_Error;
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
            }
            else if (expression->kind == AST_BitNot)
            {
                if (!Type_IsInteger(operand_info.type))
                {
                    //// ERROR: operand to bitwise not operator must be of integer type
                    info.result = Check_Error;
                }
                else if (Type_IsSoft(operand_info.type))
                {
                    //// ERROR: operand to bitwise not operator must be a hard type
                    info.result = Check_Error;
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
                    info.result = Check_Error;
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
    }
    else if (expression->kind >= AST_FirstTypeLevel && expression->kind <= AST_LastTypeLevel)
    {
        if (expression->kind == AST_ArrayType)
        {
            Check_Info operand_info = CheckExpression(context, expression->array_type.type);
            
            if (operand_info.result != Check_Complete) info.result = operand_info.result;
            else
            {
                Check_Info size_info = CheckExpression(context, expression->array_type.size);
                
                if (size_info.result != Check_Complete) info.result = size_info.result;
                else
                {
                    if (operand_info.type != Type_Typeid)
                    {
                        //// ERROR: operand of type descriptor must be of type typeid
                        info.result = Check_Error;
                    }
                    else if (operand_info.value_kind != Value_Constant)
                    {
                        //// ERROR: operand of type descriptor must be constant
                        info.result = Check_Error;
                    }
                    else if (Type_IsInteger(size_info.type))
                    {
                        //// ERROR: size of array type descriptor must be of integer type
                        info.result = Check_Error;
                    }
                    else if (size_info.value_kind != Value_Constant)
                    {
                        //// ERROR: size of array type descriptor must be constant
                        info.result = Check_Error;
                    }
                    else if (Type_IsSignedInteger(size_info.type) && (size_info.value_kind != Value_Constant || size_info.const_value.int64 < 0) ||
                             size_info.type == Type_SoftInt && !BigInt_IsGreater(size_info.const_value.soft_int, BigInt_0))
                    {
                        //// ERROR: size of array type descriptor must be positive
                        info.result = Check_Error;
                    }
                    else if (size_info.type == Type_SoftInt && BigInt_IsGreater(size_info.const_value.soft_int, BigInt_U64_MAX))
                    {
                        //// ERROR: Buy more ram
                        info.result = Check_Error;
                    }
                    else
                    {
                        u64 size = info.const_value.uint64;
                        if (size_info.type == Type_SoftInt) size = BigInt_ToU64(size_info.const_value.soft_int);
                        
                        info = (Check_Info){
                            .result      = Check_Complete,
                            .type        = Type_Typeid,
                            .value_kind  = Value_Constant,
                            .const_value = Type_ArrayOf(operand_info.type, size),
                        };
                    }
                }
            }
        }
        else
        {
            Check_Info operand_info = CheckExpression(context, expression->unary_expr);
            
            if (operand_info.result != Check_Complete) info.result = operand_info.result;
            else
            {
                if (operand_info.type != Type_Typeid)
                {
                    //// ERROR: operand of type descriptor must be of type typeid
                    info.result = Check_Error;
                }
                else if (operand_info.value_kind != Value_Constant)
                {
                    //// ERROR: operand of type descriptor must be constant
                    info.result = Check_Error;
                }
                else
                {
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
        }
    }
    else if (expression->kind == AST_Identifier)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_String)
    {
        info = (Check_Info){
            .result             = Check_Complete,
            .type               = Type_SoftString,
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
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_ArrayLiteral)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_ElementOf)
    {
        if (expression->element_of.structure == 0)
        {
            if (target_type == Type_None)
            {
                //// ERROR: Cannot determine type of implicit selector expression
                info.result = Check_Error;
            }
            else
            {
                Type_Info* type_info = Type_InfoOf(target_type);
                
                if (type_info == 0 || type_info->kind != Type_Enum)
                {
                    //// ERROR: Selector expressions can only be used on enumerations
                    info.result = Check_Error;
                }
                else
                {
                    NOT_IMPLEMENTED;
                }
            }
        }
        else
        {
            Check_Info struct_info = CheckExpression(context, expression->element_of.structure);
            
            if (struct_info.result != Check_Complete) info.result = struct_info.result;
            else
            {
                Type_Info* type_info = Type_InfoOf(struct_info.type);
                
                if (type_info == 0 || 
                    (type_info->kind != Type_Struct && type_info->kind != Type_Union &&
                     type_info->kind != Type_Enum   && (type_info->kind != Type_Array || type_info->array.size > 4)))
                {
                    //// ERROR: Illegal use of element of operator
                    info.result = Check_Error;
                }
                else
                {
                    Type_ID element_type = Type_None;
                    Interned_String element_name = expression->element_of.element;
                    
                    if (type_info->kind == Type_Array)
                    {
                        String string = String_FromInternedString(element_name);
                        
                        if (string.size == 1 && (*string.data >= 'x' && *string.data <= 'w'))
                        {
                            element_type = type_info->array.elem_type;
                        }
                        else
                        {
                            //// ERROR: array has no element named x
                            info.result = Check_Error;
                        }
                    }
                    else
                    {
                        NOT_IMPLEMENTED;
                    }
                    
                    if (element_type != Type_None)
                    {
                        info = (Check_Info){
                            .result     = Check_Complete,
                            .type       = element_type,
                            .value_kind = l value and constant?,
                        };
                        
                        if (info.value_kind == Value_Constant)
                        {
                            NOT_IMPLEMENTED;
                        }
                    }
                }
            }
        }
    }
    else if (expression->kind == AST_Conditional)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Cast)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Subscript)
    {
        Check_Info array_info = CheckExpression(context, expression->subscript_expr.array);
        
        if (array_info.result != Check_Complete) info.result = array_info.result;
        else
        {
            Check_Info index_info = CheckExpression(context, expression->subscript_expr.index);
            
            if (index_info.result != Check_Complete) info.result = index_info.result;
            else
            {
                Type_Info* type_info = Type_InfoOf(array_info.type);
                
                if (type_info == 0 || (type_info->kind != Type_Array && type_info->kind != Type_Slice &&
                                       type_info->kind != Type_Pointer))
                {
                    //// ERROR: Subscript operator requires an array, slice of pointer operand type
                    info.result = Check_Error;
                }
                else if (!Type_IsInteger(index_info.type))
                {
                    //// ERROR: Index must be of integer type
                    info.result = Check_Error;
                }
                else if (Type_IsSignedInteger(index_info.type) && (index_info.value_kind != Value_Constant || index_info.const_value.int64 < 0) || 
                         index_info.type == Type_SoftInt && BigInt_IsLess(index_info.const_value.soft_int, BigInt_0))
                {
                    //// ERROR: Index must be positive
                    info.result = Check_Error;
                }
                else if (index_info.type == Type_SoftInt && BigInt_IsGreater(index_info.const_value.soft_int, BigInt_U64_MAX))
                {
                    //// ERROR: Buy more ram
                    info.result = Check_Error;
                }
                else
                {
                    u64 index = index_info.const_value.uint64;
                    if (index_info.type == Type_SoftInt) index = BigInt_ToU64(info.const_value.soft_int);
                    
                    if (type_info->kind == Type_Array && index > type_info->array.size)
                    {
                        //// ERROR: Index too great
                        info.result = Check_Error;
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
CheckDeclaration(Check_Context* context, AST_Node* decl)
{
    CHECK_RESULT result = Check_Error;
    
    NOT_IMPLEMENTED;
    
    return result;
}