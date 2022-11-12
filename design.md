# The M Programming Language

## Motivation
The motivation for this language was to enable high degrees of metaprogramming with a simple language and compiler. However, during development it became increasingly clear that reliability, which was stated as an obvious sub-goal, turned out to be way more important. I had underestimated how fragile the current ecosystem is, and how large a problem undefined behavior is. To illustrate, when talking about reliable systems languages, Rust is often mentioned as a "safe" alternative to C++. However, any talk about Rust being "more safe" is largely bullshit. This is because both languages not only have undefined behavior, but the specs have also no exhaustive list of all cases leading to this behavior. Every guarantee the Rust borrow checker, and other language facilities, give you are therefore invalid, because they can be at any point be violated. Furthermore, both languages rely on optimizing compilers built around taking advantage of UB. This is bad as it is, but what is even worse, is that LLVM is used by languages like Julia for scientific computing. A core principle of science is doing experiments in a controlled environment, which necessitates a control over how the program is translated to machine code and executed. Using LLVM is therefore a bad idea, because, void of undefined behavior, it is still way too complex. Saying this, a likely retort is that the experiments have to be run through an optimizer, because hand optimizing them slows down iteration, and running unoptimized will take an unreasonable amount of time. My answer? Speed does not matter if the result cannot be trusted. Garbage is still garbage, however fast it is produced (and it smells way worse in heaps). I therefore firmly believe that there should be a great reset in the programming world, where new languages and optimizers are designed without bullshit like UB, and scientific computing is based on something that have actual guarantees that can be trusted. Do I believe this will ever happen? No, because I have lost all faith in modern programmers being reasonable after seeing everything move closer and closer to insanity. Although something might after a few planes fall from the sky, and the large hadron collider blows up, because of "optimizations". Either way, I have decided that I have had enough of this, and will therefore design a new language that tries to mitigate these flaws, despite being way too inexperienced to actually achieve this, and that no one will actually use the resulting language. The major goal of the language is to be simple and clearly defined, with no undefined behavior or implementation differences. Therefore, the language will have a spec, implementation and standard library, as well as being easy to make tooling for by being easily parsed, compiled and provide an API for talking with the compiler.

## Major Design Decisions
Since I want to finish this project in a reasonable amount of time, and the target audience is, well, myself, I have elected to decide upfront the below design points. This is generally a bad idea, since it reduces the solution space, possibly eliminating optimal solutions. However, since the design space for languages is so large, and I lack experience, starting from scratch will most likely end in me reenacting history. I am instead basing the design on C, since it has proven to be useful, and I personally like large parts of the programming model it imposes. The following design decisions are therefore based on what I have experienced to be good and bad properties of C, along with some personal preferences.
- the language will be a procedural structured systems-programming language
- the syntax will be largely similar to Odin and Jai (because this is close to C, and solves a lot of parsing and expression issues)
- the semantics will be largely similar to C, with notable exceptions being undefined behavior, types, "objects", calling convention and language runtime/startup

## Roadmap
- start with simple syntax and c like semantics
- working compiler
- iteratively modify the compiler and language
- write a debugger
- standardize the compiler + debugger API
- first stable language + compiler, write spec

## Syntax
I am starting with syntax, because I have a vague idea of how the semantics will work, but I have yet to actually test this. Therefore, I am deciding on a somewhat temporary syntax, so that I can implement a working compiler, and play with the semantics in conjunction with the syntax.

