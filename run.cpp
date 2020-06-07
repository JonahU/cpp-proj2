#include <iostream>
#include "parsefile.h"
#include "tokenizer.h"

using namespace std;
using namespace proj2;

/* TODO:
    - parser
    - boost python (or cpython API?)
    - c++ codegen
    - python codegen
    - parser multiple variants for std::visit lookup https://www.bfilipek.com/2018/09/visit-variants.html ?
    - tokenizer string literal (for preprocessor #include)
    - modifier inline?
    - tokenizer func def ?
    - default values ?
    - string literal ?
    - char literal ?
    - --emit-tokens flag ?
    - --emit-ast ?
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
}