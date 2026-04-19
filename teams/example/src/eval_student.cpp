#include "eval.hpp"

#include <stdexcept>

std::set<std::string> free_vars(std::shared_ptr<Term> t) {
    (void)t;
    throw std::runtime_error("free_vars: not implemented");
}

std::string fresh_var(const std::set<std::string>& used) {
    (void)used;
    throw std::runtime_error("fresh_var: not implemented");
}

std::shared_ptr<Term> substitute(std::shared_ptr<Term> m,
                                 const std::string& x,
                                 std::shared_ptr<Term> n) {
    (void)m;
    (void)x;
    (void)n;
    throw std::runtime_error("substitute: not implemented");
}

std::pair<std::shared_ptr<Term>, bool> beta_step(std::shared_ptr<Term> t) {
    (void)t;
    throw std::runtime_error("beta_step: not implemented");
}

std::shared_ptr<Term> normalize(std::shared_ptr<Term> t, int max_steps) {
    (void)t;
    (void)max_steps;
    throw std::runtime_error("normalize: not implemented");
}

std::shared_ptr<Term> make_Z_combinator() {
    throw std::runtime_error("make_Z_combinator: not implemented");
}
