#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <fstream>

#include "ast.h"
#include "parser.h"

token::token(const std::string_view value, const token_type type) : value(value), type(type) {}

std::shared_ptr<expression> parse_expression(const std::vector<token> &tokens, size_t &ind_ptr) {
    std::shared_ptr<expression> ret = std::make_shared<expression>("Unnamed");

    if(tokens[ind_ptr].type == token_type::STAR) {
        ret->is_free = true;
        ind_ptr ++;
    }

    if(tokens[ind_ptr].type == token_type::IDENTIFIER) {
        ret->name = tokens[ind_ptr].value;
        ind_ptr ++;
    } else {
        std::cerr << "Unexpected token in expression declaration" << " -> " << tokens[ind_ptr].value << " " << tokens[ind_ptr].type << std::endl;
        exit(-1);
    }

    if(tokens[ind_ptr].type == token_type::L_PAREN) {
        ind_ptr ++;

        while(true) {
            ret->args.push_back(parse_expression(tokens, ind_ptr));

            if(tokens[ind_ptr].type == token_type::R_PAREN) {
                ind_ptr ++;
                break;
            } else if(tokens[ind_ptr].type == token_type::COMMA) {
                ind_ptr ++;
            } else {
                std::cerr << "Unexpected token in expression declaration " << " -> " << tokens[ind_ptr].value << " " << tokens[ind_ptr].type << std::endl;
                exit(-1);
            }
        }
    }

    ret->recalculate_hash();

    return ret;
}

std::shared_ptr<rule> parse_rule(const std::vector<token> &tokens, size_t &ind_ptr) {
    const auto lhs = parse_expression(tokens, ind_ptr);
    const auto rhs = parse_expression(tokens, ind_ptr);
    std::shared_ptr<rule> ret = std::make_shared<rule>(lhs, rhs);

    return ret;
}

void parse(const std::string &filename) {
    const auto is_whitespace = [](const char c) {
        return c == ' ' || c == '\t' || c == '\n';
    };

    const auto is_special = [is_whitespace](const char c) {
        return c == '(' || c == ')' || c == '*' || c == ',' || is_whitespace(c);
    };

    std::vector<std::shared_ptr<rule> > rules;

    const auto read_line = [&](const std::string &line) {
        std::vector<token> tokens;
        for(size_t start = 0; start < line.size(); start ++) {
            if(is_whitespace(line[start])) {
                continue;
            } else if(line[start] == '(') {
                tokens.push_back(token("(", token_type::L_PAREN));
            } else if(line[start] == ')') {
                tokens.push_back(token(")", token_type::R_PAREN));
            } else if(line[start] == '*') {
                tokens.push_back(token("*", token_type::STAR));
            } else if(line[start] == ',') {
                tokens.push_back(token(",", token_type::COMMA));
            } else {
                size_t end = start;
                while(end + 1 < line.size() && !is_special(line[end + 1])) {
                    end ++;
                }
                tokens.push_back(token(std::string_view(line).substr(start, end - start + 1), token_type::IDENTIFIER));

                start = end;
            }
        }

        if(tokens.empty()) {
            return;
        }

        tokens.push_back(token("EOF", token_type::END_OF_FILE));

        if(tokens[0].value == "rule") {
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            if(tokens[1].value != "both") {
                std::cout << "Parsing rule " << std::endl;
                size_t ind_ptr = 1;
                const auto _rule = parse_rule(tokens, ind_ptr);
                rules.push_back(_rule);
                std::cout << _rule << std::endl;
            } else {
                std::cout << "Parsing rule both " << std::endl;
                size_t ind_ptr = 2;
                const auto _rule = parse_rule(tokens, ind_ptr);
                rules.push_back(_rule);
                const auto reverse_rule = std::make_shared<rule>(clone(_rule->rhs), clone(_rule->lhs));
                rules.push_back(reverse_rule);
                std::cout << _rule << std::endl;
            }


        } else if(tokens[0].value == "apply") {
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            std::cout << "Parsing application " << std::endl;
            size_t ind_ptr = 1;
            const std::shared_ptr<expression> expr = parse_expression(tokens, ind_ptr);

            std::cout << "Trying all applications on " << expr << std::endl;
            const auto all_applications = run_exhaustive_search(expr, rules);

            for(const auto &applied : all_applications) {
                std::cout << applied << std::endl;
            }
        
        } else if(tokens[0].value == "equal") {
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            std::cout << "Proving equality " << std::endl;

            size_t ind_ptr = 1;
            const auto lhs = parse_expression(tokens, ind_ptr);
            const auto rhs = parse_expression(tokens, ind_ptr);

            std::cout << lhs << " ?= " << rhs << std::endl;

            const auto all_applications = find_application_path(lhs, rhs, rules);

            for(const auto &applied : all_applications) {
                std::cout << applied << std::endl;
            }            

            if(!all_applications.empty()) {
                std::cout << "They are equal" << std::endl;
            } else {
                std::cout << "They are not equal" << std::endl;
            }
        }
    };

    std::ifstream file(filename);
    if(file.is_open()) {
        std::string line;   
        while(std::getline(file, line)) {
            read_line(line);
        }
        file.close();
    }
}
