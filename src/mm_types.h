typedef MM_u32 MM_Type_ID;

typedef struct MM_any
{
    struct MM_Type_Info* type_info;
    void* data;
} MM_any;

typedef MM_i128 MM_Soft_Int;
typedef MM_f64 MM_Soft_Float;
typedef MM_u8 MM_Soft_Bool;

// NOTE: type == MM_Type_None is used to indicate that no constant value is stored in this.
//       This is just to avoid a pointer indirection or an additional member in the struct,
//       and since there are no values without a type, it doesn't collide with anything.
typedef struct MM_Const_Val
{
    MM_Type_ID type;
    
    union
    {
        MM_Soft_Int soft_int;
        MM_Soft_Float soft_float;
        MM_Soft_Bool soft_bool;
        
        MM_i8 i8;
        MM_i16 i16;
        MM_i32 i32;
        MM_i64 i64;
        MM_i128 i128;
        
        MM_u8 u8;
        MM_u16 u16;
        MM_u32 u32;
        MM_u64 u64;
        MM_u128 u128;
        
        MM_f16 f16;
        MM_f32 f32;
        MM_f64 f64;
        
        MM_any any;
        
        MM_Type_ID type_id;
        
        // TODO: How should the memory for these be handled? (and how should relocation to the data and bss segment work?)
        MM_String string;
        MM_Slice slice;
        void* pointer;
    };
} MM_Const_Val;

typedef enum MM_Base_Type
{
    MM_Type_None = 0,
    
    MM_Type_FirstSoft,
    MM_Type_SoftInt = MM_Type_FirstSoft,
    MM_Type_SoftFloat,
    MM_Type_SoftBool,
    MM_Type_LastSoft = MM_Type_SoftBool,
    
    // NOTE: 2's complement signed integers
    MM_Type_FirstSignedInt,
    MM_Type_Int = MM_Type_FirstSignedInt,  // NOTE: register sized
    MM_Type_I8,
    MM_Type_I16,
    MM_Type_I32,
    MM_Type_I64,
    MM_Type_I128,
    MM_Type_LastSignedInt = MM_Type_I128,
    
    MM_Type_FirstUnsignedInt,
    MM_Type_Uint = MM_Type_FirstUnsignedInt,  // NOTE: register sized
    MM_Type_U8,
    MM_Type_U16,
    MM_Type_U32,
    MM_Type_U64,
    MM_Type_U128,
    MM_Type_LastUnsignedInt = MM_Type_U128,
    
    // NOTE: IEEE 754 floating point
    MM_Type_FirstFloat,
    MM_Type_Float = MM_Type_FirstFloat, // NOTE: 32-bit
    MM_Type_F16,
    MM_Type_F32,
    MM_Type_F64,
    MM_Type_LastFloat = MM_Type_F64,
    
    MM_Type_FirstBool,
    MM_Type_Bool = MM_Type_FirstBool, // NOTE: 8-bit
    MM_Type_B8,
    MM_Type_B16,
    MM_Type_B32,
    MM_Type_B64,
    MM_Type_B128,
    MM_Type_LastBool = MM_Type_B128,
    
    // NOTE: A "word" is an amount of bits with no further information.
    //       A w8 is therefore a regular byte, and could mean anything or nothing.
    //       byte is an alias for w8, this is just for the sake of readability
    MM_Type_FirstWord,
    MM_Type_Word = MM_Type_FirstWord, // NOTE: register sized
    MM_Type_Byte,                     // NOTE: 8-bit
    MM_Type_W8,
    MM_Type_W16,
    MM_Type_W32,
    MM_Type_W64,
    MM_Type_W128,
    MM_Type_LastWord = MM_Type_W128,
    
    MM_Type_String,
    
    MM_Type_Any,
    
    MM_Type_Typeid,
    
    MM_BASE_TYPE_COUNT
} MM_Base_Type;

typedef enum MM_TYPE_INFO_KIND
{
    MM_TypeInfo_BaseType,
    
    MM_TypeInfo_Array,
    MM_TypeInfo_Slice,
    MM_TypeInfo_Pointer,
    
    MM_TypeInfo_Struct,
    MM_TypeInfo_Union,
    MM_TypeInfo_Enum,
    MM_TypeInfo_Proc,
    MM_TypeInfo_ProcSet,
    
    MM_TypeInfo_Distinct,
} MM_TYPE_INFO_KIND;

typedef struct MM_Type_Info_Header
{
    MM_TYPE_INFO_KIND kind;
    MM_Type_ID type_id;
} MM_Type_Info_Header;

#define MM_TYPE_INFO_HEADER() union { struct MM_Type_Info_Header; struct MM_Type_Info_Header header; }

