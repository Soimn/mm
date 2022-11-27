MM_f64
MM_f64_MapToByteSize(MM_f64 n, MM_umm size)
{
    MM_ASSERT(size == 2 || size == 4 || size == 8);
    
    MM_f64 result = n;
    
    if      (size == 4) result = (MM_f64)(MM_f32)n;
    else if (size == 2) MM_NOT_IMPLEMENTED;
    
    return result;
}