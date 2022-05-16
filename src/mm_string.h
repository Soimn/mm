internal String
String_WrapCString(char* cstring)
{
    String string = { .data = (u8*)cstring };
    
    for (char* scan = cstring; *scan != 0; ++scan, ++string.size);
    
    return string;
}

internal u64
String_Hash(String string)
{
    // TODO: hash function
    return (string.size != 0 ? *string.data : 0);
}

internal bool
String_Match(String s0, String s1)
{
    bool result = (s0.size == s1.size);
    
    for (umm i = 0; i < s0.size && result; ++i)
    {
        result = (s0.data[i] == s1.data[i]);
    }
    
    return result;
}

internal void
String_Printf(Arena* arena, char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    char* scan = format;
    while (*scan != 0)
    {
        char* start = scan;
        while (*scan != 0 && *scan != '%')
        {
            if (scan[0] == '\\' && scan[1] != 0) scan += 1;
            scan += 1;
        }
        
        Arena_PushCopy(arena, start, scan - start, ALIGNOF(u8));
        
        if (*scan == '%')
        {
            ++scan;
            char c = *scan++;
            
            if      (c == '%') Arena_PushCopy(arena, "%", 1, ALIGNOF(u8));
            else if (c == 'S' || c == 's')
            {
                String string = (c == 'S' ? va_arg(args, String) : String_WrapCString(va_arg(args, char*)));
                
                Arena_PushCopy(arena, string.data, string.size, ALIGNOF(u8));
            }
            else if (c == 'c')
            {
                u32 character = va_arg(args, u32);
                
                if (character <= 0x00007F)
                {
                    char* cursor = Arena_PushSize(arena, 1, ALIGNOF(u8));
                    *cursor = (u8)character;
                }
                else if (character <= 0x0007FF)
                {
                    char* cursor = Arena_PushSize(arena, 2, ALIGNOF(u8));
                    cursor[0] = 0xC0 | (u8)(character >> 6);
                    cursor[1] = 0x80 | (u8)(character >> 0) & 0x3F;
                }
                else if (character <= 0x00FFFF)
                {
                    char* cursor = Arena_PushSize(arena, 3, ALIGNOF(u8));
                    cursor[0] = 0xE0 | (u8)(character >> 12);
                    cursor[1] = 0x80 | (u8)(character >> 6) & 0x3F;
                    cursor[2] = 0x80 | (u8)(character >> 0) & 0x3F;
                }
                else if (character <= 0x10FFFF)
                {
                    char* cursor = Arena_PushSize(arena, 4, ALIGNOF(u8));
                    cursor[0] = 0xF0 | (u8)(character >> 18);
                    cursor[1] = 0x80 | (u8)(character >> 12) & 0x3F;
                    cursor[2] = 0x80 | (u8)(character >> 6)  & 0x3F;
                    cursor[3] = 0x80 | (u8)(character >> 0)  & 0x3F;
                }
                else INVALID_CODE_PATH;
            }
            else if (c == 'U' || c == 'u' ||
                     c == 'I' || c == 'i')
            {
                umm num;
                imm inum = 0;
                
                if      (c == 'U') num = va_arg(args, u64);
                else if (c == 'u') num = va_arg(args, u32);
                else
                {
                    inum = (c == 'I' ? va_arg(args, i64) : va_arg(args, i32));
                    
                    if (inum < 0) num = (umm)-inum;
                    else          num = (umm)+inum;
                }
                
                umm place = 1;
                umm size  = 1;
                for (umm copy = num / 10; copy != 0; copy /= 10) place *= 10, size += 1;
                
                char* cursor = Arena_PushSize(arena, size + (inum < 0), ALIGNOF(u8));
                if (inum < 0) *cursor++ = '-';
                
                do
                {
                    *cursor++ = (num / place) % 10;
                    place /= 10;
                } while (place != 0);
            }
            else if (c == 'F' || c == 'f')
            {
                f64 fnum = (c == 'F' ? va_arg(args, f64) : va_arg(args, f32));
                NOT_IMPLEMENTED;
            }
            else if (c == 'B' && *scan == 'F')
            {
                ++scan;
                
                Big_Float num = va_arg(args, Big_Float);
                u8 byte_size  = va_arg(args, u8);
                
                if (byte_size == 0)
                {
                    NOT_IMPLEMENTED;
                }
                else if (byte_size == 2)
                {
                    NOT_IMPLEMENTED;
                }
                else if (byte_size == 4)
                {
                    NOT_IMPLEMENTED;
                }
                else if (byte_size == 8)
                {
                    NOT_IMPLEMENTED;
                }
                else if (byte_size == 16)
                {
                    NOT_IMPLEMENTED;
                }
                else INVALID_CODE_PATH;
            }
            else if (c == 'X' || c == 'x' || c == 'B' && *scan == 'X')
            {
                scan += (c == 'B');
                
                Big_Int num = (c == 'B' ? va_arg(args, Big_Int) : BigInt_FromU64(c == 'X' ? va_arg(args, u64) : va_arg(args, u32)));
                u8 base     = va_arg(args, u8);
                
                Big_Int place  = BigInt_FromU64(1);
                umm size       = 1;
                for (Big_Int copy = BigInt_DivU64(num, base); !BigInt_IsZero(copy); copy = BigInt_DivU64(copy, base)) place = BigInt_MulU64(place, base), size += 1;
                
                bool is_negative = (base == 0 && BigInt_IsLessU64(num, 0));
                char* cursor = Arena_PushSize(arena, size + is_negative, ALIGNOF(u8));
                
                if (is_negative)
                {
                    *cursor++ = '-';
                    num = BigInt_Neg(num);
                }
                
                base = (base == 0 ? 10 : base);
                do
                {
                    *cursor++ = BigInt_ChopToU64(BigInt_Div(num, place), sizeof(u64)) % base;
                    place = BigInt_DivU64(place, base);
                } while (!BigInt_IsZero(place));
            }
            else INVALID_CODE_PATH;
        }
    }
    
    va_end(args);
}

