#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <fstream>

#include "ast.h"

// Currently parsing doesn't throw errors on invalid programs, which is kinda sad but deal with it 
enum token_type {
    IDENTIFIER,
    L_PAREN,
    R_PAREN,
    COMMA,
    STAR,
    END_OF_FILE
};

struct token {
    std::string_view value;
    token_type type;

    token(const std::string_view value, const token_type type);
    ~token() = default;
};

std::shared_ptr<expression> parse_expression(const std::vector<token> &tokens, size_t &ind_ptr);

std::shared_ptr<rule> parse_rule(const std::vector<token> &tokens, size_t &ind_ptr);

void parse(const std::string &filename);