typedef struct MM_String
{
    MM_u8* data;
    MM_u32 size;
} MM_String;

#define MM_STRING(str) (String){ .data = (MM_u8*)(str), .size = sizeof(str) - 1 }

MM_String
MM_String_WrapCString(char* cstring)
{
    MM_String string = { .data = (MM_u8*)cstring };
    
    for (char* scan = cstring; *scan != 0; ++scan, ++string.size);
    
    return string;
}

MM_bool
MM_String_Match(MM_String s0, MM_String s1)
{
    MM_bool result = (s0.size == s1.size);
    
    for (MM_umm i = 0; i < s0.size && result; ++i)
    {
        result = (s0.data[i] == s1.data[i]);
    }
    
    return result;
}

MM_u64
MM_String_Hash(MM_String string)
{
    // TODO: hash function
    return (string.size != 0 ? *string.data : 0);
}

typedef MM_String MM_Interned_String;

MM_bool
MM_InternedString_Match(MM_Interned_String s0, MM_Interned_String s1)
{
    MM_ASSERT(s0.data != s1.data || s0.size == s1.size);
    return (s0.data == s1.data);
}

typedef struct MM_Intern_Table
{
    
} MM_Intern_Table;

MM_InternedString_FromString(MM_Intern_Table* table, MM_String)
{
}