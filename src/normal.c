//usr/bin/gcc "$0" -o bin/normal -Wall -Wextra -O3 -lm && ./bin/normal "$@"; exit $?
#include "iterator.h"
#include "table.h"
#include "util.h"

// Test for non-normal expression values - Loops over the iterator and logs
// occurrences of expressions with non-normal values, like 0, inf, and nan. This
// can be used to find inefficiencies in the iterator.
int main()
{
    int size = 12;
    ex_iterator stack[100];
    char key[100];
    char expression[100];
    for (ex_init(stack, false); ex_next(stack) && stack->volume < size;)
    {
        if (!isnormal(stack->value))
        {
            ex_iterator_str(expression, stack);
            ex_double_str(key, stack->value, 9);
            printf("collision: %s is not normal (%s)\n", expression, key);
        }
    }
    return 0;
}
