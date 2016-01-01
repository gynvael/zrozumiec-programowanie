import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;

class PoolTest {
  public static void main(String args[]) {
    // Stworzenie nowej puli wątków.
    ExecutorService pool = Executors.newFixedThreadPool(4);

    // Zadanie 100 zadań puli. Tylko 4 zadania na raz będą aktywne. Metoda
    // submit zwraca obiekt implementujący interfejs Future, który można
    // odpytać czy dane zadanie zostało już wykonane.
    // Alternatywnie możnaby użyć prostszej metody execute.
    Future[] tasks = new Future[80];
    for (int i = 0; i < 80; i++) {
      tasks[i] = pool.submit(new MyThread(i));
    }

    // Zamknij przyjmowanie nowy zadań do puli i usuń pulę po zakończeniu
    // wszystkich zadań.
    pool.shutdown();

    // Odpytaj po kolej każde zadanie czy już się zakończyło, aż wszystkie
    // się nie zakończą.
    while (!pool.isTerminated()) {
      
      // Sprawdz i wyswietl stan wszystkich zadań.
      String s = "";
      for (int i = 0; i < 80; i++) {
        s += tasks[i].isDone() ? "D" : ".";
      }

      System.out.println(s);

      // Poczekaj 50ms przed kontynuowaniem.
      try {
        Thread.sleep(50);
      } catch (InterruptedException e) {
        // W tym programie żaden wątek nie przerywa innego, więc to się
        // nigdy nie wykona.
      }
    }

    System.out.println("All done!");
  }
}

class MyThread implements Runnable{
  private int data;
  MyThread(int data) { 
    this.data = data;
  }

  public void run() {
    try {
      // Poczekaj pewną ilość czasu przed zakończeniem zadania.
      Thread.sleep(25 * (this.data % 7));
    } catch (InterruptedException e) {
      // W tym programie żaden wątek nie przerywa innego, więc to się
      // nigdy nie wykona.      
    }
  }
}

