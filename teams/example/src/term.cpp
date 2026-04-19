#include "term.hpp"
#include <stdexcept>
#include <sstream>
#include <vector>

// --- Фабрики ---

std::shared_ptr<Term> Term::make_var(const std::string& name) {
    auto t = std::make_shared<Term>();
    t->kind = TermKind::Var;
    t->var  = name;
    return t;
}

std::shared_ptr<Term> Term::make_abs(const std::string& param,
                                      std::shared_ptr<Term> body) {
    auto t  = std::make_shared<Term>();
    t->kind = TermKind::Abs;
    t->var  = param;
    t->body = body;
    return t;
}

std::shared_ptr<Term> Term::make_app(std::shared_ptr<Term> func,
                                      std::shared_ptr<Term> arg) {
    auto t  = std::make_shared<Term>();
    t->kind = TermKind::App;
    t->func = func;
    t->arg  = arg;
    return t;
}

// --- Глубокая копия ---

std::shared_ptr<Term> Term::clone(std::shared_ptr<Term> t) {
    if (!t) throw std::runtime_error("clone: null term");
    switch (t->kind) {
        case TermKind::Var: {
            return make_var(t->var);
        }
        case TermKind::Abs: {
            return make_abs(t->var, clone(t->body));
        }
        case TermKind::App: {
            return make_app(clone(t->func), clone(t->arg));
        }
    }
    throw std::runtime_error("clone: unknown kind");
}

// --- Печать ---

// Вспомогательная: нужны ли скобки вокруг func в аппликации?
static std::string term_to_str(const Term* t, bool paren_abs, bool paren_app) {
    switch (t->kind) {
        case TermKind::Var:
            return t->var;

        case TermKind::Abs: {
            std::string s = "λ" + t->var + "." + term_to_str(t->body.get(), false, false);
            return paren_abs ? "(" + s + ")" : s;
        }

        case TermKind::App: {
            std::string l = term_to_str(t->func.get(), true,  false);
            std::string r = term_to_str(t->arg.get(),  true,  true);
            std::string s = l + " " + r;
            return paren_app ? "(" + s + ")" : s;
        }
    }
    return "?";
}

std::string Term::to_string() const {
    return term_to_str(this, false, false);
}

// --- α-эквивалентность ---
// Сравниваем де Брёйновские индексы неявно через глубину связывания.

static bool alpha_eq(const Term* a, const Term* b, std::vector<std::string>& env_a, std::vector<std::string>& env_b) {
    if (a->kind != b->kind) return false;

    switch (a->kind) {
        case TermKind::Var: {
            {
                // Найти позицию в стеке связанных переменных
                int ia = -1, ib = -1;
                for (int i = (int)env_a.size()-1; i >= 0; --i)
                    if (env_a[i] == a->var) { ia = i; break; }
                for (int i = (int)env_b.size()-1; i >= 0; --i)
                    if (env_b[i] == b->var) { ib = i; break; }
                if (ia == -1 && ib == -1) return a->var == b->var; // оба свободные
                return ia == ib; // одинаковая глубина связывания
            }
        }
        case TermKind::Abs: {
            env_a.push_back(a->var);
            env_b.push_back(b->var);
            bool result = alpha_eq(a->body.get(), b->body.get(), env_a, env_b);
            env_a.pop_back();
            env_b.pop_back();
            return result;
        }   
        case TermKind::App: {
            return alpha_eq(a->func.get(), b->func.get(), env_a, env_b)
                && alpha_eq(a->arg.get(),  b->arg.get(),  env_a, env_b);
        }
    }
    return false;
}

bool Term::alpha_equiv(std::shared_ptr<Term> a, std::shared_ptr<Term> b) {
    std::vector<std::string> ea, eb;
    return alpha_eq(a.get(), b.get(), ea, eb);
}
