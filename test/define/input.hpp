#include <stdio.h>

#define A 1
#define B 2

class Class {

#ifdef A
    void methodA(int a = 1, int b = 2) {
        printf("a = %d, b = %d\n", a, b);
    }

#elifdef B
    int methodB(bool a = true) {
        printf("a = %d\n", a);
    }
#else

    int methodC(bool a = true) {
        printf("a = %d\n", a);
    }
#endif

    int methodD() {
        printf("methodD\n");
    }

};