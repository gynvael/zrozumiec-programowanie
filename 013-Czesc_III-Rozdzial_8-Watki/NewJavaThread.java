public class NewJavaThread extends Thread {

  private int data;
  private int retval;

  public void run() {
    System.out.println("I was run in a new thread with " + 
                       Integer.toHexString(data) + 
                       " as data.");
    retval = 0xC0DE;
  }

  public int getReturnValue() {
    return retval;
  }

  public static void main(String args[]) {
    // Stworzenie nowego wątku.
    NewJavaThread th = new NewJavaThread();
    th.data = 0x12345678;
    th.start();

    // Oczekiwanie na zakończenie nowego wątku.
    while (true) {
      try {
        th.join();
        break;
      } catch (InterruptedException e) {
        // Oczekiwanie zostało przerwane. W tym przypadku nie powinno się to
        // nigdy zdarzyć (drugi wątek nie robi nic, co mogłoby spowodować
        // przerwanie pierwszego wątku), niemniej kompilatory Java mogą wymagać
        // by to przerwanie zostało złapane.
      }
    }
    System.out.println("Second thread returned: " + 
                       Integer.toHexString(th.getReturnValue()));
  }
}

