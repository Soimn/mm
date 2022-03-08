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
ConstVal_Add(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (type == Type_SoftFloat) return (Const_Val){ .soft_float = BigFloat_Add(v0.soft_float, v1.soft_float) };
    else if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_Add(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsInteger(type))
    {
        imm size = Type_Sizeof(type);
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_Add(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsFloat(type))
    {
        NOT_IMPLEMENTED;
    }
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_Sub(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (type == Type_SoftFloat) return (Const_Val){ .soft_float = BigFloat_Sub(v0.soft_float, v1.soft_float) };
    else if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_Sub(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsInteger(type))
    {
        imm size = Type_Sizeof(type);
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_Sub(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsFloat(type))
    {
        NOT_IMPLEMENTED;
    }
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_Mul(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (type == Type_SoftFloat) return (Const_Val){ .soft_float = BigFloat_Mul(v0.soft_float, v1.soft_float) };
    else if (Type_IsUnsignedInteger(type))
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        Big_Int result = BigInt_ChopToSize(BigInt_Mul(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsInteger(type))
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        imm size = Type_Sizeof(type);
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_Mul(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsFloat(type))
    {
        NOT_IMPLEMENTED;
    }
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_Div(Const_Val v0, Const_Val v1, Type_ID type)
{
    Big_Int ignored;
    if (type == Type_SoftFloat) return (Const_Val){ .soft_float = BigFloat_Div(v0.soft_float, v1.soft_float) };
    else if (Type_IsUnsignedInteger(type))
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        Big_Int result = BigInt_ChopToSize(BigInt_DivMod(v0.soft_int, v1.soft_int, &ignored), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsInteger(type))
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        imm size = Type_Sizeof(type);
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_DivMod(v0.soft_int, v1.soft_int, &ignored), size), size);
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsFloat(type))
    {
        NOT_IMPLEMENTED;
    }
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_Mod(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (Type_IsUnsignedInteger(type))
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        Big_Int result;
        BigInt_ChopToSize(BigInt_DivMod(v0.soft_int, v1.soft_int, &result), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsInteger(type))
    {
        NOT_IMPLEMENTED; // TODO: Is this correct?
        imm size = Type_Sizeof(type);
        Big_Int result;
        BigInt_SignExtend(BigInt_ChopToSize(BigInt_DivMod(v0.soft_int, v1.soft_int, &result), size), size);
        return (Const_Val){ .soft_int = result };
    }
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_BitNot(Const_Val v, Type_ID type)
{
    if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_BitNot(v.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsInteger(type))
    {
        imm size = Type_Sizeof(type);
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_BitNot(v.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_BitAnd(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (Type_IsInteger(type)) return (Const_Val){ .soft_int = BigInt_BitAnd(v0.soft_int, v1.soft_int) };
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_BitOr(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (Type_IsInteger(type)) return (Const_Val){ .soft_int = BigInt_BitOr(v0.soft_int, v1.soft_int) };
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_BitXor(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (Type_IsInteger(type)) return (Const_Val){ .soft_int = BigInt_BitXor(v0.soft_int, v1.soft_int) };
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_BitShl(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_BitShl(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsInteger(type))
    {
        imm size = Type_Sizeof(type);
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_BitShl(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_BitSplatLeft(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (Type_IsUnsignedInteger(type))
    {
        Big_Int result = BigInt_ChopToSize(BigInt_BitSplatLeft(v0.soft_int, v1.soft_int), Type_Sizeof(type));
        return (Const_Val){ .soft_int = result };
    }
    else if (Type_IsInteger(type))
    {
        imm size = Type_Sizeof(type);
        Big_Int result = BigInt_SignExtend(BigInt_ChopToSize(BigInt_BitSplatLeft(v0.soft_int, v1.soft_int), size), size);
        return (Const_Val){ .soft_int = result };
    }
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_BitShr(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (Type_IsInteger(type)) return (Const_Val){ .soft_int = BigInt_BitShl(v0.soft_int, v1.soft_int) };
    else NOT_IMPLEMENTED;
}

internal inline Const_Val
ConstVal_BitSar(Const_Val v0, Const_Val v1, Type_ID type)
{
    if (Type_IsInteger(type)) return (Const_Val){ .soft_int = BigInt_BitSar(v0.soft_int, v1.soft_int) };
    else NOT_IMPLEMENTED;
}