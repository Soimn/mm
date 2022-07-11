typedef enum  MM_COMPILATION_UNIT_KIND
{
    MM_CompilationUnit_None = 0,
    
    MM_CompilationUnit_Variable,
    MM_CompilationUnit_Constant,
    MM_CompilationUnit_When,
    MM_CompilationUnit_Using,
} MM_COMPILATION_UNIT_KIND;

typedef struct MM_Compilation_Unit
{
    // TODO: origin
    // TODO: history
    // TODO: dependencies
    MM_COMPILATION_UNIT_KIND kind;
    MM_AST* ast;
    MM_Global_Symbol* symbol;
} MM_Compilation_Unit;