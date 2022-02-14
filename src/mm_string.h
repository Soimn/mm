//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public domain.
// This is a mostly unmodified version of the 32 bit hash

internal u32
MurmurHash3_x86_32(u8* key, umm len, u32 seed)
{
    imm nblocks = len / 4;
    
    u32 h1 = seed;
    
    u32 c1 = 0xcc9e2d51;
    u32 c2 = 0x1b873593;
    
    u32* blocks = (u8*)(key + nblocks*4);
    
    for (imm i = -nblocks; i != 0; i++)
    {
        u32 k1 = blocks[i];
        
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> (32 - 15));
        k1 *= c2;
        
        h1 ^= k1;
        k1 = (k1 << 13) | (k1 >> (32 - 13));
        h1 = h1*5 + 0xe6546b64;
    }
    
    u8* tail = key + nblocks*4;
    
    u32 k1 = 0;
    
    switch(len & 3)
    {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
        k1 *= c1; k1 = (k1 << 15) | (k1 >> (32 - 15)); k1 *= c2; h1 ^= k1;
    };
    
    h1 ^= len;
    
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    
    return h1;
}

internal u64
String_HashOf(String string)
{
    MurmurHash3_x86_32(string.data, string.size, 0xBADB00B5);
}

internal inline bool
String_Match(String s0, String s1)
{
    bool result = false;
    
    if (s0.size == s1.size)
    {
        umm i = 0;
        for (; i < s0.size; ++i)
        {
            if (s0.data[i] != s1.data[i]) break;
        }
        
        result = (i == s0.size);
    }
    
    return result;
}