# Expression Finder

*Find mathematical expressions given a number.*

Web demo: [https://ex.leovandriel.com](https://ex.leovandriel.com)

## Why

Sometimes you have a decimal number and wonder if there is a closed-form
expression for it.

## How

By iterating over all possible expressions and seeing which one evaluates to the
same decimal expansion.

It combines:

- Constants: 1, 2, 3, 5, e, and pi
- Operators: negation, addition, multiplication, division
- Functions: exponentiation, root, logarithm, and cosine

## Usage

Run in CLI:

    ./src/find.c [number]

To run web server locally, install [Emscripten](https://emscripten.org), then:

    ./build.sh
    ./serve.sh
    open http://localhost:8000

## See also

- [https://mrob.com/pub/ries/index.html](https://mrob.com/pub/ries/index.html)
- [https://isc.carma.newcastle.edu.au](https://isc.carma.newcastle.edu.au)
- [https://oeis.org](https://oeis.org)
- [https://xkcd.com/1047](https://xkcd.com/1047)
- [https://katex.org](https://katex.org)

## Performance

The search tree is pruned by skipping operations that have an identical
counterpart given the sub-tree. Optimization candidates can be found by looking
for duplicate expressions, see `collision.c`. Excessive pruning can lead to
missing values, which can be detected by comparing the optimized and
un-optimized iterators, see `missing.c`. To get an overview of expression count,
see `stats.c`.

The finder can generate over 1M expressions per second on a 2.4GhHz Core i5
machine. With every size increase, the number of expressions grows with
approximately a factor 7. Expressions of up to 10 nodes are generated within
seconds, see `bench.c`. Beyond that the time in seconds is about `1e-8 *
7^nodes`.

## License

MIT
