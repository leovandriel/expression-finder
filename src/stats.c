#include "iterator.h"
#include "table.h"
#include "util.h"

int main(int argc, char *argv[]) {
    size_t size = 9;
    ex_iterator stack[100];
    ht_table table;
    bool all;
    char buffer[100];
    for (int i = 0; i < 2; i++) {
        all = !i;
        const char *type = all ? "all" : "opt";
        ht_init(&table, 1000000);
        size_t last_table_count = 0;
        size_t last_iterations = 1;
        size_t last_unique = 1;
        size_t total = 0;
        for (int i = 0; i < size; i++) {
            size_t count = 0;
            for (ex_init_size(i, stack, all); ex_next(stack);) {
                char *key = ex_double_str(stack->value, buffer);
                if (!ht_get(&table, key)) {
                    ht_set(&table, key, "");
                }
                count++;
            }
            printf("%s, size: %i iterations: %lu (x%.1f)\n", type, i + 1, count, 1. * count / last_iterations);
            printf("%s, size: %i unique: %lu (%lu%%) (x%.1f)\n", type, i + 1, table.count - last_table_count, 100 * (table.count - last_table_count) / count, 1. * (table.count - last_table_count) / last_unique);
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
