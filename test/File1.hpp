#include <stdio.h>
#include <string>

#define ABC 1
int a = 1;

#define SOME_MACRO 5

extern int bdd;
namespace name {
    class A {
      public:
          int cdd3 = 1;
        void b(int a = 3, int b = 4) {

        }
        static int aaa = SOME_MACRO;
        static constexpr double fpi = 3.14159;
        const std::string cs = "Hello";
        static int aaad;
        std::vector<int> v = {1, 2, 3};
    };
}
#ifdef ABC
int h = 4;
#endif