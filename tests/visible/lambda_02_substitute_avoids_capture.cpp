#include "../helpers/lambda_test_helpers.hpp"

int main(void) {
    auto t = parse("\\y.x");
    auto result = substitute(t, "x", Term::make_var("y"));
    const bool ok = result->kind == TermKind::Abs &&
                    result->var != "y" &&
                    free_vars(result).count("y") == 1;
    CHECK_TEST(ok, "capture avoidance failed");
    return 0;
}
