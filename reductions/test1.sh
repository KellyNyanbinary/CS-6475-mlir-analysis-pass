#!/usr/bin/env bash
clang $1 -o input1
./input1 2>&1 | grep "Assertion failed:"