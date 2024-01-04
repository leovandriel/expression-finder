#include "iterator.h"
#include "table.h"
#include "util.h"

int main(int argc, char *argv[]) {
    int size = 12;
    ex_iterator stack[100];
    char key[100];
    char expression[100];
    for (ex_init(stack, false); ex_next(stack) && stack->volume < size;) {
        if (!isnormal(stack->value)) {
            ex_iterator_str(expression, stack);
            ex_double_str(key, stack->value, 9);
            printf("collision: %s is not normal (%s)\n", expression, key);
        }
    }
    return 0;
}
