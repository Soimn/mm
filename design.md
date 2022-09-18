# The M programming language

## Information
The M language is a systems programming language that aims to
- be as low level as, and with less overhead compared to, C
- have no undefined or implementation behaviour, only platform and architecture specific
- be tool friendly
- allow compile time execution, inspection and metaprogramming of code

### As low level as C
C is, as far as I am concerned, currently the best "low level" systems programming language, but that is only because the competition is garbage. Yes, C can be called "low level", but it has way too much overhead to actually comply to the notion that "low level" = "maximal control". The M language aims to rectify this by having no runtime library, no hidden overhead and give full control over calling conventions, memory layout and access,  and the resulting machine code. This will invevitably lead to more verbosity and mental overhead in some cases, but I believe this is the right choice for a systems programming language.

### No undefined or implementation behaviour
One of the major problems of C, and absolutely everything built around it, is undefined behaviour. This is problematic as a programmer because it breaks your assumptions about how the language works and how the compiler will treat the code, since the list of undefined behaviour is impossible to keep track of mentally (it is not even completely specified in the C standard) and that undefined behaviour is infectious. If it were not for this I would say C is a great language, with some minor flaws. However, with the addition of undefined behaviour, as well as differences in UB handling by different compilers and compiler versions, C goes from a great, to horrible flaming hot garbage of a "programming" language. The only reason why C hasn't died completely because of this, that I can think of, is that every other language does this too, either directly or indirectly by interfacing with the C ABI. I therefore believe that every new programming language should have absolutely no undefined behaviour, and that the spec should clearly define all behaviour on every supported platform and architecture, such that any difference in compiler implementation that is percievable by the programmer is considered a bug. 

To achieve this, the language restricts itself to a select number of platforms and architectures which it is designed for. Any platform and architecture that is not in that subset might be supported to a degree, but must emulate the behaviour defined for the other platforms and architectures.

#### Supported platforms:
- Windows
- Linux

as well as bare metal (no operating system)

#### Supported architectures:
- x64
- AArch64

other architectures may be supported in the future, but never at the cost of the above list

### Be tool friendly
A lot of languages can be both complicated to parse and check. This makes things like text editors and debuggers harder to write than need be. My solution to this problem is to both make the language easy to parse and have a stable AST representation, as well as making the compiler provide facilities for both parsing, full compilation, expression evaluation and queries for symbol-, type- and general information. Furthermore, it is currently planned to record every action the compiler takes, along with everything the compiler loads and creates (text, AST, symbol tables), into a so called *workspace*. This *workspace* will then contain all state used and required for compilation, and can therefore be used to stop compilation, and continue at a later time. This seems like nonsense, but being able to do this also enables debuggers to not only access all state the compiler uses, but also backtrack how the resulting code ended up as it is. I have yet to realize this, but I believe it will be a major improvement over the awful debugging experience that C provides. 

### Allow compiletime execution, inspection and metaprogramming
This feature is not strictly as neccessary as the others, and serves more as way of removing ugly hacks by standardizing how code can be analyzed, modified and generated. Compile time exection allows pregenerating data needed for runtime from code inside the program, instead of making a tool generate the data. Inspection allows user defined checking of code and enables code generation based on the program source. Metaprogramming allows modifying libraries without changing the source text, generation of code and, to some degree, user defined langauge constructs. These features make it easier to check and generate code and data, which makes hacks, like ugly cases of X-macros, unneccessary. 



## Specification


Primitive types
Type | Description
------|-----------
soft_int | soft 256-bit signed integer
soft_float | soft 
int | register sized signed integer
i8 | signed 8-bit integer
i16 | signed 16-bit integer
i32 | signed 32-bit integer
i64 | signed 64-bit integer
i128 | signed 128-bit integer
uint | register sized unsigned integer
u8 | unsigned 8-bit integer
u16 | unsigned 16-bit integer
u32 | unsigned 32-bit integer
u64 | unsigned 64-bit integer
u128 | unsigned 128-bit integer


















































Operators (sorted by precedence, high to low)
```c
// primary
.{}, .[] // struct literal, array literal

// type prefix
^, [], [N] // pointer to, slice of, N size array of

// postfix
^, [N], [N:M], .M, (), .{}, .[] // dereference, subscript, slice, member of, proc call, struct literal, array literal

// prefix
+, -, !, ~, & // plus, neg, not, bit not, address of

// binary
*, /, %, &, <<, >>, >>> // mul, div, rem, bit and, shl, shr, sar
+, -, |, ~ // add, sub, bit or, bit xor
==, !=, <, <=, >, >= // eq, neq, less, less_eq, greater, greater_eq
&& // logical and
|| // logical or
```

```c
<identifier> name specifier
.<expression> index/name specifier, .<int> index, .<string> name
<expression>..<expression> range specifier


Entity :: struct
{
	a: int;
	b: float;
	c: bool;
	d: int;
	e: int;
}

Entity.{ ."a" = 0, .2 = false, 3..4 = 42, b = 3.0 }

.[0..N-1 = 1, .N = 0, N+1..2*N = 1]
```