// g++ -std=c++11 decimal64_eq.cc -Wall -Wextra
// Tested on g++ (Ubuntu 4.8.2-19ubuntu1) 4.8.2
#include <cstdio>
#include <vector>

typedef float _Decimal64 __attribute__((mode(DD)));
// Alternatively class std::decimal::decimal64 from <decimal/decimal> could be
// used here. 

int main()
{
  std::vector<_Decimal64> k{
    0.0dd, 0.1dd, 0.2dd, 0.3dd, 0.4dd,
    0.5dd, 0.6dd, 0.7dd, 0.8dd, 0.9dd,
    1.0dd     
  };
  _Decimal64 x = 0.0dd;
  _Decimal64 dx = 0.1dd;

  for (const auto& ki : k) {
    // Specifier %Df should work with _Decimal64 type, but not all printf
    // implementations support it.
    // printf("%Df %Df %s \t%.20Df %.20Df\n",
    //        ki, x, x == ki ? "True" : "False", ki, x);
    printf("%f %f %s\n", (float)ki, (float)x, x == ki ? "True" : "False");
    x += dx;
  }  

  return 0;
}

