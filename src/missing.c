#include "expression.h"
#include "table.h"

int main(int argc, char *argv[]) {
    int size = 7;
    ex_iterator stack[100];
    ht_table table;
    ht_init(&table, 1000000);
    for (ex_init(stack, true); ex_next(stack) && stack->volume < size;) {
        char *key = ex_double_str(stack->value);
        // char *expression = ex_iterator_str(stack);
        // if (!strcmp(expression, ""))
        //     printf("all: %s = %s\n", expression, key);
        if (!ht_get(&table, key) && fabs(stack->value) > 1e-6 && fabs(stack->value) < 1e6) {
            char *expression = ex_iterator_str(stack);
            ht_set(&table, key, expression);
        }
    }
    for (ex_init(stack, false); ex_next(stack) && stack->volume < size + 2;) {
        char *key = ex_double_str(stack->value);
        // char *expression = ex_iterator_str(stack);
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
