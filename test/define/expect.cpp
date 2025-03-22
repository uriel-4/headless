#include "expect.hpp"

#ifdef A
void Class::methodA(int a, int b) {
    printf("a = %d, b = %d\n", a, b);
}
#endif
#ifdef B
int Class::methodB(bool a = true) {
    printf("a = %d\n", a);
}
#endif
#if !(A || B)
int Class::methodC(bool a = true) {
    printf("a = %d\n", a);
}
#endif
int Class::methodD() {
    printf("methodD\n");
}