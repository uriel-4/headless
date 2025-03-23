#include <stdio.h>

inline void a() {
    printf("a!\n");
}

class B {
public:
    inline int b(int j = 4) {
        printf("%d\n", j);
    }

};