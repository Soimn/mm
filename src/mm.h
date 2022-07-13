#ifndef MM_H
#define MM_H

#ifndef MM_DEBUG
#define MM_DEBUG 0
#endif

/// Basic Types
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
typedef signed __int8  MM_i8;
typedef signed __int16 MM_i16;
typedef signed __int32 MM_i32;
typedef signed __int64 MM_i64;
typedef signed __int128 MM_i128;

typedef unsigned __int8  MM_u8;
typedef unsigned __int16 MM_u16;
typedef unsigned __int32 MM_u32;
typedef unsigned __int64 MM_u64;
typedef unsigned __int128 MM_u128;
#else
#error "Platform is not supported"
#endif

// NOTE: umm and imm are used instead of uint and int. They are essentially
//       unsigned/signed 64 bit integers used for computation
//       "where the width does not matter", but it should be "wide enough"
typedef MM_u64 MM_umm;
typedef MM_i64 MM_imm;

typedef MM_u16 MM_f16;
typedef float  MM_f32;
typedef double MM_f64;

typedef MM_u8 MM_bool;

#define MM_true (MM_bool)(1)
#define MM_false (MM_bool)(0)

#define MM_TYPEDEF_FUNC(return_val, name, ...) typedef return_val (*name)(__VA_ARGS__)

typedef struct MM_Slice
{
    void* data;
    MM_u64 size;
} MM_Slice;

#define MM_Slice(T) MM_Slice

#define MM_STATIC_ASSERT(EX) struct __MM_STATIC_ASSERT_##__FILE__##_##__LINE__ { int a : (EX) ? 1 : -1; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif