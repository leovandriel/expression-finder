//usr/bin/gcc "$0" -o bin/eval -Wall -Wextra -O3 -lm -lflint && ./bin/eval "$@"; exit $?
#include "iterator.h"
#include "util.h"
#include "flint.h"

int main(int argc, char *argv[])
{
    char *input = argc > 1 ? argv[1] : "(1+5^/2)/2";
    int decimals = argc > 2 ? strtoul(argv[2], NULL, 10) : 100;
    int prec = round(decimals * log(10) / log(2));
    ex_iterator stack[100];
    if (ex_iterator_parse(input, stack) == NULL)
    {
        return -1;
    }
    char expression[100];
    ex_iterator_str(expression, stack);
    arb_t result;
    arb_init(result);
    ex_arb_eval(result, stack, prec);
    printf("%s = %s", expression, arb_get_str(result, prec, 0));
    return 0;
}
