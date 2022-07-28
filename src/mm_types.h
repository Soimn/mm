typedef enum MM_TYPE_KIND
{
    MM_Type_SoftInt,
    MM_Type_SoftFloat,
    MM_Type_SoftBool,
    MM_Type_SoftString,
    
    MM_Type_Int,
    MM_Type_I8,
    MM_Type_I16,
    MM_Type_I32,
    MM_Type_I64,
    MM_Type_I128,
    
    MM_Type_Uint,
    MM_Type_U8,
    MM_Type_U16,
    MM_Type_U32,
    MM_Type_U64,
    MM_Type_U128,
    
    MM_Type_Float,
    MM_Type_F16,
    MM_Type_F32,
    MM_Type_F64,
    
    MM_Type_Bool,
    MM_Type_B8,
    MM_Type_B16,
    MM_Type_B32,
    MM_Type_B64,
    MM_Type_B128,
    
    MM_Type_String,
    MM_Type_Cstring,
    
    MM_Type_Byte,
    
    MM_Type_Any,
    
    MM_Type_Typeid,
} MM_TYPE_KIND;

typedef enum MM_TYPE_INFO_KIND
{
    MM_TypeInfo_Base,
    MM_TypeInfo_Array,
    MM_TypeInfo_Slice,
    MM_TypeInfo_Pointer,
    MM_TypeInfo_Proc,
    MM_TypeInfo_ProcSet,
    MM_TypeInfo_Struct,
    MM_TypeInfo_Union,
    MM_TypeInfo_Enum,
    MM_TypeInfo_Distinct,
} MM_TYPE_INFO_KIND;

typedef struct MM_Type_Info_Header
{
    MM_TYPE_INFO_KIND kind;
} MM_Type_Info_Header;

#define MM_TYPE_INFO_HEADER() union { MM_Type_Info_Header header; struct MM_Type_Info_Header; }

typedef struct MM_Type_Info_Array
{
    MM_TYPE_INFO_HEADER();
    MM_u64 size;
    // type ref -> elem type
} MM_Type_Info_Array;

typedef struct MM_Type_Info_Slice
{
    MM_TYPE_INFO_HEADER();
    // type ref -> elem type
} MM_Type_Info_Slice;

typedef struct MM_Type_Info_Pointer
{
    MM_TYPE_INFO_HEADER();
    // type ref -> elem type
} MM_Type_Info_Pointer;

typedef struct MM_Type_Info_Parameter
{
    MM_TYPE_INFO_HEADER();
    MM_Identifier name;
    // type ref: type
} MM_Type_Info_Parameter;

typedef struct MM_Type_Info_Return_Value
{
    MM_TYPE_INFO_HEADER();
    MM_Identifier name;
    // type ref: type
} MM_Type_Info_Return_Value;

typedef struct MM_Type_Info_Proc
{
    MM_TYPE_INFO_HEADER();
    MM_Slice(MM_Type_Info_Parameter) params;
    MM_Slice(MM_Type_Info_Return_Value) return_vals;
} MM_Type_Info_Proc;

typedef struct MM_Type_Info_Proc_Set_Member
{
    MM_TYPE_INFO_HEADER();
    // type ref: type
    // symbol ref: name
} MM_Type_Info_Proc_Set_Member;

typedef struct MM_Type_Info_Proc_Set
{
    MM_TYPE_INFO_HEADER();
    MM_Slice(MM_Type_Info_Proc_Set_Member) members;
} MM_Type_Info_Proc_Set;

typedef struct MM_Type_Info_Struct_Member
{
    MM_TYPE_INFO_HEADER();
    MM_u64 offset;
    MM_Identifier name;
    MM_bool is_using;
    // type ref: type
} MM_Type_Info_Struct_Member;

typedef struct MM_Type_Info_Struct
{
    MM_TYPE_INFO_HEADER();
    MM_Slice(MM_Type_Info_Struct_Member) members;
} MM_Type_Info_Struct;

typedef struct MM_Type_Info_Union_Member
{
    MM_TYPE_INFO_HEADER();
    MM_Identifier name;
    MM_bool is_using;
    // type ref: type
} MM_Type_Info_Union_Member;

typedef struct MM_Type_Info_Union
{
    MM_TYPE_INFO_HEADER();
    MM_Slice(MM_Type_Info_Union_Member) members;
} MM_Type_Info_Union;

typedef struct MM_Type_Info_Enum_Member
{
    MM_TYPE_INFO_HEADER();
    MM_Identifier name;
    MM_Soft_Int value;
    // type ref: type
    // type ref: backing type
} MM_Type_Info_Enum_Member;

typedef struct MM_Type_Info_Enum
{
    MM_TYPE_INFO_HEADER();
    MM_Slice(MM_Type_Info_Enum_Member) members;
} MM_Type_Info_Enum;

typedef struct MM_Type_Info_Distinct
{
    MM_TYPE_INFO_HEADER();
    // type ref: backing type
    // symbol ref: name
} MM_Type_Info_Distinct;

typedef struct MM_Type_Info
{
    union
    {
        MM_TYPE_INFO_HEADER();
        MM_Type_Info_Array array_type;
        MM_Type_Info_Slice slice_type;
        MM_Type_Info_Pointer pointer_type;
        MM_Type_Info_Proc proc_type;
        MM_Type_Info_Struct struct_type;
        MM_Type_Info_Union union_type;
        MM_Type_Info_Enum enum_type;
    };
} MM_Type_Info;

// Forward decls / incomplete types