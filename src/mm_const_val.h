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