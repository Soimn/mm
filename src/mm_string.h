MM_bool
MM_String_Match(MM_String s0, MM_String s1)
{
    MM_bool result = (s0.size == s1.size);
    
    if (s0.data != s1.data)
    {
        for (MM_u64 i = 0; result && i < s0.size; ++i)
        {
            result = (s0.data[i] == s1.data[i]);
        }
    }
    
    return result;
}