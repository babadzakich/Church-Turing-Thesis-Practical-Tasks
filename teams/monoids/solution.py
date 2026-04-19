import re
import sys

TOKENS = [
    ('VAR',    r'x_\d+'),
    ('LPAREN', r'\('),
    ('RPAREN', r'\)'),
    ('COMMA',  r','),
    ('AND',    r'&'),
    ('OR',     r'\|'),
    ('NOT',    r'!'),
    ('IMP',    r'>'),
    ('QUANT',  r'[AE]'),
    ('PRED',   r'[B-DF-Z]'),
    ('ID',     r'[a-z]'),
    ('SKIP',   r'[ \t\n\r]+'),
]

_COMPILED = [(name, re.compile(pat)) for name, pat in TOKENS]


def tokenize(expr):
    tokens = []
    pos = 0
    while pos < len(expr):
        matched = False
        for token_type, regex in _COMPILED:
            m = regex.match(expr, pos)
            if m:
                if token_type != 'SKIP':
                    tokens.append((token_type, m.group(0)))
                pos += len(m.group(0))
                matched = True
                break
        if not matched:
            raise ValueError(f"Unexpected char at {pos}: {expr[pos]!r}")
    return tokens


class FormulaParser:
    def __init__(self, preds, funcs, consts):
        self.preds = preds
        self.funcs = funcs
        self.consts = consts
        self.tokens = []
        self.pos = 0

    def peek(self):
        return self.tokens[self.pos] if self.pos < len(self.tokens) else (None, None)

    def consume(self, expected_type=None):
        t_type, t_val = self.peek()
        if expected_type and t_type != expected_type:
            raise ValueError(f"Expected {expected_type}, got {t_type!r} ({t_val!r})")
        self.pos += 1
        return t_val

    def parse_term(self):
        t_type, t_val = self.peek()

        if t_type == 'VAR':
            return self.consume()

        if t_type == 'ID':
            name = self.consume()
            if name in self.consts and self.peek()[0] != 'LPAREN':
                return name
            if name in self.funcs:
                arity = self.funcs[name]
                self.consume('LPAREN')
                for i in range(arity):
                    self.parse_term()
                    if i < arity - 1:
                        self.consume('COMMA')
                self.consume('RPAREN')
                return name
        raise ValueError("Invalid term")

    def parse_formula(self):
        t_type, t_val = self.peek()

        if t_type == 'NOT':
            self.consume()
            self.parse_formula()
            return

        if t_type == 'QUANT':
            self.consume()
            self.consume('VAR')
            self.parse_formula()
            return

        if t_type == 'LPAREN':
            self.consume('LPAREN')
            self.parse_formula()
            next_t, _ = self.peek()
            if next_t in ('AND', 'OR', 'IMP'):
                self.consume()
                self.parse_formula()
            self.consume('RPAREN')
            return

        if t_type == 'PRED':
            name = self.consume()
            if name not in self.preds:
                raise ValueError(f"Unknown predicate: {name!r}")
            arity = self.preds[name]
            self.consume('LPAREN')
            for i in range(arity):
                self.parse_term()
                if i < arity - 1:
                    self.consume('COMMA')
            self.consume('RPAREN')
            return

        raise ValueError("Invalid formula start")

    def check(self, expr):
        try:
            self.tokens = tokenize(expr)
            self.pos = 0
            self.parse_formula()
            return "YES" if self.pos == len(self.tokens) else "NO"
        except Exception:
            return "NO"


def parse_pairs(line):
    parts = line.split()
    result = {}
    for i in range(0, len(parts) - 1, 2):
        result[parts[i]] = int(parts[i + 1])
    return result


def main():
    input_path = sys.argv[1] if len(sys.argv) > 1 else 'input.txt'

    with open(input_path, 'r') as f:
        lines = [line.rstrip('\n') for line in f]

    preds  = parse_pairs(lines[0]) if len(lines) > 0 and lines[0].strip() else {}
    funcs  = parse_pairs(lines[1]) if len(lines) > 1 and lines[1].strip() else {}
    consts = set(lines[2].split()) if len(lines) > 2 and lines[2].strip() else set()

    parser = FormulaParser(preds, funcs, consts)

    for expr in lines[3:]:
        expr = expr.strip()
        if expr:
            print(parser.check(expr))


if __name__ == '__main__':
    main()
