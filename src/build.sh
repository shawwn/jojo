#!/usr/bin/env bash

# c="g++"
c="clang++"

# f="-Wall"
f="-Wall -foptimize-sibling-calls"
# f="-Wall -foptimize-sibling-calls -O2"

build ()
{
    time $c $f jojo.cpp -o jojo
}

test () {
    ./tangle.js
    rm -f jojo
    build
    time ./jojo
    # time valgrind ./jojo
    # time valgrind --leak-check=full ./jojo
}

test
