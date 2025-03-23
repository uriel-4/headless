#include <stdio.h>

class Class {
    void method() {
        printf("hello world!\n");
    }

    void method2(const int& a = 1, const int& b = 2) {
        printf("hello, %d!\n", a + b);
    }

    const int method3() const {
        return 1;
    }
};