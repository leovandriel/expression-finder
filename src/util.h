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
    if (iter->root)
    {
        iter = iter->child[0];
    }
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

// Recursive sub-function of ex_iterator_parse
ex_iterator *ex_iterator_parse_in(char *string, int *index, ex_iterator *iter)
{
    ex_iterator *parent = iter;
    if (string[*index] == '(')
    {
        (*index)++;
        parent = ex_iterator_parse_in(string, index, iter);
        if (parent == NULL)
        {
            return NULL;
        }
        if (string[*index] != ')')
        {
            printf("ERROR: expected ) at %i in %s\n", *index, string);
        }
        (*index)++;
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
                parent->value = 1;
                break;
            case '2':
                parent->value = 2;
                break;
            case '3':
                parent->value = 3;
                break;
            case '5':
                parent->value = 5;
                break;
            case 'e':
                parent->value = M_E;
                break;
            case 'p':
                parent->value = M_PI;
                break;
            }
            parent->volume = 1;
            parent->arity = 0;
        }
        else if (strchr("-lc", symbol) != NULL)
        {
            *index += symbol_length;
            child = ex_iterator_parse_in(string, index, iter + 1);
            if (child == NULL)
            {
                return NULL;
            }
            switch (symbol)
            {
            case '-':
                parent->value = -child->value;
                break;
            case 'l':
                parent->value = log(child->value);
                break;
            case 'c':
                parent->value = cos(child->value);
                break;
            }
            parent->volume = child->volume + 1;
            parent->arity = 1;
        }
        else
        {
            printf("ERROR: unexpected symbol at %i in %s\n", *index, string);
            return NULL;
        }
        parent->symbol = symbol;
        parent->child[0] = child;
        parent->child[1] = NULL;
        parent->symbol_index = -1;
        parent->spread_index = -1;
        parent->all = true;
        parent->root = false;
    }
    if (string[*index] != ')' && string[*index] != '\0')
    {
        int symbol_length = 0;
        char symbol = ex_parse_symbol(string + *index, &symbol_length);
        if (strchr("+*/^r", symbol) == NULL)
        {
            printf("ERROR: unexpected symbol at %i in %s\n", *index, string);
            return NULL;
        }
        *index += symbol_length;
        ex_iterator *child0 = parent;
        parent = iter + child0->volume;
        ex_iterator *child1 = ex_iterator_parse_in(string, index, parent + 1);
        if (child1 == NULL)
        {
            return NULL;
        }
        switch (symbol)
        {
        case '+':
            parent->value = child0->value + child1->value;
            break;
        case '*':
            parent->value = child0->value * child1->value;
            break;
        case '/':
            parent->value = child0->value / child1->value;
            break;
        case '^':
            parent->value = pow(child0->value, child1->value);
            break;
        case 'r':
            parent->value = pow(child0->value, 1. / child1->value);
            break;
        }
        parent->volume = child0->volume + child1->volume + 1;
        parent->arity = 2;
        parent->symbol = symbol;
        parent->child[0] = child0;
        parent->child[1] = child1;
        parent->symbol_index = -1;
        parent->spread_index = -1;
        parent->all = true;
        parent->root = false;
    }
    return parent;
}

// Parse an expression string into an expression tree.
ex_iterator *ex_iterator_parse(char *string, ex_iterator *iter)
{
    int index = 0;
    ex_iterator *child = ex_iterator_parse_in(string, &index, iter + 1);
    if (child == NULL)
    {
        return NULL;
    }
    iter->root = true;
    iter->child[0] = child;
    iter->value = iter->child[0]->value;
    return iter;
}

bool ex_equal_symbol(ex_iterator *a, ex_iterator *b)
{
    if (a == NULL || b == NULL)
    {
        return false;
    }
    if (a->root)
    {
        a = a->child[0];
    }
    if (b->root)
    {
        b = b->child[0];
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
