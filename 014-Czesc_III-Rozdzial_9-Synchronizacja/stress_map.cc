#include <stdio.h>
#include <windows.h>

#include <string>
#include <unordered_map>

std::unordered_map<std::string, unsigned int> g_dict;
const char *g_keys[] = {
  "k1", "k2", "k3", "k4", "k5", "k6"
};

DWORD WINAPI Purger(LPVOID data) {
  unsigned int poor_mans_random = 647;

  for (;;) {
    g_dict.erase(g_keys[poor_mans_random % 6]);
    poor_mans_random = (poor_mans_random * 4967 + 1777) % 1283;
  }

  return 0;    
}

DWORD WINAPI Adder(LPVOID data) {
  unsigned int poor_mans_random = 499;

  for (;;) {
    g_dict[g_keys[poor_mans_random % 6]] = poor_mans_random;
    poor_mans_random = (poor_mans_random * 4967 + 1777) % 1283;
  }

  return 0;    
}

int main(void) {
  CreateThread(NULL, 0, Purger, NULL, 0, NULL);
  CreateThread(NULL, 0, Adder, NULL, 0, NULL);

  unsigned int result = 0;
  for(;;) {
    result += g_dict["k1"];
  }

  return (int)result;
}


