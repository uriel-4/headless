#include <stdio.h>

class Class {
  void method() {
    printf("hello world!\n");
  }

  void method2(
    const int& a = 1,
    const int& b = 2
  ) {
    printf("hello, sum of %d and %d: %d!\n", a, b, a + b);
  }

  const int method3() const {
    return 1;
  }
};