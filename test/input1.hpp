#include <stdio.h>
#include <string>

#define ABC 0
int a = 1;

#define SOME_MACRO 5

extern int bdd;
namespace name {
    class A {
    public:
        static int cdd3 = 1;
        void b(int a = 3, int b = 4) {
            int j = a + b;
            printf("%d\n", j);
        }
        static int aaa = SOME_MACRO;
        static constexpr double fpi = 3.14159;
        const std::string cs = "Hello";
        static int aaad;
        std::vector<int> v = {1, 2, 3};
    };
}

//#ifdef ABC
//int h_33 = 4;
//#endif
#if ABC
int bacc_dd = 32;
#endif

int xxxx = 1;