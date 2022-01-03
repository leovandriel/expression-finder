#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const double primitives[6] = { 1., 2., 3., 5., M_PI, M_E };
const double primitive_symbols[6] = { '1', '2', '3', '5', 'p', 'e' };
const int primitive_max = 5;
const int unary_max = 2;
const int binary_max = 4;

typedef struct ex_iterator {
    int volume;
    int symbol_index;
    int spread_index;
    int arity;
    struct ex_iterator *child[2];
    double value;
    char symbol;
    bool all;
    bool root;
} ex_iterator;

void ex_init_in(int volume, ex_iterator *iter, bool all, bool root) {
    iter->volume = volume;
    iter->symbol_index = -1;
    iter->spread_index = 0;
    iter->child[0] = NULL;
    iter->child[1] = NULL;
    iter->arity = 0;
    iter->value = 0.;
    iter->symbol = '?';
    iter->all = all;
    iter->root = root;
}

void ex_init(ex_iterator *iter, bool all) {
    ex_init_in(0, iter, all, true);
}

void ex_init_size(int volume, ex_iterator *iter, bool all) {
    ex_init_in(volume, iter, all, false);
}

char ex_buffer[1000];

char *ex_iterator_str_in(char *buffer, ex_iterator *iter) {
    if (iter->volume == 0) {
        switch (iter->symbol) {
            case 'p': strncpy(buffer, "pi", 2); buffer += 2; break;
            default: *(buffer++) = iter->symbol;
        }
        return buffer;
    } else if (iter->spread_index == 0) {
        switch(iter->symbol) {
            case 'l': strncpy(buffer, "ln(", 3); buffer += 3; break;
            case 'c': strncpy(buffer, "cos(", 4); buffer += 4; break;
            default: *(buffer++) = iter->symbol;
        }
        buffer = ex_iterator_str_in(buffer, iter->child[0]);
        switch(iter->symbol) {
            case 'l': case 'c': *(buffer++) = ')'; break;
        }
        return buffer;
    } else {
        *(buffer++) = '(';
        buffer = ex_iterator_str_in(buffer, iter->child[0]);
        switch(iter->symbol) {
            case 'r': strncpy(buffer, "^/", 2); buffer += 2; break;
            default: *(buffer++) = iter->symbol;
        }
        buffer = ex_iterator_str_in(buffer, iter->child[1]);
        *(buffer++) = ')';
        return buffer;
    }
}

char *ex_iterator_str(ex_iterator *iter) {
    if (iter->root) {
        iter = iter->child[0];
    }
    *(ex_iterator_str_in(ex_buffer, iter)) = '\0';
    return ex_buffer;
}

char ex_number_buffer[1000];

char *ex_double_str(double d) {
    memset(ex_number_buffer, 0, 1000);
    snprintf(ex_number_buffer, 999, "%.9e", d);
    return ex_number_buffer;
}

bool ex_is_round(double value) {
    return floor(value) == value;
}

bool ex_is_primitive(double value) {
    for (const double *p = primitives, *e = p + primitive_max; p <= e; p++) {
        if (value == *p || value == -*p) {
            return true;
        }
    }
    return false;
}

bool ex_eval_primitive(ex_iterator *iter) {
    if (iter->symbol_index++ != primitive_max) {
        iter->value = primitives[iter->symbol_index];
        iter->symbol = primitive_symbols[iter->symbol_index];
        iter->arity = 0;
        return true;
    }
    return false;
}

bool ex_eval_unary(ex_iterator *iter) {
    ex_iterator *child = iter->child[0];
    double value = child->value;
    char symbol = child->symbol;
    while (iter->symbol_index != unary_max) {
        switch (++iter->symbol_index) {
            case 0: { // negation
                if ((iter->all || (
                    symbol != '-'
                    && (symbol != 'l' || (child->child[0]->symbol != '/'))
                    && ((symbol != '+' && symbol != '*' && symbol != '/') || (child->child[0]->symbol != '-' && child->child[1]->symbol != '-'))
                ))) {
                    iter->value = -value;
                    iter->symbol = '-';
                    iter->arity = 1;
                    return true;
                }
            } break;
            case 1: { // logarithm
                if ((iter->all || (
                    symbol != '^' && symbol != 'r'
                    && ((symbol != '*' && symbol != '/') || (child->child[0]->symbol != 'e' && child->child[1]->symbol != 'e'))
                    && (symbol != '/' || child->child[0]->value != 1.)
                ))
                    && value > 0. && value != 1.
                ) {
                    iter->value = log(value);
                    if (iter->all || !ex_is_round(iter->value)) {
                        iter->symbol = 'l';
                        iter->arity = 1;
                        return true;
                    }
                }
            } break;
            case 2: { // cosine
                if ((iter->all || (
                    value != M_PI / 3.
                ))
                    && value > 0.001 && value < M_PI / 2.
                ) {
                    iter->value = cos(value);
                    iter->symbol = 'c';
                    iter->arity = 1;
                    return true;
                }
            } break;
        }
    }
    return false;
}

