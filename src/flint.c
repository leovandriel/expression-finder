//usr/bin/gcc "$0" -o bin/flint -Wall -Wextra -O3 -lm -lflint -lmpfr && ./bin/flint "$@"; exit $?
#include <time.h>
#include <stdio.h>
#include <math.h>

#include "flint/arb.h"
#include "mpfr.h"

int main()
{
	size_t count = 1e7;
	int digits = 17;

	{
		double a = .5, b = .5, c = 0, d = 0;
		clock_t start = clock();
		for (size_t i = 0; i < count; i++)
		{
			c = pow(a, b);
			d += c;
		}
		double delta = (clock() - start) / (double)CLOCKS_PER_SEC / count * 1e9;
		if (d > 0)
		{
			printf("math: %8.2fns (%.17f)\n", delta, c);
		}
	}

	{
		int prec = ceil(digits * log(10) / log(2));
		arf_t a, b, c;
		arf_init(a);
		arf_init(b);
		arf_init(c);
		arf_set_d(a, .5);
		arf_set_d(b, .5);
		clock_t start = clock();
		for (size_t i = 0; i < count; i++)
		{
			arf_sqrt(c, a, prec, ARF_RND_NEAR);
		}
		double delta = (clock() - start) / (double)CLOCKS_PER_SEC / count * 1e9;
		arf_clear(a);
		arf_clear(b);
		arf_clear(c);
		printf("arf : %8.2fns (%s)\n", delta, arf_get_str(c, digits));
	}

	{
		int prec = ceil(digits * log(10) / log(2));
		arb_t a, b, c;
		arb_init(a);
		arb_init(b);
		arb_init(c);
		arb_set_d(a, .5);
		arb_set_d(b, .5);
		clock_t start = clock();
		for (size_t i = 0; i < count; i++)
		{
			arb_pow(c, a, b, prec);
		}
		double delta = (clock() - start) / (double)CLOCKS_PER_SEC / count * 1e9;
		printf("arb : %8.2fns (%s)\n", delta, arb_get_str(c, prec, ARB_STR_NO_RADIUS));
		arb_clear(a);
		arb_clear(b);
		arb_clear(c);
	}

	{
		int prec = ceil(digits * log(10) / log(2));
		mpfr_t a, b, c;
		mpfr_init2(a, prec);
		mpfr_init2(b, prec);
		mpfr_init2(c, prec);
		mpfr_set_d(a, .5, MPFR_RNDN);
		mpfr_set_d(b, .5, MPFR_RNDN);
		clock_t start = clock();
		for (size_t i = 0; i < count; i++)
		{
			mpfr_pow(c, a, b, MPFR_RNDN);
		}
		double delta = (clock() - start) / (double)CLOCKS_PER_SEC / count * 1e9;
		long e;
		printf("mpfr: %8.2fns (0.%s)\n", delta, mpfr_get_str(NULL, &e, 10, digits, c, MPFR_RNDN));
		mpfr_clear(a);
		mpfr_clear(b);
		mpfr_clear(c);
	}

	for (int digits = 10; digits <= 100; digits += digits < 20 ? 1 : 10)
	{
		int prec = ceil(digits * log(10) / log(2));
		arb_t a, b, c;
		arb_init(a);
		arb_init(b);
		arb_init(c);
		arb_set_d(a, .5);
		arb_set_d(b, .5);
		clock_t start = clock();
		for (size_t i = 0; i < count; i++)
		{
			arb_pow(c, a, b, prec);
		}
		double delta = (clock() - start) / (double)CLOCKS_PER_SEC / count * 1e9;
		printf("arb : %8.2fns  %3d %5.2fns (%s)\n", delta, digits, delta / digits, arb_get_str(c, prec, ARB_STR_NO_RADIUS));
		arb_clear(a);
		arb_clear(b);
		arb_clear(c);
	}

	return 0;
}