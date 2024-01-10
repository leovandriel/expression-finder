//usr/bin/gcc "$0" -o bin/collision -Wall -Wextra -O3 -lm && ./bin/collision "$@"; exit $?
#include "iterator.h"
#include "table.h"
#include "util.h"

// Search occurrences of expressions that have the same value. This can be used
// to find new iterator conditions and reduce the number of expressions without
// loss of expressiveness.
int main()
{
    int size = 6;
    ex_iterator stack[100];
    ht_table table;
    ht_init(&table, 10000);
    size_t count = 0;
    char key[100];
    char expression[100];
    for (ex_init(stack); stack->volume < size && ex_next(stack);)
    {
        ex_double_str(key, stack->value, 9);
        char *value = ht_get(&table, key);
        ex_iterator_str(expression, stack);
        if (value)
        {
            printf("collision: %s = %s = %s\n", expression, value, key);
            count++;
        }
        else
        {
            ht_set(&table, key, expression);
        }
    }
    printf("%lu collisions\n", count);
    ht_free(&table);
    return 0;
}
