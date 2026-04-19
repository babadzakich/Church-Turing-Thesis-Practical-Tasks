#include "../helpers/lambda_test_helpers.hpp"

int main(void) {
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
    auto term = T::make_app(T::make_app(z, fact_step), church(5));
    return check_church_value(term, 120);
}
