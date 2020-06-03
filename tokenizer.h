#pragma once

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "ctre.hpp"

namespace proj2 {

constexpr auto isspace_regex    = ctll::fixed_string{"^\\s$"};
constexpr auto keyword_regex    = ctll::fixed_string{"^(int|long|short|double|float|char|void|std::string)|(std::vector|std::map)|inline|include|(struct)|(unsigned)|(const)$"};
constexpr auto symbol_regex     = ctll::fixed_string{"^\"|'|,|\\(|\\)|\\{|\\}|;|#|<|>|(\\*)|(&)$"};
constexpr auto identifier_regex = ctll::fixed_string{"^[a-zA-Z_]+\\w*$"};

constexpr auto match_isspace(std::string_view sv) noexcept {
    return ctre::match<isspace_regex>(sv);
} 

constexpr auto match_keyword(std::string_view sv) noexcept {
    return ctre::match<keyword_regex>(sv);
}

constexpr auto match_symbol(std::string_view sv) noexcept {
	return ctre::match<symbol_regex>(sv);
}

constexpr auto match_identifier(std::string_view sv) noexcept {
    return ctre::match<identifier_regex>(sv);
}

enum class modifier_t {
    m_none, m_const, m_ptr, m_ref, m_unsigned
};

enum class type_t {
    t_none, t_int, t_long, t_short, t_double, t_float, t_char, t_void, t_string
};

enum class container_t {
    c_none, c_vector, c_map
};

enum class keyword_t {
    k_none, k_struct, k_inline, k_include
};

struct base_token {
    std::string value;
    base_token(std::string&& _value) : value(_value) {}
    base_token(char _value         ) : value{_value} {}
};

struct keyword_token : base_token {
    keyword_token(std::string&& _value) : base_token(std::move(_value)) {}
};

struct symbol_token : base_token {
    symbol_token(std::string&& _value) : base_token(std::move(_value)) {}
    symbol_token(char _value         ) : base_token(_value           ) {}
};

struct identifier_token : base_token {
    identifier_token(std::string&& _value) : base_token(std::move(_value)) {}
};

struct modifier_token : base_token {
    modifier_t m_type;
    modifier_token(std::string&& _value, modifier_t _m_type = modifier_t::m_none) : base_token(std::move(_value)), m_type(_m_type) {}
    modifier_token(char          _value, modifier_t _m_type = modifier_t::m_none) : base_token(_value           ), m_type(_m_type) {}
};

using token_list = std::vector<std::unique_ptr<base_token>>;

void fill_token_list(std::unique_ptr<token_list>& my_tokens, std::string& token, modifier_t m_type) {
    std::cout << "modifier " << token << "\n";
    my_tokens->emplace_back(std::make_unique<modifier_token>(std::move(token), m_type));
}

void fill_token_list(std::unique_ptr<token_list>& my_tokens, char token, modifier_t m_type) {
    std::cout << "modifier " << token << "\n";
    my_tokens->emplace_back(std::make_unique<modifier_token>(token, m_type));
}

// TODO: simple state machine, make set of new types, add to set if identifier follows struct keyword 
void fill_token_list(std::unique_ptr<token_list>& my_tokens, std::string& token) {
    if (!token.empty()) {
        if (auto [_, is_type, is_container, is_struct, is_unsigned, is_const] = match_keyword(token); _) {
            if (is_unsigned) {
                fill_token_list(my_tokens, token, modifier_t::m_unsigned);
            } else if (is_const) {
                fill_token_list(my_tokens, token, modifier_t::m_const);
            } else {
                std::cout << "keyword " << token << std::endl;
                my_tokens->emplace_back(std::make_unique<keyword_token>(std::move(token)));
            }
        } else if (match_identifier(token)) {
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
        if (match_isspace(next_ch_str)) {
            fill_token_list(my_tokens, token);
        } else if (auto [_, is_ptr, is_ref] = match_symbol(next_ch_str); _) {
            fill_token_list(my_tokens, token);
            if (is_ptr) {
                fill_token_list(my_tokens, next_ch, modifier_t::m_ptr);
            } else if (is_ref) {
                fill_token_list(my_tokens, next_ch, modifier_t::m_ref);
            } else {
                fill_token_list(my_tokens, next_ch);
            }
        } else {
            token+=next_ch;
        }
    }
    ifs.close();
    return my_tokens;
}

}