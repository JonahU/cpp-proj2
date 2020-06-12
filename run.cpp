#include <iostream>
#include "cpptopy.h"
#include "parsefile.h"
#include "tokenizer.h"

using namespace std;
using namespace proj2;

/* TODO:
    - c++ codegen
    - python codegen

    - newline required at bottom of input header file (bug?)

    - [not possible?] convert parser visit token.type to new variant idiom (move logic from parser_token_visitor to parser_ast_visitor)

    - clean up code (particularly parser) + remove logging
    - enable if templates in parser ?
    - versioned namespaces?

    - fno-exceptions?
    - comments support ? // + /*
    - tokenizer func def ?
    - default values ?
    - string literal ?
    - char literal ?

    - --emit-tokens flag ?
    - --emit-ast ?

    RULES:
    - no overloading
    - no nested template
    - no nested struct
    - aggregate, no const (is this POD?)
    - newline at end of file
    - no extraneous parentheses
    - no operator functions
    - supported keywords: int, double, vector etc...
    - no tuple
    - no forward declarations
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