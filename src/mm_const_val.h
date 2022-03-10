typedef union Const_Val
{
    Type_ID type_id;
    u8 byte;
    Big_Int soft_int;
    Big_Float soft_float;
    bool boolean;
    Interned_String string;
    u64 pointer;
    
    void* data;
} Const_Val;

internal inline Const_Val
ConstVal_FromBigInt(Big_Int val)
{
    return (Const_Val){ .soft_int = val };
}

internal inline Const_Val
ConstVal_FromU64(u64 val)
{
    return (Const_Val){ .soft_int = BigInt_FromU64(val) };
}

internal inline Const_Val
ConstVal_FromBigFloat(Big_Float val)
{
    return (Const_Val){ .soft_float = val };
}

internal inline Const_Val
ConstVal_FromBool(bool val)
{
    return (Const_Val){ .boolean = val };
}

internal inline Const_Val
ConstVal_FromString(Interned_String val)
{
    return (Const_Val){ .string = val };
}

internal inline Const_Val
ConstVal_ConvertTo(Const_Val val, Type_ID src, Type_ID dst, bool* is_representable)
{
    Const_Val result;
    
    if (Type_IsInteger(src))
    {
        if (Type_IsUnsignedInteger(dst))
        {
            Big_Int chopped_val = BigInt_ChopToSize(val.soft_int, Type_Sizeof(dst));
            
            if (is_representable) *is_representable = BigInt_IsEqual(chopped_val, val.soft_int);
            result = ConstVal_FromBigInt(chopped_val);
        }
        else if (Type_IsInteger(dst))
        {
            umm size = ABS(Type_Sizeof(dst));
            Big_Int chopped_val = BigInt_SignExtend(BigInt_ChopToSize(val.soft_int, size), size);
            
            if (is_representable) *is_representable = BigInt_IsEqual(chopped_val, val.soft_int);
            result = ConstVal_FromBigInt(chopped_val);
        }
        else if (Type_IsBoolean(dst))
        {
            if (is_representable) *is_representable = true;
            result = ConstVal_FromBool(!BigInt_IsEqual(val.soft_int, BigInt_0));
        }
        else NOT_IMPLEMENTED;
    }
    else if (Type_IsFloat(src))
    {
        NOT_IMPLEMENTED;
    }
    else if (Type_IsBoolean(src))
    {
        NOT_IMPLEMENTED;
    }
    else NOT_IMPLEMENTED;
    
    return result;
}

internal inline bool
ConstVal_IsEqual(Const_Val v0, Const_Val v1, Type_ID type)
{
    bool result;
    
    if (Type_IsPrimitive(type)) result = Memcmp(&v0, &v1, ABS(Type_Sizeof(type)));
    else
    {
        Type_Info* type_info = Type_InfoOf(type);
        ASSERT(type_info != 0);
        
        if (type_info->kind == Type_Pointer) result = (v0.pointer == v1.pointer);
        else NOT_IMPLEMENTED;
    }
    
    return result;
}

internal inline bool
ConstVal_IsGreater(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsNumeric(type));
    
    if (Type_IsFloat(type)) return BigFloat_IsGreater(v0.soft_float, v1.soft_float);
    else                    return BigInt_IsGreater(v0.soft_int, v1.soft_int); 
}

internal inline bool
ConstVal_IsLess(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsNumeric(type));
    
    if (Type_IsFloat(type)) return BigFloat_IsLess(v0.soft_float, v1.soft_float);
    else                    return BigInt_IsLess(v0.soft_int, v1.soft_int); 
}

internal inline Const_Val
ConstVal_Add(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsNumeric(type) || Type_InfoOf(type) != 0 && Type_InfoOf(type)->kind == Type_Pointer);
    
    if (type == Type_SoftFloat) return (Const_Val){ .soft_float = BigFloat_Add(v0.soft_float, v1.soft_float) };
    else if (Type_IsFloat(type))
    {
        NOT_IMPLEMENTED;
    }
    else if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_Add(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsInteger(type))
    {
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_Add(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        NOT_IMPLEMENTED;
    }
}

