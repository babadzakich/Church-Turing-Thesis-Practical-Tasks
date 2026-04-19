#include "../helpers/lambda_test_helpers.hpp"

int main(void) {
    auto result = normalize(parse("(\\x.\\y.x) a b"));
    const bool ok = result->kind == TermKind::Var && result->var == "a";
    CHECK_TEST(ok, "expected result variable a");
    return 0;
}
