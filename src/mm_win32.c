#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near

#include "mm_platform.h"

Memory_Arena Win32_Arena;
Memory_Arena Win32_Temp_Arena;

internal void
Print(char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    Memory_Arena_Marker marker = Arena_BeginTempMemory(&Win32_Temp_Arena);
    
    umm buffer_size = String_FormatArgList((Buffer){0}, format, args);
    
    Buffer buffer = {
        .data = Arena_PushSize(&Win32_Temp_Arena, buffer_size, ALIGNOF(u8)),
        .size = buffer_size,
    };
    
    String_FormatArgList(buffer, format, args);
    
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer.data, (u32)buffer_size, 0, 0);
    
    Arena_EndTempMemory(&Win32_Temp_Arena, marker);
    
    va_end(args);
}

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
System_ReadFile(Memory_Arena* arena, Path path, String* contents)
{
    bool encountered_errors = false;
    
    HANDLE file = CreateFileA((LPCSTR)path.data, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
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

internal bool
System_ResolvePath(struct Memory_Arena* arena, Path wd, Path path, Path* resolved_dir, Path* resolved_path)
{
    bool encountered_errors = false;
    
    if (!SetCurrentDirectory((LPCSTR)wd.data)) encountered_errors = true;
    else
    {
        u32 required_size = GetFullPathNameA((LPCSTR)path.data, 0, 0, 0);
        
        Memory_Arena_Marker marker = Arena_BeginTempMemory(&Win32_Temp_Arena);
        
        u8* memory = Arena_PushSize(&Win32_Temp_Arena, required_size, ALIGNOF(u8));
        
        u8* filename;
        if (!GetFullPathNameA((LPCSTR)path.data, required_size, (LPSTR)memory, (LPSTR*)&filename)) encountered_errors = true;
        else
        {
            String dir = {
                .data = memory,
                .size = filename - (u8*)memory
            };
            
            String full_path = String_FromCString(memory);
            
            HANDLE handle = CreateFile((LPCSTR)memory, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
            CloseHandle(handle);
            
            if (handle == INVALID_HANDLE_VALUE) encountered_errors = true;
            else
            {
                if (resolved_dir)
                {
                    *resolved_dir = (Path){
                        .data = Arena_PushSize(arena, dir.size + 1, ALIGNOF(u8)),
                        .size = dir.size
                    };
                    
                    Copy(dir.data, resolved_dir->data, dir.size);
                    resolved_dir->data[resolved_dir->size] = 0;
                }
                
                
                if (resolved_path)
                {
                    *resolved_path = (Path){
                        .data = Arena_PushSize(arena, full_path.size + 1, ALIGNOF(u8)),
                        .size = full_path.size
                    };
                    
                    Copy(full_path.data, resolved_path->data, full_path.size);
                    resolved_path->data[resolved_path->size] = 0;
                }
            }
        }
        
        Arena_EndTempMemory(&Win32_Temp_Arena, marker);
    }
    
    return !encountered_errors;
}

internal bool
System_PathsRefSameFile(Path p0, Path p1)
{
    bool result = false;
    
    HANDLE h0 = CreateFileA((LPCSTR)p0.data, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    HANDLE h1 = CreateFileA((LPCSTR)p1.data, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    
    FILE_ID_INFO i0;
    FILE_ID_INFO i1;
    
    if (h0 == INVALID_HANDLE_VALUE                                     ||
        h1 == INVALID_HANDLE_VALUE                                     ||
        !GetFileInformationByHandleEx(h0, FileIdInfo, &i0, sizeof(i0)) ||
        !GetFileInformationByHandleEx(h1, FileIdInfo, &i1, sizeof(i1)))
    {
        // TODO: What should be done in this case?
    }
    
    else
    {
        result = StructCompare(&i0, &i1);
    }
    
    CloseHandle(h0);
    CloseHandle(h1);
    
    return result;
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
    
    if (node == 0)
    {
        Print("()");
    }
    
    else if (node->kind == AST_Scope)
    {
        Print("(Scope %S %b)\n", Identifier_Of(node->scope_statement.label), node->scope_statement.is_do);
        for (AST_Node* child = node->scope_statement.body; child != 0; child = child->next)
        {
            PrintASTNode(child, indent + 1);
            Print("\n");
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
    
    else if (node->kind == AST_Proc || node->kind == AST_ProcType)
    {
        Print("(%s (", (node->kind == AST_Proc ? "Proc" : "ProcType"));
        
        for (AST_Node* child = node->proc_literal.return_values; child != 0; child = child->next)
        {
            PrintASTNode(child, 0);
            if (child->next != 0) Print(", ");
        }
        
        Print(") (");
        
        for (AST_Node* child = node->proc_literal.params; child != 0; child = child->next)
        {
            PrintASTNode(child, 0);
            if (child->next != 0) Print(", ");
        }
        
        Print("))");
        
        if (node->kind == AST_Proc)
        {
            Print("\n");
            PrintASTNode(node->proc_literal.body, indent);
        }
    }
    
    else if (node->kind == AST_Struct || node->kind == AST_Union)
    {
        Print("(%s (", (node->kind == AST_Struct ? "Struct" : "Union"));
        
        for (AST_Node* child = node->struct_type.members; child != 0; child = child->next)
        {
            PrintASTNode(child, 0);
            if (child->next != 0) Print(", ");
        }
        
        Print(") ");
    }
    
    else if (node->kind == AST_Enum)
    {
        Print("(Enum (");
        
        for (AST_Node* child = node->enum_type.members; child != 0; child = child->next)
        {
            PrintASTNode(child, 0);
            if (child->next != 0) Print(", ");
        }
        
        Print(") ");
    }
    
    else if (node->kind == AST_Directive)
    {
        Print("(Directive %S (", Identifier_Of(node->directive.name));
        
        for (AST_Node* child = node->directive.params; child != 0; child = child->next)
        {
            PrintASTNode(child, 0);
            if (child->next != 0) Print(", ");
        }
        
        Print("))");
    }
    
    else if (node->kind == AST_Compound)
    {
        Print("(");
        PrintASTNode(node->compound_expr, 0);
        Print(")");
    }
    
    else if (PRECEDENCE_FROM_KIND(node->kind) >= 4 && PRECEDENCE_FROM_KIND(node->kind) <= 9 ||
             node->kind == AST_ElementOf || node->kind == AST_UfcsOf)
    {
        Print("(");
        
        switch (node->kind)
        {
            case AST_ClosedRange:          Print("..<"); break;
            case AST_HalfOpenRange:        Print("..<"); break;
            case AST_Mul:                  Print("*");   break;
            case AST_Div:                  Print("/");   break;
            case AST_Rem:                  Print("%");   break;
            case AST_BitwiseAnd:           Print("&");   break;
            case AST_ArithmeticRightShift: Print(">>>"); break;
            case AST_RightShift:           Print(">>");  break;
            case AST_LeftShift:            Print("<<");  break;
            case AST_Add:                  Print("+");   break;
            case AST_Sub:                  Print("-");   break;
            case AST_BitwiseOr:            Print("|");   break;
            case AST_BitwiseXor:           Print("^");   break;
            case AST_IsEqual:              Print("==");  break;
            case AST_IsNotEqual:           Print("!=");  break;
            case AST_IsStrictlyLess:       Print("<");   break;
            case AST_IsStrictlyGreater:    Print(">");   break;
            case AST_IsLess:               Print("<=");  break;
            case AST_IsGreater:            Print(">=");  break;
            case AST_And:                  Print("&&");  break;
            case AST_Or:                   Print("||");  break;
            case AST_ElementOf:            Print(".");   break;
            case AST_UfcsOf:               Print("->");  break;
            INVALID_DEFAULT_CASE;
        }
        
        Print(" ");
        PrintASTNode(node->binary_expr.left, 0);
        Print(" ");
        PrintASTNode(node->binary_expr.right, 0);
        Print(")");
    }
    
    else if (PRECEDENCE_FROM_KIND(node->kind) == 1 || PRECEDENCE_FROM_KIND(node->kind) == 3)
    {
        Print("(");
        
        switch (node->kind)
        {
            case AST_PointerType:      Print("PtrType");      break;
            case AST_SliceType:        Print("SliceType");    break;
            case AST_DynamicArrayType: Print("DynArrayType"); break;
            case AST_Negation:         Print("-");            break;
            case AST_Complement:       Print("~");            break;
            case AST_Not:              Print("!");            break;
            case AST_Reference:        Print("Ref");          break;
            case AST_Dereference:      Print("Deref");        break;
            case AST_Spread:           Print("Spread");       break;
            
            case AST_ArrayType:
            {
                Print("(ArrayType ");
                PrintASTNode(node->array_type.size, 0);
            } break;
            
            INVALID_DEFAULT_CASE;
        }
        
        Print(" ");
        if (node->kind == AST_ArrayType) PrintASTNode(node->array_type.elem_type, 0);
        else                             PrintASTNode(node->unary_expr, 0);
        Print(")");
    }
    
    else if (node->kind == AST_Subscript)
    {
        Print("(Subs ");
        PrintASTNode(node->subscript_expr.index, 0);
        Print(" ");
        PrintASTNode(node->subscript_expr.array, 0);
        Print(")");
    }
    
    else if (node->kind == AST_Slice)
    {
        Print("(Slice ");
        PrintASTNode(node->slice_expr.start, 0);
        Print(" ");
        PrintASTNode(node->slice_expr.one_after_end, 0);
        Print(" ");
        PrintASTNode(node->slice_expr.array, 0);
        Print(")");
    }
    
    else if (node->kind == AST_Call)
    {
        Print("(Call ");
        PrintASTNode(node->call_expr.func, 0);
        Print(" (");
        
        for (AST_Node* child = node->call_expr.params; child != 0; child = child->next)
        {
            PrintASTNode(child, 0);
            if (child->next != 0) Print(", ");
        }
        
        Print("))");
    }
    
    else if (node->kind == AST_Conditional)
    {
        Print("(Cond ");
        PrintASTNode(node->conditional_expr.condition, 0);
        Print(" ");
        PrintASTNode(node->conditional_expr.true_clause, 0);
        Print(" ");
        PrintASTNode(node->conditional_expr.false_clause, 0);
        Print(")");
    }
    
    else if (node->kind == AST_If)
    {
        Print("(If ");
        PrintASTNode(node->if_statement.init, 0);
        Print(" ");
        PrintASTNode(node->if_statement.condition, 0);
        Print(")\n");
        PrintASTNode(node->if_statement.true_body, indent);
        PrintASTNode(node->if_statement.false_body, indent);
    }
    
    else if (node->kind == AST_When)
    {
        Print("(When ");
        PrintASTNode(node->when_statement.condition, 0);
        Print(")\n");
        PrintASTNode(node->when_statement.true_body, indent);
        PrintASTNode(node->when_statement.false_body, indent);
    }
    
    else if (node->kind == AST_While)
    {
        Print("(While ");
        PrintASTNode(node->while_statement.init, 0);
        Print(" ");
        PrintASTNode(node->while_statement.condition, 0);
        Print(" ");
        PrintASTNode(node->while_statement.step, 0);
        Print(")\n");
        PrintASTNode(node->while_statement.body, indent);
    }
    
    else if (node->kind == AST_Break || node->kind == AST_Continue)
    {
        Print("(%s %S)", (node->kind == AST_Break ? "Break" : "Continue"), Identifier_Of(node->break_statement.label));
    }
    
    else if (node->kind == AST_Defer)
    {
        Print("(Defer)\n");
        PrintASTNode(node->defer_statement, indent);
    }
    
    else if (node->kind == AST_Return)
    {
        Print("(Return (");
        
        for (AST_Node* child = node->return_statement.values; child != 0; child = child->next)
        {
            PrintASTNode(child, 0);
            if (child->next != 0) Print(", ");
        }
        
        Print("))");
    }
    
    else if (node->kind == AST_Using)
    {
        Print("(Using ");
        PrintASTNode(node->using_statement.expression, 0);
        Print(")");
    }
    
    else if (node->kind == AST_Assignment)
    {
        Print("(");
        switch (node->assignment_statement.kind)
        {
            case AST_Mul:                  Print("*");   break;
            case AST_Div:                  Print("/");   break;
            case AST_Rem:                  Print("%");   break;
            case AST_BitwiseAnd:           Print("&");   break;
            case AST_ArithmeticRightShift: Print(">>>"); break;
            case AST_RightShift:           Print(">>");  break;
            case AST_LeftShift:            Print("<<");  break;
            case AST_Add:                  Print("+");   break;
            case AST_Sub:                  Print("-");   break;
            case AST_BitwiseOr:            Print("|");   break;
            case AST_BitwiseXor:           Print("^");   break;
            case AST_And:                  Print("&&");  break;
            case AST_Or:                   Print("||");  break;
            INVALID_DEFAULT_CASE;
        }
        
        Print("= ");
        PrintASTNode(node->assignment_statement.left, 0);
        Print(" ");
        PrintASTNode(node->assignment_statement.right, 0);
        Print(")");
    }
    
    else if (node->kind == AST_VariableDecl)
    {
        Print("(");
        
        for (AST_Node* name = node->var_decl.names; name != 0; name = name->next)
        {
            PrintASTNode(name, 0);
            if (name->next != 0) Print(", ");
        }
        
        Print(":");
        PrintASTNode(node->var_decl.type, 0);
        
        Print("=");
        
        for (AST_Node* value = node->var_decl.values; value != 0; value = value->next)
        {
            PrintASTNode(value, 0);
            if (value->next != 0) Print(", ");
        }
        
        Print(")");
    }
    
    else if (node->kind == AST_ConstantDecl)
    {
        Print("(");
        
        for (AST_Node* name = node->var_decl.names; name != 0; name = name->next)
        {
            PrintASTNode(name, 0);
            if (name->next != 0) Print(", ");
        }
        
        Print(":");
        PrintASTNode(node->var_decl.type, 0);
        
        Print(":");
        
        for (AST_Node* value = node->var_decl.values; value != 0; value = value->next)
        {
            PrintASTNode(value, 0);
            if (value->next != 0) Print(", ");
        }
        
        Print(")");
    }
    
    else
    {
        ASSERT(node->kind == AST_IncludeDecl);
        Print("(Include %S)", StringLiteral_Of(node->include.path));
    }
}

internal void
ArgError(char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    Memory_Arena_Marker marker = Arena_BeginTempMemory(&Win32_Temp_Arena);
    
    umm buffer_size = String_FormatArgList((Buffer){0}, format, args);
    
    Buffer buffer = {
        .data = Arena_PushSize(&Win32_Temp_Arena, buffer_size, ALIGNOF(u8)),
        .size = buffer_size,
    };
    
    String_FormatArgList(buffer, format, args);
    
#define ERR_PREFIX "Invalid Argument Error: "
#define ERR_SUFFIX "\n"
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), ERR_PREFIX, sizeof(ERR_PREFIX) - 1, 0, 0);
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer.data, (u32)buffer_size, 0, 0);
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), ERR_SUFFIX, sizeof(ERR_SUFFIX) - 1, 0, 0);
#undef ERR_PREFIX
#undef ERR_SUFFIX
    
    Arena_EndTempMemory(&Win32_Temp_Arena, marker);
    
    va_end(args);
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
    
    // NOTE: usage mm [main file] [options]
    // valid options: ...
    
    u8* scan = command_line;
    
    // TODO: Is it safe to assume this produces a valid path to core?
    Path working_dir       = {0};
    Path default_core_path = {0};
    { /// Extract path to core folder
        for (; *scan && *scan != ' '; ++scan);
        if (*scan != 0) *scan++ = 0;
        
        
        PathRemoveFileSpecA((LPSTR)command_line);
        
        String exe_path = String_FromCString(command_line);
        String addend   = STRING("\\core\\");
        
        Memory_Arena_Marker marker = Arena_BeginTempMemory(&Win32_Temp_Arena);
        
        String core_path;
        core_path.size = exe_path.size + addend.size;
        core_path.data = Arena_PushSize(&Win32_Temp_Arena, default_core_path.size + 1, ALIGNOF(u8));
        
        Copy(exe_path.data, core_path.data, exe_path.size);
        Copy(addend.data, core_path.data + exe_path.size, addend.size);
        core_path.data[core_path.size] = 0;
        
        u32 required_size = GetFullPathNameA((LPCSTR)core_path.data, 0, 0, 0);
        
        u8* memory = Arena_PushSize(&Win32_Arena, required_size, ALIGNOF(u8));
        
        if (!GetFullPathNameA((LPCSTR)core_path.data, required_size, (LPSTR)memory, 0)) encountered_errors = true;
        else
        {
            default_core_path = String_FromCString(memory);
            
            // TODO: WTF Windows
            for (umm i = 0; i < default_core_path.size; ++i)
            {
                if (default_core_path.data[i] == 0x22)
                {
                    default_core_path.data += i + 1;
                    default_core_path.size -= i + 1;
                }
            }
        }
        
        Arena_EndTempMemory(&Win32_Temp_Arena, marker);
        
        if (!encountered_errors)
        {
            required_size = GetCurrentDirectory(0, 0);
            
            working_dir.size = required_size,
            working_dir.data = Arena_PushSize(&Win32_Arena, required_size + 1, ALIGNOF(u8));
            
            if (!GetCurrentDirectory(required_size, (LPSTR)working_dir.data)) encountered_errors = true;
            else
            {
                working_dir.data[working_dir.size - 1] = '\\';
                working_dir.data[working_dir.size]     = 0;
            }
        }
    }
    
    if (!encountered_errors)
    {
        while (*scan != 0 && *scan == ' ') ++scan;
        
        u8* start = scan;
        
        while (*scan != 0 && *scan != ' ') ++scan;
        if (*scan != 0) *scan++ = 0;
        
        args->main_file = String_FromCString(start);
        
        if (args->main_file.size == 0)
        {
            ArgError("missing main file path");
            encountered_errors = true;
        }
    }
    
    if (!encountered_errors)
    {
        // TODO: parse options
    }
    
    args->path_labels      = Arena_PushSize(&Win32_Arena, sizeof(Path_Label) * 2, ALIGNOF(Path_Label));
    args->path_label_count = 2;
    
    args->path_labels[0] = (Path_Label){
        .label = {0},
        .path  = working_dir
    };
    
    args->path_labels[1] = (Path_Label){
        .label = STRING("core"),
        .path  = default_core_path
    };
    
    return !encountered_errors;
}

void __stdcall
WinMainCRTStartup()
{
    System_InitArena(&Win32_Arena);
    System_InitArena(&Win32_Temp_Arena);
    
    Command_Line_Arguments args = {0};
    if (ParseCommandLineArguments((u8*)GetCommandLineA(), &args))
    {
        if (MM_Init(args.main_file, args.path_labels, args.path_label_count))
        {
            AST_Node* decl;
            if (MM_ResolveNextDecl(&decl))
            {
                PrintASTNode(decl, 0);
                Print("\n");
            }
        }
    }
    
    ExitProcess(0);
}