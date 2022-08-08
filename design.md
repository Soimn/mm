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

// TODO: Problems:
// - procs can sort of be forward declared in two different ways
// * the two different ways sort of imply two different things, so it is okay
// - is providing the soft_int type a good idea?
// * whitout much thought, yes
// - struct[] and proc[] use the same syntax to indicate a type, and an overload set
// - shadowing, how to refer to the outer version inside the inner version? (jai uses #this for lambdas)
// - should overload sets provide a way of specifying a signature the procedures need to match?
// - should it be possible to take a pointer to an overload set?
// - runtime overloading? (this is probably something that should be done in user space)
// - is adding a new keyword for recursion a good idea? (e.g. recurse(args))
// - how does specialization work with this scheme? (proc(int) specializing proc($T))


// TODO:
// I am at this point kind of set on doing explicit overload sets, instead of implicit overloading. This
// feature could also be used to implement polymorphism by generating declarations and adding them to
// an overload set. Doing this does however raise a question: how should struct polymorhism work? If
// procedure polymorphism uses the overload sets, it would only make sense for struct poly to do the same.

Struct :: struct(N: int, T: typeid)
{
a: [N]T;
}

Struct@1int :: struct
{
a: [1]int;
}

Struct@2int :: struct
{
a: [2]int;
}

Struct@3int :: struct
{
a: [3]int;
}

Struct :: struct[Struct@1int, Struct@2int, Struct@3int];

a: Struct(1, int);
b: Struct(2, int);
c: Struct(3, int);

// TODO: It seems like polymorphism is too hard to take on right now, and probably should not be merged tightly with
//       overload sets. Polymorphism will have to wait, overloading is prioritized.

// TODO: Remaining problems (problems related to polymorphism were cut):
// - shadowing, how to refer to the outer version inside the inner version? (jai uses #this for lambdas)
// * "shadowed" builtin
// - is adding a new keyword for recursion a good idea? (e.g. recurse(args))
// * there should probably be something more like Jai's #this instead, since it could be used for several other things as well (e.g. pass itself as argument)
// - should overload sets provide a way of specifying a signature the procedures need to match?
// - should it be possible to take a pointer to an overload set?
// - runtime overloading? (this is probably something that should be done in user space)
// - soft_int and proc[] cannot be modified by ^ and [], seems problematic that there are types which do not work with ^ and []
// - there should probably be a way of querying information about symbols (e.g. is_defined()), how should this work with type checking?
// * add builtins for this

 There should probably be some way of accessing the nth outer version of a shadowed symbol, maybe steal jai's backtick syntax?
In Jai this is used to allow macros to define sybmols in the caller's scope. Backtick could mean "look one scope up", which
would then work for both accessing shadowed variables and defining variables in an outer scope

However, this does have some weird implications

if (condition)
{
`a := 0;
}
else
{
}

would then introduce a symbol in the outer scope dependent on a runtime evaluated condition "condition". For this to work
the language would have to have some sort of python-esque interpreter, which I want to avoid at all costs. This could
probably just be tagged as illegal, but it still is a pitfall in the language design.

something like
when (condition)
{
`a := 0;
}

