#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#include <shellapi.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near

#include "mm_platform.h"

Memory_Arena Win32_Arena;

internal void
System_InitArena(Memory_Arena* arena)
{
    umm reserved_size  = GB(10);
    umm committed_size = KB(16);
    
    void* memory = VirtualAlloc(0, reserved_size, MEM_RESERVE, PAGE_NOACCESS);
    
    if (!VirtualAlloc(memory, committed_size, MEM_COMMIT, PAGE_READWRITE))
    {
        //// ERROR: Out of memory
        ExitProcess(0);
    }
    
    else
    {
        *arena = (Memory_Arena){
            .base_address = (u64)memory,
            .offset       = 0,
            .size         = committed_size,
        };
    }
}

internal void
System_GrowArena(Memory_Arena* arena, umm overflow)
{
    umm mem_to_commit = RoundUpToAlignment(overflow, KB(4)) + 2*KB(4);
    
    arena->size += mem_to_commit;
    
    if (!VirtualAlloc((void*)arena->base_address, arena->size, MEM_COMMIT, PAGE_READWRITE))
    {
        //// ERROR: Out of memory
        ExitProcess(0);
    }
}

internal bool
System_ReadFile(Memory_Arena* arena, u8* path, String* contents)
{
    bool encountered_errors = false;
    
    HANDLE file = CreateFileA((LPCSTR)path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file == INVALID_HANDLE_VALUE) encountered_errors = true;
    else
    {
        DWORD high_size;
        DWORD size = GetFileSize(file, &high_size);
        
        if (high_size != 0) encountered_errors = true;
        else
        {
            contents->size = size;
            contents->data = Arena_PushSize(arena, contents->size, ALIGNOF(u8));
            
            DWORD bytes_read;
            if (!ReadFile(file, contents->data, (DWORD)contents->size, &bytes_read, 0) || bytes_read != contents->size)
            {
                encountered_errors = true;
            }
        }
        
        CloseHandle(file);
    }
    
    return !encountered_errors;
}

internal void
Print(char* format, ...)
{
    va_list args;
    va_start(args, format);
    NOT_IMPLEMENTED;
    va_end(args);
}

internal void
PrintIndent(umm indent)
{
    for (umm i = 0; i < indent; ++i) Print(" ");
}

internal void
PrintASTNode(AST_Node* node, umm indent)
{
    PrintIndent(indent);
    
    if (node->kind == AST_Scope)
    {
        Print("(Scope %S %b)", Identifier_Of(node->scope_statement.label), node->scope_statement.is_do);
        for (AST_Node* child = node->scope_statement.body; child != 0; child = child->next)
        {
            PrintASTNode(child, indent + 1);
        }
    }
    
    else if (node->kind == AST_Identifier)
    {
        Print("(I %S)", Identifier_Of(node->identifier));
    }
    
    else if (node->kind == AST_String)
    {
        Print("(\"%S\")", StringLiteral_Of(node->identifier));
    }
    
    else if (node->kind == AST_Char)
    {
        Print("(U+%u)", Character_ToCodepoint(node->character));
    }
    
    else if (node->kind == AST_Number)
    {
        if (node->number.is_float) Print("(%f)", node->number.floating);
        else                       Print("(%U)", node->number.integer);
    }
    
    else if (node->kind == AST_Boolean)
    {
        Print("(%b)", node->boolean);
    }
    
    else if (node->kind == AST_StructLiteral)
    {
        Print("(StructLit ");
        PrintASTNode(node->struct_literal.type, 0);
        Print(" (");
        
        for (AST_Node* child = node->struct_literal.params; child != 0; child = child->next)
        {
            PrintASTNode(child, 0);
            if (child->next != 0) Print(", ");
        }
        
        Print(")");
    }
    
    else if (node->kind == AST_ArrayLiteral)
    {
        Print("(ArrayLit ");
        PrintASTNode(node->array_literal.type, 0);
        Print(" (");
        
        for (AST_Node* child = node->array_literal.params; child != 0; child = child->next)
        {
            PrintASTNode(child, 0);
            if (child->next != 0) Print(", ");
        }
        
        Print(")");
    }
    
    /*
    AST_Proc,
    AST_ProcType,
    AST_Struct,
    AST_Union,
    AST_Enum,
    AST_Directive,
    AST_Compound,
    
    // precedence 1: 20 - 39
    AST_FirstTypeLevel = 20,
    AST_PointerType = AST_FirstTypeLevel,
    AST_SliceType,
    AST_ArrayType,
    AST_DynamicArrayType,
    AST_LastTypeLevel = AST_DynamicArrayType,
    
    // precedence 2: 40 - 59
    AST_FirstPostfixLevel = 40,
    AST_Subscript = AST_FirstPostfixLevel,
    AST_Slice,
    AST_Call,
    AST_ElementOf,
    AST_UfcsOf,
    AST_LastPostfixLevel = AST_UfcsOf,
    
    // precedence 3: 60 - 79
    AST_FirstPrefixLevel = 60,
    AST_Negation = AST_FirstPrefixLevel,
    AST_Complement,
    AST_Not,
    AST_Reference,
    AST_Dereference,
    AST_Spread,
    AST_LastPrefixLevel = AST_Spread,
    
    // precedence 4: 80 - 99
    AST_FirstRangeLevel = 80,
    AST_ClosedRange = AST_FirstRangeLevel,
    AST_HalfOpenRange,
    AST_LastRangeLevel = AST_HalfOpenRange,
    
    // precedence 5: 100 - 119
    AST_FirstMulLevel = 100,
    AST_Mul = AST_FirstMulLevel,
    AST_Div,
    AST_Rem,
    AST_BitwiseAnd,
    AST_ArithmeticRightShift,
    AST_RightShift,
    AST_LeftShift,
    AST_LastMulLevel = AST_LeftShift,
    
    // precedence 6: 120 - 139
    AST_FirstAddLevel = 120,
    AST_Add = AST_FirstAddLevel,
    AST_Sub,
    AST_BitwiseOr,
    AST_BitwiseXor,
    AST_LastAddLevel = AST_BitwiseOr,
    
    // precedence 7: 140 - 159
    AST_FirstComparative = 140,
    AST_IsEqual = AST_FirstComparative,
    AST_IsNotEqual,
    AST_IsStrictlyLess,
    AST_IsStrictlyGreater,
    AST_IsLess,
    AST_IsGreater,
    AST_LastComparative = AST_IsGreater,
    
    // precedence 8: 160 - 179
    AST_And = 160,
    
    // precedence 9: 180 - 199
    AST_Or = 180,
    
    // precedence 10: 200 - 219
    AST_Conditional,
    AST_LastExpression = AST_Conditional,
    
    AST_If,
    AST_When,
    AST_While,
    AST_Break,
    AST_Continue,
    AST_Defer,
    AST_Return,
    AST_Using,
    AST_Assignment,
    
    AST_VariableDecl,
    AST_ConstantDecl,
    AST_IncludeDecl,
*/
}

