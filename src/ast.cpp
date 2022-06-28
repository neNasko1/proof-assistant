#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <algorithm>

#include "ast.h"

const int64_t HASH_BASE = 257;

expression::expression(const std::string name, bool is_free) : name(name), is_free(is_free), hash(0), pow(1) {
    this->recalculate_hash();
}

expression::expression(const std::string name, std::vector<std::shared_ptr<expression> > &args, bool is_free) : name(name), args(std::move(args)), is_free(is_free), hash(0), pow(1) {
    this->recalculate_hash();
}

void expression::recalculate_hash() {
    this->hash = 0;
    this->pow = 1;

    if(this->is_free) {
        this->hash = (this->hash * HASH_BASE + (int64_t)'*');
        this->pow *= HASH_BASE;
    }

    for(const auto it : name) {
        this->hash = (this->hash * HASH_BASE + (int64_t)it);
        this->pow *= HASH_BASE;
    }

    this->hash = (this->hash * HASH_BASE + (int64_t)'(');
    this->pow *= HASH_BASE;
    
    for(const auto &it : this->args) {
        this->hash = (this->hash + it->hash * this->pow);
        this->pow = (this->pow * it->pow);

        this->hash = (this->hash * HASH_BASE + (int64_t)',');
        this->pow *= HASH_BASE;
    }

    this->hash = (this->hash * HASH_BASE + (int64_t)')');
    this->pow *= HASH_BASE;
}

std::ostream& operator <<(std::ostream &out, const std::shared_ptr<expression> &expr) {
    if(expr->is_free) { out << "*"; }
    out << expr->name;
    
    if(expr->args.size() != 0) {
        out << "(";

        bool is_starting = true;
        for(const auto &it : expr->args) {
            if(!is_starting) {
                out << ", ";
            }
            out << it;
            is_starting = false;
        }

        out << ")";
    }
    return out;
}

bool is_equal(const std::shared_ptr<expression> &expr1, const std::shared_ptr<expression> &expr2) {
    if(expr1->name != expr2->name || expr1->args.size() != expr2->args.size() || expr1->is_free != expr2->is_free) {
        return false;
    }

    if(expr1->hash != expr2->hash) {
        return false;
    }

    for(size_t i = 0; i < expr1->args.size(); i ++) {
        if(!is_equal(expr1->args[i], expr2->args[i])) {
            return false;
        }
    }

    return true;
}

std::shared_ptr<expression> clone(const std::shared_ptr<expression> &expr) {
    auto ret = std::make_shared<expression>(expr->name, expr->is_free);

    for(const auto &it : expr->args) {
        ret->args.push_back(clone(it));
    }

    ret->recalculate_hash();
    return ret;
}

std::shared_ptr<expression> clone_and_replace(const std::shared_ptr<expression> &expr, const std::shared_ptr<expression> &to_replace, const std::shared_ptr<expression> &replacement) {
    if(expr == to_replace) {
        return clone(replacement);
    }

    auto ret = std::make_shared<expression>(expr->name, expr->is_free);

    for(const auto &it : expr->args) {
        ret->args.push_back(clone_and_replace(it, to_replace, replacement));
    }

    ret->recalculate_hash();
    return ret;
}


rule::rule(const std::shared_ptr<expression> &lhs, const std::shared_ptr<expression> &rhs) : lhs(lhs), rhs(rhs) {}

std::ostream& operator <<(std::ostream &out, const std::shared_ptr<rule> &_rule) {
    return out << _rule->lhs << " = " << _rule->rhs;
}


std::optional<match_results> match(const std::shared_ptr<expression> &expr1, const std::shared_ptr<expression> &expr2) {
    match_results ret;

    #ifdef DEBUG
        std::cerr << "Trying to match " << expr1 << " ?= " << expr2 << std::endl;
    #endif

    if(expr2->is_free && expr2->args.size() == 0) {
        ret[expr2->name] = clone(expr1);
        return make_optional(std::move(ret));
    } else if(expr1->name != expr2->name) {
        return std::nullopt;
    }

    if(expr1->args.size() != expr2->args.size()) {
        return std::nullopt;
    }

    for(size_t i = 0; i < expr1->args.size(); i ++) {
        const auto arg_match = std::move(match(expr1->args[i], expr2->args[i]));

        if(!arg_match) {
            return std::nullopt;
        }

        for(const auto &entry : arg_match.value()) {
            if(ret.contains(entry.first)) {
                if(!is_equal(ret[entry.first], entry.second)) {
                    return std::nullopt;
                }
            } else {
                ret[entry.first] = entry.second;
            }
        }
    }

    return make_optional(std::move(ret));
}

