#include <string>
#include <map>

extern int a;
int j (int k = 4);

extern std::string s;

class A {
public:
    static int i;
    static int a;
    static bool b;
    static constexpr int c = 1;
    static int o, j;
    static int method(int j = 1);

    static std::string s2;
    static std::map<std::string, bool> myMap;
    static std::string str1, str2, str3;
    static std::pair<int, int> pair;

    static std::string s3;

private:

    static int y; int z = 1;

};