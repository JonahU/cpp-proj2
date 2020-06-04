#pragma once

#include <memory>
#include <string_view>
#include "tokenizer.h"

namespace proj2 { 

struct parser {
    std::unique_ptr<token_list> const& tokens;

    parser(std::unique_ptr<token_list> const& _tokens) : tokens(_tokens) {
    }
};

inline std::unique_ptr<parser> parsefile(std::ifstream& ifs) {
    auto my_tokens = tokenize(ifs);
    auto my_parser = std::make_unique<parser>(my_tokens);
    return my_parser;
}

}