#pragma once
#include <string>
#include <memory>
#include <vector>
#include <set>

// ============================================================
//  term.hpp  —  Abstract Syntax Tree for Lambda Calculus
//  Этот файл менять не нужно.
// ============================================================

// Тип узла лямбда-терма
enum class TermKind {
    Var,  // переменная:      x
    Abs,  // абстракция:      λx.M
    App   // аппликация:      (M N)
};

// Узел дерева
struct Term {
    TermKind kind;

    // Var:  var  = имя переменной
    // Abs:  var  = имя параметра,  body = тело
    // App:  func = функция,        arg  = аргумент
    std::string var;
    std::shared_ptr<Term> body;   // Abs → тело;  не используется в Var/App
    std::shared_ptr<Term> func;   // App → левая часть
    std::shared_ptr<Term> arg;    // App → правая часть

    // --- Фабричные функции (используй их, не конструктор напрямую) ---

    static std::shared_ptr<Term> make_var(const std::string& name);
    static std::shared_ptr<Term> make_abs(const std::string& param,
                                          std::shared_ptr<Term> body);
    static std::shared_ptr<Term> make_app(std::shared_ptr<Term> func,
                                          std::shared_ptr<Term> arg);

    // Глубокая копия терма
    static std::shared_ptr<Term> clone(std::shared_ptr<Term> t);

    // Печать в читаемом виде: λx.(x y)
    std::string to_string() const;

    // α-эквивалентность: λx.x  ≡  λy.y
    static bool alpha_equiv(std::shared_ptr<Term> a,
                             std::shared_ptr<Term> b);
};
