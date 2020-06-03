#pragma once

#include <cctype>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace proj2 {

std::regex const keyword_regex      ("^struct|int|double|long|char|void$");
std::regex const symbol_regex       ("^\"|'|\\(|\\)|\\{|\\}|;$"); // " ' ( ) { } ;
std::regex const identifier_regex   ("^[a-zA-Z_]+\\w*$");

struct base_token {
    std::string value;
    base_token(std::string&& _value) : value(_value) {}
    base_token(char _value)          : value{_value} {}
};

struct keyword_token : base_token {
    keyword_token(std::string&& _value) : base_token(std::move(_value)) {}
};

struct symbol_token : base_token {
    symbol_token(std::string&& _value) : base_token(std::move(_value)) {}
    symbol_token(char _value)          : base_token(_value)            {}
};

struct identifier_token : base_token {
    identifier_token(std::string&& _value) : base_token(std::move(_value)) {}
};

using token_list = std::vector<std::unique_ptr<base_token>>;

void fill_token_list(std::unique_ptr<token_list>& my_tokens, std::string& token) {
    if (!token.empty()) {
        if (std::regex_match(token, keyword_regex)) {
            std::cout << "keyword " << token << std::endl;
            my_tokens->emplace_back(std::make_unique<keyword_token>(std::move(token)));
        } else if (std::regex_match(token, identifier_regex)) {
            std::cout << "identifier " << token << std::endl;
            my_tokens->emplace_back(std::make_unique<identifier_token>(std::move(token)));
        } else {
            std::cerr << "invalid token: '" << token << "'\n"; 
            throw std::invalid_argument("invalid token"); 
        }
        token.clear();
    }
}

void fill_token_list(std::unique_ptr<token_list>& my_tokens, char token) {
    std::cout << "symbol " << token << "\n";
    my_tokens->emplace_back(std::make_unique<symbol_token>(token));
}

std::unique_ptr<token_list> tokenize(std::ifstream& ifs) {
    std::unique_ptr<token_list> my_tokens = std::make_unique<token_list>();
    char next_ch_str[] = { '\0', '\0' };
    char& next_ch = next_ch_str[0];
    std::string token;
    token.reserve(30);
    while(!ifs.eof()) {
        ifs.get(next_ch);
        if (std::isspace(next_ch)) {
            fill_token_list(my_tokens, token);
        } else if (std::regex_match(next_ch_str, symbol_regex)) {
            fill_token_list(my_tokens, token);
            fill_token_list(my_tokens, next_ch);
        } else {
            token+=next_ch;
        }
    }
    ifs.close();
    return my_tokens;
}

}