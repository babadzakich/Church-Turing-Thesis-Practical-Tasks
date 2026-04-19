#include "../include/church.hpp"
#include <stdexcept>

// λf.λx. f (f (... (f x) ...))
std::shared_ptr<Term> church(int n) {
  // Тело: f^n x  — строим изнутри
  auto x = Term::make_var("x");
  auto body = x;
  for (int i = 0; i < n; ++i)
    body = Term::make_app(Term::make_var("f"), body);
  return Term::make_abs("f", Term::make_abs("x", body));
}

int unchurch(std::shared_ptr<Term> t) {
  // Применяем t к функции-счётчику и нулю
  // f = λn. n+1  как C++ лямбда нам не нужна — нормализуем:
  // Строим (t (λn.succ_marker n) zero) и считаем вложенность
  // Простой способ: нормализуем (t (λx.λy.y x) (λx.x)) и считаем
  // слои λ → нет, проще считать через специальный терм.
  //
  // Используем: t (λn.λf.λx. f (n f x)) (λf.λx.x)
  // т.е. применяем succ и zero и смотрим результат
  // Ещё проще: применяем к "счётчику" и к нулю напрямую.
  // church(n) f x = f^n x
  // Подставим f = λa.a+1  и x = 0 — но у нас нет натуральных...
  //
  // Поэтому: нормализуем t и вручную считаем аппликации f.
  // t нормализован → λf.λx. f (f ... (f x))
  // Идём вглубь и считаем.

  // Ожидаем: Abs(f, Abs(x, body))
  if (t->kind != TermKind::Abs)
    throw std::runtime_error("unchurch: not an abstraction");
  std::string fname = t->var;
  auto inner = t->body;
  if (inner->kind != TermKind::Abs)
    throw std::runtime_error("unchurch: expected λf.λx...");
  std::string xname = inner->var;
  auto body = inner->body;

  // Считаем: body = f (f (... (f x)))  — сколько f
  int count = 0;
  auto cur = body;
  while (cur->kind == TermKind::App) {
    if (cur->func->kind != TermKind::Var || cur->func->var != fname)
      throw std::runtime_error("unchurch: unexpected structure");
    cur = cur->arg;
    count++;
  }
  if (cur->kind != TermKind::Var || cur->var != xname)
    throw std::runtime_error("unchurch: body doesn't end in x");
  return count;
}

// --- Стандартные комбинаторы ---

std::shared_ptr<Term> church_zero() {
  // λf.λx.x
  return Term::make_abs("f", Term::make_abs("x", Term::make_var("x")));
}

std::shared_ptr<Term> church_succ() {
  // λn.λf.λx. f (n f x)
  auto n_f_x =
      Term::make_app(Term::make_app(Term::make_var("n"), Term::make_var("f")),
                     Term::make_var("x"));
  auto body = Term::make_app(Term::make_var("f"), n_f_x);
  return Term::make_abs("n", Term::make_abs("f", Term::make_abs("x", body)));
}

std::shared_ptr<Term> church_add() {
  // λm.λn.λf.λx. m f (n f x)
  auto nfx =
      Term::make_app(Term::make_app(Term::make_var("n"), Term::make_var("f")),
                     Term::make_var("x"));
  auto body = Term::make_app(
      Term::make_app(Term::make_var("m"), Term::make_var("f")), nfx);
  return Term::make_abs(
      "m", Term::make_abs("n", Term::make_abs("f", Term::make_abs("x", body))));
}

std::shared_ptr<Term> church_mul() {
  // λm.λn.λf. m (n f)
  auto nf = Term::make_app(Term::make_var("n"), Term::make_var("f"));
  auto body = Term::make_app(Term::make_var("m"), nf);
  return Term::make_abs("m", Term::make_abs("n", Term::make_abs("f", body)));
}

std::shared_ptr<Term> church_pred() {
  // λn.λf.λx. n (λg.λh. h (g f)) (λu.x) (λu.u)
  auto gf = Term::make_app(Term::make_var("g"), Term::make_var("f"));
  auto h_gf = Term::make_app(Term::make_var("h"), gf);
  auto pair_step = Term::make_abs("g", Term::make_abs("h", h_gf));

  auto const_x = Term::make_abs("u", Term::make_var("x"));
  auto id = Term::make_abs("u", Term::make_var("u"));

  auto body = Term::make_app(
      Term::make_app(Term::make_app(Term::make_var("n"), pair_step), const_x),
      id);
  return Term::make_abs("n", Term::make_abs("f", Term::make_abs("x", body)));
}

std::shared_ptr<Term> church_is_zero() {
  // λn. n (λx.λa.λb.b) (λa.λb.a)
  auto false_val =
      Term::make_abs("a", Term::make_abs("b", Term::make_var("b")));
  auto const_false = Term::make_abs("x", false_val);
  auto true_val = Term::make_abs("a", Term::make_abs("b", Term::make_var("a")));
  auto body = Term::make_app(Term::make_app(Term::make_var("n"), const_false),
                             true_val);
  return Term::make_abs("n", body);
}

std::shared_ptr<Term> church_true() {
  return Term::make_abs("a", Term::make_abs("b", Term::make_var("a")));
}

std::shared_ptr<Term> church_false() {
  return Term::make_abs("a", Term::make_abs("b", Term::make_var("b")));
}

std::shared_ptr<Term> church_if() {
  // λp.λa.λb. p a b
  auto body =
      Term::make_app(Term::make_app(Term::make_var("p"), Term::make_var("a")),
                     Term::make_var("b"));
  return Term::make_abs("p", Term::make_abs("a", Term::make_abs("b", body)));
}
