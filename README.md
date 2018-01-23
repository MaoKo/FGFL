FGFL (Fast Generate, Fast Lexer) and FGFP (Fast Generate, Fast Parser)
are scientific tools.
The purpose of FGFL is to create from lexical specification (example/lex/ folder)
a DFA ready for lexical analysis.
The goal of FGFP is to create from a grammar description a LALR(1) parsing table.
Also for speed, maybe, I replace the Vector with Hash.
For constructing the LALR parsing table, I use the step-by-step approach, which
whenever I found similar LR(0) state, I merge them.
But I prefer do that, if the user want to conserve the LR(1) table.

### How to make? ###
Git clone the repository.
Then, type: make

After you can do: ./efgfl example/lex/token_c.lex
Which create a file called token_c.h.lex ready for lexing.

### TODO ###
    .   More accurate handle of error in FGFX (FGFP (95%), FGFL(80%))
    .   Optimizing FOLLOW set
    .   Handle if file already exist
    .   Improve the algorithm for constructing the
            LALR parsing table
            DFA transition table

    .   Add error recovery mechanism
    .   Add in FGFP ( ... | ... )
    .   Add $FAIL to FGFL
    .   Add ! not operator to regex (25%)
    .   Add option (?r-s:pattern) to FGFL
    .   Handle a/b*c lookahead
    .   Remove unreachable state
    .   Handle when the number of state is 1
    .   Check useless $BEGIN
    .   Learn MarkDown :)

