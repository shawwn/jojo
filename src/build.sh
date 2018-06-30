#!/usr/bin/env bash

build ()
{
    # -foptimize-sibling-calls
    # time g++ -o jojo jojo.cpp
    time clang++ -o jojo jojo.cpp
}

run () {
    ./tangle.js
    build
    ./jojo
}

run
