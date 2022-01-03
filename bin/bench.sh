#!/bin/bash

gcc -O3 -o ./build/bench ./src/bench.c && ./build/bench $1
