#include <string>
#include <map>

int a = 1;
int j (int k = 4) {
  return k * k;
}

std::string s = "123";

class A {
public:
  static auto i = 1;
  static int a = 1;
  static bool b = true;
  static constexpr int c = 1;
  static int o = 1, j = 4;
  static int method(int j = 1) {

  }

  static std::string s2 = "123" + "456";
  static std::map<std::string, std::function<void(int = 5)>> myMap = {};
  static std:strings str1 = "123", str2 = "456", str3 = "789";
  static std::pair<int, int> pair = {1, 2};

  static std::string s3 // comment with =
      = "123";

private:

  static int y; int z = 1;

};