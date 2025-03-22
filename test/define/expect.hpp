#include <stdio.h>

#define A 1
#define B 2

class Class {
#ifdef A
    void methodA(int a = 1, int b = 2);
#elifdef B
    int methodB(bool a = true);
#else
    int methodC(bool a = true);
#endif
    int methodD();
};