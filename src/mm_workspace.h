MM_Workspace*
MM_Workspace_Open()
{
    MM_Arena* workspace_arena = MM_Arena_Init(MM_KB(16));
    MM_Arena* ast_arena       = MM_Arena_Init(MM_KB(16));
    MM_Workspace* workspace = MM_Arena_Push(workspace_arena, sizeof(MM_Workspace), MM_ALIGNOF(MM_Workspace));
    
    workspace->workspace_arena = workspace_arena;
    workspace->ast_arena       = ast_arena;
    
    MM_Arena* intern_arena = MM_Arena_Init(MM_KB(16));
    MM_Arena* string_arena = MM_Arena_Init(MM_KB(16));
    workspace->intern_table = MM_Arena_Push(intern_arena, sizeof(MM_String_Intern_Table), MM_ALIGNOF(MM_String_Intern_Table));
    *workspace->intern_table = (MM_String_Intern_Table){
        .intern_arena = intern_arena,
        .string_arena = string_arena,
        .temp_marker  = 0,
        .map          = {0},
    };
    
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