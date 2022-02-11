typedef struct MM__String_Table_Entry
{
    
} MM__String_Table_Entry;

MM_Interned_String
MM_InternedString_FromString(MM_String_Table* string_table, MM_String string)
{
    
}

MM_String
MM_InternedString_ToString(MM_String_Table* string_table, MM_Interned_String istring)
{
    return ((MM__String_Table_Entry*)istring)->string;
}