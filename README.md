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
- basic aggregate support (i.e. no class keyword/ constructor/ destructor/ member functions)
- supported types: int, long, short, double, float, char, void, std::string
- supported containers: std::vector, std::map
- supported modifiers: const, pointers, l-value references, unsigned
- supported keywords: struct, inline, include
- inline variables are supported
- nested templates, nested structs, default values are not supported
- function overloading is not supported
- forward declarations are not supported
- pointer to pointer not supported

### My testing environment
- gcc 9.3.0 and c++17
- boost 1.73.0
- compile time regular expressions v2 
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
- unsigned keyword is only supported as a modifier, if type is required must use unsigned int

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
