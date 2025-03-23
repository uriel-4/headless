#line 1 "input.hpp"
#include <stdio.h>

#line 3 "input.hpp"
class Class {
#line 4 "input.hpp"
    static int a;
#line 5 "input.hpp"
    static int c;

#line 10 "input.hpp"
    void method();

#line 14 "input.hpp"
    void method2(
#line 15 "input.hpp"
      const int& a = 1,
#line 16 "input.hpp"
      const int& b = 2
#line 17 "input.hpp"
    );

#line 22 "input.hpp"
    const int method3() const;
#line 25 "input.hpp"
};

#line 27 "input.hpp"
extern int b;