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

typedef MM_u64 MM_Interned_String;

#define MM_INTERNED_STRING_NIL 0

typedef struct MM_Interned_String_Entry
{
    struct MM_Interned_String_Entry* next;
    MM_u64 hash;
    MM_u32 size;
    MM_u8 data[];
} MM_Interned_String_Entry;

typedef struct MM_Intern_Table
{
    MM_u64 table_size;
    MM_Interned_String_Entry** table;
} MM_Intern_Table;

MM_String
MM_InternedString_ToString(MM_Intern_Table* table, MM_Interned_String istring)
{
    MM_NOT_IMPLEMENTED;
    return (MM_String){};
}

MM_Interned_String
MM_InternedString_FromString(MM_Intern_Table* table, MM_String string)
{
    MM_NOT_IMPLEMENTED;
    return MM_INTERNED_STRING_NIL;
}