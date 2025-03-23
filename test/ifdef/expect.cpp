#include "expect.hpp"

#if defined(A)
void Class::methodA(int a, int b) {
    printf("a = %d, b = %d\n", a, b);
};
#endif
#if !defined(A) && defined(B)
int Class::methodB(bool a) {
    printf("a = %d\n", a);
};
#endif
#if !(defined(A) || defined(B))
int Class::methodC(bool a) {
    printf("a = %d\n", a);
};
#endif
int Class::methodD() {
    printf("methodD\n");
};