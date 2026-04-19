#include "../include/parser.hpp"
#include <cctype>
#include <stdexcept>

// ============================================================
//  Рекурсивный спуск
//  Грамматика (упрощённо):
//    expr   ::= abs | app
//    abs    ::= ('\' | 'λ') var '.' expr
//    app    ::= atom { atom }          (левоассоциативно)
//    atom   ::= var | '(' expr ')'
//    var    ::= [a-zA-Z][a-zA-Z0-9_]*
// ============================================================

struct Parser {
  const std::string &src;
  size_t pos;

  Parser(const std::string &s) : src(s), pos(0) {}

  void skip_ws() {
    while (pos < src.size() && std::isspace((unsigned char)src[pos]))
      ++pos;
  }

  char peek() {
    skip_ws();
    return pos < src.size() ? src[pos] : '\0';
  }

  char consume() {
    skip_ws();
    if (pos >= src.size())
      throw std::runtime_error("unexpected end of input");
    return src[pos++];
  }

  void expect(char c) {
    char got = consume();
    if (got != c) {
      std::string msg = "expected '";
      msg += c;
      msg += "', got '";
      msg += got;
      msg += "'";
      throw std::runtime_error(msg);
    }
  }

  // Идентификатор
  std::string parse_var() {
    skip_ws();
    if (pos >= src.size() || !std::isalpha((unsigned char)src[pos]))
      throw std::runtime_error("expected variable name at pos " +
                               std::to_string(pos));
    std::string name;
    while (pos < src.size() &&
           (std::isalnum((unsigned char)src[pos]) || src[pos] == '_'))
      name += src[pos++];
    return name;
  }

  // expr ::= abs | app
  std::shared_ptr<Term> parse_expr() {
    char c = peek();
    if (c == '\\' || (unsigned char)c == 0xce) // λ = 0xCE 0xBB in UTF-8
      return parse_abs();
    // Проверяем unicode λ
    if (pos + 1 < src.size() && (unsigned char)src[pos] == 0xCE &&
        (unsigned char)src[pos + 1] == 0xBB)
      return parse_abs();
    return parse_app();
  }

  // abs ::= ('\' | 'λ') var '.' expr
  std::shared_ptr<Term> parse_abs() {
    skip_ws();
    // Съесть λ или '\'
    if ((unsigned char)src[pos] == 0xCE) {
      pos += 2; // UTF-8 λ
    } else {
      pos++; // '\'
    }
    std::string param = parse_var();
    skip_ws();
    expect('.');
    auto body = parse_expr();
    return Term::make_abs(param, body);
  }

  // app ::= atom { atom }
  std::shared_ptr<Term> parse_app() {
    auto t = parse_atom();
    while (true) {
      char c = peek();
      if (c == '\0' || c == ')')
        break;
      // Не абстракция на верхнем уровне аппликации — это атом
      if (c == '\\' || ((unsigned char)c == 0xCE && pos + 1 < src.size() &&
                        (unsigned char)src[pos + 1] == 0xBB)) {
        // λ как аргумент — берём его как атом в скобках нет,
        // стандартная практика: аппликация до конца
        auto rhs = parse_abs();
        t = Term::make_app(t, rhs);
        break;
      }
      auto rhs = parse_atom();
      t = Term::make_app(t, rhs);
    }
    return t;
  }

  // atom ::= var | '(' expr ')'
  std::shared_ptr<Term> parse_atom() {
    char c = peek();
    if (c == '(') {
      pos++;
      auto t = parse_expr();
      expect(')');
      return t;
    }
    return Term::make_var(parse_var());
  }
};

std::shared_ptr<Term> parse(const std::string &input) {
  Parser p(input);
  auto t = p.parse_expr();
  p.skip_ws();
  if (p.pos != input.size())
    throw std::runtime_error("unexpected token at pos " +
                             std::to_string(p.pos));
  return t;
}
