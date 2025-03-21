#include <stdio>

#define SOME_MACRO 1

class A {
#ifdef SOME_MACRO
    void b();
#endif
    void c();
};