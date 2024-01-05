Expression Finder
=================

*Find mathematical expressions given a number.*

# Why

Sometimes you have a decimal number and wonder if there is a closed-form expression for it.

# How

By iterating over all possible expressions and seeing which one evaluates to the same decimal expansion.

It combines:
- Primitives: 1, 2, 3, 5, e, and pi
- Operations: negation, addition, multiplication, division
- Functions: exponentiation, root, logarithm, and cosine

# Usage

View on the web: [https://ex.leovandriel.com](https://ex.leovandriel.com)

Run locally, first install `emscripten`:

    ./bin/build.sh
    ./bin/serve.sh
    open http://localhost:8000

Build for web, install [Emscripten](https://emscripten.org) and run `./bin/build.sh`.

Run in CLI:

	./bin/find.sh [number]

# See also

- [https://mrob.com/pub/ries/index.html](https://mrob.com/pub/ries/index.html)
- [https://isc.carma.newcastle.edu.au](https://isc.carma.newcastle.edu.au)
- [https://oeis.org](https://oeis.org)
- [https://xkcd.com/1047](https://xkcd.com/1047)
- [https://katex.org](https://katex.org)

# Performance

The search tree is pruned by skipping operations that have an identical counterpart given the sub-tree, see use of `iter->all`. Optimization candidates can be found by looking for duplicate expressions, see `collision.c`. Excessive pruning can lead to missing values, which can be detected by comparing the optimized and un-optimized iterators, see `missing.c`. To get an overview of expression count, see `stats.c`.

The finder can generate over 1M expressions per second on a 2.4GhHz Core i5 machine. With every node added, the number of expressions grows with a factor 7. Expressions of up to 10 nodes are generated within seconds, see `bench.c`. Beyond that the time in seconds is about `1e-8 * 7^nodes`.

# License

MIT
