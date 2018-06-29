#!/usr/bin/env bash

build ()
{
    # time g++ -o jojo jojo.cpp
    time clang++ -o jojo jojo.cpp
}

run () {
    ./tangle.js
    build
    ./jojo
}

run
