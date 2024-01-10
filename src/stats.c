//usr/bin/gcc "$0" -o bin/stats -Wall -Wextra -O3 -lm && ./bin/stats "$@"; exit $?
#include "iterator.h"
#include "table.h"
#include "util.h"

// Counting benchmark for iterator - Loops over the iterator and logs number of
// expressions, grouped by expression size. It makes distinction
// between iterating over all possible expressions (all), and without duplicates
// (opt) expressions.
//
// NOTE: opt unique grows around 7^size/9, with around 10M/second
int main()
{
    int max = 8;
    ex_iterator stack[100];
    ht_table table;
    char key[100];
    for (int all = 0; all < 2; all++)
    {
        const char *type = all ? "all" : "opt";
        ht_init(&table, 1000000);
        size_t last_table_count = 0;
        size_t last_iterations = 1;
        size_t last_unique = 1;
        size_t total = 0;
        for (int size = 1; size <= max; size++)
        {
            size_t count = 0;
            for (ex_init(stack); ex_next_size_all(stack, size, all);)
            {
                ex_double_str(key, stack->value, 9);
                if (!ht_get(&table, key))
                {
                    ht_set(&table, key, "");
                }
                count++;
            }
            printf("%s, size: %i iterations: %lu (x%.1f)\n", type, size, count, 1. * count / last_iterations);
            printf("%s, size: %i unique: %lu (%lu%%) (x%.1f)\n", type, size, table.count - last_table_count, 100 * (table.count - last_table_count) / count, 1. * (table.count - last_table_count) / last_unique);
            last_iterations = count;
            last_unique = table.count - last_table_count;
            last_table_count = table.count;
            total += count;
        }
        printf("%s, total, iterations: %lu\n", type, total);
        printf("%s, total, unique: %lu (%lu%%)\n", type, table.count, 100 * table.count / total);
        ht_free(&table);
    }
    return 0;
}
