//usr/bin/gcc "$0" -o bin/list -Wall -Wextra -O3 -lm && ./bin/list "$@"; exit $?
#include "iterator.h"

int main()
{
    ex_iterator stack[100];
    char expression[100];
    for (int i = 0; i < 3; i++)
    {
        printf("size %i\n", i + 1);
        for (ex_init_size(i, stack, false); ex_next(stack);)
        {
            ex_iterator_str(expression, stack);
            printf("%s = %.20f\n", expression, stack->value);
        }
    }
    return 0;
}
