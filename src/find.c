//usr/bin/gcc "$0" -o bin/find -Wall -Wextra -O3 -lm && ./bin/find "$@"; exit $?
#include "iterator.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

char *latex(ex_iterator *iter, char *buffer, int prec) {
    *(buffer++) = ' ';
    if (iter->volume == 0) {
        switch (iter->symbol) {
            case 'p': strncpy(buffer, "\\pi", 3); buffer += 3; break;
            default: *(buffer++) = iter->symbol;
        }
    } else if (iter->spread_index == 0) {
        switch(iter->symbol) {
            case 'l': strncpy(buffer, "\\ln(", 4); buffer += 4; break;
            case 'c': strncpy(buffer, "\\cos(", 5); buffer += 5; break;
            case '-': *(buffer++) = iter->symbol; break;
            default: *(buffer++) = '?';
        }
        buffer = latex(iter->child[0], buffer, iter->symbol == '-' ? 5 : 0);
        switch(iter->symbol) {
            case 'l': case 'c': *(buffer++) = ')'; break;
        }
    } else {
        switch(iter->symbol) {
            case '+':
                if (prec > 2) *(buffer++) = '(';
                buffer = latex(iter->child[0], buffer, 2);
                if (iter->child[1]->symbol != '-') {
                    *(buffer++) = iter->symbol;
                }
                buffer = latex(iter->child[1], buffer, 2);
                if (prec > 2) *(buffer++) = ')';
                break;
            case '*':
                if (prec > 3) *(buffer++) = '(';
                buffer = latex(iter->child[0], buffer, 3);
                strncpy(buffer, "\\cdot", 5); buffer += 5;
                buffer = latex(iter->child[1], buffer, 3);
                if (prec > 3) *(buffer++) = ')';
                break;
            case '/':
                strncpy(buffer, "\\frac{", 6); buffer += 6;
                buffer = latex(iter->child[0], buffer, 0);
                strncpy(buffer, "}{", 2); buffer += 2;
                buffer = latex(iter->child[1], buffer, 0);
                *(buffer++) = '}';
                break;
            case 'r':
                strncpy(buffer, "\\sqrt", 5); buffer += 5;
                if (iter->child[1]->value != 2) {
                    *(buffer++) = '[';
                    buffer = latex(iter->child[1], buffer, 0);
                    *(buffer++) = ']';
                }
                *(buffer++) = '{';
                buffer = latex(iter->child[0], buffer, 0);
                *(buffer++) = '}';
                break;
            case '^':
                if (prec > 4) *(buffer++) = '(';
                buffer = latex(iter->child[0], buffer, 4);
                if (prec > 4) *(buffer++) = ')';
                strncpy(buffer, "^{", 2); buffer += 2;
                buffer = latex(iter->child[1], buffer, 4);
                *(buffer++) = '}';
                break;
            default:
                *(buffer++) = '?';
        }
    }
    *(buffer++) = ' ';
    return buffer;
}

void report(ex_iterator *iter, double value, int size) {
    char latex_buffer[1000];
#ifdef __EMSCRIPTEN__
    *(latex(iter, latex_buffer, 0)) = '\0';
    EM_ASM({ onResult(UTF8ToString($0, $1), $2, $3); }, latex_buffer, strlen(latex_buffer), value, size);
#else
    char expression[100];
    ex_iterator_str(expression, iter);
    printf("result: %s = %.20f (%i)\n", expression, value, size);
#endif
}

int next(char *target_string, ex_iterator *stack, size_t max) {
    char *dotat = strchr(target_string, '.');
    int decimals = dotat ? strlen(target_string) - (dotat - target_string) - 1 : 1;
    if (decimals > 15) {
        // 64-bit double doesn't get more than 15 digits accurately
        decimals = 15;
    }
    double max_diff = pow(.1, decimals);
    double round_factor = pow(10, decimals);
    double target_value = strtod(target_string, NULL);
    long long int round_target = llround(target_value * round_factor);
    if (!stack->root) {
        ex_init(stack, false);
    }
    for (size_t count = 0; ex_next(stack) && count < max; count++) {
        double diff = fabs(target_value - stack->value);
        if (diff < max_diff) {
            double scaled = stack->value * round_factor;
            if (round_target == (long long int)round(scaled) || round_target == (long long int)trunc(scaled)) {
                report(stack->child[0], stack->value, stack->volume + 1);
                return 1;
            }
        }
    }
    return 0;
}

#ifndef __EMSCRIPTEN__
int main(int argc, char *argv[]) {
    char *target = argc > 1 ? argv[1] : "1.6180339";
    printf("target: %s\n", target);
    ex_iterator stack[100] = {};
    while (true) {
        next(target, stack, 1000000);
    }
    return 0;
}
#endif
