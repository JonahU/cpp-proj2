#include <iostream>
#include "cpptopy.h"
#include "parsefile.h"
#include "tokenizer.h"

using namespace std;
using namespace proj2;

/* TODO:
    - [not possible?] convert parser visit token.type to new variant idiom (move logic from parser_token_visitor to parser_ast_visitor)

    - clean up code (particularly parser) + remove logging
    - add nice example + clean up examples + boostpython directories
    - README
    - tokenizer func def ?
    - replace pragma onces with IFNDEF
    - enable if templates in parser ?
    - versioned namespaces?

    - fno-exceptions?

    - comments support ? // + /*
    - default values ?
    - string literal ?
    - char literal ? (remove from tokenizer)

    - --emit-tokens flag ?
    - --emit-ast ?

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
            - newline required at end of file
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
*/

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "usage: " << argv[0] <<" <path-to-header-file>\n";
        return 1;
    }

    ifstream ifs(argv[1]);
    if (!ifs.is_open()) {
        cerr << "Failed to open file '" << argv[1] << "'\n";
        return 1;
    }

    auto tokens = tokenize(ifs);
    auto parsed = parse(tokens); 
    cpptopy(argv[1], parsed);

    return 0;
}