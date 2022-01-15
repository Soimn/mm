typedef struct Big_Int
{
    bool _stub;
} Big_Int;

typedef struct Big_Float
{
    bool _stub;
} Big_Float;

internal Big_Int BigInt_FromBigFloat(Big_Float val, bool* did_truncate);
internal Big_Int BigInt_FromU64(u64 val);
internal umm BigInt_EffectiveByteSize(Big_Int val);
internal Big_Int BigInt_TruncateToByteSize(Big_Int val, umm byte_size);
internal Big_Int BigInt_SignExtendFromByteSize(Big_Int val, umm byte_size);
internal Big_Int BigInt_Negate(Big_Int val, imm byte_size);
internal Big_Int BigInt_Complement(Big_Int val, imm byte_size);
internal Big_Int BigInt_Add(Big_Int v0, Big_Int v1, imm byte_size);
internal Big_Int BigInt_BitOr(Big_Int v0, Big_Int v1, imm byte_size);
internal Big_Int BigInt_BitXor(Big_Int v0, Big_Int v1, imm byte_size);
internal Big_Int BigInt_BitAnd(Big_Int v0, Big_Int v1, imm byte_size);
internal Big_Int BigInt_Rem(Big_Int v0, Big_Int v1, imm byte_size);
internal Big_Int BigInt_LeftShift(Big_Int v0, Big_Int v1, imm byte_size);
internal Big_Int BigInt_RightShift(Big_Int v0, Big_Int v1, bool is_arithmetic, imm byte_size);
internal bool BigInt_IsEqual(Big_Int v0, Big_Int v1);
internal bool BigInt_IsLess(Big_Int v0, Big_Int v1);
internal bool BigInt_IsGreater(Big_Int v0, Big_Int v1);
internal bool BigInt_IsZero(Big_Int val);

internal umm BigFloat_EffectiveByteSize(Big_Float val);
internal Big_Float BigFloat_FromBigInt(Big_Int val);
internal Big_Float BigFloat_FromF64(f64 val);
internal Big_Float BigFloat_Negate(Big_Float val, imm byte_size);