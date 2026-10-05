[ -f input1.c ] && clang -S -emit-llvm input1.c -o input1.ll
[ -f input2.c ] && clang -S -emit-llvm input2.c -o input2.ll
[ -f input3.c ] && clang -S -emit-llvm input3.c -o input3.ll
[ -f test1.sh ] && chmod +x test1.sh
[ -f test2.sh ] && chmod +x test2.sh
[ -f test3.sh ] && chmod +x test3.sh