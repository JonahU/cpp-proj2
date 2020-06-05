#pragma once

#include <iostream>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string_view>
#include <variant>
#include "tokenizer.h"

namespace proj2 {

struct parser_include {
    const bool is_sys_header; // <header> vs "header"
    std::string_view name;
};

struct parser_basic_type {
    const type_t     type;
    const bool       mod_const;
    const bool       mod_ptr;
    const bool       mod_ref;
    const bool       mod_unsigned;
    std::string_view custom_typename;
};

struct parser_template_type {
    const parser_basic_type                          type;
    std::unique_ptr<std::vector<parser_basic_type>>  types;
    const container_t                                container_type;
    std::string_view                                 custom_typename;
    std::unique_ptr<parser_template_type>            next;
};

struct parser_basic_variable {
    const parser_basic_type type;
    std::string_view        name;
};

struct parser_container {
    const parser_template_type type;
    std::string_view           name;
};

using parser_variable = std::variant<parser_basic_variable, parser_container>;

struct parser_struct {
    const std::vector<parser_variable> members;
    std::string_view                   name;
};
// Note: add support for nested structs?

struct parser_function {
    const std::vector<parser_variable> params;
    std::string_view                   name;
};

enum class parser_scope {
    unknown,
    scope_global,
    scope_preprocessor,
    scope_stringlit,
    scope_variable,
    scope_struct,
    scope_function, // Note: separate into func def + func decl ?
    scope_template
};

struct parser {

// Note: parser_token_visitor is here because of forward declaration problems 
struct parser_token_visitor : token_visitor_base {
    parser& my_parser;
    parser_token_visitor(parser& _my_parser) : my_parser(_my_parser) {}

    void visit(container_token const& token) const override {
        // TODO: implement properly
        my_parser += parser_scope::scope_variable;
        std::cout << "visiting container: " << token.value << "\n";
    }

    void visit(identifier_token const& token) const override {
        std::cout << "visiting identifier: " << token.value << "\n";
    }

    void visit(keyword_token const& token) const override {
        std::cout << "visiting keyword: " << token.value << "\n";
    }

    void visit(modifier_token const& token) const override {
        std::cout << "visiting modifier: " << token.value << "\n";
    }

    void visit(symbol_token const& token) const override {
        std::cout << "visiting symbol: " << token.value << "\n";
        switch (token.type) {
            case symbol_t::s_quot:
                if (my_parser.current_scope() == parser_scope::scope_preprocessor) {
                    my_parser += parser_scope::scope_stringlit;
                } else if (my_parser.current_scope() == parser_scope::scope_stringlit) {
                    --my_parser;
                }
                break;
            case symbol_t::s_comma:
                // TODO: implement this fully
                --my_parser;
                break;
            case symbol_t::s_lpar:
                // TODO: consider move this to keyword_t inline?
                my_parser += parser_scope::scope_function;
                break;
            case symbol_t::s_rpar:
                if (my_parser.current_scope() == parser_scope::scope_variable) {
                    --my_parser; // variable scope
                }
                --my_parser; // function scope
                break;
            case symbol_t::s_lcub:
                if (my_parser.current_scope() != parser_scope::scope_function) {
                    // TODO: move this to keyword_t struct
                    my_parser += parser_scope::scope_struct;
                }
                break;
            case symbol_t::s_rcub:
                break;
            case symbol_t::s_semi:
                --my_parser;
                break;
            case symbol_t::s_pound:
                my_parser += parser_scope::scope_preprocessor;
                break;
            case symbol_t::s_lt:
                if (my_parser.current_scope() != parser_scope::scope_preprocessor) {
                    my_parser += parser_scope::scope_template;
                }
                break;
            case symbol_t::s_gt:
                if (my_parser.current_scope() == parser_scope::scope_preprocessor) {
                    --my_parser;
                } else if (my_parser.current_scope() == parser_scope::scope_variable) {
                    --my_parser; // scope_variable
                    --my_parser; // scope_template
                }
                break;
            default:
                // Note: no char/string literal support yet
                std::cerr << "unsupported symbol: '" << token.value << "'\n"; 
                throw std::invalid_argument("parser: unsupported symbol"); 
        }
    }

    void visit(type_token const& token) const override {
        std::cout << "visiting type: " << token.value << "\n";

        // TODO: implement this fully
        if (my_parser.current_scope() != parser_scope::scope_struct) {
            my_parser += parser_scope::scope_variable;
        }
    }
}; 

    std::unique_ptr<token_list> const& tokens;
    parser_token_visitor               my_token_visitor;
    std::stack<parser_scope>           scope;

    parser(std::unique_ptr<token_list> const& _tokens)
        : tokens(_tokens), my_token_visitor(*this), scope() {
        parse_tokens();
        check_for_failure();
    }

    void parse_tokens() {
        scope.push(parser_scope::scope_global);
        for (std::unique_ptr<base_token> const& token_ptr: *tokens) {
            token_ptr->accept(my_token_visitor);
        }
    }

    void check_for_failure() {
        if (scope.top() != parser_scope::scope_global) {
            std::cerr << "parser failed to process '" << scope.size() - 1 << "' scopes\n"; 
            throw std::runtime_error("parser: parse failure"); 
        }
    }

    parser_scope const& current_scope() const {
        return scope.top();
    }

    void enter_scope(parser_scope const ps) {
        std::cout << "\nNEW SCOPE\n"; 
        scope.push(ps);
    }

    void exit_scope() {
        std::cout << "EXIT SCOPE\n\n";
        if (scope.size() > 0) {
            scope.pop();
        } else {
            std::cerr << "parser bad scope format\n"; 
            throw std::runtime_error("parser: parse failure"); 
        }
    }

    void operator +=(parser_scope const ps) { enter_scope(ps); }

    void operator--() { exit_scope(); }

    void operator--(int) { exit_scope(); }
};



inline std::unique_ptr<parser> parse(std::unique_ptr<token_list>& tokens) {
    std::unique_ptr<parser> my_parser = std::make_unique<parser>(tokens);
    return my_parser;
}

}