#!/bin/bash

gcc -o ./build/missing ./src/missing.c && ./build/missing $1
