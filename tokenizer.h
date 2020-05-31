#pragma once

#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace proj2 {


struct base_token {
    std::string value;
};

struct keyword_token : base_token {
};

struct symbol_token : base_token {
};

struct identifier_token : base_token {
};

using token_list = std::vector<base_token>;

std::unique_ptr<token_list> tokenize(std::ifstream& ifs) {
    std::unique_ptr<token_list> my_tokens;
    char c;
    std::string token;
    while(!ifs.eof()) {
        ifs.get(c);
        if (std::isspace(c)) {
            if (!token.empty()) {
                std::cout << token << std::endl;
                token.clear();
            }
        } else if (c == ';') {
            std::cout << token << std::endl;
            token.clear();
            // token+=c;
            // std::cout << token << std::endl;
            // token.clear();
            std::cout << c << std::endl;
        } else {
            token+=c;
        }
    }
    ifs.close();
    return nullptr;
}

}