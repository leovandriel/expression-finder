#ifndef EX_UTIL_H
#define EX_UTIL_H

#include <stdio.h>

void ex_double_str(char *buffer, double d, int precision) {
    sprintf(buffer, "%.*e", precision, d);
}

#endif