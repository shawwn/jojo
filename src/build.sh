#!/usr/bin/env bash

build ()
{
    # -foptimize-sibling-calls
    # time g++ -o jojo jojo.cpp -Wall
    time clang++ -o jojo jojo.cpp -Wall
}

run () {
    ./tangle.js
    rm -f jojo
    build
    time ./jojo
}

run
