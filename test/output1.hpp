#include <stdio.h>
#include <string>

#define ABC 1
int a = 1;

#define SOME_MACRO 5

extern int bdd;
namespace name {
    class A {
    public:
        int cdd3;
        void b(int a = 3, int b = 4);
        static int aaa;
        static double fpi;
        const std::string cs = "Hello";
        static int aaad;
        std::vector<int> v = {1, 2, 3};
    };
}

#ifdef ABC
int h = 4;
#endif