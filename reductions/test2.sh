#!/usr/bin/env bash
clang --analyze -Xanalyzer -analyzer-checker=core.DivideZero input2.c 2>&1 | grep "Division by zero"