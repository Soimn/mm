
// TODO: Replace this
internal inline u64
String_HashOf(String string)
{
    umm result = 0;
    
    umm x   = 257;
    umm m   = 4294967291;
    umm x_n = 1;
    
    for (umm i = 0; i < string.size; ++i)
    {
        result += string.data[i] * x_n;
        x_n *= x;
    }
    
    result %= m;
    
    return result;
}

internal inline bool
String_Match(String s0, String s1)
{
    bool result = false;
    
    if (s0.size == s1.size)
    {
        umm i = 0;
        for (; i < s0.size; ++i)
        {
            if (s0.data[i] != s1.data[i]) break;
        }
        
        result = (i == s0.size);
    }
    
    return result;
}

internal inline String
String_FromInternedString(Interned_String string)
{
    Interned_String_Entry* slot = (Interned_String_Entry*)string;
    if (string <= KEYWORD_KIND_MAX)
    {
        slot = MM.keyword_table[string];
    }
    
    return (String){ .data = (u8*)(slot + 1), .size = slot->size };
}