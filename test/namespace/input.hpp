#include <stdio.h>

namespace Namespace {
  class Class {

    void methodA(int a = 1, int b = 2) {
      printf("a = %d, b = %d\n", a, b);
    }

    int methodB(bool a = true) {
      printf("a = %d\n", a);
    }

    int methodC(bool a = true) {
      printf("a = %d\n", a);
    }

    int methodD() {
      printf("methodD\n");
    }

  };
}