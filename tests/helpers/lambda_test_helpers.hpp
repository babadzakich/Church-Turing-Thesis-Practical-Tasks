#ifndef LAMBDA_TEST_HELPERS_HPP
#define LAMBDA_TEST_HELPERS_HPP

#include "ackermann.hpp"
#include "church.hpp"
#include "eval.hpp"
#include "factorial.hpp"
#include "parser.hpp"

#include <cstring>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <utility>

#define CHECK_TEST(cond, msg)            \
    do {                                 \
        if (!(cond)) {                   \
            std::cerr << msg << "\n";    \
            return 1;                    \
        }                                \
    } while (0)

inline int check_church_value(std::shared_ptr<Term> term, int expected) {
    auto result = normalize(term, 200000);
    const auto actual = unchurch(result);
    CHECK_TEST(actual == expected, "unexpected Church numeral");
    return 0;
}

#endif
