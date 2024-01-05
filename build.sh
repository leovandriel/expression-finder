#!/bin/bash

emcc -Wall -Wextra -O3 -lm ./src/find.c -o ./dist/find.js -s WASM=1 -s EXPORTED_FUNCTIONS='["_next","_malloc","_free"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]'
