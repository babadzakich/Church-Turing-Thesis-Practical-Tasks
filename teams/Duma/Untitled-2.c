#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_PREDICATES 26
#define MAX_FUNCTIONS 26
#define MAX_CONSTANTS 26

int pred_arity[26];   // для 'A'..'Z', -1 если не предикат
int func_arity[26];   // для 'a'..'z', -1 если не функция
int const_flag[26];   // для 'a'..'z', 1 если константа

typedef enum {
    TOK_LPAREN, TOK_RPAREN, TOK_COMMA, TOK_NOT, TOK_AND, TOK_OR, TOK_IMPLY,
    TOK_QUANT_A, TOK_QUANT_E,
    TOK_PREDICATE,
    TOK_LOWER_ID,   
    TOK_VARIABLE,    
    TOK_END,
    TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char value;      // для PREDICATE и LOWER_ID
} Token;

typedef struct {
    const char *input;
    int pos;
    Token current;
} Lexer;

void init_tables() {
    for (int i = 0; i < 26; i++) {
        pred_arity[i] = -1;
        func_arity[i] = -1;
        const_flag[i] = 0;
    }
}

// разбор строки с предикатами
void parse_predicates_line(char *line) {
    char *token = strtok(line, " \t\n");
    while (token != NULL) {
        if (strlen(token) == 1 && isupper(token[0]) && token[0] != 'A' && token[0] != 'E') {
            char pred = token[0];
            token = strtok(NULL, " \t\n");
            if (token != NULL) {
                int arity = atoi(token);
                if (arity >= 0) {
                    pred_arity[pred - 'A'] = arity;
                }
            }
        }
        token = strtok(NULL, " \t\n");
    }
}

// разбор строки с функциями
void parse_functions_line(char *line) {
    char *token = strtok(line, " \t\n");
    while (token != NULL) {
        if (strlen(token) == 1 && islower(token[0])) {
            char func = token[0];
            token = strtok(NULL, " \t\n");
            if (token != NULL) {
                int arity = atoi(token);
                if (arity >= 0) {
                    func_arity[func - 'a'] = arity;
                }
            }
        }
        token = strtok(NULL, " \t\n");
    }
}

void parse_constants_line(char *line) {
    char *token = strtok(line, " \t\n");
    while (token != NULL) {
        if (strlen(token) == 1 && islower(token[0])) {
            const_flag[token[0] - 'a'] = 1;
        }
        token = strtok(NULL, " \t\n");
    }
}

void lexer_init(Lexer *lexer, const char *str) {
    lexer->input = str;
    lexer->pos = 0;
    lexer->current.type = TOK_ERROR;
}

void skip_whitespace(Lexer *lexer) {
    while (lexer->input[lexer->pos] == ' ' || lexer->input[lexer->pos] == '\t')
        lexer->pos++;
}

Token get_next_token(Lexer *lexer) {
    Token tok = {TOK_ERROR, 0};
    skip_whitespace(lexer);
    char c = lexer->input[lexer->pos];
    if (c == '\0') {
        tok.type = TOK_END;
        return tok;
    }
    switch (c) {
        case '(': tok.type = TOK_LPAREN; lexer->pos++; return tok;
        case ')': tok.type = TOK_RPAREN; lexer->pos++; return tok;
        case ',': tok.type = TOK_COMMA;  lexer->pos++; return tok;
        case '!': tok.type = TOK_NOT;    lexer->pos++; return tok;
        case '&': tok.type = TOK_AND;    lexer->pos++; return tok;
        case '|': tok.type = TOK_OR;     lexer->pos++; return tok;
        case '>': tok.type = TOK_IMPLY;  lexer->pos++; return tok;
    }
    if (c == 'A') { tok.type = TOK_QUANT_A; lexer->pos++; return tok; }
    if (c == 'E') { tok.type = TOK_QUANT_E; lexer->pos++; return tok; }
    if (isupper(c) && c != 'A' && c != 'E') {
        tok.type = TOK_PREDICATE;
        tok.value = c;
        lexer->pos++;
        return tok;
    }
    if (islower(c)) {
        tok.type = TOK_LOWER_ID;
        tok.value = c;
        lexer->pos++;
        return tok;
    }
    if (c == 'x' && lexer->input[lexer->pos+1] == '_') {
        int start = lexer->pos;
        lexer->pos += 2;
        if (isdigit(lexer->input[lexer->pos])) {
            while (isdigit(lexer->input[lexer->pos]))
                lexer->pos++;
            char next = lexer->input[lexer->pos];
            if (isalnum(next) && next != '_') {
                tok.type = TOK_ERROR;
                return tok;
            }
            tok.type = TOK_VARIABLE;
            return tok;
        }
    }
    tok.type = TOK_ERROR;
    return tok;
}

void lexer_advance(Lexer *lexer) {
    lexer->current = get_next_token(lexer);
}

