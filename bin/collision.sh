#!/bin/bash

gcc -o ./build/collision ./src/collision.c && ./build/collision $1
