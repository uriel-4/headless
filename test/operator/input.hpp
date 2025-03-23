#include <string>

class A {
public:
    A& operator=(const A&) {

    }
    bool operator==(const A&) const {

    }
    friend A operator*(const A&, int) {

    }
    operator int() const {

    }
    inline A& operator++() {

    }
    A operator++(int) {

    }
    A& operator[](int) {

    }
    A operator()(int) {

    }
};

template<typename T>
bool operator<(const T&, const T&) {
    return true;
};

extern template bool operator< <A>(const A&, const A&);

bool operator<(const std::string& a, const std::string& b) {
    return a < b;
};

A operator+(const A&, const A&) {

};