#define INTERNED_STRING_NIL 0
#define INTERN_MAP_SIZE 512

typedef u64 Interned_String;

typedef struct Interned_String_Entry
{
    struct Interned_String_Entry* next;
    u64 hash;
    u64 size;
} Interned_String_Entry;

#define LIST_KEYWORDS()               \
X(EMPTY_STRING,      "")          \
X(BLANK_IDENTIFIER,  "_")         \
X(Keyword_Include,   "include")   \
X(Keyword_Proc,      "proc")      \
X(Keyword_Struct,    "struct")    \
X(Keyword_Union,     "union")     \
X(Keyword_Enum,      "enum")      \
X(Keyword_True,      "true")      \
X(Keyword_False,     "false")     \
X(Keyword_As,        "as")        \
X(Keyword_If,        "if")        \
X(Keyword_Else,      "else")      \
X(Keyword_While,     "while")     \
X(Keyword_Break,     "break")     \
X(Keyword_Continue,  "continue")  \
X(Keyword_Using,     "using")     \
X(Keyword_Defer,     "defer")     \
X(Keyword_Return,    "return")    \
X(Keyword_Cast,      "cast")      \
X(Keyword_Transmute, "transmute") \
X(Keyword_Where,     "where")     \

enum
{
    Keyword_Dummy_Acc_Sentinel = sizeof(Interned_String_Entry),
    
#define X(e, s)                                                                                                     \
Keyword_Dummy_Acc_##e,                                                                                          \
e = ((Keyword_Dummy_Acc_##e - 1) + (sizeof(Interned_String_Entry) - 1)) & ~(sizeof(Interned_String_Entry) - 1), \
Keyword_Dummy_Pad_##e = e + sizeof(s) - 1,
    
    LIST_KEYWORDS()
        
#undef X
    
    Keyword_Dummy_Pad_Sentinel,
};

internal Interned_String_Entry**
InternedString__FindSpot(Workspace* workspace, u64 hash, String string)
{
    Interned_String_Entry** entry = (Interned_String_Entry**)Arena_BasePointer(workspace->string_arena) + (hash % INTERN_MAP_SIZE);
    
    for (; *entry != 0; entry = &(*entry)->next)
    {
        if ((*entry)->hash == hash && String_Match(string, (String){ .data = (u8*)(*entry + 1), .size = (*entry)->size })) break;
    }
    
    return entry;
}

internal inline Interned_String
InternedString__FromEntry(Workspace* workspace, Interned_String_Entry* entry)
{
    return (Interned_String)(entry - (Interned_String_Entry*)((u8*)Arena_BasePointer(workspace->string_arena) + sizeof(Interned_String_Entry*) * INTERN_MAP_SIZE));
}

internal inline Interned_String_Entry*
InternedString__ToEntry(Workspace* workspace, Interned_String string)
{
    return (Interned_String_Entry*)((u8*)Arena_BasePointer(workspace->string_arena) + sizeof(Interned_String_Entry*) * INTERN_MAP_SIZE + sizeof(Interned_String_Entry) * string);
}

internal void
InternedString__InitMapAndArray(Workspace* workspace)
{
    Interned_String_Entry** intern_map = Arena_PushSize(workspace->string_arena, sizeof(Interned_String_Entry*) * INTERN_MAP_SIZE, ALIGNOF(Interned_String_Entry*));
    ZeroArray(intern_map, INTERN_MAP_SIZE);
    
    Interned_String_Entry* nil_entry = Arena_PushSize(workspace->string_arena, sizeof(Interned_String_Entry), ALIGNOF(Interned_String_Entry));
    ZeroStruct(nil_entry);
    
#define X(e, s) Interned_String_Entry* e##_entry = Arena_PushSize(workspace->string_arena, sizeof(Interned_String_Entry) + sizeof(s) - 1, ALIGNOF(Interned_String_Entry)); \
u64 e##_hash = String_Hash(STRING(s));                                                                                                                                    \
Interned_String_Entry** e##_spot = InternedString__FindSpot(workspace, e##_hash, STRING(s));                                                                              \
ASSERT(*e##_spot == 0);                                                                                                                                                   \
*e##_spot = e##_entry;                                                                                                                                                    \
*e##_entry = (Interned_String_Entry){ .hash = e##_hash, .size = sizeof(s) - 1 };                                                                                          \
Copy(s, (u8*)(e##_entry + 1), sizeof(s) - 1);                                                                                                                             \
    
    LIST_KEYWORDS()
        
#undef X
}

internal Interned_String
InternedString_FromString(Workspace* workspace, String string)
{
    Interned_String_Entry** entry = InternedString__FindSpot(workspace, String_Hash(string), string);
    
    return (*entry == 0 ? INTERNED_STRING_NIL : InternedString__FromEntry(workspace, *entry));
}

internal String
InternedString_ToString(Workspace* workspace, Interned_String string)
{
    ASSERT(string != INTERNED_STRING_NIL);
    Interned_String_Entry* entry = InternedString__ToEntry(workspace, string);
    
    return (String){ .data = (u8*)(entry + 1), .size = entry->size };
}

internal bool
InternedString_IsKeyword(Interned_String string)
{
    return (string >= Keyword_Dummy_Acc_Sentinel && string < Keyword_Dummy_Pad_Sentinel);
}

internal bool
InternedString_IsIdentifier(Interned_String string)
{
    return (string >= Keyword_Dummy_Pad_Sentinel || string == BLANK_IDENTIFIER);
}

internal bool
InternedString_IsNonBlankIdentifier(Interned_String string)
{
    return (string >= Keyword_Dummy_Pad_Sentinel);
}