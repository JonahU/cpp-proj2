#include <iostream>
#include "parsefile.h"

using namespace std;
using namespace proj2;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "usage: " << argv[0] <<" <path-to-file>\n";
        return 1;
    }
    
    parsefile(argv[1]);
}