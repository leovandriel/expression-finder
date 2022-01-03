#!/bin/bash

gcc -o ./build/find ./src/find.c && ./build/find $1
