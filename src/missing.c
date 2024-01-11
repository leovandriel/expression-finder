//usr/bin/gcc "$0" -o bin/missing -Wall -Wextra -O3 -lm && ./bin/missing "$@"; exit $?
#include "iterator.h"
#include "table.h"
#include "util.h"

const char *exceptions[] = {
    "",
};

const int exceptions_size = sizeof(exceptions) / sizeof(exceptions[0]);

bool is_exception(const char *expression)
{
    for (int i = 0; i < exceptions_size; i++)
    {
        if (strcmp(expression, exceptions[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

bool in_range(double value)
{
    return fabs(value) > 1e-12 && fabs(value) < 1e12;
}

// Search occurrences of expressions that are returned by the un-optimized
// iterator (all=true) and are missing from the deduplicated one (all=false).
// This can be used to find overly restrictive conditions in the iterator, i.e.
// loss of expressiveness.
int main()
{
    int max = 8;
    ex_iterator stack[100];
    ht_table table;
    ht_init(&table, 2000000);
    char key[100];
    char expression[100];
    char combined[200];
    for (int size = 1; size <= max; size++)
    {
        printf("size: %d\n", size);
        for (ex_init(stack); ex_next_all(stack, true) && stack->size <= size;)
        {
            if (in_range(stack->value))
            {
                ex_double_str(key, stack->value, 9);
                char *existing = ht_get(&table, key);
                if (!existing)
                {
                    ex_iterator_str(expression, stack);
                    ht_set(&table, key, expression);
                }
                else
                {
                    int len = strlen(existing);
                    if (len < 100)
                    {
                        ex_iterator_str(expression, stack);
                        sprintf(combined, "%s, %s", existing, expression);
                        ht_set(&table, key, combined);
                    }
                    else if (existing[len - 1] != '.')
                    {
                        sprintf(combined, "%s...", existing);
                        ht_set(&table, key, combined);
                    }
                }
            }
        }
        for (ex_init(stack); ex_next_all(stack, false) && stack->size <= size;)
        {
            if (in_range(stack->value))
            {
                ex_double_str(key, stack->value, 9);
                if (ht_get(&table, key))
                {
                    ht_del(&table, key);
                }
            }
        }
        ht_iterator iter = ht_iterate(&table);
        for (ht_entry *entry = ht_next(&iter); entry; entry = ht_next(&iter))
        {
            if (!is_exception(entry->value))
            {
                printf("missing: %s = %s\n", entry->value, entry->key);
            }
        }
    }
    ht_free(&table);
    printf("%lu missing\n", table.count);
    return 0;
}