should however work, but this would not behave as expected when {} is seen of as a scope, since when does not have an actual scope
and the `a would then point to the outer scope of the outer scope, not the outer scope

It is also not clear if the backtick should be usable in procedures. This could be possible if a procedure is always inlined, but it is kind of shady,
and could easily be misinterpreted as accessing a shadowed version of a symbol

Merging Jai's backtick with shadowed variable access therefore seems like a bad idea

Accessing the outer version of a shadowed symbol does seem like a niche thing to do, since I don't really know of any language that supports it,
and it could be that it has really limited usability. Using something like backtick for the syntax would therefore seem like a bad idea, since
backticks are not very visible, which would add more friction to understanding a piece of code, especially since the semantics are not
very common. Using something more explicit like #shadowed symbol, for accessing the immediate outer version, and #shadowed(2) for two scopes up,
seems like a better idea. If this feature proves to be more useful in the future, a syntactic shortcut could be added. It would probably be a good
idea of doing something like cast and transmute, where it works as a builtin procedure. The name can be shadowed for now, but should probably be
changed to something more understandable.







Struct :: struct
{
	next: ^this();
}

///
proc -> proc {
	return this();
}

is the same as

A :: proc -> proc {
	return A;
}
///




overloading
overloading should be explicit by using some sort of "overload set". This is to reduce complexity in the compiler, the metaprogram, tools and in the language,
at the cost of adding more boilerplate. The current plan is to implement it similar to how Odin does it, with the addition of being able to add and remove
overloads in the metaprogram. Adding overloads in the language can be done by shadowing the overload set with a new overload set that inceludes the shadowed
set, as well as the additions. How should overloads be replaced? If the shadowed version wants to replace or remove some of the procedures in the set, this
should probably be possible. However, there are problems with explicitly removing or replacing items from what is essentially an abstract collection. Since
this would require knowledge about what the overload set contains. Doing it manually by copy pasting the items, and removing the unwanted ones, is also
a bad idea, since now there are two seperate versions of something that wants to be in sync, creating a maintenance problem. A better solution would
probably be to do something like "replace if exists", since the replacement is valuable locally, and it doesn't matter if the non-local version exists
or not, just that is isn't used. For convenience sake, it could work like any procedure that is added to an overload set, with an already resident proc
of the same signature, will always shadow the existing proc. This would probably work as expected in most cases, but would probably lead to some hard to
find bugs if it goes wrong. Doing it more explicitly is probably a better idea. Maybe tagging an overload added to a set with soemthing that indicated whether
it is meant to shadow an existing overload, or never collide. This would make the intent behind the overload set more explicit, which would hopefully
make it easier to hunt down related bugs. This is something that has to happen at the type level, and should therefore probably be a builtin (something
that should be considered is making the type system more flexible to metaprograming, however this would probably make the project way too hard to ever
finish...).

Add :: proc[]
{
Add :: proc[shadowed(Add), no_shadow(Add_v3), shadow(Add_v4)];
}

no_shadow(a) := 0;

shadow(b) := 0;

To control how new overloads should be added to an overload set, the same logic as shadowing in scopes could probably be used, since it works very similar,
and should hopefully be very familiar. Some builtins like no_shadow(symbol) and shadow(symbol) could probably be used in both contexts (like shown above).
I don't know if this is the best idea (mostly because it doesn't cover everything needed to manipulate an overload set like a normal colelction, e.g. hashmap,
but it probably shouldn't anyway). This is the solution I will be going with for now.

// TODO: remaining problems
// - should overload sets provide a way of specifying a signature the procedures need to match?
// * for now, no
// - should it be possible to take a pointer to an overload set?
// * no
// - runtime overloading? (this is probably something that should be done in user space)
// * no
// - soft_int and proc[] cannot be modified by ^ and [], seems problematic that there are types which do not work with ^ and []
// * soft_int should probably not exist, and overload sets should not allow those decorators

Is there value in restricting the signatures procs in an overload set can have? A signature like proc(int) -> int is a useless restriction, since
this would only allow one procedure and shadows of it, completely discarding the usefulness of overloading. proc(int) with some way of specifying
allowing any return value is also useless, since overloads are resolved by their arguments, not return value (this will not be changed, it adds
way too much complexity for my liking). proc(...) -> int could be somewhat useful, maybe providing an overload set of every way of making a V4,
with the restriction that it can only return a V4 (not e.g. an additional boolean or something). I don't see how proc(a, b, ...), with some way of specifying
it can return anything, is useful either. The only real benefit from these restrictions is therefore ensuring a certain return value, which could
actually be useful (it at least makes type checking easier).

Restrictions on overload sets therefore seem kind of useless, and since they are easy to retrofit (since they only add), I will opt to reconsider them later.

Should overload sets have a type, and if they should, what should the type be? If restrictions are not a problem, then all overload sets could in theory have
the same type, with the value describing the actual set (otherwise it would have to be described in the type, which would lead to a type with no associated value).
The syntax could probably be similar to procedure syntax (where the header is similar to the type, and the body is tagged on after). The only big problem remaining
is then if it should be possible to take a pointer to the overload set (this sort of ties in with whether soft types should have actual types or not). Taking the
address of an overload set does not make any sense if the overload set only exists at compile time. Overload don't need to exist at runtime, since they can easily
be represented at runtime with a table of procedures and a resolver (this is waaay too much overhead for anything serious, and should most definitely not be provided
as a general construct by the language, and instead be implemented by the madmen who want to use something like this). This means taking a pointer to an overload set
should be illegal, which in turn makes adding the pointer decorator to the overload type illegal (slices are essentially fat pointer, so the same logic follows for
them).

// TODO: Macro and proc overloading

Jai supports overloading both macros and procedures (since macros are esentially procedures with some extra fluff). Is this something that should be supported?
This could probably be where overload restrictions are useful (allowing either only macros, only procs or both macros and procs). Maybe not. This is probably
something that should be doable in a metaprogram instead of the language.

Syntax
since struct overloading does not make sense (polymorhism does, but overloading does not), the syntax should probably look like something "proc-y". The question
is then if the syntax should leave room for macros or not. Jai does macros with the same syntax as normal procs, but I kind of plan on replacing the "proc"
keyword with "macro" to emphasize that it is a macro. This does however make it weird to specify an overload set that can contain both procs and macros with
proc[]. It may cause some refactoring friction, but it seems like it is smart to start with proc[] for only procs, then add macro[] when macros are introduced,
and maybe add something thet unifies them if it seems useful.

proc[] is the type
proc[] {} is the thing

Add :proc[]: proc[] {
	Add_u8,
	Add_i8,
	Add_i16
}

The last thing to decide is how forward decl syntax should work

// need a way of declaring a soft int
A : i32 : 0;
A : i32 : ---;

// Without soft_int something like this is not possible
// this seems like a weird hole in the expressivity of the
// langauge.
A : soft_int : 0;
A : soft_int : ---;
// maybe there should be something like
A :: ---;
// although that is mostly useless apart from forward declaring soft types (if soft types cannot be expressed)
// maybe soft types should be available, however they cannot be assigned to variables

// procedures are not weird
B : proc(int) : ---;
B : proc(int) :  proc(int) ---;

// procedure overload set
BB :proc[]: ---;        // this does not make sense
BB :proc[]: proc[] ---; // forward declaring an overload set

C :typeid: ---;
C :typeid: struct ---;

// TODO: Remaining problems:
// - soft types (soft_int or not?)
// - proc overload set syntax
// - forward decl syntax semicolons

The point of forward declarations is to enable the programmer to state that a certain symbol is something vague whithout fully declaring what that
something is. This is useful when dealing with generated code, since it can inform a reader about what will be generated, as well as possibly give
the compiler the ability to issue better error messages for generated code. The first obvious question to answer is then what the minimal information
one can provide with such declarations, and what is the maximal. The maximal information is, in case of values, the type, and in case of procs, structs
and similar, the type and header. The minimal information a programmer needs to provide should probably be the type, since I don't see much value in
being able to reserve an identifier for a declaration (since most often you know at least the type). This would then mean that forward declarations
are declarations that are missing the value, or body of a declaration. This would then lead to the syntax:

A :int: ---;           // Forward declare an int
A :typeid: ---;        // Forward declare a type
A :typeid: struct ---; // Forward declare a struct type
A :: struct ---;       // Forward declare a struct type
A :struct{}: ---;      // Forward declare something of type struct{}
A :proc: ---;          // Forward declare a procedure (pointer or literal)
A :proc: proc ---;     // Forward declare a procedure literal

The problem is then how forward delcarations should work with soft types, and how overload set syntax should work.

A :soft_int: 0;

a : soft_int; // illegal
a : proc[]; // illegal

A :proc[]: proc[] {B, C};
A :proc[]: proc[] ---;
A :proc[]: ---;

A :: proc ---;
{
A :typeid: proc_set {A, B, C, D};
}

A :typeid: proc_set{B, C, D}
A :typeid: proc_set ---;
A :typeid: ---;


glclear :: proc ---
pi_int :: proc -> int ---

proc -> string
{
	return to_string(typeof(this))
}

this   - reference to the enclosing declaration's symbol
this() - equivalent to using the enclosing declaration's symbol instead of this (InfiniteRecursion :: proc { this(); })

// TODO: Unify builtins

"this" does not want to be a builtin in the sense that it works like a procedure
"shadowed" wants to be a procedure like thing
"shadow" and "no_shadow" are more like decorators
"sizeof", "alignof", "offsetof", "typeof", "cast" and "transmute" are definitely proc like

// TODO: Think about this later, shadowed, shadow and no_shadow can be implementer later, this is added now

// TODO: Additions
// - forward declarations
// * done
// - "shadowed" builtin (this could be weird when dealing with global scope, e.g. A :: 0; A :: proc -> int { return shadowed(A); })
// - "this" builtin
// * done
// - builtins for symbol info queries
// - see if '.' before name of named value is possible AND a good idea
// * not a good idea
// - add proc_set overload syntax
// * done
// - add soft types
// * done
// - forward declarations do use semicolons
// * done


// Symbol dependency tracking


Add :: proc_set { Add_i8, Add_u8 }

#overloads(Add)
Add_v3 :: proc(a, b: v3) -> v3
{
	print("outer");
	return .{ x = a.x + b.x, y = a.y + b.y };
}

{
	// This shadows the previous Add_v3
    Add_v3 :: proc(a, b: v3) -> v3
	{
		print("inner");
		return .{ x = a.x + b.x, y = a.y + b.y };
	}

	// This shadows the previous Add
	Add :: proc_set { Add, Add_v3 }

	// If you want to be explicit, you could do this too
	// Add :: proc_set { shadowed(Add), Add_v3 }

	a, b: v3;

	Add(a, b);              // prints "inner"
	Add_v3(a, b);           // prints "inner"
	shadowed(Add)(a, b);    // prints "outer"
	shadowed(Add_v3)(a, b); // prints "outer"

	{
		Add :: 0;
		print(Add);                 // prints "0"
		shadowed(Add)();            // prints "inner"
		shadowed(Add, level = 2)(); // prints "outer"
	}
}

// Depenencies on a are mapped as dependencies on GlobalThing
using GlobalThing :: struct{a: int;}.{};

// Dependencies cannot peer through when declarations (they need to be resolved before dependencies can be registered)
when (condition)
{
	B :: a;
}
else
{
	B :: a + 1;
}

A :: B;

// NOTE: default values in structs are now move to user space, .{} will zero init





The problem: øarchitecting the compiler is too hard at the moment because it is a library
memory management and ownership is the main problem right now
I don't see a way of making the library completely opaque to the user without sacrificing a _lot_ of performance
maybe the library should not live on its own and be more of a "single header library" or toolkit


how to fix memory management for use in editors?
text editors need to be responsive and can't wait on a full recompile of the project on every keystroke of the user.
incremental compilation is useful here, since the user can rarely edit all the source files at the same time, which
can be taken advantage of by allowing the editor to only recompile sections that the user has changed.
Choosing exactly what to recompile can be tricky, since text is very unstructured, and recompiling the whole file, instead
of a small text snippet, might be beneficial. The problem is then how the old file should be replaced with the new one.
- replacing old declarations
- replacing the file

Debuggers want to know as much about the program as possible, since this information can be used to aid the user. By storing all compiler
state, along with every source file, alongside other debug information, the debugger can know as much about the program that is running as
the compiler knew during compilation. Additionally, if the user encounters code in the debugger that seems off, the problem causing that
code generation weirdness can be more easily found if the debugger has access to a log of where that code came from and how it may have
been changed by the optimizer (this is especially useful for generated code).
- store all state
- keep a log of useful information that is accessible a debugger

for the compiler to be able to serve the debugger all source files, the source files have to be owned by the compiler



















I want a compiler that can compile an Odin/Jai like language and has good support for metaprogramming and tooling. The metaprogram should be able to do whatever with the compiling code, and the tooling should be able to easily reason and work with it. The most important part of tool support is giving the debugger more information.
The compiler will need to keep track of everything that happens to the code, so that it can provide this information to the debugger. The compiler should therefore
own all relevant data, with an API for managing this. This

write each stage, then decide how the API should behave














Use cases:
- text editor (syntax highlighting, intelligent code editing)
- debugger (introspection, parsing)

lex -> tokens for syntax highlighting
parse -> for watch window and other debugger related tasks
introspect -> for code navigation, completion, evaluation in debugger

x + 2*y

problem: a lot of the work in evaluating an expression in the watch window involves work the compiler does, and it seems like the compiler should provide
more help. The problem with providing more help is that there then needs to be a way of providing the compiler with the execution context. The debugger needs to
support debugging both M and C, and it should still support evaluating expressions in M that references something when debugging C. The debugger then needs to be responsible for prioviding the interface and feeding the compiler with the proper information, which implies the debugger should be given more control. The debugger then would need to keep track of the values and symbols in use and what they reference in a way that is language agnostic, and also have a way of evaluating M
expressions with those values. The easiest thing to do is to use the workspace API and make the debugger insert code that it needs to evaluate and then provide
an API for calling procedures in a workspace with arguments.


lex to tokens
compile code
run expressions
insert code
update code
introspect





Should loose expressions be allowed at global scope? This could be useful for allowing the user to do something like

#private
{
	A :: 0;
	B :: 0;
}

#public
{
	C :: 0;
}

where private and public are user defined labels stuck on statements which serve only to provide the metaprogram with information

another example is


#add_command(Ls);
Ls :: proc()
{
	print("list of files");
}

where add_command is a user defined label that tells the metaprogram to add Ls to a list of procedures to register as commands

another problem is how type related stuff should be handled, since the metaprogram cannot do much with types unless it can provide custom rules to the checker


// implemention #caller_location

main :: proc
{
    line := 0;
    
    
    IncLine :: proc(line: ^int = ^\line)
    {
        line^ += 1;
    }
    
    Print :: proc(msg: string, pos: Source_Pos = \source_pos())
    {
    }
    
    Advance :: proc(lexer: ^Lexer = \^lexer, amount := 1)
    {
        ...
    }
    
    lexer := ...;
    Advance(2);
}


A :: 1;

B :: proc(a: int = \A, b := A)
{
    A :: 3;
    
    {
        A :: 4;
        
        print(A);   // 4
        print(\A);  // 3
        print(a) ;  // 2
        print(\\A); // 1
        print(b);   // 1
    }
}

main :: proc
{
    A :: 2;
    
    B();
}





text -> |lexer| -> tokens
tokens -> |parser| -> ast,textpos

file,offset
