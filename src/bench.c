//usr/bin/gcc "$0" -o bin/bench -Wall -Wextra -O3 -lm && ./bin/bench "$@"; exit $?
#include <time.h>

#include "iterator.h"

// Performance benchmark for iterator - Loops over the iterator and logs time
// (clock) elapsed, grouped by expression size. It includes average
// iteration time, which is a good indicator for the computation efficiency of
// the iterator.
int main()
{
    ex_iterator stack[100];
    ex_init(stack);
    int last = 1;
    clock_t start = clock(); // so inaccurate
    for (size_t count = 0; ex_next(stack); count++)
    {
        if (last != stack->size)
        {
            double delta = (clock() - start) / (double)CLOCKS_PER_SEC;
            printf("size: %d  time: %.1fs count: %lu unit: %.1fns\n", last + 1, delta, count, 1e9 * delta / count);
            last = stack->size;
            start = clock();
        }
    }
    return 0;
}
