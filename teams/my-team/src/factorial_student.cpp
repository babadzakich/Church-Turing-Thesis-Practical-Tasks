#include "factorial.hpp"
#include "eval.hpp"
#include "church.hpp"

#include <stdexcept>

// Продолжение задачи про лямбда-исчисление:
// реализуйте лямбда-терм факториала только через построение термов
// с использованием term/parser/church/eval.
//
// Функция должна вернуть терм, эквивалентный:
//   λn. factorial(n)
// где n — число Чёрча.
//
// Ожидаемая идея решения:
//   1. Построить step-функцию факториала как лямбда-терм.
//   2. Использовать make_Z_combinator() для рекурсии.
//   3. Вернуть готовую лямбда-функцию, не вычисляя её в uint64_t.
//
// Private-тесты будут сами применять ваш терм к church(n),
// нормализовывать результат и сравнивать его с нужным числом Чёрча.
std::shared_ptr<Term> make_factorial() {
    using T = Term;

    auto pred_n = T::make_app(church_pred(), T::make_var("n"));
    auto self_pred_n = T::make_app(T::make_var("self"), pred_n);
    auto mul_n = T::make_app(
        T::make_app(church_mul(), T::make_var("n")),
        self_pred_n);
    auto is_zero_n = T::make_app(church_is_zero(), T::make_var("n"));
    auto branch = T::make_app(
        T::make_app(
            T::make_app(church_if(), is_zero_n),
            church(1)),
        mul_n);
    auto fact_step = T::make_abs("self", T::make_abs("n", branch));

    auto z = make_Z_combinator();
    return T::make_app(z, fact_step);
}
