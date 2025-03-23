#include <stdio.h>

class Class {
    static int a = 1;
    static int c = 1 +
        2 +
        3 +
        4;

    void method() {
        printf("hello world!\n");
    }

    void method2(
      const int& a = 1,
      const int& b = 2
    ) {
        printf("hello, sum of %d and %d: %d!\n", a, b, a + b);
        printf("another hello\n");
    }

    const int method3() const {
        return 1;
    }
};

int b = 2;