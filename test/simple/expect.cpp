#include "expect.hpp"

void Class::method() {
    printf("hello world!\n");
};
void Class::method2(const int& a, const int& b) {
    printf("hello, sum of %d and %d: %d!\n", a, b, a + b);
};
const int Class::method3() const {
    return 1;
};
