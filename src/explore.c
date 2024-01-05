#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "util.h"

#define DEPTH 20

static int precision = 10;
ht_table tables[DEPTH];
static char buffer[100];

void add(int size, double key)
{
    ex_double_str(buffer, key, precision);
    for (int i = 1; i <= size; i++)
    {
        if (ht_get(&tables[i], buffer) != NULL)
        {
            return;
        }
    }
    ht_set(&tables[size], buffer, "");
}

void list(int size)
{
    ht_iterator iter = ht_iterate(&tables[size]);
    for (ht_entry *entry = ht_next(&iter); entry; entry = ht_next(&iter))
    {
        printf("%s\n", entry->key);
    }
}

int main()
{
    int size = 0;
    ht_init(&tables[1], 7);
    add(1, 1);
    add(1, 2);
    add(1, 3);
    add(1, 5);
    add(1, M_E);
    add(1, M_PI);
    printf("size: %d, count: %lu\n", 1, tables[1].count);
    // list(1);
    for (size = 2; size < DEPTH; size++)
    {
        ht_init(&tables[size], 1 + tables[size - 1].count * 4);
        ht_iterator iter0 = ht_iterate(&tables[size - 1]);
        for (ht_entry *entry0 = ht_next(&iter0); entry0; entry0 = ht_next(&iter0))
        {
            double key0 = atof(entry0->key);
            add(size, -(key0));
            add(size, log(key0));
            add(size, cos(key0));
        }
        for (int sub1 = 1; sub1 < size - 1; sub1++)
        {
            int sub2 = size - sub1 - 1;
            ht_iterator iter1 = ht_iterate(&tables[sub1]);
            for (ht_entry *entry1 = ht_next(&iter1); entry1; entry1 = ht_next(&iter1))
            {
                double key1 = atof(entry1->key);
                ht_iterator iter2 = ht_iterate(&tables[sub2]);
                for (ht_entry *entry2 = ht_next(&iter2); entry2; entry2 = ht_next(&iter2))
                {
                    double key2 = atof(entry2->key);
                    add(size, key1 + key2);
                    // add(size, key1 - key2);
                    add(size, key1 * key2);
                    add(size, key1 / key2);
                    add(size, pow(key1, key2));
                    add(size, pow(key1, 1 / key2));
                }
            }
        }
        printf("size: %d, count: %lu (%.2fx)\n", size, tables[size].count, (double)tables[size].count / tables[size - 1].count);
        // list(size);
    }
    for (int i = 1; i < DEPTH; i++)
    {
        ht_free(&tables[i]);
    }
    return 0;
}
