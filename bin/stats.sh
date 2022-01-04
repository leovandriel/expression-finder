#!/bin/bash

gcc -O3 -o ./build/stats ./src/stats.c && ./build/stats $1
