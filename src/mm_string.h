inline MM_String
MM_String_FromCString(char* cstring)
{
    MM_String string = { .data = (MM_u8*)cstring };
    
    for (char* scan = cstring; *scan != 0; ++scan) ++string.size;
    
    return string;
}

inline MM_bool
MM_String_Match(MM_String s0, MM_String s1)
{
    MM_bool result = MM_false;
    
    if (s0.size == s1.size)
    {
        if (s0.data == s1.data) result = MM_true;
        else
        {
            result = MM_true;
            for (MM_u32 i = 0; i < s0.size && result; ++i) result = (s0.data[i] == s1.data[i]);
        }
    }
    
    return result;
}

inline MM_String
MM_String_Skip(MM_String string, MM_u32 n)
{
    n = MM_MIN(string.size, n);
    
    return (MM_String){
        .data = string.data + n,
        .size = string.size - n
    };
}

inline MM_String
MM_String_Chop(MM_String string, MM_u32 n)
{
    n = MM_MIN(string.size, n);
    
    return (MM_String){
        .data = string.data,
        .size = string.size - n
    };
}