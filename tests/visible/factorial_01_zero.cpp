#include "../helpers/lambda_test_helpers.hpp"

int main(void) {
    auto fact = make_factorial();
    auto term = Term::make_app(fact, church(0));
    return check_church_value(term, 1);
}
