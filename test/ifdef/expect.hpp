#include <stdio.h>

#define A 1
#define B 2

class Class {
/* test comment */ #ifdef A
    void methodA(int a = 1, int b = 2);
#elif defined(B)
    int methodB(bool a = true);
#else
    int methodC(bool a = true);
#endif
// #ifdef muted
    int methodD();
// #endif
};