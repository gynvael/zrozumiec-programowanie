// i686-w64-mingw32-g++ -Wall -Wextra -std=c++11 ./spammap_fix1.cc -static
#include <stdio.h>

#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

std::unordered_map<std::string, unsigned int> g_dict;
std::mutex g_dict_mutex;

const char *g_keys[] = {
  "k1", "k2", "k3", "k4", "k5", "k6"
};

void Purger() {
  unsigned int poor_mans_random = 647;

  for (;;) {
    // Zajęcie muteksu na czas operacji.
    g_dict_mutex.lock();
    g_dict.erase(g_keys[poor_mans_random % 6]);
    g_dict_mutex.unlock();
    poor_mans_random = (poor_mans_random * 4967 + 1777) % 1283;
  }
}

void Adder() {
  unsigned int poor_mans_random = 499;

  for (;;) {
    // Zajęcie muteksu na czas operacji.
    g_dict_mutex.lock();    
    g_dict[g_keys[poor_mans_random % 6]] = poor_mans_random;
    g_dict_mutex.unlock();
    poor_mans_random = (poor_mans_random * 4967 + 1777) % 1283;
  }
}

int main(void) {
  std::thread purger(Purger);
  std::thread adder(Adder);

  unsigned int result = 0;
  for(;;) {
    // Zajęcie muteksu korzystając z RAII (zostanie automatycznie zwolniony na
    // końcu bloku kodu).
    std::lock_guard<std::mutex> lock(g_dict_mutex);
    result += g_dict["k1"];
  }

  return (int)result;
}


