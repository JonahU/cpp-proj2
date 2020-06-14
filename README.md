# MPCS 51045 ADVANCED C++ FINAL PROJECT - Jonah Usadi

## Project overview

This program takes a c++ header file, parses it and outputs corresponding .cpp & .py files.
I was interested in the topic of bridging between higher level languages and c++. The idea is
that the programmer defines all their logic within the header file and then uses this utility
to generate the "glue" between python and c++. The "glue" in this case is boost python. The parser
supports basic aggregate types as well as function declarations. Function definition support has
also been added although it isn't perfect (see `Known bugs` section for more info). I also added
support for std::vector and std::map.

Please see the `examples` folder for some examples of what this program can handle.

### Supported/ not supported (non exhaustive)
- basic aggregate support (no class keyword/ constructor/ destructor/ member functions)
- std::vector, std::map, std::string
- modifiers: const, pointers, l-value references, unsigned
- a full list of supported keywords/symbols can be found in the tokenizer regexes
- nested templates, nested structs, default values are not supported
- function overloading is not supported
- forward declarations are not supported

### My testing environment
- gcc 9.3.0 and c++17
- python 3.6 (any python3 version should be fine)
- Note: ctre.hpp wouldn't compile with my version of clang, I haven't tested other compilers

### Known bugs
- const-container vs container-const bug, see `examples/extra/const_container_bug.h` for an example
- container + function definition bug, see `examples/extra/container_func_def_bug.h` for an example
- name mangling in `cpptopy.h` isn't perfect:    
    - don't name your struct "string" or end name with unsigned e.g. "mytype_unsigned"
    - see `mangle_modifiers()` for more info
- inside function definition check in tokenizer implemented in a naive way
    - tokenizer doesn't understand string literals/ comments so adding extra '{' & '}' symbols can be problematic
- newline at end of header file is required or weird things happen

### Improvements I would like to make
- versioned namespaces
- enable_if in parser
- better parser error handling/ messages

### Features that I didn't have time to implement
- std::tuple (partially implemented but not working properly)
- nested templates (removed because of complexity)
- nested structs (removed because of complexity)
- default values
- comments support (// + /*)
- --emit-tokens flag
- --emit-ast flag

## Additional comments

NOTE: I'm running out of time, this project is due in 5 minutes so I don't have time to format all of these extra notes I made while working on the project.

-implemented tokenizer (visitor patter) before duck typing lecture, implemented parser (variant + visit) after duck typing lecture
-multiple variants for lookup in parser: https://www.bfilipek.com/2018/09/visit-variants.html
-no nested structs
-no nested templates (started implementing but decided to prioritize other things)

-half baked multi dispatch
-1 pass parser (no backtracking)

-stack for nodes under construction not ideal given current abstractions
-e.g. parsing a function:

      [STACK TOP]
    <variable-node> // the 2nd parameter              
    <variable-node> // the 1st parameter
    <function-node> 
    <variable-node> // the return value (parser must assume this a global variable, doesn't know it's a type until it sees left paren)
       [BOTTOM]

    -We so far have processed int func(char one, long two)
    -Next symbol could be either ';' or '{'
    -I want to mark the function-node as either having a definition or not
    -Because I used a stack I can't directly access the function-node

    -Getting the desired results means creating a special pop_node_function
    -I should have thought more carefully about these issues...

-biggest source of bugs in the parser
-hardest to reason about, tracking lexer token, scope + ast node in a not very coherant way
-if i was designing from scratch would use std::visit and multi dispatch everywhere
-there isn't much error handling in the parser, would like to add better error messages with more time
-tried to implement a "event"-based style in the parser which sort of works but probably isn't worth its cost given the current implementation
-the basic idea is:
    -set current lexer token to current_token
    -std::visit(current_ast_node, current_token) is called everytime the parser processes a new token
    -this model is nice because I never explicitly construct an ast node then check if for the "const" keyword
    -I just construct a new node and std::visit is called on every new token
    -the idea was it would be better to split up all the complicated logic when constructing ast nodes
    -although a nice idea, there is marginal utility given the current implementation (some issues relate to the stack problem above)
    -it's useful for variable modifiers but that's about it...

-would be nice to go all out on multiple dispatch in the parser + encorporate current_scope which would simplify the code in parser_token_visitor

-better error msgs in parser
-at the moment you can get a bad variant access, segmentation fault (reading the top element of an empty std::stack) or if you're lucky a runtime exception with a slightly more useful error message

RULES:
    SUPPORTED:
        - (inline) variables
        - aggregate (is this POD?)
        - supported keywords: int, double, std::vector etc...
        - supported modifiers *, &, const, unsigned
    NOT ALLOWED:
        - no const struct members
        - no pointer-to-pointer
        - no r-value references
        - no namespaces
        - no comments
        - no function overloading (there isn't overloading in python)
        - no nested template
        - no nested struct
        - newline required at end of header file or weird things happen
        - no extraneous parentheses
        - no operator functions
        - no tuple
        - no forward declarations
        - only 1 function declaration/definition
        - should be valid c++, some things may not get caught by the lexer/parser doesn't mean the generated code will work...
    BUGS:
        - name mangling in cpptopy isn't perfect:
        - don't name your struct "string" or end it with unsigned e.g. "mytype_unsigned"
        - no curly braces within string literals or comments in a function definition
        - put const on the right, lhs const specifically for containers is broken (see const_container.h example)
