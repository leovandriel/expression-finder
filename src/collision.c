#include "iterator.h"
#include "table.h"
#include "util.h"

int main(int argc, char *argv[]) {
    int size = 6;
    ex_iterator stack[100];
    ht_table table;
    ht_init(&table, 10000);
    size_t count = 0;
    char buffer[100];
    for (ex_init(stack, false); ex_next(stack) && stack->volume < size;) {
        char *key = ex_double_str(stack->value, buffer);
        char *value = ht_get(&table, key);
        char *expression = ex_iterator_str(stack);
        if (value && strcmp(key, "inf")) {
            printf("collision: %s = %s = %s\n", expression, value, key);
            count++;
        } else {
            ht_set(&table, key, expression);
        }
    }
    printf("%lu collisions\n", count);
    ht_free(&table);
    return 0;
}
