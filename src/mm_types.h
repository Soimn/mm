typedef enum MM_Type_Kind
{
    MM_Type_Primitive,
    MM_Type_Pointer,
    MM_Type_Array,
    MM_Type_Slice,
    MM_Type_Struct,
    MM_Type_Proc,
} MM_Type_Kind;

typedef enum MM_Typeid
{
    MM_Typeid_None = 0,
    
    MM_Typeid__FirstIntegral,
    MM_Typeid__FirstSignedIntegral = MM_Typeid__FirstIntegral,
    MM_Typeid_SoftInt = MM_Typeid__FirstSignedIntegral,    // i128
    MM_Typeid_Int,                                         // register sized
    MM_Typeid_I8,
    MM_Typeid_I16,
    MM_Typeid_I32,
    MM_Typeid_I64,
    MM_Typeid_I128,
    MM_Typeid__LastSignedIntegral = MM_Typeid_I128,
    
    MM_Typeid__FirstUnsignedIntegral,
    MM_Typeid_Uint = MM_Typeid__FirstUnsignedIntegral, // register sized
    MM_Typeid_U8,
    MM_Typeid_U16,
    MM_Typeid_U32,
    MM_Typeid_U64,
    MM_Typeid_U128,
    MM_Typeid__LastUnsignedIntegral = MM_Typeid_U128,
    MM_Typeid__LastIntegral = MM_Typeid__LastUnsignedIntegral,
    
    MM_Typeid__FirstFloatingPoint,
    MM_Typeid_SoftFloat = MM_Typeid__FirstFloatingPoint,  // f64
    MM_Typeid_Float,                                      // f32
    MM_Typeid_F16,
    MM_Typeid_F32,
    MM_Typeid_F64,
    MM_Typeid__LastFloatingPoint = MM_Typeid_F64,
    
    MM_Typeid__FirstBoolean,
    MM_Typeid_SoftBool = MM_Typeid__FirstBoolean,
    MM_Typeid_Bool,
    MM_Typeid__LastBoolean = MM_Typeid_Bool,
    
    MM_Typeid__FirstString,
    MM_Typeid_SoftString = MM_Typeid__FirstString,
    MM_Typeid_String,
    MM_Typeid__LastString = MM_Typeid_String,
    
    MM_Typeid_Typeid,
} MM_Typeid;

MM_STATIC_ASSERT(sizeof(MM_Typeid) == sizeof(MM_u32));

MM_bool
MM_Typeid_IsSoft(MM_Typeid type)
{
    return (type == MM_Typeid_SoftInt || type == MM_Typeid_SoftFloat || type == MM_Typeid_SoftBool || type == MM_Typeid_SoftString);
}

MM_bool
MM_Typeid_IsIntegral(MM_Typeid type)
{
    return (type >= MM_Typeid__FirstIntegral && type <= MM_Typeid__LastIntegral);
}

MM_bool
MM_Typeid_IsSignedIntegral(MM_Typeid type)
{
    return (type >= MM_Typeid__FirstSignedIntegral && type <= MM_Typeid__LastSignedIntegral);
}

MM_bool
MM_Typeid_IsUnsignedIntegral(MM_Typeid type)
{
    return (type >= MM_Typeid__FirstUnsignedIntegral && type <= MM_Typeid__LastUnsignedIntegral);
}

MM_bool
MM_Typeid_IsFloatingPoint(MM_Typeid type)
{
    return (type >= MM_Typeid__FirstFloatingPoint && type <= MM_Typeid__LastFloatingPoint);
}

MM_bool
MM_Typeid_IsNumeric(MM_Typeid type)
{
    return (MM_Typeid_IsIntegral(type) || MM_Typeid_IsFloatingPoint(type));
}

MM_bool
MM_Typeid_IsBoolean(MM_Typeid type)
{
    return (type >= MM_Typeid__FirstBoolean && type <= MM_Typeid__LastBoolean);
}

MM_bool
MM_Typeid_IsString(MM_Typeid type)
{
    return (type >= MM_Typeid__FirstString && type <= MM_Typeid__LastString);
}

MM_Typeid
MM_Typeid_CommonType(MM_Typeid t0, MM_Typeid t1)
{
    MM_Typeid common_type = MM_Typeid_None;
    
    if (t0 == t1) common_type = t0;
    else if (MM_Typeid_IsSoft(t0) && MM_Typeid_IsSoft(t1))
    {
        if (t0 == MM_Typeid_SoftInt && t1 == MM_Typeid_SoftFloat ||
            t1 == MM_Typeid_SoftInt && t0 == MM_Typeid_SoftFloat)
        {
            common_type = MM_Typeid_SoftFloat;
        }
    }
    else if (MM_Typeid_IsSoft(t0) ^ MM_Typeid_IsSoft(t1))
    {
        MM_Typeid soft = t0;
        MM_Typeid hard = t1;
        if (MM_Typeid_IsSoft(t1))
        {
            soft = t1;
            hard = t0;
        }
        
        if (soft == MM_Typeid_SoftInt)
        {
            if (MM_Typeid_IsNumeric(hard)) common_type = hard;
        }
        else if (soft == MM_Typeid_SoftFloat)
        {
            if (MM_Typeid_IsFloatingPoint(hard)) common_type = hard;
        }
        else if (soft == MM_Typeid_SoftBool)
        {
            if (MM_Typeid_IsBoolean(hard)) common_type = hard;
        }
        else
        {
            MM_ASSERT(soft == MM_Typeid_SoftString);
            if (MM_Typeid_IsString(hard)) common_type = hard;
        }
    }
    
    return common_type;
}


MM_umm
MM_Typeid_Sizeof(MM_Typeid type)
{
    MM_umm size;
    
    switch (type)
    {
        case MM_Typeid_None:       size = 0;     break;
        case MM_Typeid_SoftInt:    size = 16;    break;
        case MM_Typeid_Int:        size = 8;     break;
        case MM_Typeid_I8:         size = 1;     break;
        case MM_Typeid_I16:        size = 2;     break;
        case MM_Typeid_I32:        size = 4;     break;
        case MM_Typeid_I64:        size = 8;     break;
        case MM_Typeid_I128:       size = 16;    break;
        case MM_Typeid_Uint:       size = 8;     break;
        case MM_Typeid_U8:         size = 1;     break;
        case MM_Typeid_U16:        size = 2;     break;
        case MM_Typeid_U32:        size = 4;     break;
        case MM_Typeid_U64:        size = 8;     break;
        case MM_Typeid_U128:       size = 16;    break;
        case MM_Typeid_SoftFloat:  size = 8;     break;
        case MM_Typeid_Float:      size = 4;     break;
        case MM_Typeid_F16:        size = 2;     break;
        case MM_Typeid_F32:        size = 4;     break;
        case MM_Typeid_F64:        size = 8;     break;
        case MM_Typeid_SoftBool:   size = 1;     break;
        case MM_Typeid_Bool:       size = 1;     break;
        case MM_Typeid_SoftString: size = 8 + 8; break;
        case MM_Typeid_String:     size = 8 + 8; break;
        case MM_Typeid_Typeid:     size = 4;     break;
        default:
        {
            MM_NOT_IMPLEMENTED;
        } break;
    }
    
    return size;
}

MM_Typeid
MM_Typeid_ArrayOf(MM_Typeid element_type)
{
    MM_NOT_IMPLEMENTED;
}