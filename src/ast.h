#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <string_view>

// An expression, without arguments is equivalent to a variable
struct expression {
    std::string name;
    std::vector<std::shared_ptr<expression> > args;
    bool is_free;

    // Some internal hash variables to help optimize is_equal
    int64_t hash;
    int64_t pow;

    expression(const std::string name, bool is_free = false);
    expression(const std::string name, std::vector<std::shared_ptr<expression> > &args, bool is_free = false);
    ~expression() = default;

    void recalculate_hash();
}; 

bool is_equal(const std::shared_ptr<expression> &expr1, const std::shared_ptr<expression> &expr2);
std::shared_ptr<expression> clone(const std::shared_ptr<expression> &expr);
std::shared_ptr<expression> clone_and_replace(const std::shared_ptr<expression> &expr, const std::shared_ptr<expression> &to_replace, const std::shared_ptr<expression> &replacement);
std::ostream& operator <<(std::ostream &out, const std::shared_ptr<expression> &expr);

struct rule {
    std::shared_ptr<expression> lhs;
    std::shared_ptr<expression> rhs;

    rule(const std::shared_ptr<expression> &lhs, const std::shared_ptr<expression> &rhs);
    ~rule() = default;
};
std::ostream& operator <<(std::ostream &out, const std::shared_ptr<rule> &_rule);

typedef std::map<std::string, std::shared_ptr<expression> > match_results;
std::optional<match_results> match(const std::shared_ptr<expression> &expr1, const std::shared_ptr<expression> &expr2);

std::optional<std::shared_ptr<expression> > apply_matched(const std::shared_ptr<expression> &expr, const match_results &matched);

std::vector<std::shared_ptr<expression> > find_all_applications(const std::shared_ptr<expression> &expr, const std::shared_ptr<rule> &applied_rule);

std::vector<std::shared_ptr<expression> > run_exhaustive_search(const std::shared_ptr<expression> &expr, const std::vector<std::shared_ptr<rule> > &rules);

std::vector<std::shared_ptr<expression> > find_application_path(const std::shared_ptr<expression> &expr, const std::shared_ptr<expression> &target, const std::vector<std::shared_ptr<rule> > &rules);