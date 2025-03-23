#include <stdio.h>

#define A 1
#define B 2

class Class {

/* test comment */ #ifdef A
    void methodA(int a = 1, int b = 2) {
        printf("a = %d, b = %d\n", a, b);
    }

#elif defined(B)
    int methodB(bool a = true) {
        printf("a = %d\n", a);
    }
#else

    int methodC(bool a = true) {
        printf("a = %d\n", a);
    }
#endif

// #ifdef muted
    int methodD() {
        printf("methodD\n");
    }
// #endif

};