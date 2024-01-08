#ifndef EX_ITERATOR_H
#define EX_ITERATOR_H

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const double primitives[6] = {1., 2., 3., 5., M_PI, M_E};
const double primitive_symbols[6] = {'1', '2', '3', '5', 'p', 'e'};
const int primitive_max = 5;
const int unary_max = 2;
const int binary_max = 4;

typedef struct ex_iterator
{
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

void ex_init_in(int volume, ex_iterator *iter, bool all, bool root)
{
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

void ex_init(ex_iterator *iter, bool all)
{
    ex_init_in(0, iter, all, true);
}

void ex_init_size(int volume, ex_iterator *iter, bool all)
{
    ex_init_in(volume, iter, all, false);
}

bool ex_is_round(double value)
{
    return floor(value) == value;
}

unsigned char primes[] = {7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251};

bool ex_is_primish(double value)
{
    if (value == M_PI || value == M_E)
    {
        return false;
    }
    double f = floor(value);
    if (f != value)
    {
        return true;
    }
    double a = fabs(f);
    if (a < 7.)
    {
        return false;
    }
    if (a > 256.)
    {
        return true;
    }
    int t = (int)a, left = 0, right = sizeof(primes) - 1;
    while (left <= right)
    {
        int mid = floor((left + right) / 2);
        if (primes[mid] < t)
        {
            left = mid + 1;
        }
        else if (primes[mid] > t)
        {
            right = mid - 1;
        }
        else
        {
            return true;
        }
    }
    return false;
}

bool ex_is_product_of_value(ex_iterator *iter, double value)
{
    if (fabs(iter->value - value) < 1e-12)
    {
        return true;
    }
    if (iter->symbol == '*' || iter->symbol == '/')
    {
        if (ex_is_product_of_value(iter->child[0], value) || ex_is_product_of_value(iter->child[1], value))
        {
            return true;
        }
    }
    else if (iter->symbol == '-' || iter->symbol == '^' || iter->symbol == 'r')
    {
        if (ex_is_product_of_value(iter->child[0], value))
        {
            return true;
        }
    }
    return false;
}

bool ex_is_product_of_symbol(ex_iterator *iter, char symbol)
{
    if (iter->symbol == symbol)
    {
        return true;
    }
    if (iter->symbol == '*')
    {
        if (ex_is_product_of_symbol(iter->child[0], symbol) || ex_is_product_of_symbol(iter->child[1], symbol))
        {
            return true;
        }
    }
    else if (iter->symbol == '-' || iter->symbol == '/')
    {
        if (ex_is_product_of_symbol(iter->child[0], symbol))
        {
            return true;
        }
    }
    return false;
}

bool ex_is_product_of_ln_below(ex_iterator *iter, double value)
{
    if (iter->symbol == 'l' && iter->child[0]->value < value)
    {
        return true;
    }
    if (iter->symbol == '*')
    {
        if (ex_is_product_of_ln_below(iter->child[0], value) || ex_is_product_of_ln_below(iter->child[1], value))
        {
            return true;
        }
    }
    else if (iter->symbol == '-' || iter->symbol == '/')
    {
        if (ex_is_product_of_ln_below(iter->child[0], value))
        {
            return true;
        }
    }
    return false;
}

bool ex_eval_primitive(ex_iterator *iter)
{
    if (iter->symbol_index++ != primitive_max)
    {
        iter->value = primitives[iter->symbol_index];
        iter->symbol = primitive_symbols[iter->symbol_index];
        iter->arity = 0;
        return true;
    }
    return false;
}

int ex_is_normal(double value) 
{
    return isnormal(value) && ((value > 1e-20 && value < 1e20) || (value < -1e-20 && value > -1e20));
}

int ex_is_primitive(char symbol)
{
    return symbol == '1' || symbol == '2' || symbol == '3' || symbol == '5'; // || symbol == 'p' || symbol == 'e';
}

bool ex_eval_unary(ex_iterator *iter)
{
    ex_iterator *child = iter->child[0];
    double value = child->value;
    char symbol = child->symbol;
    while (iter->symbol_index != unary_max) {
        switch (++iter->symbol_index) {
            case 0: { // negation
                if ((iter->all || (
                    symbol != '-' // --x = x
                    && (symbol != 'l' || (child->child[0]->symbol != '/')) // -log(x/y) = log(y/x)
                    && ((symbol != '+' && symbol != '*' && symbol != '/') || (
                        child->child[0]->symbol != '-' && child->child[1]->symbol != '-' // -(x-y) = y-x, -(-x*y) = x*y, -(-x/y) = x/y
                    ))
                ))) {
                    iter->value = -value;
                    iter->symbol = '-';
                    iter->arity = 1;
                    return true;
                }
            } break;
            case 1: { // logarithm
                if ((iter->all || (
                    symbol != '^' && symbol != 'r' // log(x^y) = y*log(x), log(sqrt(x)) = log(x)/2
                    && !ex_is_product_of_value(iter, M_E) // log(e*x)
                    && (symbol != '/' || child->child[0]->value != 1.) // log(1/x) = -log(x)
                ))
                    && value > 0. && value != 1.
                ) {
                    double value0 = log(value);
                    if (iter->all || !ex_is_round(value0)) { // log(10) = 1
                        iter->value = value0;
                        iter->symbol = 'l';
                        iter->arity = 1;
                        return true;
                    }
                }
            } break;
            case 2: { // cosine
                if ((iter->all || (
                    value != M_PI / 3. // cos(pi/3) = 1/2
                    && (symbol != '/' || child->child[0]->symbol != 'p' || (child->child[1]->value != 4. && child->child[1]->value != 6.)) // cos(pi/4) = sqrt(2)/2
                ))
                    && value > 0.0045 // cos(0.0045) ~= 1 (0.99999)
                    && value < M_PI / 2. // cos(pi/2) = 0
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

int ex_compare(ex_iterator *iter0, ex_iterator *iter1)
{
    if (iter1->arity != iter0->arity)
    {
        return iter1->arity - iter0->arity;
    }
    if (iter1->symbol != iter0->symbol)
    {
        return iter1->symbol - iter0->symbol;
    }
    for (int i = 0; i < iter0->arity; i++)
    {
        int diff = ex_compare(iter0->child[i], iter1->child[i]);
        if (diff)
        {
            return diff;
        }
    }
    return 0;
}

bool ex_eval_binary(ex_iterator *iter)
{
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
                    ex_compare(child0, child1) > 0 // 2+1 = 1+2
                    && (symbol0 != symbol1 || (symbol0 != '-' && symbol0 != 'l' && symbol0 != '+' // x+x = 2*x
                        && (symbol0 != '/' || symbol1 != '/' || child0->child[1]->value != child1->child[1]->value) // y/x+z/x = (y+z)/x
                    ))
                    && (symbol0 != '-' || child0->child[0]->symbol != 'l' || symbol1 != 'l') // -ln(x)+ln(y) = ln(y/x)
                    && (symbol1 != '+' || ex_compare(child0, child1->child[0]) > 0) // 2+(1+x) = 1+(2+x)
                    && (symbol1 != '+' || ex_is_primish(value0 + child1->child[0]->value)) // 1+(2+x) = 3+x
                    && (symbol1 != '*' || (value0 != child1->child[0]->value && value0 != child1->child[1]->value)) // x+(x*y) = (x+1)*y
                    && (symbol1 != '/' || !ex_is_round(value0) || !ex_is_round(child1->child[0]->value) || !ex_is_round(child1->child[1]->value)) // x+(y/z) = (x*z+y)/z
                    && (symbol1 != '-' || child1->child[0]->symbol != '/' || !ex_is_round(value0) || !ex_is_round(child1->child[0]->child[0]->value) || !ex_is_round(child1->child[0]->child[1]->value)) // x-(y/z) = (x*z-y)/z
                    && (value0 != 1. || (value1 != 6. && value1 != 10. && value1 != -8. && value1 != .25)) // 1+2*3 = 2+5, 1+2*5 = 2+3^2
                    && (value0 != 2. || (value1 != -9. && value1 != -15.)) // 2-3^2 = -(2+5)
                    && (value0 != 3. || (value1 != 4. && value1 != 10. && value1 != 8. && value1 != 125.)) // 2+5 = 3+2^2, 3+2*5 = 5+2^3, 3+2^3 = 2+3^2, 3+5^3 = 2^(2+5)
                    && (value0 != 5. || (value1 != 6.)) // 5+2*3 = 2+3^2
                    && ((symbol0 != 'p' && symbol0 != 'e') || (symbol1 != '^' && symbol1 != '/') || child1->child[0]->symbol != symbol0  || child1->child[1]->value != 2. ) // pi+pi^2 = pi*(1+pi), pi+pi/2 = 3*pi/2
                    && (!ex_is_primitive(symbol0) || symbol1 != '-' || child1->child[0]->symbol != '+' || !ex_is_primitive(child1->child[0]->child[0]->symbol)) // 1-(1+pi) = -pi 
                    && (!ex_is_primitive(symbol0) || symbol1 != '+' || child1->child[1]->symbol != '-' || !ex_is_primitive(child1->child[1]->child[0]->symbol)) // 1+pi-2 = pi-1 
                ))
                    && value0 != -value1
                ) {
                    double value = value0 + value1;
                    if ((iter->all || (value1 == 125. || value1 == 216. || value1 == 225. || fabs(value1) == 243. || ex_is_primish(value))) && ex_is_normal(value)) {
                        iter->value = value;
                        iter->symbol = '+';
                        iter->arity = 2;
                        return true;
                    }
                }
            } break;
            case 1: { // multiplication
                if ((iter->all || (
                    ex_compare(child0, child1) > 0 // 3*2 = 2*3
                    && symbol0 != '-' && symbol1 != '-' // -x*-y = x*y
                    && symbol0 != '/' && symbol1 != '/' // (x/y)*(z/w) = (x*z)/(y*w)
                    // && ((symbol0 != symbol1) || symbol0 != '*')
                    && (symbol1 != '*' || (value0 != child1->child[0]->value && value0 != child1->child[1]->value)) // x*(x*y) = x^2*y
                    && ((symbol1 != '^' && symbol1 != 'r') || value0 != child1->child[0]->value) // x*x^y = x^(y+1)
                    && (symbol1 != '^' || child1->child[1]->symbol != '-') // x*y^-z = x/y^z
                    && (symbol1 != '*' || ex_compare(child0, child1->child[0]) > 0) // 3*(2*x) = 2*(3*x)
                    && value0 != 1. && value0 != -1. // 1*x = x, -1*x = -x
                    && value1 != 1. && value1 != -1. // x*1 = x, x*-1 = -x
                ))) {
                    double value = value0 * value1;
                    if (ex_is_normal(value)) {
                        iter->value = value;
                        iter->symbol = '*';
                        iter->arity = 2;
                        return true;
                    }
                }
            } break;
            case 2: { // division
                if ((iter->all || (
                    value0 != value1 // x/x = 1
                    && symbol0 != '-' && symbol1 != '-' // -x/-y = x/y
                    && symbol0 != '/' && symbol1 != '/' // (x/y)/(z/w) = (x*w)/(y*z)
                    && (symbol1 != '*' || (value0 != child1->child[0]->value && value0 != child1->child[1]->value)) // x/(x*y) = 1/y
                    && (symbol0 != '*' || (value1 != child0->child[0]->value && value1 != child0->child[1]->value)) // (x*y)/x = y
                    && (symbol0 != '+' || (value1 != child0->child[0]->value && value1 != child0->child[1]->value)) // (x+y)/x = 1+y/x
                    && ((symbol0 != '*' || symbol1 != '*') || (
                        child0->child[0]->value != child1->child[0]->value // (x*y)/(x*z) = y/z
                        && child0->child[1]->value != child1->child[0]->value // (y*x)/(x*z) = y/z
                        && child0->child[0]->value != child1->child[1]->value // (x*y)/(z*x) = y/z
                        && child0->child[1]->value != child1->child[1]->value // (y*x)/(z*x) = y/z
                    ))
                    && ((symbol0 != '*' || symbol1 != '^') || (
                        child0->child[0]->value != child1->child[0]->value // (x*y)/(x^z) = y/(x^(z-1))
                        && child0->child[1]->value != child1->child[0]->value // (y*x)/(x^z) = y/(x^(z-1))
                    ))
                    && ((symbol0 != '^' || symbol1 != '*') || (
                        child0->child[0]->value != child1->child[0]->value // (x^y)/(x*z) = (x^(y-1))/z
                        && child0->child[0]->value != child1->child[1]->value // (x^y)/(z*x) = (x^(y-1))/z
                    ))
                    && ((symbol1 != '^' && symbol1 != 'r') || value0 != child1->child[0]->value) // x/(x^y) = x^(1-y)
                    && ((symbol0 != '^' && symbol0 != 'r') || child0->child[0]->value != value1) // x^y/x = x^(y-1)
                    && (symbol1 != '^' || value0 != 1.) // 1/(x^y) = x^(-y)
                    && (symbol0 != '^' || symbol1 != '^' || child0->child[0]->value != child1->child[0]->value) // (x^y)/(x^z) = x^(y-z)
                    && (symbol1 != '^' || child1->child[1]->symbol != '-') // x/y^-z = x*y^z
                    && (symbol1 != 'r' || child1->child[1]->value != 2. || value0 != child1->child[0]->value) // x/(x^/2) = x^/2
                    && (symbol0 != 'r' || child0->child[1]->value != 2. || value1 != child0->child[0]->value) // (x^/2)/x = 1/x^/2
                    && value1 != 1. && value1 != -1. // x/1 = x
                ))) {
                    double value = value0 / value1;
                    if (ex_is_normal(value)) {
                        iter->value = value;
                        iter->symbol = '/';
                        iter->arity = 2;
                        return true;
                    }
                }
            } break;
            case 3: { // root
                if ((iter->all || (
                    symbol0 != '^' && symbol0 != 'r' // (x^y)^/z
                    && value0 != 1. // 1^/x = 1
                    && (symbol0 != '/' || child0->child[0]->value != 1.) // (1/x)^/y = x^/-y
                ))
                    && value0 > 0. // -1^/x = nan
                    && value1 > 1. && ex_is_round(value1) // x^/2.5 = x^.4
                    && value1 < 1000 // x^/1000 ~= 1 
                ) {
                    double value = value1 == 2. ? sqrt(value0) : pow(value0, 1. / value1);
                    if ((iter->all || !ex_is_round(value)) && ex_is_normal(value)) { // 4^/2 = 2
                        iter->value = value;
                        iter->symbol = 'r';
                        iter->arity = 2;
                        return true;
                    }
                }
            } break;
            case 4: { // power
                if ((iter->all || (
                    symbol0 != '^' && symbol0 != 'r' // (x^y)^z = x^(y*z)
                    && (symbol0 != '/' || symbol1 != '-') // (x/y)^-z = (y/x)^z
                    && (symbol0 != 'e' || symbol1 != 'l') // e^log(x) = x
                    && (symbol0 != 'e' || !ex_is_product_of_symbol(child1, 'l')) // e^(x*log(y)) = y^x
                    && (symbol0 != 'e' || symbol1 != '-' || child1->child[0]->symbol != 'l') // e^-log(x) = 1/x
                    && (symbol1 != 'l' || value0 <= child1->child[0]->value) // 3^log(2) = 2^log(3)
                    && (symbol1 != '-' || child1->child[0]->symbol != 'l' || value0 <= child1->child[0]->child[0]->value) // 3^-log(2) = 2^-log(3)
                    && !ex_is_product_of_ln_below(child1, value0) // 3^(x*ln(2))) = 2^(x*ln(3))
                    && (symbol0 != '/' || child0->child[0]->value != 1.) // (1/x)^y = x^-y
                    && (symbol1 != '/' || child1->child[1]->symbol != 'l' || child1->child[1]->child[0]->value != value0) // x^(y/ln(x)) = e^y
                    && value0 != 1. && value0 != -1. && value1 != 1. && value1 != -1. // 1^x = 1
                ))
                    && value0 > 0. // -1^x = nan
                ) {
                    double root = 1. / value1;
                    if (iter->all || !ex_is_round(root) || root >= 1000) { // x^(1/2) = x^/2
                        double value = pow(value0, value1);
                        if (ex_is_normal(value)) {
                            iter->value = value;
                            iter->symbol = '^';
                            iter->arity = 2;
                            return true;
                        }
                    }
                }
            } break;
        }
    }
    return false;
}

bool ex_next(ex_iterator *iter)
{
    if (iter->root)
    {
        if (!iter->child[0])
        {
            iter->child[0] = iter + 1;
            ex_init_size(iter->volume, iter->child[0], iter->all);
        }
        while (!ex_next(iter->child[0]))
        {
            ex_init_size(++iter->volume, iter->child[0], iter->all);
        }
        iter->value = iter->child[0]->value;
        return true;
    }
    // if iterating primitives
    if (iter->volume == 0)
    {
        // return from primitive sub-iterator
        return ex_eval_primitive(iter);
    }
    // using loop to allow jumping with continue statements
    while (true)
    {
        // if iterating over unary functions
        if (iter->spread_index == 0)
        {
            // if sub-tree is not yet initialized
            if (!iter->child[0])
            {
                iter->child[0] = iter + 1;
                ex_init_size(iter->volume - 1, iter->child[0], iter->all);
                ex_next(iter->child[0]);
                iter->symbol_index = -1;
            }
            // if we cycled through all unary operators, get next sub-tree
            if (iter->symbol_index == unary_max)
            {
                bool has = ex_next(iter->child[0]);
                if (!has)
                {
                    if (iter->volume > 1)
                    {
                        // reset iterator to binary
                        iter->spread_index = 1;
                        iter->child[0] = NULL;
                        iter->symbol_index = binary_max;
                        // move on to binary functions
                        continue;
                    }
                    else
                    {
                        // iterator depleted
                        return false;
                    }
                }
                // set symbol iteration to beginning
                iter->symbol_index = -1;
            }
            // if we found a unary symbol that operates on the sub-tree
            if (ex_eval_unary(iter))
            {
                // iteration complete, value available
                return true;
            }
            continue;
        }
        // else iterating over binary functions
        // again, using loop to allow jumping with continue statements
        while (true)
        {
            // if first sub-tree is not yet initialized
            if (!iter->child[0])
            {
                iter->child[0] = iter + 1;
                ex_init_size(iter->spread_index - 1, iter->child[0], iter->all);
                ex_next(iter->child[0]);
            }
            // if second sub-tree is not yet initialized
            if (!iter->child[1])
            {
                iter->child[1] = iter + (iter->spread_index + 1);
                ex_init_size(iter->volume - (iter->spread_index + 1), iter->child[1], iter->all);
                ex_next(iter->child[1]);
                iter->symbol_index = -1;
            }
            if (iter->symbol_index == binary_max)
            {
                bool has0 = ex_next(iter->child[1]);
                if (!has0)
                {
                    bool has1 = ex_next(iter->child[0]);
                    if (!has1)
                    {
                        iter->spread_index++;
                        if (iter->spread_index == iter->volume)
                        {
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
            if (ex_eval_binary(iter))
            {
                // iteration complete, value available
                return true;
            }
        }
    }
}

#endif
