typedef enum MM_SYMBOL_KIND
{
    MM_Symbol_Invalid = 0,
    
    MM_Symbol_Variable,
    MM_Symbol_Constant,
    MM_Symbol_UsingLink,
    MM_Symbol_Parameter,
    MM_Symbol_ReturnValue,
    MM_Symbol_BlockLabel,
    MM_Symbol_StructMember,
    MM_Symbol_UnionMember,
    MM_Symbol_EnumMember,
} MM_SYMBOL_KIND;

typedef struct MM_Symbol
{
    MM_SYMBOL_KIND kind;
    struct MM_Symbol* prev;
    struct MM_Symbol* next;
    
    union
    {
        MM_String name;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
        } variable;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: value
        } constant;
        
        struct
        {
            MM_String name;
            // symbol
        } using_link;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value?
        } parameter;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value?
        } return_value;
        
        struct
        {
            MM_String name;
            // TODO: Reference to the block it is applied to
        } block_label;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value
            // TODO: offset?
        } struct_member;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: default value
        } union_member;
        
        struct
        {
            MM_String name;
            MM_Type_ID type;
            // TODO: value
        } enum_member;
    };
    
    MM_AST* ast;
} MM_Symbol;

typedef struct MM_Symbol_Table
{
    MM_Symbol* symbols;
    MM_Symbol** map;
    MM_u32 map_size;
    MM_u32 symbol_count;
} MM_Symbol_Table;

// TODO: Dependencies between symbols could either be through a using link, the type or the value.
//       Using links are easy to track, since they are direct links, while the type and value are
//       harder, since they may be extremely indirect. A top level declaration is treated as one unit,
//       and depdencies are only tracked between top level declarations (what about local procedures?).
//       Top level declarations can only depend on other top level declarations in their entirety, or
//       declarations that are contained in themselves. This means every dependency contained in a
//       top level declaration can be thought of as a dependency from the top level decl itself.
//       "Outbound" dependencies can therefore be gathered and stored with the top level declarations,
//       which would simplify tracking. This essentially means the only symbols interested in dependency
//       information are the global symbols. (dependency tracking within top level declarations could be
//       done as well, however, the entire point of dependency tracking is to cull the amount of rechecking
//       that would need to be done, which should probably not be a lot of work within declarations, aside from
//       the odd 10k line procedure). More sophisticated tracking could greatly cull the amount of work needed,
//       however, I don't believe that rechecking should be something that will happen a ton in practice, and
//       I am not confident that I am able to track all those details gracefully. The tracking will therefore
//       be limited to an unweighted directed (possibly) cyclic graph. The dependencies would most naturally be
//       depicted as arcs from the depender to the dependee. However, I believe it would make sense to flip
//       the direction of those arcs, since this would allow storing dependers on the side of the dependee
//       (kind of like a listener list for event systems). This would make traversing the dependencies for
//       rechecking easier, however, it would also make it harder to remove or list dependencies. Storing
//       the relationship at both ends would make both cases easier, but would increase book keeping overhead.
//       Another option is storing the relationship in some sort of table. A sparse matrix could work, but
//       I have no idea how that could be implemented. The matrix would need to be easily resizable, but
//       would only need to deal with booleans, which could then be stored as e.g. bitfields. Or...
//       They would probably be ternary, not binary, since the graph is directed. However, the addressing order
//       could be used to distinguish the directions. Maybe row first represents the "positive" direction,
//       and column first represents the "negative". The matrix would then have a diagonal of 1s, with some
//       scattered places being set, but most places are 0. The common operations are insertion, deletion,
//       iteration over rows with a given column set, and columns with a given row set. 1s could be
//       represented as the existence of an entry (0 being the place not corresponding to an entry that exists).
//       The problem is then coming up with a representation that is efficient at iterating by rows and
//       columns (with one of them fixed), while still not taking up a huge amount of memory. Two hash tables
//       could maybe work (one for the row, and one for the column). Although that would be kind of clunky to
//       work with. It could maybe be easier to just store the bitfields, although they would be gargantuan.
//       E.g. 40k declarations would require a 625x625 matrix of u64, which would take up ~25 MB of memory,
//       and extending it would either break row/column contiguity, or require a separate memory range for
//       each row/column. Two hash tables using linked lists, with embedded linked lists for each row/column,
//       seems like a decent solution.

