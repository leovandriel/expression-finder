#include "iterator.h"

int main(int argc, char *argv[]) {
    ex_iterator stack[100];
    for (int i = 0; i < 3; i++) {
        printf("size %i\n", i + 1);
        for (ex_init_size(i, stack, false); ex_next(stack);) {
            char *expression = ex_iterator_str(stack);
            if (!stack->value) {
                printf("error: %s is ZERO\n", expression);
                return -1;
            }
            if (isnan(stack->value)) {
                printf("error: %s is NAN\n", expression);
                return -1;
            }
            printf("%s = %.20f\n", expression, stack->value);
        }
    }
    return 0;
}
