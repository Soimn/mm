typedef union Const_Val
{
    u64 uint64;
    i64 int64;
    Big_Int soft_int;
    Big_Float soft_float;
    f32 float32;
    f64 float64;
    bool boolean;
    Interned_String string;
    Type_ID type_id;
    
    void* data;
} Const_Val;