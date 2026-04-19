#include "../helpers/lambda_test_helpers.hpp"

int main(void) {
    auto t = parse("\\x.\\y. x z");
    CHECK_TEST(free_vars(t) == std::set<std::string>{"z"}, "expected FV = {z}");
    return 0;
}
