#pragma once

#include <cctype>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

namespace proj2 {

std::regex const keyword_regex      ("^(struct|int|double|long|char|void)$");
std::regex const symbol_regex       ("^(\"|'|\\(|\\)|\\{|\\}|;)$"); // " ' ( ) { } ;
std::regex const identifier_regex   ("^([a-zA-Z_]+\\w*)$");

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
    char c_str[] = { '\0', '\0' };
    std::string token;
    token.reserve(30);
    while(!ifs.eof()) {
        ifs.get(c);
        c_str[0] = c;
        if (std::isspace(c)) {
            // TODO: clean up this duplicated code
            if (!token.empty()) {
                if (std::regex_match(token, keyword_regex)) {
                    std::cout << "keyword " << token << std::endl;
                } else if (std::regex_match(token, identifier_regex)) {
                    std::cout << "identifier " << token << std::endl;
                } else {
                    throw std::invalid_argument("invalid token"); 
                }
                token.clear();
            }
        } else if (std::regex_match(c_str, symbol_regex)) {
            if (!token.empty()) {
                if (std::regex_match(token, keyword_regex)) {
                    std::cout << "keyword " << token << std::endl;
                } else if (std::regex_match(token, identifier_regex)) {
                    std::cout << "identifier " << token << std::endl;
                } else {
                    throw std::invalid_argument("invalid token"); 
                }
                token.clear();
            }
            std::cout << "symbol " << c << "\n";
        } else {
            token+=c;
        }
    }
    ifs.close();
    return nullptr;
}

}