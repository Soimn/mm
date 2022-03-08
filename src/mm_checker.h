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
        
        //// ERROR
        CHECK_RETURN_ERROR("Node is not an expression");
    }
    else if (expression->kind >= AST_FirstBinary && expression->kind <= AST_LastBinary)
    {
        /*
        AST_Mul,
        AST_Div,
        AST_Rem,
        AST_BitAnd,
        AST_BitSar,
        AST_BitShr,
        AST_BitSplatShl,
        AST_BitShl,
        AST_Add,
        AST_Sub,
        AST_BitOr,
        AST_BitXor,
        AST_IsEqual,
        AST_IsNotEqual,
        AST_IsLess,
        AST_IsGreater,
        AST_IsLessEqual,
        AST_IsGreaterEqual,
        AST_And,
        AST_Or,
        */
        NOT_IMPLEMENTED;
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
                CHECK_RETURN_ERROR("operand to type descriptor must be a constant typeid");
            }
            else if (!Type_Exists(type_info.const_value.type_id)) // TODO: Polymorphism
            {
                //// ERROR
                CHECK_RETURN_ERROR("given type does not exist");
            }
            else if (!Type_IsInteger(size_info.type)        ||
                     size_info.value_kind != Value_Constant ||
                     !BigInt_IsGreater(size_info.const_value.soft_int, BigInt_0)) // IMPORTANT TODO: Maybe ConstVal_IsGreater?
            {
                //// ERROR
                CHECK_RETURN_ERROR("size of array type must be a constant positive integer");
            }
            else if (BigInt_IsGreater(size_info.const_value.soft_int, BigInt_U64_MAX))
            {
                //// ERROR
                CHECK_RETURN_ERROR("sorry, exabyte arrays are not yet supported... In the meantime buy more ram, like a lot.");
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
                    CHECK_RETURN_ERROR("operand to type descriptor must be a constant typeid");
                }
                else if (!Type_Exists(operand_info.const_value.type_id)) // TODO: Polymorphism
                {
                    //// ERROR
                    CHECK_RETURN_ERROR("given type does not exist");
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
                    CHECK_RETURN_ERROR("only addressable values can be taken the address of");
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
                    CHECK_RETURN_ERROR("only pointer types can be dereferenced");
                }
                
                return (Check_Info){
                    .result     = Check_Complete,
                    .type       = Type_StripPointer(operand_info.type),
                    .value_kind = Value_Memory,
                };
            }
            else if (expression->kind == AST_Not)
            {
                if (!Type_IsBool(operand_info.type))
                {
                    //// ERROR
                    CHECK_RETURN_ERROR("operand to logical not must be of boolean type");
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
                    CHECK_RETURN_ERROR("operand to bitwise not must be of integer type");
                }
                
                return (Check_Info){
                    .result      = Check_Complete,
                    .type        = operand_info.type,
                    .value_kind  = MAX(Value_Register, operand_info.value_kind),
                    .const_value = ConstVal_BitNot(operand_info.const_value, operand_info.type),
                };
            }
            /*
            AST_Neg,
        */
            NOT_IMPLEMENTED;
        }
    }
    else if (expression->kind == AST_Identifier)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_String)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Int)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Float)
    {
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Boolean)
    {
        NOT_IMPLEMENTED;
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
        NOT_IMPLEMENTED;
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
        NOT_IMPLEMENTED;
    }
    else if (expression->kind == AST_Conditional)
    {
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