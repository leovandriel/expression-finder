//usr/bin/gcc "$0" -o bin/list -Wall -Wextra -O3 -lm && ./bin/list "$@"; exit $?
#include "iterator.h"
#include "util.h"

// Simply list all expressions, grouped by size.
int main()
{
    ex_iterator stack[100];
    char expression[100];
    for (int size = 1; size <= 3; size++)
    {
        printf("size %i\n", size);
        for (ex_init(stack); ex_next_size(stack, size);)
        {
            ex_iterator_str(expression, stack);
            printf("%s = %.20f (%i)\n", expression, stack->value, stack->size);
        }
    }
    return 0;
}
