#!/bin/bash

gcc -o ./build/stats ./src/stats.c && ./build/stats $1
