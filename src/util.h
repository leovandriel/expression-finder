#ifndef EX_UTIL_H
#define EX_UTIL_H

#include <stdio.h>

#include "iterator.h"

// Generates canonical string representation of a double value for use in hash
// table keys.
void ex_double_str(char *buffer, double d, int precision)
{
    sprintf(buffer, "%.*e", precision, d);
}

// Recursive sub-function of ex_iterator_str
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

// Writes string representation of expression.
void ex_iterator_str(char *buffer, ex_iterator *iter)
{
    *(ex_iterator_str_in(buffer, iter)) = '\0';
}

// Writes LaTeX string representation of expression.
char *latex(ex_iterator *iter, char *buffer, int prec)
{
    *(buffer++) = ' ';
    if (iter->arity == 0)
    {
        switch (iter->symbol)
        {
        case 'p':
            strncpy(buffer, "\\pi", 3);
            buffer += 3;
            break;
        default:
            *(buffer++) = iter->symbol;
        }
    }
    else if (iter->arity == 1)
    {
        switch (iter->symbol)
        {
        case 'l':
            strncpy(buffer, "\\ln(", 4);
            buffer += 4;
            break;
        case 'c':
            strncpy(buffer, "\\cos(", 5);
            buffer += 5;
            break;
        case '-':
            *(buffer++) = iter->symbol;
            break;
        default:
            *(buffer++) = '?';
        }
        buffer = latex(iter->child[0], buffer, iter->symbol == '-' ? 5 : 0);
        switch (iter->symbol)
        {
        case 'l':
        case 'c':
            *(buffer++) = ')';
            break;
        }
    }
    else
    {
        switch (iter->symbol)
        {
        case '+':
            if (prec > 2)
                *(buffer++) = '(';
            buffer = latex(iter->child[0], buffer, 2);
            if (iter->child[1]->symbol != '-')
            {
                *(buffer++) = iter->symbol;
            }
            buffer = latex(iter->child[1], buffer, 2);
            if (prec > 2)
                *(buffer++) = ')';
            break;
        case '*':
            if (prec > 3)
                *(buffer++) = '(';
            buffer = latex(iter->child[0], buffer, 3);
            strncpy(buffer, "\\cdot", 5);
            buffer += 5;
            buffer = latex(iter->child[1], buffer, 3);
            if (prec > 3)
                *(buffer++) = ')';
            break;
        case '/':
            strncpy(buffer, "\\frac{", 6);
            buffer += 6;
            buffer = latex(iter->child[0], buffer, 0);
            strncpy(buffer, "}{", 2);
            buffer += 2;
            buffer = latex(iter->child[1], buffer, 0);
            *(buffer++) = '}';
            break;
        case 'r':
            strncpy(buffer, "\\sqrt", 5);
            buffer += 5;
            if (iter->child[1]->value != 2)
            {
                *(buffer++) = '[';
                buffer = latex(iter->child[1], buffer, 0);
                *(buffer++) = ']';
            }
            *(buffer++) = '{';
            buffer = latex(iter->child[0], buffer, 0);
            *(buffer++) = '}';
            break;
        case '^':
            if (prec > 4)
                *(buffer++) = '(';
            buffer = latex(iter->child[0], buffer, 4);
            if (prec > 4)
                *(buffer++) = ')';
            strncpy(buffer, "^{", 2);
            buffer += 2;
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

// Parse operator string representation to char code.
char ex_parse_symbol(char *string, int *length)
{
    switch (*string)
    {
    case '1':
    case '2':
    case '3':
    case '5':
    case 'e':
    case '+':
    case '-':
    case '*':
    case '/':
        *length = 1;
        return *string;
    case 'p':
        if (strncmp(string, "pi", 2) == 0)
        {
            *length = 2;
            return 'p';
        }
        break;
    case 'l':
        if (strncmp(string, "ln", 2) == 0)
        {
            *length = 2;
            return 'l';
        }
        break;
    case '^':
        if (strncmp(string, "^/", 2) == 0)
        {
            *length = 2;
            return 'r';
        }
        *length = 1;
        return '^';
    case 'c':
        if (strncmp(string, "cos", 3) == 0)
        {
            *length = 3;
            return 'c';
        }
        break;
    }
    *length = 0;
    return '?';
}

void ex_iterator_parse_memmove(ex_iterator *iter, int offset)
{
    if (iter->child[0] != NULL)
    {
        ex_iterator_parse_memmove(iter->child[0], offset);
        iter->child[0] += offset;
    }
    if (iter->child[1] != NULL)
    {
        ex_iterator_parse_memmove(iter->child[1], offset);
        iter->child[1] += offset;
    }
}

// Recursive sub-function of ex_iterator_parse
int ex_iterator_parse_in(char *string, int *index, ex_iterator *iter)
{
    int size = 0;
    if (string[*index] == '(')
    {
        (*index)++;
        int s = ex_iterator_parse_in(string, index, iter);
        if (s < 0)
        {
            return -1;
        }
        if (string[*index] != ')')
        {
            printf("ERROR: expected ) at %i in %s\n", *index, string);
        }
        (*index)++;
        size = s;
    }
    else
    {
        int symbol_length = 0;
        char symbol = ex_parse_symbol(string + *index, &symbol_length);
        ex_iterator *child = NULL;
        if (strchr("1235ep", symbol) != NULL)
        {
            *index += symbol_length;
            switch (symbol)
            {
            case '1':
                iter->value = 1;
                break;
            case '2':
                iter->value = 2;
                break;
            case '3':
                iter->value = 3;
                break;
            case '5':
                iter->value = 5;
                break;
            case 'e':
                iter->value = M_E;
                break;
            case 'p':
                iter->value = M_PI;
                break;
            }
            iter->size = 1;
            iter->arity = 0;
            size = 1;
        }
        else if (strchr("-lc", symbol) != NULL)
        {
            *index += symbol_length;
            int s = ex_iterator_parse_in(string, index, iter + 1);
            if (s < 0)
            {
                return -1;
            }
            switch (symbol)
            {
            case '-':
                iter->value = -child->value;
                break;
            case 'l':
                iter->value = log(child->value);
                break;
            case 'c':
                iter->value = cos(child->value);
                break;
            }
            iter->size = child->size + 1;
            iter->arity = 1;
            size = s + 1;
        }
        else
        {
            printf("ERROR: unexpected symbol at %i in %s\n", *index, string);
            return -1;
        }
        iter->symbol = symbol;
        iter->child[0] = child;
        iter->child[1] = NULL;
        iter->symbol_index = -1;
        iter->spread_index = -1;
    }
    if (string[*index] != ')' && string[*index] != '\0')
    {
        int symbol_length = 0;
        char symbol = ex_parse_symbol(string + *index, &symbol_length);
        if (strchr("+*/^r", symbol) == NULL)
        {
            printf("ERROR: unexpected symbol at %i in %s\n", *index, string);
            return -1;
        }
        *index += symbol_length;
        ex_iterator_parse_memmove(iter, 1);
        memmove(iter + 1, iter, sizeof(ex_iterator) * size);
        ex_iterator *child0 = iter + 1;
        ex_iterator *child1 = iter + size + 1;
        int s = ex_iterator_parse_in(string, index, child1);
        if (s < 0)
        {
            return -1;
        }
        switch (symbol)
        {
        case '+':
            iter->value = child0->value + child1->value;
            break;
        case '*':
            iter->value = child0->value * child1->value;
            break;
        case '/':
            iter->value = child0->value / child1->value;
            break;
        case '^':
            iter->value = pow(child0->value, child1->value);
            break;
        case 'r':
            iter->value = pow(child0->value, 1. / child1->value);
            break;
        }
        iter->size = child0->size + child1->size + 1;
        iter->arity = 2;
        iter->symbol = symbol;
        iter->child[0] = child0;
        iter->child[1] = child1;
        iter->symbol_index = -1;
        iter->spread_index = -1;
        size += s + 1;
    }
    return size;
}

// Parse an expression string into an expression tree.
int ex_iterator_parse(char *string, ex_iterator *iter)
{
    int index = 0;
    return ex_iterator_parse_in(string, &index, iter);
}

bool ex_equal_symbol(ex_iterator *a, ex_iterator *b)
{
    if (a == NULL || b == NULL)
    {
        return false;
    }
    if (a->symbol != b->symbol)
    {
        return false;
    }
    if (a->child[0] != NULL && b->child[0] != NULL)
    {
        if (!ex_equal_symbol(a->child[0], b->child[0]))
        {
            return false;
        }
    }
    else if (a->child[0] != NULL || b->child[0] != NULL)
    {
        return false;
    }
    if (a->child[1] != NULL && b->child[1] != NULL)
    {
        if (!ex_equal_symbol(a->child[1], b->child[1]))
        {
            return false;
        }
    }
    else if (a->child[1] != NULL || b->child[1] != NULL)
    {
        return false;
    }
    return true;
}

#endif // EX_UTIL_H
