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

internal String
String_Printf(Arena* arena, char* format, ...)
{
    String result = { .data = Arena_OffsetPointer(arena) };
    
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
                    *cursor++ = (num / place) % 10 + '0';
                    place /= 10;
                } while (place != 0);
            }
            else if (c == 'F' || c == 'f')
            {
                f64 fnum = (c == 'F' ? va_arg(args, f64) : va_arg(args, f32));
                f64 abs_fnum = (fnum < 0 ? -fnum : fnum);
                
                // TODO: replace this
                umm num = (umm)abs_fnum;
                umm place = 1;
                umm size  = 1;
                for (umm copy = num / 10; copy != 0; copy /= 10) place *= 10, size += 1;
                
                char* cursor = Arena_PushSize(arena, size + (fnum < 0), ALIGNOF(u8));
                if (fnum < 0) *cursor++ = '-';
                
                do
                {
                    *cursor++ = (num / place) % 10 + '0';
                    place /= 10;
                } while (place != 0);
                
                while ((umm)(abs_fnum * 2) != (umm)abs_fnum * 2) abs_fnum *= 2;
                
                num = (umm)abs_fnum;
                place = 1;
                size  = 1;
                for (umm copy = num / 10; copy != 0; copy /= 10) place *= 10, size += 1;
                
                cursor = Arena_PushSize(arena, size + 1, ALIGNOF(u8));
                *cursor++ = '.';
                
                do
                {
                    *cursor++ = (num / place) % 10 + '0';
                    place /= 10;
                } while (place != 0);
            }
            else if (c == 'X' || c == 'x' || c == 'B' && *scan == 'X')
            {
                scan += (c == 'B');
                
                I256 num         = (c == 'B' ? va_arg(args, I256) : I256_FromU64(c == 'X' ? va_arg(args, u64) : va_arg(args, u32)));
                u8 base          = va_arg(args, u8);
                I256 base_256    = I256_FromU64(base);
                bool is_negative = false;
                
                if (base == 0)
                {
                    base        = 10;
                    base_256    = I256_FromU64(base);
                    is_negative = I256_IsLess(num, I256_0);
                }
                
                I256 place     = I256_FromU64(1);
                umm size       = 1;
                for (I256 copy = I256_Div(num, base_256, &(I256){0}); !I256_IsZero(copy); copy = I256_Div(copy, base_256, &(I256){0})) place = I256_Mul(place, base_256), size += 1;
                
                char* cursor = Arena_PushSize(arena, size + is_negative, ALIGNOF(u8));
                
                if (is_negative)
                {
                    *cursor++ = '-';
                    num = I256_Neg(num);
                }
                
                do
                {
                    *cursor++ = I256_ChopToU64(I256_Div(num, place, &(I256){0}), sizeof(u64)) % base + '0';
                    place = I256_Div(place, base_256, &(I256){0});
                } while (!I256_IsZero(place));
            }
            else INVALID_CODE_PATH;
        }
    }
    
    va_end(args);
    
    result.size = (u8*)Arena_OffsetPointer(arena) - result.data;
    return result;
}

typedef u64 Interned_String;

typedef struct Interned_String_Entry
{
    struct Interned_String_Entry* next;
    u64 hash;
    u64 size;
    u8 data[];
} Interned_String_Entry;

#define INTERN_MAP_SIZE 512

#define INTERNED_STRING_NIL 0
#define EMPTY_STRING (sizeof(Interned_String_Entry*)*INTERN_MAP_SIZE)
#define BLANK_IDENTIFIER EMPTY_STRING + sizeof(Interned_String_Entry)
#define KEYWORD_OFFSET BLANK_IDENTIFIER + sizeof(Interned_String_Entry) + (sizeof("_") - 1)

#define LIST_KEYWORDS()               \
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
    Keyword_Dummy_Low_Sentinel = KEYWORD_OFFSET,
    
#define X(e, s)                                                                                                        \
Keyword_Dummy_Low_##e,                                                                                             \
e = ((Keyword_Dummy_Low_##e - 1) +  (ALIGNOF(Interned_String_Entry) - 1)) & ~(ALIGNOF(Interned_String_Entry) - 1), \
Keyword_Dummy_High_##e = e + sizeof(Interned_String_Entry) + sizeof(s) - 1,
    
    LIST_KEYWORDS()
#undef X
    
    Keyword_Dummy_High_Sentinel,
};

#define KEYWORD_MAX Keyword_Dummy_High_Sentinel - 1

internal Interned_String_Entry**
InternedString__FindSpot(Workspace* workspace, u64 hash, String string)
{
    Interned_String_Entry** map = Arena_BasePointer(workspace->string_arena);
    
    Interned_String_Entry** entry = &map[hash % INTERN_MAP_SIZE];
    for (; *entry != 0; entry = &(*entry)->next)
    {
        if (hash == (*entry)->hash && String_Match(string, (String){ .data = (*entry)->data, .size = (*entry)->size })) break;
        else                                                                                                            continue;
    }
    
    return entry;
}

internal Interned_String_Entry*
InternedString__ToEntry(Workspace* workspace, Interned_String string)
{
    ASSERT(string != 0);
    return (Interned_String_Entry*)((u8*)Arena_BasePointer(workspace->string_arena) + string);
}

internal Interned_String
InternedString__FromEntry(Workspace* workspace, Interned_String_Entry* entry)
{
    return (Interned_String)((u8*)entry - (u8*)Arena_BasePointer(workspace->string_arena));
}

internal void
InternedString__Init(Workspace* workspace)
{
    Interned_String_Entry** map = Arena_PushSize(workspace->string_arena, sizeof(Interned_String_Entry*)*INTERN_MAP_SIZE,
                                                 ALIGNOF(Interned_String_Entry*));
    ZeroArray(map, INTERN_MAP_SIZE);
    
#define X(e, s)\
Interned_String_Entry* e##_entry = Arena_PushSize(workspace->string_arena, sizeof(Interned_String_Entry), ALIGNOF(Interned_String_Entry)); \
String e##_string = STRING(s);                                                                                                             \
u64 e##_hash      = String_Hash(e##_string);                                                                                               \
e##_entry->next = 0;                                                                                                                       \
e##_entry->hash = e##_hash;                                                                                                                \
e##_entry->size = e##_string.size;                                                                                                         \
Arena_PushCopy(workspace->string_arena, e##_string.data, e##_string.size, ALIGNOF(u8));                                                    \
*InternedString__FindSpot(workspace, e##_hash, e##_string) = e##_entry;
    
    X(EMPTY_STRING, "");
    X(BLANK_IDENTIFIER, "_");
    
    LIST_KEYWORDS()
#undef X
}

internal String
InternedString_ToString(Workspace* workspace, Interned_String string)
{
    ASSERT(string != 0);
    Interned_String_Entry* entry = InternedString__ToEntry(workspace, string);
    return (String){ .data = entry->data, .size = entry->size };
}

internal Interned_String
InternedString_FromString(Workspace* workspace, String string)
{
    Interned_String_Entry** entry = InternedString__FindSpot(workspace, String_Hash(string), string);
    return (*entry != 0 ? InternedString__FromEntry(workspace, *entry) : INTERNED_STRING_NIL);
}

internal bool
InternedString_IsKeyword(Interned_String string)
{
    return (string >= KEYWORD_OFFSET && string <= KEYWORD_MAX);
}

internal bool
InternedString_IsNonBlankIdentifier(Interned_String string)
{
    return (string > KEYWORD_MAX);
}