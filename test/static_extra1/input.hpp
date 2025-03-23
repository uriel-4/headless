#include <map>
#include <string>

class A {
    static std::map<std::string, std::function<void(int)>> a = { { "1", [](int b){} } };
};