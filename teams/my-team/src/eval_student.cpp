#include "eval.hpp"

#include <stdexcept>
#include <iostream>

std::set<std::string> free_vars(std::shared_ptr<Term> t) {
    switch (t->kind) {
        case TermKind::Var:
            return {t->var};
        case TermKind::Abs: {
            auto body_fv = free_vars(t->body);
            body_fv.erase(t->var);
            return body_fv;
        }
        case TermKind::App: {
            auto func_fv = free_vars(t->func);
            auto arg_fv = free_vars(t->arg);
            func_fv.insert(arg_fv.begin(), arg_fv.end());
            return func_fv;
        }
    }
}

std::string fresh_var(const std::set<std::string>& used) {
    for (char c = 'a'; c <= 'z'; ++c) {
        std::string var(1, c);
        if (used.find(var) == used.end()) {
            return var;
        }
    }
    for (int i = 0; ; ++i) {
        std::string var = "a" + std::to_string(i);
        if (used.find(var) == used.end()) {
            return var;
        }
    }
}

std::shared_ptr<Term> substitute(std::shared_ptr<Term> m, const std::string& x, std::shared_ptr<Term> n) {
    switch (m->kind) {
        case TermKind::Var:
            if (m->var == x) {
                return n;
            }
            return m;
        case TermKind::Abs:
            if (m->var == x) {
                return m;
            }
            if (free_vars(n).count(m->var) > 0) {
                std::string new_var = fresh_var(free_vars(m->body));
                auto renamed_body = substitute(m->body, m->var, Term::make_var(new_var));
                auto body_sub = substitute(renamed_body, x, n);
                return Term::make_abs(new_var, body_sub);
            } else {
                auto body_sub = substitute(m->body, x, n);
                return Term::make_abs(m->var, body_sub);
            }
        case TermKind::App:
            {
                auto func_sub = substitute(m->func, x, n);
                auto arg_sub = substitute(m->arg, x, n);
                return Term::make_app(func_sub, arg_sub);
            }
    }   
}

std::pair<std::shared_ptr<Term>, bool> beta_step(std::shared_ptr<Term> t) {
    if (t->kind == TermKind::App && t->func->kind == TermKind::Abs) {
        auto new_term = substitute(t->func->body, t->func->var, t->arg);
        return {new_term, true};
    }
    switch (t->kind) {
        case TermKind::Var:
            return {t, false};
        case TermKind::Abs: {
            auto [new_body, reduced] = beta_step(t->body);
            if (reduced) {
                return {Term::make_abs(t->var, new_body), true};
            }
            return {t, false};
        }
        case TermKind::App: {
            auto [new_func, func_reduced] = beta_step(t->func);
            if (func_reduced) {
                return {Term::make_app(new_func, t->arg), true};
            }
            auto [new_arg, arg_reduced] = beta_step(t->arg);
            if (arg_reduced) {
                return {Term::make_app(t->func, new_arg), true};
            }
            return {t, false};
        }
    }
}

std::shared_ptr<Term> normalize(std::shared_ptr<Term> t, int max_steps) {
    auto current = t;
    for (int i = 0; i < max_steps; ++i) {
        auto [new_term, reduced] = beta_step(current);
        if (!reduced) {
            return current;
        }
        current = new_term;
    }
    throw std::runtime_error("step limit exceeded");
}

//  Y = λf.(λx.f(x x))(λx.f(x x))
//
//  В чистом β-редуцирующем исчислении Y f →β f (Y f).
//  Так как у нас нет ленивых вычислений, используй
//  Z-комбинатор (call-by-value safe Y):
//
//  Z = λf.(λx.f(λv.x x v))(λx.f(λv.x x v))
//
//  Задача:
//    1. Построй терм Z программно (make_var / make_abs / make_app).
//    2. Закодируй факториал через числа Чёрча (см. church.hpp ниже).
//    3. Убедись, что Z fact_step (church 5) →β church 120.
//
//  Функция ниже должна вернуть терм Z-комбинатора.
std::shared_ptr<Term> make_Z_combinator() {
    auto f = Term::make_var("f");
    auto x = Term::make_var("x");
    auto v = Term::make_var("v");
    auto inner_app = Term::make_app(x, Term::make_app(x, v));
    auto inner_abs = Term::make_abs("x", Term::make_app(f, inner_app));
    return Term::make_abs("f", Term::make_app(Term::clone(inner_abs), Term::clone(inner_abs)));
}
