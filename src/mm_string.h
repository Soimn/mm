internal inline u8
ToLowerCase(u8 c)
{
    return (c >= 'A' && c <= 'A' ? (c - 'A') + 'a' : c);
}

internal inline u8
ToUpperCase(u8 c)
{
    return (c >= 'a' && c <= 'z' ? (c - 'a') + 'A' : c);
}

internal inline bool
IsWhitespace(u8 c)
{
    return (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r' || c == '\n');
}

internal inline bool
IsAlpha(u8 c)
{
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z');
}

internal inline bool
IsDigit(u8 c)
{
    return (c >= '0' && c <= '9');
}

internal bool
String_Compare(String s0, String s1)
{
    if (s0.size == s1.size)
    {
        while (s0.size != 0 && *s0.data == *s1.data)
        {
            ++s0.data;
            --s0.size;
            
            ++s1.data;
            --s1.size;
        }
    }
    
    return (s0.size == 0 && s1.size == 0);
}

internal bool
String_CompareCaseInsensitive(String s0, String s1)
{
    if (s0.size == s1.size)
    {
        while (s0.size != 0 && ToLowerCase(*s0.data) == ToLowerCase(*s1.data))
        {
            ++s0.data;
            --s0.size;
            
            ++s1.data;
            --s1.size;
        }
    }
    
    return (s0.size == 0 && s1.size == 0);
}

internal umm
String_FormatArgList(Buffer out, const char* format, va_list args)
{
    umm required_bytes = 0;
    
    for (char* scan = (char*)format; *scan; )
    {
        if (*scan == '%')
        {
            ++scan;
            
            switch (*scan)
            {
                case '%':
                {
                    if (out.size > required_bytes) out.data[required_bytes] = *scan;
                    ++required_bytes;
                    ++scan;
                } break;
                
                case 'f':
                {
                    f64 val = va_arg(args, f64);
                    
                    ++scan;
                    
                    if (val < 0)
                    {
                        if (required_bytes < out.size) out.data[required_bytes] = '-';
                        required_bytes += 1;
                        
                        val = -val;
                    }
                    
                    // HACK
                    umm largest_place = 1;
                    u64 val_copy = (u64)val;
                    while (val_copy /= 10) largest_place *= 10;
                    
                    while (largest_place != 0)
                    {
                        if (required_bytes < out.size) out.data[required_bytes] = ((u64)val / largest_place) % 10 + '0';
                        largest_place  /= 10;
                        required_bytes += 1;
                    }
                    
                    if (required_bytes < out.size) out.data[required_bytes] = '.';
                    required_bytes += 1;
                    
                    val -= (u64)val;
                    val *= 1000;
                    
                    largest_place = 1;
                    val_copy = (u64)val;
                    while (val_copy /= 10) largest_place *= 10;
                    
                    while (largest_place != 0)
                    {
                        if (required_bytes < out.size) out.data[required_bytes] = ((u64)val / largest_place) % 10 + '0';
                        largest_place  /= 10;
                        required_bytes += 1;
                    }
                } break;
                
                case 'u':
                case 'U':
                case 'i':
                case 'I':
                {
                    u64 val;
                    if      (*scan == 'u') val = va_arg(args, u32);
                    else if (*scan == 'U') val = va_arg(args, u64);
                    else
                    {
                        i64 i_val;
                        if      (*scan == 'i') i_val = va_arg(args, i32);
                        else                   i_val = va_arg(args, i64);
                        
                        if (i_val < 0)
                        {
                            if (required_bytes < out.size) out.data[required_bytes] = '-';
                            required_bytes += 1;
                            
                            i_val  = -i_val;
                        }
                        
                        val = (u64)i_val;
                    }
                    
                    ++scan;
                    
                    umm largest_place = 1;
                    u64 val_copy = val;
                    while (val_copy /= 10) largest_place *= 10;
                    
                    while (largest_place != 0)
                    {
                        if (required_bytes < out.size) out.data[required_bytes] = (val / largest_place) % 10 + '0';
                        largest_place  /= 10;
                        required_bytes += 1;
                    }
                } break;
                
                case 'x':
                case 'X':
                {
                    u64 val;
                    if      (*scan == 'x') val = va_arg(args, u32);
                    else                   val = va_arg(args, u64);
                    
                    umm largest_place = 1;
                    u64 val_copy = val;
                    while (val_copy /= 16) largest_place *= 16;
                    
                    while (largest_place != 0)
                    {
                        u8 digit = (val / largest_place) % 16;
                        
                        if (required_bytes < out.size) out.data[required_bytes] = (digit <= '9' ? digit + '0' : digit + 'A');
                        largest_place  /= 16;
                        required_bytes += 1;
                    }
                } break;
                
                case 'b':
                {
                    ++scan;
                    
                    bool val = va_arg(args, bool);
                    
                    char* str;
                    if (val) str = "true";
                    else     str = "false";
                    
                    for (; *str; ++str)
                    {
                        if (required_bytes < out.size) out.data[required_bytes] = *str;
                        required_bytes += 1;
                    }
                } break;
                
                case 'c':
                {
                    ++scan;
                    
                    char c = va_arg(args, char);
                    
                    if (required_bytes < out.size) out.data[required_bytes] = c;
                    required_bytes += 1;
                    
                } break;
                
                case 's':
                {
                    ++scan;
                    
                    char* str = va_arg(args, char*);
                    
                    for (; *str; ++str)
                    {
                        if (required_bytes < out.size) out.data[required_bytes] = *str;
                        required_bytes += 1;
                    }
                } break;
                
                case 'S':
                {
                    ++scan;
                    
                    String string = va_arg(args, String);
                    
                    for (umm i = 0; i < string.size; ++i)
                    {
                        if (required_bytes < out.size) out.data[required_bytes] = string.data[i];
                        required_bytes += 1;
                    }
                } break;
                
                INVALID_DEFAULT_CASE;
            }
        }
        
        else
        {
            if (out.size > required_bytes) out.data[required_bytes] = *scan;
            ++required_bytes;
            ++scan;
        }
    }
    
    if (out.size > required_bytes) out.data[required_bytes] = 0;
    ++required_bytes;
    
    return required_bytes;
}

internal umm
String_Format(Buffer out, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    umm result = String_FormatArgList(out, format, args);
    va_end(args);
    
    return result;
}

internal String
String_FromCString(u8* cstring)
{
    String result = { .data = cstring };
    
    for (u8* scan = cstring; *scan; ++scan) ++result.size;
    
    return result;
}

internal bool
String_HasPrefix(String string, String prefix)
{
    return String_Compare((String){string.data, prefix.size}, prefix);
}

internal String
String_Tail(String string, umm index)
{
    return (String){.data = string.data + index, .size = string.size - index};
}

internal u64
String_Hash(String string)
{
    u64 p_n   = 1;
    u64 hash  = 0;
    u64 m     = (u64)1.0e9 + 9;
    
    for (umm i = 0; i < string.size; ++i)
    {
        hash += ((u64)string.data[i] * p_n) % m;
        p_n   = (p_n * 127) % m;
    }
    
    return hash;
}

typedef struct Interned_String_Entry
{
    Interned_String next;
    u32 hash;
    String string;
    u8 data[];
} Interned_String_Entry;

#define STRING_TO_ENTRY_POINTER(interned_string) (Interned_String_Entry*)(((interned_string) - sizeof(Interned_String_Entry)) + MM.intern_arena.base_address)

#define POINTER_TO_INTERNED_STRING(pointer) (Interned_String)(((u64)(pointer) - MM.intern_arena.base_address) + sizeof(Interned_String_Entry))

internal Interned_String
String_Intern(String string)
{
    Interned_String result = INTERNED_EMPTY_STRING;
    
    if (string.size != 0)
    {
        u32 hash        = (u32)String_Hash(string);
        u32 table_index = hash % ARRAY_SIZE(MM.intern_table);
        
        Interned_String_Entry* prev_entry = 0;
        
        for (Interned_String scan = MM.intern_table[table_index]; scan != 0; )
        {
            Interned_String_Entry* entry = STRING_TO_ENTRY_POINTER(scan);
            
            if (entry->hash == hash && String_Compare(entry->string, string))
            {
                result = scan;
                break;
            }
            
            prev_entry = entry;
            scan       = entry->next;
        }
        
        if (result == 0)
        {
            Interned_String_Entry* entry = Arena_PushSize(&MM.intern_arena,
                                                          sizeof(Interned_String_Entry) + string.size + 1,
                                                          ALIGNOF(Interned_String_Entry));
            
            *entry = (Interned_String_Entry){
                .next        = 0,
                .hash        = hash,
                .string.data = entry->data,
                .string.size = string.size,
            };
            
            Copy(string.data, entry->string.data, string.size);
            entry->string.data[entry->string.size] = 0;
            
            result = POINTER_TO_INTERNED_STRING(entry);
            
            if (prev_entry != 0) prev_entry->next             = result;
            else                 MM.intern_table[table_index] = result;
        }
    }
    
    return result;
}

internal String
Identifier_Of(Interned_String istring)
{
    String string = STRING("_");
    
    if (istring != 0)
    {
        Interned_String_Entry* entry = STRING_TO_ENTRY_POINTER(istring);
        
        ASSERT(String_Intern(entry->string) == istring);
        
        string = entry->string;
    }
    
    return string;
}

internal String
StringLiteral_Of(Interned_String istring)
{
    String string = STRING("");
    
    if (istring != 0)
    {
        Interned_String_Entry* entry = STRING_TO_ENTRY_POINTER(istring);
        
        ASSERT(String_Intern(entry->string) == istring);
        
        string = entry->string;
    }
    
    return string;
}

#undef STRING_TO_ENTRY_POINTER
#undef POINTER_TO_INTERNED_STRING

internal bool
String_IsKeyword(Interned_String string, Enum8(KEYWORD_KIND) keyword)
{
    return (MM.keyword_strings[keyword] == string);
}

u32
Character_ToCodepoint(Character c)
{
    u32 codepoint;
    
    if ((c.bytes[0] & 0x80) == 0)
    {
        codepoint = c.bytes[0];
    }
    
    else if ((c.bytes[0] & 0xE0) == 0xC0)
    {
        codepoint = (u32)(c.bytes[0] & 0x1F) << 6 | (u32)(c.bytes[1] & 0x3F);
    }
    
    else if ((c.bytes[0] & 0xF0) == 0xE0)
    {
        codepoint = (u32)(c.bytes[0] & 0x0F) << 12 | (u32)(c.bytes[1] & 0x3F) << 6 | (u32)(c.bytes[2] & 0x3F);
    }
    
    else
    {
        codepoint = (u32)(c.bytes[0] & 0x07) << 18 | (u32)(c.bytes[1] & 0x3F) << 12 | (u32)(c.bytes[2] & 0x3F) << 6 | (u32)(c.bytes[3] & 0x3F);
    }
    
    return codepoint;
}