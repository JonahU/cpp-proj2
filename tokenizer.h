#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include "ctre.hpp"

namespace proj2 {

constexpr auto identifier_regex = ctll::fixed_string{"^[a-zA-Z_]+\\w*$"};
constexpr auto headerfile_regex = ctll::fixed_string{"^[/\\w_]+\\.(?:h|hh|hpp|hxx|h\\+\\+)$"};
constexpr auto isspace_regex    = ctll::fixed_string{"^\\s$"};
constexpr auto keyword_regex    = ctll::fixed_string{"^(struct)|(inline)|(include)|(int)|(long)|(short)|(double)|(float)|(char)|(void)|(std::string)|(std::vector)|(std::map)|(std::tuple)|(unsigned)|(const)$"};
constexpr auto symbol_regex     = ctll::fixed_string{"^(\")|(,)|(\\()|(\\))|(\\{)|(\\})|(;)|(#)|(<)|(>)|(\\*)|(&)$"}; // " , ( ) { } ; # < > * &

constexpr auto match_identifier (std::string_view sv) noexcept { return ctre::match<identifier_regex>(sv) || ctre::match<headerfile_regex>(sv); }
constexpr auto match_isspace    (std::string_view sv) noexcept { return ctre::match<isspace_regex>(sv); } 
constexpr auto match_keyword    (std::string_view sv) noexcept { return ctre::match<keyword_regex>(sv); }
constexpr auto match_symbol     (std::string_view sv) noexcept { return ctre::match<symbol_regex>(sv); }

enum class container_t  { c_unknown, c_vector, c_map, c_tuple };
enum class keyword_t    { k_unknown, k_struct, k_inline, k_include };
enum class modifier_t   { m_unknown, m_const, m_ptr, m_ref, m_unsigned };
enum class symbol_t     { s_unknown, s_quot, s_comma, s_lpar, s_rpar, s_lcub, s_rcub, s_semi, s_pound, s_lt, s_gt};
enum class type_t       { t_unknown, t_custom, t_int, t_long, t_short, t_double, t_float, t_char, t_void, t_string };

// forward declarations
struct container_token;
struct identifier_token;
struct keyword_token;
struct modifier_token;
struct symbol_token;
struct type_token;

struct token_visitor_base {
    virtual void visit(container_token  const& token) const = 0;
    virtual void visit(identifier_token const& token) const = 0;
    virtual void visit(keyword_token    const& token) const = 0;
    virtual void visit(modifier_token   const& token) const = 0;
    virtual void visit(symbol_token     const& token) const = 0;
    virtual void visit(type_token       const& token) const = 0;
};

struct base_token {
    const std::string value;
    base_token(std::string&& _value) : value(std::move(_value)) {}
    base_token(char _value)          : value{_value}            {}
    virtual ~base_token() = default;
    virtual void accept(token_visitor_base const& tv) = 0;
};

struct container_token : base_token {
    const container_t type;
    container_token(std::string&& _value, container_t _type) : base_token(std::move(_value)), type(_type) {}
    virtual void accept(token_visitor_base const& tv) override { tv.visit(*this); }
};

struct identifier_token : base_token {
    identifier_token(std::string&& _value) : base_token(std::move(_value)) {}
    virtual void accept(token_visitor_base const& tv) override { tv.visit(*this); }
};

struct keyword_token : base_token {
    const keyword_t type;
    keyword_token(std::string&& _value, keyword_t _type) : base_token(std::move(_value)), type(_type) {}
    virtual void accept(token_visitor_base const& tv) override { tv.visit(*this); }
};

struct modifier_token : base_token {
    const modifier_t type;
    modifier_token(std::string&& _value, modifier_t _type) : base_token(std::move(_value)), type(_type) {}
    modifier_token(char          _value, modifier_t _type) : base_token(_value           ), type(_type) {}
    virtual void accept(token_visitor_base const& tv) override { tv.visit(*this); }
};

struct symbol_token : base_token {
    const symbol_t type;
    symbol_token(std::string&& _value, symbol_t _type) : base_token(std::move(_value)), type(_type) {}
    symbol_token(char _value         , symbol_t _type) : base_token(_value           ), type(_type) {}
    virtual void accept(token_visitor_base const& tv) override { tv.visit(*this); }
};

struct type_token : base_token {
    const type_t type;
    type_token(std::string&& _value, type_t _type) : base_token(std::move(_value)), type(_type) {}
    virtual void accept(token_visitor_base const& tv) override { tv.visit(*this); }
};

using token_list = std::vector<std::unique_ptr<base_token>>;

inline void token_list_push_container(std::unique_ptr<token_list>& my_tokens, std::string& token, container_t const c_type) {
    std::cout << "container " << token << "\n";
    my_tokens->emplace_back(std::make_unique<container_token>(std::move(token), c_type));
}

inline void token_list_push_identifier(std::unique_ptr<token_list>& my_tokens, std::string& token) {
    std::cout << "identifier " << token << std::endl;
    my_tokens->emplace_back(std::make_unique<identifier_token>(std::move(token)));
}

inline void token_list_push_keyword(std::unique_ptr<token_list>& my_tokens, std::string& token, keyword_t const k_type) {
    std::cout << "keyword " << token << "\n";
    my_tokens->emplace_back(std::make_unique<keyword_token>(std::move(token), k_type));
}

inline void token_list_push_modifier(std::unique_ptr<token_list>& my_tokens, std::string& token, modifier_t const m_type) {
    std::cout << "modifier " << token << "\n";
    my_tokens->emplace_back(std::make_unique<modifier_token>(std::move(token), m_type));
}

inline void token_list_push_modifier(std::unique_ptr<token_list>& my_tokens, char token, modifier_t const m_type) {
    std::cout << "modifier " << token << "\n";
    my_tokens->emplace_back(std::make_unique<modifier_token>(token, m_type));
}

inline void token_list_push_symbol(std::unique_ptr<token_list>& my_tokens, char token, symbol_t const s_type) {
    std::cout << "symbol " << token << "\n";
    my_tokens->emplace_back(std::make_unique<symbol_token>(token, s_type));
}

inline void token_list_push_type(std::unique_ptr<token_list>& my_tokens, std::string& token, type_t const t_type) {
    std::cout << "type " << token << "\n";
    my_tokens->emplace_back(std::make_unique<type_token>(std::move(token), t_type));
}

template <typename... Captures>
inline void fill_token_list_which_keyword(std::unique_ptr<token_list>& my_tokens, std::string& token, ctre::regex_results<Captures...> const& regex_matches, bool& next_token_is_typename) {
    // Note: c++ 20's string literal operator template + ctre named captures should make this code cleaner and safer
    auto [ _,
        is_struct,    // keywords
        is_inline,
        is_include,
        is_int,       // types
        is_long,
        is_short,
        is_double,
        is_float,
        is_char,
        is_void_,
        is_string,
        is_vector,    // containers
        is_map,
        is_tuple,
        is_unsigned_, // modifiers
        is_const_ 
        ] = regex_matches;
    if (is_struct) {
        token_list_push_keyword(my_tokens, token, keyword_t::k_struct);
        next_token_is_typename = true;
    } else if (is_inline) {
        token_list_push_keyword(my_tokens, token, keyword_t::k_inline);
    } else if (is_include) {
        token_list_push_keyword(my_tokens, token, keyword_t::k_include);
    } else if (is_int) {
        token_list_push_type(my_tokens, token, type_t::t_int);
    } else if (is_long) {
        token_list_push_type(my_tokens, token, type_t::t_long);
    } else if (is_short) {
        token_list_push_type(my_tokens, token, type_t::t_short);
    } else if (is_double) {
        token_list_push_type(my_tokens, token, type_t::t_double);
    } else if (is_float) {
        token_list_push_type(my_tokens, token, type_t::t_float);
    } else if (is_char) {
        token_list_push_type(my_tokens, token, type_t::t_char);
    } else if (is_void_) {
        token_list_push_type(my_tokens, token, type_t::t_void);
    } else if (is_string) {
        token_list_push_type(my_tokens, token, type_t::t_string);
    } else if (is_vector) {
        token_list_push_container(my_tokens, token, container_t::c_vector);
    } else if (is_map) {
        token_list_push_container(my_tokens, token, container_t::c_map);
    } else if (is_tuple) {
        token_list_push_container(my_tokens, token, container_t::c_tuple);
    } else if (is_unsigned_) {
        token_list_push_modifier(my_tokens, token, modifier_t::m_unsigned);
    } else if (is_const_) {
        token_list_push_modifier(my_tokens, token, modifier_t::m_const);
    } else {
        std::cerr << "invalid keyword: '" << token << "'\n"; 
        throw std::invalid_argument("tokenizer: invalid keyword"); 
    }
}

template <typename... Captures>
inline void fill_token_list_which_symbol(std::unique_ptr<token_list>& my_tokens,
                                         char symbol,
                                         ctre::regex_results<Captures...> const& regex_matches,
                                         bool& inside_function_def,
                                         int& scope_depth) {
    static bool end_of_function_decl = false;
    auto [ _,
        is_quot,
        is_comma,
        is_lpar,
        is_rpar,
        is_lcub,
        is_rcub,
        is_semi,
        is_pound,
        is_lt,
        is_gt,
        is_ptr,
        is_ref 
        ] = regex_matches;
    if (is_quot) {
        token_list_push_symbol(my_tokens, symbol, symbol_t::s_quot);
    } else if (is_comma) {
        token_list_push_symbol(my_tokens, symbol, symbol_t::s_comma);
    } else if (is_lpar) {
        token_list_push_symbol(my_tokens, symbol, symbol_t::s_lpar);
    } else if (is_rpar) {
        token_list_push_symbol(my_tokens, symbol, symbol_t::s_rpar);
        end_of_function_decl = true;
    } else if (is_lcub) {
        token_list_push_symbol(my_tokens, symbol, symbol_t::s_lcub);
        if (end_of_function_decl) { inside_function_def = true; ++scope_depth; }
    } else if (is_rcub) {
        token_list_push_symbol(my_tokens, symbol, symbol_t::s_rcub);
        end_of_function_decl = false;
    } else if (is_semi) {
        token_list_push_symbol(my_tokens, symbol, symbol_t::s_semi);
        end_of_function_decl = false;
    } else if (is_pound) {
        token_list_push_symbol(my_tokens, symbol, symbol_t::s_pound);
    } else if (is_lt) {
        token_list_push_symbol(my_tokens, symbol, symbol_t::s_lt);
    } else if (is_gt) {
        token_list_push_symbol(my_tokens, symbol, symbol_t::s_gt);
    } else if (is_ptr) {
        token_list_push_modifier(my_tokens, symbol, modifier_t::m_ptr);
    } else if (is_ref) {
        token_list_push_modifier(my_tokens, symbol, modifier_t::m_ref);
    } else {
        std::cerr << "invalid symbol: '" << symbol << "'\n"; 
        throw std::invalid_argument("tokenizer: invalid symbol"); 
    }
}

inline void fill_token_list_keyword_or_identifier(std::unique_ptr<token_list>& my_tokens, std::string& token, std::set<std::string, std::less<>>& new_types) {
    static bool next_token_is_typename = false;
    if (!token.empty()) {
        if (auto keyword_match = match_keyword(token)) {
            fill_token_list_which_keyword(my_tokens, token, keyword_match, next_token_is_typename);
        } else if (new_types.find(token) != new_types.end()) {
            token_list_push_type(my_tokens, token, type_t::t_custom);
        } else if (match_identifier(token)) {
            if (next_token_is_typename) {
                new_types.insert(token);
                token_list_push_type(my_tokens, token, type_t::t_custom);
                next_token_is_typename = false;
            } else {
                token_list_push_identifier(my_tokens, token);
            }
        } else {
            std::cerr << "invalid token: '" << token << "'\n"; 
            throw std::invalid_argument("tokenizer: invalid token"); 
        }
        token.clear();
    }
}

inline std::unique_ptr<token_list> tokenize(std::ifstream& ifs) {
    std::unique_ptr<token_list> my_tokens = std::make_unique<token_list>();
    std::set<std::string, std::less<>> new_types;

    char next_ch_str[] = { '\0', '\0' }; // regex match wants a string
    char& next_ch = next_ch_str[0]; // but ifstream.get() wants a char
    std::string token;

    bool inside_function_def = false;
    int scope_depth = 0; // track open & close curly braces within a function

    while(!ifs.eof()) {
        ifs.get(next_ch);
        if (inside_function_def) { // ignore whatever is inside the function definition
            if      (next_ch == '{'  ) ++scope_depth;
            else if (next_ch == '}'  ) --scope_depth;
            if      (scope_depth != 0)      continue;
        }

        if (match_isspace(next_ch_str)) {
            fill_token_list_keyword_or_identifier(my_tokens, token, new_types);
        } else if (auto symbol_match = match_symbol(next_ch_str)) {
            fill_token_list_keyword_or_identifier(my_tokens, token, new_types);
            fill_token_list_which_symbol(my_tokens, next_ch, symbol_match, inside_function_def, scope_depth);
        } else {
            token+=next_ch;
        }
    }
    ifs.close();
    return my_tokens;
}

}