int ex_compare(ex_iterator *iter0, ex_iterator *iter1) {
    if (iter1->arity != iter0->arity) {
        return iter1->arity - iter0->arity;
    }
    if (iter1->symbol != iter0->symbol) {
        return iter1->symbol - iter0->symbol;
    }
    for (int i = 0; i < iter0->arity; i++) {
        int diff = ex_compare(iter0->child[i], iter1->child[i]);
        if (diff) {
            return diff;
        }
    }
    return 0;
}

bool ex_eval_binary(ex_iterator *iter) {
    ex_iterator *child0 = iter->child[0];
    ex_iterator *child1 = iter->child[1];
    double value0 = child0->value;
    double value1 = child1->value;
    char symbol0 = child0->symbol;
    char symbol1 = child1->symbol;
    while (iter->symbol_index != binary_max) {
        switch (++iter->symbol_index) {
            case 0: { // addition
                if ((iter->all || (
                    ex_compare(child0, child1) > 0
                    && (symbol0 != symbol1 || (symbol0 != '-' && symbol0 != 'l' && symbol0 != '+'
                        && (symbol0 != '/' || child0->child[1]->value != child1->child[1]->value)
                    ))
                    && (symbol1 != '+' || ex_compare(child0, child1->child[0]) > 0)
                    && (symbol1 != '*' || (value0 != child1->child[0]->value && value0 != child1->child[1]->value))
                ))
                    && value0 != -value1
                ) {
                    iter->value = value0 + value1;
                    if (iter->all || !ex_is_primitive(iter->value)) {
                        iter->symbol = '+';
                        iter->arity = 2;
                        return true;
                    }
                }
            } break;
            case 1: { // multiplication
                if ((iter->all || (
                    ex_compare(child0, child1) > 0
                    && symbol0 != '-' && symbol1 != '-'
                    && symbol0 != '/' && symbol1 != '/'
                    && ((symbol0 != symbol1) || symbol0 != '*')
                    && (symbol1 != '*' || (value0 != child1->child[0]->value && value0 != child1->child[1]->value))
                    && (symbol1 != '^' || value0 != child1->child[0]->value)
                    && (symbol1 != '*' || ex_compare(child0, child1->child[0]) > 0)
                    && value0 != 1. && value0 != -1.
                    && value1 != 1. && value1 != -1.
                ))) {
                    iter->value = value0 * value1;
                    iter->symbol = '*';
                    iter->arity = 2;
                    return true;
                }
            } break;
            case 2: { // division
                if ((iter->all || (
                    value0 != value1
                    && symbol0 != '-' && symbol1 != '-'
                    && symbol0 != '/' && symbol1 != '/'
                    && (symbol1 != '*' || (value0 != child1->child[0]->value && value0 != child1->child[1]->value))
                    && (symbol0 != '*' || (value1 != child0->child[0]->value && value1 != child0->child[1]->value))
                    && (symbol0 != '+' || (value1 != child0->child[0]->value && value1 != child0->child[1]->value))
                    && ((symbol0 != '*' || symbol1 != '*') || (
                        child0->child[0]->value != child1->child[0]->value
                        && child0->child[1]->value != child1->child[0]->value
                        && child0->child[0]->value != child1->child[1]->value
                        && child0->child[1]->value != child1->child[1]->value
                    ))
                    && ((symbol0 != '*' || symbol1 != '^') || (
                        child0->child[0]->value != child1->child[0]->value
                        && child0->child[1]->value != child1->child[0]->value
                    ))
                    && ((symbol0 != '^' || symbol1 != '*') || (
                        child0->child[0]->value != child1->child[0]->value
                        && child0->child[0]->value != child1->child[1]->value
                    ))
                    && ((symbol1 != '^' && symbol1 != 'r') || value0 != 1.)
                    && (symbol0 != '^' || symbol1 != '^' || child0->child[0]->value != child1->child[0]->value)
                    && (symbol0 != '^' || (child0->child[0]->value != value1))
                    && (symbol1 != 'r' || child1->child[1]->value != 2. || value0 != child1->child[0]->value)
                    && value1 != 1. && value1 != -1.
                ))) {
                    iter->value = value0 / value1;
                    iter->symbol = '/';
                    iter->arity = 2;
                    return true;
                }
            } break;
            case 3: { // root
                if ((iter->all || (
                    symbol0 != '^' && symbol0 != 'r'
                    && value0 != 1.
                    && (symbol0 != '/' || child0->child[0]->value != 1.)
                ))
                    && value0 > 0. && value1 > 1. && ex_is_round(value1)
                ) {
                    iter->value = value1 == 2. ? sqrt(value0) : pow(value0, 1. / value1);
                    if (iter->all || !ex_is_round(iter->value)) {
                        iter->symbol = 'r';
                        iter->arity = 2;
                        return true;
                    }
                }
            } break;
            case 4: { // power
                if ((iter->all || (
                    symbol0 != '^' && symbol0 != 'r'
                    && (symbol0 != 'e' || symbol1 != 'l')
                    && (symbol0 != 'e' || symbol1 != '-' || child1->child[0]->symbol != 'l')
                    && (symbol1 != 'l' || value0 <= child1->child[0]->value)
                    && (symbol0 != '/' || child0->child[0]->value != 1.)
                    && value0 != 1. && value0 != -1. && value1 != 1. && value1 != -1.
                    && !ex_is_round(1. / value1)
                ))
                    && value0 > 0.
                ) {
                    iter->value = pow(value0, value1);
                    iter->symbol = '^';
                    iter->arity = 2;
                    return true;
                }
            } break;
        }
    }
    return false;
}

