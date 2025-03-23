#include "expect.hpp"

A& A::operator=(const A&) {

};
bool A::operator==(const A&) const {

};
A operator*(const A&, int) {

};
A::operator int() const {

};
A A::operator++(int) {

};
A& A::operator[](int) {

};
A A::operator()(int) {

};
template<typename T>
bool operator<(const T&, const T&) {
    return true;
};
bool operator<(const std::string& a, const std::string& b) {
    return a < b;
};
A operator+(const A&, const A&) {

};