internal void
PrintAST()
{
    for (AST_Node* node = MM.ast; node != 0; node = node->next)
    {
        PrintASTNode(node, 0);
    }
}

typedef struct Command_Line_Arguments
{
    Path_Label* path_labels;
    u32 path_label_count;
    String main_file;
} Command_Line_Arguments;

internal bool
ParseCommandLineArguments(u8* command_line, Command_Line_Arguments* args)
{
    bool encountered_errors = false;
    
    u8* scan       = command_line;
    u8* last_slash = command_line;
    while (*scan != 0 && *scan != ' ')
    {
        if (*scan == '\\') last_slash = scan;
        ++scan;
    }
    
    /// Path labels
    {
        args->path_labels      = Arena_PushSize(&Win32_Arena, sizeof(Path_Label), ALIGNOF(Path_Label));
        args->path_label_count = 1;
        
        args->path_labels[0].label = STRING("core");
        
        String exe_dir = {
            .data = command_line,
            .size = (last_slash - scan) + 1
        };
        String addend = STRING("core/");
        args->path_labels[0].path.data = Arena_PushSize(&Win32_Arena, exe_dir.size + addend.size, ALIGNOF(u8));
        args->path_labels[0].path.size = exe_dir.size + addend.size;
        
        Copy(exe_dir.data, args->path_labels[0].path.data, exe_dir.size);
        Copy(addend.data, args->path_labels[0].path.data + exe_dir.size, addend.size);
    }
    
    while (*scan == ' ') ++scan;
    
    /// main file
    String main_file = {
        .data = scan,
        .size = 0,
    };
    
    last_slash = scan;
    while (*scan != 0 && *scan != ' ')
    {
        if (*scan == '\\') last_slash = scan;
        ++scan;
    }
    
    main_file.size = scan - main_file.data;
    
    if (main_file.size == 0)
    {
        //// ERROR: missing main file
        encountered_errors = true;
    }
    
    else
    {
        if (String_Compare(main_file, STRING("C:\\"))) args->main_file = main_file;
        else
        {
            umm required_size = GetCurrentDirectory(0, 0);
            
            args->main_file = (String){
                .data = Arena_PushSize(&Win32_Arena, required_size + main_file.size - 1, ALIGNOF(u8)),
                .size = required_size - 1,
            };
            
            GetCurrentDirectory((DWORD)args->main_file.size, (LPSTR)args->main_file.data);
            
            Copy(main_file.data, args->main_file.data + required_size - 1, main_file.size);
        }
    }
    
    return !encountered_errors;
}

void __stdcall
WinMainCRTStartup()
{
    System_InitArena(&Win32_Arena);
    
    Command_Line_Arguments args = {0};
    if (ParseCommandLineArguments((u8*)GetCommandLineA(), &args))
    {
        if (MM_Init(args.main_file, args.path_labels, args.path_label_count))
        {
            NOT_IMPLEMENTED;
        }
    }
    
    ExitProcess(0);
}