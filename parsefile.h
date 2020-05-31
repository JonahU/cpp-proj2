#pragma once

#include <memory>
#include <string_view>
#include "tokenizer.h"

namespace proj2 { 

struct parser {

};

std::unique_ptr<parser> parsefile(std::string const& path_to_file) {
    auto tokens = tokenize(path_to_file);
    return std::make_unique<parser>();
}

}