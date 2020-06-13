#pragma once

#include <algorithm>
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

// helper type for using std::visit with lambdas (Source: cppreference.com)
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

struct ast_type_container {
    container_t                 type;
    bool                        mod_const;
    bool                        mod_ptr;
    bool                        mod_ref;
    std::vector<ast_type_basic> template_types; // no nested templates
};

using ast_type = std::variant<ast_type_basic, ast_type_container>;

struct ast_basic_variable {
    ast_type_basic type;
    std::string    name;
};

struct ast_container {
    ast_type_container type;
    std::string        name;
};

using ast_variable = std::variant<ast_basic_variable, ast_container>;

struct ast_function {
    ast_type                  return_type;
    std::vector<ast_variable> params;
    std::string               name;
};

struct ast_struct {
    std::vector<ast_variable> members; // no nested structs
    std::string               name;
};

struct ast_include {
    bool        quote; // <header> vs "header"
    std::string name;
};

using ast_node = std::variant<
    ast_basic_variable,
    ast_container,
    ast_function,
    ast_include,
    ast_struct>;
using ast = std::vector<ast_node>;

// helper abstract base class for operating on ast nodes (used in cpptopy.h)
struct ast_visitor_base {
    virtual void operator() (ast_basic_variable const&) const = 0;
    virtual void operator() (ast_container      const&) const = 0;
    virtual void operator() (ast_function       const&) const = 0;
    virtual void operator() (ast_include        const&) const = 0;
    virtual void operator() (ast_struct         const&) const = 0;
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
inline std::ostream& operator<< (std::ostream& os, parser_scope ps) {
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


// Note: variants can't hold references, look at reference_wrapper?
using token_tag = std::variant<
    container_token  const*,
    identifier_token const*,
    keyword_token    const*,
    modifier_token   const*,
    symbol_token     const*,
    type_token       const*>;


class parser {
// Note: visitors are declared inside parser because of forward declaration problems

struct parser_ast_visitor { // Note: doesn't inherit from ast_visitor_base because the function prototypes are different
    parser& my_parser;
    parser_ast_visitor(parser& _my_parser) : my_parser(_my_parser) {}

    void operator() (ast_basic_variable& node, identifier_token const* token) const;
    void operator() (ast_basic_variable& node, modifier_token   const* token) const;
    void operator() (ast_basic_variable& node, type_token       const* token) const;

    void operator() (ast_container& node, container_token  const* token) const;
    void operator() (ast_container& node, identifier_token const* token) const;
    void operator() (ast_container& node, modifier_token   const* token) const;
    void operator() (ast_container& node, type_token       const* token) const;

    // void operator() (ast_function&, placeholder_token const* token) const; // not used currently

    void operator() (ast_include& node, identifier_token const* token) const;

    void operator() (ast_struct& node, type_token       const* token) const;

    // Note: with c++20 can change this to operator() (auto&, auto&)
    template <typename Node, typename Token>
    void operator() (Node&, Token&) const {} // Default case, do nothing
};

struct parser_token_visitor : token_visitor_base {
    // Note: lots of annoying boilerplate in this class, metaprogramming would be nice but virtuals can't be templated, hmm...
    parser& my_parser;
    parser_token_visitor(parser& _my_parser) : my_parser(_my_parser) {}

    void visit(container_token const& token) const override {
        std::cout << "visiting container: " << token.value << "\n";
        my_parser.set_current_token(token);

        my_parser += parser_scope::variable;
        my_parser.push_node<ast_container>();

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
            my_parser == parser_scope::template_     ||
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
        
        parser_scope prev_scope = parser_scope::unknown;
        switch (token.type) {
            case symbol_t::s_quot:
                if (my_parser == parser_scope::preprocessor) {
                    my_parser += parser_scope::stringlit;
                } else if (my_parser == parser_scope::stringlit) {
                    my_parser -= parser_scope::stringlit;
                    if (my_parser == parser_scope::preprocessor) {
                        my_parser -= parser_scope::preprocessor;
                        my_parser.pop_node<ast_include>();
                    }
                }
                break;

            case symbol_t::s_comma:
                my_parser -= parser_scope::variable;
                break;

            case symbol_t::s_lpar:
                my_parser += parser_scope::function_decl;
                my_parser.push_node<ast_function>();
                break;

            case symbol_t::s_rpar:
                if (my_parser == parser_scope::variable)
                    my_parser -= parser_scope::variable; // last function param
                my_parser -= parser_scope::function_decl;
                my_parser.pop_node<ast_function>();
                break;

            case symbol_t::s_lcub:
                if (my_parser == parser_scope::struct_decl)
                    my_parser += parser_scope::struct_def;
                else if (my_parser == parser_scope::function_decl)
                    my_parser += parser_scope::function_def;
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
                prev_scope = my_parser.current_scope();
                --my_parser;
                if (my_parser.node_exists() &&
                    my_parser == parser_scope::global &&
                    prev_scope == parser_scope::variable)
                        my_parser.pop_node_global_variable();
                break;

            case symbol_t::s_pound:
                my_parser += parser_scope::preprocessor;
                my_parser.push_node<ast_include>();
                break;

            case symbol_t::s_lt:
                if (my_parser != parser_scope::preprocessor)
                    my_parser += parser_scope::template_;
                break;

            case symbol_t::s_gt:
                if (my_parser == parser_scope::preprocessor) {
                    my_parser -= parser_scope::preprocessor;
                    my_parser.pop_node<ast_include>();
                } else if (my_parser == parser_scope::variable) {
                    my_parser -= parser_scope::variable;
                    my_parser -= parser_scope::template_;
                    my_parser.pop_node<ast_container>();
                }
                break;
            default:
                // Note: no char literal support yet
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
    std::vector<ast_variable>          temp_extraction_members;


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
        if (!ast_nodes_under_construction.empty()) {
            std::cerr << "parser failed to construct '" << ast_nodes_under_construction.size() << "' ast nodes\n"; 
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
        // std::cout << "NEW SCOPE >" << ps << "\n"; 
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
        // std::cout << "EXIT SCOPE<" << expected_scope << "\n";
        exit_scope();
    }

    bool operator ==(parser_scope const ps) const { return current_scope() == ps; }

    bool operator !=(parser_scope const ps) const { return current_scope() != ps; }

    void operator +=(parser_scope const ps) { enter_scope(ps); }

    void operator -=(parser_scope const ps) { exit_scope(ps); }

    void operator --()    { /* std::cout << "EXIT SCOPE<\n"; */ exit_scope(); }

    void operator --(int) { exit_scope(); }

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

    ast_node current_node_pop() {
        ast_node temp = std::move(current_node());
        ast_nodes_under_construction.pop();
        return temp;
    }

    bool node_exists() {
        return !ast_nodes_under_construction.empty();
    }

    template <class Node> // TODO: enable if
    void push_node() {
        ast_nodes_under_construction.push(Node());
    }

    void update_node() {
        if (node_exists()) {
            std::visit(my_ast_visitor, current_node(), get_current_token());
        }
    }

    void pop_node_global_variable() {
        std::visit(overloaded {
            [this](ast_basic_variable&){ pop_node<ast_basic_variable>(); },
            [this](ast_container&)     { pop_node<ast_container>(); move_node_to_ast(); },
            [this](auto&)              { throw std::runtime_error("invalid call to pop_node_global_variable()"); }
        }, current_node());
    }

    template <class Node> // TODO: enable if
    void pop_node() {
        if (!std::holds_alternative<Node>(current_node()))
            extract_members<Node>();
        if(std::holds_alternative<ast_function>(current_node()))
            move_function_to_ast();
        else if (!std::holds_alternative<ast_container>(current_node()))
            move_node_to_ast();
    }

    template <class Node> // TODO: enable if?
    void extract_members() {
        // extract struct members, func params or template types
        while (!std::holds_alternative<Node>(current_node())) {
            std::visit(overloaded {
                [this](ast_basic_variable&& node){ temp_extraction_members.push_back(std::move(node)); },
                [this](ast_container&&      node){ temp_extraction_members.push_back(std::move(node)); },
                [this](auto&&               node){ throw std::runtime_error("parser: invalid call to extract_members() [extraction step]"); }
            }, current_node_pop());
        }
        std::reverse(temp_extraction_members.begin(), temp_extraction_members.end()); // correct the order of params/ types

        // move members into struct or params into func or types in template
        std::visit(overloaded {
            [this](ast_function&  node){ node.params  = std::move(temp_extraction_members); },
            [this](ast_struct&    node){ node.members = std::move(temp_extraction_members); },
            [this](ast_container& node){
                // convert vector<ast_variable> -> vector<ast_type_basic>
                for (ast_variable& var: temp_extraction_members) {
                    ast_basic_variable& basic_var = std::get<ast_basic_variable>(var); // will throw bad_variant_access if template contains a container
                    node.type.template_types.push_back(std::move(basic_var.type));
                }
            },
            [this](auto& node){ throw std::runtime_error("parser: invalid call to extract_members() [move step]"); }
        }, current_node());
        temp_extraction_members.clear();
    }

    void move_function_to_ast() {
        // Note: this function will throw if there is no function return value
        ast_node the_function   = current_node_pop();
        ast_node the_return_val = current_node_pop();
        ast_function& func_ref = std::get<ast_function>(the_function);

        std::visit(overloaded {
            [&func_ref](ast_basic_variable& retv_ref){
                func_ref.name        = std::move(retv_ref.name);
                func_ref.return_type = std::move(retv_ref.type);
            },
            [&func_ref](ast_container& retv_ref){
                func_ref.name        = std::move(retv_ref.name);
                func_ref.return_type = std::move(retv_ref.type);
            },
            [&func_ref](auto&){
                std::cerr << "parser unexpected function return type";
                throw std::runtime_error("parser: invalid function return type"); 
            }
        }, the_return_val);
        my_ast->push_back(std::move(the_function));
    }

    void move_node_to_ast() {
        my_ast->push_back(current_node_pop());
    }

public:
    parser(std::unique_ptr<token_list> const& _tokens) :
            tokens(_tokens),
            my_ast(std::make_unique<ast>()),
            scope(),
            ast_nodes_under_construction(),
            my_token_visitor(*this),
            my_ast_visitor(*this),
            current_token_tag(),
            temp_extraction_members() {
        parse_tokens();
        check_for_failure();
    }

    std::unique_ptr<ast> move_ast() {
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
            std::cerr << "parser bad variable modifier\n"; 
            throw std::runtime_error("parser: parse failure"); 
    }
}

void parser::parser_ast_visitor::operator() (ast_basic_variable& node, type_token const* token) const {
    node.type.type = token->type;
    if (token->type == type_t::t_custom) {
        node.type.custom_typename = token->value;
    }
}

void parser::parser_ast_visitor::operator() (ast_container& node, container_token const* token) const {
    node.type.type = token->type;
}

void parser::parser_ast_visitor::operator() (ast_container& node, identifier_token const* token) const {
    node.name = token->value;
}


void parser::parser_ast_visitor::operator() (ast_container& node, modifier_token const* token) const {
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
        default:
            std::cerr << "parser bad container modifier\n"; 
            throw std::runtime_error("parser: parse failure"); 
    }
}

void parser::parser_ast_visitor::operator() (ast_container& node, type_token const* token) const {
    node.name = token->value;
}


void parser::parser_ast_visitor::operator() (ast_include& node, identifier_token const* token) const {
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