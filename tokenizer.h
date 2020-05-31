#pragma once

#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
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

std::unique_ptr<std::vector<base_token>> tokenize(std::string const& path_to_file) {
    std::ifstream ifs(path_to_file);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file '" << path_to_file << "'\n";
        return nullptr;
    }

    std::unique_ptr<std::vector<base_token>> my_tokens;
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

        // std::getline(ifs, line);
        // std::istringstream iss(line);
        // while (iss) {
        //     std::string token;
        //     iss >> token;
        //     if (token.length() > 1 || token != "") {
        //         std::cout << '\'' <<  token << "'\n";
        //     }
        // }
    }
    ifs.close();
    return nullptr;
}

}