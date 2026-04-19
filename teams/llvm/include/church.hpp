#pragma once
#include "term.hpp"

// ============================================================
//  church.hpp  —  Числа Чёрча и стандартные комбинаторы
//  Этот файл менять не нужно. Реализация — в church.cpp (дана).
// ============================================================

// Число Чёрча: n  →  λf.λx. f(f(...(f x)...))  (n раз)
std::shared_ptr<Term> church(int n);

// Декодировать число Чёрча обратно в int
// (нормализует терм и считает аппликации)
// Бросает std::runtime_error если терм не является числом Чёрча.
int unchurch(std::shared_ptr<Term> t);

// Стандартные комбинаторы (уже построены как термы)
std::shared_ptr<Term> church_zero();   // λf.λx.x
std::shared_ptr<Term> church_succ();   // λn.λf.λx. f (n f x)
std::shared_ptr<Term> church_add();    // λm.λn.λf.λx. m f (n f x)
std::shared_ptr<Term> church_mul();    // λm.λn.λf. m (n f)
std::shared_ptr<Term> church_pred();   // λn.λf.λx. n (λg.λh.h(g f)) (λu.x) (λu.u)
std::shared_ptr<Term> church_is_zero();// λn. n (λx.λa.λb.b) (λa.λb.a)

// Булевы значения
std::shared_ptr<Term> church_true();   // λa.λb.a
std::shared_ptr<Term> church_false();  // λa.λb.b
std::shared_ptr<Term> church_if();     // λp.λa.λb. p a b
