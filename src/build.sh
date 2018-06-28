#!/usr/bin/env bash

build ()
{
    # time g++ -o jojo jojo.cpp
    time clang++ -o jojo jojo.cpp
}

run () {
    build
    ./jojo
}

run
