MM_Workspace*
MM_Workspace_Open(MM_Workspace_Options options)
{
    MM_Arena* workspace_arena = MM_Arena_Init(MM_KB(16), MM_GB(32));
    MM_Arena* ast_arena       = MM_Arena_Init(MM_KB(16), MM_GB(32));
    MM_Workspace* workspace = MM_Arena_Push(workspace_arena, sizeof(MM_Workspace), MM_ALIGNOF(MM_Workspace));
    
    workspace->workspace_arena = workspace_arena;
    workspace->ast_arena       = ast_arena;
    
    workspace->intern_arena       = MM_Arena_Init(MM_KB(16), MM_GB(32));
    workspace->string_arena       = MM_Arena_Init(MM_KB(16), MM_GB(32));
    workspace->file_table_arena   = MM_Arena_Init(MM_KB(16), MM_GB(32));
    workspace->file_content_arena = MM_Arena_Init(MM_KB(16), MM_GB(32));
    
    workspace->intern_table = MM_String_InitInternTable(workspace->intern_arena, workspace->string_arena);
    workspace->file_table   = MM_FileTable_Init(workspace->file_table_arena, options.path_labels, options.path_label_count);
    
    return workspace;
}

void
MM_Workspace_Close(MM_Workspace** workspace)
{
    MM_Arena_Free(&(*workspace)->intern_table->string_arena);
    MM_Arena_Free(&(*workspace)->intern_table->intern_arena);
    MM_Arena_Free(&(*workspace)->ast_arena);
    MM_Arena_Free(&(*workspace)->workspace_arena);
    *workspace = 0;
}

MM_Error
MM_Workspace_AddFile(MM_Workspace* workspace, MM_String working_dir, MM_String path, MM_File_ID* result)
{
    MM_Error error = MM_ErrorNone;
    
    MM_File_Table* file_table = workspace->file_table;
    
    MM_String label         = { .data = 0 };
    MM_String dir_path      = working_dir;
    MM_String relative_path = path;
    
    for (MM_umm i = 0; i < path.size; ++i)
    {
        if (path.data[i] == ':')
        {
            label         = (MM_String){ .data = path.data, .size = i };
            relative_path = (MM_String){ .data = path.data + (i + 1), .size = path.size - (i + 1)};
        }
    }
    
    if (label.data != 0)
    {
        MM_umm i = 0;
        for (; i < file_table->path_label_count; ++i)
        {
            if (MM_String_Match(label, file_table->path_labels[i].name))
            {
                dir_path = file_table->path_labels[i].path;
                break;
            }
        }
        
        if (i == file_table->path_label_count)
        {
            //// ERROR
            return (MM_Error){ .code = MM_Error_UnknownLabel }; // TODO: Better error info
        }
    }
    
    MM_Arena_Marker marker = MM_Arena_GetMarker(workspace->workspace_arena);
    
    MM_String resolved_path = { .size = dir_path.size + relative_path.size };
    resolved_path.data = MM_Arena_PushCopy(workspace->workspace_arena, dir_path.data, dir_path.size, MM_ALIGNOF(MM_u8));
    MM_Arena_PushCopy(workspace->workspace_arena, relative_path.data, relative_path.size, MM_ALIGNOF(MM_u8));
    
    MM_File_ID file_id = MM_FileTable_FileIDFromPath(file_table, resolved_path);
    
    if (file_id != MM_FILE_ID_NIL) *file_id = file_id;
    else
    {
        MM_NOT_IMPLEMENTED;
    }
    
    MM_Arena_PopToMarker(workspace->workspace_arena, marker);
    
    return MM_ErrorNone;
}