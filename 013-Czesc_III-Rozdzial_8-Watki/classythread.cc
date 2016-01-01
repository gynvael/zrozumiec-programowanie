#include <stdio.h>
#include <windows.h>

#include <memory>

class MyThread {
 public:
  MyThread(int data) : data_(data) { }

  // Główna metoda nowego wątku.
  void ThreadProc() {
    printf("Proof that I have access to data: %x\n", data_);
  }

  static DWORD WINAPI ThreadProcWrapper(LPVOID obj) {
    MyThread *my_thread = static_cast<MyThread*>(obj);
    my_thread->ThreadProc();
    return 0;
  }

 private:
  int data_;
};

int main() {
  // Stworzenie nowego wątku.
  std::unique_ptr<MyThread> second_thread(new MyThread(0xCAFE));
  HANDLE h = CreateThread(NULL, 0, 
                          MyThread::ThreadProcWrapper, second_thread.get(), 
                          0, NULL);

  if (h == NULL) {
    fprintf(stderr, "Creating the second thread failed.\n");
    return 1;
  }

  // Oczekiwanie na zakończenie nowego wątku.
  WaitForSingleObject(h, INFINITE);
  CloseHandle(h);
  return 0;
}

