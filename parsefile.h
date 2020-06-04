#pragma once

#include <memory>
#include "tokenizer.h"

namespace proj2 { 

struct token_visitor : token_visitor_base {
    void visit(container_token const& token) const override {
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
    }

    void visit(type_token const& token) const override {
        std::cout << "visiting type: " << token.value << "\n";
    }
};

struct parser {
    std::unique_ptr<token_list> const& tokens;
    token_visitor my_token_visitor;

    parser(std::unique_ptr<token_list> const& _tokens) : tokens(_tokens) {}

    void parse_tokens() {
        for (std::unique_ptr<base_token>& token_ptr: *tokens) {
            token_ptr->accept(my_token_visitor);
        } 
    }
};

inline std::unique_ptr<parser> parse(std::unique_ptr<token_list>& my_tokens) {
    auto my_parser = std::make_unique<parser>(my_tokens);
    my_parser->parse_tokens();
    return my_parser;
}

}