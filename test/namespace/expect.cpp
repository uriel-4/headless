#include "expect.hpp"

void Namespace::Class::methodA(int a, int b) {
    printf("a = %d, b = %d\n", a, b);
};
int Namespace::Class::methodB(bool a) {
    printf("a = %d\n", a);
};
int Namespace::Class::methodC(bool a) {
    printf("a = %d\n", a);
};
int Namespace::Class::methodD() {
    printf("methodD\n");
};