std::optional<std::shared_ptr<expression> > apply_matched(const std::shared_ptr<expression> &expr, const match_results &matched) {
    auto ret = make_shared<expression>(expr->name);

    if(expr->is_free) {
        if(matched.contains(expr->name)) {
            // TODO: If a name can be replaced by a function then we essentially have lambda calculus
            const auto matched_res = matched.at(expr->name); 
            if(expr->args.size() == 0) {
                ret = clone(matched_res);
            } else {
                std::cerr << "TODO: this will be really cool, but for now it is unsupported" << std::endl;
                return std::nullopt;
            }
        } else {
            ret->is_free = true;
        }
    }

    for(const auto &it : expr->args) {
        const auto apply_opt = apply_matched(it, matched);
    
        if(!apply_opt) {
            return std::nullopt;
        }

        ret->args.push_back(apply_opt.value());
    }
    ret->recalculate_hash();

    return ret;
}

std::vector<std::shared_ptr<expression> > find_all_applications(const std::shared_ptr<expression> &expr, const std::shared_ptr<rule> &applied_rule) {
    std::vector<std::shared_ptr<expression> > ret;

    const std::function<void(const std::shared_ptr<expression> &)> recurse = [&](const std::shared_ptr<expression> &to_match) -> void {
        const auto match_opt = match(to_match, applied_rule->lhs);

        if(match_opt) {
            #ifdef DEBUG            
                std::cerr << "Matched " << to_match << " -> ";
                for(const auto &entry : match_opt.value()) {
                    std::cerr << "{" << entry.first << " -> " << entry.second << "} "; 
                }
                std::cerr << std::endl;
            #endif

            const auto apply_opt = apply_matched(applied_rule->rhs, match_opt.value());

            if(apply_opt) {
                #ifdef DEBUG
                    std::cerr << "Cloning and replacing " << expr << " " << to_match << " " << apply_opt.value() << std::endl;
                #endif

                ret.push_back(clone_and_replace(expr, to_match, apply_opt.value()));
            }
        }

        for(const auto &it : to_match->args) {
            recurse(it);
        }
    };

    recurse(expr);

    return std::move(ret);
}

std::vector<std::shared_ptr<expression> > run_exhaustive_search(const std::shared_ptr<expression> &expr, const std::vector<std::shared_ptr<rule> > &rules) {
    std::vector<std::shared_ptr<expression> > queue;
    queue.push_back(expr);

    for(size_t i = 0; i < queue.size(); i ++) {
        const auto curr = clone(queue[i]);
        #ifdef DEBUG
            std::cerr << curr << std::endl;
        #endif
        
        for(const auto &_rule : rules) {
            const auto all_adj = std::move(find_all_applications(curr, _rule));

            for(const auto &adj : all_adj) {
                #ifdef DEBUG
                    std::cerr << "Searching for " << adj << std::endl;
                #endif

                bool ok = true;
                for(size_t j = 0; j < queue.size(); j ++) {
                    if(is_equal(queue[j], adj)) {
                        ok = false;
                        break;
                    }
                }

                if(ok) {
                    queue.push_back(adj);
                }
            }
        }
    }

    return queue;
}

std::vector<std::shared_ptr<expression> > find_application_path(const std::shared_ptr<expression> &expr, const std::shared_ptr<expression> &target, const std::vector<std::shared_ptr<rule> > &rules) {
    std::vector<std::shared_ptr<expression> > queue;
    std::map<int32_t, int32_t> parent;
    std::map<int64_t, std::vector<std::shared_ptr<expression> > > used;

    parent[0] = -1;
    queue.push_back(expr);
    used[expr->hash].push_back(expr);

    int32_t found = -1;

    for(size_t i = 0; i < queue.size(); i ++) {
        const auto curr = clone(queue[i]);

        if(is_equal(queue[i], target)) {
            found = i;
            break;
        }

        #ifdef DEBUG
            std::cerr << curr << std::endl;
        #endif
        
        for(const auto &_rule : rules) {
            const auto all_adj = std::move(find_all_applications(curr, _rule));

            for(const auto &adj : all_adj) {
                #ifdef DEBUG
                    std::cerr << "Searching for " << adj << std::endl;
                #endif

                bool ok = true;
                for(const auto &probable_match : used[adj->hash]) {
                    if(is_equal(probable_match, adj)) {
                        ok = false;
                        break;
                    }
                }

                if(ok) {
                    queue.push_back(adj);
                    parent[queue.size() - 1] = i;
                    used[adj->hash].push_back(adj);
                }
            }
        }
    }

    std::vector<std::shared_ptr<expression> > ret;

    while(found != -1) {
        ret.push_back(queue[found]);
        found = parent[found];
    }

    std::reverse(ret.begin(), ret.end());

    return ret;
}