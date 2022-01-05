#ifndef EX_UTIL_H
#define EX_UTIL_H

#include <stdio.h>

char *ex_double_str(double d, char *buffer) {
    sprintf(buffer, "%.9e", d);
    return buffer;
}

#endif