#!/usr/bin/env bash
test -f input1.c && llvm-reduce --test=test1.sh input1.ll -o reduced1.mlir &> /dev/null ; echo $?
test -f input2.c && llvm-reduce --test=test2.sh input2.ll -o reduced2.mlir &> /dev/null ; echo $?
test -f input3.c && llvm-reduce --test=test3.sh input3.ll -o reduced3.mlir &> /dev/null ; echo $?
./cleanup.sh