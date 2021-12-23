Gather -> list of entities
gen declarations and typedefs
gen functions

internal void
GatherSymbols(Symbol* symbol)
{
    if (symbol->kind == Symbol_Var)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (symbol->kind == Symbol_Const)
    {
        Type_Info* info = TypeInfo_FromID(symbol->type);
        
        if (info->kind == Type_Proc)
        {
            NOT_IMPLEMENTED;
        }
        
        else
        {
            NOT_IMPLEMENTED;
        }
    }
    
    else if (symbol->kind == Symbol_Package)
    {
        NOT_IMPLEMENTED;
    }
    
    else if (symbol->kind == Symbol_Parameter)
    {
        NOT_IMPLEMENTED;
    }
    
    else
    {
        ASSERT(symbol->kind == Symbol_ReturnValue);
        NOT_IMPLEMENTED;
    }
}

internal void
GenCCode(Package_ID main_package)
{
    NOT_IMPLEMENTED;
    Symbol* entry_point = 0;
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