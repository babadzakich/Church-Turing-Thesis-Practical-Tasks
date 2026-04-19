#include <stdlib.h>
#include <string.h>
#include "machine_turing.h"

#define MAX_STEPS 10000000
#define INITIAL_TAPE_SIZE 256
#define INITIAL_TRANSITION_CAPACITY 32

TuringMachine *create_machine(const char *tape_initial)
{
    TuringMachine *machine = malloc(sizeof(TuringMachine));
    if (!machine)
        return NULL;

    int input_len = (int)strlen(tape_initial);
    int tape_size = input_len * 4 + 64;
    if (tape_size < INITIAL_TAPE_SIZE)
        tape_size = INITIAL_TAPE_SIZE;

    machine->tape_size = tape_size;
    machine->tape = calloc(tape_size, sizeof(char));
    if (!machine->tape)
    {
        free(machine);
        return NULL;
    }
    memcpy(machine->tape, tape_initial, input_len);

    machine->head = 0;
    machine->current_state = '0';
    machine->transitions = malloc(sizeof(Transition) * INITIAL_TRANSITION_CAPACITY);
    if (!machine->transitions)
    {
        free(machine->tape);
        free(machine);
        return NULL;
    }
    machine->num_transitions = 0;
    machine->transitions_capacity = INITIAL_TRANSITION_CAPACITY;

    return machine;
}

void add_transition(TuringMachine *tm, char current_state, char read_symbol,
                    char write_symbol, char direction, char next_state)
{
    if (tm->num_transitions >= tm->transitions_capacity)
    {
        tm->transitions_capacity *= 2;
        tm->transitions = realloc(tm->transitions,
                                  tm->transitions_capacity * sizeof(Transition));
    }
    Transition tr = {current_state, read_symbol, write_symbol, direction, next_state};
    tm->transitions[tm->num_transitions++] = tr;
}

void run_machine(TuringMachine *tm)
{
    for (int step = 0; step < MAX_STEPS; step++)
    {
        if (tm->current_state == 'A' || tm->current_state == 'R')
            return;

        if (tm->head < 0)
            return;

        char symbol = (tm->head < tm->tape_size) ? tm->tape[tm->head] : '\0';

        int found = 0;
        for (int i = 0; i < tm->num_transitions; i++)
        {
            Transition *tr = &tm->transitions[i];
            if (tr->current_state == tm->current_state && tr->read_symbol == symbol)
            {
                tm->tape[tm->head] = tr->write_symbol;
                if (tr->direction == 'R')
                    tm->head++;
                else if (tr->direction == 'L')
                    tm->head--;
                tm->current_state = tr->next_state;
                found = 1;
                break;
            }
        }

        if (!found)
            return;

        if (tm->head >= tm->tape_size)
        {
            int new_size = tm->tape_size * 2;
            tm->tape = realloc(tm->tape, new_size);
            memset(tm->tape + tm->tape_size, 0, new_size - tm->tape_size);
            tm->tape_size = new_size;
        }
    }
}

void destroy_machine(TuringMachine *tm)
{
    free(tm->tape);
    free(tm->transitions);
    free(tm);
}

/*
 * Алгоритм копирования строки S -> S#S на машине Тьюринга.
 *
 * Лента инициализируется как "@<input>\0..." где '@' - левый ограничитель.
 * Состояния:
 *   '0' - идём вправо до конца, пишем '#'
 *   '1' - идём влево до '@', затем переходим в '2'
 *   '2' - ищем непомеченный символ (a->X, b->Y); '#' -> начало восстановления
 *   '3' - несём 'a' вправо до первого пустого, пишем 'a', идём влево ('5')
 *   '4' - несём 'b' вправо до первого пустого, пишем 'b', идём влево ('5')
 *   '5' - идём влево до '@', переходим в '2'
 *   '7' - идём влево до '@' для восстановления
 *   '8' - восстанавливаем метки: X->a, Y->b; при '#' -> принятие
 *   'A' - принятие
 */
