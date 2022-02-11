#define MM__U8_MAX  0xFF
#define MM__U16_MAX 0xFFFF
#define MM__U32_MAX 0xFFFFFFFF
#define MM__U64_MAX 0xFFFFFFFFFFFFFFFF

#define MM__I8_MIN  0xFF
#define MM__I16_MIN 0xFFFF
#define MM__I32_MIN 0xFFFFFFFF
#define MM__I64_MIN 0xFFFFFFFFFFFFFFFF

#define MM__I8_MAX  0x7F
#define MM__I16_MAX 0x7FFF
#define MM__I32_MAX 0x7FFFFFFF
#define MM__I64_MAX 0x7FFFFFFFFFFFFFFF

#if MM_DEBUG
#define MM__ASSERT(EX) ((EX) ? 1 : (*(volatile int*)0 = 0))
#else
#define MM__ASSERT(EX)
#endif

#define MM__NOT_IMPLEMENTED MM__ASSERT(!"NOT_IMPLEMENTED")
#define MM__INVALID_DEFAULT_CASE default: MM__ASSERT(!"INVALID_DEFAULT_CASE"); break
#define MM__INVALID_CODE_PATH MM__ASSERT(!"INVALID_CODE_PATH")

#define MM__OFFSETOF(element, type) (umm)&((type*)0)->element
#define MM__ALIGNOF(T) OFFSETOF(t, struct { u8 b; T t; })

#define MM__MIN(a, b) ((a) < (b) ? (a) : (b))
#define MM__MAX(a, b) ((a) > (b) ? (a) : (b))
#define MM__ABS(n) ((n) < 0 ? -(n) : (n))
#define MM__SGN(n) ((n) < 0 ? -1 : ((n) == 0 ? 0 : 1))

#define MM__KB(N) ((umm)(N) << 10)
#define MM__MB(N) ((umm)(N) << 20)
#define MM__GB(N) ((umm)(N) << 30)
#define MM__TB(N) ((umm)(N) << 40)

#define MM__ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define MM__internal static
#define MM__global static

// NOTE: This is just a hack to work around a parsing bug in 4coder
#define MM__TYPEDEF_FUNC(return_val, name, ...) typedef return_val (*name)(__VA_ARGS__)

MM__global MM_u32 MM__KeywordTable[MM_KEYWORD_KIND_COUNT] = {0};

#define MM__IS_STRING_KEYWORD(S) ((MM_Interned_String)(S) < MM_KEYWORD_KIND_COUNT)

typedef struct MM__String_Table_Entry
{
    struct MM__String_Table_Entry* next;
    MM_String string;
} MM__String_Table_Entry;
