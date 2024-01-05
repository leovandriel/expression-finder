//usr/bin/gcc "$0" -o bin/missing -Wall -Wextra -O3 -lm && ./bin/missing "$@"; exit $?
#include "iterator.h"
#include "table.h"
#include "util.h"

int main(int argc, char *argv[]) {
    int size = 7;
    ex_iterator stack[100];
    ht_table table;
    ht_init(&table, 1000000);
    char key[100];
    char expression[100];
    for (ex_init(stack, true); ex_next(stack) && stack->volume < size;) {
        ex_double_str(key, stack->value, 9);
        // ex_iterator_str(expression, stack);
        // if (!strcmp(expression, ""))
        //     printf("all: %s = %s\n", expression, key);
        if (!ht_get(&table, key) && fabs(stack->value) > 1e-6 && fabs(stack->value) < 1e6) {
            ex_iterator_str(expression, stack);
            ht_set(&table, key, expression);
        }
    }
    for (ex_init(stack, false); ex_next(stack) && stack->volume < size + 2;) {
        ex_double_str(key, stack->value, 9);
        // ex_iterator_str(expression, stack);
        // if (!strcmp(expression, ""))
        //     printf("opt: %s = %s\n", expression, key);
        if (ht_get(&table, key)) {
            ht_del(&table, key);
        }
    }
    ht_iterator iter = ht_iterate(&table);
    for (ht_entry *entry = ht_next(&iter); entry; entry = ht_next(&iter)) {
        printf("missing: %s = %s\n", entry->value, entry->key);
    }
    printf("%lu missing\n", table.count);
    ht_free(&table);
    return 0;
}
