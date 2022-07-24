MM_bool
MM_String_Match(MM_String s0, MM_String s1)
{
    if      (s0.size != s1.size) return MM_false;
    else if (s0.data == s1.data) return MM_true;
    else
    {
        MM_bool do_match = MM_true;
        
        for (MM_umm i = 0; i < s0.size && do_match; ++i)
        {
            do_match = (s0.data[i] == s1.data[1]);
        }
        
        return do_match;
    }
}

MM_bool
MM_String_InternMatch(MM_String s0, MM_String s1)
{
    return (s0.data == s1.data);
}

MM_u64
MM_String_Hash(MM_String s)
{
    MM_NOT_IMPLEMENTED;
    return 0;
}

typedef struct MM_String_Intern_Entry
{
    struct MM_String_Intern_Entry* next;
    MM_u64 hash;
    MM_String string;
} MM_String_Intern_Entry;

#ifndef MM_STRING_INTERN_TABLE_MAP_SIZE
#define MM_STRING_INTERN_TABLE_MAP_SIZE 512
#endif

typedef struct MM_String_Intern_Table
{
    MM_Arena* intern_arena;
    MM_Arena* string_arena;
    MM_Arena_Marker temp_marker;
    MM_String_Intern_Entry map[MM_STRING_INTERN_TABLE_MAP_SIZE];
} MM_String_Intern_Table;

MM_String
MM_String_Intern(MM_String_Intern_Table* intern_table, MM_String s)
{
    MM_u64 hash = MM_String_Hash(s);
    
    MM_String_Intern_Entry* map_entry = &intern_table->map[hash % MM_STRING_INTERN_TABLE_MAP_SIZE];
    
    MM_String_Intern_Entry* entry      = map_entry;
    MM_String_Intern_Entry* prev_entry = 0;
    
    while (MM_true)
    {
        if (entry->hash == hash && MM_String_Match(s, entry->string)) break;
        else
        {
            prev_entry = entry;
            entry      = entry->next;
        }
        
        if (entry == 0)
        {
            MM_String_Intern_Entry* new_entry;
            if (prev_entry == 0) new_entry = map_entry;
            else
            {
                new_entry        = MM_Arena_Push(intern_table->intern_arena, sizeof(MM_String_Intern_Entry), MM_ALIGNOF(MM_String_Intern_Entry));
                prev_entry->next = new_entry;
            }
            
            *new_entry = (MM_String_Intern_Entry){
                .next        = 0,
                .hash        = hash,
                .string.data = MM_Arena_Push(intern_table->string_arena, s.size, 1),
                .string.size = s.size,
            };
            
            MM_Copy(new_entry->string.data, s.data, s.size);
            
            entry = new_entry;
            
            break;
        }
    }
    
    return entry->string;
}

MM_Arena*
MM_String_BeginIntern(MM_String_Intern_Table* intern_table)
{
    MM_ASSERT(intern_table->temp_marker == 0);
    intern_table->temp_marker = MM_Arena_GetMarker(intern_table->string_arena);
    
    return intern_table->string_arena;
}

void
MM_String_AbortIntern(MM_String_Intern_Table* intern_table)
{
    MM_ASSERT(intern_table->temp_marker != 0);
    
    MM_Arena_PopToMarker(intern_table->string_arena, intern_table->temp_marker);
    
    intern_table->temp_marker = 0;
}

MM_String
MM_String_EndIntern(MM_String_Intern_Table* intern_table, MM_String s)
{
    MM_ASSERT(intern_table->temp_marker != 0);
    
    MM_u64 hash = MM_String_Hash(s);
    
    MM_String_Intern_Entry* map_entry = &intern_table->map[hash % MM_STRING_INTERN_TABLE_MAP_SIZE];
    
    MM_String_Intern_Entry* entry      = map_entry;
    MM_String_Intern_Entry* prev_entry = 0;
    
    while (MM_true)
    {
        if (entry->hash == hash && MM_String_Match(s, entry->string))
        {
            // NOTE: There is already an entry that matches the new temp entry
            //       Return existing entry and pop string
            MM_Arena_PopToMarker(intern_table->string_arena, intern_table->temp_marker);
            break;
        }
        else
        {
            prev_entry = entry;
            entry      = entry->next;
        }
        
        if (entry == 0)
        {
            // NOTE: There is no entry that matches the new temp entry
            //       Add new entry and keep string
            
            MM_String_Intern_Entry* new_entry;
            if (prev_entry == 0) new_entry = map_entry;
            else
            {
                new_entry        = MM_Arena_Push(intern_table->intern_arena, sizeof(MM_String_Intern_Entry), MM_ALIGNOF(MM_String_Intern_Entry));
                prev_entry->next = new_entry;
            }
            
            *new_entry = (MM_String_Intern_Entry){
                .next        = 0,
                .hash        = hash,
                .string.data = s.data,
                .string.size = s.size,
            };
            
            entry = new_entry;
            
            break;
        }
    }
    
    intern_table->temp_marker = 0;
    
    return entry->string;
}