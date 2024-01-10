//usr/bin/gcc "$0" -o bin/needle -Wall -Wextra -O3 -lm -lflint && ./bin/needle "$@"; exit $?
#include "iterator.h"
#include "util.h"
#include "flint.h"

// Find if iterator is missing a expression.
int main(int argc, char *argv[])
{
    char *input = argc > 1 ? argv[1] : "(1+5^/2)/2";
    int all = argc > 2 ? strcmp(argv[2], "all") == 0 : false;
    ex_iterator stack_a[100];
    if (ex_iterator_parse(input, stack_a) == NULL)
    {
        return -1;
    }
    char key[100];
    char expression[100];
    ex_iterator_str(expression, stack_a);
    ex_double_str(key, stack_a->value, 9);
    printf("searching for %s = %s\n", expression, key);
    int size = 12;
    ex_iterator stack_b[100];
    for (ex_init(stack_b, false); stack_b->volume < size && ex_next(stack_b);)
    {
        if (ex_equal_symbol(stack_a, stack_b) 
        || ex_equal_symbol(stack_a->child[0], stack_b) 
        || ex_equal_symbol(stack_a->child[1], stack_b)
        || (!all && (fabs(stack_a->value - stack_b->value) < 1e-12))
        )
        {
            ex_iterator_str(expression, stack_b);
            ex_double_str(key, stack_b->value, 9);
            printf("found in %s: %s = %s\n", all ? "all" : "opt", expression, key);
        }
    }
    return 0;
}
