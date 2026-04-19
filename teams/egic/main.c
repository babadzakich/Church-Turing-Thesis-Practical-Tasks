#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct functional_symbol {
    char symbol;
    int mestnost_func;
} functional_symbol;

typedef struct pred_symbol {
    char symbol;
    int mestnost_pred;
} pred_symbol;

int check_symbol(char c, functional_symbol* func, pred_symbol* pred) {
    for (int i = 0; i < 10; i++) {
        if (c == func[i].symbol) return func[i].mestnost_func;
        if (c == pred[i].symbol) return pred[i].mestnost_pred;
    }
    return 0;
}

int main() {
    functional_symbol func[10];
    pred_symbol pred[10];
    char con[10];
    char line[1001];

    fgets(line, 1001, stdin);
    char *token = strtok(line, " \n");
    int i = 0;
    while (token != NULL && i < 10) {
        pred[i].symbol = token[0];
        pred[i].mestnost_pred = token[1] - '0';
        i++;
        token = strtok(NULL, " \n");
    }

    fgets(line, 1001, stdin);
    token = strtok(line, " \n");
    i = 0;
    while (token != NULL && i < 10) {
        func[i].symbol = token[0];
        func[i].mestnost_func = token[1] - '0';
        i++;
        token = strtok(NULL, " \n");
    }

    fgets(line, 1001, stdin);
    token = strtok(line, " \n");
    i = 0;
    while (token != NULL && i < 10) {
        con[i] = token[0];
        i++;
        token = strtok(NULL, " \n");
    }

    char in[1001];
    while (fgets(in, 1001, stdin) != NULL) {
        in[strcspn(in, "\n")] = '\0';
        if (strlen(in) == 0) continue;

        int count_ls = 0, count_rs = 0, count_niz = 0, count_x = 0, count_amper = 0;
        int valid = 1;

        for (size_t len = 0; len < strlen(in); len++) {
            char c = in[len];
            if (c == ' ') continue;
            if (c == 'E' || c == 'A' || c == '&' || c == '|' || c == '!') continue;
            else if (c == '(') count_ls++;
            else if (c == ')') count_rs++;
            else if (c == '_') count_niz++;
            else if (c == 'x') count_x++;
            else if (c == '&') count_amper++;
            else if (c == ',') continue;
            else {
                int res = check_symbol(c, func, pred);
                if (res == 0) {
                    int const_flag = 0;
                    for (int k = 0; k < 10; k++) {
                        if (c == con[k]) const_flag = 1;
                    }
                    if (!const_flag) {
                        printf("NO\n");
                        valid = 0;
                        break;
                    }
                }
            }
        }
        if (!valid) continue;


        if (count_rs != count_ls || count_niz != count_x) {
            printf("NO");
        } else {
            printf("YES");
        }
    }
    return 0;
}