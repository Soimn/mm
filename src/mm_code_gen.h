Gather -> list of entities
gen declarations and typedefs
gen functions

typedef struct Entity_Table
{
    Symbol** entities;
    u32 entity_count;
    u32 entity_capacity;
} Entity_Table;

internal Symbol*
CG_FindEntryPoint(Package_ID main_package)
{
    Package* package = Package_FromID(main_package);
    ASSERT(package != 0);
    
    for (Symbol_Table_Iterator it = Symbol)
        NOT_IMPLEMENTED;
}

internal void
CG_GatherEntities(Entity_Table* table, Symbol* symbol)
{
    NOT_IMPLEMENTED;
}

internal void
CGC_SortEntities(Entity_Table* table)
{
    NOT_IMPLEMENTED;
}

internal void
CGC_GenForwardDeclarations(Entity_Table* table, File_Buffer* out_file)
{
    NOT_IMPLEMENTED;
}

internal void
CGC_GenDefinitions(Entity_Table* table, File_Buffer* out_file)
{
    NOT_IMPLEMENTED;
}

internal bool
CG_GenCCode(Package_ID main_package, File_Buffer* out_file)
{
    bool encountered_errors = false;
    
    Symbol* entry_point = CG_FindEntryPoint(main_package);
    
    if (entry_point == 0)
    {
        //// ERROR: missing entry point
        encountered_errors = true;
    }
    
    else
    {
        Entity_Table entity_table;
        
        CG_GatherEntities(&entity_table, entry_point);
        CGC_SortEntities(&entity_table);
        CGC_GenForwardDeclarations(&entity_table, out_file);
        CGC_GenDefinitions(&entity_table, out_file);
    }
    
    return !encountered_errors;
}


package main;

using import "math";
using import "io";

main :: proc
{
    result := Complex(sin(1.45) + cos(1.45), 2);
    print(result);
}

// io
math.sin
math.cos
math.Complex
io.print
main.main

list of entities needed