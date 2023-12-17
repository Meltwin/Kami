#include "iostream"

struct Tester {
  static void callerA() { std::cout << "Caller A" << std::endl; }
  static void callerB() { std::cout << "Caller B" << std::endl; }
  static void callerC() { std::cout << "Caller C" << std::endl; }
};

#define call_funct(func) Tester::func()

int main() {
  call_funct(callerA);
  call_funct(callerB);
  call_funct(callerC);
}