internal inline Const_Val
ConstVal_Sub(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsNumeric(type) || Type_InfoOf(type) != 0 && Type_InfoOf(type)->kind == Type_Pointer);
    
    if (type == Type_SoftFloat) return (Const_Val){ .soft_float = BigFloat_Sub(v0.soft_float, v1.soft_float) };
    else if (Type_IsFloat(type))
    {
        NOT_IMPLEMENTED;
    }
    else if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_Sub(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsInteger(type))
    {
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_Sub(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        NOT_IMPLEMENTED;
    }
}

internal inline Const_Val
ConstVal_Neg(Const_Val v, Type_ID type)
{
    ASSERT(Type_IsNumeric(type));
    
    if (type == Type_SoftFloat) return (Const_Val){ .soft_float = BigFloat_Neg(v.soft_float) };
    else if (Type_IsFloat(type))
    {
        NOT_IMPLEMENTED;
    }
    else if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_Neg(v.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_Neg(v.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_Mul(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsNumeric(type));
    
    if (type == Type_SoftFloat) return (Const_Val){ .soft_float = BigFloat_Mul(v0.soft_float, v1.soft_float) };
    else if (Type_IsFloat(type))
    {
        NOT_IMPLEMENTED;
    }
    else if (Type_IsUnsignedInteger(type))
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        Big_Int result = BigInt_ChopToSize(BigInt_Mul(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_Mul(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_Div(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsNumeric(type));
    
    Big_Int ignored;
    if (type == Type_SoftFloat) return (Const_Val){ .soft_float = BigFloat_Div(v0.soft_float, v1.soft_float) };
    else if (Type_IsFloat(type))
    {
        NOT_IMPLEMENTED;
    }
    else if (Type_IsUnsignedInteger(type))
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        Big_Int result = BigInt_ChopToSize(BigInt_DivMod(v0.soft_int, v1.soft_int, &ignored), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_DivMod(v0.soft_int, v1.soft_int, &ignored), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_Mod(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsInteger(type));
    
    if (Type_IsUnsignedInteger(type))
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        Big_Int result;
        BigInt_ChopToSize(BigInt_DivMod(v0.soft_int, v1.soft_int, &result), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        umm size = ABS(Type_Sizeof(type));
        Big_Int result;
        BigInt_SignExtend(BigInt_ChopToSize(BigInt_DivMod(v0.soft_int, v1.soft_int, &result), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_BitNot(Const_Val v, Type_ID type)
{
    ASSERT(Type_IsInteger(type));
    
    if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_BitNot(v.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_BitNot(v.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_BitAnd(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsInteger(type) || Type_IsBoolean(type));
    
    if      (Type_IsBoolean(type)) return (Const_Val){ .boolean = v0.boolean & v1.boolean };
    else if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_BitAnd(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_BitAnd(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_BitOr(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsInteger(type) || Type_IsBoolean(type));
    
    if      (Type_IsBoolean(type)) return (Const_Val){ .boolean = v0.boolean | v1.boolean };
    else if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_BitOr(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_BitOr(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_BitXor(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsInteger(type) || Type_IsBoolean(type));
    
    if      (Type_IsBoolean(type)) return (Const_Val){ .boolean = v0.boolean ^ v1.boolean };
    else if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_BitXor(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_BitXor(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_BitShl(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsInteger(type));
    
    if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_BitShl(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_BitShl(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_BitSplatLeft(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsInteger(type));
    
    if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_BitSplatLeft(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_BitSplatLeft(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_BitShr(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsInteger(type));
    
    if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_BitShr(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else
    {
        umm size = ABS(Type_Sizeof(type));
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_BitShr(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
}

internal inline Const_Val
ConstVal_BitSar(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsInteger(type));
    return (Const_Val){ .soft_int = BigInt_BitSar(v0.soft_int, v1.soft_int) };
}

internal inline Const_Val
ConstVal_And(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsBoolean(type));
    return (Const_Val){ .boolean = v0.boolean && v1.boolean };
}

internal inline Const_Val
ConstVal_Or(Const_Val v0, Const_Val v1, Type_ID type)
{
    ASSERT(Type_IsBoolean(type));
    return (Const_Val){ .boolean = v0.boolean || v1.boolean };
}