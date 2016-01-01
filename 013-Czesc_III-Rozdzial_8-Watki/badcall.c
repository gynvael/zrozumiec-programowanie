void func(void (*funcptr)(void)) { }
void __stdcall stdfunc(void) { }

int main(void) {
  func(stdfunc);
  return 0;
}


