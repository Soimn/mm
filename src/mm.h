internal Identifier
Identifier_Add(String string)
{
    ASSERT(string.data != 0 && string.size != 0);
    
    // HACK
    // TODO: Implement a hash table with stable pointers
    
    Identifier result = BLANK_IDENTIFIER;
    
    imm free = -1;
    umm i    = 0;
    for (; i < MM.identifier_table_size; ++i)
    {
        if (String_Compare(string, MM.identifier_table[i]))
        {
            result = (Identifier)i;
            break;
        }
        
        else if (MM.identifier_table[i].data == 0) free = (imm)i;
    }
    
    // NOTE: no existing entry was found, add a new entry
    if (i == MM.identifier_table_size)
    {
        // TODO: grow table
        ASSERT(free != -1);
        
        void* memory = Arena_PushSize(&MM.string_arena, string.size, 1);
        Copy(string.data, memory, string.size);
        
        MM.identifier_table[free] = (String){
            .data = memory,
            .size = string.size
        };
        
        result = (Identifier)free;
    }
    
    return result;
}

internal String
Identifier_ToString(Identifier identifier)
{
    return MM.identifier_table[identifier];
}

internal inline bool
Identifier_IsKeyword(Identifier identifier, Enum8(KEYWORD_KIND) keyword)
{
    return (MM.keyword_identifiers[keyword] == identifier);
}

internal inline struct Type_Info*
TypeInfo_FromTypeID(Type_ID id)
{
    return &MM.type_table[id];
}

internal Type_ID
TypeInfo_Add(Type_Info info)
{
    Type_ID id = {0};
    
    NOT_IMPLEMENTED;
    
    return id;
}

internal inline Package*
Package_FromID(Package_ID id)
{
    return &MM.packages[id];
}

internal Package_ID
Package_IDFromPath(String base_dir, String relative_path)
{
    Package_ID package_id = INVALID_PACKAGE_ID;
    
    String package_name = {
        .data = relative_path.data,
        .size = relative_path.size
    };
    
    for (umm i = 0; i < relative_path.size; ++i)
    {
        if (relative_path.data[i] == '/')
        {
            package_name.data = relative_path.data + (i + 1);
            package_name.size = relative_path.size - (i + 1);
        }
    }
    
    Identifier package_ident = Identifier_Add(package_name);
    
    Memory_Arena_Marker mem_marker = Arena_BeginTempMemory(&MM.temp_arena);
    
    String path = {
        .data = Arena_PushSize(&MM.temp_arena, base_dir.size + relative_path.size, 1),
        .size = base_dir.size + relative_path.size
    };
    
    NOT_IMPLEMENTED;
    
    for (umm i = 0; i < MM.package_count; ++i)
    {
        Package* package = Package_FromID((Package_ID)i);
        
        if (package_ident == package->name)
        {
            package_id = (Package_ID)i;
            break;
        }
    }
    
    if (package_id != INVALID_PACKAGE_ID)
    {
        if (!String_Compare(path, Package_FromID(package_id)->path))
        {
            //// ERROR: Duplicate packages
            package_id = INVALID_PACKAGE_ID;
        }
    }
    
    else
    {
        NOT_IMPLEMENTED;
    }
    
    Arena_EndTempMemory(&MM.temp_arena, mem_marker);
    
    return package_id;
}

internal void
MM_Init()
{
    ZeroStruct(&MM);
    
    /// Init memory
    NOT_IMPLEMENTED;
    
    /// Init keyword lookup table
    String KeywordStrings[KEYWORD_COUNT] = {
        [Keyword_Do]             = STRING("do"),
        [Keyword_In]             = STRING("in"),
        [Keyword_Where]          = STRING("where"),
        [Keyword_Proc]           = STRING("proc"),
        [Keyword_Struct]         = STRING("struct"),
        [Keyword_Union]          = STRING("union"),
        [Keyword_Enum]           = STRING("enum"),
        [Keyword_True]           = STRING("true"),
        [Keyword_False]          = STRING("false"),
        [Keyword_As]             = STRING("as"),
        [Keyword_If]             = STRING("if"),
        [Keyword_Else]           = STRING("else"),
        [Keyword_When]           = STRING("when"),
        [Keyword_While]          = STRING("while"),
        [Keyword_For]            = STRING("for"),
        [Keyword_Break]          = STRING("break"),
        [Keyword_Continue]       = STRING("continue"),
        [Keyword_Using]          = STRING("using"),
        [Keyword_Defer]          = STRING("defer"),
        [Keyword_Return]         = STRING("return"),
        [Keyword_Import]         = STRING("import"),
        [Keyword_Foreign]        = STRING("foreign"),
        [Keyword_Package]        = STRING("package"),
    };
    
    for (umm i = Keyword_Invalid + 1; i < KEYWORD_COUNT; ++i)
    {
        MM.keyword_identifiers[i] = Identifier_Add(KeywordStrings[i]);
    }
    
    /// Init type table
    
    // allocate type table
    NOT_IMPLEMENTED;
    
    for (Type_ID i = Type_FirstUntyped;
         i <= Type_LastUntyped;
         ++i)
    {
        MM.type_table[i] = (Type_Info){ .kind = (u8)i };
    }
    
#define TYPED_BASE_TYPE(id, type, ident)            \
MM.type_table[id] = (Type_Info){                \
.kind      = id,                            \
.size      = sizeof(type),                  \
.alignment = ALIGNOF(type),                 \
.name      = Identifier_Add(STRING(ident)), \
}
    
    TYPED_BASE_TYPE(Type_String, String, "string");
    TYPED_BASE_TYPE(Type_Char,   u32,    "char");
    TYPED_BASE_TYPE(Type_Bool,   u8,     "bool");
    TYPED_BASE_TYPE(Type_Int,    i64,    "int");
    TYPED_BASE_TYPE(Type_I8,     i8,     "i8");
    TYPED_BASE_TYPE(Type_I16,    i16,    "i16");
    TYPED_BASE_TYPE(Type_I32,    i32,    "i32");
    TYPED_BASE_TYPE(Type_I64,    i64,    "i64");
    TYPED_BASE_TYPE(Type_Uint,   u64,    "uint");
    TYPED_BASE_TYPE(Type_U8,     u8,     "u8");
    TYPED_BASE_TYPE(Type_U16,    u16,    "u16");
    TYPED_BASE_TYPE(Type_U32,    u32,    "u32");
    TYPED_BASE_TYPE(Type_U64,    u64,    "u64");
    TYPED_BASE_TYPE(Type_Float,  f64,    "f64");
    TYPED_BASE_TYPE(Type_F32,    f32,    "f32");
    TYPED_BASE_TYPE(Type_F64,    f64,    "f64");
}