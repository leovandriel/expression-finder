#ifndef EX_FLINT_H
#define EX_FLINT_H

#include <flint/arb.h>

#include "iterator.h"

void ex_arb_eval(arb_t result, ex_iterator *iter, int prec)
{
    if (iter->root)
    {
        iter = iter->child[0];
    }
    if (iter->symbol == '1' || iter->symbol == '2' || iter->symbol == '3' || iter->symbol == '5')
    {
        arb_set_d(result, iter->value);
    }
    else if (iter->symbol == 'e')
    {
        arb_const_e(result, prec);
    }
    else if (iter->symbol == 'p')
    {
        arb_const_pi(result, prec);
    }
    else if (iter->symbol == '+')
    {
        arb_t v;
        arb_init(v);
        ex_arb_eval(result, iter->child[0], prec);
        ex_arb_eval(v, iter->child[1], prec);
        arb_add(result, result, v, prec);
    }
    else if (iter->symbol == '*')
    {
        arb_t v;
        arb_init(v);
        ex_arb_eval(result, iter->child[0], prec);
        ex_arb_eval(v, iter->child[1], prec);
        arb_mul(result, result, v, prec);
    }
    else if (iter->symbol == '/')
    {
        arb_t v;
        arb_init(v);
        ex_arb_eval(result, iter->child[0], prec);
        ex_arb_eval(v, iter->child[1], prec);
        arb_div(result, result, v, prec);
    }
    else if (iter->symbol == '^')
    {
        arb_t v;
        arb_init(v);
        ex_arb_eval(result, iter->child[0], prec);
        ex_arb_eval(v, iter->child[1], prec);
        arb_pow(result, result, v, prec);
    }
    else if (iter->symbol == 'r')
    {
        arb_t v;
        arb_init(v);
        ex_arb_eval(result, iter->child[0], prec);
        ex_arb_eval(v, iter->child[1], prec);
        arb_inv(v, v, prec);
        arb_pow(result, result, v, prec);
    }
    else if (iter->symbol == '-')
    {
        ex_arb_eval(result, iter->child[0], prec);
        arb_neg_round(result, result, prec);
    }
    else if (iter->symbol == 'l')
    {
        ex_arb_eval(result, iter->child[0], prec);
        arb_log(result, result, prec);
    }
    else if (iter->symbol == 'c')
    {
        ex_arb_eval(result, iter->child[0], prec);
        arb_cos(result, result, prec);
    }
    else
    {
        printf("!!unknow op: %c\n", iter->symbol);
    }
}

#endif // EX_FILT_H