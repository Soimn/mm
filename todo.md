* due to the "parse everything before allocating" scheme all pointers to sub nodes are back pointers, this might become a performance problem later down the line
* forward decl expressions seems weird, maybe change them?
* maybe reconsider the whole :proc: --- and :: proc --- thing

* todo for next devlog
- changes to operator precedence
- removal of .(), cast is now a builtin
- adding aliasing to using
- proc() -> () is illegal
- loads more...