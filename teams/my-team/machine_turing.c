#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "machine_turing.h"

#define MAX_STEPS 10000000      // Ограничение на число шагов
#define INITIAL_TAPE_SIZE 256 // Начальный размер ленты

#ifndef IS_DEF_SIG
#define IS_DEF_SIG
#define START_STATE 1 // Начальное состояние МТ
#define END_STATE 0 // Конечное состояние МТ
#define LEFT 'L'
#define RIGHT 'R'
#endif


TuringMachine* create_machine(const char* tape_initial) {
    TuringMachine *tm = (TuringMachine*)malloc(sizeof(TuringMachine));

    int tape_init_len = (int)strlen(tape_initial);
    tm->tape = calloc(tape_init_len + 1, sizeof(char));
    memmove(tm->tape, tape_initial, sizeof(char) * tape_init_len);
    tm->tape_size = tape_init_len;

    tm->head = 0;
    tm->current_state = START_STATE;

    tm->transitions = (Transition*)malloc(sizeof(Transition) * INITIAL_TAPE_SIZE);
    tm->transitions_capacity = INITIAL_TAPE_SIZE;
    tm->num_transitions = 0;

    return tm;
}

void add_transition(TuringMachine* tm, char current_state, char read_symbol, char write_symbol, char direction, char next_state) {
    if (tm->num_transitions == tm->transitions_capacity) {
        tm->transitions_capacity *= 2;
        tm->transitions = (Transition*)realloc(tm->transitions, sizeof(Transition) * tm->transitions_capacity);
    }
    Transition* tr = tm->transitions + tm->num_transitions++;
    
    tr->current_state = current_state;
    tr->read_symbol = read_symbol;
    tr->write_symbol = write_symbol;
    tr->direction = direction;
    tr->next_state = next_state;
}

// костыли сигнатуры, было бы хорошо, если бы было два массива, + и -
static inline void realloc_left(TuringMachine* tm) {
    tm->head += tm->tape_size;
    char* new_tape = (char*)calloc(tm->tape_size * 2, sizeof(char));
    memmove(new_tape + tm->tape_size, tm->tape, sizeof(char) * tm->tape_size);
    tm->tape_size *= 2;
    tm->tape = new_tape;
}

static inline void realloc_right(TuringMachine* tm) {
    tm->tape = realloc(tm->tape, sizeof(char) * tm->tape_size * 2);
    memset(tm->tape + tm->tape_size, 0, tm->tape_size);
    tm->tape_size *= 2;
}   


// поиск команды (для возможно написать адекватную структуру)
static inline int get_tr(Transition* tr, int tr_size, char current_state, char read_symbol) {
    for (int i = 0; i < tr_size; i++)
        if (tr[i].current_state == current_state && tr[i].read_symbol == read_symbol)
            return i;
    return -1;
}

// переход к следующему состоянию
static inline void next_state(TuringMachine* tm) {
    int idx_tr = get_tr(tm->transitions, tm->num_transitions, tm->current_state, tm->tape[tm->head]);
    if (idx_tr == -1) // если нет, то всё UB
        return;
    //двигаемся, записываем
    Transition* cur_tr = tm->transitions + idx_tr;
    tm->current_state = cur_tr->next_state;
    tm->tape[tm->head] = cur_tr->write_symbol;
    switch (cur_tr->direction) {
        case LEFT:
            if (tm->head == 0)
                realloc_left(tm);
            tm->head--;
            break;
        case RIGHT:
            if (tm->head == tm->tape_size - 1)
                realloc_right(tm);
            tm->head++;
            break;
    }
}

void run_machine(TuringMachine* tm) {
    int count_steps = 0;
    while (tm->current_state != END_STATE && count_steps != MAX_STEPS) {
        next_state(tm);
        count_steps++;
    }
}

void destroy_machine(TuringMachine* tm) {
    free(tm->tape);
    free(tm->transitions);
    free(tm);
}

char* process_string(const char* input) {
    TuringMachine* tm = create_machine(input);
    add_transition(tm, START_STATE, 'a', 0, RIGHT, 12);
    add_transition(tm, START_STATE, 'b', 0, RIGHT, 13);
    add_transition(tm, START_STATE, 0, '#',  0, END_STATE);

    add_transition(tm, 12, 'a', 'a', RIGHT, 12);
    add_transition(tm, 12, 'b', 'b', RIGHT, 12);
    add_transition(tm, 13, 'a', 'a', RIGHT, 13);
    add_transition(tm, 13, 'b', 'b', RIGHT, 13);
    add_transition(tm, 12, 0, '#', RIGHT, 2);
    add_transition(tm, 13, 0, '#', RIGHT, 3);

    add_transition(tm, 10, 'a', 0, RIGHT, 2); 
    add_transition(tm, 10, 'b', 0, RIGHT, 3);

    add_transition(tm, 2, 'a', 'a', RIGHT, 2); // a: qa -> qa
    add_transition(tm, 2, 'b', 'b', RIGHT, 2); // b: qa -> qa
    add_transition(tm, 2, '#', '#', RIGHT, 2); // #: qa -> qa

    add_transition(tm, 3, 'a', 'a', RIGHT, 3); // a: qb -> qb
    add_transition(tm, 3, 'b', 'b', RIGHT, 3); // b: qb -> qb
    add_transition(tm, 3, '#', '#', RIGHT, 3); // #: qb -> qb

    add_transition(tm, 2, 0, 'a', LEFT, 4); // qaa: 4
    add_transition(tm, 3, 0, 'b', LEFT, 5); // qbb: 5

    add_transition(tm, 4, 'a', 'a', LEFT, 4); // a: qaa -> qaa
    add_transition(tm, 4, 'b', 'b', LEFT, 4); // b: qaa -> qaa
    add_transition(tm, 4, '#', '#', LEFT, 4); // #: qaa -> qaa

    add_transition(tm, 5, 'a', 'a', LEFT, 5); // a: qbb -> qbb
    add_transition(tm, 5, 'b', 'b', LEFT, 5); // b: qbb -> qbb
    add_transition(tm, 5, '#', '#', LEFT, 5); // #: qbb -> qbb

    add_transition(tm, 4, 0, 'a', RIGHT, 10);
    add_transition(tm, 5, 0, 'b', RIGHT, 10);

    run_machine(tm);

    // printf("%s(%d)\n", tm->tape, tm->tape_size);
    return tm->tape;
}

// int main() {
//     process_string("abab");
// }