// g++ -std=c++11 condpred.cc -pthread
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

// Prosty kontener na pracę, w założeniu dzielony między wątkami.
class WorkContainer {
  public:
   WorkContainer() : some_number_(0) { }
   std::condition_variable& cond() {
     return cond_;
   }

   std::mutex& mutex() {
     return m_;
   }

   int value() {
     return some_number_;
   }

   void set_value(int v) {
     some_number_ = v;
   }

  private:
   int some_number_;
   std::condition_variable cond_;
   std::mutex m_;  // Synchronizuje dostęp do powyższych obiektów.
};

void Producer(WorkContainer *work) {
  std::chrono::milliseconds sleep_time(15);
  for (int i = 0; i < 10; i++) {
    std::this_thread::sleep_for(sleep_time);

    printf("Setting value: %i\n", i);
    work->mutex().lock();
    work->set_value(i);
    work->mutex().unlock();

    // Powiadom oczekujący wątek.
    work->cond().notify_one();  // C++11 nie wymaga aby wątek miał zajęty muteks
                                // w momencie wywołania notify_*. 
  }
}

void Consumer(WorkContainer *work) {
  // Obudź się tylko dla wartości 8.
  std::unique_lock<std::mutex> lock(work->mutex());
  work->cond().wait(lock, [&work](){
    // Anonimowa funkcja (lambda) która używa zmiennej work od rodzica i
    // sprawdza warunek, zwracając true lub false.
    return work->value() == 8;
  });
  printf("Woke up at value: %i\n", work->value());
}

int main() {
  // Stwórz kontener na wszystkie obiekty związane z pracą i uruchom wątek
  // konsumenta oraz producenta (w tej kolejności).
  WorkContainer work;
  std::thread con(Consumer, &work);
  std::thread pro(Producer, &work);

  // Poczekaj aż oba wątki zakończą pracę.  
  pro.join();
  con.join();
  return 0;
}

