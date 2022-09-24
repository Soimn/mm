# The M programming language
## Design Rambling
I started making a lanaguage because it anoyed me how very simple things become complicated/introduce too much garbage, this is very prevailant in languages such as Java, but is still present in more sane, but still insane, languages such as C.

I kept on wanting to make a language because of how disgusted I was by C's UB and the alluring benefits of metaprogramming

I wanted to stop making languages because if I would ever be able to finish one, it would likely provide only marginal improvements on C, making the effort wasted, when it could have rather been spent on making games.

I still kept on working on the language, because my time investment was too significant, making it hard to justify stopping.

Goals: simple, sane and easy to make tools for
Subgoal: expressive metaprogramming

I want the language to be simple (in the C sence, not like Lisp), because I dispise working in languages were the language complexity holds me back from doing actual work

I want the language to be sane and without UB, since yuck, everyone who actually trusts C++ code compiled by LLVM to be used in hard realtime systems is either a total nutjob, or the living breathing, example of the "this is fine" meme

The final goal is to make making tooling easy. A lot of problems with C comes from how hard it is to work with on a tooling level, since it is fucking impossible to parse correctly. Making a language easily parsable, have simple semantics and no significant leeway for differences between compilers would make debugger, text editors and other tools, way easier to develop and enable much more advanced tools , which the programmer will benefit greatly from.

Expressive metaprogramming is a subgoal, since metaprogramming provides a great deal of power, but that comes at the cost of it being way too complex to wrap neatly up in a language spec without making it so hard to use correctly that the benefit of doing something meta is inly marginal. Since I, and probably everyone else, have nowhere near the experience needed to design a competent metaprogramming system, I will instead include metaprogramming as a subgoal and only design in favor of metaprogramming when it does not violate the original 3 goals.

Preemptive decisions:
- odin/jai like synatx, since this is easy to parse and C like

small syntax problems
types
compiler arch

odin like syntax with some changes
c like semantics, except types
goto?

names/symbols
typers of values
values

namespacing
type conversion, introspection, value of types
const &, parameters

start with kernel language

```
if (condition) statement else statement
while (init; condition; step) statement
return expression
break label
continue label
{}
a := 0;
A :: 0;
a = b;
```

```
identifier
0.0, 0
"string"
true
struct {}
union {} // ?
enum {}
proc() -> {}

^, [], [N]

., [N], [N:N], (), .{}, .[]

+, -, ~, !, &, ^

*, /, %, &, <<, >>, >>>
+, -, |, ~
==, !=, <, <=, >, >=
&&
||
```

```c
const A = 0;
var b: int = 0;
var b = 0;
```

```c
const main = proc
{
	const N = 100;
	var board_mem: [2][N*N]bool;
	var boards := [2]^[N*N]bool.{&board_mem[0], &board_mem[1]}

	while true
	{
		while y := 0; y < N; y += 1
		{
			while x := 0; x < N; x += 1
			{
				var min_y, max_y := max(0, y - 1), min(y + 1, N - 1);
				var min_x, max_x := max(0, x - 1), min(x + 1, N - 1);

				var neighbours := 0;
				while ny := min_y; ny <= max_y; ny += 1
				{
					while nx := min_x; nx <= max_x; nx += 1
					{
						neighbours += cast(boards[0][ny*N + nx]);
					}
				}

				if neighbours < 2 || neighbours > 3 do boards[1][y*N + x] = false;
				else boards[0][y*N + x] || neighbours == 3 do boards[1][y*N + x] = true;
			}
		}

		boards[0], boards[1] = boards[1], boards[0];
	}
}
```

```c
main :: proc
{
	N :: 100;
	board_mem: [2][N*N]bool;
	boards := [2]^[N*N]bool.{&board_mem[0], &board_mem[1]}

	while (true)
	{
		while (y := 0; y < N; y += 1)
		{
			while (x := 0; x < N; x += 1)
			{
				min_y, max_y := max(0, y - 1), min(y + 1, N - 1);
				min_x, max_x := max(0, x - 1), min(x + 1, N - 1);

				neighbours := 0;
				while (ny := min_y; ny <= max_y; ny += 1)
				{
					while (nx := min_x; nx <= max_x; nx += 1)
					{
						neighbours += cast(boards[0][ny*N + nx]);
					}
				}

				if   (neighbours < 2 || neighbours > 3) boards[1][y*N + x] = false;
				else (boards[0][y*N + x] || neighbours == 3) boards[1][y*N + x] = true;
			}
		}

		boards[0], boards[1] = boards[1], boards[0];
	}
}
```



```c
defer
using
```


types
description of data layout, usage and possible transformations
size, alignment, usage, (operations)

soft types
distinct types
implicit conversions (only make sense for math, makes the expression more mathy and less obscure)


soft, distinct, alias

operator overloading? no
conversion overloading? no

soft int, float and bool and string
distinct int, i8, i16, ... b8, b16, ..., string, cstring

byte, word

implicit conversion rules
soft int -> int, i*, uint, u* as long as the value fits
soft float -> f16, f32, f64 as long as the value is representable exactly
soft bool -> b8, b16 always
soft string -> string, cstring always
alias of T -> T
soft int -> float, f32, f64
\^T -> \^byte
\^byte -> \^T

conversion rules
implicit conversion
int, i*, uint, u*, float, f*, bool, b* -> int, i*, uint, u*, float, f*, bool, b*

^, \[N], \[], \[\^]
^ is a pointer to a single element
\[N] is a fixed N sized owning array, where N is known at compile time, and *owning* mean that it is not a view into memory in a range, it is the memory in that range
^\[N] is a pointer to a fixed N sized array, which essentially works as a non owning fixed array
\[] is a pointer and length pair
\[^] is an array with no specified length

```c
[^:0]int
[]int
[:0]int
```

\^byte = rawptr
\[]byte = \[]u8
\[^:0]byte = cstring
\[:0] = zstring
\[] = string


nil - the nothing address, 0,

namespaces:
using collapses namespaces, and aliases
using a;
using a as b;

goto:
labels
defer

assembly?
asm label:;
asm jmp label;

asm mov rax, i.a;
asm mov rbx, i.b;
asm add rax, rax, rbx;
asm mov c, rax;

asm {
	mov rax, i.a // comments
	mov rbx, i.b
	add rax, rax, rbx
	mov c, rax
}

Compiler arch:
library centered around the concept of a "Workspace"
the workspace holds all state
procedures in the library may be either statefull or stateless, stateless proc: e.g. LexString, statefull proc: e.g. CheckProc
a workspace is serializable as a M workspace file