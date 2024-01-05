//usr/bin/gcc "$0" -o bin/bench -Wall -Wextra -O3 -lm && ./bin/bench "$@"; exit $?
#include <time.h>

#include "iterator.h"

int main() {
    ex_iterator stack[100];
    ex_init(stack, false);
    int last = 0;
    clock_t start = clock(); // so inaccurate
    for (size_t count = 0; ex_next(stack); count++) {
        if (last != stack->volume) {
            double delta = (clock() - start) / (double)CLOCKS_PER_SEC;
            printf("size: %d  time: %.1fs count: %lu unit: %.1fns\n", last + 1, delta, count, 1e9 * delta / count);
            last = stack->volume;
            start = clock();
        }
    }
    return 0;
}