// TODO: There should probably be a way of declaring a procedure without defining it, and later providing the definition.
//       This would nicely fit in with the problem of checking cyclicly dependent procedures, since they require
//       separating the procedure header from the body to complete. Mirroring this in the language would clarify this
//       for usage in the metaprogram, and would also provide a way of declaring declarations that will be generated
//       by the metaprogram, such that tools can gather more information up front about how the program will end up.
//       (E.g. declaring that the symbol SerliazeEntity is a procedure of the form proc(^String_Builder, ^Entity)
//       will provide a tool that doesn't want to do a full live compile the information needed to properly e.g.
//       highlight SerializeEntity as a procedure, and provide autocompletion). However, this does kind of conflict with
//       the whole "no shadowing or redefinitions" mindset I designed the language with. Having thought more about it,
//       the concept of shadowing does not seem as terrible. The obvious reason is that allowing shadowing is always
//       better than not, if the language provides a way of enforcing user defined rules on shadowing usage, and that
//       it does not severly hinder tools. However, shadowing can also be useful, in that it can provide a way of
//       modifying the context of generated code (e.g. if the code expects a context, the caller can hide its own
//       context and provide a separate one by declaring a shadowing copy of the context) and also provide more
//       utility to explicit overloading. The last one is a big advantage, since this means overloading can be
//       entirely explicit, while still being as useful as implicit overloading. By making overloading explicit
//       and allowing the metaprogram to add overloads to the explicit overload set gives the compiler the
//       nice advantage of always having to serve a single declaration to the metaprogram when the metaprogram
//       asks for the declaration of a symbol. If overloading is implicit, the compiler would have to serve
//       n declarations, which would all have to be handled by the metaprogram, and this special case in the
//       metaprogram would not be very special, since it would need to be handled on every declaration query,
//       since the metaprogram cannot know if the symbol is overloaded without checking. By making overloading
//       explicit, by providing a way of declaring procedure overload sets, would allow the metaprogram to always
//       handle one declaration at a time, and destinguish between overload sets and other declarations by type.
//       Making overloading explicit would increase friction when writing code, however, this could be alleviated
//       by using some sort of tagging that can inform the metaprogram to generate the boilerplate. This tagging
//       should be mostly user controlled, and the language should only provide an expressive way of using these
//       tags (The examples use Jai/Odin compiler directive syntax for these tags). Another benefit of doing it this
//       way, is that polymorphism could be an abstraction over procedure overload sets and compiler generated
//       procedures. The only remaining problem I can think of is that this forward declaration should probably also
//       work for regular values, not only procs, structs, unions and enums, since there is value in stating that
//       a given symbol is of some type but the value is generated later.
/*
   
SerializeEntity :: proc(builder: ^String_Builder, entity: ^Entity) ---

SerializeEntity :: proc(builder: ^String_Builder, entity: ^Entity) ---

SerializeEntity :: proc(builder: ^String_Builder, entity: ^Entity)
{
}


#overloads(Add)
Add_U32 :: proc(a: u32, b: u32) -> u32
{
    return a + b;
}

Add :: proc[Add_u32, Add_f32];

{
    Add :: proc[Add, Add_v3]; // local overload
    
#verloads(Add);
    Add_v3 :: proc(a: v3, b: v3) -> v3
    {
    }
}

Add_p :: proc(t: $T, s: $S) -> T where T != S

Add_p :: proc[Add_p@u32i32, Add_p@i320u32];

Add_p@u32i32 :: proc(t: u32, s: i32) -> u32
{
}

Add_p@i32u32 :: proc(t: i32, s: u32) -> i32
{
}

Entity :: struct ---

Entity_Kind :: enum ---

Entity_Kind :: enum
{
    Door,
    Window,
    Apple
}

A :: proc(int) -> int ---
A :: proc(f32) -> f32 ---

B :: 0;

{
    B :: "Hello World";
    print(B);
}

print("This is the % message", (B < 3 ? .["1st", "2nd", "3rd"][B] : format_string("%th", B)));


// need a way of declaring a soft int
A : i32 : 0;
A : i32 : ---;

// maybe?
A : soft_int : 0;
A : soft_int : ---;

// procedures are weird
B : proc(int) : ---;
B : proc(int) :  proc(int) ---;

// procedure overload set
BB :proc[]: ---;    // forward declaring an overload set with unspecified content
BB :proc[]: proc[]; // declaring an overload set with no content

// structs are not
C :typeid: ---;
C :typeid: struct ---;

// struct overload set
 CC :struct[]: ---;    // forward declaring an overload set with unspecified content
 CC :struct[]: struct[]; // declaring an overload set with no content

// overload set with specified signature
DD :: proc[proc($T) -> int]

// proc like syntax with signatures for overload sets?
format_string :proc[proc($T) -> string]: ---;
format_string :proc[proc($T) -> string]: proc[proc($T) -> string] ---;
format_string :proc[proc($T) -> string]: proc[proc($T) -> string] {format_int, format_float};
*/

// TODO: Problems:
// - struct[] and proc[] use the same syntax to indicate a type, and an overload set
// - procs can sort of be forward declared in two different ways
// * the two different ways sort of imply two different things, so it is okay
// - is providing the soft_int type a good idea?
// * whitout much thought, yes
// - shadowing, how to refer to the outer version inside the inner version?
// - should overload sets provide a way of specifying a signature the procedures need to match?
// - should it be possible to take a pointer to an overload set?
// - runtime overloading? (this is probably something that should be done in user space)