typedef struct MM_Type_Info_Base_Type
{
    MM_TYPE_INFO_HEADER();
} MM_Type_Info_Base_Type;

typedef struct MM_Type_Info_Array
{
    MM_TYPE_INFO_HEADER();
    struct MM_Type_Info* elem_type;
    MM_u32 size;
} MM_Type_Info_Array;

typedef struct MM_Type_Info_Slice
{
    MM_TYPE_INFO_HEADER();
    struct MM_Type_Info* elem_type;
} MM_Type_Info_Slice;

typedef struct MM_Type_Info_Pointer
{
    MM_TYPE_INFO_HEADER();
    struct MM_Type_Info* elem_type;
} MM_Type_Info_Pointer;

typedef struct MM_Type_Info_Struct_Member
{
    MM_TYPE_INFO_HEADER();
    MM_String name;
    struct MM_Type_Info* type;
} MM_Type_Info_Struct_Member;

typedef struct MM_Type_Info_Struct
{
    MM_TYPE_INFO_HEADER();
    MM_bool is_complete;
    MM_Slice(MM_Type_Info_Struct_Member) members;
} MM_Type_Info_Struct;

typedef struct MM_Type_Info_Union_Member
{
    MM_TYPE_INFO_HEADER();
    MM_String name;
    struct MM_Type_Info* type;
} MM_Type_Info_Union_Member;

typedef struct MM_Type_Info_Union
{
    MM_TYPE_INFO_HEADER();
    MM_bool is_complete;
    MM_Slice(MM_Type_Info_Union_Member) members;
} MM_Type_Info_Union;

typedef struct MM_Type_Info_Enum_Member
{
    MM_TYPE_INFO_HEADER();
    MM_String name;
    struct MM_Type_Info* backing_type;
    MM_i128 value;
} MM_Type_Info_Enum_Member;

typedef struct MM_Type_Info_Enum
{
    MM_TYPE_INFO_HEADER();
    MM_bool is_complete;
    MM_Slice(MM_Type_Info_Enum_Member) members;
} MM_Type_Info_Enum;

typedef struct MM_Type_Info_Parameter
{
    MM_TYPE_INFO_HEADER();
    MM_String name;
    struct MM_Type_Info* type;
    
    MM_Const_Val value;
} MM_Type_Info_Parameter;

typedef struct MM_Type_Info_Return_Value
{
    MM_TYPE_INFO_HEADER();
    MM_String name;
    struct MM_Type_Info* type;
    
    MM_Const_Val value;
} MM_Type_Info_Return_Value;

typedef struct MM_Type_Info_Proc
{
    MM_TYPE_INFO_HEADER();
    MM_Slice(MM_Type_Info_Parameter) params;
    MM_Slice(MM_Type_Info_Return_Value) return_values;
} MM_Type_Info_Proc;

typedef struct MM_Type_Info_Proc_Set_Entry
{
    MM_TYPE_INFO_HEADER();
    struct MM_Type_Info* type;
    MM_bool no_shadow;
    
    struct MM_Symbol* symbol;
} MM_Type_Info_Proc_Set_Entry;

typedef struct MM_Type_Info_Proc_Set
{
    MM_TYPE_INFO_HEADER();
    MM_bool is_complete;
    MM_Slice(MM_Type_Info_Proc_Set_Entry) entries;
    
    struct MM_Symbol* symbol;
} MM_Type_Info_Proc_Set;

typedef struct MM_Type_Info_Distinct
{
    
    struct MM_Type_Info* aliased_type;
    
    struct MM_Symbol* symbol;
} MM_Type_Info_Distinct;

typedef struct MM_Type_Info
{
    union
    {
        MM_TYPE_INFO_HEADER();
        MM_Type_Info_Base_Type base_type_info;
        MM_Type_Info_Array array_info;
        MM_Type_Info_Slice slice_info;
        MM_Type_Info_Pointer pointer_info;
        MM_Type_Info_Struct_Member struct_member_info;
        MM_Type_Info_Struct struct_info;
        MM_Type_Info_Union_Member union_member_info;
        MM_Type_Info_Union union_info;
        MM_Type_Info_Enum_Member enum_member_info;
        MM_Type_Info_Enum enum_info;
        MM_Type_Info_Parameter parameter_info;
        MM_Type_Info_Return_Value return_value_info;
        MM_Type_Info_Proc proc_info;
        MM_Type_Info_Proc_Set_Entry proc_set_entry_info;
        MM_Type_Info_Proc_Set proc_set_info;
        MM_Type_Info_Distinct distinct_info;
    };
} MM_Type_Info;