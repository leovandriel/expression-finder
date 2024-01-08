#ifndef EX_UTIL_H
#define EX_UTIL_H

#include <stdio.h>

#include "iterator.h"

void ex_double_str(char *buffer, double d, int precision)
{
    sprintf(buffer, "%.*e", precision, d);
}

char *ex_iterator_str_in(char *buffer, ex_iterator *iter)
{
    if (iter->arity == 0)
    {
        switch (iter->symbol)
        {
        case 'p':
            strncpy(buffer, "pi", 2);
            buffer += 2;
            break;
        default:
            *(buffer++) = iter->symbol;
        }
        return buffer;
    }
    else if (iter->arity == 1)
    {
        switch (iter->symbol)
        {
        case 'l':
            strncpy(buffer, "ln(", 3);
            buffer += 3;
            break;
        case 'c':
            strncpy(buffer, "cos(", 4);
            buffer += 4;
            break;
        default:
            *(buffer++) = iter->symbol;
        }
        buffer = ex_iterator_str_in(buffer, iter->child[0]);
        switch (iter->symbol)
        {
        case 'l':
        case 'c':
            *(buffer++) = ')';
            break;
        }
        return buffer;
    }
    else
    {
        *(buffer++) = '(';
        buffer = ex_iterator_str_in(buffer, iter->child[0]);
        switch (iter->symbol)
        {
        case 'r':
            strncpy(buffer, "^/", 2);
            buffer += 2;
            break;
        default:
            *(buffer++) = iter->symbol;
        }
        buffer = ex_iterator_str_in(buffer, iter->child[1]);
        *(buffer++) = ')';
        return buffer;
    }
}

void ex_iterator_str(char *buffer, ex_iterator *iter)
{
    if (iter->root)
    {
        iter = iter->child[0];
    }
    *(ex_iterator_str_in(buffer, iter)) = '\0';
}

#endif // EX_UTIL_H
