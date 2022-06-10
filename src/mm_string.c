MM_API MM_String
MM_String_WrapCString(char* cstring)
{
    MM_String string = { .data = (MM_u8*)cstring };
    
    for (char* scan = cstring; *scan != 0; ++scan, ++string.size);
    
    return string;
}

MM_API MM_bool
MM_String_Match(MM_String s0, MM_String s1)
{
    MM_bool result = (s0.size == s1.size);
    
    if (s0.data != s1.data)
    {
        for (MM_umm i = 0; i < s0.size && result; ++i)
        {
            result = (s0.data[i] == s1.data[i]);
        }
    }
    
    return result;
}

MM_API MM_u64
MM_String_Hash(MM_String string)
{
    // TODO: hash function
    return (string.size != 0 ? *string.data : 0);
}