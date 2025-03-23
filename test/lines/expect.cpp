#include "expect.hpp"

#line 4 "input.hpp"
int Class::a = 1;

#line 5 "input.hpp"
int Class::c = 1 +
#line 6 "input.hpp"
        2 +
#line 7 "input.hpp"
        3 +
#line 8 "input.hpp"
        4;

#line 10 "input.hpp"
void Class::method() {
#line 11 "input.hpp"
    printf("hello world!\n");
#line 12 "input.hpp"
};

#line 14 "input.hpp"
void Class::method2(
#line 15 "input.hpp"
    const int& a,
#line 16 "input.hpp"
    const int& b
) {
#line 18 "input.hpp"
    printf("hello, sum of %d and %d: %d!\n", a, b, a + b);
#line 19 "input.hpp"
    printf("another hello\n");
#line 20 "input.hpp"
};

#line 22 "input.hpp"
const int Class::method3() const {
#line 23 "input.hpp"
    return 1;
#line 24 "input.hpp"
};

#line 27 "input.hpp"
int b = 2;
