#pragma once

#include <iostream>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <utility>
#include "tokenizer.h"

namespace proj2 {

// helper type for the visitor in pop_node() (Source: cppreference.com)
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct ast_type_basic {
    type_t      type;
    bool        mod_const;
    bool        mod_ptr;
    bool        mod_ref;
    bool        mod_unsigned;
    std::string custom_typename;
};

struct ast_type_template {
    ast_type_basic                     type;
    container_t                        container_type;
    std::string                        custom_typename;
    std::unique_ptr<ast_type_template> nested; // Note: nested templates not tested yet
};

struct ast_basic_variable {
    ast_type_basic type;
    std::string    name;
};

struct ast_container {
    ast_type_template type;
    std::string       name;
};

using ast_variable = std::variant<ast_basic_variable, ast_container>;

struct ast_function {
    ast_variable              return_type;
    std::vector<ast_variable> params;
    std::string               name;
};

struct ast_struct {
    std::vector<ast_variable>   members;
    std::unique_ptr<ast_struct> nested; // Note: nested structs not tested yet
    std::string                 name;
};

struct ast_include {
    bool        quote; // <header> vs "header"
    std::string name;
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
        case parser_scope::variable      : return os << "variable"     ;
        case parser_scope::struct_decl   : return os << "struct_decl"  ;
        case parser_scope::struct_def    : return os << "struct_def"   ;
        case parser_scope::function_decl : return os << "function_decl";
        case parser_scope::function_def  : return os << "function_def" ;
        case parser_scope::template_     : return os << "template_"    ;
        default                          : return os << "unknown"      ;
    };
}

using ast_node = std::variant<
    ast_basic_variable,
    ast_container,
    ast_function,
    ast_include,
    ast_struct>;
using ast = std::vector<ast_node>;

// Note: variants can't hold references
using token_tag = std::variant<
    std::monostate, // because tokens aren't default constructable
    container_token  const*,
    identifier_token const*,
    keyword_token    const*,
    modifier_token   const*,
    symbol_token     const*,
    type_token       const*>;

// helper abstract base class for operating on ast nodes
struct ast_visitor_base {
    virtual void operator() (ast_basic_variable&) const = 0;
    virtual void operator() (ast_container&)      const = 0;
    virtual void operator() (ast_function&)       const = 0;
    virtual void operator() (ast_include&)        const = 0;
    virtual void operator() (ast_struct&)         const = 0;
};

class parser {
// Note: visitors are declared inside parser because of forward declaration problems

struct parser_ast_visitor { // Note: doesn't inherit from ast_visitor_base because the function prototypes are different
    parser& my_parser;
    parser_ast_visitor(parser& _my_parser) : my_parser(_my_parser) {}

    // void operator() (ast_basic_variable&) const {

    void operator() (ast_basic_variable& node, identifier_token const* token) const;
    void operator() (ast_basic_variable& node, modifier_token   const* token) const;
    void operator() (ast_basic_variable& node, type_token       const* token) const;

    // void operator() (ast_container&) const {
    // void operator() (ast_function&) const {

    void operator() (ast_include& node, identifier_token const* token) const;

    void operator() (ast_struct& node, identifier_token const* token) const;
    void operator() (ast_struct& node, type_token       const* token) const;

    // Note: with c++20 can change this to operator() (auto&, auto&)
    template <typename Node, typename Token>
    void operator() (Node&, Token&) const {} // Default case, do nothing
};

struct parser_token_visitor : token_visitor_base {
    // Note: lots of annoying boilerplate in this class, could metaprogramming could help?
    parser& my_parser;
    parser_token_visitor(parser& _my_parser) : my_parser(_my_parser) {}

    void visit(container_token const& token) const override {
        std::cout << "visiting container: " << token.value << "\n";
        my_parser.set_current_token(token);

        my_parser += parser_scope::variable;

        my_parser.update_node();
    }

    void visit(identifier_token const& token) const override {
        std::cout << "visiting identifier: " << token.value << "\n";
        my_parser.set_current_token(token);

        my_parser.update_node();
    }

    void visit(keyword_token const& token) const override {
        std::cout << "visiting keyword: " << token.value << "\n";
        my_parser.set_current_token(token);

        if (token.type == keyword_t::k_struct) {
            my_parser += parser_scope::struct_decl;
            my_parser.push_node<ast_struct>();
        }

        my_parser.update_node();
    }

    void visit(modifier_token const& token) const override {
        std::cout << "visiting modifier: " << token.value << "\n";
        my_parser.set_current_token(token);

        if (my_parser == parser_scope::struct_def    ||
            my_parser == parser_scope::function_decl ||
            my_parser == parser_scope::global) 
        {
            my_parser += parser_scope::variable;
            my_parser.push_node<ast_basic_variable>(); // modifier must be unsigned, so push basic_variable
        }

        my_parser.update_node();

    }

