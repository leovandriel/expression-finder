//usr/bin/gcc "$0" -o bin/find -Wall -Wextra -O3 -lm && ./bin/find "$@"; exit $?
#include "iterator.h"
#include "util.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Display a matching expression.
void report(ex_iterator *iter, double value, int size)
{
#ifdef __EMSCRIPTEN__
    char latex_buffer[1000];
    *(latex(iter, latex_buffer, 0)) = '\0';
    EM_ASM({ onResult(UTF8ToString($0, $1), $2, $3); }, latex_buffer, strlen(latex_buffer), value, size);
#else
    char expression[100];
    ex_iterator_str(expression, iter);
    printf("result: %s = %.20f (%i)\n", expression, value, size);
#endif
}

// Iterate over the next block of expressions to find a match.
int next(char *target_string, ex_iterator *stack, size_t max)
{
    char *dotat = strchr(target_string, '.');
    int decimals = dotat ? strlen(target_string) - (dotat - target_string) - 1 : 1;
    if (decimals > 15)
    {
        // 64-bit double doesn't get more than 15 digits accurately
        decimals = 15;
    }
    double max_diff = pow(.1, decimals);
    double round_factor = pow(10, decimals);
    double target_value = strtod(target_string, NULL);
    long long int round_target = llround(target_value * round_factor);
    if (!stack->root)
    {
        ex_init(stack, false);
    }
    for (size_t count = 0; ex_next(stack) && count < max; count++)
    {
        double diff = fabs(target_value - stack->value);
        if (diff < max_diff)
        {
            double scaled = stack->value * round_factor;
            if (round_target == (long long int)round(scaled) || round_target == (long long int)trunc(scaled))
            {
                report(stack->child[0], stack->value, stack->volume + 1);
                return 1;
            }
        }
    }
    return 0;
}

// Takes input decimal value and iterates over all expressions to find potential
// matches.
#ifndef __EMSCRIPTEN__
int main(int argc, char *argv[])
{
    char *target = argc > 1 ? argv[1] : "1.6180339";
    printf("target: %s\n", target);
    ex_iterator stack[100] = {};
    for (size_t count = 0;; count++)
    {
        next(target, stack, 1000000);
        printf(" %luM (%d)\r", count, stack->volume);
        fflush(stdout);
    }
    return 0;
}
#endif
