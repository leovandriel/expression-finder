#include <time.h>

#include "expression.h"

int main(int argc, char *argv[]) {
    ex_iterator stack[100];
    ex_init(stack, false);
    size_t last = 0;
    clock_t start = clock(); // so inaccurate
    for (size_t count = 0; ex_next(stack); count++) {
        if (last != stack->volume) {
            double delta = (clock() - start) / (double)CLOCKS_PER_SEC;
            printf("size: %lu  time: %.1fs count: %lu unit: %.1fns\n", last + 1, delta, count, 1e9 * delta / count);
            last = stack->volume;
            start = clock();
        }
    }
    return 0;
}
