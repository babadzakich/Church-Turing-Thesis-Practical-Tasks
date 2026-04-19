#include <stdlib.h>
#include <string.h>
#include "machine_turing.h"

#define MAX_STEPS 10000000      // Ограничение на число шагов
#define INITIAL_TAPE_SIZE 256   // Начальный размер ленты

TuringMachine* create_machine(const char *tape_initial) {
    TuringMachine* tm = malloc(sizeof(TuringMachine));
    if (tm == NULL) return NULL;

    int tape_len = strlen(tape_initial);
    if (tape_len < INITIAL_TAPE_SIZE - 1) {
        tm->tape = calloc(INITIAL_TAPE_SIZE, 1);
        tm->tape_size = INITIAL_TAPE_SIZE;
    } else {
        tm->tape = calloc(tape_len, 1);
        tm->tape_size = tape_len;
    }
    
    if (tm->tape == NULL) {
        free(tm);
        return NULL;
    }
    strncpy(tm->tape + 1, tape_initial, tm->tape_size);

    tm->transitions = calloc(8, sizeof(Transition));
    if (tm->transitions == NULL) {
        free(tm->tape);
        free(tm);
        return NULL;
    }
    tm->num_transitions = 0;
    tm->transitions_capacity = 8;

    tm->head = 1;
    tm->current_state = 1;
    return tm;
}

void add_transition(TuringMachine *tm, char current_state, char read_symbol, 
                    char write_symbol, char direction, char next_state) {
    if (tm->num_transitions == tm->transitions_capacity) {
        Transition* new_tr = calloc(tm->transitions_capacity * 2, sizeof(Transition));
        if (new_tr == NULL) return;

        memcpy(new_tr, tm->transitions, sizeof(Transition) * tm->num_transitions);
        free(tm->transitions);
        tm->transitions = new_tr;
        tm->transitions_capacity *= 2;
    }

    Transition* tr = &(tm->transitions[tm->num_transitions++]);
    tr->current_state = current_state;
    tr->read_symbol = read_symbol;
    tr->write_symbol = write_symbol;
    tr->direction = direction;
    tr->next_state = next_state;
}


void run_machine(TuringMachine *tm) {
    for (size_t _step = 0; _step < MAX_STEPS; _step++) {
        char state = tm->current_state;
        int head = tm->head;
        
        if (head >= tm->tape_size) {
            char* new_tape = calloc(tm->tape_size * 2, 1);
            if (new_tape == NULL) return;

            memcpy(new_tape, tm->tape, tm->tape_size);
            free(tm->tape);
            tm->tape = new_tape;
            tm->tape_size *= 2;
        }

        int transition_found = 0;
        for (int i = 0; i < tm->num_transitions; ++i) {
            Transition* tr = (tm->transitions + i);
            if (tr->current_state != state || tr->read_symbol != tm->tape[head]) continue;

            tm->tape[head] = tr->write_symbol;
            tm->current_state = tr->next_state;
            if (tr->direction == 'L') tm->head--;
            else if (tr->direction == 'R') tm->head++;
            transition_found = 1;
        }
        if (transition_found == 0) return;
    }
}

void destroy_machine(TuringMachine *tm) {
    free(tm->tape);
    tm->tape = NULL;
    free(tm->transitions);
    tm->transitions = NULL;
    free(tm);
}

char* process_string(const char *input) {
    TuringMachine* tm = create_machine(input);

    // установка разделителя
    add_transition(tm, 1, 'a', 'a', 'R', 1);
    add_transition(tm, 1, 'b', 'b', 'R', 1);
    add_transition(tm, 1,  0,  '#', 'L', 2);
    add_transition(tm, 2, 'a', 'a', 'L', 2);
    add_transition(tm, 2, 'b', 'b', 'L', 2);
    add_transition(tm, 2,  0,   0,  'R', 3);
    // копирование a
    add_transition(tm, 3, 'a',  1,  'R', 4);
    add_transition(tm, 4, 'a', 'a', 'R', 4);
    add_transition(tm, 4, 'b', 'b', 'R', 4);
    add_transition(tm, 4, '#', '#', 'R', 4);
    add_transition(tm, 4,  0,  'a', 'L', 6);
    // копирование b
    add_transition(tm, 3, 'b',  2,  'R', 5);
    add_transition(tm, 5, 'a', 'a', 'R', 5);
    add_transition(tm, 5, 'b', 'b', 'R', 5);
    add_transition(tm, 5, '#', '#', 'R', 5);
    add_transition(tm, 5,  0,  'b', 'L', 6);
    // возврат
    add_transition(tm, 6, 'a', 'a', 'L', 6);
    add_transition(tm, 6, 'b', 'b', 'L', 6);
    add_transition(tm, 6, '#', '#', 'L', 6);
    add_transition(tm, 6,  1,  'a', 'R', 3);
    add_transition(tm, 6,  2,  'b', 'R', 3);
    // конец
    add_transition(tm, 3, '#', '#', 'L', 0);

    run_machine(tm);

    char* answer = calloc(tm->tape_size + 1, 1);
    strncpy(answer, tm->tape + 1, tm->tape_size);

    destroy_machine(tm);
    return answer;
}