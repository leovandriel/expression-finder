#!/bin/bash

gcc -o ./build/list ./src/list.c && ./build/list $1
