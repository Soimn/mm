#ifndef MM_FILE_TABLE_MAP_SIZE
#define MM_FILE_TABLE_MAP_SIZE 32
#endif

typedef struct MM_File
{
    struct MM_File* next;
    MM_u64 hash;
    MM_String name;
    MM_String path;
    MM_String contents;
} MM_File;

typedef struct MM_File_Table
{
    struct MM_Arena* table_arena;
    MM_Path_Label* path_labels;
    MM_u32 path_label_count;
    MM_File* map[MM_FILE_TABLE_MAP_SIZE];
} MM_File_Table;

MM_File_Table*
MM_FileTable_Init(MM_Arena* table_arena, MM_Path_Label* path_labels, MM_umm path_label_count)
{
    MM_File_Table* file_table = MM_Arena_PushZero(table_arena, sizeof(MM_File_Table), MM_ALIGNOF(MM_File_Table));
    *file_table = (MM_File_Table){
        .table_arena = table_arena,
    };
    
    file_table->path_label_count = path_label_count;
    file_table->path_labels      = MM_Arena_PushCopy(table_arena, path_labels, sizeof(MM_Path_Label)*path_label_count,
                                                     MM_ALIGNOF(MM_Path_Label));
    
    return file_table;
}

MM_String
MM_FilenameFromPath(MM_String path)
{
    MM_imm last_slash = -1;
    for (MM_umm i = 0; i < path.size; ++i)
    {
        if (path.data[i] == '/') last_slash = (MM_imm)i;
    }
    
    return (MM_String){
        .data = path.data + (last_slash + 1),
        .size = path.size - (last_slash + 1),
    };
}

MM_File*
MM_FileTable_FileFromPath(MM_File_Table* table, MM_String path)
{
    MM_File* result = 0;
    
    MM_String name = MM_FilenameFromPath(path);
    
    MM_u64 hash   = MM_String_Hash(name);
    MM_File* scan = table->map[hash % MM_FILE_TABLE_MAP_SIZE];
    
    for (; scan != 0; scan = scan->next)
    {
        if (scan->hash == hash && MM_String_Match(scan->name, name) && MM_SYSTEM_PATHS_POINT_TO_SAME_FILE(scan->path, path))
        {
            result = scan;
        }
    }
    
    return result;
}

MM_File_ID
MM_FileTable_FileIDOfFile(MM_File_Table* table, MM_File* file)
{
    MM_File_ID result = MM_FILE_ID_NIL;
    
    if (file != 0)
    {
        MM_imm diff = (MM_u8*)file - (MM_u8*)table->table_arena;
        MM_ASSERT(diff > 0 && (MM_umm)diff > MM_ROUND_UP(sizeof(MM_Arena), sizeof(MM_File)) / sizeof(MM_File));
        MM_ASSERT((MM_umm)diff + sizeof(file) + file->path.size < MM_Arena_UsedBytes(table->table_arena));
        
        result = diff / sizeof(MM_File);
    }
    
    return result;
}

MM_File_ID
MM_FileTable_FileIDFromPath(MM_File_Table* table, MM_String path)
{
    MM_File* file = MM_FileTable_FileFromPath(table, path);
    return (file != 0 ? MM_FileTable_FileIDOfFile(table, file) : MM_FILE_ID_NIL);
}

MM_File_ID
MM_FileTable_AddFile(MM_File_Table* table, MM_String path, MM_String contents)
{
    MM_String name = MM_FilenameFromPath(path);
    MM_u64 hash    = MM_String_Hash(name);
    
    MM_File** slot = &table->map[hash % MM_FILE_TABLE_MAP_SIZE];
    MM_File* scan  = *slot;
    
    for (; scan != 0; scan = scan->next, slot = &(*slot)->next)
    {
        if (scan->hash == hash && MM_String_Match(scan->name, name) && MM_SYSTEM_PATHS_POINT_TO_SAME_FILE(scan->path, path))
        {
            break;
        }
    }
    
    MM_ASSERT(scan == 0 && slot != 0 && *slot == 0);
    
    *slot = MM_Arena_Push(table->table_arena, sizeof(MM_File), MM_ALIGNOF(MM_File));
    
    MM_String new_path = {
        .data = MM_Arena_PushCopy(table->table_arena, path.data, path.size, MM_ALIGNOF(MM_u8)),
        .size = path.size,
    };
    
    **slot = (MM_File){
        .next     = 0,
        .hash     = hash,
        .name     = MM_FilenameFromPath(new_path),
        .path     = new_path,
        .contents = contents,
    };
    
    return MM_FileTable_FileIDOfFile(table, *slot);
}