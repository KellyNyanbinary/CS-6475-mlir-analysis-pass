#include <assert.h>
#include <stdio.h>

int main(void) {
  int a = 192;
  int b = 48;
  int c = 12;
  int d = 3;
  int e = a | b;
  int f = c | d;
  int g = e & f;

  if (g != 0) {
    return 42;
  }
}