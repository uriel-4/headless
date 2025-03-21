#include <stdio>

#define SOME_MACRO 1

class A {
  #ifdef SOME_MACRO
  void b() {
    printf("b exists!\n");
  }
  #endif
  void c() {
    printf("c exists!\n");
  }
};