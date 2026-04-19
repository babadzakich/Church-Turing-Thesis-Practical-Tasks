#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "machine_turing.h"

#define MAX_STEPS 10000000      // Ограничение на число шагов
#define INITIAL_TAPE_SIZE 256 // Начальный размер ленты


TuringMachine* create_machine(const char *tape_initial) {
    TuringMachine* tm = (TuringMachine*)malloc(sizeof(TuringMachine));
    if (tm == NULL) {
        return NULL;
    }
    int len = strlen(tape_initial);
    int SIZE = INITIAL_TAPE_SIZE;
    if (len*2+10 >= INITIAL_TAPE_SIZE)
    {
        SIZE = len*2+10;
    }
    tm->tape_size = SIZE;
    tm->tape = (char*)malloc(sizeof(char) * SIZE);
    if (tm->tape == NULL) {
        return NULL;
    }
    for (int i = 0; i < SIZE; i++) {
        tm->tape[i] = '_';
    }

    for (int i = 1; i <= len; i++) {
        tm->tape[i] = tape_initial[i-1];
    }
    tm->head = 1;
    tm->current_state = 1;
    Transition *transitions = (Transition*)malloc(sizeof(Transition));
    if (transitions == NULL) {
        return NULL;
    }
    tm->num_transitions = 1;
    tm->transitions_capacity = 1;
    tm->transitions = transitions;

    return tm;
}

void add_transition(TuringMachine *tm, char current_state, char read_symbol, 
                    char write_symbol, char direction, char next_state) {
    if ((tm->num_transitions+1) >= tm->transitions_capacity) {
       tm->transitions_capacity = (tm->transitions_capacity==0) ? 4 : tm->transitions_capacity * 2;
       Transition* new_transitions = (Transition*)realloc(tm->transitions, sizeof(Transition) * tm->transitions_capacity);
       tm->transitions = new_transitions;
    }
    tm->transitions[tm->num_transitions].current_state = current_state;
    tm->transitions[tm->num_transitions].read_symbol = read_symbol;
    tm->transitions[tm->num_transitions].write_symbol = write_symbol;
    tm->transitions[tm->num_transitions].direction = direction;
    tm->transitions[tm->num_transitions].next_state = next_state;
    tm->num_transitions++;
}


void run_machine(TuringMachine *tm) {
    int q = tm->current_state;
    int step = 0;

    int out_of_tape = 0;
    int is_found = 0;
    while (q != 0 && step <= MAX_STEPS) {

        if (!(0 <= tm->head && tm->head <= tm->tape_size)) {
            out_of_tape = 1;
            break;
        }

        for (int i = 1; i <= tm->num_transitions; i++) {
            if ((tm->transitions[i].current_state == q) && (tm->tape[tm->head] == tm->transitions[i].read_symbol)) {
                is_found = 1;
                tm->tape[tm->head] = tm->transitions[i].write_symbol;
                q = tm->transitions[i].next_state;
                if (tm->transitions[i].direction == 'R') {
                    tm->head++;
                }
                else if (tm->transitions[i].direction == 'L') {
                    tm->head--;
                }
                break;
            }
        }

        if (!is_found) {
            break;
        }
        step++;
    }

    if (is_found == 0 || out_of_tape == 1)
    {
        tm->current_state = REJECT;
    }
    tm->current_state = ACCEPT;
}

void destroy_machine(TuringMachine *tm) {
    free(tm->tape);
    free(tm->transitions);
    free(tm);
}

char* process_string(const char *input) {
    TuringMachine* tm;
    tm = create_machine(input);

    add_transition(tm, 1, 'a', 'a', 'R', 1);
    add_transition(tm, 1, 'b', 'b', 'R', 1);
    add_transition(tm, 1, '_', '#', 'L', 2);

    add_transition(tm, 2, 'a', 'a', 'L', 2);
    add_transition(tm, 2, 'b', 'b', 'L', 2);
    add_transition(tm, 2, 'c', 'c', 'R', 3);
    add_transition(tm, 2, 'd', 'd', 'R', 3);
    add_transition(tm, 2, '_', '_', 'R', 3);

    add_transition(tm, 3, 'a', 'c', 'R', 4);
    add_transition(tm, 3, 'b', 'd', 'R', 8);

    add_transition(tm, 4, 'a', 'a', 'R', 4);
    add_transition(tm, 4, 'b', 'b', 'R', 4);
    add_transition(tm, 4, '#', '#', 'R', 5);

    add_transition(tm, 5, '_', 'a', 'L', 7);
    add_transition(tm, 5, 'a', 'a', 'R', 5);
    add_transition(tm, 5, 'b', 'b', 'R', 5);


    add_transition(tm, 3, '#', '#', 'L', 6);
    add_transition(tm, 6, 'd', 'b', 'L', 6);
    add_transition(tm, 6, 'c', 'a', 'L', 6);
    add_transition(tm, 6, '_', '_', 'R', 0);

    add_transition(tm, 7, 'a', 'a', 'L', 7);
    add_transition(tm, 7, 'b', 'b', 'L', 7);
    add_transition(tm, 7, '#', '#', 'L', 2);

    add_transition(tm, 8, 'a', 'a', 'R', 8);
    add_transition(tm, 8, 'b', 'b', 'R', 8);
    add_transition(tm, 8, '#', '#', 'R', 9);

    add_transition(tm, 9, '_', 'b', 'L', 7);
    add_transition(tm, 9, 'a', 'a', 'R', 9);
    add_transition(tm, 9, 'b', 'b', 'R', 9);

    run_machine(tm);

    int len = strlen(tm->tape);
    char* str = (char*)malloc((1+len)*sizeof(char));
    int i = 0;
    for (; i < len; i++)
    {
        if (tm->tape[i+1] == '_')
            break;
        str[i]=tm->tape[i+1];
    }
    str[i]='\0';

    return str;
}

