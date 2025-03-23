#ifndef EXPECT_HPP
#define EXPECT_HPP

#include <stdio.h>

class Class {
    void method();
    void method2(const int& a = 1, const int& b = 2);
    const int method3() const;
};

#endif