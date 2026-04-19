#include "../helpers/lambda_test_helpers.hpp"

int main(void) {
    auto ack = make_ackermann();
    auto term = Term::make_app(Term::make_app(ack, church(0)), church(0));
    return check_church_value(term, 1);
}
