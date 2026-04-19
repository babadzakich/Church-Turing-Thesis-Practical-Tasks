#include "../include/ackermann.hpp"

#include "../include/church.hpp"
#include "../include/eval.hpp"

std::shared_ptr<Term> make_ackermann() {
  // A(0, n) = n + 1
  // A(m + 1, 0) = A(m, 1)
  // A(m + 1, n + 1) = A(m, A(m + 1, n))
  return Term::make_abs("m", Term::make_var("m"));
}