Comments are considered whitespace (this may be changed later on, since comments could provide value to the AST)
Comments come in two forms:
- single line comments: "//", which end at either a newline or end of file
- multi line comments: "/\*" opens, "\*/" closes, can be nested, must be closed before end of file 
Syntax is described with modified EBNF, concatenation is implicit, lexical tokens are prefixed with T\_ , H\_ denote helper non-terminals, whitespace is ignored between but not within tokens, N\*something means repeating the something N times
```ebnf
H_letter       = ? alphabetical ASCII characters ?
H_digit        = ? numerical ASCII characters ?
H_binary_digit = "0" | "1"
H_hex_digit    = H_digit | ? upper or lower case A-F ?

T_identifier = H_letter {H_digit | H_letter | "_"}
             | "_" (H_digit | H_letter | "_") {H_digit | H_letter | "_"}
T_blank      = "_"
T_integer    = H_digit {H_digit | "_"}
             | "0b" {"_"} H_binary_digit {H_binary_digit | "_"}
             | "0x" {"_"} H_hex_digit {H_hex_digit | "_"}

H_float_exponent  = "e" ["+" | "-"] {"_"} H_digit {H_digit | "_"}
H_float_fraction  = {"_"} H_digit {digit | "_"}
H_hex_float_digit = {"_"} H_hex_digit
T_float           = H_digit {H_digit | "_"} "." H_float_fraction [float_exponent]
                  | "0h" 4*H_hex_float_digit
                  | "0h" 8*H_hex_float_digit
                  | "0h" 16*H_hex_float_digit

H_escape_sequence = "\\" "\\"
                  | "\\" '"'
                  | "\\" "'"
                  | "\\" "a"
                  | "\\" "b"
                  | "\\" "f"
                  | "\\" "n"
                  | "\\" "r"
                  | "\\" "t"
                  | "\\" "v"
                  | "\\" "x" 2*hex_digit
                  | "\\" "u" 4*hex_digit
                  | "\\" "U" 8*hex_digit
T_string = '"' (H_escape_sequence | ? any UTF-8 unicode codepoint except '"' ?) '"'

T_Struct    = "struct"
T_True      = "true"
T_False     = "false"
T_Nil       = "nil"
T_If        = "if"
T_When      = "when"
T_Else      = "else"
T_While     = "while"
T_Break     = "break"
T_Continue  = "continue"
T_Return    = "return"

T_Cast      = "cast"
T_Transmute = "transmute"
T_Sizeof    = "sizeof"
T_Alignof   = "alignof"
T_Offsetof  = "offsetof"

H_Builtin   = T_Cast
            | T_Transmute
            | T_Sizeof
            | T_Alignof
            | T_Offsetof


T_OpenParen     = "("
T_CloseParen    = ")"
T_OpenBracket   = "["
T_CloseBracket  = "]"
T_OpenBrace     = "{"
T_CloseBrace    = "}"
T_Hat           = "^"
T_Comma         = ","
T_Colon         = ":"
T_Semicolon     = ";"
T_TpMinus       = "---"
T_Period        = "."
T_PeriodParen   = ".("
T_PeriodBracket = ".["
T_PeriodBrace   = ".{"
T_Bang          = "!"
T_Arrow         = "->"
T_Blank         = "_"
T_StarEQ        = "*="
T_SlashEQ       = "/="
T_PercentEQ     = "%="
T_AndEQ         = "&="
T_ShlEQ         = "<<="
T_ShrEQ         = ">>="
T_SarEQ         = ">>>="
T_PlusEQ        = "+="
T_MinusEQ       = "-="
T_OrEQ          = "|="
T_TildeEQ       = "~="
T_AndAndEQ      = "&&="
T_OrOrEQ        = "||="
T_Star          = "*"
T_Slash         = "/"
T_Percent       = "%"
T_And           = "&"
T_Shl           = "<<"
T_Shr           = ">>"
T_Sar           = ">>>"
T_Plus          = "+"
T_Minus         = "-"
T_Or            = "|"
T_Tilde         = "~"
T_EqualEQ       = "=="
T_BangEQ        = "!="
T_Less          = "<"
T_LessEQ        = "<="
T_Greater       = ">"
T_GreaterEQ     = ">="
T_AndAnd        = "&&"
T_OrOr          = "||"

identifier   = T_identifier | T_blank
string       = T_string
int          = T_integer
float        = T_float
bool         = T_True | T_False
proc_type    = T_proc [T_OpenParen paramter_list T_CloseParen] [T_Arrow return_value_list]
proc_lit     = proc_type (block_statement | T_TpMinus)
struct_type  = T_Struct T_OpenBrace [declaration_wsemi {declaration_wsemi}] T_CloseBrace
compound     = T_OpenParen expression T_CloseParen
builtin_call = H_Builtin T_OpenParen argument_list T_CloseParen
struct_lit_inferred = T_PeriodOpenBrace arugment_list T_CloseBrace
array_lit_inferred  = T_PeriodOpenBracket argument_list T_CloseBracket

primary_expression = identifier
                   | string
                   | int
                   | float
                   | bool
                   | proc_type
                   | proc_lit
                   | struct
                   | compound
                   | builtin_call
                   | struct_lit_inferred
                   | array_lit_inferred
                   
argument      = [expression T_Comma] expression
argument_list = [argument] {T_Comma argument}

parameter      = expression {T_Comma expression} T_Colon expression
               | expression {T_Comma expression} T_Colon [expression] T_Equals expression
parameter_list = [expression] {T_Comma expression}
               | parameter {T_Comma parameter}

return_value_list = (expression | T_OpenParen parameter_list T_CloseParen)

pointer_type = T_Hat type_prefix
slice_type   = T_OpenBracket T_CloseBracket type_prefix
array_type   = T_OpenBracket expression T_CloseBracket type_prefix
type_prefix = pointer_typr
            | slice_type
            | array_type

dereference = postfix_expression T_Hat
subscript   = postfix_expression T_OpenBracket expression T_CloseBracket
slice       = postfix_expression T_OpenBracket [expression] T_Colon [expression] T_CloseBracket
call        = postfix_expression T_OpenParen argument_list T_CloseParen
member      = postfix_expression T_Period identifier
struct_lit  = postfix_expression T_PeriodOpenBrace arugment_list T_CloseBrace
array_lit   = postfix_expression T_PeriodOpenBracket argument_list T_CloseBracket
postfix_expression = dereference
                   | subscript
                   | slice
                   | call
                   | member
                   | struct_lit
                   | array_lit
                   | primary_expression

pos       = T_Plus prefix_expression
neg       = T_Minus prefix_expression
not       = T_Bang prefix_expression
bit_not   = T_Tilde prefix_expression
reference = T_And prefix_expression
prefix_expression = pos
                  | neg
                  | not
                  | bit_not
                  | reference
                  | postfix_expression

mul     = mul_level_binary_expression T_Star prefix_expression
div     = mul_level_binary_expression T_Slash prefix_expression
rem     = mul_level_binary_expression T_Percent prefix_expression
bit_and = mul_level_binary_expression T_And prefix_expression
bit_shl = mul_level_binary_expression T_Shl prefix_expression
bit_shr = mul_level_binary_expression T_Shr prefix_expression
bit_sar = mul_level_binary_expression T_Sar prefix_expression
mul_level_binary_expression = mul
                            | div
                            | rem
                            | bit_and
                            | bit_shl
                            | bit_shr
                            | bit_sar
                            | prefix_expression

add     = add_level_binary_expression T_Plus mul_level_binary_expression
sub     = add_level_binary_expression T_Minus mul_level_binary_expression
bit_or  = add_level_binary_expression T_Or mul_level_binary_expression
bit_xor = add_level_binary_expression T_Tilde mul_level_binary_expression
add_level_binary_expression = add
                            | sub
                            | bit_or
                            | bit_xor
                            | mul_level_binary_expression

cmp_equal     = add_level_binary_expression T_EqualEquals add_level_binary_expression
cmp_noteq     = add_level_binary_expression T_BangEQ add_level_binary_expression
cmp_less      = add_level_binary_expression T_Less add_level_binary_expression
cmp_lesseq    = add_level_binary_expression T_LessEQ add_level_binary_expression
cmp_greater   = add_level_binary_expression T_Greater add_level_binary_expression
cmp_greatereq = add_level_binary_expression T_GreaterEQ add_level_binary_expression
cmp_level_binary_expression = cmp_equal
                            | cmp_noteq
                            | cmp_less
                            | cmp_lesseq
                            | cmp_greater
                            | cmp_greatereq
                            | add_level_binary_expression

and = and_level_binary_expression T_AndAnd cmp_level_binary_expression
and_level_binary_expression = and
                            | cmp_level_binary_expression

or = or_level_binary_expression T_OrOr and_level_binary_expression
or_level_binary_expression = or
                           | and_level_binary_expression

expression = or_level_binary_expression

var          = expression {T_Comma expression} T_Colon expression
             | expression {T_Comma expression} T_Colon [expression] T_Equals (T_TpMinus | expression {T_Comma expression})
const        = expression {T_Comma expression} T_Colon [expression] T_Colon expression {T_Comma expression})
const_nosemi = expression {T_Comma expression} T_Colon [expression] T_Colon (proc_lit | struct)
when         = T_When T_OpenParen expression T_CloseParen statement [T_Else statement]
declaration = var
            | const
            | const_nosemi
            | when
declaration_wsemi = when          
			     | var T_Semicolon
                 | const T_Semicolon
                 | const_nosemi
            
block      = [T_Colon identifier] T_OpenBrace {statement} T_CloseBrace
if         = [T_Colon identifier] T_If T_OpenParen [(declaration | assignment | expression) T_Semicolon] expression T_CloseParen statement [T_Else statement]
while      = [T_Colon identifier] T_While T_OpenParen expression T_CloseParen statement
           | T_While T_OpenParen (declaration | assignment | expression) T_Semicolon expression [T_Semicolon (assignment | expression)] T_CloseParen statement
break      = T_Break [identifier]
continue   = T_Continue [identifier]
return     = T_Return argument_list
H_assignment_token = T_StarEQ
                   | T_SlashEQ
                   | T_PercentEQ
                   | T_AndEQ
                   | T_ShlEQ
                   | T_ShrEQ
                   | T_SarEQ
                   | T_PlusEQ
                   | T_MinusEQ
                   | T_OrEQ
                   | T_TildeEQ
                   | T_AndAndEQ
                   | T_OrOrEQ       
assignment = expression {T_Comma expression} H_assignment_token expression {T_Comma expression}
statement = block
          | if
          | while
          | break T_Semicolon
          | continue T_Semicolon
          | return T_Semicolon
          | assignment T_Semicolon
          | expression T_Semicolon
	      | declaration_wsemi

program = var T_Semicolon
        | const T_Semicolon
        | const_nosemi
```