Token lexer_peek(Lexer *lexer) {
    return lexer->current;
}

void lexer_consume(Lexer *lexer) {
    lexer_advance(lexer);
}

bool expect(Lexer *lexer, TokenType type) {
    if (lexer->current.type == type) {
        lexer_consume(lexer);
        return true;
    }
    return false;
}

bool parseFormula(Lexer *lexer) { return parseImplication(lexer); }

bool parseImplication(Lexer *lexer) {
    if (!parseDisjunction(lexer)) return false;
    while (lexer_peek(lexer).type == TOK_IMPLY) {
        lexer_consume(lexer);
        if (!parseDisjunction(lexer)) return false;
    }
    return true;
}

bool parseDisjunction(Lexer *lexer) {
    if (!parseConjunction(lexer)) return false;
    while (lexer_peek(lexer).type == TOK_OR) {
        lexer_consume(lexer);
        if (!parseConjunction(lexer)) return false;
    }
    return true;
}

bool parseConjunction(Lexer *lexer) {
    if (!parseUnary(lexer)) return false;
    while (lexer_peek(lexer).type == TOK_AND) {
        lexer_consume(lexer);
        if (!parseUnary(lexer)) return false;
    }
    return true;
}

bool parseUnary(Lexer *lexer) {
    Token tok = lexer_peek(lexer);
    if (tok.type == TOK_NOT) {
        lexer_consume(lexer);
        return parseUnary(lexer);
    }
    if (tok.type == TOK_LPAREN) {
        lexer_consume(lexer);
        if (!parseFormula(lexer)) return false;
        if (!expect(lexer, TOK_RPAREN)) return false;
        return true;
    }
    if (tok.type == TOK_QUANT_A || tok.type == TOK_QUANT_E) {
        lexer_consume(lexer);
        if (lexer_peek(lexer).type != TOK_VARIABLE) return false;
        lexer_consume(lexer);
        return parseUnary(lexer);
    }
    if (tok.type == TOK_PREDICATE) {
        return parseAtomic(lexer);
    }
    return false;
}

bool parseAtomic(Lexer *lexer) {
    Token pred = lexer_peek(lexer);
    if (pred.type != TOK_PREDICATE) return false;
    char pred_char = pred.value;
    lexer_consume(lexer);
    if (!expect(lexer, TOK_LPAREN)) return false;
    int arg_count = 0;
    if (lexer_peek(lexer).type != TOK_RPAREN) {
        if (!parseTerm(lexer)) return false;
        arg_count++;
        while (lexer_peek(lexer).type == TOK_COMMA) {
            lexer_consume(lexer);
            if (!parseTerm(lexer)) return false;
            arg_count++;
        }
    }
    if (!expect(lexer, TOK_RPAREN)) return false;
    int arity = pred_arity[pred_char - 'A'];
    if (arity == -1) return false;
    return (arg_count == arity);
}

bool parseTerm(Lexer *lexer) {
    Token tok = lexer_peek(lexer);
    if (tok.type == TOK_VARIABLE) {
        lexer_consume(lexer);
        return true;
    }
    if (tok.type == TOK_LOWER_ID) {
        char c = tok.value;
        lexer_consume(lexer);
        if (const_flag[c - 'a']) return true;          // константа
        int arity = func_arity[c - 'a'];
        if (arity == -1) return false;                 // неизвестный символ
        if (!expect(lexer, TOK_LPAREN)) return false;
        int arg_count = 0;
        if (lexer_peek(lexer).type != TOK_RPAREN) {
            if (!parseTerm(lexer)) return false;
            arg_count++;
            while (lexer_peek(lexer).type == TOK_COMMA) {
                lexer_consume(lexer);
                if (!parseTerm(lexer)) return false;
                arg_count++;
            }
        }
        if (!expect(lexer, TOK_RPAREN)) return false;
        return (arg_count == arity);
    }
    return false;
}

bool is_formula(const char *expr) {
    Lexer lexer;
    lexer_init(&lexer, expr);
    lexer_advance(&lexer);
    if (lexer.current.type == TOK_ERROR) return false;
    bool result = parseFormula(&lexer);
    if (result && lexer_peek(&lexer).type == TOK_END)
        return true;
    return false;
}

int main() {
    init_tables();
    FILE *input = fopen("input.txt", "r");
    char line[1024];
    if (fgets(line, sizeof(line), input)) parse_predicates_line(line);
    if (fgets(line, sizeof(line), input)) parse_functions_line(line);
    if (fgets(line, sizeof(line), input)) parse_constants_line(line);
    FILE *output = fopen("output.txt", "w");
    while (fgets(line, sizeof(line), input)) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) {
            fprintf(output, "NO\n");
            continue;
        }
        fprintf(output, "%s\n", is_formula(line) ? "YES" : "NO");
    }
    fclose(input);
    fclose(output);
    return 0;
}