    void visit(symbol_token const& token) const override {
        std::cout << "visiting symbol: " << token.value << "\n";
        my_parser.set_current_token(token);

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
                    my_parser.pop_node<ast_struct>();
                } else if (my_parser == parser_scope::function_def) {
                    my_parser -=parser_scope::function_def;
                }
                break;
            case symbol_t::s_semi:
                --my_parser;
                break;
            case symbol_t::s_pound:
                my_parser += parser_scope::preprocessor;
                my_parser.push_node<ast_include>();
                break;
            case symbol_t::s_lt:
                if (my_parser != parser_scope::preprocessor) {
                    my_parser += parser_scope::template_;
                }
                break;
            case symbol_t::s_gt:
                if (my_parser == parser_scope::preprocessor) {
                    my_parser -= parser_scope::preprocessor;
                    my_parser.pop_node<ast_include>();
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

        my_parser.update_node();
    }

    void visit(type_token const& token) const override {
        std::cout << "visiting type: " << token.value << "\n";
        my_parser.set_current_token(token);

        if (my_parser != parser_scope::struct_decl &&
            my_parser != parser_scope::variable) 
        {
            my_parser += parser_scope::variable;
            my_parser.push_node<ast_basic_variable>();
        }

        my_parser.update_node();
    }
};

    std::unique_ptr<token_list> const& tokens;
    std::unique_ptr<ast>               my_ast;
    std::stack<parser_scope>           scope;
    std::stack<ast_node>               ast_nodes_under_construction;
    parser_token_visitor        const  my_token_visitor;
    parser_ast_visitor          const  my_ast_visitor;
    token_tag                          current_token_tag;

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

    token_tag const& get_current_token() const {
        return current_token_tag; 
    }

    template <class Token> // TODO: enable if
    void set_current_token(Token const& token) {
        current_token_tag = &token;
    }

    ast_node& current_node() {
        return ast_nodes_under_construction.top();
    }

    bool constructing_nodes() {
        return !ast_nodes_under_construction.empty();
    }

    template <class Node> // TODO: enable if
    void push_node() {
        ast_nodes_under_construction.push(Node());
    }

    void update_node() {
        if (constructing_nodes()) {
            std::visit(my_ast_visitor, current_node(), get_current_token());
        }
    }

    template <class Node> // TODO: enable if
    void pop_node() {
        if (!std::holds_alternative<Node>(current_node())) {
            std::vector<ast_variable> members; // TODO: improve this? it is wasteful. It also hold members in reverse order
            while (!std::holds_alternative<Node>(current_node())) {
                std::visit(overloaded {
                    [&members](ast_basic_variable& node){ members.push_back(std::move(node)); },
                    [&members](ast_container&      node){ members.push_back(std::move(node)); },
                    [&members](auto&               node){ throw std::runtime_error("parser: invalid pop_node()"); }
                }, current_node());
                ast_nodes_under_construction.pop();
            }
            std::visit(overloaded {
                [&members](ast_function&  node){ node.params  = std::move(members); },
                [&members](ast_struct&    node){ node.members = std::move(members); },
                [&members](auto&          node){ throw std::runtime_error("parser: invalid pop_node()"); }
            }, current_node());
        }
        my_ast->push_back(std::move(current_node()));
        ast_nodes_under_construction.pop();
    }

public:
    parser(std::unique_ptr<token_list> const& _tokens) :
            tokens(_tokens),
            my_ast(std::make_unique<ast>()),
            scope(),
            ast_nodes_under_construction(),
            my_token_visitor(*this),
            my_ast_visitor(*this),
            current_token_tag() {
        parse_tokens();
        check_for_failure();
    }

    std::unique_ptr<ast>&& move_ast() {
        return std::move(my_ast);
    }
};

void parser::parser_ast_visitor::operator() (ast_basic_variable& node, identifier_token const* token) const {
    node.name = token->value;
}

void parser::parser_ast_visitor::operator() (ast_basic_variable& node, modifier_token const* token) const {
    switch (token->type) {
        case modifier_t::m_const:
            node.type.mod_const = true;
            break;
        case modifier_t::m_ptr:
            node.type.mod_ptr = true;
            break;
        case modifier_t::m_ref:
            node.type.mod_ref = true;
            break;
        case modifier_t::m_unsigned:
            node.type.mod_unsigned = true;
            break;
        default:
            std::cerr << "parser bad modifier\n"; 
            throw std::runtime_error("parser: parse failure"); 
    }

}
void parser::parser_ast_visitor::operator() (ast_basic_variable& node, type_token const* token) const {
    node.type.type = token->type;
    if (token->type == type_t::t_custom) {
        node.type.custom_typename = token->value;
    }
}

void parser::parser_ast_visitor::operator() (ast_include& node, identifier_token const* token) const {
    // expect parser_scope::preprocessor
    // if (my_parser != parser_scope::preprocessor) {
    //     std::cerr << "parser unexpected include within scope '" << my_parser.current_scope() << "'\n"; 
    //     throw std::runtime_error("parser: parse failure"); 
    // }
    node.name = token->value;
}

void parser::parser_ast_visitor::operator() (ast_struct& node, identifier_token const* token) const {
    node.name = token->value;
}

void parser::parser_ast_visitor::operator() (ast_struct& node, type_token const* token) const {
    node.name = token->value;
}


inline std::unique_ptr<ast> parse(std::unique_ptr<token_list> const& tokens) {
    parser my_parser(tokens);
    return my_parser.move_ast();
}

}