bool ex_next(ex_iterator *iter) {
    if (iter->root) {
        if (!iter->child[0]) {
            iter->child[0] = iter + 1;
            ex_init_size(iter->volume, iter->child[0], iter->all);
        }
        while (!ex_next(iter->child[0])) {
            ex_init_size(++iter->volume, iter->child[0], iter->all);
        }
        iter->value = iter->child[0]->value;
        return true;
    }
    // if iterating primitives
    if (iter->volume == 0) {
        // return from primitive sub-iterator
        return ex_eval_primitive(iter);
    }
    // using loop to allow jumping with continue statements
    while (true) {
        // if iterating over unary functions
        if (iter->spread_index == 0) {
            // if sub-tree is not yet initialized
            if (!iter->child[0]) {
                iter->child[0] = iter + 1;
                ex_init_size(iter->volume - 1, iter->child[0], iter->all);
                ex_next(iter->child[0]);
                iter->symbol_index = -1;
            }
            // if we cycled through all unary operators, get next sub-tree
            if (iter->symbol_index == unary_max) {
                bool has = ex_next(iter->child[0]);
                if (!has) {
                    if (iter->volume > 1) {
                        // reset iterator to binary
                        iter->spread_index = 1;
                        iter->child[0] = NULL;
                        iter->symbol_index = binary_max;
                        // move on to binary functions
                        continue;
                    } else {
                        // iterator depleted
                        return false;
                    }
                }
                // set symbol iteration to beginning
                iter->symbol_index = -1;
            }
            // if we found a unary symbol that operates on the sub-tree
            if (ex_eval_unary(iter)) {
                // iteration complete, value available
                return true;
            }
            continue;
        }
        // else iterating over binary functions
        // again, using loop to allow jumping with continue statements
        while (true) {
            // if first sub-tree is not yet initialized
            if (!iter->child[0]) {
                iter->child[0] = iter + 1;
                ex_init_size(iter->spread_index - 1, iter->child[0], iter->all);
                ex_next(iter->child[0]);
            }
            // if second sub-tree is not yet initialized
            if (!iter->child[1]) {
                iter->child[1] = iter + (iter->spread_index + 1);
                ex_init_size(iter->volume - (iter->spread_index + 1), iter->child[1], iter->all);
                ex_next(iter->child[1]);
                iter->symbol_index = -1;
            }
            if (iter->symbol_index == binary_max) {
                bool has0 = ex_next(iter->child[1]);
                if (!has0) {
                    bool has1 = ex_next(iter->child[0]);
                    if (!has1) {
                        iter->spread_index++;
                        if (iter->spread_index == iter->volume) {
                            // iterator depleted
                            return false;
                        }
                        // mark second sub-tree for re-initialization
                        iter->child[0] = NULL;
                    }
                    // mark first sub-tree for re-initialization
                    iter->child[1] = NULL;
                    continue;
                }
                // set symbol iteration to beginning
                iter->symbol_index = -1;
            }
            // if we found a binary symbol that operates on the sub-tree
            if (ex_eval_binary(iter)) {
                // iteration complete, value available
                return true;
            }
        }
    }
}

#endif
