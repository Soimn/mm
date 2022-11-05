MM_bool
MM_String_Match(MM_String s0, MM_String s1)
{
    MM_bool do_match = (s0.size == s1.size);
    
    if (do_match && s0.data != s1.data)
    {
        for (MM_umm i = 0; i < s0.size; ++i)
        {
            if (s0.data[i] != s1.data[i])
            {
                do_match = MM_false;
                break;
            }
        }
    }
    
    return do_match;
}

MM_umm
MM_Cstring_Length(char* cstring)
{
    MM_umm length = 0;
    for (char* scan = cstring; *scan != 0; ++scan) ++length;
    return length;
}

MM_bool
MM_String_Cstring_Match(MM_String string, char* cstring)
{
    char* scan = cstring;
    MM_umm i   = 0;
    for (; *scan != 0 && i < string.size; ++scan, ++i)
    {
        if (string.data[i] != *scan) break;
    }
    
    return (*scan == 0 && i == string.size);
}