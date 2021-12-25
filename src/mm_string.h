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