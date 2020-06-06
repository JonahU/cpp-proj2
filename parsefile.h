#pragma once

#include <iostream>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string_view>
#include <variant>
#include <vector>
#include "tokenizer.h"

namespace proj2 {

// TODO: remove const from all parser type members?

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

struct parser_basic_variable {
    const parser_basic_type type;
    std::string_view        name;
};

struct parser_template_type {
    const parser_basic_type               type;
    const container_t                     container_type;
    std::string_view                      custom_typename;
    std::unique_ptr<parser_template_type> nested;
};

struct parser_container {
    const parser_template_type type;
    std::string_view           name;
};

using parser_variable = std::variant<parser_basic_variable, parser_container>;

struct parser_struct {
    const std::vector<parser_variable> members;
    std::unique_ptr<parser_struct>     nested; // Note: nested structs not tested yet
    std::string_view                   name;
};

struct parser_function {
    const std::vector<parser_variable> params;
    std::string_view                   name;
};

enum class parser_scope {
    unknown,
    global,
    preprocessor,
    stringlit,
    variable,
    struct_decl,
    struct_def,
    function_decl,
    function_def, // Note: func def not tested yet
    template_
};

// For error messages
std::ostream& operator<< (std::ostream& os, parser_scope ps) {
    switch (ps) {
        case parser_scope::global        : return os << "global"       ;
        case parser_scope::preprocessor  : return os << "preprocessor" ;
        case parser_scope::stringlit     : return os << "stringlit"    ;
        case parser_scope::variable      : return os << "varaible"     ;
        case parser_scope::struct_decl   : return os << "struct_decl"  ;
        case parser_scope::struct_def    : return os << "struct_def"   ;
        case parser_scope::function_decl : return os << "function_decl";
        case parser_scope::function_def  : return os << "function_def" ;
        case parser_scope::template_     : return os << "template_"     ;
        default                          : return os << "unknown"      ;
    };
}

using ast_node = std::variant<
    parser_include,
    parser_basic_type,
    parser_basic_variable,
    parser_template_type,
    parser_container>;
using ast = std::vector<ast_node>;

class parser {

// Note: parser_token_visitor is here because of forward declaration problems 
struct parser_token_visitor : token_visitor_base {
    parser& my_parser;
    parser_token_visitor(parser& _my_parser) : my_parser(_my_parser) {}

    void visit(container_token const& token) const override {
        my_parser += parser_scope::variable;
        std::cout << "visiting container: " << token.value << "\n";
    }

    void visit(identifier_token const& token) const override {
        std::cout << "visiting identifier: " << token.value << "\n";
    }

    void visit(keyword_token const& token) const override {
        std::cout << "visiting keyword: " << token.value << "\n";
        if (token.type == keyword_t::k_struct) {
            my_parser += parser_scope::struct_decl;
        }
    }

    void visit(modifier_token const& token) const override {
        std::cout << "visiting modifier: " << token.value << "\n";
    }

    void visit(symbol_token const& token) const override {
        std::cout << "visiting symbol: " << token.value << "\n";
        switch (token.type) {
            case symbol_t::s_quot:
                if (my_parser == parser_scope::preprocessor) {
                    my_parser += parser_scope::stringlit;
                } else if (my_parser == parser_scope::stringlit) {
                    my_parser -= parser_scope::stringlit;
                }
                break;
            case symbol_t::s_comma:
                my_parser -= parser_scope::variable;
                break;
            case symbol_t::s_lpar:
                my_parser += parser_scope::function_decl;
                break;
            case symbol_t::s_rpar:
                if (my_parser == parser_scope::variable) {
                    my_parser -= parser_scope::variable;
                }
                my_parser -= parser_scope::function_decl;
                break;
            case symbol_t::s_lcub:
                if (my_parser == parser_scope::struct_decl) {
                    my_parser += parser_scope::struct_def;
                } else if (my_parser == parser_scope::function_decl) {
                    my_parser += parser_scope::function_def;
                }
                break;
            case symbol_t::s_rcub:
                if (my_parser == parser_scope::struct_def) {
                    my_parser -= parser_scope::struct_def;
                } else if (my_parser == parser_scope::function_def) {
                    my_parser -=parser_scope::function_def;
                }
                break;
            case symbol_t::s_semi:
                --my_parser;
                break;
            case symbol_t::s_pound:
                my_parser += parser_scope::preprocessor;
                break;
            case symbol_t::s_lt:
                if (my_parser != parser_scope::preprocessor) {
                    my_parser += parser_scope::template_;
                }
                break;
            case symbol_t::s_gt:
                if (my_parser == parser_scope::preprocessor) {
                    my_parser -= parser_scope::preprocessor;
                } else if (my_parser == parser_scope::variable) {
                    my_parser -= parser_scope::variable;
                    my_parser -= parser_scope::template_;
                }
                break;
            default:
                // Note: no char/string literal support yet
                std::cerr << "unsupported symbol: " << token.value << "\n"; 
                throw std::invalid_argument("parser: unsupported symbol"); 
        }
    }

    void visit(type_token const& token) const override {
        std::cout << "visiting type: " << token.value << "\n";
        if (my_parser.current_scope() != parser_scope::struct_decl) {
            my_parser += parser_scope::variable;
        }
    }
};

    std::unique_ptr<token_list> const& tokens;
    std::unique_ptr<ast>               my_ast;
    parser_token_visitor        const  my_token_visitor;
    std::stack<parser_scope>           scope;

    void parse_tokens() {
        scope.push(parser_scope::global);
        for (std::unique_ptr<base_token> const& token_ptr: *tokens) {
            token_ptr->accept(my_token_visitor);
        }
    }

    void check_for_failure() {
        if (scope.top() != parser_scope::global) {
            std::cerr << "parser failed to process '" << get_depth() - 1 << "' scopes\n"; 
            throw std::runtime_error("parser: parse failure"); 
        }
    }

    int get_depth() const {
        return scope.size();
    }

    parser_scope current_scope() const {
        return scope.top();
    }

    void enter_scope(parser_scope const ps) {
        std::cout << "NEW SCOPE >" << ps << "\n"; 
        scope.push(ps);
    }

    void exit_scope() {
        if (scope.size() < 1) {
            std::cerr << "parser bad scope exit\n"; 
            throw std::runtime_error("parser: parse failure"); 
        }
        scope.pop();
    }

    void exit_scope(parser_scope const expected_scope) {
        if (current_scope() != expected_scope) {
            std::cerr << "parser expected scope '" << expected_scope << "', got '" << current_scope() << "'\n"; 
            throw std::runtime_error("parser: parse failure"); 
        }
        std::cout << "EXIT SCOPE<" << expected_scope << "\n";
        exit_scope();
    }

    bool operator ==(parser_scope const ps) const { return current_scope() == ps; }

    bool operator !=(parser_scope const ps) const { return current_scope() != ps; }

    void operator +=(parser_scope const ps) { enter_scope(ps); }

    void operator -=(parser_scope const ps) { exit_scope(ps); }

    void operator--()    { std::cout << "EXIT SCOPE<\n"; exit_scope(); }

    void operator--(int) { exit_scope(); }

public:
    parser(std::unique_ptr<token_list> const& _tokens)
        : tokens(_tokens), my_ast(), my_token_visitor(*this), scope() {
        parse_tokens();
        check_for_failure();
    }

    std::unique_ptr<ast>&& move_ast() {
        return std::move(my_ast);
    }
};

inline std::unique_ptr<ast> parse(std::unique_ptr<token_list>& tokens) {
    parser my_parser(tokens);
    return my_parser.move_ast();
}

}