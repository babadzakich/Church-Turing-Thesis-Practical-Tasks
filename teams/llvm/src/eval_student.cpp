#include "../include/eval.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <sys/types.h>

std::set<std::string> used_vars(std::shared_ptr<Term> t) {
  std::set<std::string> used;

  switch (t->kind) {
  case TermKind::Var: {
    used.insert(t->var);
    break;
  }
  case TermKind::Abs: {
    used.insert(t->var);
    auto used1 = used_vars(t->body);
    used.insert(used1.begin(), used1.end());
    break;
  }
  case TermKind::App: {
    auto used1 = used_vars(t->func);
    auto used2 = used_vars(t->arg);
    std::set_union(used1.begin(), used1.end(), used2.begin(), used2.end(),
                   std::inserter(used, used.begin()));
    break;
  }
  }

  return used;
}

std::set<std::string> bind_vars(std::shared_ptr<Term> t) {
  std::set<std::string> bv;

  switch (t->kind) {
  case TermKind::Var: {
    break;
  }
  case TermKind::Abs: {
    bv.insert(t->var);
    break;
  }
  case TermKind::App: {
    auto bv1 = bind_vars(t->func);
    auto bv2 = bind_vars(t->arg);
    std::set_union(bv1.begin(), bv1.end(), bv2.begin(), bv2.end(),
                   std::inserter(bv, bv.begin()));
    break;
  }
  }

  return bv;
}

std::set<std::string> free_vars(std::shared_ptr<Term> t) {
  std::set<std::string> fv;

  switch (t->kind) {
  case TermKind::Var: {
    fv.insert(t->var);
    break;
  }
  case TermKind::Abs: {
    auto fv1 = free_vars(t->body);
    fv1.erase(t->var);
    fv.insert(fv1.begin(), fv1.end());
    break;
  }
  case TermKind::App: {
    auto fv1 = free_vars(t->func);
    auto fv2 = free_vars(t->arg);
    std::set_union(fv1.begin(), fv1.end(), fv2.begin(), fv2.end(),
                   std::inserter(fv, fv.begin()));
    break;
  }
  }

  return fv;
}

std::string fresh_var(const std::set<std::string> &used) {
  std::string alphabet = "abcdefghijklmnopqrstuvwxyz";

  for (std::size_t i = 0; i < alphabet.length(); i++) {
    std::string ch{alphabet[i]};
    if (used.find(ch) == used.end()) {
      return ch;
    }
  }

  const int maximum = 256;
  for (std::size_t i = 0; i < alphabet.length(); i++) {
    for (int j = 0; j < maximum; j++) {
      std::string ch{alphabet[i] + std::to_string(j)};
      if (used.find(ch) == used.end()) {
        return ch;
      }
    }
  }

  throw std::runtime_error("Failed to find fresh variable");
}

std::shared_ptr<Term> alpha_step(std::shared_ptr<Term> t, const std::string &x,
                                 const std::string &y) {
  // auto fv = free_vars(t);
  // bool is_free = fv.find(x) == fv.end();

  switch (t->kind) {
  case TermKind::Var: {
    if (t->var == x) {
      t->var = y;
    }
    break;
    // return t->var == x ? Term::make_var(y) : t;
  }
  case TermKind::Abs: {
    if (t->var == x) {
      t->var = y;
    }
    alpha_step(t->body, x, y);
    break;
    // return Term::make_abs(t->var == x ? y : t->var, alpha_step(t->body, x,
    // y));
  }
  case TermKind::App: {
    t->func = alpha_step(t->func, x, y);
    t->body = alpha_step(t->arg, x, y);
    break;
    // return Term::make_app(alpha_step(t->func, x, y), alpha_step(t->arg, x,
    // y));
  }
  }

  return t;
}

std::string alpha_red(std::shared_ptr<Term> t, const std::string &x) {
  // std::string str {t->to_string()};
  // vector -> set?
  // std::vector<char> v {str.begin(), str.end()};
  // std::set<std::string> used;
  auto used = used_vars(t);
  used.insert(x);
  const std::string y = fresh_var(used);
  alpha_step(t, x, y);
  return y;
}

std::shared_ptr<Term> substitute(std::shared_ptr<Term> m, const std::string &x,
                                 std::shared_ptr<Term> n) {
  auto fv2{free_vars(n)};
  auto bv2{bind_vars(n)};

  switch (m->kind) {
  case TermKind::Var: {
    // 1. x[x := t] = t
    // 2. y[x := t] = y,  y != x
    // 3. c[x := t] = c
    return m->var == x ? n : m;
  }
  case TermKind::Abs: {
    // 5. λx.t[x := t] = λx.t
    if (m->var == x) {
      break;
    }

    // 6. (λy.m')[x := n] = λy.(m'[x := n]),  x != y, x ∉ FV(m')? || y ∉ FV(n)
    while (fv2.find(m->var) != fv2.end()) {
      alpha_red(m, m->var);
    }
    while (bv2.find(m->var) != bv2.end()) {
      alpha_red(m, m->var);
    }

    m->body = substitute(m->body, x, n);
    return m;

    break;
  }
  case TermKind::App: {
    // 4. s1s2[x := t] = s1[x := t]s2[x := t]
    m->func = substitute(m->func, x, n);
    m->arg = substitute(m->arg, x, n);
    break;
  }
  }

  return m;
}

std::pair<std::shared_ptr<Term>, bool> beta_step(std::shared_ptr<Term> t) {

  switch (t->kind) {
  case TermKind::Var: {
    break;
  }
  case TermKind::Abs: {
    // Abs without N, going deeper
    return beta_step(t);
  }
  case TermKind::App: {
    if (t->func->kind == TermKind::Abs) {
      t = substitute(t->func->body, t->func->var, t->arg);
      return {t, true};
    }
    break;
  }
  }

  return {t, false};
  (void)t;
  throw std::runtime_error("beta_step: not implemented");
}

std::shared_ptr<Term> normalize(std::shared_ptr<Term> t, int max_steps) {
  std::pair<std::shared_ptr<Term>, bool> r;
  int i = 0;

  do {
    i++;
    r = beta_step(t);
  } while (r.second || i < max_steps);

  return t;
}

std::shared_ptr<Term> make_Z_combinator() {
  throw std::runtime_error("make_Z_combinator: not implemented");
}
