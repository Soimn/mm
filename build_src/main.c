#include "stdio.h"
#include "string.h"
#include "stdlib.h"

// IMPORTANT NOTE: This code is intentionally shitty

void
GenHeader()
{
    FILE* src_file = 0;
    fopen_s(&src_file, "..\\src\\mm.h", "r");
    FILE* gen_file = 0;
    fopen_s(&gen_file, ".\\mm.h", "w");
    
    char buffer[1024] = {0};
    while (fgets(buffer, sizeof(buffer), src_file))
    {
        if (strncmp(buffer, "#include", sizeof("#include") - 1) == 0)
        {
            char* path = buffer;
            
            while (*path++ != '"');
            
            char* end = path - 1;
            while (*++end != '"');
            *end = 0;
            
            sprintf(buffer, "../src/%s", path);
            FILE* include;
            fopen_s(&include, buffer, "r");
            
            fprintf(gen_file, "// NOTE: included from: %s\n", path);
            fprintf(gen_file, "#line 1 \"%s\"\n", path);
            while (!feof(include)) fprintf(gen_file, "%s", fgets(buffer, sizeof(buffer), include));
            fprintf(gen_file, "\n\n");
            
            fclose(include);
        }
        else
        {
            fprintf(gen_file, "%s", buffer);
        }
    }
    
    fclose(src_file);
    fclose(gen_file);
}

void
Build(int is_debug)
{
    const char* diag_flags = "/nologo /FC /diagnostics:column /W4 /wd4204 /wd4100 /wd4201 /wd4200 /wd4223 -Wno-microsoft-anon-tag -Wno-logical-op-parentheses -Wno-unused-function";
    
    const char* nocrt_flags      = "/Zl /GR- /GS- /Gs9999999";
    const char* nocrt_link_flags = "/INCREMENTAL:NO /opt:ref /STACK:0x100000,0x100000 /NODEFAULTLIB /SUBSYSTEM:console";
    
    const char* debug_comp_flags = "/DMM_DEBUG=1 /Od /Zo /Z7";
    const char* release_comp_flags = "/O2 /Zo /Z7";
    const char* perf_comp_flags = (is_debug ? debug_comp_flags : release_comp_flags);
    
    const char* format_str = "clang-cl /LD %s %s %s ..\\src\\mm.c /link %s Kernel32.lib /PDB:libmm.pdb /out:libmm.dll";
    
    char* command = malloc(1 + snprintf(0, 0, format_str, diag_flags, nocrt_flags, perf_comp_flags, nocrt_link_flags));
    sprintf(command, format_str, diag_flags, nocrt_flags, perf_comp_flags, nocrt_link_flags);
    
    system(command);
}

int
main(int argc, const char** argv)
{
    GenHeader();
    Build(strcmp(argv[1], "debug") == 0);
}