char *process_string(const char *input)
{
    if (!input)
        input = "";

    int input_len = (int)strlen(input);
    char *tape_init = malloc(input_len + 2);
    if (!tape_init)
        return NULL;
    tape_init[0] = '@';
    memcpy(tape_init + 1, input, input_len);
    tape_init[input_len + 1] = '\0';

    TuringMachine *tm = create_machine(tape_init);
    free(tape_init);
    if (!tm)
        return NULL;

    tm->head = 1;
    tm->current_state = '0';

    /* Состояние '0': дойти до конца и поставить '#' */
    add_transition(tm, '0', 'a', 'a', 'R', '0');
    add_transition(tm, '0', 'b', 'b', 'R', '0');
    add_transition(tm, '0', '\0', '#', 'L', '1');

    /* Состояние '1': вернуться влево к '@' */
    add_transition(tm, '1', 'a', 'a', 'L', '1');
    add_transition(tm, '1', 'b', 'b', 'L', '1');
    add_transition(tm, '1', '#', '#', 'L', '1');
    add_transition(tm, '1', 'X', 'X', 'L', '1');
    add_transition(tm, '1', 'Y', 'Y', 'L', '1');
    add_transition(tm, '1', '@', '@', 'R', '2');

    /* Состояние '2': найти непомеченный символ */
    add_transition(tm, '2', 'X', 'X', 'R', '2');
    add_transition(tm, '2', 'Y', 'Y', 'R', '2');
    add_transition(tm, '2', 'a', 'X', 'R', '3');
    add_transition(tm, '2', 'b', 'Y', 'R', '4');
    add_transition(tm, '2', '#', '#', 'L', '7');

    /* Состояние '3': несём 'a', идём к концу копии */
    add_transition(tm, '3', 'a', 'a', 'R', '3');
    add_transition(tm, '3', 'b', 'b', 'R', '3');
    add_transition(tm, '3', 'X', 'X', 'R', '3');
    add_transition(tm, '3', 'Y', 'Y', 'R', '3');
    add_transition(tm, '3', '#', '#', 'R', '3');
    add_transition(tm, '3', '\0', 'a', 'L', '5');

    /* Состояние '4': несём 'b', идём к концу копии */
    add_transition(tm, '4', 'a', 'a', 'R', '4');
    add_transition(tm, '4', 'b', 'b', 'R', '4');
    add_transition(tm, '4', 'X', 'X', 'R', '4');
    add_transition(tm, '4', 'Y', 'Y', 'R', '4');
    add_transition(tm, '4', '#', '#', 'R', '4');
    add_transition(tm, '4', '\0', 'b', 'L', '5');

    /* Состояние '5': вернуться влево к '@' */
    add_transition(tm, '5', 'a', 'a', 'L', '5');
    add_transition(tm, '5', 'b', 'b', 'L', '5');
    add_transition(tm, '5', 'X', 'X', 'L', '5');
    add_transition(tm, '5', 'Y', 'Y', 'L', '5');
    add_transition(tm, '5', '#', '#', 'L', '5');
    add_transition(tm, '5', '@', '@', 'R', '2');

    /* Состояние '7': идём влево к '@' для восстановления меток */
    add_transition(tm, '7', 'X', 'X', 'L', '7');
    add_transition(tm, '7', 'Y', 'Y', 'L', '7');
    add_transition(tm, '7', '@', '@', 'R', '8');

    /* Состояние '8': восстанавливаем X -> a, Y -> b */
    add_transition(tm, '8', 'X', 'a', 'R', '8');
    add_transition(tm, '8', 'Y', 'b', 'R', '8');
    add_transition(tm, '8', '#', '#', 'N', 'A');

    run_machine(tm);

    /* Результат - содержимое ленты от позиции 1 (после '@') до '\0' */
    int start = 1;
    int len = 0;
    while (start + len < tm->tape_size && tm->tape[start + len] != '\0')
        len++;

    char *result = malloc(len + 1);
    if (result)
    {
        memcpy(result, tm->tape + start, len);
        result[len] = '\0';
    }

    destroy_machine(tm);
    return result;
}
