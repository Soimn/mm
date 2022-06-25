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